// Copyright (c) 1996-2009 Nokia Corporation and/or its subsidiary(-ies).
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
// f32\sfat\sl_vfat.cpp
// 
//

#include "sl_std.h"
#include "sl_cache.h"
#include <e32svr.h>
#include <e32math.h>


IMPORT_C const TFatUtilityFunctions* GetFatUtilityFunctions();

const TInt KMaxLengthWithoutTilde = 8;
const TUint8 KLeadingE5Replacement = 0x05;

// use second half of ISO Latin 1 character set for extended chars
const TUint KExtendedCharStart=0x80;
const TUint KExtendedCharEnd=0xff;

LOCAL_C TBool IsLegalChar(TChar aCharacter,TBool aAllowWildChars,TBool aUseExtendedChars=EFalse,TBool aInScanDrive=EFalse)
//
// Returns ETrue if aCharacter is legal inside a dos filename
//
	{
	if ((aCharacter==KMatchOne) || (aCharacter==KMatchAny))
		return(aAllowWildChars);
	if ((TUint)aCharacter < 0x20)
	    return EFalse;
   	// Don't check illegal ascii char because some non-English char value may
   	// fall in this area
    if (aInScanDrive)
    	return ETrue;
	return LocaleUtils::IsLegalShortNameCharacter(aCharacter,aUseExtendedChars);
	}

LOCAL_C void ReplaceFirstCharacterIfClashesWithE5L(TDes8& aShortName)
	{
	if (0 < aShortName.Length() && aShortName[0] == KEntryErasedMarker)
		{
		aShortName[0] = KLeadingE5Replacement;
		}
	}

LOCAL_C void ReplaceIllegalCharactersL(TDes& aLongName, TUint aCharacterToReplaceWith)
	{
	TBool alreadyFoundExtensionDelimiter=EFalse;

	TInt LongNameLen = aLongName.Length();
	TInt extDelimiterIndex = aLongName.LocateReverse(KExtDelimiter);

	for (TInt i=LongNameLen-1; i>=0; --i) // iterate backwards as aLongName may change length during the loop, and also because we want to leave the *right-most* occurrence of KExtDelimiter unchanged
		{
		TUint character=aLongName[i];
		if (character==(TUint)KExtDelimiter)
			{
			if (alreadyFoundExtensionDelimiter)
				{
				aLongName[i]=(TText)aCharacterToReplaceWith; // A.B.C becomes A_B.C
				}
			alreadyFoundExtensionDelimiter=ETrue;
			}
		else
			{
			// the code below doesn't need any #if defined(_UNICODE) stuff as a narrow-build aLongName would not contain values above 0xff (which is well below the surrogates area in Unicode 0xd800-0xdfff)
			TBool isSurrogatePair=EFalse;

			// LAST character in file name or file ext CAN NOT be HIGH surrogate
			if (i==LongNameLen-1 || i==extDelimiterIndex-1)
				{
				if (IsHighSurrogate((TText16)character))
					{
					// Corrupt surrogate
					User::Leave(KErrBadName);
					}
				}
			// FIRST character in file name or file ext CAN NOT be LOW surrogate
			if (i==0 || i==extDelimiterIndex+1)
				{
				if (IsLowSurrogate((TText16)character))
					{
					// Corrupt surrogate
					User::Leave(KErrBadName);
					}
				}
			// if LOW Surrogate
			if (IsLowSurrogate((TText16)character))
				{
				// check for HIGH surrogate
				if (!IsHighSurrogate(aLongName[--i]))
					{
					// Corrupt surrogate
					User::Leave(KErrBadName);
					}
				// surrogate pair found
				character&=~0xdc00;
				character|=((aLongName[i]&~0xd800)<<10);
				character+=0x00010000; // this must be added - it cannot be bitwise-"or"-ed
				isSurrogatePair=ETrue;
				}

			// if High Surrogate
			if (!isSurrogatePair && IsHighSurrogate((TText16)character))
				{
				// Corrupt surrogate
				User::Leave(KErrBadName);
				}

			if (!IsLegalChar(character, EFalse))
				{
				if (isSurrogatePair)
					{
					aLongName.Delete(i+1, 1);
					}
				aLongName[i]=(TText)aCharacterToReplaceWith;
				}
			}
		}
	}

