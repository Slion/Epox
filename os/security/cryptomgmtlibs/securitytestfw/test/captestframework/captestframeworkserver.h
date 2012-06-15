/*
* Copyright (c) 2002-2009 Nokia Corporation and/or its subsidiary(-ies).
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
*
*/




/**
 @file 
*/

#ifndef __CAPTESTFRAMEWORK_SERVER_H__
#define __CAPTESTFRAMEWORK_SERVER_H__
#include <test/testexecuteserverbase.h>

class CCapTestFrameworkServer : public CTestServer
	{
public:
	static CCapTestFrameworkServer* NewL();
	virtual CTestStep* CreateTestStep(const TDesC& aStepName);
	RFs& Fs(){return iFs;};

private:
	RFs iFs;
	};
#endif	/* ndef __CAPTESTFRAMEWORK_SERVER_H__ */
