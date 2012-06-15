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
// e32/drivers/usbcc/ps_usbc.cpp
// Platform independent layer (PIL) of the USB Device controller driver (PDD).
// Interface to the USB LDD.
// 
//

/**
 @file ps_usbc.cpp
 @internalTechnology
*/

#include <drivers/usbc.h>


/**
	TUsbcInterfaceSet and TUsbcInterface
	====================================

	TUsbcInterfaceSet represents a 'USB Interface' and TUsbcInterface
	represents an 'Alternate Setting of a USB Interface'.

	Since every LDD governs exactly one interface, the above distinction is
	made only within the USB implementation. At the LDD API, there is/are
	simply one or more settings for this single interface, numbered from '0'
	(the default) to 'n', and specified by the parameter 'TInt aInterfaceNum'.

	Within the PDD implementation, for a TUsbcInterfaceSet number the parameter
	'TInt aIfcSet' is used (local variable ifcset); for a TUsbcInterface number
	the parameter 'TInt aIfc' is used (local variable ifc).


	iConfigs[0] and CurrentConfig()
	===============================

	One problem with this file is that it always uses iConfigs[0] and not
	CurrentConfig(). This is mainly because the API to the LDD doesn't know
	about the concept of multiple configurations, and thus always assumes one
	single configuration (which is also always active: a further problem).

	In the file chapter9.cpp this issue doesn't exist, since there we always
	have to obey the USB protocol, and in this way will use the configuration
	which is selected by the host (which will then again currently always be
	iConfigs[0].)


	iEp0ClientId and iEp0DataReceiving
	==================================

	The purpose of these two members of class DUsbClientController is the
	following.

	They are used only during Ep0 control transactions which have an OUT (Rx)
	data stage. The special problem with these transactions is twofold. For one
	thing we have to know that what we are receiving is data and not a Setup
	packet. Furthermore we cannot deduce from the received data itself to whom
	it is addressed (that's because of the shared nature of Ep0).

	So in order to recognize data packets we use iEp0DataReceiving. This
	variable is set to TRUE either 1) upon processing a standard request which
	has a DATA_OUT phase (only SET_DESCRIPTOR), or 2) if we have identified a
	class-specific request which has a DATA_OUT phase and we have also found
	the recipient for that request.

	In order to be able to tell whether received Ep0 data is to be processed by
	the PIL or a LDD, we use iEp0ClientId. iEp0ClientId is usually NULL, which
	means it is our data. However it is set to the client ID of an LDD in case
	2) above. That way we can subsequently hand over received data to the
	correct client LDD.

	iEp0DataReceived tracks the amount of data already received - it is used to
	determine the end of the DATA_OUT phase, irrespective of the owner of the
	data. The total amount that is to be received can be obtained via
	iSetup.iLength. (iSetup holds in that case the Setup packet of the current
	Control transfer.)

	iEp0ClientDataTransmitting is only set to TRUE if a client sets up an Ep0
	write. After that transmission has completed we use this value to decide
	whether we have to report the completion to a client or not. (If this
	variable is FALSE, we did set up the write and thus no client notification
	is necessary.)

*/

//
// === Global and Local Variables ==================================================================
//

GLDEF_D DUsbClientController* DUsbClientController::UsbClientController[] = {NULL, NULL};

static const TInt KUsbReconnectDelay   = 500;				// milliseconds
static const TInt KUsbCableStatusDelay = 500;				// milliseconds


//
// === USB Controller member function implementations - LDD API (public) ===========================
//


/** The class destructor.

	This rarely gets called, except, for example when something goes
	wrong during construction.

	It's not exported because it is virtual.
*/
DUsbClientController::~DUsbClientController()
	{
	__KTRACE_OPT(KUSB, Kern::Printf("DUsbClientController::~DUsbClientController()"));
	if (iPowerHandler)
		{
		iPowerHandler->Remove();
		delete iPowerHandler;
		}
	// ResetAndDestroy() will call for every array element the destructor of the pointed-to object,
	// before deleting the element itself, and closing the array.
	iConfigs.ResetAndDestroy();
	__KTRACE_OPT(KUSB, Kern::Printf("DUsbClientController::~DUsbClientController(): Done."));
	}


/** To be called by the OTG/Host stack in an OTG setup to disable USB device
	functionality.

	The OTG stack calls this function when VBus is no longer valid, when the
	B-device swaps out of peripheral mode, or when moving out of the
	A_PERIPHERAL state.

	The Client stack will disable the D+ pull-up immediately when the function
	is called.

	During DisableClientStack() the Client stack will notify its registered
	applications on the user-side (including the USB Manager) about a USB
	device state change event, a transition to the "Undefined" state.
*/
EXPORT_C void DUsbClientController::DisableClientStack()
	{
	__KTRACE_OPT(KUSB, Kern::Printf("DUsbClientController::DisableClientStack()"));
	if (!iStackIsActive)
		{
		__KTRACE_OPT(KUSB, Kern::Printf("  Already disabled - returning"));
		return;
		}
	iOtgClientConnect = EFalse;
	TInt r = EvaluateOtgConnectFlags();					 // will disconnect UDC
	if (r != KErrNone)
		{
		__KTRACE_OPT(KPANIC, Kern::Printf("  Error: EvaluateOtgConnectFlags() failed: %d", r));
		}

	// Reset OTG features, leave attributes as is (just as in USB Reset case)
	// (OTG spec 1.3 sections 6.5.x all say "... on a bus reset or at the end
	//  of a session." VBus drop is the end of a session.)
	iOtgFuncMap &= KUsbOtgAttr_SrpSupp | KUsbOtgAttr_HnpSupp;
	OtgFeaturesNotify();
	// Tear down the current configuration (if any)
	ChangeConfiguration(0);

	if (iDeviceState != EUsbcDeviceStateUndefined)
		{
		// Not being in state UNDEFINED implies that the cable is inserted.
		if (iHardwareActivated)
			{
			NextDeviceState(EUsbcDeviceStatePowered);
			}
		// (If the hardware is NOT activated at this point, we can only be in
		//	state EUsbcDeviceStateAttached, so we don't have to move to it.)
		}
	DeActivateHardwareController();					 // turn off UDC altogether
	iStackIsActive = EFalse;
	// Complete all pending requests, returning KErrDisconnected
	RunClientCallbacks();
	// Notify registered clients on the user side about a USB device state
	// change event and a transition to the "Undefined" state.
	NextDeviceState(EUsbcDeviceStateUndefined);
	}


/** To be called by the OTG/Host stack in an OTG setup to enable USB device
	functionality.

	Once called, the function will return quickly, but it will by then not
	necessarily have enabled the D+ pull-up*. The Client stack can enable the D+
	pull-up (via the transceiver) from that moment on and as long as the OTG
	stack doesn't call DisableClientStack().

	*) It will enable the D+ pull-up immediately if the user-side USB support
	has already been loaded. This should always be the case when the OTG stack
	is calling this function during the transition to the A_PERIPHERAL state,
	i.e. when acting as an A-device.
*/
EXPORT_C void DUsbClientController::EnableClientStack()
	{
	__KTRACE_OPT(KUSB, Kern::Printf("DUsbClientController::EnableClientStack()"));
	if (iStackIsActive)
		{
		__KTRACE_OPT(KUSB, Kern::Printf("  Already enabled - returning"));
		return;
		}
	iStackIsActive = ETrue;
	// If the UDC is still off, we switch it on here.
	TInt r = ActivateHardwareController();
	if (r != KErrNone)
		{
		__KTRACE_OPT(KPANIC, Kern::Printf("  Error: ActivateHardwareController() failed: %d", r));
		}
	iOtgClientConnect = ETrue;
	r = EvaluateOtgConnectFlags();							// may connect UDC
	if (r != KErrNone)
		{
		__KTRACE_OPT(KPANIC, Kern::Printf("  Error: EvaluateOtgConnectFlags() failed: %d", r));
		}
	}


/** Called by LDD to see if controller is usable.

	@return ETrue if controller is in normal state, EFalse if it is disabled.
*/
EXPORT_C TBool DUsbClientController::IsActive()
	{
	__KTRACE_OPT(KUSB, Kern::Printf("DUsbClientController::IsActive()"));
	return iStackIsActive;
	}


/** Called by LDD to register client callbacks.

	@return KErrNone if successful, KErrAlreadyExists callback exists.
*/
EXPORT_C TInt DUsbClientController::RegisterClientCallback(TUsbcClientCallback& aCallback)
	{
	__KTRACE_OPT(KUSB, Kern::Printf("DUsbClientController::RegisterClientCallback()"));
	if (iClientCallbacks.Elements() == KUsbcMaxListLength)
		{
		__KTRACE_OPT(KPANIC, Kern::Printf("  Error: Maximum list length reached: %d",
										  KUsbcMaxListLength));
		return KErrGeneral;
		}
	TSglQueIter<TUsbcClientCallback> iter(iClientCallbacks);
	TUsbcClientCallback* p;
	while ((p = iter++) != NULL)
		if (p == &aCallback)
			{
			__KTRACE_OPT(KUSB, Kern::Printf("	 Error: ClientCallback @ 0x%x already registered", &aCallback));
			return KErrAlreadyExists;
			}
	iClientCallbacks.AddLast(aCallback);
	return KErrNone;
	}


/** Returns a pointer to the USB client controller object.

	This function is static.

	@param aUdc The number of the UDC (0..n) for which the pointer is to be returned.

	@return A pointer to the USB client controller object.
*/
EXPORT_C DUsbClientController* DUsbClientController::UsbcControllerPointer(TInt aUdc)
	{
	__KTRACE_OPT(KUSB, Kern::Printf("DUsbClientController::UsbcControllerPointer()"));
	if (aUdc < 0 || aUdc > 1)
		{
		__KTRACE_OPT(KPANIC, Kern::Printf("  Error: aUdc out of range (%d)", aUdc));
		return NULL;
		}
	return UsbClientController[aUdc];
	}


/** Fills the buffer passed in as an argument with endpoint capability information.

 	@see DUsbClientController::DeviceCaps()
	@see TUsbcEndpointData
	@see TUsbDeviceCaps

	@param aClientId A pointer to the LDD making the enquiry.
	@param aCapsBuf A reference to a descriptor buffer, which, on return, contains an array of
	TUsbcEndpointData elements; there are TUsbDeviceCaps::iTotalEndpoints elements in the array;
	call DeviceCaps() to get the number of elements required.
*/
EXPORT_C void DUsbClientController::EndpointCaps(const DBase* aClientId, TDes8& aCapsBuf) const
	{
	__KTRACE_OPT(KUSB, Kern::Printf("DUsbClientController::EndpointCaps()"));
	// Here we do not simply call DUsbClientController::DeviceEndpointCaps(),
	// because that function fills an array which comprises of _all_ endpoints,
	// whereas this function omits ep0 and all unusable endpoints.
	// Apart from that, we have to fill an array of TUsbcEndpointData, not TUsbcEndpointCaps.
	TUsbcEndpointData data[KUsbcMaxEndpoints];
	const TInt ifcset_num = ClientId2InterfaceNumber(aClientId);
	for (TInt i = 2, j = 0; i < iDeviceTotalEndpoints; ++i)
		{
		__KTRACE_OPT(KUSB, Kern::Printf("DUsbClientController::Caps: RealEndpoint #%d", i));
		if (iRealEndpoints[i].iCaps.iTypesAndDir != KUsbEpNotAvailable)
			{
			__KTRACE_OPT(KUSB, Kern::Printf("DUsbClientController::Caps: --> UsableEndpoint #%d", j));
			data[j].iCaps = iRealEndpoints[i].iCaps;
			if (ifcset_num < 0)
				{
				// If this LDD doesn't own an interface, but the Ep points to one,
				// then that must be the interface of a different LDD. Hence the Ep
				// is not available for this LDD.
				data[j].iInUse = (iRealEndpoints[i].iIfcNumber != NULL);
				}
			else
				{
				// If this LDD does already own an interface, and the Ep also points to one,
				// then the Ep is not available for this LDD only if that interface is owned
				// by a different LDD (i.e. if the interface number is different).
				// Reason: Even though the endpoint might already be part of an interface setting,
				// it is still available for a different alternate setting of the same interface.
				data[j].iInUse = ((iRealEndpoints[i].iIfcNumber != NULL) &&
								  (*(iRealEndpoints[i].iIfcNumber) != ifcset_num));
				}
			j++;
			}
		}
	// aCapsBuf resides in userland
	TPtrC8 des((TUint8*)data, sizeof(data));
	const TInt r = Kern::ThreadDesWrite((reinterpret_cast<const DLddUsbcChannel*>(aClientId))->Client(),
										&aCapsBuf, des, 0, KChunkShiftBy0, NULL);
	if (r != KErrNone)
		{
		Kern::ThreadKill((reinterpret_cast<const DLddUsbcChannel*>(aClientId))->Client(),
						 EExitPanic, r, KUsbPILKillCat);
		}
	}


/** Fills the buffer passed in as an argument with device capability information.

	@see TUsbDeviceCaps
	@see TUsbDeviceCapsV01

	@param aClientId A pointer to the LDD making the enquiry.
	@param aCapsBuf A reference to a descriptor buffer which, on return, contains
	a TUsbDeviceCaps structure.
*/
EXPORT_C void DUsbClientController::DeviceCaps(const DBase* aClientId, TDes8& aCapsBuf) const
	{
	__KTRACE_OPT(KUSB, Kern::Printf("DUsbClientController::DeviceCaps()"));
	TUsbDeviceCaps caps;
	caps().iTotalEndpoints = iDeviceUsableEndpoints;		// not DeviceTotalEndpoints()!
	caps().iConnect = SoftConnectCaps();
	caps().iSelfPowered = iSelfPowered;
	caps().iRemoteWakeup = iRemoteWakeup;
	caps().iHighSpeed = DeviceHighSpeedCaps();
	caps().iFeatureWord1 = CableDetectWithoutPowerCaps() ?
		caps().iFeatureWord1 | KUsbDevCapsFeatureWord1_CableDetectWithoutPower :
		caps().iFeatureWord1 & ~KUsbDevCapsFeatureWord1_CableDetectWithoutPower;
	caps().iFeatureWord1 = DeviceResourceAllocV2Caps() ?
		caps().iFeatureWord1 | KUsbDevCapsFeatureWord1_EndpointResourceAllocV2 :
		caps().iFeatureWord1 & ~KUsbDevCapsFeatureWord1_EndpointResourceAllocV2;
	caps().iReserved = 0;

	// aCapsBuf resides in userland
	const TInt r = Kern::ThreadDesWrite((reinterpret_cast<const DLddUsbcChannel*>(aClientId))->Client(),
										&aCapsBuf, caps, 0, KChunkShiftBy0, NULL);
	if (r != KErrNone)
		{
		Kern::ThreadKill((reinterpret_cast<const DLddUsbcChannel*>(aClientId))->Client(),
						 EExitPanic, r, KUsbPILKillCat);
		}
	}


TUsbcEndpointInfoArray::TUsbcEndpointInfoArray(const TUsbcEndpointInfo* aData, TInt aDataSize)
	{
	iType = EUsbcEndpointInfo;
	iData = (TUint8*) aData;
	if (aDataSize > 0)
		iDataSize = aDataSize;
	else
		iDataSize = sizeof(TUsbcEndpointInfo);
	}


inline TUsbcEndpointInfo& TUsbcEndpointInfoArray::operator[](TInt aIndex) const
	{
	return *(TUsbcEndpointInfo*) &iData[aIndex * iDataSize];
	}


EXPORT_C TInt DUsbClientController::SetInterface(const DBase* aClientId, DThread* aThread,
												 TInt aInterfaceNum, TUsbcClassInfo& aClass,
												 TDesC8* aString, TInt aTotalEndpointsUsed,
												 const TUsbcEndpointInfo aEndpointData[],
												 TInt (*aRealEpNumbers)[6], TUint32 aFeatureWord)
	{
	TUsbcEndpointInfoArray endpointData = TUsbcEndpointInfoArray(aEndpointData);
	return SetInterface(aClientId, aThread, aInterfaceNum, aClass, aString, aTotalEndpointsUsed,
						endpointData, (TInt*) aRealEpNumbers, aFeatureWord);
	}


/** Creates a new USB interface (one setting), complete with endpoints, descriptors, etc.,
	and chains it into the internal device configuration tree.

	@param aClientId A pointer to the LDD owning the new interface.
	@param aThread A pointer to the thread the owning LDD is running in.
	@param aInterfaceNum The interface setting number of the new interface setting. This must be 0
	if it is the first setting of the interface that gets created, or 1 more than the last setting
	that was created for this interface.
	@param aClass Contains information about the device class this interface might belong to.
	@param aString A pointer to a string that is used for the string descriptor of this interface.
	@param aTotalEndpointsUsed The number of endpoints used by this interface (and also the number of
	elements of the following array).
	@param aEndpointData An array with aTotalEndpointsUsed elements, containing information about the
	endpoints of this interface.

	@return KErrNotSupported if Control endpoints are requested by the LDD but aren't supported by the PIL,
	KErrInUse if at least one requested endpoint is - temporarily or permanently - not available for use,
	KErrNoMemory if (endpoint, interface, string) descriptor allocation fails, KErrGeneral if something else
	goes wrong during endpoint or interface or descriptor creation, KErrNone if interface successfully set up.
*/
EXPORT_C TInt DUsbClientController::SetInterface(const DBase* aClientId, DThread* aThread,
												 TInt aInterfaceNum, TUsbcClassInfo& aClass,
												 TDesC8* aString, TInt aTotalEndpointsUsed,
												 const TUsbcEndpointInfoArray aEndpointData,
												 TInt aRealEpNumbers[], TUint32 aFeatureWord)
	{
	__KTRACE_OPT(KUSB, Kern::Printf("DUsbClientController::SetInterface()"));
	if (aInterfaceNum != 0)
		{
		__KTRACE_OPT(KUSB, Kern::Printf("  alternate interface setting request: #%d", aInterfaceNum));
		}
#ifndef USB_SUPPORTS_CONTROLENDPOINTS
	for (TInt i = 0; i < aTotalEndpointsUsed; ++i)
		{
		if (aEndpointData[i].iType == KUsbEpTypeControl)
			{
			__KTRACE_OPT(KPANIC, Kern::Printf("  Error: control endpoints not supported"));
			return KErrNotSupported;
			}
		}
#endif
	// Check for endpoint availability & check those endpoint's capabilities
	const TInt ifcset_num = ClientId2InterfaceNumber(aClientId);
	// The passed-in ifcset_num may be -1 now, but that's intended.
	if (!CheckEpAvailability(aTotalEndpointsUsed, aEndpointData, ifcset_num))
		{
		__KTRACE_OPT(KPANIC, Kern::Printf("  Error: endpoints not (all) available"));
		return KErrInUse;
		}
	// Create & setup new interface
	TUsbcInterface* ifc = CreateInterface(aClientId, aInterfaceNum, aFeatureWord);
	if (ifc == NULL)
		{
		__KTRACE_OPT(KPANIC, Kern::Printf("  Error: ifc == NULL"));
		return KErrGeneral;
		}
	// Create logical endpoints
	TInt r = CreateEndpoints(ifc, aTotalEndpointsUsed, aEndpointData, aRealEpNumbers);
	if (r != KErrNone)
		{
		__KTRACE_OPT(KPANIC, Kern::Printf("  Error: CreateEndpoints() != KErrNone"));
		DeleteInterface(ifc->iInterfaceSet->iInterfaceNumber, aInterfaceNum);
		return r;
		}
	// Create & setup interface, string, and endpoint descriptors
	r = SetupIfcDescriptor(ifc, aClass, aThread, aString, aEndpointData);
	if (r != KErrNone)
		{
		return r;
		}
	return KErrNone;
	}


/** Releases an existing USB interface (one setting), complete with endpoints, descriptors, etc.,
	and removes it from the internal device configuration tree.

	@param aClientId A pointer to the LDD owning the interface.
	@param aInterfaceNum The setting number of the interface setting to be deleted. This must be
	the highest numbered (or 'last') setting for this interface.

	@return KErrNotFound if interface (not setting) for some reason cannot be found, KErrArgument if an
	invalid interface setting number is specified (not existing or existing but too small), KErrNone if
	interface successfully released or if this client doesn't own any interface.
*/
EXPORT_C TInt DUsbClientController::ReleaseInterface(const DBase* aClientId, TInt aInterfaceNum)
	{
	__KTRACE_OPT(KUSB, Kern::Printf("DUsbClientController::ReleaseInterface(..., %d)", aInterfaceNum));
	const TInt ifcset = ClientId2InterfaceNumber(aClientId);
	if (ifcset < 0)
		{
		__KTRACE_OPT(KUSB, Kern::Printf(" interface not found")); // no error
		return KErrNone;
		}
	TUsbcInterfaceSet* const ifcset_ptr = InterfaceNumber2InterfacePointer(ifcset);
	if (!ifcset_ptr)
		{
		__KTRACE_OPT(KUSB, Kern::Printf(" Error: interface number %d doesn't exist", ifcset));
		return KErrNotFound;
		}
	const TInt setting_count = ifcset_ptr->iInterfaces.Count();
	if ((setting_count - 1) != aInterfaceNum)
		{
		__KTRACE_OPT(KUSB,
					 Kern::Printf(" > Error: interface settings must be released in descending order:\n\r"
								  "   %d setting(s) exist, #%d was requested to be released.\n\r"
								  "   (#%d has to be released first)",
								  setting_count, aInterfaceNum, setting_count - 1));
		return KErrArgument;
		}
	// Tear down current setting (invalidate configured state)
	__KTRACE_OPT(KUSB, Kern::Printf(" > tearing down InterfaceSet %d", ifcset));
	// Cancel all transfers on the current setting of this interface and deconfigure all its endpoints.
	InterfaceSetTeardown(ifcset_ptr);
	// 'Setting 0' means: delete all existing settings.
	if (aInterfaceNum == 0)
		{
		TInt m = ifcset_ptr->iInterfaces.Count();
		while (m > 0)
			{
			m--;
			// Ground the physical endpoints' logical_endpoint_pointers
			const TInt n = ifcset_ptr->iInterfaces[m]->iEndpoints.Count();
			for (TInt i = 0; i < n; ++i)
				{
				TUsbcPhysicalEndpoint* ptr = const_cast<TUsbcPhysicalEndpoint*>
					(ifcset_ptr->iInterfaces[m]->iEndpoints[i]->iPEndpoint);
				ptr->iLEndpoint = NULL;
				}
			// Delete the setting itself + its ifc & ep descriptors
			DeleteInterface(ifcset, m);
			iDescriptors.DeleteIfcDescriptor(ifcset, m);
			}
		}
	else
		{
		// Ground the physical endpoints' logical_endpoint_pointers
		const TInt n = ifcset_ptr->iInterfaces[aInterfaceNum]->iEndpoints.Count();
		for (TInt i = 0; i < n; ++i)
			{
			TUsbcPhysicalEndpoint* ptr = const_cast<TUsbcPhysicalEndpoint*>
				(ifcset_ptr->iInterfaces[aInterfaceNum]->iEndpoints[i]->iPEndpoint);
			ptr->iLEndpoint = NULL;
			}
		// Delete the setting itself + its ifc & ep descriptors
		DeleteInterface(ifcset, aInterfaceNum);
		iDescriptors.DeleteIfcDescriptor(ifcset, aInterfaceNum);
		}
	// Delete the whole interface if all settings are gone
	if (ifcset_ptr->iInterfaces.Count() == 0)
		{
		DeleteInterfaceSet(ifcset);
		}
	// We now no longer have a valid current configuration
	iCurrentConfig = 0;
	if (iDeviceState == EUsbcDeviceStateConfigured)
		{
		NextDeviceState(EUsbcDeviceStateAddress);
		}
	// If it was the last interface(set)...
	if (iConfigs[0]->iInterfaceSets.Count() == 0)
		{
		__KTRACE_OPT(KUSB, Kern::Printf("  No ifc left -> turning off UDC"));
		// First disconnect the device from the bus
		UsbDisconnect();
		DeActivateHardwareController();
		// (this also disables endpoint zero; we cannot have a USB device w/o interface, see 9.6.3)
		}
	return KErrNone;
	}


