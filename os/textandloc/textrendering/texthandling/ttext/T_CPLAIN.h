/*
* Copyright (c) 2010 Nokia Corporation and/or its subsidiary(-ies).
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

#ifndef __T_CPLAIN_H__
#define __T_CPLAIN_H__

#include <test/testexecutestepbase.h>

class CT_CPLAIN : public CTestStep
	{
public:
	CT_CPLAIN();
protected:
	TVerdict doTestStepL();
private:
	};

_LIT(KTestStep_T_CPLAIN, "T_CPLAIN");

#endif
