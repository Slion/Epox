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

#ifndef TESTSTEPCIERRORCONCEALMENT_H
#define TESTSTEPCIERRORCONCEALMENT_H

#include <e32base.h>
#include <testframework.h>

// Custom interface definitions
#include <mmf/server/devsoundstandardcustominterfaces.h>
#include <mmf/plugin/mmfdevsoundcustominterface.hrh>


#include "devsoundciutestdevices.hrh"

// Test system includes
#include "tsu_mmf_devsound_ciu_step.h"
#include "tsu_mmf_devsound_ciu_suite.h"


class CTestStepCIErrorConcealmentIntfc : public CTestStepCustominterfaceUnificationUnit
	{
public:
	CTestStepCIErrorConcealmentIntfc(const TDesC& aTestName):CTestStepCustominterfaceUnificationUnit(aTestName){}
	TVerdict DoTestStepL();

private:
	TVerdict DoTestStep0062L();
	TVerdict DoTestStep0063L();
	TVerdict DoTestStep0064L();
	TVerdict DoTestStep0065L();
	};

#endif