TShortName DoGenerateShortNameL(const TDesC& aLongName,TInt& aNum,TBool aUseTildeSelectively)
//
// Create a legal shortname from aLongName
//
	{

	TFileName longName(aLongName);
	longName.UpperCase();
	ReplaceIllegalCharactersL(longName, '_');
	TPtrC longNameWithoutExtension(longName);
	TPtrC longNameExtension(KNullDesC);
	const TInt positionOfExtension=longName.LocateReverse(KExtDelimiter);
	if (positionOfExtension==0)
		{
		// No filename specified, so use the extension as the basis of the shortname.
		// Make sure we always append a tilde+number in this case to avoid generating the same 
		// short filename as one of the protected folders ("\SYS", "\RESOURCE","\PRIVATE")
		longNameWithoutExtension.Set(longName.Mid(positionOfExtension+1));
		aUseTildeSelectively = EFalse;
		if (aNum < 0)
			aNum = 1;
		}
	else if (positionOfExtension!=KErrNotFound)
		{
		longNameWithoutExtension.Set(longName.Left(positionOfExtension));
		longNameExtension.Set(longName.Mid(positionOfExtension+1));
		}
	
	// Converts the original file name main part into 8-bit character string
	TShortName tempShortName(0);

	LocaleUtils::ConvertFromUnicodeL(tempShortName, longNameWithoutExtension);
	const TInt originalNameLength = tempShortName.Length();

	// Converts the original file name extension part into 8-bit character string
	TShortName tempShortNameExt(0);
	
	LocaleUtils::ConvertFromUnicodeL(tempShortNameExt, longNameExtension);
	const TInt extensionNameLength = tempShortNameExt.Length();
	// // const TInt extensionNameLength = tempShortNameExt.Length();

	// Checks the length of both original file name main part and original file name extension part
	if(aUseTildeSelectively)
		{
		// don't append ~<aNum>
		if(originalNameLength<=KMaxLengthWithoutTilde && extensionNameLength<=KMaxFatFileNameExt)
			aNum=-1;
		}

	// Applies tilde and number if necessary
	TBuf8<5> tildeAndNumber;
	if (aNum>=0)
		{
		tildeAndNumber.Append('~');
		tildeAndNumber.AppendNumUC(aNum,EHex);
		}
	const TInt lengthOfTildeAndNumber=tildeAndNumber.Length();

	// Creates actual shortname from longname of the original file
	TShortName shortName(11);
#if defined(_DEBUG)
	shortName.Fill(0x01); // fill shortName with garbage to ensure that every byte is written to by this function
#endif

	// Fills the main part of the shortname of the original file
	const TInt numberOfBytesFreeBeforeTilde=KMaxFatFileNameWithoutExt-lengthOfTildeAndNumber;

	TPtr8 portionOfShortNameBeforeTilde((TUint8*)shortName.Ptr(), 0, numberOfBytesFreeBeforeTilde);
	TInt lengthOfPortionOfShortNameBeforeTilde = 
				(originalNameLength < numberOfBytesFreeBeforeTilde) ? originalNameLength : numberOfBytesFreeBeforeTilde;

	portionOfShortNameBeforeTilde.Copy((TUint8*)tempShortName.Ptr(), lengthOfPortionOfShortNameBeforeTilde);
	if( lengthOfPortionOfShortNameBeforeTilde != originalNameLength)
		{
		for( int i = 0; i<lengthOfPortionOfShortNameBeforeTilde; i++)
			{
			if(portionOfShortNameBeforeTilde[i] >= 0x80) //leading byte found
				{
				if( i == lengthOfPortionOfShortNameBeforeTilde - 1) //leading byte found on the edge
					{
					lengthOfPortionOfShortNameBeforeTilde -= 1;
					break;
					}
				else
					{
					i++;
					}
				}
			}
		}
	Mem::Copy(((TUint8*)shortName.Ptr())+lengthOfPortionOfShortNameBeforeTilde, tildeAndNumber.Ptr(), lengthOfTildeAndNumber);
	TInt i;
	for (i=lengthOfPortionOfShortNameBeforeTilde+lengthOfTildeAndNumber; i<KMaxFatFileNameWithoutExt; ++i)
		{
		shortName[i]=' ';
		}

	// Fills the extension part of the shortname of the original file
	TInt lengthOfExt = 
				(extensionNameLength < KMaxFatFileNameExt) ? extensionNameLength : KMaxFatFileNameExt;
	
	if( lengthOfExt != extensionNameLength)
		{
		for( int i = 0; i<lengthOfExt; i++)
			{
			if(tempShortNameExt[i] >= 0x80)
				{
				if( i == lengthOfExt - 1)
					{
					lengthOfExt -= 1;
					break;
					}
				else
					{
					i++;
					}
				}
			}
		}			
	Mem::Copy(((TUint8*)shortName.Ptr()) + KMaxFatFileNameWithoutExt, tempShortNameExt.Ptr(), lengthOfExt);
	for (i = KMaxFatFileNameWithoutExt + lengthOfExt; i<KMaxFatFileNameWithoutExt+KMaxFatFileNameExt; ++i)
		{
		shortName[i]=' ';
		}
	
	ReplaceFirstCharacterIfClashesWithE5L(shortName);		
	return shortName;
	}


