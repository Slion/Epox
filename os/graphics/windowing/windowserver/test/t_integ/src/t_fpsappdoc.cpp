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

#include <eikapp.h>
#include <eikdoc.h>

#include "t_fpsappdoc.h"
#include "t_fpsappui.h"

/**
 *	Constructor that constructs a new document.
 *	@param aApp - The application instance that is creating the document. 
 *	
 */  
CFpsDocument::CFpsDocument(CEikApplication& aApp)
		: CEikDocument(aApp)
	{
	}
	
/**
 *	Constructs the application UI
 *	@return A partially-constructed application UI object. 
 *	
 */  
CEikAppUi* CFpsDocument::CreateAppUiL()
	{
    return new(ELeave) CFpsAppUi;
	}
