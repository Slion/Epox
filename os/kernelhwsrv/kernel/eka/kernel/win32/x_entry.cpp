// Copyright (c) 1998-2009 Nokia Corporation and/or its subsidiary(-ies).
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
// e32\kernel\win32\x_entry.cpp
// 
//

#include <kernel/kernel.h>

#ifdef __VC32__
#define __FLTUSED
#endif //__VC32__

// include the static data definitions
#include "win32crt.h"

// include compiler helpers
#include "x86hlp.inl"

GLREF_C TInt KernelModuleEntry(TInt);

extern "C"
int atexit(void (__cdecl *)(void))
	{
	return 0;
	}

#ifdef __CW32__

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

extern "C"
BOOL WINAPI _Win32DllMain(HINSTANCE, DWORD, LPVOID)
	{
	return 1;
	}
#endif

extern "C"
EXPORT_C TInt _E32Dll(TInt aReason)
//
// EPOC Dll entrypoint for extension
// Call extension global constructors
//
	{
	if (aReason==KModuleEntryReasonExtensionInit1)
		constructStatics();
	return KernelModuleEntry(aReason);
	}

