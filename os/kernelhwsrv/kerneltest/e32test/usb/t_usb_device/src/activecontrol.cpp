// Copyright (c) 2000-2009 Nokia Corporation and/or its subsidiary(-ies).
// All rights reserved.
// This component and the accompanying materials are made available
// under the terms of the License "Eclipse Public License v1.0"
// which accompanies this distribution, and is available
// at the URL "http://www.eclipse.org/legal/epl-v10.html".
//
// Initial Contributors:
// Nokia Corporation - initial contribution.
//
// Contributors:
//
// Description:
// e32test/usb/t_usb_device/src/activecontrol.cpp
// USB Test Program T_USB_DEVICE, functional part.
// Device-side part, to work against T_USB_HOST running on the host.
// 
//


#include "general.h"									
#include "usblib.h"											// Helpers
#include "config.h"
#include "activecontrol.h"
#include "apitests.h"
#include "activerw.h"

void StartMassStorage(RDEVCLIENT* aPort);
void StopMassStorage(RDEVCLIENT* aPort);

enum Ep0Requests
	{
	EStop = 0x1,
	EVersion = 0x10,
	ETestParam = 0x20,
	ETestResult = 0x30,
	ETestFail = 0x40,
	ETestConnect = 0x50,
	ETestDisconnect = 0x60,
	ETestMassStorage = 0x70,
	ETestIdleCounter = 0x80,
	};

extern RTest test;
extern TBool gVerbose;
extern TBool gSkip;
extern TBool gTempTest;
extern TBool gStopOnFail;
extern TBool gAltSettingOnNotify;
extern TInt gSoakCount;
extern CActiveRW* gRW[KMaxConcurrentTests];				// the USB read/write active object
extern IFConfigPtr gInterfaceConfig [128] [KMaxInterfaceSettings];
extern TInt gActiveTestCount;
#ifdef USB_SC
extern RChunk gChunk;
#endif

TInt firstBulkOutEndpoint = -1;

_LIT(KTestIdleCounterChunkName, "TestIdleCounter");
_LIT(KTestIdleCounterPanic, "IdleCounter");

enum TTestIdleCounterPanic
	{
	ETestIdleCounterWrongCommand
	};

enum TTestIdleCounterCommand
	{
	ETestIdleCounterDoNothing,
	ETestIdleCounterReset,
	ETestIdleCounterClose
	};

struct TTestIdleCounter
	{
	volatile TInt64 iCounter;
	volatile TTestIdleCounterCommand iCommand;
	};

TInt IdleCounterThread(TAny*)
	{
	TInt r;
	//
	RThread().SetPriority(EPriorityAbsoluteVeryLow);
	//
	RChunk chunk;
	r = chunk.CreateGlobal(KTestIdleCounterChunkName,
			sizeof(struct TTestIdleCounter),
			sizeof(struct TTestIdleCounter) + 1);
	if (r == KErrNone)
		{
		struct TTestIdleCounter* counter = (struct TTestIdleCounter*) chunk.Base();
		counter->iCounter = 0;
		counter->iCommand = ETestIdleCounterDoNothing;
		//
		FOREVER
			{
			TInt command = counter->iCommand;
			if (command == ETestIdleCounterReset)
				{
				counter->iCounter = 0;
				counter->iCommand = ETestIdleCounterDoNothing;
				}
			else if (command == ETestIdleCounterClose)
				{
				break;
				}
			else if (command != ETestIdleCounterDoNothing)
				{
				RThread().Panic(KTestIdleCounterPanic, ETestIdleCounterWrongCommand);
				}
			//
			counter->iCounter++;
			}
		//
		chunk.Close();
		}
	return r;
	}

//
// --- class CActiveControl ---------------------------------------------------------
//

CActiveControl::CActiveControl(CConsoleBase* aConsole, TDes * aConfigFile, TDes * aScriptFile)
	: CActive(EPriorityNormal),
	  iConsole(aConsole),
	  iSoftwareConnect(EFalse),
	  iHighSpeed(EFalse),
	  iConfigFileName(aConfigFile),
	  iScriptFileName(aScriptFile),
	  iEp0PacketSize(0)
	{}


CActiveControl* CActiveControl::NewLC(CConsoleBase* aConsole, TDes * aConfigFile, TDes * aScriptFile)
	{
	CActiveControl* self = new (ELeave) CActiveControl(aConsole, aConfigFile, aScriptFile);
	CleanupStack::PushL(self);
	self->ConstructL();
	return self;
	}


CActiveControl* CActiveControl::NewL(CConsoleBase* aConsole, TDes * aConfigFile, TDes * aScriptFile)
	{
	CActiveControl* self = NewLC(aConsole, aConfigFile, aScriptFile);
	CleanupStack::Pop();
	return self;
	}


