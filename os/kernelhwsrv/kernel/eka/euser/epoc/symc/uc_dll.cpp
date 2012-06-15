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
// e32\euser\epoc\win32\uc_dll.cpp
// This file contains the DLL entrypoint
// 
//

#include "u32std.h"

typedef void* HANDLE;

#ifdef __VC32__
#define __FLTUSED
#endif //__VC32__


#define WIN32_LEAN_AND_MEAN
#include <windows.h>

// DLL entry point
/*
extern "C"
BOOL WINAPI DllMain(HINSTANCE, DWORD fdwReason, LPVOID)
	{
		return 1;
	}
*/

// include the static data definitions
//#include "win32crt.h"

// include compiler helpers
//#include "x86hlp.inl"

extern "C"
EXPORT_C TInt _E32Dll(TInt aReason)
	{
	if (aReason==KModuleEntryReasonProcessAttach)
		{
		//constructStatics();
		}
	if (aReason==KModuleEntryReasonProcessDetach)
		{
		//destroyStatics();
		}
	return KErrNone;
	}
