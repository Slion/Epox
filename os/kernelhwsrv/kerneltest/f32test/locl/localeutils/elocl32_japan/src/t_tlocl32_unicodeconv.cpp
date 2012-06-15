/*
* Copyright (c) 2005 Nokia Corporation and/or its subsidiary(-ies).
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



#include <e32svr.h>
#include <e32std.h>
#include <e32def.h>
#include <e32des8.h> 

#include "t_tlocl32_unicodeconv.h"

//replacement character to be used when unicode cannot be converted
const TUint8 KForeignReplacement = 0x5F;

//This function converts from Unicoded characters, to foreign characters and adds them into a descriptor
void UnicodeConv::ConvertFromUnicodeL(TDes8& aForeign, const TDesC16& aUnicode)
	{	
	const TInt unicodeLength = aUnicode.Length();
	aForeign.Zero();
	
	//loop going through the character of the unicode descriptor
	for(TInt i=0; i<unicodeLength; i++)
		{
		const TUint16 unicodeChar = aUnicode[i];
		
		if (aForeign.Length() >= aForeign.MaxLength())
			{
			// User::Leave(KErrOverflow);
			User::Leave(KErrBadName);
			}
		
		//charcters from 0x0000 to 0x007F, can be mapped directly
		if(unicodeChar<0x0080)
			{
			aForeign.Append(static_cast<TUint8>(unicodeChar));
			}
		else
			{
			TInt trailByte = KErrNotFound;
			TInt returnValue = TConvDataStruct::ConvertSingleUnicode(unicodeChar,trailByte);
			
			if(returnValue!=KErrNotFound)
				{
				if(trailByte!=KErrNotFound)		
					{
					aForeign.Append(static_cast<TUint8>(returnValue));
					// as two bytes are being added check enough space for second
					if (aForeign.Length() >= aForeign.MaxLength())
						{											
						// User::Leave(KErrOverflow);
						User::Leave(KErrBadName);
						}	
					aForeign.Append(static_cast<TUint8>(trailByte));						
					}
				else
					aForeign.Append(static_cast<TUint8>(returnValue));
				}		
			else
				aForeign.Append(KForeignReplacement);
			}
		}
	}

//This function converts from foreign characters into unicode and adds them into a descriptor
void UnicodeConv::ConvertToUnicodeL(TDes16& aUnicode, const TDesC8& aForeign)
	{
	const TInt foreignLength = aForeign.Length();
	aUnicode.Zero();

	//loop going through the characters of the foreign descriptor
	for(TInt i = 0; i<foreignLength; i++)
		{
		const TUint8 leadForeign = aForeign[i];
		
		if (aUnicode.Length() >= aUnicode.MaxLength())
			{
			// User::Leave(KErrOverflow);
			User::Leave(KErrBadName);
			}
		
		TUint8 tailForeign = 0x00;
		//charcters from 0x00 to 0x7F, can be mapped directly
		if(leadForeign < 0x80)
			aUnicode.Append(static_cast<TUint16>(leadForeign));
		else
			{
			if((i+1)<foreignLength)
				tailForeign = aForeign[i+1];

			const TLeadOrSingle* structPtr = TConvDataStruct::KFirstByteConversions + (leadForeign-0x80);
			
			if(structPtr->iUnicodeIfSingle)
				aUnicode.Append(structPtr->iUnicodeIfSingle);
			else
				{
				if(TConvDataStruct::KMinTrailByte<=tailForeign && tailForeign<=TConvDataStruct::KMaxTrailByte)
					aUnicode.Append(TConvDataStruct::KDoubleByteConversions[structPtr->iDoubleByteIndex+
						(tailForeign - TConvDataStruct::KMinTrailByte)]);
				else
					aUnicode.Append(0xFFFD);
				i++;
				}
			}
		}
	}

TBool UnicodeConv::IsLegalShortNameCharacter (TUint aCharacter)
	{
	//1. aCharacter >= 0x0080 
	if (aCharacter>=0x0080)
		{
		if (aCharacter>0xFFFF)
			return EFalse;
		
		TInt trailByte = KErrNotFound;
		TInt returnValue = TConvDataStruct::ConvertSingleUnicode(aCharacter,trailByte);
		
		if(returnValue!=KErrNotFound)
			return ETrue;
		else
			return EFalse;
		}
	
    // For most common cases: 
    // Note: lower case characters are considered legal DOS char here. 
	if ((aCharacter>='a' && aCharacter<='z') || 
	    (aCharacter>='A' && aCharacter<='Z') || 
	    (aCharacter>='0' && aCharacter<='9'))
			{
			return ETrue;
			}
    // Checking for illegal chars: 
    // 2. aCharacter <= 0x20 
    // Note: leading 0x05 byte should be guarded by callers of this function 
    //  as the information of the position of the character is required. 
	if (aCharacter < 0x20)
		return EFalse;
	// Space (' ') is not considered as a legal DOS char here.
	if (aCharacter == 0x20)
		return EFalse;
	
	// 3. 0x20 < aCharacter < 0x80 
    // According to FAT Spec, "following characters are not legal in any bytes of DIR_Name": 
    switch (aCharacter) 
            { 
            case 0x22:        // '"' 
            case 0x2A:        // '*' 
            case 0x2B:        // '+' 
            case 0x2C:        // ',' 
            //case 0x2E:        // '.'   // Although '.' is not allowed in any bytes of DIR_Name, it 
                                         // is a valid character in short file names. 
            case 0x2F:        // '/' 
            case 0x3A:        // ':' 
            case 0x3B:        // ';' 
            case 0x3C:        // '<' 
            case 0x3D:        // '=' 
            case 0x3E:        // '>' 
            case 0x3F:        // '?' 
            case 0x5B:        // '[' 
            case 0x5C:        // '\' 
            case 0x5D:        // ']' 
            case 0x7C:        // '|' 
            	return EFalse; 
            default: 
            	return ETrue; 
            } 
	}		