/** Enforces a USB re-enumeration by disconnecting the UDC from the bus (if it is currently connected) and
	re-connecting it.

	This only works if the PSL supports it, i.e. if SoftConnectCaps() returns ETrue.
*/
EXPORT_C TInt DUsbClientController::ReEnumerate()
	{
	__KTRACE_OPT(KUSB, Kern::Printf("DUsbClientController::ReEnumerate()"));
	// If, in an OTG setup, the client stack is disabled, there's no point in
	// trying to reenumerate the device. In fact, we then don't even want to
	// turn on the UDC via ActivateHardwareController().
	if (!iStackIsActive)
		{
		__KTRACE_OPT(KUSB, Kern::Printf("  Client stack disabled -> returning here"));
		return KErrNotReady;
		}
	// We probably don't check here whether SoftConnectCaps() is ETrue, and
	// return if not, because we might still want to execute
	// ActivateHardwareController(). UsbConnect() and UsbDisconnect() should be
	// no-ops if not supported by the PSL.
	if (iConfigs[0]->iInterfaceSets.Count() == 0)
		{
		__KTRACE_OPT(KUSB, Kern::Printf("  > No interface registered -> no need to re-enumerate"));
		return KErrNone;;
		}
	if (!iHardwareActivated)
		{
		// If the UDC is still off, we switch it on here.
		const TInt r = ActivateHardwareController();
		if (r != KErrNone)
				{
				__KTRACE_OPT(KPANIC, Kern::Printf("  Error: ActivateHardwareController() failed: %d", r));
				return r;
				}
		// Finally connect the device to the bus
		UsbConnect();
		}
	else
		{
		UsbDisconnect();
		// Now we have to wait a certain amount of time, in order to give the host the opportunity
		// to come to terms with the new situation.
		// (The ETrue parameter makes the callback get called in DFC instead of in ISR context.)
		iReconnectTimer.OneShot(KUsbReconnectDelay, ETrue);
		}
	return KErrNone;;
	}


/** Powers up the UDC if one or more interfaces exist.

	@return KErrNone if UDC successfully powered up, KErrNotReady if no
	interfaces have been registered yet, KErrHardwareNotAvailable if UDC
	couldn't be activated.
*/
EXPORT_C TInt DUsbClientController::PowerUpUdc()
	{
	__KTRACE_OPT(KUSB, Kern::Printf("DUsbClientController::PowerUpUdc()"));
	// If, in an OTG setup, the client stack is disabled, we mustn't turn on
	// the UDC via ActivateHardwareController() as that would already configure
	// Ep0.
	if (!iStackIsActive)
		{
		__KTRACE_OPT(KUSB, Kern::Printf("  Client stack disabled -> returning here"));
		return KErrNotReady;
		}
	if (iConfigs[0]->iInterfaceSets.Count() == 0)
		{
		__KTRACE_OPT(KUSB, Kern::Printf("  > No interface registered -> won't power up UDC"));
		return KErrNotReady;
		}
	// If the UDC is still off, we switch it on here.
	const TInt r = ActivateHardwareController();
	if (r != KErrNone)
		{
		__KTRACE_OPT(KPANIC, Kern::Printf("  Error: ActivateHardwareController() failed: %d", r));
		}
	return r;
	}


/** Connects the UDC to the bus.

	This only works if the PSL supports it, i.e. if SoftConnectCaps() returns ETrue.

	@return KErrNone if UDC successfully connected, KErrGeneral if there was an error.
*/
EXPORT_C TInt DUsbClientController::UsbConnect()
	{
	__KTRACE_OPT(KUSB, Kern::Printf("DUsbClientController::UsbConnect()"));
#ifdef USB_OTG_CLIENT
	iClientSupportReady = ETrue;
	const TInt r = EvaluateOtgConnectFlags();
	const TInt irq = NKern::DisableAllInterrupts();
	if (iUsbResetDeferred) // implies (iOtgHnpHandledByHw == ETrue)
		{
		__KTRACE_OPT(KUSB, Kern::Printf("  Resetting USB Reset 'defer' flag"));
		iUsbResetDeferred = EFalse;
		(void) ProcessResetEvent(EFalse);
		}
	NKern::RestoreInterrupts(irq);
#else
	const TInt r = UdcConnect();
#endif // USB_OTG_CLIENT
	return r;
	}


/** Disconnects the UDC from the bus.

	This only works if the PSL supports it, i.e. if SoftConnectCaps() returns ETrue.

	@return KErrNone if UDC successfully disconnected, KErrGeneral if there was an error.
*/
EXPORT_C TInt DUsbClientController::UsbDisconnect()
	{
	__KTRACE_OPT(KUSB, Kern::Printf("DUsbClientController::UsbDisconnect()"));
#ifdef USB_OTG_CLIENT
	iClientSupportReady = EFalse;
	const TInt r = EvaluateOtgConnectFlags();
#else
	const TInt r = UdcDisconnect();
#endif // USB_OTG_CLIENT
	// There won't be any notification by the PSL about this,
	// so we have to notify the LDD/user ourselves:
	if ((r == KErrNone) && (iDeviceState != EUsbcDeviceStateUndefined))
		{
		// Not being in state UNDEFINED implies that the cable is inserted.
		if (iHardwareActivated)
			{
			NextDeviceState(EUsbcDeviceStatePowered);
			}
		// (If the hardware is NOT activated at this point, we can only be in
		//	state EUsbcDeviceStateAttached, so we don't have to move to it.)
		}
	return r;
	}


/** Registers a notification callback for changes of the USB device state.

	In the event of a device state change, the callback's state member gets updated (using SetState) with a
	new TUsbcDeviceState value, and then the callback is executed (DoCallback). 'USB device state' here refers
	to the Visible Device States as defined in chapter 9 of the USB specification.

	@param aCallback A reference to a properly filled in status callback structure.

	@return KErrNone if callback successfully registered, KErrGeneral if this callback is already registered
	(it won't be registered twice).
*/
EXPORT_C TInt DUsbClientController::RegisterForStatusChange(TUsbcStatusCallback& aCallback)
	{
	__KTRACE_OPT(KUSB, Kern::Printf("DUsbClientController::RegisterForStatusChange()"));
	if (iStatusCallbacks.Elements() == KUsbcMaxListLength)
		{
		__KTRACE_OPT(KPANIC, Kern::Printf("  Error: Maximum list length reached: %d",
										  KUsbcMaxListLength));
		return KErrGeneral;
		}
	if (IsInTheStatusList(aCallback))
		{
		__KTRACE_OPT(KUSB, Kern::Printf("  Error: StatusCallback @ 0x%x already registered", &aCallback));
		return KErrGeneral;
		}
	const TInt irq = NKern::DisableAllInterrupts();
	iStatusCallbacks.AddLast(aCallback);
	NKern::RestoreInterrupts(irq);
	return KErrNone;
	}


/** De-registers (removes from the list of pending requests) a notification callback for the USB device
	status.

	@param aClientId A pointer to the LDD owning the status change callback.

	@return KErrNone if callback successfully unregistered, KErrNotFound if the callback couldn't be found.
*/
EXPORT_C TInt DUsbClientController::DeRegisterForStatusChange(const DBase* aClientId)
	{
	__KTRACE_OPT(KUSB, Kern::Printf("DUsbClientController::DeRegisterForStatusChange()"));
	__ASSERT_DEBUG((aClientId != NULL), Kern::Fault(KUsbPILPanicCat, __LINE__));
	const TInt irq = NKern::DisableAllInterrupts();
	TSglQueIter<TUsbcStatusCallback> iter(iStatusCallbacks);
	TUsbcStatusCallback* p;
	while ((p = iter++) != NULL)
		{
		if (p->Owner() == aClientId)
			{
			__KTRACE_OPT(KUSB, Kern::Printf("  removing StatusCallback @ 0x%x", p));
			iStatusCallbacks.Remove(*p);
			NKern::RestoreInterrupts(irq);
			return KErrNone;
			}
		}
	__KTRACE_OPT(KUSB, Kern::Printf("  client not found"));
	NKern::RestoreInterrupts(irq);
	return KErrNotFound;
	}


/** Registers a notification callback for changes of the state of endpoints.

	In the event of a state change of an endpoint that is spart of an interface which is owned by the LDD
	specified in the callback structure, the callback's state member gets updated (using SetState) with a new
	value, and the callback is executed (DoCallback). 'Endpoint state' here refers to the state of the
	ENDPOINT_HALT feature of an endpoint as described in chapter 9 of the USB specification. The contents of
	the state variable reflects the state of the halt features for all endpoints of the current interface
	setting: bit 0 represents endpoint 1, bit 1 endpoint 2, etc. A set bit means 'endpoint halted', a cleared
	bit 'endpoint not halted'.

	@param aCallback A reference to a properly filled in endpoint status callback structure.

	@return KErrNone if callback successfully registered, KErrGeneral if this callback is already registered
	(it won't be registered twice).
*/
EXPORT_C TInt DUsbClientController::RegisterForEndpointStatusChange(TUsbcEndpointStatusCallback& aCallback)
	{
	__KTRACE_OPT(KUSB, Kern::Printf("DUsbClientController::RegisterForEndpointStatusChange()"));
	if (iEpStatusCallbacks.Elements() == KUsbcMaxListLength)
		{
		__KTRACE_OPT(KPANIC, Kern::Printf("  Error: Maximum list length reached: %d",
										  KUsbcMaxListLength));
		return KErrGeneral;
		}
	if (IsInTheEpStatusList(aCallback))
		{
		__KTRACE_OPT(KUSB, Kern::Printf("  Error: EpStatusCallback @ 0x%x already registered", &aCallback));
		return KErrGeneral;
		}
	const TInt irq = NKern::DisableAllInterrupts();
	iEpStatusCallbacks.AddLast(aCallback);
	NKern::RestoreInterrupts(irq);
	return KErrNone;
	}


/** De-registers (removes from the list of pending requests) a notification callback for changes of the state
	of endpoints.

	@param aClientId A pointer to the LDD owning the endpoint status change callback.

	@return KErrNone if callback successfully unregistered, KErrNotFound if the callback couldn't be found.
*/
EXPORT_C TInt DUsbClientController::DeRegisterForEndpointStatusChange(const DBase* aClientId)
	{
	__KTRACE_OPT(KUSB, Kern::Printf("DUsbClientController::DeRegisterForEndpointStatusChange()"));
	__ASSERT_DEBUG((aClientId != NULL), Kern::Fault(KUsbPILPanicCat, __LINE__));
	const TInt irq = NKern::DisableAllInterrupts();
	TSglQueIter<TUsbcEndpointStatusCallback> iter(iEpStatusCallbacks);
	TUsbcEndpointStatusCallback* p;
	while ((p = iter++) != NULL)
		{
		if (p->Owner() == aClientId)
			{
			__KTRACE_OPT(KUSB, Kern::Printf("  removing EpStatusCallback @ 0x%x", p));
			iEpStatusCallbacks.Remove(*p);
			NKern::RestoreInterrupts(irq);
			return KErrNone;
			}
		}
	__KTRACE_OPT(KUSB, Kern::Printf("  client not found"));
	NKern::RestoreInterrupts(irq);
	return KErrNotFound;
	}


/** Returns the number of the currently active alternate interface setting for this interface.

	@param aClientId A pointer to the LDD owning the interface.
	@param aInterfaceNum Here the interface gets written to.

	@return KErrNotFound if an interface for this client couldn't be found, KErrNone if setting value was
	successfully written.
*/
EXPORT_C TInt DUsbClientController::GetInterfaceNumber(const DBase* aClientId, TInt& aInterfaceNum) const
	{
	__KTRACE_OPT(KUSB, Kern::Printf("DUsbClientController::GetInterfaceNumber()"));
	const TInt ifcset = ClientId2InterfaceNumber(aClientId);
	if (ifcset < 0)
		{
		__KTRACE_OPT(KPANIC, Kern::Printf("  Error (ifc < 0)"));
		return KErrNotFound;
		}
	const TUsbcInterfaceSet* const ifcset_ptr = InterfaceNumber2InterfacePointer(ifcset);
	if (!ifcset_ptr)
		{
		__KTRACE_OPT(KPANIC, Kern::Printf("  Error: interface number %d doesn't exist", ifcset));
		return KErrNotFound;
		}
	aInterfaceNum = ifcset_ptr->iCurrentInterface;
	return KErrNone;
	}


/** This is normally called once by an LDD's destructor, either after a Close() on the user side,
	or during general cleanup.

	It might also be called by the LDD when some internal unrecoverable error occurs.

	This function
	- de-registers a possibly pending device state change notification request,
	- de-registers a possibly pending endpoint state change notification request,
	- releases all interfaces + settings owned by this LDD,
	- cancels all remaining (if any) read/write requests.

	@param aClientId A pointer to the LDD to be unregistered.

	@return KErrNone.
*/
EXPORT_C TInt DUsbClientController::DeRegisterClient(const DBase* aClientId)
	{
	__KTRACE_OPT(KUSB, Kern::Printf("DUsbClientController::DeRegisterClient(0x%x)", aClientId));
	// Cancel all device state notification requests
	DeRegisterForStatusChange(aClientId);
	// Cancel all endpoint state notification requests
	DeRegisterForEndpointStatusChange(aClientId);
	DeRegisterForOtgFeatureChange(aClientId);
	DeRegisterClientCallback(aClientId);
	// Delete the interface including all its alternate settings which might exist.
	// (If we release the default setting (0), all alternate settings are deleted as well.)
	const TInt r = ReleaseInterface(aClientId, 0);
	// Cancel all remaining (if any) read/write requests
	DeleteRequestCallbacks(aClientId);
	__KTRACE_OPT(KUSB, Kern::Printf("DUsbClientController::DeRegisterClient: Done."));
	return r;
	}


/** Returns the currently used Ep0 max packet size.

	@return The currently used Ep0 max packet size.
*/
EXPORT_C TInt DUsbClientController::Ep0PacketSize() const
	{
	const TUsbcLogicalEndpoint* const ep = iRealEndpoints[0].iLEndpoint;
	if (iHighSpeed)
		{
		__KTRACE_OPT(KUSB, Kern::Printf("  Ep0 size = %d (HS)", ep->iEpSize_Hs));
		return ep->iEpSize_Hs;
		}
	else
		{
		__KTRACE_OPT(KUSB, Kern::Printf("  Ep0 size = %d (FS)", ep->iEpSize_Fs));
		return ep->iEpSize_Fs;
		}
	}


/** Stalls Ep0.

	@param aClientId A pointer to the LDD wishing to stall Ep0 (this is for PIL internal purposes only).

	@return KErrNone if endpoint zero successfully stalled, KErrGeneral otherwise.
*/
EXPORT_C TInt DUsbClientController::Ep0Stall(const DBase* aClientId)
	{
	__KTRACE_OPT(KUSB, Kern::Printf("DUsbClientController::Ep0Stall()"));
	if (aClientId == iEp0ClientId)
		{
		ResetEp0DataOutVars();
		}
	const TInt err = StallEndpoint(KEp0_Out);
	if (err < 0)
		{
		return err;
		}
	else
		return StallEndpoint(KEp0_In);
	}


/** Sends a zero-byte status packet on Ep0.

	@param aClientId A pointer to the LDD wishing to send the status packet (not used at present).
*/
EXPORT_C void DUsbClientController::SendEp0StatusPacket(const DBase* /* aClientId */)
	{
	__KTRACE_OPT(KUSB, Kern::Printf("DUsbClientController::SendEp0StatusPacket()"));
	SendEp0ZeroByteStatusPacket();
	}


/** Returns the current USB device state.

	'USB device state' here refers to the Visible Device States as defined in chapter 9 of the USB
	specification.

	@return The current USB device state, or EUsbcDeviceStateUndefined if the UDC doesn't allow device state
	tracking (PSL's DeviceStateChangeCaps() returns EFalse).
*/
EXPORT_C TUsbcDeviceState DUsbClientController::GetDeviceStatus() const
	{
	__KTRACE_OPT(KUSB, Kern::Printf("DUsbClientController::GetDeviceStatus()"));
	return iDeviceState;
	}


/** Returns the state of an endpoint.

	'Endpoint state' here refers to the state of the ENDPOINT_HALT feature of
	an endpoint as described in chapter 9 of the USB specification.

	@param aClientId A pointer to the LDD owning the interface which contains the endpoint to be queried.
	@param aEndpointNum The number of the endpoint to be queried.

	@return The current endpoint state, or EEndpointStateUnknown if the endpoint couldn't be found.
*/
EXPORT_C TEndpointState DUsbClientController::GetEndpointStatus(const DBase* aClientId, TInt aEndpointNum) const
	{
	__KTRACE_OPT(KUSB, Kern::Printf("DUsbClientController::GetEndpointStatus()"));
	return EndpointStallStatus(aEndpointNum) ?
		EEndpointStateStalled :
		EEndpointStateNotStalled;
	}


/** Sets up a data read request for an endpoint.

	@param aCallback A reference to a properly filled in data transfer request callback structure.

	@return KErrNone if callback successfully registered or if this callback is already registered
	(but it won't be registered twice), KErrNotFound if the endpoint couldn't be found, KErrArgument if
	endpoint number invalid (PSL), KErrGeneral if something else goes wrong.
*/
EXPORT_C TInt DUsbClientController::SetupReadBuffer(TUsbcRequestCallback& aCallback)
	{
	__KTRACE_OPT(KUSB, Kern::Printf("DUsbClientController::SetupReadBuffer()"));
	const TInt ep = aCallback.iRealEpNum;
	__KTRACE_OPT(KUSB, Kern::Printf("  logical ep: #%d", aCallback.iEndpointNum));
	__KTRACE_OPT(KUSB, Kern::Printf("  real ep:    #%d", ep));
	TInt err = KErrGeneral;
	if (ep != 0)
		{
		if (iRequestCallbacks[ep])
			{
			__KTRACE_OPT(KPANIC, Kern::Printf("  Warning: RequestCallback already registered for that ep"));
			if (iRequestCallbacks[ep] == &aCallback)
				{
				__KTRACE_OPT(KPANIC, Kern::Printf("  (this same RequestCallback @ 0x%x)", &aCallback));
				}
			else
				{
				__KTRACE_OPT(KPANIC, Kern::Printf("  (a different RequestCallback @ 0x%x)", &aCallback));
				}
			return KErrNone;
			}
		// This may seem awkward:
		// First we add a callback, and then, in case of an error, we remove it again.
		// However this is necessary because the transfer request might complete (through
		// an ISR) _before_ the SetupEndpointRead function returns. Since we don't know the
		// outcome, we have to provide the callback before making the setup call.
		//
		__KTRACE_OPT(KUSB, Kern::Printf("  adding RequestCallback[%d] @ 0x%x", ep, &aCallback));
		iRequestCallbacks[ep] = &aCallback;
		if ((err = SetupEndpointRead(ep, aCallback)) != KErrNone)
			{
			__KTRACE_OPT(KPANIC, Kern::Printf("  removing RequestCallback @ 0x%x (due to error)",
											  &aCallback));
			iRequestCallbacks[ep] = NULL;
			}
		}
	else													// (ep == 0)
		{
		if (iEp0ReadRequestCallbacks.Elements() == KUsbcMaxListLength)
			{
			__KTRACE_OPT(KPANIC, Kern::Printf("  Error: Maximum list length reached: %d",
											  KUsbcMaxListLength));
			return KErrGeneral;
			}
		if (IsInTheRequestList(aCallback))
			{
			__KTRACE_OPT(KUSB, Kern::Printf("  RequestCallback @ 0x%x already registered", &aCallback));
			return KErrNone;
			}
		// Ep0 reads don't need to be prepared - there's always one pending
		__KTRACE_OPT(KUSB, Kern::Printf("  adding RequestCallback @ 0x%x (ep0)", &aCallback));
		TInt irq = NKern::DisableAllInterrupts();
		iEp0ReadRequestCallbacks.AddLast(aCallback);
		NKern::RestoreInterrupts(irq);
		err = KErrNone;
		if (iEp0_RxExtraData)
			{
			__KTRACE_OPT(KUSB, Kern::Printf("  iEp0_RxExtraData: trying again..."));
			const TBool rx_data = iEp0DataReceiving;
			const TInt irq = NKern::DisableAllInterrupts();
			err = ProcessEp0ReceiveDone(iEp0_RxExtraCount);
			NKern::RestoreInterrupts(irq);
			if (err == KErrNone)
				{
				iEp0_RxExtraData = EFalse;
				// Queue a new Ep0 read (because xxxProceed only re-enables the interrupt)
				SetupEndpointZeroRead();
				if (rx_data)
					{
					Ep0ReceiveProceed();
					}
				else
					{
					Ep0ReadSetupPktProceed();
					}
				__KTRACE_OPT(KUSB, Kern::Printf("  :-)"));
				}
			else
				{
				__KTRACE_OPT(KPANIC, Kern::Printf("  Error: :-("));
				err = KErrGeneral;
				}
			return err;
			}
		}
	return err;
	}


/** Sets up a data write request for an endpoint.

	@param aCallback A reference to a properly filled in data transfer request callback structure.

	@return KErrNone if callback successfully registered or if this callback is already registered
	(but it won't be registered twice), KErrNotFound if the endpoint couldn't be found, KErrArgument if
	endpoint number invalid (PSL), KErrGeneral if something else goes wrong.
*/
EXPORT_C TInt DUsbClientController::SetupWriteBuffer(TUsbcRequestCallback& aCallback)
	{
	__KTRACE_OPT(KUSB, Kern::Printf("DUsbClientController::SetupWriteBuffer()"));
	TInt ep = aCallback.iRealEpNum;
	__KTRACE_OPT(KUSB, Kern::Printf("  logical ep: #%d", aCallback.iEndpointNum));
	__KTRACE_OPT(KUSB, Kern::Printf("  real ep:	   #%d", ep));
	if (iRequestCallbacks[ep])
		{
		__KTRACE_OPT(KPANIC, Kern::Printf("  Warning: RequestCallback already registered for that ep"));
		if (iRequestCallbacks[ep] == &aCallback)
			{
			__KTRACE_OPT(KPANIC, Kern::Printf("  (this same RequestCallback @ 0x%x)", &aCallback));
			return KErrNone;
			}
		else
			{
			__KTRACE_OPT(KPANIC, Kern::Printf("  (a different RequestCallback @ 0x%x - poss. error)",
											  &aCallback));
			return KErrGeneral;
			}
		}
	if (ep == 0)
		{
		if (iEp0_TxNonStdCount)
			{
			if (iEp0_TxNonStdCount > aCallback.iLength)
				{
				__KTRACE_OPT(KPANIC, Kern::Printf("  Warning: Ep0 is sending less data than requested"));
				if ((aCallback.iLength % iEp0MaxPacketSize == 0) && !aCallback.iZlpReqd)
					{
					__KTRACE_OPT(KPANIC, Kern::Printf("  Warning: Zlp should probably be requested"));
					}
				}
			else if (iEp0_TxNonStdCount < aCallback.iLength)
				{
				__KTRACE_OPT(KPANIC, Kern::Printf("  Warning: Ep0 is sending more data than requested"));
				}
			iEp0_TxNonStdCount = 0;
			}
		// Ep0 IN needs to be adjusted: the LDD uses 0 for both Ep0 directions.
		ep = KEp0_Tx;
		}
	// This may seem awkward:
	// First we add a callback, and then, in case of an error, we remove it again.
	// However this is necessary because the transfer request might complete (through
	// an ISR) _before_ the SetupEndpointWrite function returns. Since we don't know the
	// outcome, we have to provide the callback before making the setup call.
	//
	__KTRACE_OPT(KUSB, Kern::Printf("  adding RequestCallback[%d] @ 0x%x", ep, &aCallback));
	iRequestCallbacks[ep] = &aCallback;
	if (ep == KEp0_Tx)
		{
		iEp0ClientDataTransmitting = ETrue;			 // this must be set before calling SetupEndpointZeroWrite
		if (SetupEndpointZeroWrite(aCallback.iBufferStart, aCallback.iLength, aCallback.iZlpReqd) != KErrNone)
			{
			__KTRACE_OPT(KPANIC, Kern::Printf("  removing RequestCallback @ 0x%x (due to error)", &aCallback));
			iRequestCallbacks[ep] = NULL;
			iEp0ClientDataTransmitting = EFalse;
			}
		}
	else if (SetupEndpointWrite(ep, aCallback) != KErrNone)
		{
		__KTRACE_OPT(KPANIC, Kern::Printf("  removing RequestCallback @ 0x%x (due to error)", &aCallback));
		iRequestCallbacks[ep] = NULL;
		}
	return KErrNone;
	}


