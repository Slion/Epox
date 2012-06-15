/*
* Copyright (c) 2006-2009 Nokia Corporation and/or its subsidiary(-ies).
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
* Name        : wsdDll1.h
* Part of     : wsdDll1 DLL
* Contains declarations for the wsdDll1 APIs
* This material, including documentation and any related 
* computer programs, is protected by copyright controlled by 
* Nokia Corporation. All rights are reserved. Copying, 
* including reproducing, storing,  adapting or translating, any 
* or all of this material requires the prior written consent of 
* Nokia Corporation. This material also contains confidential 
* information which may not be disclosed to others without the 
* prior written consent of Nokia Corporation.
*
*/



#ifndef __WSD_DLL1_H__
#define __WSD_DLL1_H__

#include <e32cons.h>

IMPORT_C TInt GetDll1Int();
IMPORT_C void GetDll1Buf(TDes& aBuf);
IMPORT_C void SetDll1Int(const TInt& aArg);
IMPORT_C void SetDll1Buf(TDesC& aArg);

#endif // ifndef __WSD_DLL1_H__
