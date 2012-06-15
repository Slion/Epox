// Copyright (c) 2010 Nokia Corporation and/or its subsidiary(-ies).
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

/**
 @file
 @test
 @internalComponent - Internal Symbian test code
*/

#include "egltest_endpoint_stress_engine.h"
#include "egltest_endpoint_engine.h"

_LIT(KEGL_EndpointStressTests, "EGL_EndpointStressTests");


NONSHARABLE_CLASS(CEglTest_TestStep_StressTests): public CStressTestEngine
    {
public:
    static CEglTest_TestStep_StressTests *New();
private:
    CEglTest_TestStep_StressTests();
    };

NONSHARABLE_CLASS(CEglTest_TestStep_StressTests2): public CStressTestEngine
    {
public:
    static CEglTest_TestStep_StressTests2 *New();
private:
    CEglTest_TestStep_StressTests2();
    };
