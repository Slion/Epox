/*
* Copyright (c) 2007-2009 Nokia Corporation and/or its subsidiary(-ies).
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
* tpropertystep.h
*
*/


/**
 @file  
 @internalTechnology
*/


#if (!defined PROPERTYSTEP_H)
#define PROPERTYSTEP_H
#include <test/testexecutestepbase.h>


class CPropertyStep: public CTestStep
	{
public:
	CPropertyStep();
	virtual ~CPropertyStep();
	virtual TVerdict doTestStepPreambleL();
	virtual TVerdict doTestStepL();
	virtual TVerdict doTestStepPostambleL();
	
// Please add/modify your class members here:
protected:
	void ProcessPropertyDataL(TPtrC& propertyData);

	};

_LIT(KPropertyStep,"PropertyStep");

#endif
