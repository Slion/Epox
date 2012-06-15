/*
* Copyright (c) 2005-2009 Nokia Corporation and/or its subsidiary(-ies).
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
*
*/



#ifndef DATA_WRAPPER_BASE_H
#define DATA_WRAPPER_BASE_H

//	EPOC includes
#include <datawrapper.h>
#define SECS_TO_MS(x)  (x*1000000)

_LIT(KConsname,				"Test Console");

class CDataWrapperBase : public CDataWrapper
	{
public:
	class TEnumEntryTable
		{
	public:
		const TDesC&	iString;
		TInt			iValue;
		};

public:
	TBool					GetBoolFromConfig(const TDesC& aSectName,const TDesC& aKeyName,TBool& aResult);
	TBool					GetIntFromConfig(const TDesC& aSectName, const TDesC& aKeyName, TInt& aResult);
	TBool GetRealFromConfig(const TDesC& aSectName, const TDesC& aKeyName, TReal& aResult);
	TBool					GetStringFromConfig(const TDesC& aSectName, const TDesC& aKeyName, TPtrC& aResult);
	TBool					GetHexFromConfig(const TDesC& aSectName, const TDesC& aKeyName, TInt& aResult);
	TBool					GetUintFromConfig(const TDesC& aSectName, const TDesC& aKeyName, TUint& aResult);
	TBool					GetArrayRectFromConfig(const TDesC& aSectName, const TDesC& aKeyName, RPointerArray<HBufC>& aResult);
	TBool 					KeyPress();
	void 					Timedelay(TInt aTimeoutInSecs);
	virtual void			InitialiseL();
	inline RFs&				FileServer() { return iFs; }
	inline CConsoleBase*	GetConsole() { return(Console::NewL(KConsname,TSize(KConsFullScreen,KConsFullScreen)));}
	TBool					GetEnumFromConfig(const TDesC& aSectName, const TDesC& aKeyName, const TEnumEntryTable* aTable, TInt& aResult);

protected:
	CDataWrapperBase();
	virtual ~CDataWrapperBase();

private:
	TBool	GetCommandStringParameterL(const TDesC& aSectName, const TDesC& aKeyName, TPtrC& aResult);
	TBool 	KeyCheck();

private:
	// Included ini files
	RPointerArray<CIniData>	iInclude;
	RPointerArray<HBufC>	iBuffer;
	RFs                     iFs;
	RTimer					iTimer;
	};

#endif // DATA_WRAPPER_BASE_H
