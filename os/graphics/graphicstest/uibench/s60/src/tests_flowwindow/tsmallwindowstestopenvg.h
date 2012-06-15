// Copyright (c) 2009 Nokia Corporation and/or its subsidiary(-ies).
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
 
#ifndef TSMALLWINDOWSTESTOPENVG_H
#define TSMALLWINDOWSTESTOPENVG_H

#include "te_graphicsperformanceSuiteStepBase.h"
#include "tflowwindowscontroller.h"
#include <w32std.h>

/**
 *  CTSmallWindowsTest
 * 
 */
class CTSmallWindowsTestOpenVG : public CTe_graphicsperformanceSuiteStepBase
	{
public:
    CTSmallWindowsTestOpenVG();
    virtual ~CTSmallWindowsTestOpenVG();

private:
    // From CTestStep
    virtual TVerdict doTestStepPreambleL();
    virtual TVerdict doTestStepL();
    
    void FlowWindowsL();

private:    
    CTFlowWindowsController* iFlowWindowsController;
    RArray<TPtrC> iFileNames;
    TSize iWindowSize;
    TInt iIterationsToTest; 
	};

_LIT(KTSmallWindowsOpenVG, "tsmallwindowsopenvg");

#endif // TSMALLWINDOWSTESTOPENVG_H