void CActiveControl::ConstructL()
	{
	CActiveScheduler::Add(this);
	TInt r;
	
	User::LeaveIfError(iFs.Connect());

	test.Start (_L("Configuration"));
	
	test_Compare(iConfigFileName->Length(),!=,0);
		
	iTimer.CreateLocal();
	iPending = EPendingNone;
	
	test.Next (_L("Open configuration file"));
	// set the session path to use the ROM if no drive specified
	r=iFs.SetSessionPath(_L("Z:\\test\\"));
	test_KErrNone(r);

	r = iConfigFile.Open(iFs, * iConfigFileName, EFileShareReadersOnly | EFileStreamText | EFileRead);
	test_KErrNone(r);
	TUSB_VERBOSE_PRINT1("Configuration file %s Opened successfully", iConfigFileName->PtrZ());

	test.Next (_L("Process configuration file"));
	test(ProcessConfigFile (iConfigFile,iConsole,&iLddPtr));
	
	iConfigFile.Close();

	test.Next (_L("LDD in configuration file"));
	test_NotNull(iLddPtr);
		
	LDDConfigPtr lddPtr = iLddPtr;
	TInt nextPort = 0;
	while (lddPtr != NULL)
		{
		// Load logical driver (LDD)
		// (There's no physical driver (PDD) with USB: it's a kernel extension DLL which
		//  was already loaded at boot time.)
		test.Next (_L("Loading USB LDD"));
		TUSB_VERBOSE_PRINT1("Loading USB LDD ",lddPtr->iName.PtrZ());
		r = User::LoadLogicalDevice(lddPtr->iName);
		test(r == KErrNone || r == KErrAlreadyExists);
	
		IFConfigPtr ifPtr = lddPtr->iIFPtr;
		
		test.Next (_L("Opening Channels"));
		for (TInt portNumber = nextPort; portNumber < nextPort+lddPtr->iNumChannels; portNumber++)
			{
			test_Compare(lddPtr->iNumChannels,>,0);

			// Open USB channel
			r = iPort[portNumber].Open(0);
			test_KErrNone(r);
			TUSB_VERBOSE_PRINT("Successfully opened USB port");

			// Query the USB device/Setup the USB interface
			if (portNumber == nextPort)
				{
				// Change some descriptors to contain suitable values
				SetupDescriptors(lddPtr, &iPort[portNumber]);
				}
				
			if (portNumber == 0)
				{
				QueryUsbClientL(lddPtr, &iPort[portNumber]);
				}

			test_NotNull(ifPtr);

			IFConfigPtr defaultIfPtr = ifPtr;
			SetupInterface(&ifPtr,portNumber);
					
			#ifdef USB_SC
			RChunk *tChunk = &gChunk;
			test_KErrNone(iPort[portNumber].FinalizeInterface(tChunk));
			#endif

			// 	allocate endpoint DMA and double buffering for all endpoints on default interface
			for (TUint8 i = 1; i <= defaultIfPtr->iInfoPtr->iTotalEndpointsUsed; i++)
				{
				defaultIfPtr->iEpDMA[i-1] ? AllocateEndpointDMA(&iPort[portNumber],(TENDPOINTNUMBER)i) : DeAllocateEndpointDMA(&iPort[portNumber],(TENDPOINTNUMBER)i);
				#ifndef USB_SC
				defaultIfPtr->iEpDoubleBuff[i-1] ? AllocateDoubleBuffering(&iPort[portNumber],(TENDPOINTNUMBER)i) : DeAllocateDoubleBuffering(&iPort[portNumber],(TENDPOINTNUMBER)i);
				#endif
				}				

			}
	
		iTotalChannels += lddPtr->iNumChannels;
		nextPort += lddPtr->iNumChannels;	
		lddPtr = lddPtr->iPtrNext;	
		}
		
	TUSB_VERBOSE_PRINT("All Interfaces and Alternate Settings successfully set up");
	
	test.Next (_L("Start Idle Counter Thread"));
	r = iIdleCounterThread.Create(_L("IdleCounter"), IdleCounterThread, KDefaultStackSize, KMinHeapSize, KMinHeapSize, NULL);
	test_KErrNone(r);
	iIdleCounterThread.Resume();
	// Allow some time for low-priority counter process
	User::After(100000); // 0.1 second
	r = iIdleCounterChunk.OpenGlobal(KTestIdleCounterChunkName, EFalse);
	test_KErrNone(r);
	iIdleCounter = (struct TTestIdleCounter*) iIdleCounterChunk.Base();
	test_NotNull(iIdleCounter);
	// Allow some time for low-priority counter process
	User::After(100000); // 0.1 second
	TInt64 val1 = iIdleCounter->iCounter;
	User::After(1000000); // 1 second
	TInt64 val2 = iIdleCounter->iCounter;
	TUSB_PRINT1("Idle Counter when test inactive: %Ldinc/ms", (val2 - val1) / 1000);

	test.Next (_L("Enumeration..."));
	r = ReEnumerate();
	test_KErrNone(r);
		
	TUSB_VERBOSE_PRINT("Device successfully re-enumerated\n");


	if (iLddPtr->iHighSpeed && !gSkip)
		{
		test.Next (_L("High Speed"));
		test(iHighSpeed);	
		}
			
	test.Next (_L("Create Notifiers"));
	for (TInt portNumber = 0; portNumber < iTotalChannels; portNumber++)
		{

		// Create device state active object
		iDeviceStateNotifier[portNumber] = CActiveDeviceStateNotifier::NewL(iConsole, &iPort[portNumber], portNumber);
		test_NotNull(iDeviceStateNotifier[portNumber]);
		iDeviceStateNotifier[portNumber]->Activate();
		TUSB_VERBOSE_PRINT("Created device state notifier");

		// Create endpoint stall status active object
		iStallNotifier[portNumber] = CActiveStallNotifier::NewL(iConsole, &iPort[portNumber]);
		test_NotNull(iStallNotifier[portNumber]);
		iStallNotifier[portNumber]->Activate();
		TUSB_VERBOSE_PRINT("Created stall notifier");
			
		TestInvalidSetInterface (&iPort[portNumber],iNumInterfaceSettings[portNumber]);			
		TestInvalidReleaseInterface (&iPort[portNumber],iNumInterfaceSettings[portNumber]);
			
		}
		
	test.Next (_L("Endpoint Zero Max Packet Sizes"));
	TUint ep0Size = iPort[0].EndpointZeroMaxPacketSizes();
	switch (ep0Size)
		{
		case KUsbEpSize8 :
			iEp0PacketSize = 8;
			break;
					
		case KUsbEpSize16 :
			iEp0PacketSize = 16;
			break;

		case KUsbEpSize32 :
			iEp0PacketSize = 32;
			break;

		case KUsbEpSize64 :
			iEp0PacketSize = 64;
			break;
					
		default:
			iEp0PacketSize = 0;
			break;		
		}
	test_Compare(iEp0PacketSize,>,0);

	test.Next (_L("Set Device Control"));
	r = iPort[0].SetDeviceControl();
	test_KErrNone(r);

	#ifdef USB_SC
	r = iPort[0].OpenEndpoint(iEp0Buf,0);
	test_KErrNone(r);
	#endif
	
	test.End();
	
	}

void CActiveControl::ReConnect()
	{
	TInt r;

	test.Start (_L("Reconnecting USB"));
	LDDConfigPtr lddPtr = iLddPtr;
	TInt nextPort = 0;
	while (lddPtr != NULL)
		{
		IFConfigPtr ifPtr = lddPtr->iIFPtr;
		
		test.Next (_L("Opening Channels"));
		for (TInt portNumber = nextPort; portNumber < nextPort+lddPtr->iNumChannels; portNumber++)
			{
			// Open USB channel
			r = iPort[portNumber].Open(0);
			test_KErrNone(r);
			TUSB_VERBOSE_PRINT("Successfully opened USB port");

			// Query the USB device/Setup the USB interface
			if (portNumber == nextPort)
				{
				// Change some descriptors to contain suitable values
				SetupDescriptors(lddPtr, &iPort[portNumber]);
				}
				
			IFConfigPtr defaultIfPtr = ifPtr;
			SetupInterface(&ifPtr,portNumber);
					
			#ifdef USB_SC
			RChunk *tChunk = &gChunk;
			test_KErrNone(iPort[portNumber].FinalizeInterface(tChunk));
			#endif

			// 	allocate endpoint DMA and double buffering for all endpoints on default interface
			for (TUint8 i = 1; i <= defaultIfPtr->iInfoPtr->iTotalEndpointsUsed; i++)
				{
				defaultIfPtr->iEpDMA[i-1] ? AllocateEndpointDMA(&iPort[portNumber],(TENDPOINTNUMBER)i) : DeAllocateEndpointDMA(&iPort[portNumber],(TENDPOINTNUMBER)i);
				#ifndef USB_SC
				defaultIfPtr->iEpDoubleBuff[i-1] ? AllocateDoubleBuffering(&iPort[portNumber],(TENDPOINTNUMBER)i) : DeAllocateDoubleBuffering(&iPort[portNumber],(TENDPOINTNUMBER)i);
				#endif
				}				
			}
	
		nextPort += lddPtr->iNumChannels;	
		lddPtr = lddPtr->iPtrNext;	
		}
		
	TUSB_VERBOSE_PRINT("All Interfaces and Alternate Settings successfully set up");

	test.Next (_L("Enumeration..."));
	r = ReEnumerate();
	test_KErrNone(r);
		
	TUSB_VERBOSE_PRINT("Device successfully re-enumerated\n");
	
	for (TInt portNumber = 0; portNumber < iTotalChannels; portNumber++)
		{
		// Create device state active object
		iDeviceStateNotifier[portNumber] = CActiveDeviceStateNotifier::NewL(iConsole, &iPort[portNumber], portNumber);
		test_NotNull(iDeviceStateNotifier[portNumber]);
		iDeviceStateNotifier[portNumber]->Activate();
		TUSB_VERBOSE_PRINT("Created device state notifier");

		// Create endpoint stall status active object
		iStallNotifier[portNumber] = CActiveStallNotifier::NewL(iConsole, &iPort[portNumber]);
		test_NotNull(iStallNotifier[portNumber]);
		iStallNotifier[portNumber]->Activate();
		TUSB_VERBOSE_PRINT("Created stall notifier");

		if (portNumber == 0)
			{
			test.Next (_L("Set Device Control"));
			r = iPort[portNumber].SetDeviceControl();
			test_KErrNone(r);

			#ifdef USB_SC
			r = iPort[portNumber].OpenEndpoint(iEp0Buf,0);
			test_KErrNone(r);
			#endif
			
			}
		}
	
	test.End();
	}
	
