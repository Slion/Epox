// Copyright (c) 2008-2009 Nokia Corporation and/or its subsidiary(-ies).
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
 @file
 @test
 @internalComponent
*/

#ifndef __T_WSERV_GENERICPLUGIN_SERVER_H__
#define __T_WSERV_GENERICPLUGIN_SERVER_H__

#include <testexecuteserverbase.h>

// Entry point
GLDEF_C TInt E32Main(void);

class CT_WServGenericpluginServer : public CTestServer
	{
public:
	static CT_WServGenericpluginServer*	NewL();
	virtual CTestStep*			CreateTestStep(const TDesC& aStepName);

protected:
	void	ConstructL();
	};

#endif // __T_WSERV_GENERICPLUGIN_SERVER_H__