/** Cancels a data read request for an endpoint.

	The request callback will be removed from the queue and the
	callback function won't be executed.

	@param aClientId A pointer to the LDD owning the interface which contains the endpoint.
	@param aRealEndpoint The number of the endpoint for which the transfer request is to be cancelled.
*/
EXPORT_C void DUsbClientController::CancelReadBuffer(const DBase* aClientId, TInt aRealEndpoint)
	{
	__KTRACE_OPT(KUSB, Kern::Printf("DUsbClientController::CancelReadBuffer(%d)", aRealEndpoint));
	if (aRealEndpoint < 0)
		{
		__KTRACE_OPT(KPANIC, Kern::Printf("  Error: ep # < 0: %d", aRealEndpoint));
		return;
		}
	// Note that we here don't cancel Ep0 read requests at the PSL level!
	if (aRealEndpoint > 0)
		{
		CancelEndpointRead(aRealEndpoint);
		}
	DeleteRequestCallback(aClientId, aRealEndpoint, EControllerRead);
	}


/** Cancels a data write request for an endpoint.

	It cannot be guaranteed that the data is not sent nonetheless, as some UDCs don't permit a flushing of a
	TX FIFO once it has been filled. The request callback will be removed from the queue in any case and the
	callback function won't be executed.

	@param aClientId A pointer to the LDD owning the interface which contains the endpoint.
	@param aRealEndpoint The number of the endpoint for which the transfer request is to be cancelled.
*/
EXPORT_C void DUsbClientController::CancelWriteBuffer(const DBase* aClientId, TInt aRealEndpoint)
	{
	__KTRACE_OPT(KUSB, Kern::Printf("DUsbClientController::CancelWriteBuffer(%d)", aRealEndpoint));
	if (aRealEndpoint < 0)
		{
		__KTRACE_OPT(KPANIC, Kern::Printf("  Error: ep # < 0: %d", aRealEndpoint));
		return;
		}
	if (aRealEndpoint == 0)
		{
		// Ep0 IN needs to be adjusted: the LDD uses 0 for both Ep0 directions.
		aRealEndpoint = KEp0_Tx;
		}
	CancelEndpointWrite(aRealEndpoint);
	if (aRealEndpoint == KEp0_Tx)
		{
		// Since Ep0 is shared among clients, we don't have to care about the client id.
		iEp0WritePending = EFalse;
		}
	DeleteRequestCallback(aClientId, aRealEndpoint, EControllerWrite);
	}


/** Halts (stalls) an endpoint (but not Ep0).

	@param aClientId A pointer to the LDD owning the interface which contains the endpoint to be stalled.
	@param aEndpointNum The number of the endpoint.

	@return KErrNotFound if endpoint couldn't be found (includes Ep0), KErrNone if endpoint successfully
	stalled, KErrGeneral otherwise.
*/
EXPORT_C TInt DUsbClientController::HaltEndpoint(const DBase* aClientId, TInt aEndpointNum)
	{
	__KTRACE_OPT(KUSB, Kern::Printf("DUsbClientController::HaltEndpoint(%d)", aEndpointNum));
	const TInt r = StallEndpoint(aEndpointNum);
	if (r == KErrNone)
		{
		iRealEndpoints[aEndpointNum].iHalt = ETrue;
		}
	else if (r == KErrArgument)
		{
		return KErrNotFound;
		}
	return r;
	}


/** Clears the halt condition of an endpoint (but not Ep0).

	@param aClientId A pointer to the LDD owning the interface which contains the endpoint to be un-stalled.
	@param aEndpointNum The number of the endpoint.

	@return KErrNotFound if endpoint couldn't be found (includes Ep0), KErrNone if endpoint successfully
	stalled, KErrGeneral otherwise.
*/
EXPORT_C TInt DUsbClientController::ClearHaltEndpoint(const DBase* aClientId, TInt aEndpointNum)
	{
	__KTRACE_OPT(KUSB, Kern::Printf("DUsbClientController::ClearHaltEndpoint(%d)", aEndpointNum));
	const TInt r = ClearStallEndpoint(aEndpointNum);
	if (r == KErrNone)
		{
		iRealEndpoints[aEndpointNum].iHalt = EFalse;
		}
	else if (r == KErrArgument)
		{
		return KErrNotFound;
		}
	return r;
	}


/** This function requests 'device control' for an LDD.

	Class or vendor specific Ep0 requests addressed to the USB device as a whole (Recipient field in
	bmRequestType byte of a Setup packet set to zero) are delivered to the LDD that owns device control. For
	obvious reasons only one USB LDD can have device control at any given time.

	@param aClientId A pointer to the LDD requesting device control.

	@return KErrNone if device control successfully claimed or if this LDD already owns it, KErrGeneral if
	device control already owned by a different client.
*/
EXPORT_C TInt DUsbClientController::SetDeviceControl(const DBase* aClientId)
	{
	__KTRACE_OPT(KUSB, Kern::Printf("DUsbClientController::SetDeviceControl()"));
	if (iEp0DeviceControl)
		{
		if (iEp0DeviceControl == aClientId)
			{
			__KTRACE_OPT(KPANIC, Kern::Printf("  Warning: Device Control already owned by this client"));
			return KErrNone;
			}
		__KTRACE_OPT(KPANIC, Kern::Printf("  Error: Device Control already claimed by a different client"));
		return KErrGeneral;
		}
	iEp0DeviceControl = aClientId;
	return KErrNone;
	}


/** This function releases device control for an LDD.

	@see DUsbClientController::SetDeviceControl()

	@param aClientId A pointer to the LDD releasing device control.

	@return KErrNone if device control successfully released, KErrGeneral if device control owned by a
	different client or by no client at all.
*/
EXPORT_C TInt DUsbClientController::ReleaseDeviceControl(const DBase* aClientId)
	{
	__KTRACE_OPT(KUSB, Kern::Printf("DUsbClientController::ReleaseDeviceControl()"));
	if (iEp0DeviceControl)
		{
		if (iEp0DeviceControl == aClientId)
			{
			__KTRACE_OPT(KUSB, Kern::Printf("  Releasing Device Control"));
			iEp0DeviceControl = NULL;
			return KErrNone;
			}
		__KTRACE_OPT(KPANIC, Kern::Printf("  Error: Device Control owned by a different client"));
		}
	else
		{
		__KTRACE_OPT(KPANIC, Kern::Printf("  Error: Device Control not owned by any client"));
		}
	return KErrGeneral;
	}


/** Returns all available (configurable) max packet sizes for Ep0.

	The information is coded as bitwise OR'ed values of KUsbEpSizeXXX constants (the bitmap format used for
	TUsbcEndpointCaps.iSupportedSizes).

	@return All available (configurable) max packet sizes for Ep0.
*/
EXPORT_C TUint DUsbClientController::EndpointZeroMaxPacketSizes() const
	{
	__KTRACE_OPT(KUSB, Kern::Printf("DUsbClientController::EndpointZeroMaxPacketSizes()"));
	return iRealEndpoints[0].iCaps.iSizes;
	}


/** Sets (configures) the max packet size for Ep0.

	For available sizes as returned by DUsbClientController::EndpointZeroMaxPacketSizes()

	Note that for HS operation the Ep0 size cannot be chosen, but is fixed at 64 bytes.

	@return KErrNotSupported if invalid size specified, KErrNone if new max packet size successfully set or
	requested size was already set.
*/
EXPORT_C TInt DUsbClientController::SetEndpointZeroMaxPacketSize(TInt aMaxPacketSize)
	{
	__KTRACE_OPT(KUSB, Kern::Printf("DUsbClientController::SetEndpointZeroMaxPacketSize(%d)",
									aMaxPacketSize));

	if (DeviceHighSpeedCaps())
		{
		// We're not going to mess with this on a HS device.
		return KErrNone;
		}

	if (!(iRealEndpoints[0].iCaps.iSizes & PacketSize2Mask(aMaxPacketSize)))
		{
		__KTRACE_OPT(KPANIC, Kern::Printf("  Error: invalid size"));
		return KErrNotSupported;
		}
	if (iRealEndpoints[0].iLEndpoint->iEpSize_Fs == aMaxPacketSize)
		{
		__KTRACE_OPT(KUSB, Kern::Printf("  this packet size already set -> returning"));
		return KErrNone;
		}
	const TUsbcLogicalEndpoint* const ep0_0 = iRealEndpoints[0].iLEndpoint;
	const TUsbcLogicalEndpoint* const ep0_1 = iRealEndpoints[1].iLEndpoint;
	const_cast<TUsbcLogicalEndpoint*>(ep0_0)->iEpSize_Fs = aMaxPacketSize;
	const_cast<TUsbcLogicalEndpoint*>(ep0_1)->iEpSize_Fs = aMaxPacketSize;

	// @@@ We should probably modify the device descriptor here as well...

	if (iHardwareActivated)
		{
		// De-configure endpoint zero
		DeConfigureEndpoint(KEp0_Out);
		DeConfigureEndpoint(KEp0_In);
		// Re-configure endpoint zero
		const_cast<TUsbcLogicalEndpoint*>(ep0_0)->iInfo.iSize = ep0_0->iEpSize_Fs;
		const_cast<TUsbcLogicalEndpoint*>(ep0_1)->iInfo.iSize = ep0_1->iEpSize_Fs;
		ConfigureEndpoint(0, ep0_0->iInfo);
		ConfigureEndpoint(1, ep0_1->iInfo);
		iEp0MaxPacketSize = ep0_0->iInfo.iSize;
		}
	return KErrNone;
	}


/** Returns the current USB Device descriptor.

	@param aThread A pointer to the thread the LDD requesting the descriptor is running in.
	@param aDeviceDescriptor A reference to a buffer into which the requested descriptor should be written
	(most likely located user-side).

	@return The return value of the thread write operation, Kern::ThreadWrite(), when writing to the target
	buffer.
*/
EXPORT_C TInt DUsbClientController::GetDeviceDescriptor(DThread* aThread, TDes8& aDeviceDescriptor)
	{
	__KTRACE_OPT(KUSB, Kern::Printf("DUsbClientController::GetDeviceDescriptor()"));
	return iDescriptors.GetDeviceDescriptorTC(aThread, aDeviceDescriptor);
	}


/** Sets a new USB Device descriptor.

	@param aThread A pointer to the thread the LDD requesting the setting of the descriptor is running in.
	@param aDeviceDescriptor A reference to a buffer which contains the descriptor to be set (most likely
	located user-side).

	@return The return value of the thread read operation, Kern::ThreadRead(), when reading from the source
	buffer in case of a failure, KErrNone if the new descriptor was successfully set.
*/
EXPORT_C TInt DUsbClientController::SetDeviceDescriptor(DThread* aThread, const TDes8& aDeviceDescriptor)
	{
	__KTRACE_OPT(KUSB, Kern::Printf("DUsbClientController::SetDeviceDescriptor()"));
	return iDescriptors.SetDeviceDescriptorTC(aThread, aDeviceDescriptor);
	}


/** Returns the current USB Device descriptor size.

	@param aThread A pointer to the thread the LDD requesting the descriptor size is running in.
	@param aSize A reference to a buffer into which the requested descriptor size should be written
	(most likely located user-side).

	@return The return value of the thread write operation, Kern::ThreadWrite(), when writing to the target
	buffer.
*/
EXPORT_C TInt DUsbClientController::GetDeviceDescriptorSize(DThread* aThread, TDes8& aSize)
	{
	__KTRACE_OPT(KUSB, Kern::Printf("DUsbClientController::GetDeviceDescriptorSize()"));
	// We do not really enquire here....
	const TPtrC8 size(reinterpret_cast<const TUint8*>(&KUsbDescSize_Device), sizeof(KUsbDescSize_Device));
	return Kern::ThreadDesWrite(aThread, &aSize, size, 0);
	}


/** Returns the current USB configuration descriptor.

	@param aThread A pointer to the thread the LDD requesting the descriptor is running in.
	@param aConfigurationDescriptor A reference to a buffer into which the requested descriptor should be
	written (most likely located user-side).

	@return The return value of the thread write operation, Kern::ThreadWrite(), when writing to the target
	buffer.
*/
EXPORT_C TInt DUsbClientController::GetConfigurationDescriptor(DThread* aThread, TDes8& aConfigurationDescriptor)
	{
	__KTRACE_OPT(KUSB, Kern::Printf("DUsbClientController::GetConfigurationDescriptor()"));
	return iDescriptors.GetConfigurationDescriptorTC(aThread, aConfigurationDescriptor);
	}


/** Sets a new USB configuration descriptor.

	@param aThread A pointer to the thread the LDD requesting the setting of the descriptor is running in.
	@param aConfigurationDescriptor A reference to a buffer which contains the descriptor to be set (most
	likely located user-side).

	@return The return value of the thread read operation, Kern::ThreadRead() when reading from the source
	buffer in case of a failure, KErrNone if the new descriptor was successfully set.
*/
EXPORT_C TInt DUsbClientController::SetConfigurationDescriptor(DThread* aThread,
															   const TDes8& aConfigurationDescriptor)
	{
	__KTRACE_OPT(KUSB, Kern::Printf("DUsbClientController::SetConfigurationDescriptor()"));
	return iDescriptors.SetConfigurationDescriptorTC(aThread, aConfigurationDescriptor);
	}


/** Returns the current USB configuration descriptor size.

	@param aThread A pointer to the thread the LDD requesting the descriptor size is running in.
	@param aSize A reference to a buffer into which the requested descriptor size should be written
	(most likely located user-side).

	@return The return value of the thread write operation, Kern::ThreadWrite(), when writing to the target
	buffer.
*/
EXPORT_C TInt DUsbClientController::GetConfigurationDescriptorSize(DThread* aThread, TDes8& aSize)
	{
	__KTRACE_OPT(KUSB, Kern::Printf("DUsbClientController::GetConfigurationDescriptorSize()"));
	// We do not really enquire here....
	const TPtrC8 size(reinterpret_cast<const TUint8*>(&KUsbDescSize_Config), sizeof(KUsbDescSize_Config));
	return Kern::ThreadDesWrite(aThread, &aSize, size, 0);
	}


/** Returns the current USB OTG descriptor.

	@param aThread A pointer to the thread the LDD requesting the descriptor size is running in.
	@param aOtgDesc A reference to a buffer into which the requested descriptor should be
	written (most likely located user-side).

	@return KErrNotSupported or the return value of the thread write operation, Kern::ThreadDesWrite(),
	when writing to the target buffer.
*/
EXPORT_C TInt DUsbClientController::GetOtgDescriptor(DThread* aThread, TDes8& aOtgDesc) const
	{
	__KTRACE_OPT(KUSB, Kern::Printf("DUsbClientController::GetOtgDescriptor()"));
	if (!iOtgSupport)
		{
		return KErrNotSupported;
		}
	return iDescriptors.GetOtgDescriptorTC(aThread, aOtgDesc);
	}


/** Sets a new OTG descriptor.

	@param aThread A pointer to the thread the LDD requesting the descriptor size is running in.
	@param aOtgDesc A reference to a buffer which contains new OTG descriptor.

	@return KErrNotSupported or the return value of the thread read operation, Kern::ThreadDesRead().
*/
EXPORT_C TInt DUsbClientController::SetOtgDescriptor(DThread* aThread, const TDesC8& aOtgDesc)
	{
	__KTRACE_OPT(KUSB, Kern::Printf("DUsbClientController::SetOtgDescriptor()"));
	if (!iOtgSupport)
		{
		return KErrNotSupported;
		}
	TBuf8<KUsbDescSize_Otg> otg;
	const TInt r = Kern::ThreadDesRead(aThread, &aOtgDesc, otg, 0);
	if (r != KErrNone)
		{
		return r;
		}
	// Check descriptor validity
	if (otg[0] != KUsbDescSize_Otg || otg[1] != KUsbDescType_Otg || otg[2] > 3)
		{
		__KTRACE_OPT(KPANIC, Kern::Printf("  Error: Invalid OTG descriptor"));
		return KErrGeneral;
		}
	__KTRACE_OPT(KUSB, Kern::Printf("  iOtgFuncMap before: 0x%x", iOtgFuncMap));
	// Update value in controller as well
	const TUint8 hnp = otg[2] & KUsbOtgAttr_HnpSupp;
	const TUint8 srp = otg[2] & KUsbOtgAttr_SrpSupp;
	if (hnp && !srp)
		{
		__KTRACE_OPT(KPANIC, Kern::Printf("  Warning: Invalid OTG attribute combination (HNP && !SRP"));
		}
	if (hnp && !(iOtgFuncMap & KUsbOtgAttr_HnpSupp))
		{
		__KTRACE_OPT(KUSB, Kern::Printf("  Setting attribute KUsbOtgAttr_HnpSupp"));
		iOtgFuncMap |= KUsbOtgAttr_HnpSupp;
		}
	else if (!hnp && (iOtgFuncMap & KUsbOtgAttr_HnpSupp))
		{
		__KTRACE_OPT(KUSB, Kern::Printf("  Removing attribute KUsbOtgAttr_HnpSupp"));
		iOtgFuncMap &= ~KUsbOtgAttr_HnpSupp;
		}
	if (srp && !(iOtgFuncMap & KUsbOtgAttr_SrpSupp))
		{
		__KTRACE_OPT(KUSB, Kern::Printf("  Setting attribute KUsbOtgAttr_SrpSupp"));
		iOtgFuncMap |= KUsbOtgAttr_SrpSupp;
		}
	else if (!srp && (iOtgFuncMap & KUsbOtgAttr_SrpSupp))
		{
		__KTRACE_OPT(KUSB, Kern::Printf("  Removing attribute KUsbOtgAttr_SrpSupp"));
		iOtgFuncMap &= ~KUsbOtgAttr_SrpSupp;
		}
	__KTRACE_OPT(KUSB, Kern::Printf("  iOtgFuncMap after:  0x%x", iOtgFuncMap));
	return iDescriptors.SetOtgDescriptor(otg);
	}


/** Returns current OTG features of USB device.

	@param aThread A pointer to the thread the LDD requesting the descriptor size is running in.
	@param aFeatures A reference to a buffer into which the requested OTG features should be written.

	@return KErrNotSupported or the return value of the thread write operation, Kern::ThreadDesWrite().
*/
EXPORT_C TInt DUsbClientController::GetOtgFeatures(DThread* aThread, TDes8& aFeatures) const
	{
	__KTRACE_OPT(KUSB, Kern::Printf("DUsbClientController::GetOtgFeatures()"));
	if (!iOtgSupport)
		{
		return KErrNotSupported;
		}
	TBuf8<1> features(1);
	features[0] = iOtgFuncMap & 0x1C;
	return Kern::ThreadDesWrite(aThread, &aFeatures, features, 0);
    }


/** Returns current OTG features of USB device. This function is intended to be
	called only from kernel side.

	@param aFeatures The reference to which the current features should be set at.
	@return KErrNone if successful, KErrNotSupported if OTG is unavailable.
*/
EXPORT_C TInt DUsbClientController::GetCurrentOtgFeatures(TUint8& aFeatures) const
	{
	__KTRACE_OPT(KUSB, Kern::Printf("DUsbClientController::GetCurrentOtgFeatures()"));
	if (!iOtgSupport)
		{
		return KErrNotSupported;
		}
	aFeatures = iOtgFuncMap & 0x1C;
	return KErrNone;
	}


/** Registers client request for OTG feature change. Client is notified when any OTG
	feature is changed.

	@see KUsbOtgAttr_B_HnpEnable, KUsbOtgAttr_A_HnpSupport, KUsbOtgAttr_A_AltHnpSupport

	@param aCallback Callback function. Gets called when OTG features change

	@return KErrNone if successful, KErrAlreadyExists if aCallback is already in the queue.
*/
EXPORT_C TInt DUsbClientController::RegisterForOtgFeatureChange(TUsbcOtgFeatureCallback& aCallback)
	{
	__KTRACE_OPT(KUSB, Kern::Printf("DUsbClientController::RegisterForOtgFeatureChange()"));
	if (iOtgCallbacks.Elements() == KUsbcMaxListLength)
		{
		__KTRACE_OPT(KPANIC, Kern::Printf("  Error: Maximum list length reached: %d",
										  KUsbcMaxListLength));
		return KErrGeneral;
		}
	if (IsInTheOtgFeatureList(aCallback))
		{
		__KTRACE_OPT(KUSB, Kern::Printf("  Error: OtgFeatureCallback @ 0x%x already registered", &aCallback));
		return KErrAlreadyExists;
		}
	const TInt irq = NKern::DisableAllInterrupts();
	iOtgCallbacks.AddLast(aCallback);
	NKern::RestoreInterrupts(irq);
	return KErrNone;
	}


/** De-registers (removes from the list of pending requests) a notification callback for
	OTG feature change.

	@param aClientId A pointer to the LDD owning the endpoint status change callback.

	@return KErrNone if callback successfully unregistered, KErrNotFound if the callback couldn't be found.
*/
EXPORT_C TInt DUsbClientController::DeRegisterForOtgFeatureChange(const DBase* aClientId)
	{
	__KTRACE_OPT(KUSB, Kern::Printf("DUsbClientController::DeRegisterForOtgFeatureChange()"));
	__ASSERT_DEBUG((aClientId != NULL), Kern::Fault(KUsbPILPanicCat, __LINE__));
	const TInt irq = NKern::DisableAllInterrupts();
	TSglQueIter<TUsbcOtgFeatureCallback> iter(iOtgCallbacks);
	TUsbcOtgFeatureCallback* p;
	while ((p = iter++) != NULL)
		{
		if (!aClientId || p->Owner() == aClientId)
			{
			__KTRACE_OPT(KUSB, Kern::Printf("  removing OtgFeatureCallback @ 0x%x", p));
			iOtgCallbacks.Remove(*p);
			NKern::RestoreInterrupts(irq);
			return KErrNone;
			}
		}
	__KTRACE_OPT(KUSB, Kern::Printf("  client not found"));
	NKern::RestoreInterrupts(irq);
	return KErrNotFound;
	}


