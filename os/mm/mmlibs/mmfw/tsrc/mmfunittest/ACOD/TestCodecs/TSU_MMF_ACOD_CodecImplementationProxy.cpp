// Copyright (c) 2002-2009 Nokia Corporation and/or its subsidiary(-ies).
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

#include "TSU_MMF_ACOD_AAAABBBBTestCodec.h"
#include "TSU_MMF_ACOD_AAAABBBBTest2Codec.h"

#include <ecom/implementationproxy.h>
#include "TSU_MMF_ACOD_TestCodecImplementationUIDs.hrh"


// __________________________________________________________________________
// Exported proxy for instantiation method resolution
// Define the interface UIDs

const TImplementationProxy ImplementationTable[] = 
	{
		IMPLEMENTATION_PROXY_ENTRY(KMmfUidCodecAAAABBBBTest,	CMMFAAAABBBBTestCodec::NewL),
		IMPLEMENTATION_PROXY_ENTRY(KMmfUidCodecAAAABBBBTest2,	CMMFAAAABBBBTest2Codec::NewL)
	};

EXPORT_C const TImplementationProxy* ImplementationGroupProxy(TInt& aTableCount)
	{
	aTableCount = sizeof(ImplementationTable) / sizeof(TImplementationProxy);

	return ImplementationTable;
	}