/**
Check whether a Dos name is legal or not.

@param aName                The entry name to be analysed (may be represented as TDes16& or TDes8&)
@param anAllowWildCards	    Flag to indicate whether to allow wildcards in name or not
@param aUseExtendedChars    Flag to indicate if extended characters are allowed
@param aInScanDrive         Flag to indicate whether called when scanning drive
@param aAllowLowerCase      ETrue to allow lower case in the analysed DOS name

@return ETrue if the name is a legal DOS one.
*/

static TBool DoCheckLegalDosName(const TDesC& aName, TBool anAllowWildCards, TBool aUseExtendedChars, TBool aInScanDrive, TBool aAllowLowerCase, TBool aIsForFileCreation)
	{
    const TInt count=aName.Length();
	if (count==0)
		return EFalse;

	TInt valid=0;
	TInt i=0;
	
    //-- check the entry name
	while (i<count)
		{
		TChar c=aName[i++];
		if (c==KExtDelimiter)
			{
			// DOS entry names must contain at least one valid character before the extension
			if (i == 1)
				return EFalse;
			break;
			}
		
          if(!aAllowLowerCase && c.IsLower())
            return EFalse; //-- low case is not allowed

		if (!IsLegalChar(c,anAllowWildCards,aUseExtendedChars,aInScanDrive))
		    {
			return EFalse;
		    }
		
		if (aIsForFileCreation)
			{
			if ((aUseExtendedChars && (TUint) c > KExtendedCharEnd) || 
					(!aUseExtendedChars && (TUint) c > KExtendedCharStart))
				{
				return EFalse;
				}
			}

		if (c!=KMatchAny)
			if (++valid>KMaxFatFileNameWithoutExt)
				return EFalse;
		}
	
	//-- check entry extension
    valid=0;
	while (i<count)
		{
		TChar c=aName[i++];
		if (c==KExtDelimiter)
			return EFalse;
		
        if(!aAllowLowerCase && c.IsLower())
            return EFalse; //-- low case is not allowed

		if (!IsLegalChar(c,anAllowWildCards,aUseExtendedChars,aInScanDrive))
			return EFalse;
		
		if (aIsForFileCreation)
			{
			if ((aUseExtendedChars && (TUint) c > KExtendedCharEnd) || 
					(!aUseExtendedChars && (TUint) c > KExtendedCharStart))
				{
				return EFalse;
				}
			}

		if (c!=KMatchAny)
			if (++valid>KMaxFatFileNameExt)
				return EFalse;
		}
	
	// Unicode file name checking for file opening.
	if (!aIsForFileCreation && GetFatUtilityFunctions())
		{
		TBuf8<KMaxFileName*2> convertedName8;
		TRAPD(err, LocaleUtils::ConvertFromUnicodeL(convertedName8, aName, TFatUtilityFunctions::EOverflowActionLeave));
		if (err != KErrNone)
			return EFalse;
		
		const TInt len8 = convertedName8.Length();
		TInt j = 0; 
		TInt nonWildChar = 0;
		while (j < len8)
			{
			const TUint8 c8 = convertedName8[j++];
			if (c8 == KExtDelimiter)
				break;
			if (c8 == '*' && !anAllowWildCards)
				return EFalse;
			if (c8 == '*' && anAllowWildCards)
				continue;
			
			if (++nonWildChar > KMaxFatFileNameWithoutExt)
				return EFalse;
			}
		
		// check extension part
		nonWildChar = 0;
		while (j < len8)
			{
			const TUint8 c8 = convertedName8[j++];
			if (c8 == KExtDelimiter)
				return EFalse;
			if (c8 == '*' && !anAllowWildCards)
				return EFalse;
			if (c8 == '*' && anAllowWildCards)
				continue;
			
			if (++nonWildChar > KMaxFatFileNameExt)
				return EFalse;
			}
		}

	return ETrue;
	}

