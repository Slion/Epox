/*
* Copyright (c) 2004-2009 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
* This component and the accompanying materials are made available
* under the terms of the License "Eclipse Public License v1.0"
* which accompanies this distribution, and is available
* at the URL "http://www.eclipse.org/legal/epl-v10.html".
*
* Initial Contributors:
* Nokia Corporation - initial contribution.
*
* Contributors:
*
* Description:
*
*/


/**
 @file
 @internalTechnology
*/

#include "cbulkonlytransport.h"
#include "cbulkonlytransportusbcldd.h"
#include "usbmsshared.h"
#include "massstoragedebug.h"
#include "cusbmassstorageserver.h"

#define InEndpoint EEndpoint1
#define OutEndpoint EEndpoint2

//This value defined in USB Mass Storage Bulk Only Transrt spec and not supposed to be changed
LOCAL_D const TInt KRequiredNumberOfEndpoints = 2; // in addition to endpoint 0.

LOCAL_D const TInt KUsbNumInterfacesOffset = 4;

////////////////////////////////////
/**
Called by CBulkOnlyTransportUsbcLdd to create an instance of CControlInterfaceUsbcLdd

@param aParent reference to the CBulkOnlyTransportUsbcLdd
*/


CControlInterfaceUsbcLdd* CControlInterfaceUsbcLdd::NewL(CBulkOnlyTransportUsbcLdd& aParent)
	{
	CControlInterfaceUsbcLdd* self = new(ELeave) CControlInterfaceUsbcLdd(aParent);
	CleanupStack::PushL(self);
	self->ConstructL();
	CActiveScheduler::Add(self);
	CleanupStack::Pop();
	return self;
	}


void CControlInterfaceUsbcLdd::ConstructL()
	{
	}


/**
c'tor

@param aParent reference to the CBulkOnlyTransportUsbcLdd
*/
CControlInterfaceUsbcLdd::CControlInterfaceUsbcLdd(CBulkOnlyTransportUsbcLdd& aParent)
	:CActive(EPriorityStandard),
	 iParent(aParent),
	 iCurrentState(ENone)
	{
	}


/**
d'tor
*/
CControlInterfaceUsbcLdd::~CControlInterfaceUsbcLdd()
	{
	__FNLOG("CControlInterfaceUsbcLdd::~CControlInterfaceUsbcLdd ");
	Cancel();
	}


/**
Called by CBulkOnlyTransport HwStart to start control interface
*/
TInt CControlInterfaceUsbcLdd::Start()
	{
	__FNLOG("CControlInterfaceUsbcLdd::Start ");
	TInt res = ReadEp0Data();
	return (res);
	}


/**
Called by desctructor of CBulkOnlyTransportUsbcLdd to stop control interface
*/
void CControlInterfaceUsbcLdd::Stop()
	{
	__FNLOG("CControlInterfaceUsbcLdd::Stop ");
	if (!IsActive())
		{
		__PRINT(_L("Not active\n"));
		return;
		}

	__PRINT(_L("\nStopping...\n"));


	iCurrentState = ENone;

	Cancel();
	}


/**
Cancel outstanding request (if any)
*/
void CControlInterfaceUsbcLdd::DoCancel()
	{
	__FNLOG("CControlInterfaceUsbcLdd::DoCancel ");
	switch(iCurrentState)
		{
		case EReadEp0Data:
			iParent.Ldd().ReadCancel(EEndpoint0);
			break;
		case ESendMaxLun:
			iParent.Ldd().WriteCancel(EEndpoint0);
			break;
		default:
			__PRINT(_L("\nWrong state !\n"));
			__ASSERT_DEBUG(EFalse, User::Panic(KUsbMsSvrPncCat, EMsControlInterfaceBadState));
		}
	}