void CActiveControl::SetupInterface(IFConfigPtr* aIfPtr, TInt aPortNumber)
	{
	test.Start (_L("Setup Interface"));
	
	// first of all set the default interface	
	TUSB_PRINT2 ("Set Default Interface with %d endpoints bandwidth 0x%x",(*aIfPtr)->iInfoPtr->iTotalEndpointsUsed,(*aIfPtr)->iBandwidthIn | (*aIfPtr)->iBandwidthOut);
	#ifdef USB_SC
	TUsbcScInterfaceInfoBuf ifc = *((*aIfPtr)->iInfoPtr);
	TInt r = iPort[aPortNumber].SetInterface(0, ifc);
	#else
	TUsbcInterfaceInfoBuf ifc = *((*aIfPtr)->iInfoPtr);
	TInt r = iPort[aPortNumber].SetInterface(0, ifc, (*aIfPtr)->iBandwidthIn | (*aIfPtr)->iBandwidthOut);
	#endif
	test_KErrNone(r);

	TBuf8<KUsbDescSize_Interface> ifDescriptor;
	r = iPort[aPortNumber].GetInterfaceDescriptor(0, ifDescriptor);
	test_KErrNone(r);

	// Check the interface descriptor
	test(ifDescriptor[KIfcDesc_SettingOffset] == 0 && ifDescriptor[KIfcDesc_NumEndpointsOffset] == (*aIfPtr)->iInfoPtr->iTotalEndpointsUsed &&
	    ifDescriptor[KIfcDesc_ClassOffset] == (*aIfPtr)->iInfoPtr->iClass.iClassNum &&
	    ifDescriptor[KIfcDesc_SubClassOffset] == (*aIfPtr)->iInfoPtr->iClass.iSubClassNum &&
	    ifDescriptor[KIfcDesc_ProtocolOffset] == (*aIfPtr)->iInfoPtr->iClass.iProtocolNum);

	if ((*aIfPtr)->iNumber != 0 && ifDescriptor[KIfcDesc_NumberOffset] != (*aIfPtr)->iNumber)
		{
		ifDescriptor[KIfcDesc_NumberOffset] = (*aIfPtr)->iNumber;
		r = iPort[aPortNumber].SetInterfaceDescriptor(0, ifDescriptor);	
		test_KErrNone(r);
		}
	else
		{
		(*aIfPtr)->iNumber = ifDescriptor[KIfcDesc_NumberOffset];	
		}
	TUint8 interfaceNumber = (*aIfPtr)->iNumber;
	TUSB_PRINT1 ("Interface Number %d",interfaceNumber);
		
	// Check all endpoint descriptors
	TBuf8<KUsbDescSize_AudioEndpoint> epDescriptor;
	for (TUint i = 0; i < (*aIfPtr)->iInfoPtr->iTotalEndpointsUsed; i++)
		{
		if (!gSkip)
			{
			TestEndpointDescriptor (&iPort[aPortNumber],0,i+1,(*aIfPtr)->iInfoPtr->iEndpointData[i]);	

			}

		if (firstBulkOutEndpoint < 0 && ((*aIfPtr)->iInfoPtr->iEndpointData[i].iDir & KUsbEpDirOut) &&
			(*aIfPtr)->iInfoPtr->iEndpointData[i].iType == KUsbEpTypeBulk)
			{
			firstBulkOutEndpoint = i+1;	
			}
		}

	TUSB_PRINT1 ("Interface number is %d",interfaceNumber);
	(*aIfPtr)->iPortNumber = aPortNumber;
	gInterfaceConfig [interfaceNumber] [0] = *aIfPtr;

	TInt alternateNumber = 1;
	// check for any alternatate interfaces and set any that are found
	* aIfPtr = (*aIfPtr)->iPtrNext;
	if (* aIfPtr != NULL)
		{
		test(SupportsAlternateInterfaces());

		IFConfigPtr ifPtr = *aIfPtr;
		while (ifPtr != NULL)
			{
			if (ifPtr->iAlternateSetting)
				{
				ifc = *(ifPtr->iInfoPtr);
				#ifdef USB_SC
				TUSB_PRINT2 ("Set Alternate Interface Setting %d with %d endpoints",alternateNumber,ifPtr->iInfoPtr->iTotalEndpointsUsed);
				r = iPort[aPortNumber].SetInterface(alternateNumber, ifc);
				#else
				TUSB_PRINT3 ("Set Alternate Interface Setting %d with %d endpoints bandwidth 0x%x",alternateNumber,ifPtr->iInfoPtr->iTotalEndpointsUsed,ifPtr->iBandwidthIn | iLddPtr->iIFPtr->iBandwidthOut);
				r = iPort[aPortNumber].SetInterface(alternateNumber, ifc, ifPtr->iBandwidthIn | iLddPtr->iIFPtr->iBandwidthOut);
				#endif
				test_KErrNone(r);
					
				r = iPort[aPortNumber].GetInterfaceDescriptor(alternateNumber, ifDescriptor);
				test_KErrNone(r);

				// Check the interface descriptor
				test(ifDescriptor[KIfcDesc_SettingOffset] == alternateNumber && ifDescriptor[KIfcDesc_NumEndpointsOffset] == (*aIfPtr)->iInfoPtr->iTotalEndpointsUsed &&
				    ifDescriptor[KIfcDesc_ClassOffset] == (*aIfPtr)->iInfoPtr->iClass.iClassNum &&
				    ifDescriptor[KIfcDesc_SubClassOffset] == (*aIfPtr)->iInfoPtr->iClass.iSubClassNum &&
				    ifDescriptor[KIfcDesc_ProtocolOffset] == (*aIfPtr)->iInfoPtr->iClass.iProtocolNum);

				// Check all endpoint descriptors
				for (TUint i = 0; i < ifPtr->iInfoPtr->iTotalEndpointsUsed; i++)
					{
					TInt desc_size;
					r = iPort[aPortNumber].GetEndpointDescriptorSize(alternateNumber, i+1, desc_size);
					test_KErrNone(r);
					test_Equal(KUsbDescSize_Endpoint + (*aIfPtr)->iInfoPtr->iEndpointData[i].iExtra,static_cast<TUint>(desc_size));

					r = iPort[aPortNumber].GetEndpointDescriptor(alternateNumber, i+1, epDescriptor);
					test_KErrNone(r);
					
					test((((*aIfPtr)->iInfoPtr->iEndpointData[i].iDir & KUsbEpDirIn) && (epDescriptor[KEpDesc_AddressOffset] & 0x80) ||
						!((*aIfPtr)->iInfoPtr->iEndpointData[i].iDir & KUsbEpDirIn) && !(epDescriptor[KEpDesc_AddressOffset] & 0x80)) &&
						EpTypeMask2Value((*aIfPtr)->iInfoPtr->iEndpointData[i].iType) == (TUint)(epDescriptor[KEpDesc_AttributesOffset] & 0x03) &&
						(*aIfPtr)->iInfoPtr->iEndpointData[i].iInterval == epDescriptor[KEpDesc_IntervalOffset]);


					if (!gSkip && (*aIfPtr)->iInfoPtr->iEndpointData[i].iExtra)
						{
						test.Next(_L("Extended Endpoint Descriptor Manipulation"));
						TUint8 addr = 0x85;										// bogus address
						if (epDescriptor[KEpDesc_SynchAddressOffset] == addr)
							addr++;
						epDescriptor[KEpDesc_SynchAddressOffset] = addr;
						r = iPort[aPortNumber].SetEndpointDescriptor(alternateNumber, i+1, epDescriptor);
						test_KErrNone(r);

						TBuf8<KUsbDescSize_AudioEndpoint> descriptor2;
						r = iPort[aPortNumber].GetEndpointDescriptor(alternateNumber, i+1, descriptor2);
						test_KErrNone(r);

						test.Next(_L("Compare endpoint descriptor with value set"));
						r = descriptor2.Compare(epDescriptor);
						test_KErrNone(r);						
						}
					}
				
					
				// if no error move on to the next interface
				ifPtr->iPortNumber = aPortNumber;
				ifPtr->iNumber = interfaceNumber;
				gInterfaceConfig [interfaceNumber] [alternateNumber] = ifPtr;

				alternateNumber++;
				ifPtr = ifPtr->iPtrNext;
				* aIfPtr = ifPtr;
				}
			else
				{
				ifPtr = NULL;
				}
			}
		}
	iNumInterfaceSettings[aPortNumber] = alternateNumber;
	if (!gSkip)
		{
		TestInvalidSetInterface (&iPort[aPortNumber],iNumInterfaceSettings[aPortNumber]);			
		TestInvalidReleaseInterface (&iPort[aPortNumber],iNumInterfaceSettings[aPortNumber]);

		TestDescriptorManipulation(iLddPtr->iHighSpeed,&iPort[aPortNumber],alternateNumber);
		TestOtgExtensions(&iPort[aPortNumber]);
		TestEndpoint0MaxPacketSizes(&iPort[aPortNumber]);
		}
		
	test.End();
	}


