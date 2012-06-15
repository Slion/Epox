/*
* Copyright (c) 2006-2010 Nokia Corporation and/or its subsidiary(-ies).
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

#ifndef __T_CP1250_H__
#define __T_CP1250_H__

#include <test/testexecutestepbase.h>

class CT_CP1250 : public CTestStep
    {
public:
    CT_CP1250();
protected:  
    TVerdict doTestStepL();
private:
	void TestL();
	void OOMTestL();
    };

_LIT(KTestStep_T_CP1250, "T_CP1250");

#endif