/**
Implement CControlInterfaceUsbcLdd state machine
*/
void CControlInterfaceUsbcLdd::RunL()
	{
	__FNLOG("CControlInterfaceUsbcLdd::RunL ");
	if (iStatus != KErrNone)
		{
		__PRINT1(_L("Error %d in RunL\n"), iStatus.Int());

		//read EP0  again
		ReadEp0Data();
		return;
		}

	switch (iCurrentState)
		{
		case ESendMaxLun:
			ReadEp0Data();
			break;

		case EReadEp0Data:
			DecodeEp0Data();
			break;

		default:
			__PRINT(_L("  error: (Shouldn't end up here...)\n"));
			__ASSERT_DEBUG(EFalse, User::Panic(KUsbMsSvrPncCat, EMsControlInterfaceBadState));
			break;
		}
	return;
	}


/**
Post a read request to EEndpoint0 to read request header
*/
TInt CControlInterfaceUsbcLdd::ReadEp0Data()
	{
	__FNLOG("CControlInterfaceUsbcLdd::ReadEp0Data ");
	if (IsActive())
		{
		__PRINT(_L("Still active\n"));
		return KErrServerBusy;
		}
	iParent.Ldd().Read(iStatus, EEndpoint0, iData, KRequestHdrSize);

	iCurrentState = EReadEp0Data;

	SetActive();
	return KErrNone;
	}


/**
Decode request header and do appropriate action - get max LUN info or post a reset request
*/
void CControlInterfaceUsbcLdd::DecodeEp0Data()
	{
	__FNLOG("CControlInterfaceUsbcLdd::DecodeEp0Data ");
	if (IsActive())
		{
		__PRINT(_L("Still active\n"));
		__ASSERT_DEBUG(EFalse, User::Panic(KUsbMsSvrPncCat, EMsControlInterfaceStillActive));
		return;
		}

	TInt err = iRequestHeader.Decode(iData);

	if(err != KErrNone)
		return;

    switch(iRequestHeader.iRequest)
		{
		//
		// GET MAX LUN (0xFE)
		//
		case TUsbRequestHdr::EReqGetMaxLun:
			{
			__PRINT1(_L("DecodeEp0Data : 'Get Max LUN' Request MaxLun = %d"),iParent.MaxLun() );

            if (   iRequestHeader.iRequestType != 0xA1 //value from USB MS BOT spec
                || iRequestHeader.iIndex > 15
                || iRequestHeader.iValue != 0
                || iRequestHeader.iLength != 1)
                {
    		    __PRINT(_L("GetMaxLun command packet check error"));
                iParent.Ldd().EndpointZeroRequestError();
                break;
                }
			iData.FillZ(1);  //Return only 1 byte to host
			iData[0] = static_cast<TUint8>(iParent.MaxLun());	// Supported Units
			iParent.Ldd().Write(iStatus, EEndpoint0, iData, 1);

			iCurrentState = ESendMaxLun;
			SetActive();

			return;
			}
		//
		// RESET (0xFF)
		//
		case TUsbRequestHdr::EReqReset:
			{
			__PRINT(_L("DecodeEp0Data : 'Mass Storage Reset' Request"));

            if (   iRequestHeader.iRequestType != 0x21 //value from USB MS BOT spec
                || iRequestHeader.iIndex > 15
                || iRequestHeader.iValue != 0
                || iRequestHeader.iLength != 0)
                {
			    __PRINT(_L("MSC Reset command packet check error"));
                iParent.Ldd().EndpointZeroRequestError();
                break;
                }
			iParent.HwStop();
			iParent.Controller().Reset();
			iParent.HwStart(ETrue);

            err = iParent.Ldd().SendEp0StatusPacket();

			return;
			}
		//
		// Unknown?
		//
		default:
			{
			__PRINT(_L("DecodeEp0Data : Unknown Request"));

			}
		}
		ReadEp0Data();  //try to get another request
	}


//
// --- class CBulkOnlyTransportUsbcLdd ---------------------------------------------------------
//

CBulkOnlyTransportUsbcLdd::CBulkOnlyTransportUsbcLdd(TInt aNumDrives,CUsbMassStorageController& aController)
	:CBulkOnlyTransport(aNumDrives, aController),
	 iSwap(ETrue)
	{
	__FNLOG("CBulkOnlyTransportUsbcLdd::CBulkOnlyTransportUsbcLdd");
	}

