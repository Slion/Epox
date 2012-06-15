/**
* Copyright (c) 2004-2009 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
* This component and the accompanying materials are made available
* under the terms of "Eclipse Public License v1.0"
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
 @file Te_RFileLoggerSuiteStepBase.h
*/

#if (!defined __TE_RFILELOGGER_STEP_BASE__)
#define __TE_RFILELOGGER_STEP_BASE__
#include <test/testexecutestepbase.h>
#include <test/rfilelogger.h>
// Please add your include here if you have 

/****************************************************************************
* The reason to have a new step base is that it is very much possible
* that the all individual test steps have project related common variables 
* and members 
* and this is the place to define these common variable and members.
* 
****************************************************************************/
class CTe_RFileLoggerSuiteStepBase : public CTestStep
	{
public:
	virtual ~CTe_RFileLoggerSuiteStepBase();
	CTe_RFileLoggerSuiteStepBase();
	virtual TVerdict doTestStepPreambleL(); 
	virtual TVerdict doTestStepPostambleL();

//Please add your class members which will be common to all individual test steps:
protected:
	};

#endif