CActiveControl::~CActiveControl()
	{
	TUSB_PRINT("CActiveControl::~CActiveControl()");

	Cancel();
	
	iTimer.Close();
	
	// delete interfaces		
	while (iLddPtr->iIFPtr)
		{
		IFConfigPtr* ifPtrPtr = & iLddPtr->iIFPtr;
		while ((*ifPtrPtr)->iPtrNext)
			{
			ifPtrPtr = &(*ifPtrPtr)->iPtrNext;
			}
		delete (*ifPtrPtr)->iInfoPtr->iString;
		delete (*ifPtrPtr)->iInfoPtr;
		delete (*ifPtrPtr);
		* ifPtrPtr = NULL;
		}

	while (iLddPtr)
		{
		LDDConfigPtr* lddPtrPtr = &iLddPtr;	
		while ((*lddPtrPtr)->iPtrNext)
			{
			lddPtrPtr = &(*lddPtrPtr)->iPtrNext;
			}
		delete (*lddPtrPtr)->iManufacturer;
		delete (*lddPtrPtr)->iProduct;
		delete (*lddPtrPtr)->iSerialNumber;
		delete (*lddPtrPtr);
		* lddPtrPtr = NULL;
		}

	iFs.Close();
	}

void CActiveControl::DoCancel()
	{
	TUSB_VERBOSE_PRINT("CActiveControl::DoCancel()");
	iConsole->ReadCancel();
	}

void CActiveControl::SetMSFinished(TBool aState)
	{
	if (aState)
		{
		if (iPending != EPendingEject)
			{
			iPending = EPendingEject;
			iTimer.After(iStatus,KMSFinishedDelay);
			if (!IsActive())
				{
				SetActive();
				}		
			}
		}
	else
		{
		if (iPending == EPendingEject)
			{
			iPending = EPendingCancel;
			iTimer.Cancel();
			}
		}
	}
	
void CActiveControl::RequestEp0ControlPacket()
	{
	TUSB_VERBOSE_PRINT("CActiveControl::RequestEp0ControlPacket()");
	// A request is issued to read a packet for endpoint 0
	__ASSERT_ALWAYS(!IsActive(), User::Panic(KActivePanic, 660));
	#ifdef	USB_SC
	TInt r = 0;
	do
		{
		r = iEp0Buf.GetBuffer (iEp0Packet,iEp0Size,iEp0Zlp,iStatus);
		TUSB_VERBOSE_PRINT4("Get Buffer Return code %d Status %d PacketPtr 0x%x Size %d", r, iStatus.Int(),iEp0Packet,iEp0Size);	
		test_Value(r, (r == KErrNone) || (r == KErrCompletion) || (r == TEndpointBuffer::KStateChange) || (r == KErrAlternateSettingChanged));  
		if (r == KErrCompletion)
			{
			// ignore anything except a setup packet
			if ((TInt)iEp0Size == KSetupPacketSize)
				{
				iEp0SetUpPacket.Copy((TUint8 *)iEp0Packet,iEp0Size);
				r = ProcessEp0ControlPacket();
				}
			}
		else
			{
			if (r == KErrNone)
				{
				iPending = EPendingEp0Read;
				SetActive();
				}
			}
		}
	while ((r == KErrCompletion) || (r == TEndpointBuffer::KStateChange) || (r == KErrAlternateSettingChanged));
	#else
	iPort[0].ReadPacket(iStatus,EEndpoint0, iEp0SetUpPacket,KSetupPacketSize);
	iPending = EPendingEp0Read;
	SetActive();
	#endif
	}

void CActiveControl::RunL()
	{
	TInt r = KErrNone;
	
	TUSB_VERBOSE_PRINT("CActiveControl::RunL()");
	
	switch (iPending)
		{
		case EPendingNone :
			break;
			
		case EPendingEp0Read :
			iPending = EPendingNone;
			if (iStatus != KErrNone)
				{
				TUSB_PRINT1("ActiveControl::Error %d in Ep0 Read Packet", iStatus.Int());
				test(EFalse);
				}
			#ifdef USB_SC
			// for shared chunks this means that data is available in the buffer
			// but the data has yet to be transferred to a local buffer
			RequestEp0ControlPacket();
			#else
			if (ProcessEp0ControlPacket() == KErrCompletion)
				RequestEp0ControlPacket();
			#endif		
			break;		

		case EPendingTimer :
			iPending = EPendingNone;
			if (iStatus != KErrNone)
				{
				TUSB_PRINT1("ActiveControl::Error %d in Connection Timer Delay", iStatus.Int());
				test(EFalse);
				}
			r = iPort[0].DeviceConnectToHost();
			test_KErrNone (r);
		
			test.End();
		
			RequestEp0ControlPacket();
			break;
			
		case EPendingEject :
			iPending = EPendingNone;
			if (iStatus != KErrNone)
				{
				TUSB_PRINT1("ActiveControl::Error %d in Eject Timer Delay", iStatus.Int());
				test(EFalse);
				}
			StopMassStorage(&iPort[0]);
			#ifdef USB_SC
				iEp0Buf.Close();
			#endif
			ReConnect();
							
			RequestEp0ControlPacket();
			break;
					
		case EPendingCancel :
			iPending = EPendingNone;
			if (iStatus != KErrNone && iStatus != KErrCancel)
				{
				TUSB_PRINT1("ActiveControl::Error %d in Eject Timer Delay", iStatus.Int());
				test(EFalse);
				}
		}
	
	}