/** Returns a specific standard USB interface descriptor.

	@param aThread A pointer to the thread the LDD requesting the descriptor is running in.
	@param aClientId A pointer to the LDD requesting the descriptor.
	@param aSettingNum The setting number of the interface for which the descriptor is requested.
	@param aInterfaceDescriptor A reference to a buffer into which the requested descriptor should be written
	(most likely located user-side).

	@return KErrNotFound if the specified interface couldn't be found, otherwise the return value of the thread
	write operation, Kern::ThreadWrite(), when writing to the target buffer.
*/
EXPORT_C TInt DUsbClientController::GetInterfaceDescriptor(DThread* aThread, const DBase* aClientId,
														   TInt aSettingNum, TDes8& aInterfaceDescriptor)
	{
	__KTRACE_OPT(KUSB, Kern::Printf("DUsbClientController::GetInterfaceDescriptor(x, 0x%08x, %d, y)",
									aClientId, aSettingNum));
	const TInt ifcset = ClientId2InterfaceNumber(aClientId);
	if (ifcset < 0)
		{
		__KTRACE_OPT(KPANIC, Kern::Printf("  Error: Interface not found from client ID"));
		return KErrNotFound;
		}
	return iDescriptors.GetInterfaceDescriptorTC(aThread, aInterfaceDescriptor, ifcset, aSettingNum);
	}


/** Sets a new standard USB interface descriptor.

	This function can also be used, by the user, and under certain conditions, to change an interface's number
	(reported as bInterfaceNumber in the descriptor). The conditions are: 1) We cannot accept a number that is
	already used by another interface, 2) We allow the interface number to be changed only when it's still the
	only setting, and 3) We allow the interface number to be changed only for the default setting (0). (All
	alternate settings created for that interface thereafter will inherit the new, changed number.)

	@param aThread A pointer to the thread the LDD requesting the setting of the descriptor is running in.
	@param aClientId A pointer to the LDD requesting the setting of the descriptor.
	@param aSettingNum The setting number of the interface for which the descriptor is to be set.
	@param aInterfaceDescriptor A reference to a buffer which contains the descriptor to be set (most
	likely located user-side).

	@return KErrNotFound if the specified interface couldn't be found, the return value of the thread read
	operation, Kern::ThreadRead(), when reading from the source buffer in case of a failure, KErrArgument if the
	interface  number is to be changed (via bInterfaceNumber in the descriptor) and either the requested
	interface number is already used by another interface or the interface has more than one setting. KErrNone
	if the new descriptor was successfully set.
*/
EXPORT_C TInt DUsbClientController::SetInterfaceDescriptor(DThread* aThread, const DBase* aClientId,
														   TInt aSettingNum, const TDes8& aInterfaceDescriptor)
	{
	__KTRACE_OPT(KUSB, Kern::Printf("DUsbClientController::SetInterfaceDescriptor(x, 0x%08x, %d, y)",
									aClientId, aSettingNum));
	const TInt ifcset = ClientId2InterfaceNumber(aClientId);
	if (ifcset < 0)
		{
		__KTRACE_OPT(KPANIC, Kern::Printf("  Error: Interface not found from client ID"));
		return KErrNotFound;
		}
	TBuf8<KUsbDescSize_Interface> new_ifc;
	TInt r = Kern::ThreadDesRead(aThread, &aInterfaceDescriptor, new_ifc, 0);
	if (r != KErrNone)
		{
		__KTRACE_OPT(KPANIC, Kern::Printf("  Error: Copying interface descriptor buffer failed (%d)", r));
		return r;
		}
	const TInt ifcset_new = new_ifc[2];
	const TBool ifc_num_changes = (ifcset != ifcset_new);
	TUsbcInterfaceSet* const ifcset_ptr = InterfaceNumber2InterfacePointer(ifcset);
	if (!ifcset_ptr)
		{
		__KTRACE_OPT(KPANIC, Kern::Printf("  Error: interface number %d doesn't exist", ifcset));
		return KErrNotFound;
		}
	if (ifc_num_changes)
		{
		// If the user wants to change the interface number, we need to do some sanity checks:
		if (InterfaceExists(ifcset_new))
			{
			// Obviously we cannot accept a number that is already used by another interface.
			__KTRACE_OPT(KPANIC, Kern::Printf("  Error: interface number %d already in use", ifcset_new));
			return KErrArgument;
			}
		if (ifcset_ptr->iInterfaces.Count() > 1)
			{
			// We allow the interface number to be changed only when it's the only setting.
			__KTRACE_OPT(KPANIC, Kern::Printf("  Error: interface has more than one alternate setting"));
			return KErrArgument;
			}
		if (aSettingNum != 0)
			{
			// We allow the interface number to be changed only when it's the default setting.
			__KTRACE_OPT(KPANIC, Kern::Printf("  Error: interface number can only be changed for setting 0"));
			return KErrArgument;
			}
		}
	if ((r = iDescriptors.SetInterfaceDescriptor(new_ifc, ifcset, aSettingNum)) != KErrNone)
		{
		__KTRACE_OPT(KPANIC, Kern::Printf("  Error: iDescriptors.SetInterfaceDescriptorfailed"));
		return r;
		}
	if (ifc_num_changes)
		{
		// Alright then, let's do it...
		__KTRACE_OPT(KUSB, Kern::Printf("  about to change interface number from %d to %d",
										ifcset, ifcset_new));
		ifcset_ptr->iInterfaceNumber = ifcset_new;
		}
	return KErrNone;
	}


/** Returns the size of a specific standard USB interface descriptor.

	@param aThread A pointer to the thread the LDD requesting the descriptor size is running in.
	@param aClientId A pointer to the LDD requesting the descriptor size.
	@param aSettingNum The setting number of the interface for which the descriptor size is requested.
	@param aSize A reference to a buffer into which the requested descriptor size should be written (most
	likely located user-side).

	@return KErrNotFound if the specified interface couldn't be found, otherwise the return value of the thread
	write operation, Kern::ThreadWrite(), when writing to the target buffer.
*/
EXPORT_C TInt DUsbClientController::GetInterfaceDescriptorSize(DThread* aThread, const DBase* aClientId,
															   TInt /*aSettingNum*/, TDes8& aSize)
	{
	__KTRACE_OPT(KUSB, Kern::Printf("DUsbClientController::GetInterfaceDescriptorSize()"));
	const TInt ifcset = ClientId2InterfaceNumber(aClientId);
	if (ifcset < 0)
		{
		__KTRACE_OPT(KPANIC, Kern::Printf("  Error: Interface not found from client ID"));
		return KErrNotFound;
		}
	// Actually, we do not really enquire here....
	const TPtrC8 size(reinterpret_cast<const TUint8*>(&KUsbDescSize_Interface), sizeof(KUsbDescSize_Interface));
	Kern::ThreadDesWrite(aThread, &aSize, size, 0);
	return KErrNone;
	}


/** Returns a specific standard USB endpoint descriptor.

	@param aThread A pointer to the thread the LDD requesting the descriptor is running in.
	@param aClientId A pointer to the LDD requesting the descriptor.
	@param aSettingNum The setting number of the interface that contains the endpoint for which the
	descriptor is requested.
	@param aEndpointNum The endpoint for which the descriptor is requested.
	@param aEndpointDescriptor A reference to a buffer into which the requested descriptor should be written
	(most likely located user-side).

	@return KErrNotFound if the specified interface or endpoint couldn't be found, otherwise the return value
	of the thread write operation, Kern::ThreadWrite(), when writing to the target buffer.
*/
EXPORT_C TInt DUsbClientController::GetEndpointDescriptor(DThread* aThread, const DBase* aClientId,
														  TInt aSettingNum, TInt aEndpointNum,
														  TDes8& aEndpointDescriptor)
	{
	__KTRACE_OPT(KUSB, Kern::Printf("DUsbClientController::GetEndpointDescriptor(x, 0x%08x, %d, %d, y)",
									aClientId, aSettingNum, aEndpointNum));
	const TInt ifcset = ClientId2InterfaceNumber(aClientId);
	if (ifcset < 0)
		{
		__KTRACE_OPT(KPANIC, Kern::Printf("  Error: Interface not found from client ID"));
		return KErrNotFound;
		}
	return iDescriptors.GetEndpointDescriptorTC(aThread, aEndpointDescriptor, ifcset,
												aSettingNum, EpIdx2Addr(aEndpointNum));
	}


/** Sets a new standard USB endpoint descriptor.

	@param aThread A pointer to the thread the LDD requesting the setting of the descriptor is running in.
	@param aClientId A pointer to the LDD requesting the setting of the descriptor.
	@param aSettingNum The setting number of the interface that contains the endpoint for which the
	descriptor is to be set.
	@param aEndpointNum The endpoint for which the descriptor is to be set.
	@param aEndpointDescriptor A reference to a buffer which contains the descriptor to be set (most
	likely located user-side).

	@return KErrNotFound if the specified interface or endpoint couldn't be found, the return value of the
	thread read operation, Kern::ThreadRead(), when reading from the source buffer in case of a read failure,
	KErrNone if the new descriptor was successfully set.
*/
EXPORT_C TInt DUsbClientController::SetEndpointDescriptor(DThread* aThread, const DBase* aClientId,
														  TInt aSettingNum, TInt aEndpointNum,
														  const TDes8& aEndpointDescriptor)
	{
	__KTRACE_OPT(KUSB, Kern::Printf("DUsbClientController::SetEndpointDescriptor(x, 0x%08x, %d, %d, y)",
									aClientId, aSettingNum, aEndpointNum));
	const TInt ifcset = ClientId2InterfaceNumber(aClientId);
	if (ifcset < 0)
		{
		__KTRACE_OPT(KPANIC, Kern::Printf("  Error: Interface not found from client ID"));
		return KErrNotFound;
		}
	return iDescriptors.SetEndpointDescriptorTC(aThread, aEndpointDescriptor, ifcset,
												aSettingNum, EpIdx2Addr(aEndpointNum));
	}


/** Returns the size of a specific standard USB endpoint descriptor.

	@param aThread A pointer to the thread the LDD requesting the descriptor size is running in.
	@param aClientId A pointer to the LDD requesting the descriptor size.
	@param aSettingNum The setting number of the interface that contains the endpoint for which the
	descriptor size is requested.
	@param aEndpointNum The endpoint for which the descriptor size is requested.
	@param aEndpointDescriptor A reference to a buffer into which the requested descriptor size should be
	written (most likely located user-side).

	@return KErrNotFound if the specified interface or endpoint couldn't be found, otherwise the return value
	of the thread write operation, kern::ThreadWrite(), when writing to the target buffer.
*/
EXPORT_C TInt DUsbClientController::GetEndpointDescriptorSize(DThread* aThread, const DBase* aClientId,
															  TInt aSettingNum, TInt aEndpointNum,
															  TDes8& aSize)
	{
	__KTRACE_OPT(KUSB, Kern::Printf("DUsbClientController::GetEndpointDescriptorSize(x, 0x%08x, %d, %d, y)",
									aClientId, aSettingNum, aEndpointNum));
	const TInt ifcset = ClientId2InterfaceNumber(aClientId);
	if (ifcset < 0)
		{
		__KTRACE_OPT(KPANIC, Kern::Printf("  Error: Interface not found from client ID"));
		return KErrNotFound;
		}
	TInt s;
	TInt r = iDescriptors.GetEndpointDescriptorSize(ifcset, aSettingNum,
													EpIdx2Addr(aEndpointNum), s);
	if (r == KErrNone)
		{
		TPtrC8 size(reinterpret_cast<const TUint8*>(&s), sizeof(s));
		r = Kern::ThreadDesWrite(aThread, &aSize, size, 0);
		}
	else
		{
		__KTRACE_OPT(KPANIC, Kern::Printf("  Error: endpoint descriptor not found"));
		}
	return r;
	}


/** Returns the current Device_Qualifier descriptor. On a USB device which doesn't support high-speed
	operation this function will return an error. Note that the contents of the descriptor depend on
	the current device speed (full-speed or high-speed).

	@param aThread A pointer to the thread the LDD requesting the descriptor is running in.
	@param aDeviceQualifierDescriptor A reference to a buffer into which the requested descriptor
	should be written (most likely located user-side).

	@return KErrNotSupported if this descriptor is not supported, otherwise the return value of the thread
	write operation, Kern::ThreadWrite(), when writing to the target buffer.
*/
EXPORT_C TInt DUsbClientController::GetDeviceQualifierDescriptor(DThread* aThread,
																 TDes8& aDeviceQualifierDescriptor)
	{
	__KTRACE_OPT(KUSB, Kern::Printf("DUsbClientController::GetDeviceQualifierDescriptor()"));
	return iDescriptors.GetDeviceQualifierDescriptorTC(aThread, aDeviceQualifierDescriptor);
	}


/** Sets a new Device_Qualifier descriptor. On a USB device which doesn't support high-speed
	operation this function will return an error. Note that the contents of the descriptor should take
	into account the current device speed (full-speed or high-speed) as it is dependent on it.

	@param aThread A pointer to the thread the LDD requesting the setting of the descriptor is running in.
	@param aDeviceQualifierDescriptor A reference to a buffer which contains the descriptor to be set (most
	likely located user-side).

	@return KErrNotSupported if this descriptor is not supported, otherwise the return value of the thread
	read operation, Kern::ThreadRead(), when reading from the source buffer in case of a failure, KErrNone if
	the new descriptor was successfully set.
*/
EXPORT_C TInt DUsbClientController::SetDeviceQualifierDescriptor(DThread* aThread,
																 const TDes8& aDeviceQualifierDescriptor)
	{
	__KTRACE_OPT(KUSB, Kern::Printf("DUsbClientController::SetDeviceQualifierDescriptor()"));
	return iDescriptors.SetDeviceQualifierDescriptorTC(aThread, aDeviceQualifierDescriptor);
	}


/** Returns the current Other_Speed_Configuration descriptor. On a USB device which doesn't support high-speed
	operation this function will return an error. Note that the contents of the descriptor depend on the
	current device speed (full-speed or high-speed).

	@param aThread A pointer to the thread the LDD requesting the descriptor is running in.
	@param aConfigurationDescriptor A reference to a buffer into which the requested descriptor
	should be written (most likely located user-side).

	@return KErrNotSupported if this descriptor is not supported, otherwise the return value of the thread
	write operation, Kern::ThreadWrite(), when writing to the target buffer.
*/
EXPORT_C TInt DUsbClientController::GetOtherSpeedConfigurationDescriptor(DThread* aThread,
																		 TDes8& aConfigurationDescriptor)
	{
	__KTRACE_OPT(KUSB, Kern::Printf("DUsbClientController::GetOtherSpeedConfigurationDescriptor()"));
	return iDescriptors.GetOtherSpeedConfigurationDescriptorTC(aThread, aConfigurationDescriptor);
	}


/** Sets a new Other_Speed_Configuration descriptor. On a USB device which doesn't support high-speed
	operation this function will return an error. Note that the contents of the descriptor should take
	into account the current device speed (full-speed or high-speed) as it is dependent on it.

	@param aThread A pointer to the thread the LDD requesting the setting of the descriptor is running in.
	@param aConfigurationDescriptor A reference to a buffer which contains the descriptor to be set (most
	likely located user-side).

	@return KErrNotSupported if this descriptor is not supported, otherwise the return value of the thread
	read operation, Kern::ThreadRead(), when reading from the source buffer in case of a failure, KErrNone if
	the new descriptor was successfully set.
*/
EXPORT_C TInt DUsbClientController::SetOtherSpeedConfigurationDescriptor(DThread* aThread,
																		 const TDes8& aConfigurationDescriptor)
	{
	__KTRACE_OPT(KUSB, Kern::Printf("DUsbClientController::SetOtherSpeedConfigurationDescriptor()"));
	return iDescriptors.SetOtherSpeedConfigurationDescriptorTC(aThread, aConfigurationDescriptor);
	}


/** Returns a block of all available non-standard (class-specific) interface descriptors for a specific
	interface.

	@param aThread A pointer to the thread the LDD requesting the descriptor block is running in.
	@param aClientId A pointer to the LDD requesting the descriptor block.
	@param aSettingNum The setting number of the interface for which the descriptor block is requested.
	@param aInterfaceDescriptor A reference to a buffer into which the requested descriptor(s) should be
	written (most likely located user-side).

	@return KErrNotFound if the specified interface couldn't be found, otherwise the return value of the thread
	write operation, Kern::ThreadWrite(), when writing to the target buffer.
*/
EXPORT_C TInt DUsbClientController::GetCSInterfaceDescriptorBlock(DThread* aThread, const DBase* aClientId,
																  TInt aSettingNum,
																  TDes8& aInterfaceDescriptor)
	{
	__KTRACE_OPT(KUSB, Kern::Printf("DUsbClientController::GetCSInterfaceDescriptorBlock(x, 0x%08x, %d, y)",
									aClientId, aSettingNum));
	const TInt ifcset = ClientId2InterfaceNumber(aClientId);
	if (ifcset < 0)
		{
		__KTRACE_OPT(KPANIC, Kern::Printf("  Error: Interface not found from client ID"));
		return KErrNotFound;
		}
	return iDescriptors.GetCSInterfaceDescriptorTC(aThread, aInterfaceDescriptor, ifcset, aSettingNum);
	}


/** Sets a block of (i.e. one or more) non-standard (class-specific) interface descriptors for a specific
	interface.

	@param aThread A pointer to the thread the LDD requesting the setting of the descriptor block is running
	in.
	@param aClientId A pointer to the LDD requesting the setting of the descriptor block.
	@param aSettingNum The setting number of the interface for which the setting of the descriptor block is
	requested.
	@param aInterfaceDescriptor A reference to a buffer which contains the descriptor block to be set (most
	likely located user-side).
	@param aSize The size of the descriptor block to be set.

	@return KErrNotFound if the specified interface couldn't be found, KErrArgument if aSize is less than 2,
	KErrNoMemory if enough memory for the new descriptor(s) couldn't be allocated, otherwise the return value
	of the thread read operation, Kern::ThreadRead(), when reading from the source buffer.
*/
EXPORT_C TInt DUsbClientController::SetCSInterfaceDescriptorBlock(DThread* aThread, const DBase* aClientId,
																  TInt aSettingNum,
																  const TDes8& aInterfaceDescriptor, TInt aSize)
	{
	__KTRACE_OPT(KUSB,
				 Kern::Printf("DUsbClientController::SetCSInterfaceDescriptorBlock(x, 0x%08x, %d, y, %d)",
							  aClientId, aSettingNum, aSize));
	const TInt ifcset = ClientId2InterfaceNumber(aClientId);
	if (ifcset < 0)
		{
		__KTRACE_OPT(KPANIC, Kern::Printf("  Error: Interface not found from client ID"));
		return KErrNotFound;
		}
	if (aSize < 2)
		{
		__KTRACE_OPT(KPANIC, Kern::Printf("  Error: aSize < 2 (%d)", aSize));
		return KErrArgument;
		}
	return iDescriptors.SetCSInterfaceDescriptorTC(aThread, aInterfaceDescriptor, ifcset, aSettingNum, aSize);
	}


/** Returns the total size all non-standard (class-specific) interface descriptors for a specific interface.

	@param aThread A pointer to the thread the LDD requesting the descriptor block size is running in.
	@param aClientId A pointer to the LDD requesting the descriptor block size.
	@param aSettingNum The setting number of the interface for which the descriptor block size is
	requested.
	@param aSize A reference to a buffer into which the requested descriptor block size should be written (most
	likely located user-side).

	@return KErrNotFound if the specified interface couldn't be found, otherwise the return value of the thread
	write operation, Kern::ThreadWrite(), when writing to the target buffer.
*/
EXPORT_C TInt DUsbClientController::GetCSInterfaceDescriptorBlockSize(DThread* aThread, const DBase* aClientId,
																	  TInt aSettingNum, TDes8& aSize)
	{
	__KTRACE_OPT(KUSB,
				 Kern::Printf("DUsbClientController::GetCSInterfaceDescriptorBlockSize(x, 0x%08x, %d, y)",
							  aClientId, aSettingNum));
	const TInt ifcset = ClientId2InterfaceNumber(aClientId);
	if (ifcset < 0)
		{
		__KTRACE_OPT(KPANIC, Kern::Printf("  Error: Interface not found from client ID"));
		return KErrNotFound;
		}
	TInt s;
	const TInt r = iDescriptors.GetCSInterfaceDescriptorSize(ifcset, aSettingNum, s);
	if (r == KErrNone)
		{
		const TPtrC8 size(reinterpret_cast<const TUint8*>(&s), sizeof(s));
		Kern::ThreadDesWrite(aThread, &aSize, size, 0);
		}
	else
		{
		__KTRACE_OPT(KPANIC, Kern::Printf("  Error: cs interface descriptor not found"));
		}
	return r;
	}


/** Returns a block of all available non-standard (class-specific) endpoint descriptors for a specific endpoint.

	@param aThread A pointer to the thread the LDD requesting the descriptor block is running in.
	@param aClientId A pointer to the LDD requesting the descriptor block.
	@param aSettingNum The setting number of the interface that contains the endpoint for which the
	descriptor block is requested.
	@param aEndpointNum The endpoint for which the descriptor block is requested.
	@param aEndpointDescriptor A reference to a buffer into which the requested descriptor(s) should be written
	(most likely located user-side).

	@return KErrNotFound if the specified interface or endpoint couldn't be found, otherwise the return value
	of the thread write operation, Kern::ThreadWrite(), when writing to the target buffer.
*/
EXPORT_C TInt DUsbClientController::GetCSEndpointDescriptorBlock(DThread* aThread, const DBase* aClientId,
																 TInt aSettingNum, TInt aEndpointNum,
																 TDes8& aEndpointDescriptor)
	{
	__KTRACE_OPT(KUSB,
				 Kern::Printf("DUsbClientController::GetCSEndpointDescriptorBlock(x, 0x%08x, %d, %d, y)",
							  aClientId, aSettingNum, aEndpointNum));
	const TInt ifcset = ClientId2InterfaceNumber(aClientId);
	if (ifcset < 0)
		{
		__KTRACE_OPT(KPANIC, Kern::Printf("  Error: Interface not found from client ID"));
		return KErrNotFound;
		}
	return iDescriptors.GetCSEndpointDescriptorTC(aThread, aEndpointDescriptor, ifcset,
												  aSettingNum, EpIdx2Addr(aEndpointNum));
	}


/** Sets a block of (i.e. one or more) non-standard (class-specific) endpoint descriptors for a specific
	endpoint.

	@param aThread A pointer to the thread the LDD requesting the setting of the descriptor block is running
	in.
	@param aClientId A pointer to the LDD requesting the setting of the descriptor block.
	@param aSettingNum The setting number of the interface that contains the endpoint for which the
	descriptor block is to be set.
	@param aEndpointNum The endpoint for which the descriptor block is to be set.
	@param aEndpointDescriptor A reference to a buffer which contains the descriptor block to be set (most
	likely located user-side).
	@param aSize The size of the descriptor block to be set.

	@return KErrNotFound if the specified interface or endpoint couldn't be found, KErrArgument if aSize is
	less than 2, KErrNoMemory if enough memory for the new descriptor(s) couldn't be allocated, otherwise the
	return value of the thread read operation, Kern::ThreadRead(), when reading from the source buffer.
*/
EXPORT_C TInt DUsbClientController::SetCSEndpointDescriptorBlock(DThread* aThread, const DBase* aClientId,
																 TInt aSettingNum, TInt aEndpointNum,
																 const TDes8& aEndpointDescriptor, TInt aSize)
	{
	__KTRACE_OPT(KUSB,
				 Kern::Printf("DUsbClientController::SetCSEndpointDescriptorBlock(x, 0x%08x, %d, %d, y)",
							  aClientId, aSettingNum, aEndpointNum));
	const TInt ifcset = ClientId2InterfaceNumber(aClientId);
	if (ifcset < 0)
		{
		__KTRACE_OPT(KPANIC, Kern::Printf("  Error: Interface not found from client ID"));
		return KErrNotFound;
		}
	if (aSize < 2)
		{
		__KTRACE_OPT(KPANIC, Kern::Printf("  Error: aSize < 2 (%d)", aSize));
		return KErrArgument;
		}
	return iDescriptors.SetCSEndpointDescriptorTC(aThread, aEndpointDescriptor, ifcset,
												  aSettingNum, EpIdx2Addr(aEndpointNum), aSize);
	}


