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

#include <e32std.h>
#include "panics.h"

void Panic(TEventCheckerPanic aPanic)
	{
	_LIT(KEventCheckerPanicCategory, "EVENTCHECKER");
	User::Panic(KEventCheckerPanicCategory, aPanic);
	}
