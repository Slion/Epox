/*
* Copyright (c) 2007-2009 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
* This component and the accompanying materials are made available
* under the terms of the License "Eclipse Public License v1.0"
* which accompanies this distribution, and is available
* at the URL "http://www.eclipse.org/legal/epl-v10.html".
*
* Initial Contributors:
* Nokia Corporation - initial contribution.
*
* Contributors:
*
* Description: 
*
*/




/**
 @file
 @internalTechnology
*/
#ifndef __FILECOMPARE__
#define __FILECOMPARE__

#include <e32cmn.h>
#include <e32base.h>
#include <e32std.h>
#include <e32def.h>
#include <f32file.h>

	class TFileCompare
		{
	public:
		static TBool CompareL(TPtrC aFile1Path, TPtrC aFile2Path);
				
	private:

		};
#endif
