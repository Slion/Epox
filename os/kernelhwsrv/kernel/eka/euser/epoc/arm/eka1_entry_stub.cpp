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
// e32\euser\epoc\arm\eka1_entry_stub.cpp
// 
//

#include "u32std.h"
#include <e32base.h>
#include <e32base_private.h>
#include <e32rom.h>
#include <e32svr.h>
#include <u32exec.h>

#ifdef __MARM__
// Only need V7 support on ARM platforms
extern "C" void RunV7Thread(TBool aNotFirst, SThreadCreateInfo& aInfo)
	{
	SStdEpocThreadCreateInfo& cinfo = (SStdEpocThreadCreateInfo&)aInfo;
	TInt r = UserHeap::SetupThreadHeap(aNotFirst, cinfo);
	if (r==KErrNone)
		{
		if (!aNotFirst)
			{
			// Init statics for implicitly linked DLLs
			User::InitProcess();
			}

		// call file or thread entry point
		// this executes static ctors for EXEs
		r = (*cinfo.iFunction)(cinfo.iPtr);
		}
	User::Exit(r);
	}
#endif