/** Returns the total size all non-standard (class-specific) endpoint descriptors for a specific endpoint.

	@param aThread A pointer to the thread the LDD requesting the descriptor block size is running in.
	@param aClientId A pointer to the LDD requesting the descriptor block size.
	@param aSettingNum The setting number of the interface for which the descriptor block size is
	requested.
	@param aEndpointNum The endpoint for which the descriptor block size is requested.
	@param aSize A reference to a buffer into which the requested descriptor block size should be written (most
	likely located user-side).

	@return KErrNotFound if the specified interface or endpoint couldn't be found, otherwise the return value
	of the thread write operation, Kern::ThreadWrite(), when writing to the target buffer.
*/
EXPORT_C TInt DUsbClientController::GetCSEndpointDescriptorBlockSize(DThread* aThread, const DBase* aClientId,
																	 TInt aSettingNum, TInt aEndpointNum,
																	 TDes8& aSize)
	{
	__KTRACE_OPT(KUSB,
				 Kern::Printf("DUsbClientController::GetCSEndpointDescriptorBlockSize(x, 0x%08x, %d, %d, y)",
							  aClientId, aSettingNum, aEndpointNum));
	const TInt ifcset = ClientId2InterfaceNumber(aClientId);
	if (ifcset < 0)
		{
		__KTRACE_OPT(KPANIC, Kern::Printf("  Error: Interface not found from client ID"));
		return KErrNotFound;
		}
	TInt s;
	const TInt r = iDescriptors.GetCSEndpointDescriptorSize(ifcset, aSettingNum,
															EpIdx2Addr(aEndpointNum), s);
	if (r == KErrNone)
		{
		const TPtrC8 size(reinterpret_cast<const TUint8*>(&s), sizeof(s));
		Kern::ThreadDesWrite(aThread, &aSize, size, 0);
		}
	else
		{
		__KTRACE_OPT(KPANIC, Kern::Printf("  Error: cs endpoint descriptor not found"));
		}
	return r;
	}


/** Returns the currently set string descriptor language ID (LANGID) code.

	@param aThread A pointer to the thread the LDD requesting the LANGID is running in.
	@param aLangId A reference to a buffer into which the requested code should be written (most likely
	located user-side).

	@return The return value of the thread write operation, Kern::ThreadDesWrite(),
	when writing to the target buffer.
*/
EXPORT_C TInt DUsbClientController::GetStringDescriptorLangId(DThread* aThread, TDes8& aLangId)
	{
	__KTRACE_OPT(KUSB, Kern::Printf("DUsbClientController::GetStringDescriptorLangId()"));
	return iDescriptors.GetStringDescriptorLangIdTC(aThread, aLangId);
	}


/** Sets the string descriptor language ID (LANGID) code.

	@param aLangId The langauge ID code to be written.

	@return KErrNone.
*/
EXPORT_C TInt DUsbClientController::SetStringDescriptorLangId(TUint16 aLangId)
	{
	__KTRACE_OPT(KUSB, Kern::Printf("DUsbClientController::GetStringDescriptorLangId()"));
	return iDescriptors.SetStringDescriptorLangId(aLangId);
	}


/** Returns the currently set Manufacturer string (which is referenced by the iManufacturer field in the device
	descriptor).

	(Thus, the function should actually be called either 'GetManufacturerString'
	or 'GetManufacturerStringDescriptorString'.)

	@param aThread A pointer to the thread the LDD requesting the string is running in.
	@param aString A reference to a buffer into which the requested string should be written (most likely
	located user-side).

	@return KErrNotFound if the string descriptor couldn't be found (PIL internal error), otherwise the return
	value of the thread write operation, Kern::ThreadWrite(), when writing to the target buffer.
*/
EXPORT_C TInt DUsbClientController::GetManufacturerStringDescriptor(DThread* aThread, TDes8& aString)
	{
	__KTRACE_OPT(KUSB, Kern::Printf("DUsbClientController::GetManufacturerStringDescriptor()"));
	return iDescriptors.GetManufacturerStringDescriptorTC(aThread, aString);
	}


/** Sets a new Manufacturer string in the Manufacturer string descriptor (which is referenced by the
	iManufacturer field in the device descriptor).

	(Thus, the function should actually be called either
	'SetManufacturerString' or 'SetManufacturerStringDescriptorString'.)

	@param aThread A pointer to the thread the LDD requesting the setting of the string is running in.
	@param aString A reference to a buffer which contains the string to be set (most likely located
	user-side).

	@return KErrNoMemory if not enough memory for the new descriptor or the string could be allocated, the
	return value of the thread read operation, Kern::ThreadRead(), if reading from the source buffer goes wrong,
	KErrNone if new string descriptor successfully set.
*/
EXPORT_C TInt DUsbClientController::SetManufacturerStringDescriptor(DThread* aThread, const TDes8& aString)
	{
	__KTRACE_OPT(KUSB, Kern::Printf("DUsbClientController::SetManufacturerStringDescriptor()"));
	return iDescriptors.SetManufacturerStringDescriptorTC(aThread, aString);
	}


/** Removes (deletes) the Manufacturer string descriptor (which is referenced by the
	iManufacturer field in the device descriptor).

	@return KErrNone if successful, KErrNotFound if the string descriptor couldn't be found
*/
EXPORT_C TInt DUsbClientController::RemoveManufacturerStringDescriptor()
	{
	__KTRACE_OPT(KUSB, Kern::Printf("DUsbClientController::RemoveManufacturerStringDescriptor()"));
	return iDescriptors.RemoveManufacturerStringDescriptor();
	}


/** Returns the currently set Product string (which is referenced by the iProduct field in the device
	descriptor).

	(Thus, the function should actually be called either 'GetProductString' or
	'GetProductStringDescriptorString'.)

	@param aThread A pointer to the thread the LDD requesting the string is running in.
	@param aString A reference to a buffer into which the requested string should be written (most likely
	located user-side).

	@return KErrNotFound if the string descriptor couldn't be found (PIL internal error), otherwise the return
	value of the thread write operation, Kern::ThreadWrite(), when writing to the target buffer.
*/
EXPORT_C TInt DUsbClientController::GetProductStringDescriptor(DThread* aThread, TDes8& aString)
	{
	__KTRACE_OPT(KUSB, Kern::Printf("DUsbClientController::GetProductStringDescriptor()"));
	return iDescriptors.GetProductStringDescriptorTC(aThread, aString);
	}


/** Sets a new Product string in the Product string descriptor (which is referenced by the iProduct field in
	the device descriptor).

	(Thus, the function should actually be called either 'SetProductString' or
	'SetProductStringDescriptorString'.)

	@param aThread A pointer to the thread the LDD requesting the setting of the string is running in.
	@param aString A reference to a buffer which contains the string to be set (most likely located
	user-side).

	@return KErrNoMemory if not enough memory for the new descriptor or the string could be allocated, the
	return value of the thread read operation, Kern::ThreadRead(), if reading from the source buffer goes wrong,
	KErrNone if new string descriptor successfully set.
*/
EXPORT_C TInt DUsbClientController::SetProductStringDescriptor(DThread* aThread, const TDes8& aString)
	{
	__KTRACE_OPT(KUSB, Kern::Printf("DUsbClientController::SetProductStringDescriptor()"));
	return iDescriptors.SetProductStringDescriptorTC(aThread, aString);
	}


/** Removes (deletes) the Product string descriptor (which is referenced by the
	iProduct field in the device descriptor).

	@return KErrNone if successful, KErrNotFound if the string descriptor couldn't be found
*/
EXPORT_C TInt DUsbClientController::RemoveProductStringDescriptor()
	{
	__KTRACE_OPT(KUSB, Kern::Printf("DUsbClientController::RemoveProductStringDescriptor()"));
	return iDescriptors.RemoveProductStringDescriptor();
	}


/** Returns the currently set SerialNumber string (which is referenced by the iSerialNumber field in the device
	descriptor).

	(Thus, the function should actually be called either 'GetSerialNumberString' or
	'GetSerialNumberStringDescriptorString'.)

	@param aThread A pointer to the thread the LDD requesting the string is running in.
	@param aString A reference to a buffer into which the requested string should be written (most likely
	located user-side).

	@return KErrNotFound if the string descriptor couldn't be found (PIL internal error), otherwise the return
	value of the thread write operation, Kern::ThreadWrite(), when writing to the target buffer.
*/
EXPORT_C TInt DUsbClientController::GetSerialNumberStringDescriptor(DThread* aThread, TDes8& aString)
	{
	__KTRACE_OPT(KUSB, Kern::Printf("DUsbClientController::GetSerialNumberStringDescriptor()"));
	return iDescriptors.GetSerialNumberStringDescriptorTC(aThread, aString);
	}


/** Sets a new SerialNumber string in the SerialNumber string descriptor (which is referenced by the
	iSerialNumber field in the device descriptor).

	(Thus, the function should actually be called either
	'SetSerialNumberString' or 'SetSerialNumberStringDescriptorString'.)

	@param aThread A pointer to the thread the LDD requesting the setting of the string is running in.
	@param aString A reference to a buffer which contains the string to be set (most likely located
	user-side).

	@return KErrNoMemory if not enough memory for the new descriptor or the string could be allocated, the
	return value of the thread read operation, Kern::ThreadRead(), if reading from the source buffer goes wrong,
	KErrNone if new string descriptor successfully set.
*/
EXPORT_C TInt DUsbClientController::SetSerialNumberStringDescriptor(DThread* aThread, const TDes8& aString)
	{
	__KTRACE_OPT(KUSB, Kern::Printf("DUsbClientController::SetSerialNumberStringDescriptor()"));
	return iDescriptors.SetSerialNumberStringDescriptorTC(aThread, aString);
	}


/** Removes (deletes) the Serial Number string descriptor (which is referenced by the
	iSerialNumber field in the device descriptor).

	@return KErrNone if successful, KErrNotFound if the string descriptor couldn't be found
*/
EXPORT_C TInt DUsbClientController::RemoveSerialNumberStringDescriptor()
	{
	__KTRACE_OPT(KUSB, Kern::Printf("DUsbClientController::RemoveSerialNumberStringDescriptor()"));
	return iDescriptors.RemoveSerialNumberStringDescriptor();
	}


/** Returns the currently set Configuration string (which is referenced by the iConfiguration field in the
	configuration descriptor).

	(Thus, the function should actually be called either 'GetConfigurationString' or
	'GetConfigurationStringDescriptorString'.)

	@param aThread A pointer to the thread the LDD requesting the string is running in.
	@param aString A reference to a buffer into which the requested string should be written (most likely
	located user-side).

	@return KErrNotFound if the string descriptor couldn't be found (PIL internal error), otherwise the return
	value of the thread write operation, Kern::ThreadWrite(), when writing to the target buffer.
*/
EXPORT_C TInt DUsbClientController::GetConfigurationStringDescriptor(DThread* aThread, TDes8& aString)
	{
	__KTRACE_OPT(KUSB, Kern::Printf("DUsbClientController::GetConfigurationStringDescriptor()"));
	return iDescriptors.GetConfigurationStringDescriptorTC(aThread, aString);
	}


/** Sets a new Configuration string in the Configuration string descriptor (which is referenced by the
	iConfiguration field in the configuration descriptor).

	(Thus, the function should actually be called either
	'SetConfigurationString' or 'SetConfigurationStringDescriptorString'.)

	@param aThread A pointer to the thread the LDD requesting the setting of the string is running in.
	@param aString A reference to a buffer which contains the string to be set (most likely located
	user-side).

	@return KErrNoMemory if not enough memory for the new descriptor or the string could be allocated, the
	return value of the thread read operation, Kern::ThreadRead(), if reading from the source buffer goes wrong,
	KErrNone if new string descriptor successfully set.
*/
EXPORT_C TInt DUsbClientController::SetConfigurationStringDescriptor(DThread* aThread, const TDes8& aString)
	{
	__KTRACE_OPT(KUSB, Kern::Printf("DUsbClientController::SetConfigurationStringDescriptor()"));
	return iDescriptors.SetConfigurationStringDescriptorTC(aThread, aString);
	}


/** Removes (deletes) the Configuration string descriptor (which is referenced by the
	iConfiguration field in the configuration descriptor).

	@return KErrNone if successful, KErrNotFound if the string descriptor couldn't be found.
*/
EXPORT_C TInt DUsbClientController::RemoveConfigurationStringDescriptor()
	{
	__KTRACE_OPT(KUSB, Kern::Printf("DUsbClientController::RemoveConfigurationStringDescriptor()"));
	return iDescriptors.RemoveConfigurationStringDescriptor();
	}


/** Copies the string descriptor at the specified index in the string descriptor array into
	the aString argument.

	@param aIndex The position of the string descriptor in the string descriptor array.
	@param aThread A pointer to the thread the LDD requesting the string is running in.
	@param aString A reference to a buffer into which the requested string should be written (most likely
	located user-side).

	@return KErrNone if successful, KErrNotFound if no string descriptor exists at the specified index, or the
	return value of the thread write operation, Kern::ThreadWrite(), when writing to the target buffer.
*/
EXPORT_C TInt DUsbClientController::GetStringDescriptor(DThread* aThread, TUint8 aIndex, TDes8& aString)
	{
	__KTRACE_OPT(KUSB, Kern::Printf("DUsbClientController::GetStringDescriptor(%d)", aIndex));
	return iDescriptors.GetStringDescriptorTC(aThread, aIndex, aString);
	}


/** Sets the aString argument to be a string descriptor at the specified index in the string
	descriptor array. If a string descriptor already exists at that position then it will be replaced.

	@param aIndex The position of the string descriptor in the string descriptor array.
	@param aThread A pointer to the thread the LDD requesting the setting of the string is running in.
	@param aString A reference to a buffer which contains the string to be set (most likely located
	user-side).

	@return KErrNone if successful, KErrArgument if aIndex is invalid, KErrNoMemory if no memory is available
	to store the new string (an existing descriptor at that index will be preserved), or the return value of
	the thread read operation, Kern::ThreadRead(), if reading from the source buffer goes wrong.
*/
EXPORT_C TInt DUsbClientController::SetStringDescriptor(DThread* aThread, TUint8 aIndex, const TDes8& aString)
	{
	__KTRACE_OPT(KUSB, Kern::Printf("DUsbClientController::SetStringDescriptor(%d)", aIndex));
	return iDescriptors.SetStringDescriptorTC(aThread, aIndex, aString);
	}


/** Removes (deletes) the string descriptor at the specified index in the string descriptor array.

	@param aIndex The position of the string descriptor in the string descriptor array.

	@return KErrNone if successful, KErrNotFound if no string descriptor exists at the specified index.
*/
EXPORT_C TInt DUsbClientController::RemoveStringDescriptor(TUint8 aIndex)
	{
	__KTRACE_OPT(KUSB, Kern::Printf("DUsbClientController::RemoveStringDescriptor(%d)", aIndex));
	return iDescriptors.RemoveStringDescriptor(aIndex);
	}


/** Allocates an endpoint resource.

	If the resource gets successfully allocated, it will be used from when the current bus transfer
	has been completed.

	@param aClientId A pointer to the LDD requesting the endpoint resource.
	@param aEndpointNum The number of the endpoint.
	@param aResource The endpoint resource to be allocated.

	@return KErrNone if the resource has been successfully allocated, KErrNotSupported if the endpoint
	does not support the resource requested, and KErrInUse if the resource is already consumed and
	cannot be allocated. KErrArgument if the endpoint number is invalid.
*/
EXPORT_C TInt DUsbClientController::AllocateEndpointResource(const DBase* /*aClientId*/, TInt aEndpointNum,
															 TUsbcEndpointResource aResource)
	{
	__KTRACE_OPT(KUSB, Kern::Printf("DUsbClientController::AllocateEndpointResource()"));
	return AllocateEndpointResource(aEndpointNum, aResource);
	}


/** Deallocates (frees) an endpoint resource.

	The resource will be removed from when the current bus transfer has been completed.

	@param aClientId A pointer to the LDD requesting the freeing of the endpoint resource.
	@param aEndpointNum The number of the endpoint.
	@param aResource The endpoint resource to be deallocated.

	@return KErrNone if the resource has been successfully deallocated, KErrNotSupported if the endpoint
	does not support the resource requested. KErrArgument if the endpoint number is invalid.
*/
EXPORT_C TInt DUsbClientController::DeAllocateEndpointResource(const DBase* /*aClientId*/, TInt aEndpointNum,
															   TUsbcEndpointResource aResource)
	{
	__KTRACE_OPT(KUSB, Kern::Printf("DUsbClientController::DeAllocateEndpointResource()"));
	return DeAllocateEndpointResource(aEndpointNum, aResource);
	}


/** Queries the use of and endpoint resource.

	If the resource gets successfully allocated, it will be used from when the current bus transfer
	has been completed.

	@param aClientId A pointer to the LDD querying the endpoint resource.
	@param aEndpointNum The number of the endpoint.
	@param aResource The endpoint resource to be queried.

	@return ETrue if the specified resource is in use at the endpoint, EFalse if not or if there was any error
	during the execution of the function.
*/
EXPORT_C TBool DUsbClientController::QueryEndpointResource(const DBase* /*aClientId*/, TInt aEndpointNum,
														   TUsbcEndpointResource aResource)
	{
	__KTRACE_OPT(KUSB, Kern::Printf("DUsbClientController::QueryEndpointResource()"));
	return QueryEndpointResource(aEndpointNum, aResource);
	}


EXPORT_C TInt DUsbClientController::EndpointPacketSize(const DBase* aClientId, TInt aEndpointNum)
	{
	__KTRACE_OPT(KUSB, Kern::Printf("DUsbClientController::EndpointPacketSize(0x%08x, %d)",
									aClientId, aEndpointNum));

	const TUsbcInterfaceSet* const ifcset_ptr = ClientId2InterfacePointer(aClientId);
	if (!ifcset_ptr)
		{
		__KTRACE_OPT(KPANIC, Kern::Printf("  Error: interface or clientid not found"));
		return -1;
		}
	const TUsbcInterface* const ifc_ptr = ifcset_ptr->iInterfaces[ifcset_ptr->iCurrentInterface];
	const RPointerArray<TUsbcLogicalEndpoint>& ep_array = ifc_ptr->iEndpoints;
	const TInt n = ep_array.Count();
	for (TInt i = 0; i < n; i++)
		{
		const TUsbcLogicalEndpoint* const ep = ep_array[i];
		if (EpAddr2Idx(ep->iPEndpoint->iEndpointAddr) == static_cast<TUint>(aEndpointNum))
			{
			__KTRACE_OPT(KUSB, Kern::Printf("  Endpoint packet sizes: FS = %d  HS = %d",
											ep->iEpSize_Fs, ep->iEpSize_Hs));
			const TInt size = iHighSpeed ? ep->iEpSize_Hs : ep->iEpSize_Fs;
			__KTRACE_OPT(KUSB, Kern::Printf("  Returning %d", size));
			return size;
			}
		}
	__KTRACE_OPT(KPANIC, Kern::Printf("  Error: endpoint not found"));
	return -1;
	}


//
// === USB Controller member function implementations - LDD API (public) ===========================
//

EXPORT_C TBool DUsbClientController::CurrentlyUsingHighSpeed()
	{
	__KTRACE_OPT(KUSB, Kern::Printf("DUsbClientController::CurrentlyUsingHighSpeed()"));
	return EFalse;
	}


//
// === USB Controller member function implementations - PSL API (public) ===========================
//

/** Gets called by the PSL to register a newly created derived class controller object.

	@param aUdc The number of the new UDC. It should be 0 for the first (or only) UDC in the system, 1 for the
	second one, and so forth. KUsbcMaxUdcs determines how many UDCs are supported.

	@return A pointer to the controller if successfully registered, NULL if aUdc out of (static) range.

	@publishedPartner @released
*/
DUsbClientController* DUsbClientController::RegisterUdc(TInt aUdc)
	{
	__KTRACE_OPT(KUSB, Kern::Printf("DUsbClientController::RegisterUdc()"));
	if (aUdc < 0 || aUdc > (KUsbcMaxUdcs - 1))
		{
		__KTRACE_OPT(KPANIC, Kern::Printf("  Error: aUdc out of range (%d)", aUdc));
		return NULL;
		}
	return UsbClientController[aUdc] = this;
	}


//
// === USB Controller member function implementations - PSL API (protected) ========================
//

