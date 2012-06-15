/*
* Copyright (c) 2005-2009 Nokia Corporation and/or its subsidiary(-ies).
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
* Series 60 version of Hindi Demo application
*
*/


#include "HindiDemoapp.h"

#include <eikstart.h>

EXPORT_C CApaApplication* NewApplication()
	{
    return new CHindiDemoApplication();
	}

GLDEF_C TInt E32Main()
	{
	return EikStart::RunApplication(&NewApplication);
	}
	
	
// -----------------------------------------------------------------------
// dll entry point
// -----------------------------------------------------------------------
/* not needed for 3.0
GLDEF_C TInt E32Dll(TDllReason)
{
    return KErrNone;
} */


