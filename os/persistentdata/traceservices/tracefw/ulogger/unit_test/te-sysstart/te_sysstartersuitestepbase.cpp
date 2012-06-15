// Copyright (c) 2007-2009 Nokia Corporation and/or its subsidiary(-ies).
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
 @file te_sysstartersuitestepbase.cppp
 @internalTechnology
*/

#include "te_sysstartersuitestepbase.h"
#include "te_sysstartersuitedefs.h"

// Device driver constants

TVerdict CTe_sysstartSuiteStepBase::doTestStepPreambleL()
/**
 * @return - TVerdict
 * Implementation of CTestStep base class virtual
 * It is used for doing all initialisation common to derived classes in here.
 * Make it being able to leave if there are any errors here as there's no point in
 * trying to run a test step if anything fails.
 * The leave will be picked up by the framework.
 */
	{
	INFO_PRINTF1(_L("installing scheduler"));
	INFO_PRINTF1(_L("Sys Start Unit tests"));
	iScheduler = NULL;
	iScheduler = new(ELeave) CActiveScheduler;
	CActiveScheduler::Install(iScheduler);

	SetTestStepResult(EPass);
	return TestStepResult();
	}

TVerdict CTe_sysstartSuiteStepBase::doTestStepPostambleL()
/**
 * @return - TVerdict
 * Implementation of CTestStep base class virtual
 * It is used for doing all after test treatment common to derived classes in here.
 * Make it being able to leave
 * The leave will be picked up by the framework.
 */
	{
	//SetTestStepResult(EPass);  // or EFail
	return TestStepResult();
	}

CTe_sysstartSuiteStepBase::~CTe_sysstartSuiteStepBase()
	{
	INFO_PRINTF1(_L("deleting scheduler"));
	delete iScheduler;
	}

CTe_sysstartSuiteStepBase::CTe_sysstartSuiteStepBase()
	{
	}