/**
Constructs the CBulkOnlyTransportUsbcLdd object
*/
void CBulkOnlyTransportUsbcLdd::ConstructL()
	{
	__FNLOG("CBulkOnlyTransportUsbcLdd::ConstructL()");
	iControlInterface = CControlInterfaceUsbcLdd::NewL(*this);
	iDeviceStateNotifier = CActiveDeviceStateNotifierBase::NewL(*this, *this);
	CActiveScheduler::Add(this);
	}

CBulkOnlyTransportUsbcLdd::~CBulkOnlyTransportUsbcLdd()
	{
	__FNLOG("CBulkOnlyTransportUsbcLdd::~CBulkOnlyTransportUsbcLdd");
	if (iInterfaceConfigured)
		{
		delete iControlInterface ;
		delete iDeviceStateNotifier;
		}
	}	

RDevUsbcClient& CBulkOnlyTransportUsbcLdd::Ldd()
	{
	return iLdd;
	}


/**
Set or unset configuration descriptor for USB MassStorage Bulk Only transport

@param aUnset indicate whether set or unset descriptor
@return KErrNone if operation was completed successfully, errorcode otherwise
*/
TInt CBulkOnlyTransportUsbcLdd::SetupConfigurationDescriptor(TBool aUnset)
	{
	__FNLOG("CBulkOnlyTransportUsbcLdd::SetupConfigurationDescriptor");
	TInt ret(KErrNone);
	
		if ((ret = iLdd.Open(0)) != KErrNone)
			return ret;

	TInt configDescriptorSize(0);
	iLdd.GetConfigurationDescriptorSize(configDescriptorSize);
	if (static_cast<TUint>(configDescriptorSize) != KUsbDescSize_Config)
		{
		return KErrCorrupt;
		}

	TBuf8<KUsbDescSize_Config> configDescriptor;
	ret = iLdd.GetConfigurationDescriptor(configDescriptor);
	if (ret != KErrNone)
		{
		return ret;
		}

	// I beleive that other fields setted up during LDD initialisation
	if (aUnset)
		{
		--configDescriptor[KUsbNumInterfacesOffset];
		}
	else
		{
		++configDescriptor[KUsbNumInterfacesOffset];
		}
	ret = iLdd.SetConfigurationDescriptor(configDescriptor);

	if (aUnset)
		{
		iLdd.Close();
		}
	return ret;
	}