/**
    Check whether a Dos name is legal or not. Unicode version
    parameters and return value absolutely the same as in DoCheckLegalDosName()
*/
TBool IsLegalDosName(const TDesC16& aName, TBool anAllowWildCards, TBool aUseExtendedChars, TBool aInScanDrive, TBool aAllowLowerCase, TBool aIsForFileCreation)
	{

	__PRINT(_L("IsLegalDosName 16"));

    return DoCheckLegalDosName(aName, anAllowWildCards, aUseExtendedChars, aInScanDrive, aAllowLowerCase, aIsForFileCreation);	
	}

TBool CFatMountCB::FindShortNameL(const TShortName& aName,TEntryPos& anEntryPos)
//
// Returns ETrue and the entryPos of aName if found or EFalse
//
	{
	
	__PRINT(_L("VFAT::CFatMountCB::FindShortNameL"));	
	TFatDirEntry fatEntry;
	TInt count=0;
	FOREVER
		{
		count++;
		ReadDirEntryL(anEntryPos,fatEntry);
		MoveToDosEntryL(anEntryPos,fatEntry);
		if (fatEntry.IsEndOfDirectory())
			break;
		if (IsRootDir(anEntryPos)&&(anEntryPos.iPos+StartOfRootDirInBytes()==(RootDirEnd()-KSizeOfFatDirEntry)))	
			if (fatEntry.IsErased())
				break;//Allows maximum number of entries in root directory
		if (fatEntry.Name()==aName)
			return ETrue;
		MoveToNextEntryL(anEntryPos);
		if (IsRootDir(anEntryPos)&&(StartOfRootDirInBytes()+anEntryPos.iPos==RootDirEnd()))
			break;//Allows maximum number of entries in root directory
		}
	return EFalse;
	}
	
TBool CFatMountCB::IsUniqueNameL(const TShortName& aName,TInt aDirCluster)
//
// Returns ETrue if aName is unique, EFalse if a matching name is found.
//
	{

	__PRINT(_L("VFAT::CFatMountCB::IsUniqueNameL"));	
	TEntryPos entryPos(aDirCluster,0);
	if (FindShortNameL(aName,entryPos))
		return(EFalse);
	return(ETrue);
	}

void CFatMountCB::ReplaceClashingNameL(const TShortName& aNewName,const TEntryPos& anEntryPos)
//
// A legal dos name has been typed that clashes with a computer generated shortname
// Change the shortname to something else.
//
	{

	__PRINT(_L("VFAT::CFatMountCB::ReplaceClashingNameL"));	
	TFatDirEntry entry;
	ReadDirEntryL(anEntryPos,entry);
	__ASSERT_ALWAYS(entry.IsEndOfDirectory()==EFalse,User::Leave(KErrCorrupt));
	entry.SetName(aNewName);
	WriteDirEntryL(anEntryPos,entry);
//	We now need to fix up VFAT entries with a new checksum reflecting new shortname
//	Calculate new checksum
	TUint8 checksum=CalculateShortNameCheckSum(aNewName);
//	Now go back and adjust all VFAT entries corresponding to this shortname
	TEntryPos entryPos=anEntryPos;
	FOREVER
		{
		entryPos.iPos-=KSizeOfFatDirEntry;	
		ReadDirEntryL(entryPos,entry);
		entry.iData[0x0D]=checksum;
		if (entry.iData[0]&0x40)
			break;
		}
	}