TInt CActiveControl::ProcessEp0ControlPacket()
	{
	TUint16 value = *reinterpret_cast<TUint16*>(&iEp0SetUpPacket[KUsb_Ep0wValueOffset]);
	TUint16 index = *reinterpret_cast<TUint16*>(&iEp0SetUpPacket[KUsb_Ep0wIndexOffset]);
	TUint16 length= *reinterpret_cast<TUint16*>(&iEp0SetUpPacket[KUsb_Ep0wLengthOffset]);
	TUSB_VERBOSE_PRINT3("ProcessEp0ControlPacket length 0x%x value 0x%x index 0x%x",length,value,index);
	TRequestStatus ep0Status;
	TUint8 host_ver_major;
	TUint8 host_ver_minor;
	TUint8 host_ver_micro;
	TUint8 usbio_ver_major;
	TUint8 usbio_ver_minor;
	#ifndef USB_SC
	TBuf8<KMaxControlPacketSize> ep0DataPacket;
	#endif
	TestParamPtr tpPtr;
	TBool firstSettingThread = (index & KFirstSettingThreadMask) ? ETrue : EFalse;
	TBool lastSettingThread = (index & KLastSettingThreadMask) ? ETrue : EFalse;
	index &= ~(KFirstSettingThreadMask | KLastSettingThreadMask);
    CActiveRW* pActiveRW;
	TInt r;
	TBool sendStatus;

	if (((iEp0SetUpPacket[KUsb_Ep0RequestTypeOffset] & KUsbRequestType_DestMask) == KUsbRequestType_DestDevice) &&
		((iEp0SetUpPacket[KUsb_Ep0RequestTypeOffset] & KUsbRequestType_TypeMask) == KUsbRequestType_TypeClass))
		{
		TUSB_VERBOSE_PRINT("Received Device Directed setup packet");
		if ((iEp0SetUpPacket[KUsb_Ep0RequestTypeOffset] & KUsbRequestType_DirMask) == KUsbRequestType_DirToDev)
			{
			iEp0DataBuffer.SetLength(0);
			while (iEp0DataBuffer.Length() < length)
				{
				TUSB_VERBOSE_PRINT("Reading Ep0 data packet");
				#ifdef USB_SC
				r = iEp0Buf.GetBuffer (iEp0Packet,iEp0Size,iEp0Zlp,ep0Status);
				test_Value(r, r == KErrNone || r == KErrCompletion || (r == KErrAlternateSettingChanged));
				while (r == KErrNone)  
					{
					TUSB_VERBOSE_PRINT("Waiting for Ep0 data packet");
					User::WaitForRequest(ep0Status);
					test_KErrNone(ep0Status.Int());
					r = iEp0Buf.GetBuffer (iEp0Packet,iEp0Size,iEp0Zlp,ep0Status);
					test_Value(r, r == KErrNone || r == KErrCompletion || (r == KErrAlternateSettingChanged));
					}
				TUSB_VERBOSE_PRINT1("Ep0 data packet - size %d",iEp0Size);
				iEp0DataBuffer.Append((TUint8 *)iEp0Packet,iEp0Size);								
				#else
				TUint16 packetLength = Min(length-iEp0DataBuffer.Length(),iEp0PacketSize);
				iPort[0].ReadPacket(ep0Status, EEndpoint0, ep0DataPacket, packetLength);
				User::WaitForRequest(ep0Status);
				if (ep0Status == KErrNone)
					{
					iEp0DataBuffer.Append(ep0DataPacket);				
					}
				else
					{
					TUSB_PRINT1("ActiveControl::Error %d in Ep0 Read Data Packet", ep0Status.Int());
					test(EFalse);
					return KErrNone;						
					}
				#endif
				}
			TUSB_VERBOSE_PRINT4("Setup ToDevice Type %d length %d value %d index %d",iEp0SetUpPacket[KUsb_Ep0RequestOffset],length,value,index);
			sendStatus = ETrue;
			switch (iEp0SetUpPacket[KUsb_Ep0RequestOffset])	
				{
				case EStop :
					// send this now as the port will be disconnected
					sendStatus = EFalse;
					r = iPort[0].SendEp0StatusPacket();					
					test_KErrNone(r);
		
					if (value && firstBulkOutEndpoint > 0)
						{
						PrintHostLog();
						}
						
					for (TInt portNumber = 0; portNumber < iTotalChannels; portNumber++)
						{
						// base class cancel -> calls our DoCancel
						delete iDeviceStateNotifier[portNumber];
						delete iStallNotifier[portNumber];
						if (portNumber == 0)
							{
							r = iPort[portNumber].RemoveStringDescriptor(stridx1);
							if (r != KErrNone)
								{
								TUSB_PRINT1("Error %d on string removal", r);
								}
							r = iPort[portNumber].RemoveStringDescriptor(stridx2);
							if (r != KErrNone)
								{
								TUSB_PRINT1("Error %d on string removal", r);
								}	
							}
						TUSB_VERBOSE_PRINT1 ("Closing USB channel number %d",portNumber);
						iPort[portNumber].Close();											// close USB channel
						}
					TUSB_VERBOSE_PRINT("Closing Idle Counter Thread");
					iIdleCounter->iCommand = ETestIdleCounterClose;
					iIdleCounterChunk.Close();
					// Allow time for low-priority thread to close
					User::After(100000);
					iIdleCounterThread.Close();
					
					CActiveScheduler::Stop();
					break;
					
				case EVersion :
					TUSB_PRINT("Receiving t_usb_host version");
					host_ver_major = iEp0DataBuffer[0];
					host_ver_minor = iEp0DataBuffer[1];
					host_ver_micro = iEp0DataBuffer[2];
					usbio_ver_major = iEp0DataBuffer[3];
					usbio_ver_minor = iEp0DataBuffer[4];
					TUSB_PRINT5("Host-side: t_usb_host v%d.%d.%d  USBIO v%d.%d\n",
						host_ver_major, host_ver_minor, host_ver_micro,
						usbio_ver_major, usbio_ver_minor);
					if (host_ver_major < KHostVersionMajor)
						{
						TUSB_PRINT1("t_usb_host version not sufficient (need at least v%d.x.x)\n",KHostVersionMajor);
						User::Leave(-1);
						return KErrNone;
						}
					// Just using '<' instead of the seemingly absurd '<= && !==' doesn't work without
					// GCC compiler warning because Kxxx can also be zero (in which case there's no '<').
					else if ((host_ver_minor <= KHostVersionMinor) &&
			 				!(host_ver_minor == KHostVersionMinor))
						{
						TUSB_PRINT2("t_usb_host version not sufficient (need at least v%d.%d.x)\n",
							KHostVersionMajor, KHostVersionMinor);
						test(EFalse);
						return KErrNone;
						}
					// Just using '<' instead of the seemingly absurd '<= && !==' doesn't work without
					// GCC compiler warning because Kxxx can also be zero (in which case there's no '<').
					else if ((host_ver_micro <= KHostVersionMicro) &&
			 				!(host_ver_micro == KHostVersionMicro))
						{
						TUSB_PRINT3("USBRFLCT version not sufficient (need at least v%d.%d.%d)\n",
									KHostVersionMajor, KHostVersionMinor, KHostVersionMicro);
						test(EFalse);
						return KErrNone;
						}
					break;
					
				case ETestParam :
					tpPtr = (TestParamPtr)(&iEp0DataBuffer[0]);
					TUSB_VERBOSE_PRINT4("Test Params - interface %d repeat %d settingRepeat %d beforeIndex %d",tpPtr->interfaceNumber,tpPtr->repeat,tpPtr->settingRepeat,tpPtr->beforeIndex);
					if (index >= KMaxConcurrentTests)
						{
						TUSB_PRINT2("Test index %d is greater than maximum allowed (%d) concurrent tests",index,KMaxConcurrentTests);
						test(EFalse);
						return KErrNone;
						}
					// Create Reader/Writer active object
					pActiveRW = CActiveRW::NewL(iConsole, &iPort[gInterfaceConfig[tpPtr->interfaceNumber][tpPtr->alternateSetting]->iPortNumber], iFs, index, lastSettingThread);
					if (!pActiveRW)
						{
						TUSB_PRINT("Failed to create reader/writer");
						test(EFalse);
						return KErrNone;
						}
					TUSB_VERBOSE_PRINT("Created reader/writer");
					pActiveRW->SetTestParams(tpPtr);
					switch (value)
						{
					case 'X' :
						test.Start (_L("Xml"));
						break;
							
					case 'L' :
						test.Start (_L("Loop"));
						pActiveRW->SetTransferMode(ELoop);
						gAltSettingOnNotify = ETrue;
						if (tpPtr->settingRepeat && !firstSettingThread)
							{
							pActiveRW->Suspend(ESuspend);	
							}
						else
							{
							pActiveRW->StartOrSuspend();						
							}
						break;
							
					case 'C' :
						test.Start (_L("Compare"));
						pActiveRW->SetTransferMode(ELoopComp);
						gAltSettingOnNotify = ETrue;
						if (tpPtr->settingRepeat && !firstSettingThread)
							{
							pActiveRW->Suspend(ESuspend);	
							}
						else
							{
							pActiveRW->StartOrSuspend();						
							}
						break;
							
					case 'S' :
						test.Start (_L("Stream"));
						if (tpPtr->outPipe > KMaxEndpointsPerClient)
							{
							pActiveRW->SetTransferMode(ETransmitOnly);						
							gAltSettingOnNotify = ETrue;
							if (tpPtr->settingRepeat && !firstSettingThread)
								{
								pActiveRW->Suspend(ESuspend);	
								}
							else
								{
								pActiveRW->StartOrSuspend();						
								}
							}
						else
							{
							pActiveRW->SetTransferMode(EReceiveOnly);						
							gAltSettingOnNotify = ETrue;
							if (tpPtr->settingRepeat && !firstSettingThread)
								{
								pActiveRW->Suspend(ESuspend);	
								}
							else
								{
								pActiveRW->StartOrSuspend();						
								}
							}					
						break;
							
					case 'F' :
						test.Start (_L("File"));
						// send this now as the file setup takes a long time
						sendStatus = EFalse;
						r = iPort[0].SendEp0StatusPacket();					
						test_KErrNone(r);
						if (tpPtr->outPipe > KMaxEndpointsPerClient)
							{
							pActiveRW->SetTransferMode(ETransmitOnly);
							TInt maxFileSize = tpPtr->maxSize * tpPtr->repeat;						
							pActiveRW->ReadFromDisk((TChar)tpPtr->minSize,maxFileSize);
							gAltSettingOnNotify = ETrue;
							if (tpPtr->settingRepeat && !firstSettingThread)
								{
								pActiveRW->Suspend(ESuspend);	
								}
							else
								{
								pActiveRW->StartOrSuspend();						
								}
							}
						else
							{
							pActiveRW->SetTransferMode(EReceiveOnly);						
							pActiveRW->WriteToDisk((TChar)tpPtr->minSize);
							gAltSettingOnNotify = ETrue;
							if (tpPtr->settingRepeat && !firstSettingThread)
								{
								pActiveRW->Suspend(ESuspend);	
								}
							else
								{
								pActiveRW->StartOrSuspend();						
								}
							}					
						break;
						
					default :
						TUSB_PRINT1("Invalid test value %X",value);
						test(EFalse);
						}
						
					gRW[index] = pActiveRW;
					break;
					
				case ETestResult :
					TUSB_VERBOSE_PRINT2 ("Test index %d complete - value %d",index,value);
					// if failure, send this first to prevent panic corrupting EP0 
					if (!value)
						{
						sendStatus = EFalse;
						r = iPort[0].SendEp0StatusPacket();					
						}
					if (index < KMaxConcurrentTests)
						{
						if (gRW[index] != NULL)
							{
							gRW[index]->TestComplete (value);
							break;
							}
						}
					if (index == KHostErrorIndex)
						{
						if (!value)
							{
							TUSB_PRINT("Host Test Fail");							
							}
						}
					else
						{
						TUSB_PRINT2("Invalid test index %d for result %d",index,value);
						}
					if (!value)
						{
						test(EFalse);
						}
					break;

				case ETestFail :
					User::Leave(-1);
					break;

				case ETestConnect :
					test.Start (_L("Connect"));
					sendStatus = EFalse;
					r = iPort[0].SendEp0StatusPacket();					
					if (iSoftwareConnect)
						{
						r = iPort[0].DeviceDisconnectFromHost();
						test_KErrNone (r);
						
						TUint32 waitTime = (TUint32)value * 1000;
						if (waitTime == 0)
							{
							waitTime = 5000;		// default to 5 milliseconds
							}
						iTimer.After(iStatus,waitTime);
						iPending = EPendingTimer;
						
						SetActive();
						}
					else
						{
						iConsole->Printf(_L("This device does not support software\n"));
						iConsole->Printf(_L("disconnect/reconnect\n"));
						iConsole->Printf(_L("Please physically unplug and replug\n"));
						iConsole->Printf(_L("the USB cable NOW... "));
						test.End ();
						}				
					break;

				case ETestDisconnect :
					test.Start (_L("Disconnect"));
					// send this now as the port will be disconnected
					sendStatus = EFalse;
					r = iPort[0].SendEp0StatusPacket();					
					if (iSoftwareConnect)
						{
						r = iPort[0].DeviceDisconnectFromHost();
						test_KErrNone (r);
						}
					else
						{
						iConsole->Printf(_L("This device does not support software\n"));
						iConsole->Printf(_L("disconnect/reconnect\n"));
						iConsole->Printf(_L("Please physically unplug and replug\n"));
						iConsole->Printf(_L("the USB cable NOW... "));
						}				

					test.End ();
					break;

				case ETestMassStorage :
					test.Start (_L("Select Mass Storage"));
				
					// send this now as the port will be disconnected
					sendStatus = EFalse;
					r = iPort[0].SendEp0StatusPacket();					
					test_KErrNone(r);
			
					for (TInt portNumber = 0; portNumber < iTotalChannels; portNumber++)
						{
						delete iDeviceStateNotifier[portNumber];
						delete iStallNotifier[portNumber];
						if (portNumber == 0)
							{
							r = iPort[portNumber].RemoveStringDescriptor(stridx1);
							if (r != KErrNone)
								{
								TUSB_PRINT1("Error %d on string removal", r);
								}
							r = iPort[portNumber].RemoveStringDescriptor(stridx2);
							if (r != KErrNone)
								{
								TUSB_PRINT1("Error %d on string removal", r);
								}	
							}
						TUSB_VERBOSE_PRINT1 ("Closing USB channel number %d",portNumber);
						iPort[portNumber].Close();											// close USB channel
						}
		
					r = iPort[0].Open(0);
					test_KErrNone(r);
					TUSB_VERBOSE_PRINT("Successfully opened USB port");

					SetupDescriptors(iLddPtr, &iPort[0],value);
					StartMassStorage(&iPort[0]);
		
					test.Next (_L("Enumeration..."));
					r = ReEnumerate();				
					test_KErrNone(r);


					test.End ();
					break;
				}
			if (sendStatus)
				{
				r = iPort[0].SendEp0StatusPacket();
				if (r != KErrNone)
					{
					TUSB_PRINT1("ActiveControl::Error %d in Ep0 Send Status Packet", r);
					test(EFalse);
					return KErrNone;						
					}				
				}
			}
		else
			{
			if ((iEp0SetUpPacket[KUsb_Ep0RequestOffset] == EVersion) && length > 0)
				{
				TUSB_PRINT4("Sending t_usb_device version: %d.%d.%d length %d \n", KDeviceVersionMajor, KDeviceVersionMinor, KDeviceVersionMicro, length);
				#ifdef	USB_SC
				TUint8 *ep0Buffer;
				TUint8 *ep0BufPtr;
				TUint ep0Length;
				iEp0Buf.GetInBufferRange(((TAny*&)ep0Buffer),ep0Length);
				
				ep0BufPtr = ep0Buffer;
				*(ep0Buffer++) = KDeviceVersionMajor;
				*(ep0Buffer++) = KDeviceVersionMinor;
				*(ep0Buffer++) = KDeviceVersionMicro;
				TUint8 i=3;
				if (iConfigFileName->Length())
					{
					for(TUint8 j=0; j < iConfigFileName->Length() && i < length; j++)
						{
						i++;
						*(ep0Buffer++) = (*iConfigFileName)[j];
						}
					}
				if (iScriptFileName->Length())
					{
					for(TUint8 j=0; j < iScriptFileName->Length() && i < length; j++)
						{
						i++;
						*(ep0Buffer++) = (*iScriptFileName)[j];
						}
					}
				*(ep0Buffer++) = 0;
				r = iEp0Buf.WriteBuffer(ep0BufPtr,length,FALSE,ep0Status);
				test_KErrNone(r);
				#else				
				iEp0DataBuffer.FillZ(length);
				iEp0DataBuffer[0] = KDeviceVersionMajor;
				iEp0DataBuffer[1] = KDeviceVersionMinor;
				iEp0DataBuffer[2] = KDeviceVersionMicro;
				iEp0DataBuffer.SetLength(3);
				iEp0DataBuffer.Append (*iConfigFileName);
				iEp0DataBuffer.Append (*iScriptFileName);
				iEp0DataBuffer.SetLength(length);				
				iPort[0].Write(ep0Status, EEndpoint0, iEp0DataBuffer, length);
				#endif
				User::WaitForRequest(ep0Status);
				test_KErrNone(ep0Status.Int());
				}
			else if ((iEp0SetUpPacket[KUsb_Ep0RequestOffset] == ETestIdleCounter) && length >= sizeof(TInt64))
				{
				// for a non zero request value if any tests still active send zero otherwise the counter value
				TInt64 val = (value == 0 || gActiveTestCount == 0) ? iIdleCounter->iCounter : 0;

				TUSB_PRINT1("Sending counter value %Ld\n", val);
				#ifdef	USB_SC

				TUint8 *ep0Buffer;
				TUint ep0Length;
				iEp0Buf.GetInBufferRange(((TAny*&)ep0Buffer),ep0Length);
				
				*((TInt64*) ep0Buffer) = val;
				
				r = iEp0Buf.WriteBuffer(ep0Buffer,length,FALSE,ep0Status);
				test_KErrNone(r);
				#else				

				iEp0DataBuffer.FillZ(length);
				*((TInt64*) iEp0DataBuffer.Ptr()) = val;
				iEp0DataBuffer.SetLength(sizeof(TInt64));
				iPort[0].Write(ep0Status, EEndpoint0, iEp0DataBuffer, length);
				#endif

				User::WaitForRequest(ep0Status);
				test_KErrNone(ep0Status.Int());
				}
			}
		if (iEp0SetUpPacket[KUsb_Ep0RequestOffset] != EStop && iEp0SetUpPacket[KUsb_Ep0RequestOffset] != ETestConnect &&
			iEp0SetUpPacket[KUsb_Ep0RequestOffset] != ETestMassStorage)
			{
			return KErrCompletion;		
			}				
		}
	else
		{
		TUSB_PRINT1("Error : Incorrect SetUp Packet Request Type %X", iEp0SetUpPacket[0]);			
		test(EFalse);
		return KErrNone;
		}
	
	return KErrNone;
	}
	
