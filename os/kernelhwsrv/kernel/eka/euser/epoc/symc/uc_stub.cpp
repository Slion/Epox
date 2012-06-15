// Copyright (c) 1995-2009 Nokia Corporation and/or its subsidiary(-ies).
// All rights reserved.
// This component and the accompanying materials are made available
// under the terms of the License "Eclipse Public License v1.0"
// which accompanies this distribution, and is available
// at the URL "http://www.eclipse.org/legal/epl-v10.html".
//
// Initial Contributors:
// Nokia Corporation - initial contribution.
//
// Contributors:
//
// Description:
// e32\euser\epoc\symc\uc_stub.cpp
// 
//

/**
@file uc_stub.cpp

Stubs are executable that can actually be launch from the host platform.
They are not managed by the Symbian OS Kernel.
*/

#include <e32std.h>
#include <e32std_private.h>
#include <e32wins.h>

/*
extern "C"
GLDEF_C TInt _E32Startup()
//
// Unused in the stub
//
	{
	return KErrNone;
	}
*/

#ifdef WIN32

#include <windows.h>


HINSTANCE gInstance;
HINSTANCE gPrevInstance;
LPSTR gCmdLine;
int gCmdShow;


int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
	{
   gInstance=hInstance;
   gPrevInstance=hPrevInstance;
   gCmdLine=lpCmdLine;
   gCmdShow=nCmdShow;

	BootEpoc(ETrue);
	return 0;
	};

#endif