TBool CFatMountCB::GenerateShortNameL(TInt aDirCluster,const TDesC& aName,TShortName& aGeneratedName, TBool aForceRandomize)
//
// Generate a legal dos filename as an alias for aName.
// Returns ETrue if aName is a legal dos name.
//
	{

	__PRINT(_L("VFAT::CFatMountCB::GenerateShortNameL"));
	// Given the long file-name "ABCDEFGHI.TXT", EPOC used to generate short 
	// file-names in the following pecking order:
	//     "ABCDEFGH.TXT",
	//     "ABCDEF~0.TXT",
	//     "ABCDEF~1.TXT",
	//     "ABCDEF~2.TXT",
	//     etc.
	// Now, however, EPOC behaves in a more Windows-like manner and 
	// generates short file-names in this pecking order:
	//     "ABCDEF~1.TXT",
	//     "ABCDEF~2.TXT",
	//     "ABCDEF~3.TXT",
	//     "ABCDEF~4.TXT",
	// After failing to find an unused short name 4 times in a row, 
	// a random number is used to speed up the process. So subsequent
	// short-file names become
	//     "ABC~nnnn.TXT"	where nnnn is a random number
	//    
	TBool useTildeSelectively = ETrue;
	TInt endNum = KMaxDuplicateShortName;	//	0xFFFF
	const TInt KMaxNonRandomShortFileNames = 4;

	TInt i = 1;

	TBool randomize = aForceRandomize;
	if (randomize)
		{
		i = (TInt) (Math::Random() & KMaxDuplicateShortName);
		endNum = (i - 1) & KMaxDuplicateShortName;
		}

	while(i != endNum)
		{
		aGeneratedName=DoGenerateShortNameL(aName,i,useTildeSelectively);

		if (IsUniqueNameL(aGeneratedName,aDirCluster))
			break;

		if (i == KMaxNonRandomShortFileNames && !randomize)
			{
			randomize = ETrue;
			i = (TInt) (Math::Random() & KMaxDuplicateShortName);
			endNum = (i - 1) & KMaxDuplicateShortName;
			}
		else if (i == -1)
			{
			useTildeSelectively=EFalse;
			i = 1;
			}
		else
			i = (i + 1) & KMaxDuplicateShortName;
		}

	if (i == endNum)
		User::Leave(KErrAlreadyExists);

	if((i == -1) && IsLegalDosName(aName,EFalse,EFalse,EFalse,EFalse,ETrue))
		{
		// Original file name is a legal 8.3 name
		return(ETrue);
		}
	else
		{
	    return(EFalse);
		}


	}

void TFatDirEntry::InitializeAsVFat(TUint8 aCheckSum)
//
// Initialize a FAT entry as a VFAT filename
//
	{

	Mem::Fill(this,sizeof(SFatDirEntry),0xFF);
	iData[0x0B]=0x0F;
	iData[0x0C]=0x00; iData[0x0D]=aCheckSum;
	iData[0x1A]=0x00; iData[0x1B]=0x00;
	}

void TFatDirEntry::SetVFatEntry(const TDesC& aName,TInt aLen)
//
// Write up to KMaxVFatEntryName unicode chars from aName to the entry
//
	{

	TInt rem=aName.Length()-aLen;
	TPtrC section(aName.Ptr()+aLen,Min(rem,KMaxVFatEntryName));
	TBuf16<KMaxVFatEntryName> buf16;
	buf16.Copy(section);
	if (rem<KMaxVFatEntryName)
		{
		rem++;
		buf16.ZeroTerminate();
		buf16.SetLength(rem); // Zero termination doesn't increase the buf length
		}
	TUint8 orderNo=(TUint8)(aLen/KMaxVFatEntryName+1);
	TInt s=Min(rem,5);
	Mem::Copy(&iData[0x01],buf16.Ptr(),s*2);//Copy up to 10 bytes of buf16 into iData
	TInt offset=s;
	rem-=s;
	s=Min(rem,6);
	Mem::Copy(&iData[0x0E],buf16.Ptr()+offset,s*2);
	offset+=s;
	rem-=s;
	s=Min(rem,2);
	Mem::Copy(&iData[0x1C],buf16.Ptr()+offset,s*2);
	rem-=s;
	if (rem==0)
		orderNo|=0x40;
	iData[0]=orderNo;
	}

void TFatDirEntry::ReadVFatEntry(TDes16& aBuf) const
//
// Read KMaxVFatEntryName unicode chars from the entry
//
	{

	aBuf.SetLength(KMaxVFatEntryName);
	Mem::Copy(&aBuf[0],&iData[0x01],5*2);
	Mem::Copy(&aBuf[5],&iData[0x0E],6*2);
	Mem::Copy(&aBuf[11],&iData[0x1C],2*2);
	}