void CActiveControl::PrintHostLog()
	{
	TRequestStatus status = 0;
	wchar_t lineBuf[128];
	TUint j = 0;
	
	TUSB_VERBOSE_PRINT("Reading Host log file\n");

	#ifdef USB_SC
	TInt r = 0;
	TEndpointBuffer scReadBuf;
	TAny * scReadData;
	TUint8 * scCharPtr;
	TUint readSize;
	TBool readZlp = EFalse;

	r = iPort->OpenEndpoint(scReadBuf,firstBulkOutEndpoint);
	test_KErrNone(r);
	do
		{
		r = scReadBuf.GetBuffer (scReadData,readSize,readZlp,status);
		// The following line can be reinstated once the shared chunk failure is fixed
		// that prevents the readZlp flag from being set
		// test_Value(r, (r == KErrNone) || (r == KErrCompletion) || (r == KErrEof));
		if (r == KErrCompletion)
			{
			TUSB_VERBOSE_PRINT1("Host log file %d bytes read\n",readSize);
			scCharPtr = (TUint8 *)scReadData;
			// Print the host log file
			for (TUint i = 0; i < readSize; i++)
				{
				if (* scCharPtr == '\r')
					{
					lineBuf[j++] = '\0';
					RDebug::Print (_L("%s"),lineBuf);
					j = 0;	
					}
				else
					{
					if (* scCharPtr != '\n')
						{
						lineBuf[j++] = * scCharPtr;				
						}
					}
				scCharPtr++;
				}
			}
		if (r == KErrNone)
			{
			User::WaitForRequest(status);
			test_KErrNone(status.Int());	
			}
		}
	while (r >= KErrNone && !readZlp);
	#else
	TPtr8 readBuf((TUint8 *)User::Alloc(KHostLogFileSize),KHostLogFileSize,KHostLogFileSize);
	iPort[0].ReadUntilShort(status, (TEndpointNumber)firstBulkOutEndpoint, readBuf);
	User::WaitForRequest(status);
	test_KErrNone(status.Int());
	TUSB_VERBOSE_PRINT1("Host log file %d bytes read\n",readBuf.Length());
	for (TUint i = 0; i < readBuf.Length(); i++)
		{
		if (readBuf[i] == '\r')
			{
			lineBuf[j++] = '\0';
			RDebug::Print (_L("%s"),lineBuf);
			j = 0;	
			}
		else
			{
			if (readBuf[i] != '\n')
				{
				lineBuf[j++] = readBuf[i];				
				}
			}
		}
	User::Free ((TAny *)readBuf.Ptr());
	#endif
	}
	
