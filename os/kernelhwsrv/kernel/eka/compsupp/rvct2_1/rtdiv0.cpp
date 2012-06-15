// Copyright (c) 2002-2009 Nokia Corporation and/or its subsidiary(-ies).
// All rights reserved.
// This component and the accompanying materials are made available
// under the terms of the License "ARM EABI LICENCE.txt"
// which accompanies this distribution, and is available
// in kernel/eka/compsupp.
//
// Initial Contributors:
// Nokia Corporation - initial contribution.
//
// Contributors:
//
// Description:
//

#include <e32std.h>
#include <e32std_private.h>

extern "C" {
EXPORT_C void __rt_div0 (void)
    {
	User::RaiseException(EExcIntegerDivideByZero);
    }

}