void CFatMountCB::WriteDirEntryL(TEntryPos& aPos,const TFatDirEntry& aFatDirEntry,const TDesC& aLongName)
//
// Write a VFAT directory entry to disk at position aPos - leave aPos refering to the dos entry
// Assumes sufficient space has been created for it by AddDirEntry.
//
	{

	__PRINT(_L("VFAT::CFatMountCB::WriteDirEntryL"));	
	__ASSERT_DEBUG(aLongName.Length(),Fault(EVFatNoLongName));
	TEntryPos startPos(aPos.iCluster,aPos.iPos);
	TUint8  localBuf[KDefaultSectorSize];
	TUint8 cksum=CalculateShortNameCheckSum(aFatDirEntry.Name());
	TInt numEntries=NumberOfVFatEntries(aLongName.Length())-1; // Excluding dos entry
	// see if all entries written to one sector
	// single sector writes not supported if sector size>default size 
	TInt dosOffset=numEntries<<KSizeOfFatDirEntryLog2;
	TInt absolutePos=(aPos.iCluster<<ClusterSizeLog2())+ClusterRelativePos(aPos.iPos);
	TBool isSameSector=(((absolutePos^(absolutePos+dosOffset))>>SectorSizeLog2())==0 && ((TUint)(1<<SectorSizeLog2())<=KDefaultSectorSize));
	TFatDirEntry vFatEntry;
	vFatEntry.InitializeAsVFat(cksum);
	TInt offset=0;
	while (numEntries--)
		{
		vFatEntry.SetVFatEntry(aLongName,KMaxVFatEntryName*numEntries);//	KMaxVFatEntryName=13
		if(isSameSector)
			{
			Mem::Copy(&localBuf[offset],&vFatEntry,KSizeOfFatDirEntry);
			offset+=KSizeOfFatDirEntry;
			MoveToNextEntryL(aPos);
			}
		else
			{
			WriteDirEntryL(aPos,vFatEntry);
			MoveToNextEntryL(aPos);
			}
		}
	if(isSameSector)
		{
		Mem::Copy(&localBuf[offset],&aFatDirEntry,KSizeOfFatDirEntry);
		
        //-- use special interface to access FAT directory file
        DirWriteL(startPos,TPtrC8(&localBuf[0],dosOffset+KSizeOfFatDirEntry));
        }
	else
		WriteDirEntryL(aPos,aFatDirEntry);
	}

void CFatMountCB::EraseDirEntryL(TEntryPos aPos,const TFatDirEntry& aFirstEntry)
//
// Mark all entries in a VFat directory entry as erased
//
	{
	__PRINT(_L("VFAT::CFatMountCB::EraseDirEntryL"));
	TInt numEntries=0;
	if (aFirstEntry.IsVFatEntry())
		numEntries=aFirstEntry.NumFollowing();
	if(IsRuggedFSys()&&numEntries)
		{
		TInt count=numEntries;
		TEntryPos pos=aPos;
		while(count--)
			MoveToNextEntryL(pos);
		EraseDirEntryL(pos);
		numEntries--;
		}
	FOREVER
		{
		EraseDirEntryL(aPos);
		if (!numEntries--)
			break;
		MoveToNextEntryL(aPos);
		}
	}


void  LocaleUtils::ConvertFromUnicodeL(TDes8& aForeign, const TDesC16& aUnicode, TFatUtilityFunctions::TOverflowAction aOverflowAction)
//
// Convert the volume label using the algorithm specified in the current locale-DLL.
//
	{
	if(aOverflowAction == TFatUtilityFunctions::EOverflowActionLeave)
		{
		GetCodePage().ConvertFromUnicodeL(aForeign, aUnicode, TCodePageUtils::EOverflowActionLeave);
		}
	else
		{
		GetCodePage().ConvertFromUnicodeL(aForeign, aUnicode, TCodePageUtils::EOverflowActionTruncate);
		}
	}

void  LocaleUtils::ConvertToUnicodeL(TDes16& aUnicode, const TDesC8& aForeign, TFatUtilityFunctions::TOverflowAction aOverflowAction)
//
// Convert the volume label using the algorithm specified in the current locale-DLL.
//
	{
	if(aOverflowAction == TFatUtilityFunctions::EOverflowActionLeave)
		{
		GetCodePage().ConvertToUnicodeL(aUnicode, aForeign, TCodePageUtils::EOverflowActionLeave);
		}
	else
		{
		GetCodePage().ConvertToUnicodeL(aUnicode, aForeign, TCodePageUtils::EOverflowActionTruncate);
		}
	}

TBool LocaleUtils::IsLegalShortNameCharacter(TUint aCharacter,TBool aUseExtendedChars)
//
// Convert the volume label using the algorithm specified in the current locale-DLL.
//
	{
	return GetCodePage().IsLegalShortNameCharacter(aCharacter, aUseExtendedChars);
	}
