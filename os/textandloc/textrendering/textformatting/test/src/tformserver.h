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
* @file
* @internalComponent 
*
*/


#ifndef TFORMSERVER_H
#define TFORMSERVER_H

// INCLUDES
#include <test/testexecuteserverbase.h>

// CLASS DECLARATION

/**
 *  Ctformserver
 * 
 */
class CTFormServer : public CTestServer
    {
public:
    static CTFormServer* NewL();
//from  CTestServer
    virtual CTestStep* CreateTestStep(const TDesC& aStepName);

    };

#endif // TFORMSERVER_H