void CActiveControl::QueryUsbClientL(LDDConfigPtr aLddPtr, RDEVCLIENT* aPort)
	{
	// Get device/endpoint capabilities
	//
	// A TPckg, or TPckBuf was not used in the following, because
	//
	//	 TPckgBuf<TUsbcEndpointData[KUsbcMaxEndpoints]> databuf;
	//
	// doesn't work. Also,
	//
	//	 TUsbcEndpointData data[KUsbcMaxEndpoints];
	//	 TPckgBuf<TUsbcEndpointData[KUsbcMaxEndpoints]> databuf(data);
	//
	// doesn't work. Also,
	//
	//	 TUsbcEndpointData data[KUsbcMaxEndpoints];
	//	 TPckgBuf<TUsbcEndpointData[]> databuf(data);
	//
	// doesn't work.
	// So we seem to have to stick to the ugly cast below.
	//
	//	 TUsbcEndpointData data[KUsbcMaxEndpoints];
	//	 TPtr8 databuf(reinterpret_cast<TUint8*>(data), sizeof(data), sizeof(data));
	//

	// Device
	// === Device Descriptor
	
	test.Start(_L("Query device and Endpoint Capabilities"));


	TUsbDeviceCaps d_caps;
	TInt r = aPort->DeviceCaps(d_caps);
	test_KErrNone(r);

	const TInt n = d_caps().iTotalEndpoints;

	TUSB_PRINT("###  USB device capabilities:");
	TUSB_PRINT1("Number of endpoints:                %d", n);
	TUSB_PRINT1("Supports Software-Connect:          %s",
				d_caps().iConnect ? _S("yes") : _S("no"));
	TUSB_PRINT1("Device is Self-Powered:             %s",
				d_caps().iSelfPowered ? _S("yes") : _S("no"));
	TUSB_PRINT1("Supports Remote-Wakeup:             %s",
				d_caps().iRemoteWakeup ? _S("yes") : _S("no"));
	TUSB_PRINT1("Supports High-speed:                %s",
				d_caps().iHighSpeed ? _S("yes") : _S("no"));
	TUSB_PRINT1("Supports unpowered cable detection: %s\n",
				(d_caps().iFeatureWord1 & KUsbDevCapsFeatureWord1_CableDetectWithoutPower) ?
				_S("yes") : _S("no"));
	TUSB_PRINT("");

	iSoftwareConnect = d_caps().iConnect;					// we need to remember this
	test_Equal(aLddPtr->iSoftConnect,iSoftwareConnect);

	// only check capabilities if set; therefore allowing them to be disabled
	if (aLddPtr->iSelfPower)
		{
		test(d_caps().iSelfPowered);	
		}
	
	// only check capabilities if set; therefore allowing them to be disabled
	if (aLddPtr->iRemoteWakeup)
		{
		test(d_caps().iRemoteWakeup);		
		}

	test_Equal(d_caps().iFeatureWord1 & KUsbDevCapsFeatureWord1_CableDetectWithoutPower,aLddPtr->iFeatures);

	// only check capability if set; therefore allowing it to be disabled
	if (aLddPtr->iHighSpeed)
		{
		test(d_caps().iHighSpeed);		
		}
	
	test_Equal(aLddPtr->iNumEndpoints,n);

	// Endpoints
	TUsbcEndpointData data[KUsbcMaxEndpoints];
	TPtr8 dataptr(reinterpret_cast<TUint8*>(data), sizeof(data), sizeof(data));
	r = aPort->EndpointCaps(dataptr);
	test_KErrNone(r);

	TUSB_PRINT("### USB device endpoint capabilities:");
	for (TInt i = 0; i < n; i++)
		{
		const TUsbcEndpointCaps* caps = &data[i].iCaps;
		
		
		TBuf<40> sizeStr(_S("unknown"));
		if (caps->iSizes == KUsbEpNotAvailable)
			{
			sizeStr = _S("Not Available");	
			}		
		else
			{
			sizeStr.SetLength(0);
			if (caps->iSizes & KUsbEpSizeCont)
				sizeStr.Append(_S(" Continuous"),11);
			if (caps->iSizes & KUsbEpSize8)
				sizeStr.Append(_S(" 8"),2);
			if (caps->iSizes & KUsbEpSize16)
				sizeStr.Append(_S(" 16"),3);
			if (caps->iSizes & KUsbEpSize32)
				sizeStr.Append(_S(" 32"),3);
			if (caps->iSizes & KUsbEpSize64)
				sizeStr.Append(_S(" 64"),3);
			if (caps->iSizes & KUsbEpSize128)
				sizeStr.Append(_S(" 128"),4);
			if (caps->iSizes & KUsbEpSize256)
				sizeStr.Append(_S(" 256"),4);
			if (caps->iSizes & KUsbEpSize512)
				sizeStr.Append(_S(" 512"),4);
			if (caps->iSizes & KUsbEpSize1023)
				sizeStr.Append(_S(" 1023"),5);
			if (caps->iSizes & KUsbEpSize1024)
				sizeStr.Append(_S(" 1024"),5);
			}

		TBuf<40> typeStr(_S("unknown"));
		if (caps->iTypesAndDir == KUsbEpNotAvailable)
			typeStr = _S("Not Available");
		if (caps->iTypesAndDir & (KUsbEpTypeControl | KUsbEpTypeBulk | KUsbEpTypeInterrupt | KUsbEpTypeIsochronous))
			{
			typeStr.SetLength(0);
			if (caps->iTypesAndDir & KUsbEpTypeBulk)
				typeStr.Append(_S("Control "),8);
			if (caps->iTypesAndDir & KUsbEpTypeBulk)
				typeStr.Append(_S("Bulk "),5);
			if (caps->iTypesAndDir & KUsbEpTypeInterrupt)
				typeStr.Append(_S("Interrupt "),10);
			if (caps->iTypesAndDir & KUsbEpTypeIsochronous)
				typeStr.Append(_S("Isochronous"),11);			
			}
			
		TBuf<20> directionStr(_S("unknown"));
		
		if (caps->iTypesAndDir & KUsbEpDirIn)
			directionStr = _S("In");
		if (caps->iTypesAndDir & KUsbEpDirOut)
			directionStr = _S("Out");
		if (caps->iTypesAndDir & KUsbEpDirBidirect)
			directionStr = _S("Both");
				
		TUSB_PRINT4("Endpoint:%d Sizes =%s Type = %s - %s",
					i+1,sizeStr.PtrZ(), typeStr.PtrZ(), directionStr.PtrZ());
		}
	TUSB_PRINT("");

	test.End();
			
	}


