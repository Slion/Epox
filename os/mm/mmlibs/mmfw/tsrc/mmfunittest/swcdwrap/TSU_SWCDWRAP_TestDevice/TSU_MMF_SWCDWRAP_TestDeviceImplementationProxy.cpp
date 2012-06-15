// Copyright (c) 2003-2009 Nokia Corporation and/or its subsidiary(-ies).
// All rights reserved.
// This component and the accompanying materials are made available
// under the terms of "Eclipse Public License v1.0"
// which accompanies this distribution, and is available
// at the URL "http://www.eclipse.org/legal/epl-v10.html".
//
// Initial Contributors:
// Nokia Corporation - initial contribution.
//
// Contributors:
//
// Description:
//

#include "TSU_MMF_SWCDWRAP_TestDevice.h"
#include <ecom/implementationproxy.h>
#include "TSU_MMF_SWCDWRAP_TestDeviceImplementationUID.hrh"


// __________________________________________________________________________
// Exported proxy for instantiation method resolution
// Define the interface UIDs

const TImplementationProxy ImplementationTable[] = 
	{
	IMPLEMENTATION_PROXY_ENTRY(KMmfUidTSUSWCDWRAPTestDevice,	CMMFTestNullDevice::NewL),
	IMPLEMENTATION_PROXY_ENTRY(KMmfUidTSUSWCDWRAPTest2To1Device,	CMMFTest2To1Device::NewL)
	};


EXPORT_C const TImplementationProxy* ImplementationGroupProxy(TInt& aTableCount)
	{
	aTableCount = sizeof(ImplementationTable) / sizeof(TImplementationProxy);

	return ImplementationTable;
	}
