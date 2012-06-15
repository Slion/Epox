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
// Entry point for the ECom dll only
// 
//

/**
 @file TSU_MMF_BASECL_TestSrcSinkImplementationProxy.cpp
*/

//#include <e32base.h>
#include "TSU_MMF_BASECL_TestSrcSink.h"
#include <ecom/implementationproxy.h>
#include "TSU_MMF_BASECL_TestSrcSinkImplementationUID.hrh"



// __________________________________________________________________________
// Exported proxy for instantiation method resolution
// Define the interface UIDs



const TImplementationProxy ImplementationTable[] = 
	{
	IMPLEMENTATION_PROXY_ENTRY( KMmfUidTSUBASECLTestSource,	CMMF_TSU_BASECL_TestSource::NewSourceL),
	IMPLEMENTATION_PROXY_ENTRY( KMmfUidTSUBASECLTestSink,	CMMF_TSU_BASECL_TestSink::NewSinkL)
	};

EXPORT_C const TImplementationProxy* ImplementationGroupProxy(TInt& aTableCount)
	{
	aTableCount = sizeof(ImplementationTable) / sizeof(TImplementationProxy);

	return ImplementationTable;
	}