/** Initialises an instance of this class, which is the base class of the derived class (= PSL, which is
	supposed to call this function).

	It does the following things:

	- disconnects the UDC from the bus,
	- initialises the USB descriptor pool, uses data from the PSL (see function argument list)
	- creates and initialises the basic USB device configuration
	- initialises the array of physical endpoints
	- initialises Ep0 structures (but doesn't configure & enable Ep0 yet)
	- creates and installs the USB power handler

	@param aDeviceDesc A pointer to a valid standard USB device descriptor or NULL. The values initially
	required in the descriptor follow from its constructor. The descriptor is not copied over, but rather this
	pointer is queued directly into the descriptor pool. Must be writable memory.

	@param aConfigDesc A pointer to a valid standard USB configuration descriptor or NULL. The values
	initially required in the descriptor follow from its constructor. The descriptor is not copied over, but
	rather this pointer is queued directly into the descriptor pool. Must be writable memory.

	@param aLangId A pointer to a valid USB language ID (string) descriptor. The values initially required in
	the descriptor follow from its constructor. The descriptor is not copied over, but rather this pointer is
	queued directly into the descriptor pool. Must be writable memory. Other than the remaining four string
	descriptors, this one is not optional. The reason is that the USB spec mandates a LangId descriptor as
	soon as a single string descriptor gets returned by the device. So, even though the device might omit the
	Manufacturer, Product, SerialNumber, and Configuration string descriptors, it is at this point not known
	whether there will be any Interface string descriptors. Since any USB API user can create an interface
	with an Interface string descriptor, we have to insist here on the provision of a LangId string
	descriptor. (The PIL decides at run-time whether or not to return the LangId string descriptor to the
	host, depending on whether there exist any string descriptors at that time.)

	@param aManufacturer A pointer to a valid USB string descriptor or NULL. The values initially required in
	the descriptor follow from its constructor. The descriptor is not copied over, but rather this pointer is
	queued directly into the descriptor pool. Must be writable memory. This descriptor will be referenced by
	the iManufacturer field in the device descriptor.

	@param aProduct A pointer to a valid USB string descriptor or NULL. The values initially required in the
	descriptor follow from its constructor. The descriptor is not copied over, but rather this pointer is
	queued directly into the descriptor pool. Must be writable memory. This descriptor will be referenced by
	the iProduct field in the device descriptor.

	@param aSerialNum A pointer to a valid USB string descriptor or NULL. The values initially required in the
	descriptor follow from its constructor. The descriptor is not copied over, but rather this pointer is
	queued directly into the descriptor pool. Must be writable memory. This descriptor will be referenced by
	the iSerialNumber field in the device descriptor.

	@param aConfig A pointer to a valid USB string descriptor or NULL. The values initially required in the
	descriptor follow from its constructor. The descriptor is not copied over, but rather this pointer is
	queued directly into the descriptor pool. Must be writable memory. This descriptor will be referenced by
	the iConfiguration field in the configuration descriptor.

	@param aOtgDesc A pointer to a valid USB OTG descriptor (if OTG is supported by this device and is to be
	supported by the driver) or NULL. The values initially required in the descriptor follow from its
	constructor. The descriptor is not copied over, but rather this pointer is queued directly into the
	descriptor pool. Must be writable memory.

	@return EFalse, if USB descriptor pool initialisation fails, or if configuration creation fails, or if the
	PSL reports more endpoints than the constant KUsbcMaxEndpoints permits, or if the Ep0 logical endpoint
	creation fails, or if the creation of the power handler fails; ETrue, if base class object successfully
	initialised.

	@publishedPartner @released
*/
TBool DUsbClientController::InitialiseBaseClass(TUsbcDeviceDescriptor* aDeviceDesc,
												TUsbcConfigDescriptor* aConfigDesc,
												TUsbcLangIdDescriptor* aLangId,
												TUsbcStringDescriptor* aManufacturer,
												TUsbcStringDescriptor* aProduct,
												TUsbcStringDescriptor* aSerialNum,
												TUsbcStringDescriptor* aConfig,
                                                TUsbcOtgDescriptor* aOtgDesc)
	{
	__KTRACE_OPT(KUSB, Kern::Printf("DUsbClientController::InitialiseBaseClass()"));
	// We don't want the host to see us (at least not yet):
	UsbDisconnect();

	// Initialise USB descriptor pool
	if (iDescriptors.Init(aDeviceDesc, aConfigDesc, aLangId, aManufacturer, aProduct,
						  aSerialNum, aConfig, aOtgDesc) !=	KErrNone)
		{
		__KTRACE_OPT(KUSB, Kern::Printf("  Error: Descriptor initialization failed"));
		return EFalse;
		}

	if (aOtgDesc)
		{
		iOtgSupport = ETrue;
		iOtgFuncMap = aOtgDesc->DescriptorData()[2];
		// We're only interested in the following capability if this is
		// actually an OTG device.
		iOtgHnpHandledByHw = DeviceHnpHandledByHardwareCaps();
		}

	// Some member variables
	iSelfPowered  = aConfigDesc->Byte(7) & (1 << 6);		// Byte 7: bmAttributes
	iRemoteWakeup = aConfigDesc->Byte(7) & (1 << 5);
	iRmWakeupStatus_Enabled = EFalse;						// default

	if (DeviceHighSpeedCaps())
		{
		if (iDescriptors.InitHs() != KErrNone)
			{
			return EFalse;
			}
		}

	// Create and initialise our first (and only) configuration
	TUsbcConfiguration* config = new TUsbcConfiguration(1);
	if (!config)
		{
		return EFalse;
		}
	iConfigs.Append(config);

	// Some variable initializations (needed here because of the goto's)
	const TUsbcEndpointCaps* caps = NULL;
	TUsbcEndpointInfo info(KUsbEpTypeControl, KUsbEpDirOut, 0);
	TUsbcLogicalEndpoint* ep = NULL;

	// Initialise the array of physical endpoints
	iDeviceTotalEndpoints = DeviceTotalEndpoints();
	__KTRACE_OPT(KUSB, Kern::Printf("  DeviceTotalEndpoints: %d", iDeviceTotalEndpoints));
	// KUsbcMaxEndpoints doesn't include ep 0
	if ((iDeviceTotalEndpoints > (KUsbcMaxEndpoints + 2)) ||
		((iDeviceTotalEndpoints * sizeof(TUsbcPhysicalEndpoint)) > sizeof(iRealEndpoints)))
		{
		__KTRACE_OPT(KPANIC, Kern::Printf("  Error: too many endpoints! (change KUsbcMaxEndpoints: %d)",
										  KUsbcMaxEndpoints));
		goto exit_1;
		}
	caps = DeviceEndpointCaps();
	for (TInt i = 0; i < iDeviceTotalEndpoints; ++i)
		{
		iRealEndpoints[i].iEndpointAddr = EpIdx2Addr(i);
		__KTRACE_OPT(KUSB, Kern::Printf("  Caps[%02d] - iTypes: 0x%08x iSizes: 0x%08x",
										i, caps[i].iTypesAndDir, caps[i].iSizes));
		iRealEndpoints[i].iCaps = caps[i];
		iRealEndpoints[i].iCaps.iReserved[0] = 0;
		iRealEndpoints[i].iCaps.iReserved[1] = 0;
		if ((i > 1) && (caps[i].iTypesAndDir != KUsbEpNotAvailable))
			{
			__KTRACE_OPT(KUSB, Kern::Printf("  --> UsableEndpoint: #%d", i));
			iDeviceUsableEndpoints++;
			}
		}

	// Initialise Ep0 structures (logical endpoints are numbered 1..KMaxEndpointsPerClient,
	// and virtual 0 is real 0):
	// -- Ep0 OUT
	iEp0MaxPacketSize = caps[0].MaxPacketSize();
	__KTRACE_OPT(KUSB, Kern::Printf("  using Ep0 maxpacketsize of %d bytes", iEp0MaxPacketSize));
	info.iSize = iEp0MaxPacketSize;
	ep = new TUsbcLogicalEndpoint(this, 0, info, NULL, &iRealEndpoints[KEp0_Out]);
	if (!ep)
		{
		goto exit_1;
		}
	__KTRACE_OPT(KUSB, Kern::Printf("  creating ep: mapping real ep %d --> logical ep 0", KEp0_Out));
	iRealEndpoints[KEp0_Out].iLEndpoint = ep;
	// -- Ep0 IN
	info.iDir = KUsbEpDirIn;
	ep = new TUsbcLogicalEndpoint(this, 0, info, NULL, &iRealEndpoints[KEp0_In]);
	if (!ep)
		{
		goto exit_2;
		}
	__KTRACE_OPT(KUSB, Kern::Printf("  creating ep: mapping real ep %d --> logical ep 0", KEp0_In));
	iRealEndpoints[KEp0_In].iLEndpoint = ep;

	// Create the power handler
	iPowerHandler = new DUsbcPowerHandler(this);
	if (!iPowerHandler)
		{
		goto exit_3;
		}
	iPowerHandler->Add();

	// Misc stuff
	iTrackDeviceState = DeviceStateChangeCaps();
	if (!iTrackDeviceState)
		{
		// There shouldn't really be any PSL that doesn't support Device State
		// tracking, but we cannot simply enforce it as we have to preserve
		// backwards compatibility.
		__KTRACE_OPT(KUSB, Kern::Printf("  Warning: USB Device State tracking not supported by PSL"));
		}

	return ETrue;

 exit_3:
	delete iRealEndpoints[KEp0_In].iLEndpoint;
 exit_2:
	delete iRealEndpoints[KEp0_Out].iLEndpoint;
 exit_1:
	iConfigs.ResetAndDestroy();

	return EFalse;
	}


/** The standard constructor for this class.

	@publishedPartner @released
 */
DUsbClientController::DUsbClientController()
	: iEp0ReceivedNonStdRequest(EFalse),
	  iRmWakeupStatus_Enabled(EFalse),
	  iEp0_RxBuf(),
	  iDeviceTotalEndpoints(0),
	  iDeviceUsableEndpoints(0),
	  iDeviceState(EUsbcDeviceStateUndefined),
	  iDeviceStateB4Suspend(EUsbcDeviceStateUndefined),
	  iSelfPowered(EFalse),
	  iRemoteWakeup(EFalse),
	  iTrackDeviceState(EFalse),
	  iHardwareActivated(EFalse),
	  iOtgSupport(EFalse),
	  iOtgHnpHandledByHw(EFalse),
	  iOtgFuncMap(0),
	  iHighSpeed(EFalse),
	  iSetup(),
	  iEp0MaxPacketSize(0),
	  iEp0ClientId(NULL),
	  iEp0DataReceived(0),
	  iEp0DataReceiving(EFalse),
	  iEp0WritePending(EFalse),
	  iEp0ClientDataTransmitting(EFalse),
	  iEp0DeviceControl(NULL),
	  iDescriptors(iEp0_TxBuf),
	  iCurrentConfig(0),
	  iConfigs(1),
	  iRealEndpoints(),
	  iEp0_TxBuf(),
	  iEp0_RxExtraCount(0),
	  iEp0_RxExtraData(EFalse),
	  iEp0_TxNonStdCount(0),
	  iEp0ReadRequestCallbacks(_FOFF(TUsbcRequestCallback, iLink)),
	  iClientCallbacks(_FOFF(TUsbcClientCallback, iLink)),
	  iStatusCallbacks(_FOFF(TUsbcStatusCallback, iLink)),
	  iEpStatusCallbacks(_FOFF(TUsbcEndpointStatusCallback, iLink)),
	  iOtgCallbacks(_FOFF(TUsbcOtgFeatureCallback, iLink)),
	  iReconnectTimer(ReconnectTimerCallback, this),
	  iCableStatusTimer(CableStatusTimerCallback, this),
	  iPowerUpDfc(PowerUpDfc, this, 3),
	  iPowerDownDfc(PowerDownDfc, this, 3),
	  iStandby(EFalse),
#ifdef USB_OTG_CLIENT
	  // In the OTG case the device starts out disabled
	  iStackIsActive(EFalse),
#else
	  iStackIsActive(ETrue),
#endif // USB_OTG_CLIENT
	  iOtgClientConnect(EFalse),
	  iClientSupportReady(EFalse),
	  iDPlusEnabled(EFalse),
	  iUsbResetDeferred(EFalse),
	  iEnablePullUpOnDPlus(NULL),
	  iDisablePullUpOnDPlus(NULL),
	  iOtgContext(NULL)
	{
	__KTRACE_OPT(KUSB, Kern::Printf("DUsbClientController::DUsbClientController()"));

#ifndef SEPARATE_USB_DFC_QUEUE
	iPowerUpDfc.SetDfcQ(Kern::DfcQue0());
  	iPowerDownDfc.SetDfcQ(Kern::DfcQue0());
#endif // SEPARATE_USB_DFC_QUEUE

	for (TInt i = 0; i < KUsbcEpArraySize; i++)
		iRequestCallbacks[i] = NULL;
	}


/** This function gets called by the PSL upon detection of either of the following events:
	- USB Reset,
	- USB Suspend event,
	- USB Resume signalling,
	- The USB cable has been attached (inserted) or detached (removed).

	@param anEvent An enum denoting the event that has occured.

	@return KErrArgument if the event is not recognized, otherwise KErrNone.

	@publishedPartner @released
*/
TInt DUsbClientController::DeviceEventNotification(TUsbcDeviceEvent anEvent)
	{
	__KTRACE_OPT(KUSB, Kern::Printf("DUsbClientController::DeviceEventNotification()"));

	// This function may be called by the PSL from within an ISR -- so we have
	// to take care what we do here (and also in all functions that get called
	// from here).

	switch (anEvent)
		{
	case EUsbEventSuspend:
		return ProcessSuspendEvent();
	case EUsbEventResume:
		return ProcessResumeEvent();
	case EUsbEventReset:
		return ProcessResetEvent();
	case EUsbEventCableInserted:
		return ProcessCableInsertEvent();
	case EUsbEventCableRemoved:
		return ProcessCableRemoveEvent();
		}
	return KErrArgument;
	}


/** This function gets called by the PSL upon completion of a pending data transfer request.

	This function is not to be used for endpoint zero completions (use Ep0RequestComplete instead).

	@param aCallback A pointer to a data transfer request callback structure which was previously passed to
	the PSL in a SetupReadBuffer() or SetupWriteBuffer() call.

	@publishedPartner @released
*/
void DUsbClientController::EndpointRequestComplete(TUsbcRequestCallback* aCallback)
	{
	__KTRACE_OPT(KUSB, Kern::Printf("DUsbClientController::EndpointRequestComplete(%p)", aCallback));

	// This function may be called by the PSL from within an ISR -- so we have
	// to take care what we do here (and also in all functions that get called
	// from here).

	// We don't test aCallback for NULL here (and therefore risk a crash)
	// because the PSL should never give us a NULL argument. If it does it
	// means the PSL is buggy and ought to be fixed.
	ProcessDataTransferDone(*aCallback);
	}


/** This function should be called by the PSL after reception of an Ep0
	SET_FEATURE request with a feature selector of either {b_hnp_enable,
	a_hnp_support, a_alt_hnp_support}, but only when that Setup packet is not
	handed up to the PIL (for instance because it is auto-decoded and
	'swallowed' by the UDC hardware).

	@param aHnpState A bitmask indicating the present state of the three OTG
	feature selectors as follows:

	bit.0 == a_alt_hnp_support
	bit.1 == a_hnp_support
	bit.2 == b_hnp_enable

	@see DUsbClientController::ProcessSetClearDevFeature()

	@publishedPartner @released
*/
void DUsbClientController::HandleHnpRequest(TInt aHnpState)
// This function is called by the PSL from within an ISR -- so we have to take care what we do here
// (and also in all functions that get called from here).
	{
	__KTRACE_OPT(KUSB, Kern::Printf("DUsbClientController::HandleHnpRequest(%d)", aHnpState));

	if (!iOtgSupport)
		{
		__KTRACE_OPT(KPANIC, Kern::Printf("  Error: Request only supported on a OTG device"));
		return;
		}
	if (!(iOtgFuncMap & KUsbOtgAttr_HnpSupp))
		{
		__KTRACE_OPT(KPANIC, Kern::Printf("  Error: Request only valid if OTG device supports HNP"));
		return;
		}
	//	(case KUsbFeature_B_HnpEnable:)
	if (aHnpState & 0x04)
		{
		iOtgFuncMap |= KUsbOtgAttr_B_HnpEnable;
		}
	// (case KUsbFeature_A_HnpSupport:)
	if (aHnpState & 0x02)
		{
		iOtgFuncMap |= KUsbOtgAttr_A_HnpSupport;
		}
	// (case KUsbFeature_A_AltHnpSupport:)
	if (aHnpState & 0x01)
		{
		iOtgFuncMap |= KUsbOtgAttr_A_AltHnpSupport;
		}
	OtgFeaturesNotify();
	}


/** This function gets called by the PSL upon completion of a pending endpoint zero data transfer request.

	@param aRealEndpoint Either 0 for Ep0 OUT (= Read), or 1 for Ep0 IN (= Write).
	@param aCount The number of bytes received or transmitted, respectively.
	@param aError The error status of the completed transfer request. Can be KErrNone if no error, KErrCancel
	if transfer was cancelled, or KErrPrematureEnd if a premature status end was encountered.

	@return KErrNone if no error during transfer completion processing, KErrGeneral if the request was a read &
	a Setup packet was received & the recipient for that packet couldn't be found (invalid packet: Ep0 has been
	stalled), KErrNotFound if the request was a read & the recipient for that packet (Setup or data) _was_
	found - however no read had been set up by that recipient (this case should be used by the PSL to disable
	the Ep0 interrupt at that point and give the LDD time to set up a new Ep0 read; once the 'missing' read
	was set up either Ep0ReceiveProceed or Ep0ReadSetupPktProceed will be called by the PIL).

	@publishedPartner @released
*/
TInt DUsbClientController::Ep0RequestComplete(TInt aRealEndpoint, TInt aCount, TInt aError)
	{
	__KTRACE_OPT(KUSB, Kern::Printf("DUsbClientController::Ep0RequestComplete(%d)", aRealEndpoint));

	// This function may be called by the PSL from within an ISR -- so we have
	// to take care what we do here (and also in all functions that get called
	// from here).

	__ASSERT_DEBUG((aRealEndpoint < 2), Kern::Fault(KUsbPILPanicCat, __LINE__));
	if (aError != KErrNone && aError != KErrPrematureEnd)
		{
		__KTRACE_OPT(KUSB, Kern::Printf(" Error: Ep0 request failed (code %d). "
										"Setting up new Read request.", aError));
		if (aRealEndpoint == KEp0_Rx)
			{
			__KTRACE_OPT(KUSB, Kern::Printf(" (RX request failed)"));
			StallEndpoint(KEp0_Out);
			}
		else
			{
			__KTRACE_OPT(KUSB, Kern::Printf(" (TX request failed)"));
			iEp0WritePending = EFalse;
			StallEndpoint(KEp0_In);
			}
		// our only remedy: set up a new read request
		SetupEndpointZeroRead();
		return KErrNone;
		}
	TInt r;
	if (aRealEndpoint & 0x01)
		{
		r = ProcessEp0TransmitDone(aCount, aError);
		}
	else
		{
		r = ProcessEp0ReceiveDone(aCount);
		if (r == KErrNotFound)
			{
			// Don't set up new read yet if data weren't delivered.
			// (The PSL is supposed, upon encountering this return value,
			//  to turn off Ep0's interrupt.)
			return r;
			}
		}
	if (iEp0WritePending == EFalse)
		{
		// we're done & no write request has been set up.
		// so: setup an Ep0 read again
		__KTRACE_OPT(KUSB, Kern::Printf(" Setting up new Ep0 read request."));
		SetupEndpointZeroRead();
		}
	return r;
	}


/** This function should be called by the PSL once the UDC (and thus the USB device) is in the Address state.

	@publishedPartner @released
*/
void DUsbClientController::MoveToAddressState()
	{
	__KTRACE_OPT(KUSB, Kern::Printf("DUsbClientController::MoveToAddressState()"));

	// This function may be called by the PSL from within an ISR -- so we have
	// to take care what we do here (and also in all functions that get called
	// from here).

	NextDeviceState(EUsbcDeviceStateAddress);
	}


/** This function should be called by the PSL before certain UDC operations to inform the power model about
	the electrical current requirements.

	(The exact use of this function is currently not quite clear, so not calling it probably won't harm.)

	@param aCurrent The required electrical current.

	@publishedPartner @released
*/
void DUsbClientController::SetCurrent(TInt aCurrent)
	{
	__KTRACE_OPT(KUSB, Kern::Printf("DUsbClientController::SetCurrent(%d)", aCurrent));

	// Not much for the moment... (What should we do here?)
	return;
	}


//
// === Platform Specific Layer (PSL) - private/virtual =============================================
//

TInt DUsbClientController::OpenDmaChannel(TInt aRealEndpoint)
	{
	__KTRACE_OPT(KUSB, Kern::Printf("DUsbClientController::OpenDmaChannel(%d)", aRealEndpoint));
	return KErrNone;
	}


void DUsbClientController::CloseDmaChannel(TInt aRealEndpoint)
	{
	__KTRACE_OPT(KUSB, Kern::Printf("DUsbClientController::CloseDmaChannel(%d)", aRealEndpoint));
	}


TBool DUsbClientController::CableDetectWithoutPowerCaps() const
	{
	__KTRACE_OPT(KUSB, Kern::Printf("DUsbClientController::CableDetectWithoutPowerCaps()"));
	// Should be overridden in PSL if applicable.
	return EFalse;
	}


TBool DUsbClientController::DeviceHighSpeedCaps() const
	{
	__KTRACE_OPT(KUSB, Kern::Printf("DUsbClientController::DeviceHighSpeedCaps()"));
	// Should be overridden in PSL if applicable.
	return EFalse;
	}


TBool DUsbClientController::DeviceResourceAllocV2Caps() const
	{
	__KTRACE_OPT(KUSB, Kern::Printf("DUsbClientController::DeviceResourceAllocV2Caps()"));
	// Should be overridden in PSL if applicable.
	return EFalse;
	}


TBool DUsbClientController::DeviceHnpHandledByHardwareCaps() const
	{
	__KTRACE_OPT(KUSB, Kern::Printf("DUsbClientController::DeviceHnpHandledByHardwareCaps()"));
	// Should be overridden in PSL if applicable.
	return EFalse;
	}


TInt DUsbClientController::EnterTestMode(TInt aTestSelector)
	{
	__KTRACE_OPT(KUSB, Kern::Printf("DUsbClientController::EnterTestMode(%d)", aTestSelector));
	// Should be overridden in PSL if applicable.
	return KErrNotSupported;
	}


TBool DUsbClientController::PowerDownWhenActive() const
	{
 	__KTRACE_OPT(KUSB, Kern::Printf("DUsbClientController::PowerDownWhenActive()"));
 	return EFalse;
	}


TInt DUsbClientController::PowerDown()
	{
 	__KTRACE_OPT(KUSB, Kern::Printf("DUsbClientController::PowerDown()"));
 	return KErrNone;
	}


TInt DUsbClientController::PowerUp()
	{
 	__KTRACE_OPT(KUSB, Kern::Printf("DUsbClientController::PowerUp()"));
 	return KErrNone;
	}


TInt DUsbClientController::OtgEnableUdc()
	{
 	__KTRACE_OPT(KUSB, Kern::Printf("DUsbClientController::OtgEnableUdc()"));
 	return KErrNone;
	}


TInt DUsbClientController::OtgDisableUdc()
   	{
 	__KTRACE_OPT(KUSB, Kern::Printf("DUsbClientController::OtgDisableUdc()"));
 	return KErrNone;
	}


//
// === USB Controller member function implementations - Internal utility functions (private) =======
//

TInt DUsbClientController::DeRegisterClientCallback(const DBase* aClientId)
    {
	__KTRACE_OPT(KUSB, Kern::Printf("DUsbClientController::DeRegisterClientCallback()"));
	__ASSERT_DEBUG((aClientId != NULL), Kern::Fault(KUsbPILPanicCat, __LINE__));
	TSglQueIter<TUsbcClientCallback> iter(iClientCallbacks);
	TUsbcClientCallback* p;
	while ((p = iter++) != NULL)
		if (p->Owner() == aClientId)
			{
			__KTRACE_OPT(KUSB, Kern::Printf("  removing ClientCallback @ 0x%x", p));
			iClientCallbacks.Remove(*p);
			return KErrNone;
			}
	__KTRACE_OPT(KUSB, Kern::Printf("  Client not found"));
	return KErrNotFound;
    }


TBool DUsbClientController::CheckEpAvailability(TInt aEndpointsUsed,
												const TUsbcEndpointInfoArray& aEndpointData,
												TInt aIfcNumber) const
	{
	__KTRACE_OPT(KUSB, Kern::Printf("DUsbClientController::CheckEpAvailability()"));
	if (aEndpointsUsed > KMaxEndpointsPerClient)
		{
		__KTRACE_OPT(KPANIC, Kern::Printf("  Error: too many endpoints claimed (%d)", aEndpointsUsed));
		return EFalse;
		}
	TBool reserve[KUsbcEpArraySize]; // iDeviceTotalEndpoints can be equal to 32
	memset(reserve, EFalse, sizeof(reserve));				// reset the array
	for (TInt i = 0; i < aEndpointsUsed; ++i)
		{
		__KTRACE_OPT(KUSB, Kern::Printf("  checking for (user) endpoint #%d availability...", i + 1));
		TInt j = 2;
		while (j < iDeviceTotalEndpoints)
			{
			if ((iRealEndpoints[j].EndpointSuitable(&aEndpointData[i], aIfcNumber)) &&
				(reserve[j] == EFalse))
				{
				__KTRACE_OPT(KUSB, Kern::Printf("  ---> found suitable endpoint: RealEndpoint #%d", j));
				reserve[j] = ETrue;							// found one: mark this ep as reserved
				break;
				}
			__KTRACE_OPT(KUSB, Kern::Printf("  -> endpoint not suitable: RealEndpoint #%d", j));
			j++;
			}
		if (j == iDeviceTotalEndpoints)
			{
			return EFalse;
			}
		}
	return ETrue;
	}