/**
Set up interface descriptor

@return KErrNone if operation was completed successfully, errorcode otherwise
*/
TInt CBulkOnlyTransportUsbcLdd::SetupInterfaceDescriptors()
	{
	__FNLOG("CBulkOnlyTransportUsbcLdd::SetupInterfaceDescriptors");
	// Device caps
	TUsbDeviceCaps d_caps;
	TInt ret = iLdd.DeviceCaps(d_caps);
	if (ret != KErrNone)
		{
		return ret;
		}
	TInt totalEndpoints = d_caps().iTotalEndpoints;
	if (totalEndpoints  < KRequiredNumberOfEndpoints)
		{
		return KErrHardwareNotAvailable;
		}

	// Endpoint caps
	TUsbcEndpointData data[KUsbcMaxEndpoints];
	TPtr8 dataptr(reinterpret_cast<TUint8*>(data), sizeof(data), sizeof(data));
	ret = iLdd.EndpointCaps(dataptr);
	if (ret != KErrNone)
		{
		return ret;
		}

	// Set the active interface
	TUsbcInterfaceInfoBuf ifc;
	TInt ep_found = 0;
	TBool foundBulkIN = EFalse;
	TBool foundBulkOUT = EFalse;

	for (TInt i = 0; i < totalEndpoints ; i++)
		{
		const TUsbcEndpointCaps* caps = &data[i].iCaps;
		const TInt maxPacketSize = caps->MaxPacketSize();
		if (!foundBulkIN &&
			(caps->iTypesAndDir & (KUsbEpTypeBulk | KUsbEpDirIn)) == (KUsbEpTypeBulk | KUsbEpDirIn))
			{
			// InEndpoint is going to be our TX (IN, write) endpoint
			ifc().iEndpointData[0].iType = KUsbEpTypeBulk;
			if((d_caps().iFeatureWord1 & KUsbDevCapsFeatureWord1_EndpointResourceAllocV2) == KUsbDevCapsFeatureWord1_EndpointResourceAllocV2)
				ifc().iEndpointData[0].iFeatureWord1  = KUsbcEndpointInfoFeatureWord1_DMA|KUsbcEndpointInfoFeatureWord1_DoubleBuffering;
			ifc().iEndpointData[0].iDir  = KUsbEpDirIn;
			ifc().iEndpointData[0].iSize = maxPacketSize;
			ifc().iEndpointData[0].iInterval_Hs = 0;
			foundBulkIN = ETrue;
			if (++ep_found == KRequiredNumberOfEndpoints)
				{
				break;
				}
			continue;
			}
		if (!foundBulkOUT &&
			(caps->iTypesAndDir & (KUsbEpTypeBulk | KUsbEpDirOut)) == (KUsbEpTypeBulk | KUsbEpDirOut))
			{
			// OutEndpoint is going to be our RX (OUT, read) endpoint
			ifc().iEndpointData[1].iType = KUsbEpTypeBulk;
			if((d_caps().iFeatureWord1 & KUsbDevCapsFeatureWord1_EndpointResourceAllocV2) == KUsbDevCapsFeatureWord1_EndpointResourceAllocV2)
				ifc().iEndpointData[1].iFeatureWord1  = KUsbcEndpointInfoFeatureWord1_DMA|KUsbcEndpointInfoFeatureWord1_DoubleBuffering;
			ifc().iEndpointData[1].iDir  = KUsbEpDirOut;
			ifc().iEndpointData[1].iSize = maxPacketSize;
			ifc().iEndpointData[1].iInterval_Hs = 0;
			foundBulkOUT = ETrue;
			if (++ep_found == KRequiredNumberOfEndpoints)
				{
				break;
				}
			continue;
			}
		}
	if (ep_found != KRequiredNumberOfEndpoints)
		{
		return KErrHardwareNotAvailable;
		}

	_LIT16(string, "USB Mass Storage Interface");
	ifc().iString = const_cast<TDesC16*>(&string);
	ifc().iTotalEndpointsUsed = KRequiredNumberOfEndpoints;
	ifc().iClass.iClassNum    = 0x08;	// Mass Storage
	ifc().iClass.iSubClassNum = 0x06;	// SCSI Transparent Command Set
	ifc().iClass.iProtocolNum = 0x50;	// Bulk Only Transport

	TUint bandwidth_priority = (EUsbcBandwidthOUTDefault | EUsbcBandwidthINDefault);
	if (d_caps().iHighSpeed)
		{
		// If this device supports USB High-speed, then we request 64KB buffers
		// (otherwise the default 4KB ones will do).
		bandwidth_priority = (EUsbcBandwidthOUTPlus2 | EUsbcBandwidthINPlus2);
		// Also, tell the Protocol about it, because it might want to do some
		// optimizing too.
		iProtocol->ReportHighSpeedDevice();
		}
	ret = iLdd.SetInterface(0, ifc, bandwidth_priority);
	return ret;
	}

void CBulkOnlyTransportUsbcLdd::ReleaseInterface()
	{
	iLdd.ReleaseInterface(0);
	}

TInt CBulkOnlyTransportUsbcLdd::StartControlInterface()
	{
	return iControlInterface->Start();
	}

void CBulkOnlyTransportUsbcLdd::CancelControlInterface()
	{
	iControlInterface->Cancel();
	}

void CBulkOnlyTransportUsbcLdd::ActivateDeviceStateNotifier()
	{
	iDeviceStateNotifier->Activate();
	}

void CBulkOnlyTransportUsbcLdd::CancelDeviceStateNotifier()
	{
	iDeviceStateNotifier->Cancel();
	}

void CBulkOnlyTransportUsbcLdd::CancelReadWriteRequests()
	{
	__FNLOG("CBulkOnlyTransportUsbcLdd::CancelReadWriteRequests");
	iLdd.WriteCancel(InEndpoint);
	iLdd.ReadCancel(OutEndpoint);
	}

