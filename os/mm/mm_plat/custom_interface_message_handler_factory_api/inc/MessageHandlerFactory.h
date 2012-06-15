/*
* Copyright (c) 2005-2006 Nokia Corporation and/or its subsidiary(-ies). 
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
* Description:  Message Handler Factory
*
*/

#ifndef __CMESSAGEHANDLERFACTORY_H
#define __CMESSAGEHANDLERFACTORY_H

#include <e32base.h>
#include <mmfcontroller.h>

class CMessageHandlerFactory : public CBase
	{
public:
	IMPORT_C static CMMFObject* Create(TUid aInterfaceUid, TAny* aCustomInterface, CMMFObjectContainer& aContainer);
	};

#endif