TUsbcInterface* DUsbClientController::CreateInterface(const DBase* aClientId, TInt aIfc, TUint32 aFeatureWord)
// We know that 9.2.3 says: "Interfaces are numbered from zero to one less than the number of
// concurrent interfaces supported by the configuration."  But since we permit the user to
// change interface numbers, we can neither assume nor enforce anything about them here.
	{
	__KTRACE_OPT(KUSB, Kern::Printf("DUsbClientController::CreateInterface(x, aIfc=%d)", aIfc));
	TUsbcInterfaceSet* ifcset_ptr = NULL;
	TInt ifcset = ClientId2InterfaceNumber(aClientId);
	TBool new_ifc;
	if (ifcset < 0)
		{
		// New interface(set), so we need to find a number for it.
		new_ifc = ETrue;
		const TInt num_ifcsets = iConfigs[0]->iInterfaceSets.Count();
		if (num_ifcsets == 255)
			{
			__KTRACE_OPT(KPANIC, Kern::Printf("  Error: Too many interfaces already exist: 255"));
			return NULL;
			}
		// Find the smallest interface number that has not yet been used.
		for (ifcset = 0; ifcset < 256; ++ifcset)
			{
			TBool n_used = EFalse;
			for (TInt i = 0; i < num_ifcsets; ++i)
				{
				if ((iConfigs[0]->iInterfaceSets[i]->iInterfaceNumber) == ifcset)
					{
					__KTRACE_OPT(KUSB, Kern::Printf("  interface number %d already used", ifcset));
					n_used = ETrue;
					break;
					}
				}
			if (!n_used)
				{
				break;
				}
			}
		if (ifcset == 256)
			{
			__KTRACE_OPT(KPANIC, Kern::Printf("  Error: no available interface number found"));
			return NULL;
			}
		// append the ifcset
		__KTRACE_OPT(KUSB, Kern::Printf("  creating new InterfaceSet %d first", ifcset));
		if (aIfc != 0)
			{
			__KTRACE_OPT(KPANIC, Kern::Printf("  Error: invalid interface setting number (1): %d", aIfc));
			return NULL;
			}
		if ((ifcset_ptr = new TUsbcInterfaceSet(aClientId, ifcset)) == NULL)
			{
			__KTRACE_OPT(KPANIC,
						 Kern::Printf("  Error: new TUsbcInterfaceSet(aClientId, ifcset_num) failed"));
			return NULL;
			}
		iConfigs[0]->iInterfaceSets.Append(ifcset_ptr);
		}
	else /* if (ifcset_num >= 0) */
		{
		// use an existent ifcset
		new_ifc = EFalse;
		__KTRACE_OPT(KUSB, Kern::Printf("  using existing InterfaceSet %d", ifcset));
		ifcset_ptr = InterfaceNumber2InterfacePointer(ifcset);
		if (aIfc != ifcset_ptr->iInterfaces.Count())
			{
			// 9.2.3: "Alternate settings range from zero to one less than the number of alternate
			// settings for a specific interface." (Thus we can here only append a setting.)
			__KTRACE_OPT(KPANIC, Kern::Printf("  Error: invalid interface setting number (2): %d", aIfc));
			return NULL;
			}
		// Check whether the existing interface belongs indeed to this client
		if (ifcset_ptr->iClientId != aClientId)
			{
			__KTRACE_OPT(KPANIC, Kern::Printf("  Error: iClientId (%p) != aClientId (%p)",
											  ifcset_ptr->iClientId, aClientId));
			return NULL;
			}
		}
	const TBool no_ep0_requests = aFeatureWord & KUsbcInterfaceInfo_NoEp0RequestsPlease;
	TUsbcInterface* const ifc_ptr = new TUsbcInterface(ifcset_ptr, aIfc, no_ep0_requests);
	if (!ifc_ptr)
		{
		__KTRACE_OPT(KPANIC, Kern::Printf("  Error: new TUsbcInterface(ifcset, aIfc) failed"));
		if (new_ifc)
			{
			DeleteInterfaceSet(ifcset);
			}
		return NULL;
		}
	ifcset_ptr->iInterfaces.Append(ifc_ptr);
	return ifc_ptr;
	}


#define RESET_SETTINGRESERVE \
	for (TInt i = start_ep; i < iDeviceTotalEndpoints; i++) \
		{ \
		if (iRealEndpoints[i].iSettingReserve) \
			iRealEndpoints[i].iSettingReserve = EFalse; \
		} \

TInt DUsbClientController::CreateEndpoints(TUsbcInterface* aIfc, TInt aEndpointsUsed,
										   const TUsbcEndpointInfoArray& aEndpointData,
										   TInt aRealEpNumbers[])
	{
	__KTRACE_OPT(KUSB, Kern::Printf("DUsbClientController::CreateEndpoints()"));
	const TInt ifc_num = aIfc->iInterfaceSet->iInterfaceNumber;
	const TInt start_ep = 2;
	for (TInt i = 0; i < aEndpointsUsed; ++i)
		{
		for (TInt j = start_ep; j < iDeviceTotalEndpoints; ++j)
			{
			if (iRealEndpoints[j].EndpointSuitable(&aEndpointData[i], ifc_num))
				{
				// Logical endpoints are numbered 1..KMaxEndpointsPerClient (virtual 0 is real 0 and 1)
				TUsbcLogicalEndpoint* const ep = new TUsbcLogicalEndpoint(this, i + 1, aEndpointData[i],
																		  aIfc, &iRealEndpoints[j]);
				if (!ep)
					{
					__KTRACE_OPT(KPANIC, Kern::Printf("  Error: new TUsbcLogicalEndpoint() failed"));
					aIfc->iEndpoints.ResetAndDestroy();
					RESET_SETTINGRESERVE;
					return KErrNoMemory;
					}
				aIfc->iEndpoints.Append(ep);
				// Check on logical endpoint's sizes for compliance with special restrictions.
				if (aIfc->iSettingCode == 0)
					{
					// For details see last paragraph of 5.7.3 "Interrupt Transfer Packet Size Constraints".
					if ((ep->iInfo.iType == KUsbEpTypeInterrupt) && (ep->iEpSize_Hs > 64))
						{
						__KTRACE_OPT(KPANIC, Kern::Printf("  Warning: INT ep HS size = %d on default ifc setting",
														  ep->iEpSize_Hs));
						__KTRACE_OPT(KPANIC, Kern::Printf("           (should be <= 64)"));
						}
					// For details see last paragraph of 5.6.3 "Isochronous Transfer Packet Size Constraints".
					else if ((ep->iInfo.iType == KUsbEpTypeIsochronous) && (ep->iInfo.iSize > 0))
						{
						__KTRACE_OPT(KPANIC, Kern::Printf("  Warning: ISO ep size = %d on default ifc setting",
														  ep->iInfo.iSize));
						__KTRACE_OPT(KPANIC, Kern::Printf("           (should be zero or ep non-existent)"));
						}
					}
				// If the endpoint doesn't support DMA (now or never) the next operation
				// will be a successful no-op.
				const TInt r = OpenDmaChannel(j);
				if (r != KErrNone)
					{
					__KTRACE_OPT(KPANIC, Kern::Printf("  Error: Opening of DMA channel failed"));
					aIfc->iEndpoints.ResetAndDestroy();
					RESET_SETTINGRESERVE;
					return r;
					}
				__KTRACE_OPT(KUSB, Kern::Printf("  creating ep: mapping real ep %d -> logical ep %d",
												j, i + 1));
				iRealEndpoints[j].iIfcNumber = &aIfc->iInterfaceSet->iInterfaceNumber;
				iRealEndpoints[j].iSettingReserve = ETrue;
				__KTRACE_OPT(KUSB,
							 Kern::Printf("  ep->iInfo: iType=0x%x iDir=0x%x iSize=%d iInterval=%d",
										  ep->iInfo.iType, ep->iInfo.iDir, ep->iInfo.iSize,
										  ep->iInfo.iInterval));
				__KTRACE_OPT(KUSB,
							 Kern::Printf("  ep->iInfo: iInterval_Hs=%d iTransactions=%d iExtra=%d",
										  ep->iInfo.iInterval_Hs, ep->iInfo.iTransactions,
										  ep->iInfo.iExtra));
				// Store real endpoint numbers:
				// array[x] holds the number for logical ep x.
				aRealEpNumbers[i + 1] = j;
				break;
				}
			}
		}
	aRealEpNumbers[0] = 0;								// ep0: 0.
	__KTRACE_OPT(KUSB,{
		Kern::Printf("  Endpoint Mapping for Interface %d / Setting %d:", ifc_num, aIfc->iSettingCode);
		Kern::Printf("Logical  | Real");
		Kern::Printf("Endpoint | Endpoint");
		for (TInt ep = 0; ep <= aEndpointsUsed; ++ep) Kern::Printf("   %2d       %3d",ep, aRealEpNumbers[ep]);
		});
	RESET_SETTINGRESERVE;
	return KErrNone;
	}


TInt DUsbClientController::SetupIfcDescriptor(TUsbcInterface* aIfc, TUsbcClassInfo& aClass, DThread* aThread,
											  TDesC8* aString, const TUsbcEndpointInfoArray& aEndpointData)
	{
	__KTRACE_OPT(KUSB, Kern::Printf("DUsbClientController::SetupIfcDescriptor()"));

	// Interface descriptor
	TUsbcDescriptorBase* d = TUsbcInterfaceDescriptor::New(aIfc->iInterfaceSet->iInterfaceNumber,
														   aIfc->iSettingCode,
														   aIfc->iEndpoints.Count(),
														   aClass);
	if (!d)
		{
		__KTRACE_OPT(KPANIC, Kern::Printf("  Error: Memory allocation for ifc desc failed."));
		return KErrNoMemory;
		}
	iDescriptors.InsertDescriptor(d);

	// Interface string descriptor
	if (aString)
		{
		// we don't know the length of the string, so we have to allocate memory dynamically
		TUint strlen = Kern::ThreadGetDesLength(aThread, aString);
		if (strlen > KUsbStringDescStringMaxSize)
			{
			__KTRACE_OPT(KPANIC, Kern::Printf("  Warning: $ descriptor too long - string will be truncated"));
			strlen = KUsbStringDescStringMaxSize;
			}
		HBuf8* const stringbuf = HBuf8::New(strlen);
		if (!stringbuf)
			{
			__KTRACE_OPT(KPANIC, Kern::Printf("  Error: Memory allocation for ifc $ desc string failed."));
			iDescriptors.DeleteIfcDescriptor(aIfc->iInterfaceSet->iInterfaceNumber,
											 aIfc->iSettingCode);
			return KErrNoMemory;
			}
		stringbuf->SetMax();
		// the aString points to data that lives in user memory, so we have to copy it:
		TInt r = Kern::ThreadDesRead(aThread, aString, *stringbuf, 0);
		if (r != KErrNone)
			{
			__KTRACE_OPT(KPANIC, Kern::Printf("  Error: Thread read error"));
			iDescriptors.DeleteIfcDescriptor(aIfc->iInterfaceSet->iInterfaceNumber,
											 aIfc->iSettingCode);
			delete stringbuf;
			return r;
			}
		TUsbcStringDescriptor* const sd = TUsbcStringDescriptor::New(*stringbuf);
		if (!sd)
			{
			__KTRACE_OPT(KPANIC, Kern::Printf("  Error: Memory allocation for ifc $ desc failed."));
			iDescriptors.DeleteIfcDescriptor(aIfc->iInterfaceSet->iInterfaceNumber,
											 aIfc->iSettingCode);
			delete stringbuf;
			return KErrNoMemory;
			}
		iDescriptors.SetIfcStringDescriptor(sd, aIfc->iInterfaceSet->iInterfaceNumber, aIfc->iSettingCode);
		delete stringbuf;									// the (EPOC) descriptor was copied by New()
		}

	// Endpoint descriptors
	for (TInt i = 0; i < aIfc->iEndpoints.Count(); ++i)
		{
		// The reason for using another function argument for Endpoint Info
		// (and not possibly - similar to the Endpoint Address -
		// "aIfc->iEndpoints[i]->iPEndpoint->iLEndpoint->iInfo") is that this time
		// there are no logical endpoints associated with our real endpoints,
		// i.e. iLEndpoint is NULL!.
		if (aEndpointData[i].iExtra)
			{
			// if a non-standard endpoint descriptor is requested...
			if (aEndpointData[i].iExtra != 2)
				{
				// ...then it must be a Audio Class endpoint descriptor. Else...
				__KTRACE_OPT(KPANIC, Kern::Printf("  Error: EP desc extension > 2 bytes (%d)",
												  aEndpointData[i].iExtra));
				iDescriptors.DeleteIfcDescriptor(aIfc->iInterfaceSet->iInterfaceNumber,
												 aIfc->iSettingCode);
				return KErrArgument;
				}
			d = TUsbcAudioEndpointDescriptor::New(aIfc->iEndpoints[i]->iPEndpoint->iEndpointAddr,
												  aEndpointData[i]);
			}
		else
			{
			d = TUsbcEndpointDescriptor::New(aIfc->iEndpoints[i]->iPEndpoint->iEndpointAddr,
											 aEndpointData[i]);
			}
		if (!d)
			{
			__KTRACE_OPT(KPANIC, Kern::Printf("  Error: Memory allocation for ep desc #%d failed.", i));
			iDescriptors.DeleteIfcDescriptor(aIfc->iInterfaceSet->iInterfaceNumber,
											 aIfc->iSettingCode);
			return KErrNoMemory;
			}
		iDescriptors.InsertDescriptor(d);
		}

	return KErrNone;
	}


TInt DUsbClientController::ClientId2InterfaceNumber(const DBase* aClientId) const
	{
	const TInt num_ifcsets = iConfigs[0]->iInterfaceSets.Count();
	for (TInt i = 0; i < num_ifcsets; ++i)
		{
		if (iConfigs[0]->iInterfaceSets[i]->iClientId == aClientId)
			{
			return iConfigs[0]->iInterfaceSets[i]->iInterfaceNumber;
			}
		}
	return -1;
	}


TUsbcInterfaceSet* DUsbClientController::ClientId2InterfacePointer(const DBase* aClientId) const
	{
	const TInt num_ifcsets = iConfigs[0]->iInterfaceSets.Count();
	for (TInt i = 0; i < num_ifcsets; ++i)
		{
		if (iConfigs[0]->iInterfaceSets[i]->iClientId == aClientId)
			{
			return iConfigs[0]->iInterfaceSets[i];
			}
		}
	return NULL;
	}


const DBase* DUsbClientController::InterfaceNumber2ClientId(TInt aIfcSet) const
	{
	if (!InterfaceExists(aIfcSet))
		{
		return NULL;
		}
	return InterfaceNumber2InterfacePointer(aIfcSet)->iClientId;
	}


TUsbcInterfaceSet* DUsbClientController::InterfaceNumber2InterfacePointer(TInt aIfcSet) const
	{
	const TInt num_ifcsets = iConfigs[0]->iInterfaceSets.Count();
	for (TInt i = 0; i < num_ifcsets; ++i)
		{
		if ((iConfigs[0]->iInterfaceSets[i]->iInterfaceNumber) == aIfcSet)
			{
			return iConfigs[0]->iInterfaceSets[i];
			}
		}
	return NULL;
	}


TInt DUsbClientController::ActivateHardwareController()
	{
	__KTRACE_OPT(KUSB, Kern::Printf("DUsbClientController::ActivateHardwareController()"));
	if (iHardwareActivated)
		{
		__KTRACE_OPT(KUSB, Kern::Printf("  already active -> returning"));
		return KErrNone;
		}
	// Initialise HW
	TInt r = StartUdc();
	if (r != KErrNone)
		{
		__KTRACE_OPT(KPANIC, Kern::Printf("  Error: StartUdc() failed"));
		return KErrHardwareNotAvailable;
		}
	r = OtgEnableUdc();							   // turn on UDC (OTG flavour)
	if (r != KErrNone)
		{
		__KTRACE_OPT(KPANIC, Kern::Printf("  Error: OtgEnableUdc() failed: %d", r));
		}
	iHardwareActivated = ETrue;

	// Configure & enable endpoint zero
	const TUsbcLogicalEndpoint* const ep0_0 = iRealEndpoints[0].iLEndpoint;
	const TUsbcLogicalEndpoint* const ep0_1 = iRealEndpoints[1].iLEndpoint;
	if (iHighSpeed)
		{
		const_cast<TUsbcLogicalEndpoint*>(ep0_0)->iInfo.iSize = ep0_0->iEpSize_Hs;
		const_cast<TUsbcLogicalEndpoint*>(ep0_1)->iInfo.iSize = ep0_1->iEpSize_Hs;
		}
	else
		{
		const_cast<TUsbcLogicalEndpoint*>(ep0_0)->iInfo.iSize = ep0_0->iEpSize_Fs;
		const_cast<TUsbcLogicalEndpoint*>(ep0_1)->iInfo.iSize = ep0_1->iEpSize_Fs;
		}
	ConfigureEndpoint(0, ep0_0->iInfo);
	ConfigureEndpoint(1, ep0_1->iInfo);
	iEp0MaxPacketSize = ep0_0->iInfo.iSize;

	__KTRACE_OPT(KUSB, Kern::Printf("  Controller activated."));
	if (UsbConnectionStatus())
		{
		if (iDeviceState == EUsbcDeviceStateUndefined)
			{
			NextDeviceState(EUsbcDeviceStateAttached);
			}
		NextDeviceState(EUsbcDeviceStatePowered);
		}
	return  KErrNone;;
	}


void DUsbClientController::DeActivateHardwareController()
	{
	__KTRACE_OPT(KUSB, Kern::Printf("DUsbClientController::DeActivateHardwareController()"));
	if (!iHardwareActivated)
		{
		__KTRACE_OPT(KUSB, Kern::Printf("  not active -> returning"));
		return;
		}
	// Deconfigure & disable endpoint zero
	DeConfigureEndpoint(KEp0_Out);
	DeConfigureEndpoint(KEp0_In);
	// Stop HW
	TInt r = OtgDisableUdc();					  // turn off UDC (OTG flavour)
	if (r != KErrNone)
		{
		__KTRACE_OPT(KPANIC, Kern::Printf("  Error: OtgDisableUdc() failed: %d", r));
		}
	StopUdc();
	iHardwareActivated = EFalse;
	__KTRACE_OPT(KUSB, Kern::Printf("  Controller deactivated."));
	if (UsbConnectionStatus())
		{
		NextDeviceState(EUsbcDeviceStateAttached);
		}
	return;
	}


void DUsbClientController::DeleteInterfaceSet(TInt aIfcSet)
	{
	__KTRACE_OPT(KUSB, Kern::Printf("DUsbClientController::DeleteInterfaceSet(%d)", aIfcSet));
	TUsbcInterfaceSet* const ifcset_ptr = InterfaceNumber2InterfacePointer(aIfcSet);
	if (!ifcset_ptr)
		{
		__KTRACE_OPT(KPANIC, Kern::Printf("  Error: invalid interface number: %d", aIfcSet));
		return;
		}
	const TInt idx = iConfigs[0]->iInterfaceSets.Find(ifcset_ptr);
	if (idx == KErrNotFound)
		{
		__KTRACE_OPT(KPANIC, Kern::Printf("  Error: interface not found in array"));
		return;
		}
	iConfigs[0]->iInterfaceSets.Remove(idx);
	delete ifcset_ptr;
	}


void DUsbClientController::DeleteInterface(TInt aIfcSet, TInt aIfc)
	{
	__KTRACE_OPT(KUSB, Kern::Printf("DUsbClientController::DeleteInterface(%d, %d)", aIfcSet, aIfc));
	TUsbcInterfaceSet* const ifcset_ptr = InterfaceNumber2InterfacePointer(aIfcSet);
	if (!ifcset_ptr)
		{
		__KTRACE_OPT(KPANIC, Kern::Printf("  Error: invalid interface number: %d", aIfcSet));
		return;
		}
	if (ifcset_ptr->iInterfaces.Count() <= aIfc)
		{
		__KTRACE_OPT(KPANIC, Kern::Printf("  Error: invalid interface setting: %d", aIfc));
		return;
		}
	TUsbcInterface* const ifc_ptr = ifcset_ptr->iInterfaces[aIfc];
	// Always first remove, then delete (see ~TUsbcLogicalEndpoint() for the reason why)
	ifcset_ptr->iInterfaces.Remove(aIfc);
	delete ifc_ptr;
	if (aIfc == ifcset_ptr->iCurrentInterface)
		{
		__KTRACE_OPT(KUSB, Kern::Printf(" > Warning: deleting current interface setting"));
		ifcset_ptr->iCurrentInterface = 0;
		}
	}


void DUsbClientController::CancelTransferRequests(TInt aRealEndpoint)
	{
	__KTRACE_OPT(KUSB, Kern::Printf("DUsbClientController::CancelTransferRequests(aRealEndpoint=%d)",
									aRealEndpoint));
	const DBase* const clientId = PEndpoint2ClientId(aRealEndpoint);
	if (EpIdx2Addr(aRealEndpoint) & KUsbEpAddress_In)
		{
		CancelWriteBuffer(clientId, aRealEndpoint);
		}
	else
		{
		CancelReadBuffer(clientId, aRealEndpoint);
		}
	}


void DUsbClientController::DeleteRequestCallback(const DBase* aClientId, TInt aEndpointNum,
												 TTransferDirection aTransferDir)
	{
	__KTRACE_OPT(KUSB, Kern::Printf("DUsbClientController::DeleteRequestCallback()"));
	// Ep0 OUT
	if (aEndpointNum == 0)
		{
		const TInt irq = NKern::DisableAllInterrupts();
		TSglQueIter<TUsbcRequestCallback> iter(iEp0ReadRequestCallbacks);
		TUsbcRequestCallback* p;
		while ((p = iter++) != NULL)
			{
			if (p->Owner() == aClientId)
				{
				__ASSERT_DEBUG((p->iRealEpNum == 0), Kern::Fault(KUsbPILPanicCat, __LINE__));
				__ASSERT_DEBUG((p->iTransferDir == EControllerRead), Kern::Fault(KUsbPILPanicCat, __LINE__));
				__KTRACE_OPT(KUSB, Kern::Printf("  removing RequestCallback @ 0x%x (ep0)", p));
				iEp0ReadRequestCallbacks.Remove(*p);
				}
			}
		NKern::RestoreInterrupts(irq);
		return;
		}
	// Other endpoints
	TUsbcRequestCallback* const p = iRequestCallbacks[aEndpointNum];
	if (p)
		{
 		__ASSERT_DEBUG((p->Owner() == aClientId), Kern::Fault(KUsbPILPanicCat, __LINE__));
		__ASSERT_DEBUG((p->iTransferDir == aTransferDir), Kern::Fault(KUsbPILPanicCat, __LINE__));
		__KTRACE_OPT(KUSB, Kern::Printf("  removing RequestCallback @ 0x%x", p));
		iRequestCallbacks[aEndpointNum] = NULL;
		}
	}