void CBulkOnlyTransportUsbcLdd::AllocateEndpointResources()
	{
	TUsbDeviceCaps d_caps;
	TInt ret = iLdd.DeviceCaps(d_caps);
	if (ret == KErrNone)
		{
		if((d_caps().iFeatureWord1 & KUsbDevCapsFeatureWord1_EndpointResourceAllocV2) != KUsbDevCapsFeatureWord1_EndpointResourceAllocV2)
			{
				// Set up DMA if possible (errors are non-critical)
			TInt err = iLdd.AllocateEndpointResource(OutEndpoint, EUsbcEndpointResourceDMA);
			if (err != KErrNone)
				{
				__PRINT1(_L("Set DMA on OUT endpoint failed with error code: %d"), err);
				}
			err = iLdd.AllocateEndpointResource(InEndpoint, EUsbcEndpointResourceDMA);
			if (err != KErrNone)
				{
				__PRINT1(_L("Set DMA on IN endpoint failed with error code: %d"), err);
				}

				// Set up Double Buffering if possible (errors are non-critical)
			err = iLdd.AllocateEndpointResource(OutEndpoint, EUsbcEndpointResourceDoubleBuffering);
			if (err != KErrNone)
				{
				__PRINT1(_L("Set Double Buffering on OUT endpoint failed with error code: %d"), err);
				}
			err = iLdd.AllocateEndpointResource(InEndpoint, EUsbcEndpointResourceDoubleBuffering);
			if (err != KErrNone)
				{
				__PRINT1(_L("Set Double Buffering on IN endpoint failed with error code: %d"), err);
				}
			}
		}
	}

TInt CBulkOnlyTransportUsbcLdd::GetDeviceStatus(TUsbcDeviceState& deviceStatus)
	{
	return iLdd.DeviceStatus(deviceStatus);
	}

void CBulkOnlyTransportUsbcLdd::FlushData()
	{
	TInt bytes;
	const TInt err = iLdd.QueryReceiveBuffer(OutEndpoint, bytes);
	if (err != KErrNone || bytes <= 0)
		{
		__PRINT1(_L("Error: err=%d bytes=%d"), bytes);
		}
	else
		{
		__PRINT1(_L("RxBuffer has %d bytes"), bytes);
		ReadAndDiscardData(bytes);
		}
	}
/**
 * Read out rest data from OutEndpoint and discard them
 */
void CBulkOnlyTransportUsbcLdd::ReadAndDiscardData(TInt aBytes)
	{
	__FNLOG("CBulkOnlyTransportUsbcLdd::ReadAndDiscardData");
	iDiscardBuf.SetMax();
	const TUint bufsize = static_cast<TUint>(iDiscardBuf.Length());
	TRequestStatus status;
	while (aBytes > 0)
		{
		__PRINT1(_L("Bytes still to be read: %d\n"), aBytes);
		iLdd.ReadOneOrMore(status, OutEndpoint, iDiscardBuf, bufsize);
		User::WaitForRequest(status);
		TInt err = status.Int();
		if (err != KErrNone)
			{
			// Bad.
			break;
			}
		aBytes -= iDiscardBuf.Length();
		}
	}

/**
Called by the protocol to determine how many bytes of data are available in the read buffer.

@return The number of bytes available in the read buffer
*/
TInt CBulkOnlyTransportUsbcLdd::BytesAvailable()
	{
	TInt bytes = 0;
	TInt err = iLdd.QueryReceiveBuffer(OutEndpoint, bytes);
	if (err != KErrNone)
		bytes = 0;
	return bytes;
	}


void CBulkOnlyTransportUsbcLdd::StallEndpointAndWaitForClear()
	{
	__FNLOG("CBulkOnlyTransportUsbcLdd::StallEndpointAndWaitForClear");

	// Now stall this endpoint
	__PRINT1(_L("Stalling endpoint %d"), InEndpoint);
	TInt r = iLdd.HaltEndpoint(InEndpoint);
	if (r != KErrNone)
		{
		__PRINT2(_L("Error: stalling ep %d failed: %d"), InEndpoint, r);
		}
	TEndpointState ep_state;
	TInt i = 0;
	do
		{
		// Wait for 10ms before checking the ep status
		User::After(10000);
		iLdd.EndpointStatus(InEndpoint, ep_state);
		if (++i >= 550)
			{
			// 5.5 secs should be enough (see 9.2.6.1 Request Processing Timing)
			__PRINT1(_L("Error: Checked for ep %d de-stall for 5.5s - giving up now"), InEndpoint);
			// We can now only hope for a Reset Recovery
			return;
			}
		} while ((ep_state == EEndpointStateStalled) && iStarted);
	__PRINT2(_L("Checked for ep %d de-stall: %d time(s)"), InEndpoint, i);
	}


