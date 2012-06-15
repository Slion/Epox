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

/**
 @file
 @test
 @internalComponent
*/

#ifndef __T_APP1_DOC_H__
#define __T_APP1_DOC_H__

#include <eikdoc.h>

/**
 *	CTApp1Document for creating the application UI
 *	
 */  
class CTApp1Document : public CEikDocument
	{
public:
	// creates a CTApp1Document object
	static CTApp1Document* NewL(CEikApplication& aApp);
	CTApp1Document(CEikApplication& aApp);
	void ConstructL();
private: 
	// Inherited from CEikDocument for creating the AppUI
	CEikAppUi* CreateAppUiL();
	};

#endif // __T_APP1_DOC_H__