void CActiveControl::AllocateEndpointDMA(RDEVCLIENT* aPort,TENDPOINTNUMBER aEndpoint)
	{
	TBool res = EFalse;
	
	TInt r = aPort->AllocateEndpointResource(aEndpoint, EUsbcEndpointResourceDMA);
	if (r == KErrNone)
		RDebug::Print(_L("DMA allocation on endpoint %d: KErrNone"), aEndpoint);
	else if (r == KErrInUse)
		RDebug::Print(_L("DMA allocation on endpoint %d: KErrInUse"), aEndpoint);
	else if (r == KErrNotSupported)
		RDebug::Print(_L("DMA allocation on endpoint %d: KErrNotSupported"), aEndpoint);
	else
		RDebug::Print(_L("DMA allocation on endpoint %d: unexpected return value %d"),
					  aEndpoint, r);
	#ifdef	USB_SC
	res = aPort->QueryEndpointResourceUse(aEndpoint, EUsbcEndpointResourceDMA);
	#else
	res = aPort->QueryEndpointResourceUse(aEndpoint, EUsbcEndpointResourceDMA);
	#endif
	
	TUSB_PRINT2("DMA on endpoint %d %s\n",
				aEndpoint, res ? _S("allocated") : _S("not allocated"));

	if ((r == KErrNone) && !res)
		RDebug::Print(_L("(Allocation success but negative query result: contradiction!)\n"));
	else if ((r != KErrNone) && res)
		RDebug::Print(_L("(Allocation failure but positive query result: contradiction!)\n"));
	}


void CActiveControl::DeAllocateEndpointDMA(RDEVCLIENT* aPort,TENDPOINTNUMBER aEndpoint)
	{
	TBool res = FALSE;	
	TInt r = aPort->DeAllocateEndpointResource(aEndpoint, EUsbcEndpointResourceDMA);
	if (r == KErrNone)
		RDebug::Print(_L("DMA deallocation on endpoint %d: KErrNone"), aEndpoint);
	else if (r == KErrNotSupported)
		RDebug::Print(_L("DMA deallocation on endpoint %d: KErrNotSupported"), aEndpoint);
	else
		RDebug::Print(_L("DMA deallocation on endpoint %d: unexpected return value %d"),
					  aEndpoint, r);
	#ifdef	USB_SC
	res = aPort->QueryEndpointResourceUse(aEndpoint, EUsbcEndpointResourceDMA);
	#else
	res = aPort->QueryEndpointResourceUse(aEndpoint, EUsbcEndpointResourceDMA);
	#endif
	
	TUSB_PRINT2("DMA on endpoint %d %s\n",
				aEndpoint, res ? _S("allocated") : _S("not allocated"));
	}

#ifndef USB_SC
void CActiveControl::AllocateDoubleBuffering(RDEVCLIENT* aPort,TENDPOINTNUMBER aEndpoint)
	{
	TBool res = FALSE;
	TInt r = aPort->AllocateEndpointResource(aEndpoint, EUsbcEndpointResourceDoubleBuffering);
	if (r == KErrNone)
		RDebug::Print(_L("Double Buffering allocation on endpoint %d: KErrNone"), aEndpoint);
	else if (r == KErrInUse)
		RDebug::Print(_L("Double Buffering allocation on endpoint %d: KErrInUse"), aEndpoint);
	else if (r == KErrNotSupported)
		RDebug::Print(_L("Double Buffering allocation on endpoint %d: KErrNotSupported"), aEndpoint);
	else
		RDebug::Print(_L("Double Buffering allocation on endpoint %d: unexpected return value %d"),
					  aEndpoint, r);
	res = aPort->QueryEndpointResourceUse(aEndpoint, EUsbcEndpointResourceDoubleBuffering);
	TUSB_PRINT2("Double Buffering on endpoint %d %s\n",
				aEndpoint, res ? _S("allocated") : _S("not allocated"));

	if ((r == KErrNone) && !res)
		RDebug::Print(_L("(Allocation success but negative query result: contradiction!)\n"));
	else if ((r != KErrNone) && res)
		RDebug::Print(_L("(Allocation failure but positive query result: contradiction!)\n"));
	}


void CActiveControl::DeAllocateDoubleBuffering(RDEVCLIENT* aPort,TENDPOINTNUMBER aEndpoint)
	{
	TInt r = aPort->DeAllocateEndpointResource(aEndpoint, EUsbcEndpointResourceDoubleBuffering);
	if (r == KErrNone)
		RDebug::Print(_L("Double Buffering deallocation on endpoint %d: KErrNone"), aEndpoint);
	else if (r == KErrNotSupported)
		RDebug::Print(_L("Double Buffering deallocation on endpoint %d: KErrNotSupported"), aEndpoint);
	else
		RDebug::Print(_L("Double Buffering deallocation on endpoint %d: unexpected return value %d"),
					  aEndpoint, r);
	TBool res = aPort->QueryEndpointResourceUse(aEndpoint, EUsbcEndpointResourceDoubleBuffering);
	TUSB_PRINT2("Double Buffering on endpoint %d %s\n",
				aEndpoint, res ? _S("allocated") : _S("not allocated"));
	}

#endif

TInt CActiveControl::ReEnumerate()
	{
	TRequestStatus enum_status;
	iPort[0].ReEnumerate(enum_status);
	if (!iSoftwareConnect)
		{
		iConsole->Printf(_L("This device does not support software\n"));
		iConsole->Printf(_L("disconnect/reconnect\n"));
		iConsole->Printf(_L("Please physically unplug and replug\n"));
		iConsole->Printf(_L("the USB cable NOW... "));
		}
	iConsole->Printf(_L("\n>>> Start the t_usb_win program on the host <<<\n"));
	User::WaitForRequest(enum_status);
	if (enum_status != KErrNone)
		{
		TUSB_PRINT1("Error: Re-enumeration status = %d", enum_status.Int());
		return KErrGeneral;
		}
	TUsbcDeviceState device_state =	EUsbcDeviceStateUndefined;
	TInt r = iPort[0].DeviceStatus(device_state);
	if (r != KErrNone)
		{
		TUSB_PRINT1("Error %d on querying device state", r);
		}
	else
		{
		TUSB_PRINT1("Current device state: %s",
					(device_state == EUsbcDeviceStateUndefined) ? _S("Undefined") :
					((device_state == EUsbcDeviceStateAttached) ? _S("Attached") :
					 ((device_state == EUsbcDeviceStatePowered) ? _S("Powered") :
					  ((device_state == EUsbcDeviceStateDefault) ? _S("Default") :
					   ((device_state == EUsbcDeviceStateAddress) ? _S("Address") :
						((device_state == EUsbcDeviceStateConfigured) ? _S("Configured") :
						 ((device_state == EUsbcDeviceStateSuspended) ? _S("Suspended") :
						  _S("Unknown"))))))));
		}

	// Check the speed of the established physical USB connection
	iHighSpeed = iPort[0].CurrentlyUsingHighSpeed();
	if (iHighSpeed)
		{
		TUSB_PRINT("---> USB High-speed Testing\n");
		}
	else
		{
		TUSB_PRINT("---> USB Full-speed Testing\n");
		}

	return KErrNone;
	}

// -eof-