/**
Read CBW data (KCbwLength) from the host into the read buffer.
*/
void CBulkOnlyTransportUsbcLdd::ReadCBW()
	{
	__FNLOG("CBulkOnlyTransportUsbcLdd::ReadCBW");
	if (IsActive())
		{
		__PRINT(_L("Still active\n"));
		__ASSERT_DEBUG(EFalse, User::Panic(KUsbMsSvrPncCat, EMsBulkOnlyStillActive));
		return;
		}

	iCbwBuf.SetMax();
	iLdd.ReadUntilShort(iStatus, OutEndpoint, iCbwBuf, KCbwLength);

	iCurrentState = EWaitForCBW;
	SetActive();
	}

void CBulkOnlyTransportUsbcLdd::ExpireData(TAny* /*aAddress*/)
	{
	// Intentionally left blank
	}

void CBulkOnlyTransportUsbcLdd::ProcessCbwEvent()
	{
	DecodeCBW();
	}


/**
Request data form the host for the protocol

@param aLength amount of data (in bytes) to be received from the host
*/
void CBulkOnlyTransportUsbcLdd::ReadData(TUint aLength)
	{
	__FNLOG("CBulkOnlyTransportUsbcLdd::ReadData");
	if (IsActive())
		{
		__PRINT(_L("Still active\n"));
		__ASSERT_DEBUG(EFalse, User::Panic(KUsbMsSvrPncCat, EMsBulkOnlyStillActive));
		return;
		}
	
	SetReadDataBufPtr(aLength);
	iLdd.Read(iStatus, OutEndpoint, iReadBufPtr, aLength);

	iCurrentState = EReadingData;
	SetActive();
	}

void CBulkOnlyTransportUsbcLdd::WriteUsb(TRequestStatus& aStatus, TPtrC8& aDes, TUint aLength, TBool aZlpRequired)
	{
	iLdd.Write(aStatus, InEndpoint, aDes, aLength, aZlpRequired);
	}

void CBulkOnlyTransportUsbcLdd::SetCbwPtr()
	{
	iCbwBufPtr.Set(iCbwBuf.Ptr(), iCbwBuf.Length());
	}

TPtr8& CBulkOnlyTransportUsbcLdd::SetCommandBufPtr(TUint aLength)
	{
	iCommandBufPtr.Set((TUint8*) iCommandBuf.Ptr(), aLength, aLength );
	return iCommandBufPtr;
	}

void CBulkOnlyTransportUsbcLdd::SetReadDataBufPtr(TUint aLength) //Write10(Host->Device
	{
	if (iSwap)
		{
		iDataBuf1.SetLength(aLength);
		iReadBufPtr.Set(iDataBuf1.LeftTPtr(iDataBuf1.Length()));
		iSwap = EFalse;
		}
	else
		{
		iDataBuf2.SetLength(aLength);
		iReadBufPtr.Set(iDataBuf2.LeftTPtr(iDataBuf2.Length()));
		iSwap = ETrue;
		}
	}

TPtr8& CBulkOnlyTransportUsbcLdd::SetDataBufPtr() //Read10(Device->Host)
	{
	if (iSwap)
		{
		iDataBufPtr.Set((TUint8*) iDataBuf1.Ptr(), KMaxBufSize, KMaxBufSize);
		iSwap = EFalse;
		}
	else
		{
		iDataBufPtr.Set((TUint8*) iDataBuf2.Ptr(), KMaxBufSize, KMaxBufSize);
		iSwap = ETrue;
		}
	return iDataBufPtr;
	}

