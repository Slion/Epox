/*
* Copyright (c) 2005-2009 Nokia Corporation and/or its subsidiary(-ies).
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
* Example CTestStep derived implementation
*
*/


/**
 @file
 @internalTechnology
*/
#include "keyexchangeasynchronousgeneralusagestep.h"

CKeyExchangeASynchronousGeneralUsageStep::~CKeyExchangeASynchronousGeneralUsageStep()
/**
 * Destructor
 */
	{
	}

CKeyExchangeASynchronousGeneralUsageStep::CKeyExchangeASynchronousGeneralUsageStep()
/**
 * Constructor
 */
	{
	// **MUST** call SetTestStepName in the constructor as the controlling
	// framework uses the test step name immediately following construction to set
	// up the step's unique logging ID.
	SetTestStepName(KKeyExchangeASynchronousGeneralUsageStep);
	}

TVerdict CKeyExchangeASynchronousGeneralUsageStep::doTestStepPreambleL()
/**
 * @return - TVerdict code
 * Override of base class virtual
 */
	{
	INFO_PRINTF1(_L("Please delete this line or modify it. I am in Test Step Preamble in Class CKeyExchangeASynchronousGeneralUsageStep"));
	// uncomment the following 3 lines if you have common pre setting to all the test steps in there
	// CTe_FactoryObjectsSuiteStepBase::doTestStepPreambleL();
	// if (TestStepResult()!=EPass)
	//    return   TestStepResult();
	// process some pre setting to this test step then set SetTestStepResult to EFail or Epass.
	SetTestStepResult(EPass);
	return TestStepResult();
	}


TVerdict CKeyExchangeASynchronousGeneralUsageStep::doTestStepL()
/**
 * @return - TVerdict code
 * Override of base class pure virtual
 * Our implementation only gets called if the base class doTestStepPreambleL() did
 * not leave. That being the case, the current test result value will be EPass.
 */
	{
	  if (TestStepResult()==EPass)
		{


		SetTestStepResult(EPass);
		}
	  return TestStepResult();
	}



TVerdict CKeyExchangeASynchronousGeneralUsageStep::doTestStepPostambleL()
/**
 * @return - TVerdict code
 * Override of base class virtual
 */
	{
	INFO_PRINTF1(_L("Please delete this line or modify it. I am in Test Step Postamble in Class CKeyExchangeASynchronousGeneralUsageStep"));
	// process something post setting to the test step
	// uncomment the following line if you have common post setting to all the test steps in there
	// CTe_FactoryObjectsSuiteStepBase::doTestStepPostambleL();
	// uncomment the following line if you have post process or remove the following line if no post process
	// SetTestStepResult(EPass);		// or EFail
	return TestStepResult();
	}
