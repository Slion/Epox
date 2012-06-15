// Copyright (c) 1996-2009 Nokia Corporation and/or its subsidiary(-ies).
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
// Common utils
// 
//

#include <e32std.h>
#include "w32comm.h"

void TWriteDescriptorType::SetDescriptorOnIpcArgs(TIpcArgs& aIpcArgs) const
	{
	switch (iBufferType)
		{
		case EDes8:
			aIpcArgs.Set(KReplyBufferMessageSlot, reinterpret_cast<TDes8*>(iArgument));
			break;
		case EDes16:
			aIpcArgs.Set(KReplyBufferMessageSlot, reinterpret_cast<TDes16*>(iArgument));
			break;
		case EDesC8:
		case EDesC16:
		default:
			break;
		}
	}

void TReadDescriptorType::SetDescriptorOnIpcArgs(TIpcArgs& aIpcArgs) const
	{
	switch (iBufferType)
		{
		case EDesC8:
			aIpcArgs.Set(KRemoteBufferMessageSlot, reinterpret_cast<const TDesC8*>(iArgument));
			break;
		case EDesC16:
			aIpcArgs.Set(KRemoteBufferMessageSlot, reinterpret_cast<const TDesC16*>(iArgument));
			break;
		case EDes8:
		case EDes16:
		default:
			break;
		}
	}