void CBulkOnlyTransportUsbcLdd::SetPaddingBufPtr(TUint aLength)
	{
	iPaddingBufPtr.Set((TUint8*) iBuf.Ptr(), aLength, aLength );
	}


void CBulkOnlyTransportUsbcLdd::SetCswBufPtr(TUint aLength)
	{
	iCswBufPtr.Set((TUint8*) iCswBuf.Ptr(), aLength, aLength );
	}

void CBulkOnlyTransportUsbcLdd::ProcessReadingDataEvent()
	{
	TInt ret = KErrNone;
    FOREVER
		{
		if (iReadSetUp)
			{
			ret = iProtocol->ReadComplete(KErrNone);
			}

		TUint deviceDataLength = iBufSize; // This is the amount (maximum in case of SC Ldd) to be read next.

		if(ret == KErrCompletion)
			{
			// The protocol has indicated with KErrCompletion that sufficient
			// data is available in the buffer to process the transfer immediately.

			iDataResidue -= iReadBufPtr.Length();
			SetReadDataBufPtr(deviceDataLength);

			iLdd.Read(iStatus, OutEndpoint, iReadBufPtr, deviceDataLength);
			User::WaitForRequest(iStatus);
			if (iStatus != KErrNone)
				{
				// An error occurred - halt endpoints for reset recovery
				__PRINT1(_L("Error %d in EReadingData, halt endpoints \n"), iStatus.Int());
				SetPermError();
				return;
				}
			}
		else if(ret == KErrNotReady)
			{
			// The protocol has indicated with KErrNotReady that insufficient
			// data is available in the buffer, so should wait for it to arrive

            iDataResidue -= iReadBufPtr.Length();
			ReadData(deviceDataLength);
			break;
			}
		else
			{
			// The protocol has indicated that transfer is
			// complete, so send the CSW response to the host.
			iDataResidue -= iReadBufPtr.Length();
			iReadSetUp = EFalse;

			if (ret != KErrNone)
				{
				iCmdStatus = ECommandFailed;
				}

			if (iDataResidue)
				{
				__PRINT(_L("Discarding residue"));
				// we have to read as much data as available that host PC sends;
				// otherwise, bulk-out endpoint will need to keep sending NAK back.
				ReadAndDiscardData(iDataResidue);
				}
			SendCSW(iCbwTag, iDataResidue, iCmdStatus);
			break;
			}
		}

	}

void CBulkOnlyTransportUsbcLdd::DiscardData(TUint aLength)
	{
	iBuf.SetLength(KBOTMaxBufSize);
	TUint c = 0;
	TRequestStatus status;
	while (c < aLength)
		{
		TInt len;
		if (aLength - c >  KBOTMaxBufSize)
			{
			len = KBOTMaxBufSize;
			}
		else
			{
			len = aLength - c;
			}

		iLdd.Read(status, OutEndpoint, iBuf, len);
		User::WaitForRequest(status);
		c +=  KBOTMaxBufSize;
		}
	}

void CBulkOnlyTransportUsbcLdd::WriteToClient(TUint aLength)
	{
	SetDataBufPtr();
	iLdd.Read(iStatus, OutEndpoint, iDataBufPtr, aLength);
    User::WaitForRequest(iStatus);
    iProtocol->ReadComplete(KErrGeneral);
	}

#ifdef MSDC_MULTITHREADED
void CBulkOnlyTransportUsbcLdd::GetBufferPointers(TPtr8& aDes1, TPtr8& aDes2)
	{
	aDes1.Set((TUint8*) iDataBuf1.Ptr(), KMaxBufSize, KMaxBufSize);
	aDes2.Set((TUint8*) iDataBuf2.Ptr(), KMaxBufSize, KMaxBufSize);
	}
#endif

void CBulkOnlyTransportUsbcLdd::Activate(TRequestStatus& aStatus, TUint& aDeviceState)
    {
	iLdd.AlternateDeviceStatusNotify(aStatus, aDeviceState);
    }


void CBulkOnlyTransportUsbcLdd::Cancel()
    {
	iLdd.AlternateDeviceStatusNotifyCancel();
    }