void DUsbClientController::DeleteRequestCallbacks(const DBase* aClientId)
	{
	// aClientId being NULL means: delete all requests for *all* clients.
	__KTRACE_OPT(KUSB, Kern::Printf("DUsbClientController::DeleteRequestCallbacks()"));
	// Ep0 OUT
	const TInt irq = NKern::DisableAllInterrupts();
	TSglQueIter<TUsbcRequestCallback> iter(iEp0ReadRequestCallbacks);
	TUsbcRequestCallback* p;
	while ((p = iter++) != NULL)
		{
		if (!aClientId || p->Owner() == aClientId)
			{
			__KTRACE_OPT(KUSB, Kern::Printf("  removing RequestCallback @ 0x%x (ep0)", p));
			iEp0ReadRequestCallbacks.Remove(*p);
			}
		}
	NKern::RestoreInterrupts(irq);
	// Other endpoints
	for (TInt i = 1; i < KUsbcEpArraySize; i++)
		{
		TUsbcRequestCallback* const p = iRequestCallbacks[i];
		if (p && (!aClientId || p->Owner() == aClientId))
			{
			__KTRACE_OPT(KUSB, Kern::Printf("  removing RequestCallback @ 0x%x", p));
			iRequestCallbacks[i] = NULL;
			}
		}
	}


void DUsbClientController::StatusNotify(TUsbcDeviceState aState, const DBase* aClientId)
	{
	__KTRACE_OPT(KUSB, Kern::Printf("DUsbClientController::StatusNotify()"));

	// This function may be called by the PSL (via chapter9.cpp) from within an
	// ISR -- so we have to take care what we do here (and also in all
	// functions that get called from here).

	TSglQueIter<TUsbcStatusCallback> iter(iStatusCallbacks);
	TUsbcStatusCallback* p;
	while ((p = iter++) != NULL)
		{
		if (!aClientId || aClientId == p->Owner())
			{
			__KTRACE_OPT(KUSB, Kern::Printf("  notifying LDD @ 0x%x about %d", p->Owner(), aState));
			p->SetState(aState);
			p->DoCallback();
			}
		}
	}


void DUsbClientController::EpStatusNotify(TInt aRealEndpoint)
	{
	__KTRACE_OPT(KUSB, Kern::Printf("DUsbClientController::EpStatusNotify()"));

	// This function may be called by the PSL (via chapter9.cpp) from within an
	// ISR -- so we have to take care what we do here (and also in all
	// functions that get called from here).

	const DBase* const client_id = PEndpoint2ClientId(aRealEndpoint);
	if (!client_id)
		{
		__KTRACE_OPT(KPANIC, Kern::Printf("  Error: Client not found for real ep %d", aRealEndpoint));
		return;
		}
	// Check if there is a notification request queued for that client (if not, we can return here).
	TSglQueIter<TUsbcEndpointStatusCallback> iter(iEpStatusCallbacks);
	TUsbcEndpointStatusCallback* p;
	while ((p = iter++) != NULL)
		{
		if (p->Owner() == client_id)
			{
			break;
			}
		}
	if (!p)
		{
		__KTRACE_OPT(KUSB, Kern::Printf("  No notification request for that client, returning"));
		return;
		}
	const TInt ifcset = ClientId2InterfaceNumber(client_id);
	if (ifcset < 0)
		{
		__KTRACE_OPT(KPANIC, Kern::Printf("  Error: Ifcset not found for clientid %d", client_id));
		return;
		}
	const TUsbcInterfaceSet* const ifcset_ptr = InterfaceNumber2InterfacePointer(ifcset);
	if (!ifcset_ptr)
		{
		__KTRACE_OPT(KPANIC, Kern::Printf("  Error: Ifcset pointer not found for ifcset %d", ifcset));
		return;
		}
	const TUsbcInterface* const ifc_ptr = ifcset_ptr->CurrentInterface();
	if (!ifc_ptr)
		{
		__KTRACE_OPT(KPANIC, Kern::Printf("  Error: Current ifc pointer not found for ifcset %d", ifcset));
		return;
		}
	TUint state = 0;
	const TInt eps = ifc_ptr->iEndpoints.Count();
	for (TInt i = 0; i < eps; i++)
		{
		const TUsbcLogicalEndpoint* const ep_ptr = ifc_ptr->iEndpoints[i];
		__KTRACE_OPT(KUSB, Kern::Printf("  checking logical ep #%d for stall state...",
										ep_ptr->iLEndpointNum));
		if (ep_ptr->iPEndpoint->iHalt)
			{
			__KTRACE_OPT(KUSB, Kern::Printf("  -- stalled"));
			// set the bit n to 1, where n is the logical endpoint number minus one
			state |= (1 << (ep_ptr->iLEndpointNum - 1));
			}
		else
			{
			__KTRACE_OPT(KUSB, Kern::Printf("  -- not stalled"));
			}
		}
	__KTRACE_OPT(KUSB, Kern::Printf(" passing ep state 0x%x on to LDD @ 0x%x", state, client_id));
	p->SetState(state);
	p->DoCallback();
	}


void DUsbClientController::OtgFeaturesNotify()
	{
	__KTRACE_OPT(KUSB, Kern::Printf("DUsbClientController::OtgFeaturesNotify()"));

	// This function may be called from the PSL (via PIL's chapter9.cpp) from
	// within an ISR -- so we have to take care what we do here (and also in
	// all functions that get called from here).

	TSglQueIter<TUsbcOtgFeatureCallback> iter(iOtgCallbacks);
	TUsbcOtgFeatureCallback* p;
	while ((p = iter++) != NULL)
		{
		p->SetFeatures(iOtgFuncMap & 0x1C);
		p->DoCallback();
		}
	}


void DUsbClientController::RunClientCallbacks()
	{
	__KTRACE_OPT(KUSB, Kern::Printf("DUsbClientController::RunClientCallbacks()"));
	TSglQueIter<TUsbcClientCallback> iter(iClientCallbacks);
	TUsbcClientCallback* p;
	while ((p = iter++) != NULL)
		{
		__KTRACE_OPT(KUSB, Kern::Printf("Callback 0x%x", p));
		p->DoCallback();
		}
	}


void DUsbClientController::ProcessDataTransferDone(TUsbcRequestCallback& aRcb)
	{
	__KTRACE_OPT(KUSB, Kern::Printf("DUsbClientController::ProcessDataTransferDone()"));
	// This piece can only be called in thread context from ProcessEp0DataReceived() /
	// ProcessEp0SetupReceived() via the call to ProcessEp0ReceiveDone() in
	// SetupReadBuffer(), which is guarded by an interrupt lock.
	TInt ep = aRcb.iRealEpNum;
	if (ep == 0)
		{
		if (aRcb.iTransferDir == EControllerRead)
			{
			// Ep0 OUT is special
			iEp0ReadRequestCallbacks.Remove(aRcb);
			}
		else												// EControllerWrite
			{
			// Ep0 IN needs to be adjusted: it's '1' within the PIL.
			ep = KEp0_Tx;
			}
		}
	if (ep > 0)												// not 'else'!
		{
		__ASSERT_DEBUG((iRequestCallbacks[ep] == &aRcb), Kern::Fault(KUsbPILPanicCat, __LINE__));
		__KTRACE_OPT(KUSB, Kern::Printf(" > removing RequestCallback[%d] @ 0x%x", ep, &aRcb));
		iRequestCallbacks[ep] = NULL;
		}
	aRcb.DoCallback();
	}


void DUsbClientController::NextDeviceState(TUsbcDeviceState aNextState)
	{
	__KTRACE_OPT(KUSB, Kern::Printf("DUsbClientController::NextDeviceState()"));
#ifdef _DEBUG
	const char* const states[] = {"Undefined", "Attached", "Powered", "Default",
								  "Address", "Configured", "Suspended"};
	if ((aNextState >= EUsbcDeviceStateUndefined) &&
		(aNextState <= EUsbcDeviceStateSuspended))
		{
		__KTRACE_OPT(KUSB, Kern::Printf("  next device state: %s", states[aNextState]));
		}
	else
		{
		__KTRACE_OPT(KPANIC, Kern::Printf("  Error: Unknown next device state: %d", aNextState));
		}
	// Print a warning when an invalid state transition is detected
	// 'Undefined' is not a state that is mentioned in the USB spec, but
	// that's what we're in once the cable gets pulled (for instance).
	switch (iDeviceState)
		{
	case EUsbcDeviceStateUndefined:
		// valid: Undefined -> Attached
		if (aNextState != EUsbcDeviceStateAttached)
			break;
		goto OK;
	case EUsbcDeviceStateAttached:
		// valid: Attached -> {Undefined, Powered}
		if ((aNextState != EUsbcDeviceStateUndefined) &&
			(aNextState != EUsbcDeviceStatePowered))
			break;
		goto OK;
	case EUsbcDeviceStatePowered:
		// valid: Powered -> {Undefined, Attached, Default, Suspended}
		if ((aNextState != EUsbcDeviceStateUndefined) &&
			(aNextState != EUsbcDeviceStateAttached) &&
			(aNextState != EUsbcDeviceStateDefault)	 &&
			(aNextState != EUsbcDeviceStateSuspended))
			break;
		goto OK;
	case EUsbcDeviceStateDefault:
		// valid: Default -> {Undefined, Powered, Default, Address, Suspended}
		if ((aNextState != EUsbcDeviceStateUndefined) &&
			(aNextState != EUsbcDeviceStatePowered) &&
			(aNextState != EUsbcDeviceStateDefault) &&
			(aNextState != EUsbcDeviceStateAddress) &&
			(aNextState != EUsbcDeviceStateSuspended))
			break;
		goto OK;
	case EUsbcDeviceStateAddress:
		// valid: Address -> {Undefined, Powered, Default, Configured, Suspended}
		if ((aNextState != EUsbcDeviceStateUndefined) &&
			(aNextState != EUsbcDeviceStatePowered) &&
			(aNextState != EUsbcDeviceStateDefault) &&
			(aNextState != EUsbcDeviceStateConfigured) &&
			(aNextState != EUsbcDeviceStateSuspended))
			break;
		goto OK;
	case EUsbcDeviceStateConfigured:
		// valid: Configured -> {Undefined, Powered, Default, Address, Suspended}
		if ((aNextState != EUsbcDeviceStateUndefined) &&
			(aNextState != EUsbcDeviceStatePowered) &&
			(aNextState != EUsbcDeviceStateDefault) &&
			(aNextState != EUsbcDeviceStateAddress) &&
			(aNextState != EUsbcDeviceStateSuspended))
			break;
		goto OK;
	case EUsbcDeviceStateSuspended:
		// valid: Suspended -> {Undefined, Powered, Default, Address, Configured}
		if ((aNextState != EUsbcDeviceStateUndefined) &&
			(aNextState != EUsbcDeviceStatePowered) &&
			(aNextState != EUsbcDeviceStateDefault) &&
			(aNextState != EUsbcDeviceStateAddress) &&
			(aNextState != EUsbcDeviceStateConfigured))
			break;
		goto OK;
	default:
		__KTRACE_OPT(KPANIC, Kern::Printf("  Error: Unknown current device state: %d", iDeviceState));
		goto OK;
		}
	// KUSB only (instead of KPANIC) so as not to worry people too much where
	// a particular h/w regularly enforces invalid (but harmless) transitions
	__KTRACE_OPT(KUSB, Kern::Printf("  Warning: Invalid next state from %s", states[iDeviceState]));
OK:
#endif // _DEBUG

	iDeviceState = aNextState;
	StatusNotify(iDeviceState);
	}


TInt DUsbClientController::ProcessSuspendEvent()
	{
	__KTRACE_OPT(KUSB, Kern::Printf("DUsbClientController::ProcessSuspendEvent()"));
	// A suspend interrupt has been received and needs attention.
	iDeviceStateB4Suspend = iDeviceState;
	// We have to move to the Suspend state immediately (in case it's a genuine Suspend)
	// because 7.1.7.6 says: "The device must actually be suspended, [...] after no more
	// than 10ms of bus inactivity [...]." Assuming we got the interrupt 3ms after the
	// Suspend condition arose, we have now 7ms left.
	NextDeviceState(EUsbcDeviceStateSuspended);
	Suspend();
	// For some reason we get this interrupt also when the USB cable has been pulled.
	// So we want to see if that is the case in order to move to the Undefined state instead.
	// However, instead of immediately checking the status of the USB cable we wait for a
	// short moment (KUsbCableStatusDelay, see top of file), until things have become stable.
	// Then, in the timer callback, we can change the device state once more if necessary.
	iCableStatusTimer.OneShot(KUsbCableStatusDelay);
	return KErrNone;
	}


//
// ISR (from CableStatusTimerCallback)
//
TInt DUsbClientController::ProcessSuspendEventProceed()
	{
	__KTRACE_OPT(KUSB, Kern::Printf("DUsbClientController::ProcessSuspendEventProceed()"));
	if (!UsbConnectionStatus())
		{
		// If we are no longer connected to the bus, we go into Undefined state (from Suspend).
		__KTRACE_OPT(KUSB, Kern::Printf(" > USB cable detached"));
		NextDeviceState(EUsbcDeviceStateUndefined);
		}
	return KErrNone;
	}


TInt DUsbClientController::ProcessResumeEvent()
	{
	__KTRACE_OPT(KUSB, Kern::Printf("DUsbClientController::ProcessResumeEvent()"));
	iCableStatusTimer.Cancel();
	if (iDeviceState == EUsbcDeviceStateSuspended)
		{
		NextDeviceState(iDeviceStateB4Suspend);
		}
	Resume();
	return KErrNone;
	}


TInt DUsbClientController::ProcessResetEvent(TBool aPslUpcall)
	{
	__KTRACE_OPT(KUSB, Kern::Printf("DUsbClientController::ProcessResetEvent()"));

	if (aPslUpcall)
		{
		// Call back into PSL if we're coming from there.
		// Also, do it always, even when PIL processing will be deferred.
		Reset();
		}

#ifdef USB_OTG_CLIENT
	if (iUsbResetDeferred) // implies (iOtgHnpHandledByHw == ETrue)
		{
		__KTRACE_OPT(KUSB, Kern::Printf("  User-side (still) not ready -> returning"));
		return KErrNone;
		}
	else if (iOtgHnpHandledByHw && !iClientSupportReady)
		{
		// Wait with the PIL Reset processing until user-side is ready
		__KTRACE_OPT(KUSB, Kern::Printf("  User-side not ready -> deferring"));
		iUsbResetDeferred = ETrue;
		return KErrNone;
		}
#endif // USB_OTG_CLIENT

	iCableStatusTimer.Cancel();
	if (iDeviceState == EUsbcDeviceStateAttached)
		{
		NextDeviceState(EUsbcDeviceStatePowered);
		}
	// Notify the world. (This will just queue a DFC, so users won't actually be
	// notified before we return. But we change the device state already here so
	// ChangeConfiguration will see the correct one.)
	NextDeviceState(EUsbcDeviceStateDefault);
	// Tear down the current configuration (never called from thread)
	ChangeConfiguration(0);
	// Reset essential vars
	iRmWakeupStatus_Enabled = EFalse;
	ResetEp0DataOutVars();
	iEp0_RxExtraData = EFalse;
	iEp0WritePending = EFalse;
	iEp0ClientDataTransmitting = EFalse;
	// Reset OTG features, leave attributes as is
	iOtgFuncMap &= KUsbOtgAttr_SrpSupp | KUsbOtgAttr_HnpSupp;
	if (iOtgSupport)
		{
		OtgFeaturesNotify();
		}

	// Check whether there's a speed change
	const TBool was_hs = iHighSpeed;
	iHighSpeed = CurrentlyUsingHighSpeed();
	if (!was_hs && iHighSpeed)
		{
		__KTRACE_OPT(KUSB, Kern::Printf("  Moving to High-speed"));
		EnterHighSpeed();
		}
	else if (was_hs && !iHighSpeed)
		{
		__KTRACE_OPT(KUSB, Kern::Printf("  Moving to Full-speed"));
		EnterFullSpeed();
		}

	// Setup initial Ep0 read (SetupEndpointZeroRead never called from thread)
	if (SetupEndpointZeroRead() != KErrNone)
		{
		__KTRACE_OPT(KPANIC, Kern::Printf("  Error: while setting up Ep0 read"));
		return KErrGeneral;
		}

	return KErrNone;
	}


TInt DUsbClientController::ProcessCableInsertEvent()
	{
	__KTRACE_OPT(KUSB, Kern::Printf("DUsbClientController::ProcessCableInsertEvent()"));
#ifdef USB_OTG_CLIENT
	__KTRACE_OPT(KPANIC, Kern::Printf("  Error: EUsbEventCableInsert shouldn't be sent by an OTG Client PSL"));
	return KErrArgument;
#else
	NextDeviceState(EUsbcDeviceStateAttached);
	if (iHardwareActivated)
		{
		NextDeviceState(EUsbcDeviceStatePowered);
		}
	return KErrNone;
#endif	// #ifdef USB_OTG_CLIENT
	}


TInt DUsbClientController::ProcessCableRemoveEvent()
	{
	__KTRACE_OPT(KUSB, Kern::Printf("DUsbClientController::ProcessCableRemoveEvent()"));
#ifdef USB_OTG_CLIENT
	__KTRACE_OPT(KPANIC, Kern::Printf("  Error: EUsbEventCableRemoved shouldn't be sent by an OTG Client PSL"));
	return KErrArgument;
#else
	// Tear down the current configuration (if any)
	ChangeConfiguration(0);
	NextDeviceState(EUsbcDeviceStateUndefined);
	return KErrNone;
#endif	// #ifdef USB_OTG_CLIENT
	}


void DUsbClientController::EnterFullSpeed()
	{
	__KTRACE_OPT(KUSB, Kern::Printf("DUsbClientController::EnterFullSpeed()"));
	iDescriptors.UpdateDescriptorsFs();
	}


void DUsbClientController::EnterHighSpeed()
	{
	__KTRACE_OPT(KUSB, Kern::Printf("DUsbClientController::EnterHighSpeed()"));
	iDescriptors.UpdateDescriptorsHs();
	}


//
// Called whenever either iOtgClientConnect or iClientSupportReady changes value.
//
TInt DUsbClientController::EvaluateOtgConnectFlags()
	{
	__KTRACE_OPT(KUSB, Kern::Printf("DUsbClientController::EvaluateOtgConnectFlags()"));

	TInt r = KErrNone;

	// Check to see if the current flag states result in a change to the
	// need to activate the DPLUS pull-up
	TBool enableDPlus;
	if (!iOtgHnpHandledByHw)
		{
		// the default
		enableDPlus = (iOtgClientConnect && iClientSupportReady);
		}
	else
		{
		// certain h/w: handles HNP connect/disconnect automatically
		__KTRACE_OPT(KUSB, Kern::Printf("  HNP-handling h/w: only considering user-side readiness"));
		enableDPlus = iClientSupportReady;
		}

	if (enableDPlus == iDPlusEnabled)
		{
		return r;
		}

	// There has been a changed requirement that must be serviced...
	if (enableDPlus)
		{
		__KTRACE_OPT(KUSB, Kern::Printf("  calling (*iEnablePullUpOnDPlus)()"));
		if (iEnablePullUpOnDPlus != NULL)
			{
			iDPlusEnabled = enableDPlus;
			// First we move to Suspend state to trigger a state change
			// notification in any case, even if no cable and/or host are
			// connected. The next Reset will get us out of it again.
			iDeviceStateB4Suspend = iDeviceState;
			NextDeviceState(EUsbcDeviceStateSuspended);
			r = (*iEnablePullUpOnDPlus)(iOtgContext);
			if (r != KErrNone)
				{
				__KTRACE_OPT(KPANIC, Kern::Printf("  Error: iEnablePullUpOnDPlus() = %d", r));
				}
			}
		else
			{
			__KTRACE_OPT(KUSB, Kern::Printf("  Warning: iEnablePullUpOnDPlus pointer not ready"));
			// We cannot enforce the presence of the pointer (via an ASSERT)
			// since it might only be available at a later point.
			// We shouldn't return an error at this point either, since the
			// problem will be a systematic one.
			}
		}
	else
		{
		__KTRACE_OPT(KUSB, Kern::Printf("  calling (*iDisablePullUpOnDPlus)()"));
		if (iDisablePullUpOnDPlus != NULL)
			{
			iDPlusEnabled = enableDPlus;
			r = (*iDisablePullUpOnDPlus)(iOtgContext);
			if (r != KErrNone)
				{
				__KTRACE_OPT(KPANIC, Kern::Printf("  Error: iDisablePullUpOnDPlus() = %d", r));
				}
			}
		else
			{
			__KTRACE_OPT(KUSB, Kern::Printf("  Warning: iDisablePullUpOnDPlus pointer not ready"));
			// We cannot enforce the presence of the pointer (via an ASSERT)
			// since it might only be available at a later point.
			// We shouldn't return an error at this point either, since the
			// problem will be a systematic one.
			}
		}
	return r;
	}


//
// DFC (static)
//
void DUsbClientController::ReconnectTimerCallback(TAny *aPtr)
	{
	__KTRACE_OPT(KUSB, Kern::Printf("DUsbClientController::ReconnectTimerCallback()"));
	if (!aPtr)
		{
		__KTRACE_OPT(KPANIC, Kern::Printf("  Error: !aPtr"));
		return;
		}
	DUsbClientController* const ptr = static_cast<DUsbClientController*>(aPtr);
	ptr->UsbConnect();
	}


//
// ISR (static)
//
void DUsbClientController::CableStatusTimerCallback(TAny *aPtr)
	{
	__KTRACE_OPT(KUSB, Kern::Printf("DUsbClientController::CableStatusTimerCallback()"));
	if (!aPtr)
		{
		__KTRACE_OPT(KPANIC, Kern::Printf("  Error: !aPtr"));
		return;
		}
	DUsbClientController* const ptr = static_cast<DUsbClientController*>(aPtr);
	ptr->ProcessSuspendEventProceed();
	}


//
// static
//
void DUsbClientController::PowerUpDfc(TAny* aPtr)
	{
	__KTRACE_OPT(KUSB, Kern::Printf("DUsbClientController::PowerUpDfc"));
	if (!aPtr)
		{
		__KTRACE_OPT(KPANIC, Kern::Printf("	 Error: !aPtr"));
		return;
		}
	DUsbClientController* const ptr = static_cast<DUsbClientController*>(aPtr);
	__PM_ASSERT(ptr->iStandby);
	(void) ptr->PowerUp();
	ptr->iStandby = EFalse;
	ptr->iPowerHandler->PowerUpDone();
	}


//
// static
//
void DUsbClientController::PowerDownDfc(TAny* aPtr)
	{
	__KTRACE_OPT(KUSB, Kern::Printf("DUsbClientController::PowerDownDfc"));
	if (!aPtr)
		{
		__KTRACE_OPT(KPANIC, Kern::Printf("	 Error: !aPtr"));
		return;
		}
	DUsbClientController* const ptr = static_cast<DUsbClientController*>(aPtr);
	__PM_ASSERT(!ptr->iStandby);
	ptr->iStandby = ETrue;
	// We might not want to power down when the UDC is active:
	if (!ptr->iHardwareActivated || ptr->PowerDownWhenActive())
		{
		(void) ptr->PowerDown();
		__KTRACE_OPT(KUSB, Kern::Printf("Calling PowerHandler->PowerDownDone()"));
		ptr->iPowerHandler->PowerDownDone();
		}
	else
		{
		__KTRACE_OPT(KUSB, Kern::Printf("Not calling PowerHandler->PowerDownDone()"));
		__KTRACE_OPT(KUSB, Kern::Printf("  because UDC is active."));
		}
	}


// -EOF-
