// Copyright (c) 2004-2009 Nokia Corporation and/or its subsidiary(-ies).
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
 @file Te_authentication_TEFSuiteStepBase.cpp
*/

#include "Te_authentication_TEFSuiteStepBase.h"
#include "Te_authentication_TEFSuiteDefs.h"

// Device driver constants

TVerdict CTe_authentication_TEFSuiteStepBase::doTestStepPreambleL()
/**
 * @return - TVerdict
 * Implementation of CTestStep base class virtual
 * It is used for doing all initialisation common to derived classes in here.
 * Make it being able to leave if there are any errors here as there's no point in
 * trying to run a test step if anything fails.
 * The leave will be picked up by the framework.
 */
	{
	SetTestStepResult(EPass);
	return TestStepResult();
	}

TVerdict CTe_authentication_TEFSuiteStepBase::doTestStepPostambleL()
/**
 * @return - TVerdict
 * Implementation of CTestStep base class virtual
 * It is used for doing all after test treatment common to derived classes in here.
 * Make it being able to leave
 * The leave will be picked up by the framework.
 */
	{
	SetTestStepResult(EPass);
	return TestStepResult();
	}

CTe_authentication_TEFSuiteStepBase::~CTe_authentication_TEFSuiteStepBase()
	{
	}

CTe_authentication_TEFSuiteStepBase::CTe_authentication_TEFSuiteStepBase()
	{
	}
