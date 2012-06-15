// Copyright (c) 2005-2009 Nokia Corporation and/or its subsidiary(-ies).
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

#ifndef __TE_SSA_ScheduleSuiteServer_H__
#define __TE_SSA_ScheduleSuiteServer_H__
#include <test/testexecuteserverbase.h>
		
class CSSA_ScheduleSuiteSvr : public CTestServer
	{
public:
	static CSSA_ScheduleSuiteSvr* NewL();
	virtual CTestStep* CreateTestStep(const TDesC& aStepName);
	};

#endif
