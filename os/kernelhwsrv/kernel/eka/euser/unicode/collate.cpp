// Copyright (c) 1997-2009 Nokia Corporation and/or its subsidiary(-ies).
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
// The Unicode collation system.
// 
//

#include "collateimp.h"
#include "foldtable.inl"

#include <collate.h>
#include <unicode.h>
#include "u32std.h"

// maximum size of string which has its own sort key
// 16 instead of 8, in case all supplementary characters
static const TInt KKeyedStringBufferSize = 16;


inline TText16 GetHighSurrogate(TUint aChar)
/**
Retrieve the high surrogate of a supplementary character.

@param aChar The 32-bit code point value of a Unicode character.

@return High surrogate of aChar, if aChar is a supplementary character; 
        aChar itself, if aChar is not a supplementary character.
*/
	{
	return STATIC_CAST(TText16, 0xD7C0 + (aChar >> 10));
	}

inline TText16 GetLowSurrogate(TUint aChar)
/**
Retrieve the low surrogate of a supplementary character.

@param aChar The 32-bit code point value of a Unicode character.

@return Low surrogate of aChar, if aChar is a supplementary character; 
        zero, if aChar is not a supplementary character.
*/
	{
	return STATIC_CAST(TText16, 0xDC00 | (aChar & 0x3FF));
	}

inline TUint JoinSurrogate(TText16 aHighSurrogate, TText16 aLowSurrogate)
/**
Combine a high surrogate and a low surrogate into a supplementary character.

@return The 32-bit code point value of the generated Unicode supplementary
        character.
*/
	{
	return ((aHighSurrogate - 0xD7F7) << 10) + aLowSurrogate;
	}

// Creates a one or two collation keys sequence corresponding to the input character.
// Returns the number of keys output.
static TInt CreateDefaultCollationKeySequence(TInt aChar, TCollationKey* aBuffer)
	{
	if (aChar >= 0x3400 && aChar <= 0x9FFF)	// CJK
		{
		aBuffer[0].iLow = (TUint32)aChar << 16 | 0x0105;
		aBuffer[0].iHigh = aChar;
		return 1;//Collation key sequence consists of 1 key
		}
	aBuffer[0].iLow = 0xFF800000 | ((aChar << 1) & 0x3F0000) | 0x0104;	// no stop bit
	aBuffer[0].iHigh = 1;
	aBuffer[1].iLow = (aChar << 16) | 0x80000105;		// stop bit
	aBuffer[1].iHigh = 0;
	return 2;//Collation key sequence consists of 2 keys
	}

// Finds a character's key in the main index, or returns -1 if it is not there
static TInt FindCollationKeyIndex(TInt aChar, const TCollationKeyTable& aTable)
	{
	TInt n = aTable.iIndices;
	const TUint32 *base = aTable.iIndex;
	const TUint32 *start = base;
	const TUint32 *end = aTable.iIndex + n - 1;
	const TUint32 *p = base;
	TInt currentCharLength = 0;

	while (n > 0)
		{
		TInt pivot = n / 2;
		p += pivot;
		if ((p < start) || (p > end)) 
			{
			break;
			}
		TInt c = *p >> 16;
		if (IsHighSurrogate( (TText16)c ))
			{
			if ((p < end) && (IsLowSurrogate( (TText16)((*(p+1))>>16) )))
				{
				currentCharLength = 2;
				c = JoinSurrogate( (TText16)(*p>>16), (TText16)((*(p+1))>>16) );
				}
			}
		else if (IsLowSurrogate( (TText16)c ))
			{
			if ((p > start) && (IsHighSurrogate( (TText16)((*(p-1))>>16) )))
				{
				p--;
				pivot = pivot - 1;
				currentCharLength = 2;
				c = JoinSurrogate( (TText16)(*p>>16), (TText16)((*(p+1))>>16) );
				}
			}
		else
			{
			currentCharLength = 1;
			}
		if (aChar == c)		// found it
			{
			return *p & 0xFFFF;
			}
		if (aChar < c)		// it's before
			{
			n = pivot;
			}
		else					// it's after
			{
			ASSERT(currentCharLength != 0);
			base = p + currentCharLength;
			n -= pivot + currentCharLength;
			}
		p = base;
		}
	return -1;
	}

static void ProcessKeys(TUint32& aKey1, TUint32& aKey2, TUint aFlags)
	{
	if (aFlags & TCollationMethod::EFoldCase)
		{
		static const TUint case_fold_table[21] =
			{ 0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x2, 0x3, 0x4, 0x5, 0x6,
			  0xD, 0xE, 0xF, 0x10, 0x11, 0x12, 0x13, 0x14 };
		aKey1 = case_fold_table[aKey1];
		aKey2 = case_fold_table[aKey2];
		}					
	if (aFlags & TCollationMethod::ESwapCase)
		{
		static const TUint case_swap_table[21] =
			{ 0, 0x1, 0x8, 0x9, 0xA, 0xB, 0xC, 0x7, 0x2, 0x3, 0x4, 0x5, 0x6,
			  0xD, 0xE, 0xF, 0x10, 0x11, 0x12, 0x13, 0x14 };
		aKey1 = case_swap_table[aKey1];
		aKey2 = case_swap_table[aKey2];
		}
	if (aFlags & TCollationMethod::ESwapKana)
		{
		static const TUint kana_swap_table[21] =
			{ 0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8, 0x9, 0xA, 0xB, 0xC,
			  0x13, 0x14, 0xD, 0xE, 0xF, 0x10, 0x11, 0x12 };
		aKey1 = kana_swap_table[aKey1];
		aKey2 = kana_swap_table[aKey2];
		}
	}

// Returns the position of the character in the string, or aLength if it is not present.
// If aChar is found but it is preceded by aEscapeChar (aEscapeChar != 0), then the search continues.
static TInt FindCharacter(TInt aChar, TInt aEscapeChar, const TUint16* aString, TInt aLength)
	{
	TBool isEscaped = EFalse;
	for(TInt pos=0;pos!=aLength;++pos,++aString)
		{
		if(isEscaped) 
			{ 
			isEscaped = EFalse; 
			} 
		else if(*aString == aEscapeChar) 
			{ 
			isEscaped = ETrue; 
			} 
		else if(*aString == aChar) 
			{ 
			if(!isEscaped)
				{
				return pos; 
				}
			}		
		}
	return aLength;
	}

/*
The standard collation data, containing keys for all the WGL4 characters, plus
commonly-used control characters and spaces. Generated by COLTAB.
*/
static const TUint32 TheKey[] = 
	{
	0x21e0112,0x21e0113,0x2260112,0x2260112,0x2260113,0x2740112,0x2740113,0x6c60178,
	0x266017a,0x6c70179,0x6c60178,0x266017a,0x6c90179,0x6c60178,0x266017a,0x6cd0179,
	0x6c80178,0x266017a,0x6c90179,0x6c80178,0x266017a,0x6cd0179,0x6ca0178,0x266017a,
	0x6cd0179,0x6cc0178,0x266017a,0x6cd0179,0x6f70110,0x2650112,0x8050111,0x74b0110,
	0x78d0111,0x74b0110,0x7bd0111,0x78d0110,0x7a10111,0x78d0128,0x7a10129,0x7bd0110,
	0x2290113,0x7bd0128,0x2290113,0x7ed0128,0x8050111,0x805dd10,0x71f0111,0x805dd28,
	0x71f0129,0x85ddd10,0x85d0111,0x8750150,0x7e50151,0x9060110,0x7ed0111,0x3,
	0x201010b,0x202010b,0x203010b,0x204010b,0x205010b,0x206010b,0x207010b,0x208010b,
	0x209010b,0x2090113,0x209016f,0x209020b,0x209130b,0x209160b,0x209180b,0x2091d0b,
	0x209240b,0x209280b,0x2092a0b,0x2092f0b,0x209330b,0x209360b,0x209390b,0x2093b0b,
	0x2093f0b,0x2096b0b,0x20b010b,0x20c010b,0x20d010b,0x20d016f,0x20e010b,0x20f010b,
	0x210010b,0x211010b,0x214010b,0x21a010b,0x21c010b,0x21e010b,0x21f010b,0x221010b,
	0x222010b,0x226010b,0x229010b,0x22d010b,0x22e010b,0x22f010b,0x230010b,0x231010b,
	0x232010b,0x233010b,0x234010b,0x235010b,0x236010b,0x237010b,0x23c010b,0x23d010b,
	0x23e010b,0x23f010b,0x240010b,0x241010b,0x242010b,0x243010b,0x25e010b,0x25f010b,
	0x260010b,0x261010b,0x262010b,0x263010b,0x265010b,0x266010b,0x267010b,0x268010b,
	0x269010b,0x26a010b,0x26c010b,0x26e010b,0x26f010b,0x270010b,0x274010b,0x2ac010b,
	0x2ad010b,0x2af010b,0x2d6010b,0x2ff010b,0x300010b,0x301010b,0x302010b,0x303010b,
	0x304010b,0x317010b,0x35c010b,0x35f010b,0x366010b,0x368010b,0x369010b,0x36a010b,
	0x36b010b,0x36c010b,0x36d010b,0x36e010b,0x36f010b,0x370010b,0x371010b,0x372010b,
	0x374010b,0x375010b,0x378010b,0x37c010b,0x37d010b,0x381010b,0x382010b,0x38a010b,
	0x38c010b,0x3a2010b,0x3b9010b,0x3bb010b,0x3bc010b,0x42f010b,0x43d010b,0x44d010b,
	0x44e010b,0x4d6010b,0x4d8010b,0x4e2010b,0x4e6010b,0x4ea010b,0x4ee010b,0x4f2010b,
	0x4fa010b,0x502010b,0x50a010b,0x512010b,0x526010b,0x527010b,0x528010b,0x529010b,
	0x52a010b,0x52b010b,0x52c010b,0x52d010b,0x52e010b,0x52f010b,0x530010b,0x531010b,
	0x532010b,0x533010b,0x534010b,0x535010b,0x536010b,0x537010b,0x538010b,0x539010b,
	0x53a010b,0x53b010b,0x53c010b,0x53d010b,0x53e010b,0x53f010b,0x540010b,0x541010b,
	0x542010b,0x556010b,0x55a010b,0x55e010b,0x562010b,0x566010b,0x567010b,0x568010b,
	0x569010b,0x56c010b,0x56d010b,0x576010b,0x577010b,0x578010b,0x57e010b,0x586010b,
	0x588010b,0x590010b,0x596010b,0x597010b,0x59b010b,0x5a4010b,0x5a5010b,0x5b2010b,
	0x5f0010b,0x5f1010b,0x5f2010b,0x5f6010b,0x5f8010b,0x616010b,0x619010b,0x61b010b,
	0x61c010b,0x620010b,0x621010b,0x6b4010b,0x6b5010b,0x1309,0x1609,0x1809,
	0x1d09,0x2209,0x2409,0x2809,0x2f09,0x3009,0x3309,0x3609,
	0x3909,0x3b09,0x4109,0x2c20109,0x2c30109,0x2c40109,0x2c50109,0x2c60109,
	0x2cd0109,0x2ce0109,0x2d10109,0x2d50109,0x2fa0109,0x6c50109,0x6c60109,0x6c60151,
	0x6c70109,0x6c70151,0x6c80109,0x6c80151,0x6c90109,0x6ca0109,0x6cb0109,0x6cc0109,
	0x6cd0109,0x6ce0109,0x6cf0109,0x6cf0121,0x6cf0151,0x6d30109,0x6d30121,0x6e30109,
	0x6e30121,0x6f70109,0x6f70121,0x7030109,0x7030121,0x7070109,0x7070121,0x7170109,
	0x7170121,0x71f0109,0x71f0121,0x74b0109,0x74b0121,0x74f0109,0x7530109,0x7530121,
	0x7730109,0x7730121,0x77f0109,0x77f0121,0x78d0109,0x78d0121,0x7910109,0x7a10109,
	0x7a10121,0x7b10109,0x7b10121,0x7bd0109,0x7bd0115,0x7bd0121,0x7c50109,0x7c50121,
	0x7e50109,0x7e50121,0x7ed0109,0x7ed0121,0x7ed0151,0x8010109,0x8010121,0x8050109,
	0x8050121,0x8050151,0x80d0109,0x80d0121,0x81d0109,0x81d0121,0x8290109,0x8290121,
	0x8310109,0x8350109,0x8350121,0x85d0109,0x85d0121,0x85dde11,0x8750109,0x8750121,
	0x8790109,0x8790121,0x88d0109,0x88d0121,0x8a50109,0x8a50121,0x8b10109,0x8b10121,
	0x8b90109,0x8b90121,0x8bd0109,0x8bd0121,0x8c90109,0x8c90121,0x8e90109,0x8e90121,
	0x9360109,0x9360121,0x9370109,0x9370121,0x9380109,0x9380121,0x9390109,0x9390121,
	0x93a0109,0x93a0121,0x93d0109,0x93d0121,0x93e0109,0x93e0121,0x93f0109,0x93f0121,
	0x9400109,0x9400121,0x9420109,0x9420121,0x9430109,0x9430121,0x9440109,0x9440111,
	0x9440121,0x9450109,0x9450121,0x9460109,0x9460121,0x9470109,0x9470121,0x9480109,
	0x9480121,0x94a0109,0x94a0121,0x94b0109,0x94b0121,0x94c0109,0x94c0121,0x94d0109,
	0x94d0121,0x94e0109,0x94e0121,0x94f0109,0x94f0121,0x9500109,0x9500121,0x9510109,
	0x9510121,0x95a0109,0x95a0121,0x9660109,0x9660121,0x96a0109,0x96a0121,0x96e0109,
	0x96e0121,0x9720109,0x9720121,0x97e0109,0x97e0121,0x9820109,0x9820121,0x98a0109,
	0x98a0121,0x98e0109,0x98e0121,0x9920109,0x9920121,0x99a0109,0x99a0121,0x99e0109,
	0x99e0121,0x9a60109,0x9a60121,0x9aa0109,0x9aa0121,0x9ae0109,0x9ae0121,0x9b20109,
	0x9b20121,0x9ca0109,0x9ca0121,0x9ce0109,0x9ce0121,0x9d20109,0x9d20121,0x9d60109,
	0x9d60121,0x9e60109,0x9e60121,0x9ea0109,0x9ea0121,0x9f20109,0x9f20121,0x9fe0109,
	0x9fe0121,0xa020109,0xa020121,0xa0a0109,0xa0a0121,0xa120109,0xa120121,0xa160109,
	0xa160121,0xa260109,0xa260121,0xa2a0109,0xa2a0121,0xa460109,0xa460121,0xa4e0109,
	0xa4e0121,0xa660109,0xa660121,0xa6a0109,0xa6a0121,0xa6e0109,0xa6e0121,0xa720109,
	0xa720121,0xa760109,0xa760121,0xa7a0109,0xa7a0121,0xa820109,0xa820121,0xa860109,
	0xa860121,0xa8a0109,0xa8a0121,
	};

static const TUint32 TheIndex[] = 
	{
	0x37,0x10037,0x20037,0x30037,0x40037,0x50037,0x60037,0x70037,
	0x80037,0x90038,0xa0039,0xb003a,0xc003b,0xd003c,0xe0037,0xf0037,
	0x100037,0x110037,0x120037,0x130037,0x140037,0x150037,0x160037,0x170037,
	0x180037,0x190037,0x1a0037,0x1b0037,0x1c0037,0x1d0037,0x1e0037,0x1f0037,
	0x200040,0x21005d,0x22006a,0x230080,0x24010d,0x250081,0x26007f,0x270063,
	0x280070,0x290071,0x2a007b,0x2b0096,0x2c005a,0x2d0053,0x2e0061,0x2f007c,
	0x300115,0x310116,0x320118,0x33011a,0x34011c,0x35011d,0x36011e,0x37011f,
	0x380120,0x390121,0x3a005c,0x3b005b,0x3c009a,0x3d009b,0x3e009c,0x3f005f,
	0x40007a,0x410123,0x420128,0x43012a,0x44012c,0x450132,0x460134,0x470137,
	0x480139,0x49013d,0x4a0140,0x4b0142,0x4c0145,0x4d0149,0x4e014b,0x4f0150,
	0x500155,0x510157,0x52015a,0x53015c,0x54015f,0x550163,0x560165,0x570167,
	0x580169,0x59016b,0x5a016d,0x5b0072,0x5c007e,0x5d0073,0x5e0047,0x5f0043,
	0x600045,0x610122,0x620127,0x630129,0x64012b,0x650131,0x660133,0x670136,
	0x680138,0x69013c,0x6a013f,0x6b0141,0x6c0143,0x6d0148,0x6e014a,0x6f014f,
	0x700154,0x710156,0x720159,0x73015b,0x74015e,0x750162,0x760164,0x770166,
	0x780168,0x79016a,0x7a016c,0x7b0074,0x7c009e,0x7d0075,0x7e00a0,0xa00042,
	0xa1005e,0xa2010c,0xa3010e,0xa4010b,0xa5010f,0xa6009f,0xa70076,0xa80049,
	0xa90078,0xaa0124,0xab006e,0xac009d,0xad0052,0xae0079,0xaf004f,0xb0008a,
	0xb10097,0xb20119,0xb3011b,0xb40044,0xb50187,0xb60077,0xb70062,0xb8004d,
	0xb90117,0xba0151,0xbb006f,0xbc000a,0xbd0007,0xbe0010,0xbf0060,0xc60126,
	0xd00130,0xd70099,0xd80153,0xde016f,0xdf0031,0xe60125,0xf0012f,0xf70098,
	0xf80152,0xfe016e,0x110012e,0x111012d,0x126013b,0x127013a,0x131013e,0x1320025,
	0x1330023,0x1380158,0x13f0029,0x1400027,0x1410147,0x1420146,0x1490035,0x14a014e,
	0x14b014d,0x152002f,0x153002d,0x1660161,0x1670160,0x17f015d,0x1920135,0x2c60087,
	0x2c70088,0x2c90089,0x2d80046,0x2d9004c,0x2da0048,0x2db004e,0x2dd004b,0x30000fe,
	0x30100fd,0x3020100,0x3030105,0x3040109,0x30600ff,0x3070106,0x3080103,0x30a0102,
	0x30b0104,0x30c0101,0x30d010a,0x3270107,0x3280108,0x3840044,0x385004a,0x3870062,
	0x3910171,0x3920173,0x3930175,0x3940177,0x3950179,0x396017b,0x397017d,0x398017f,
	0x3990181,0x39a0183,0x39b0185,0x39c0188,0x39d018a,0x39e018c,0x39f018e,0x3a00190,
	0x3a10192,0x3a30194,0x3a40196,0x3a50198,0x3a6019a,0x3a7019c,0x3a8019e,0x3a901a0,
	0x3b10170,0x3b20172,0x3b30174,0x3b40176,0x3b50178,0x3b6017a,0x3b7017c,0x3b8017e,
	0x3b90180,0x3ba0182,0x3bb0184,0x3bc0186,0x3bd0189,0x3be018b,0x3bf018d,0x3c0018f,
	0x3c10191,0x3c20193,0x3c30193,0x3c40195,0x3c50197,0x3c60199,0x3c7019b,0x3c8019d,
	0x3c9019f,0x40201ae,0x40401b2,0x40501b8,0x40601bc,0x40801be,0x40901c4,0x40a01ca,
	0x40b01d6,0x40f01e2,0x41001a2,0x41101a4,0x41201a6,0x41301a8,0x41401ac,0x41501b0,
	0x41601b4,0x41701b6,0x41801ba,0x41a01c0,0x41b01c2,0x41c01c6,0x41d01c8,0x41e01cc,
	0x41f01ce,0x42001d0,0x42101d2,0x42201d4,0x42301d8,0x42401da,0x42501dc,0x42601de,
	0x42701e0,0x42801e4,0x42901e6,0x42a01e8,0x42b01ea,0x42c01ec,0x42d01ee,0x42e01f0,
	0x42f01f2,0x43001a1,0x43101a3,0x43201a5,0x43301a7,0x43401ab,0x43501af,0x43601b3,
	0x43701b5,0x43801b9,0x43a01bf,0x43b01c1,0x43c01c5,0x43d01c7,0x43e01cb,0x43f01cd,
	0x44001cf,0x44101d1,0x44201d3,0x44301d7,0x44401d9,0x44501db,0x44601dd,0x44701df,
	0x44801e3,0x44901e5,0x44a01e7,0x44b01e9,0x44c01eb,0x44d01ed,0x44e01ef,0x44f01f1,
	0x45201ad,0x45401b1,0x45501b7,0x45601bb,0x45801bd,0x45901c3,0x45a01c9,0x45b01d5,
	0x45f01e1,0x49001aa,0x49101a9,0x20000041,0x20010041,0x20020041,0x20030041,0x20040041,
	0x20050041,0x20060041,0x20070042,0x20080041,0x20090041,0x200a0041,0x200b003f,0x200c0037,
	0x200d0037,0x200e0037,0x200f0037,0x20100054,0x20110055,0x20120056,0x20130057,0x20140058,
	0x20150059,0x20170051,0x20180064,0x20190065,0x201a0066,0x201b0067,0x201c006b,0x201d006c,
	0x201e006d,0x20200083,0x20210084,0x20220085,0x20260002,0x2028003d,0x2029003e,0x202a0037,
	0x202b0037,0x202c0037,0x202d0037,0x202e0037,0x20300082,0x20320086,0x20330005,0x20390068,
	0x203a0069,0x203c0000,0x203e0050,0x2044007d,0x207f014c,0x20a30110,0x20a40111,0x20a70112,
	0x20ac0113,0x2105001c,0x21130144,0x2116002b,0x21220033,0x212601a0,0x212e0114,0x215b000d,
	0x215c0013,0x215d0016,0x215e0019,0x2190008b,0x2191008d,0x2192008c,0x2193008e,0x2194008f,
	0x21950090,0x21a80091,0x22020092,0x22060093,0x220f0094,0x22110095,0x221200a1,0x221500a2,
	0x221900a3,0x221a00a4,0x221e00a5,0x221f00a6,0x222900a7,0x222b00a8,0x224800a9,0x226100aa,
	0x226400ab,0x226500ac,0x230200ad,0x231000ae,0x232000af,0x232100b0,0x250000b1,0x250200b2,
	0x250c00b3,0x251000b4,0x251400b5,0x251800b6,0x251c00b7,0x252400b8,0x252c00b9,0x253400ba,
	0x253c00bb,0x255000bc,0x255100bd,0x255200be,0x255300bf,0x255400c0,0x255500c1,0x255600c2,
	0x255700c3,0x255800c4,0x255900c5,0x255a00c6,0x255b00c7,0x255c00c8,0x255d00c9,0x255e00ca,
	0x255f00cb,0x256000cc,0x256100cd,0x256200ce,0x256300cf,0x256400d0,0x256500d1,0x256600d2,
	0x256700d3,0x256800d4,0x256900d5,0x256a00d6,0x256b00d7,0x256c00d8,0x258000d9,0x258400da,
	0x258800db,0x258c00dc,0x259000dd,0x259100de,0x259200df,0x259300e0,0x25a000e1,0x25a100e2,
	0x25aa00e3,0x25ab00e4,0x25ac00e5,0x25b200e6,0x25ba00e7,0x25bc00e8,0x25c400e9,0x25ca00ea,
	0x25cb00eb,0x25cf00ec,0x25d800ed,0x25d900ee,0x25e600ef,0x263a00f0,0x263b00f1,0x263c00f2,
	0x264000f3,0x264200f4,0x266000f5,0x266300f6,0x266500f7,0x266600f8,0x266a00f9,0x266b00fa,
	0xfb01001f,0xfb020021,0xfeff0037,0xfffc00fb,0xfffd00fc,
	};

static const TCollationKeyTable TheStandardTable = 
	{ TheKey, TheIndex, 517, NULL, NULL, 0 };

const TCollationKeyTable* StandardCollationMethod()
	{
	return &TheStandardTable;
	};

inline void Increment(TUint16 const *& aPointer,TBool aNarrow)
	{
	aPointer =  aNarrow ? (const TUint16*)(((const TUint8*)aPointer) + 1) : aPointer + 1;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////
// TCollationValueIterator
/////////////////////////////////////////////////////////////////////////////////////////////////

/**
Initializes TCollationValueIterator object with a new character sequence.
@param aSourceIt An iterator used to access the input character (non-normalized or 
                 normalized) sequence.
@internalComponent
*/
void TCollationValueIterator::SetSourceIt(TUTF32Iterator& aSourceIt)
	{
    iCurrentKeyPos = 0;
	iKey.iKeys = 0;
    iDecompStrIt.Set(aSourceIt);
	}

/**
Gets current raw key.
Note: the method may move the iterator one or more positions forward if there are no produced
      collation keys.
@param aKey A reference to a TCollationKey object, initialized with the
            current collation key after the call, if there is available key.
@return ETrue Successfull call, aKey initialized with the current collation key,
        EFalse - the iteration has come to the end.
@internalComponent
*/
TBool TCollationValueIterator::GetCurrentKey(TCollationKey& aKey)
	{
    ASSERT(iCurrentKeyPos <= iKey.iKeys);
	if(!ProduceCollationKeys())
        {
        return EFalse;
        }
	aKey = iKey.iKey[iCurrentKeyPos];
	return ETrue;
	}

/**
Gets current key at the specified level.
Note: the method may move the iterator one or more positions forward if there are no produced
      collation keys.
@param aLevel Desired level of the collation key: 0..3
@param aKey A reference to TUint32 where the retrieved key will be stored.
@return ETrue Success, EFalse - end of the iteration.
@internalComponent
*/
TBool TCollationValueIterator::GetCurrentKey(TInt aLevel, TUint32& aKey)
	{
	TCollationKey rawKey;
	if(GetCurrentKey(rawKey))
        {
	    //Key values are ignored if their ignore bit is set and the level is less than 3: in other words, the
	    //actual Unicode value is never ignored. This does NOT conform to the system of alternate weightings
	    //described in Unicode Technical Report 10, and will probably have to be changed.
        aKey = (aLevel < 3 && (rawKey.iLow & TCollationKeyTable::EIgnoreFlag) && !IgnoringNone()) ? 0 : rawKey.Level(aLevel);
        return ETrue;
        }
    return EFalse;
	}

/**
The method iterates through the controlled character sequence and tries to find first non-zero
corresponding collation key at the specified level.
@param aLevel Desired level of the collation key: 0..3
@return Non-zero collation key value or 0 if the iteration has come to the end.
@internalComponent
*/
TUint32 TCollationValueIterator::GetNextNonZeroKey(TInt aLevel)
	{
    TUint32 key = 0;
	while(GetCurrentKey(aLevel, key) && key == 0)
		{
		Increment();
		}
	return key;
	}

/**
The method determines wheter the specified as a parameter character matches current iterator's
character.
If there is a match, the iterator will be moved one position forward.
Note: the method may move the iterator one or more positions forward if there are no produced
      collation keys.
@param aMatch The character to compare with the current iterator's character.
@return ETrue The characters match, EFalse otherwise (or the iteration has come to the end).
@internalComponent
*/
TBool TCollationValueIterator::MatchChar(TChar aMatch)
	{
    TUint32 key;
    if(GetCurrentKey(3, key))
        {
	    // Find a match for the quaternary key.. will probably be the unicode value
	    // This is a bit poor.
	    if(aMatch == key)
		    {
		    Increment();
		    return ETrue;
		    }
        }
	return EFalse;
	}

/**
Note: the method may move the iterator one or more positions forward if there are no produced
      collation keys.
@return The method returns ETrue if the iterator is at a combining character, EFalse otherwise
        (or the iterator has come to the end)
@internalComponent
*/
TBool TCollationValueIterator::AtCombiningCharacter()
	{
	TCollationKey rawKey;
	if(!GetCurrentKey(rawKey))
        {
		return EFalse;		// iteration ended
        }
	return rawKey.IsStarter() ? (TBool)EFalse : (TBool)ETrue;
	}

/**
Skips the following combining characters if they are.
Note: the method may move the iterator one or more positions forward.
@return The number of skipped combining characters.
@internalComponent
*/
TInt TCollationValueIterator::SkipCombiningCharacters()
	{
	TInt count;
	for(count=0;AtCombiningCharacter();++count)
        {
		Increment();
        }
	return count;
	}

/**
Moves the iterator one step forward making the next collation key available for getting
using GetCurrentKey().
@return ETrue Successfull call, there is a collation key available.
        EFalse - the iteration has come to the end.
@internalComponent
@see TCollationValueIterator::GetCurrentKey()
*/
TBool TCollationValueIterator::Increment()
	{
	ASSERT(iCurrentKeyPos <= iKey.iKeys);
	if(!ProduceCollationKeys())
        {
        return EFalse;
        }
	++iCurrentKeyPos;
	return ETrue;
	}

/**
Returns the position in the underlying string of the iteration,
if this is well defined. It is not well defined if either we are
half way through keys defined as a string in the collation table
or if we are half way through a canonically reordered sequence.
@return The position in the underlying string if this is well
	defined, or 0 if it is not.
*/
const TText16* TCollationValueIterator::CurrentPositionIfAtCharacter()
	{
	if (!ProduceCollationKeys())
		return iCurrentPosition;
	return iCurrentKeyPos == 0? iCurrentPosition : 0;
	}

/**
Produces the longest possible collation keys sequence using the decomposed character sequence,
pointed by iDecompStrIt iterator. But this will happen only if all keys from iKey array are 
consumed.
@return ETrue Successfull call, iKey initialized with the produced collation keys sequence,
        EFalse - the iteration has come to the end.
@internalComponent
*/
TBool TCollationValueIterator::ProduceCollationKeys()
	{
    //iKey.iKeys represents the keys count in iKey array, so load more keys, only if all 
    //collation keys are already consumed.
    if(iCurrentKeyPos == iKey.iKeys)
        {
		iCurrentPosition = iDecompStrIt.CurrentPositionIfAtCharacter();
		if(iDecompStrIt.AtEnd())
            {//No more characters in the input decomposed canonical string
			return EFalse;
            }
        //Try to get the next collation key sequence. There should be at least one key.
		GetNextRawKeySequence();
	    ASSERT(iKey.iKeys > 0);
        iCurrentKeyPos = 0;
        }
	return ETrue;
	}

/**
Consume zero or more characters from the input and convert them into zero or more collation keys.
@internalComponent
*/
void TCollationValueIterator::GetNextRawKeySequence()
	{
    //Store the first character combining class type for later use.
    TChar firstChar = iDecompStrIt.Get(0);
    TBool combining = !::IsBaseCharacter(firstChar);
	// Initialise.
	iKey.iCharactersConsumed = 0;
	iKey.iKeys = 0;
	// See if the override table has a key for the current collation unit.
	if(iMethod.iOverrideTable)
        {
		GetKeyFromTable(iMethod.iOverrideTable);
        }
	// If not, try the main table.
	if(iKey.iCharactersConsumed == 0)
        {
		GetKeyFromTable(iMethod.iMainTable);
        }
	//If no key was found use a default value depending on the current character.
	//For CJK characters:
	//the Unicode value itself as the primary key and 1 as the secondary and tertiary keys;
	//the lower 16 bits end up as 0x0105 because the bottom two bits are used for the ignorable bit,
	//which is clear, and the stop bit, which is set.
	//For other characters:
	//Return two keys containing the 21 bits of the character code (anything from 0 to 0x10FFFF), as
	//explained in Unicode Technical Report 10.
	if(iKey.iCharactersConsumed == 0)
		{
		iKey.iCharactersConsumed = 1;
		iDecompStrIt.Next(1);
        iKey.iKeys = ::CreateDefaultCollationKeySequence(firstChar, iKey.iKey);
		}
	if(!combining)
        {
		iKey.iKey[0].iHigh |= (TUint32)TCollationKey::KFlagIsStarter;
        }
	}

/**
Search for the string aText.
Put the key index in aIndex if found, otherwise set aIndex to -1.
If the sought string might be a prefix to a key in the table set aPossiblePrefix to TRUE.
@internalComponent
*/
static void GetStringKey(const TCollationKeyTable* aTable,const TText* aText,TInt aLength,
						 TInt& aIndex,TBool& aPossiblePrefix)
	{
	aIndex = -1;
	aPossiblePrefix = EFalse;
	TInt n =	aTable->iStringIndices;
	const TUint32* base = aTable->iStringIndex;
	const TUint32* p = base;
	TInt pivot;
	while (n > 0)
		{
		pivot = n / 2;
		p += pivot;
		TUint16 string_index = (TUint16)(*p >> 16);
		const TText* cur_text = aTable->iString + string_index + 1;
		TInt cur_length = aTable->iString[string_index];
		TInt order = TUnicode::Compare(aText,aLength,cur_text,cur_length);
		if (order == 0) // found it
			{
			aIndex = *p & 0xFFFF;
			aPossiblePrefix = ETrue;
			break;
			}
		if (order < 1 && !aPossiblePrefix)
			{
			if (aLength < cur_length && TUnicode::Compare(aText,aLength,cur_text,aLength) == 0)
				aPossiblePrefix = ETrue;
			n = pivot;
			}
		else
			{
			base = p + 1;
			n -= pivot + 1;
			}
		p = base;
		}
	}

/**
Consumes output from iDecompStrIt, produces list of keys in iKey.
@param aTable A const pointer to the collation key table used by the method.
@internalComponent
*/
void TCollationValueIterator::GetKeyFromTable(const TCollationKeyTable* aTable)
	{
    ASSERT(aTable != NULL);
	iKey.iCharactersConsumed = 0;
	iKey.iKeys = 0;

	TInt cur_char = iDecompStrIt.Get(0);

	// Find the longest matching string.
	TInt index = -1;
	if(aTable->iStringIndices > 0)
		{
		TInt moved = 0;
		TText text[KKeyedStringBufferSize];
		TInt textLen = 0;
		if (cur_char <= 0xFFFF)
			{
			text[textLen++] = static_cast <TText> (cur_char);
			}
		else 
			{
			text[textLen++] = GetHighSurrogate(cur_char);
			text[textLen++] = GetLowSurrogate(cur_char);
			}
		TBool possible_prefix = ETrue;
		for(TInt i = 1; (i < KKeyedStringBufferSize) && possible_prefix; i++)
			{
			++moved;
			TInt c = iDecompStrIt.Get(i);//get the next character
			if(c == -1)
                {
				break;
                }
			if (c <= 0xFFFF)
				{
				text[textLen++] = static_cast <TText> (c);
				}
			else
				{
				text[textLen++] = GetHighSurrogate(c);
				text[textLen++] = GetLowSurrogate(c);
				}
			TInt cur_index = -1;
            ::GetStringKey(aTable, text, textLen, cur_index, possible_prefix);
			if(cur_index != -1)
				{
				index = cur_index;
				iKey.iCharactersConsumed = i + 1;
				}
			}
		if (iKey.iCharactersConsumed < moved)
			{
			moved = 0;
			}
		while (moved != iKey.iCharactersConsumed)
			{
			++moved;
			}
        if(moved > 0)
            {
		    iDecompStrIt.Next(moved);//adjust the iterator start position
            }
		}

	// Now search the main index.
	if(index == -1)
		{
        index = ::FindCollationKeyIndex(cur_char, *aTable);
		if(0 <= index)
			{
			iKey.iCharactersConsumed = 1;
		    iDecompStrIt.Next(1);//adjust the iterator start position
			}
		}

	// Fill in the key or keys.
	if(index != -1)
		{
		const TUint32* p = &aTable->iKey[index];
		TCollationKey* q = iKey.iKey;
		iKey.iKeys = 0;
		while(iKey.iKeys < TKeyInfo::EMaxKeys)
			{
			q->iLow = *p;
			q->iHigh = cur_char;
			iKey.iKeys++;
			if(*p & 1)
                {
				break;
                }
			q++;
			p++;
			}
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////
// TCollate
/////////////////////////////////////////////////////////////////////////////////////////////////

/**
Construct a TCollate object based on the collation method specified
within aCharSet, if any. If there is none, or aCharSet is null, the
standard collation method will be used. 
aMask and aFlags provide a method for overriding the flags in the collation method: 
Each flag set to 1 in aMask is a flag that will be overridden and set to the
corresponding flag value in aFlags.
Ownership of aCharSet is not passed.
@param aCharSet Locale-specific character attribute and collation data
@param aMask Provides a method for overriding the flags in the collation method
@param aFlags Provides a method for overriding the flags in the collation method
@internalComponent
*/
TCollate::TCollate(const LCharSet* aCharSet, TUint aMask, TUint aFlags)
	{
	iMethod.iMainTable = NULL;
	iMethod.iOverrideTable = NULL;
	iMethod.iFlags = 0;
	if (aCharSet && aCharSet->iCollationDataSet && aCharSet->iCollationDataSet->iMethod)
        {
		iMethod = aCharSet->iCollationDataSet->iMethod[0];
        }
	if (iMethod.iMainTable == NULL)
        {
		iMethod.iMainTable = &TheStandardTable;
        }
	if (aMask)
		{
		iMethod.iFlags &= ~aMask;
		iMethod.iFlags |= (aMask & aFlags);
		}
	}

/**
Construct a TCollate object based on an already constructed
TCollationMethod specified in aMethod. 
Ownership is not passed.
@param aMethod Collation keys table
@internalComponent
*/
TCollate::TCollate(const TCollationMethod& aMethod) :
	iMethod(aMethod)
	{
	if(!iMethod.iMainTable)
        {
		iMethod.iMainTable = &TheStandardTable;
        }
	}

/**
Compare the string beginning at aString1 of length aLength1 against the
string beginning at aString2 of length aLength2. 

@param aString1 First string to compare
@param aLength1 Length of aString1
@param aString2 Second string to compare
@param aLength2 Length of aString2
@param aMaxLevel Determines the tightness of the collation. At level 0, only
                 character identities are distinguished. At level 1 accents are
                 distinguished as well. At level 2 case is distinguished as well. At
                 level 3 all non canonically equivalent Unicode characters are considered 
                 different. By default aMaxLevel is 3.
@return EStringsIdentical The strings are identical.
        ELeftComparesLessAndIsNotPrefix For example: aString1 = "aaa", aString2 = "zzzz".
        ELeftIsPrefixOfRight For example: aString1 = "abc", aString2 = "abcd".
        ERightIsPrefixOfLeft For example: aString1 = "abcd", aString2 = "abc".
        ERightComparesLessAndIsNotPrefix For example: aString1 = "zzzz", aString2 = "aaa".
@internalComponent
*/
TCollate::TComparisonResult TCollate::Compare(const TUint16 *aString1, TInt aLength1,
                                              const TUint16 *aString2, TInt aLength2,
                                              TInt aMaxLevel) const
	{
    TUTF32Iterator itL(aString1, aString1 + aLength1);
    TUTF32Iterator itR(aString2, aString2 + aLength2);
	return CompareKeySequences(itL, itR, aMaxLevel, 0, 0);
	}

/**
Find the string beginning at aString2 of length aLength2 in the string
beginning at aString1 of length aLength1.

@param aString1 String to search
@param aLength1 Length of aString1
@param aString2 String to search for
@param aLength2 Length of aString2
@param aMaxLevel Determines the tightness of the collation. At level 0, only
                 character identities are distinguished. At level 1 accents are
                 distinguished as well. At level 2 case is distinguishes as well. At
                 level 3 all valid different Unicode characters are considered different.
@param aString2WildChar Wild card character which may be specified for aString2. By default
                 wild card character is not specified and not used.
@return KErrNotFound aString2 not found in aString1. 
        Non-negative value telling the position in aString1 where the first occurrence of 
        aString2 was found.
@internalComponent
*/
TInt TCollate::Find(const TUint16 *aString1, TInt aLength1,
                    const TUint16 *aString2, TInt aLength2,
                    TInt aMaxLevel, TUint aString2WildChar) const
	{
	TInt dummy(0);
	return Find(aString1, aLength1, aString2,aLength2, dummy, aMaxLevel,aString2WildChar );
	}

/**
Find the string beginning at aString2 of length aLength2 in the string
beginning at aString1 of length aLength1.

@param aString1 String to search
@param aLength1 Length of aString1
@param aString2 String to search for
@param aLength2 Length of aString2
@param aLengthFound A refernce to the length of the match found in the candidate string
@param aMaxLevel Determines the tightness of the collation. At level 0, only
                 character identities are distinguished. At level 1 accents are
                 distinguished as well. At level 2 case is distinguishes as well. At
                 level 3 all valid different Unicode characters are considered different.
@param aString2WildChar Wild card character which may be specified for aString2. By default
                 wild card character is not specified and not used.
@return KErrNotFound aString2 not found in aString1. 
        Non-negative value telling the position in aString1 where the first occurrence of 
        aString2 was found.
@internalComponent
*/
TInt TCollate::Find(const TUint16 *aString1, TInt aLength1,
                    const TUint16 *aString2, TInt aLength2,
                    TInt &aLengthFound, TInt aMaxLevel, TUint aString2WildChar) const
	{
    TUTF32Iterator itL(aString1, aString1 + aLength1);
    TUTF32Iterator itR(aString2, aString2 + aLength2);
	return FindKeySequence(itL, itR, aMaxLevel, aString2WildChar, 0, aLengthFound);
	}
	
/**
Match the pattern defined by aSearchTerm with aCandidate. 
Return the index in aCandidate of the start of the first pattern matched - 
that is, the first character in aSearchTerm after all wild-sequence characters
have been matched. Return KErrNotFound if there is no match.

For example, if aCandidate is "abcdefghijkl", the following values of aSearchTerm yield the 
following results:
"abc*" gives 0
"abc" gives KErrNotFound
"xyz" gives KErrNotFound
"*def" gives KErrNotFound
"*def*" gives 3
"*d?f*" gives 3
"a*kl" gives 0
"*d*kl" gives 4

To match a pattern anywhere in aCandidate, aSearchTerm must both start and end 
with aString2WildSequenceChar

@param aCandidate String to search
@param aCandidateLength Length of aCandidate
@param aSearchTerm String to search for
@param aSearchTermLength Length of aSearchTerm
@param aMaxLevel Determines the tightness of the collation. At level 0, only
                 character identities are distinguished. At level 1 accents are
                 distinguished as well. At level 2 case is distinguishes as well. At
                 level 3 all valid different Unicode characters are considered different.
@param aWildChar Wild card character which may be specified for aSearchTerm. By default
                 the wild card character used is '?'.
@param aWildSequenceChar Wild card sequence character which may be specified for aSearchTerm.
                 Its default value is '*'.
@param aEscapeChar Escape character. If it is non-zero and precdes aWildChar and aWildSequenceChar characters in
                 aCandidate string, then these characters should be treated as normal characters.
@return The index in aCandidate of the start of the first pattern matched.

@internalComponent.
*/
TInt TCollate::Match(const TUint16 *aCandidate, TInt aCandidateLength,
                     const TUint16 *aSearchTerm,TInt aSearchTermLength, 
                     TInt aMaxLevel, TUint aWildChar, TUint aWildSequenceChar,
                     TUint aEscapeChar) const
	{
	ASSERT(0 <= aSearchTermLength);
	ASSERT(0 <= aCandidateLength);

	if(aMaxLevel == 3 && (iMethod.iFlags & TCollationMethod::EFoldCase))
        {
		aMaxLevel = 2;
        }

    TUTF32Iterator candidate(aCandidate, aCandidate + aCandidateLength);
    TUTF32Iterator searchTerm(aSearchTerm, aSearchTerm + aSearchTermLength);

	TInt firstMatch = KErrNotFound;
    TInt segEnd = ::FindCharacter(aWildSequenceChar, aEscapeChar, aSearchTerm, aSearchTermLength);

	// Is there any prefix that the candidate string must have?
    // aSearchTerm looks like "abc*...". Then segEnd will be 3 (the position of '*').
    // Check that aCandidate begins with "abc" too.
	if(segEnd != 0 || aSearchTermLength == 0)
		{
		searchTerm = TUTF32Iterator(aSearchTerm, aSearchTerm + segEnd);
		TComparisonResult order = CompareKeySequences(candidate, searchTerm, aMaxLevel, aWildChar, aEscapeChar);
		if(order != ERightIsPrefixOfLeft && order != EStringsIdentical)
            {
			return KErrNotFound;
            }
		if(aSearchTermLength == segEnd)
            {
			return order == EStringsIdentical ? 0 : KErrNotFound;
            }
		firstMatch = 0;
		}

	// search for all remaining segments
    // For example: aSearchTerm = "abc*def*ghi", aCandidate = "abc...".
    // aCandidate was already searched for "abc" and segEnd = 3. 
    // Search aCandidate for the remaining segments: "def" and "ghi".
	while(aSearchTermLength != (segEnd + 1))
		{
		++segEnd;
		aSearchTermLength -= segEnd;
		aSearchTerm += segEnd;
        segEnd = ::FindCharacter(aWildSequenceChar, aEscapeChar, aSearchTerm, aSearchTermLength);
		searchTerm = TUTF32Iterator(aSearchTerm, aSearchTerm + segEnd);//searchTerm holds the next aSearchTerm segment
        //We will store here the current position of candidate string.
        const TUint16* candidateCurrentPos = candidate.CurrentPosition();		
        TInt dummy(0);
        TInt match = FindKeySequence(candidate, searchTerm, aMaxLevel, aWildChar, aEscapeChar, dummy);	
		if (match < 0)
            {
			return KErrNotFound;
            }
		if (aSearchTermLength == segEnd)
			{
			candidate.SetStart(candidateCurrentPos + match);
   			TComparisonResult order = CompareKeySequences(candidate, searchTerm, aMaxLevel, aWildChar, aEscapeChar);
			if (order == EStringsIdentical)
				return firstMatch < 0 ? (match + candidateCurrentPos - aCandidate): firstMatch;
			while (match >= 0)
				{
				// We are at the very end of the search term, so this segment must
				// match the end of the candidate string.
				candidate.SetStart(candidateCurrentPos + match + 1);
	            candidateCurrentPos = candidate.CurrentPosition();
				match = FindKeySequence(candidate, searchTerm, aMaxLevel, aWildChar, aEscapeChar, dummy);
				candidate.SetStart(candidateCurrentPos + match);
				order = CompareKeySequences(candidate, searchTerm, aMaxLevel, aWildChar, aEscapeChar);
				if (order == EStringsIdentical)
					return firstMatch < 0 ? (match + candidateCurrentPos - aCandidate): firstMatch;
				}
			return KErrNotFound;
			}
        //Initialize the first match position, if not initialized yet
		if (firstMatch < 0 && segEnd != 0)
            {
			firstMatch = match;
            }
		}
	return firstMatch < 0 ? aCandidateLength : firstMatch;
	}

/**
Compare values output from the iterators. After the comparison, if
ERightIsPrefixOfLeft or EStringsIdentical is returned, then aLeft
will be pointing at the next character (at MaxLevel) after the match.
If right is shown to be a prefix of left, this means that it has been
checked at all requested levels. If it is reported that the right is a
prefix of the left, then this will mean also that there are no unmatched
combining characters on the left.

@internalComponent
*/
TCollate::TComparisonResult TCollate::CompareKeySequences(TUTF32Iterator& aLeft, TUTF32Iterator& aRight,
                                                          TInt aMaxLevel, TInt aRightStringWildChar, TInt aEscapeChar) const
	{
	// Clamp the maximum level of the comparison.
	if(aMaxLevel < 0)
        {
		aMaxLevel = 0;
        }
	if(aMaxLevel > 3)
        {
		aMaxLevel = 3;
        }
	//Case folding forces the maximum level to 2. Case folding could only be done at level 3, which
	//makes use of the actual Unicode values, if we had access to a case conversion table appropriate for
	//the collation method.
	if(aMaxLevel == 3 && (iMethod.iFlags & TCollationMethod::EFoldCase))
        {
		aMaxLevel = 2;
        }
    TCollationValueIterator itL(iMethod);
    TCollationValueIterator itR(iMethod);
	// Perform the comparison.
	TComparisonResult order = EStringsIdentical;
	TComparisonResult accumulatedOrder = EStringsIdentical;
	const TText16* endOfLeft = 0;
	for (int cur_level = 0; cur_level <= aMaxLevel; cur_level++)
		{
	    itL.SetSourceIt(aLeft);
	    itR.SetSourceIt(aRight);

		for (;;)
			{
			TUint32 c2 = itR.GetNextNonZeroKey(cur_level);
			if (c2 == 0)
				{
				TUint32 more = itL.GetNextNonZeroKey(cur_level);
				if (cur_level == 0)
					endOfLeft = itL.CurrentPositionIfAtCharacter();
   				if (more == 0)
                   {//No non-zero keys at all
					order = EStringsIdentical;
					}
				else if (!(TCollationMethod::EIgnoreCombining & iMethod.iFlags)
					&& itL.AtCombiningCharacter())
					{
					order = ERightComparesLessAndIsNotPrefix;
					}
				else
					{
					order = ERightIsPrefixOfLeft;
					}
				break;
				}
			TUint32 c1 = itL.GetNextNonZeroKey(cur_level);
			if (c1 == 0)
				{
				order = ELeftIsPrefixOfRight;
				break;
				}
				
			itL.Increment();
			if(cur_level == 0 && aEscapeChar != 0 && itR.MatchChar(aEscapeChar)) 
				{//Escape character found. Get the next key.
				c2 = itR.GetNextNonZeroKey(cur_level);
				itR.Increment();
				}
			else
				{
				if(aRightStringWildChar && itR.MatchChar(aRightStringWildChar))
					{
					itL.SkipCombiningCharacters();
					itR.SkipCombiningCharacters();
					c1 = c2;
					}
				else
	                {
					itR.Increment();
	                }
				}

			// Has an order been determined by key difference?
			if (c1 != c2)
				{
				// Fold to lower case, or switch ordering for case or kana syllabary if necessary.
				if (cur_level == 2 && (c1 <= (0x14 * 4) && c2 <= (0x14 * 4)))
					{
					// Divide keys by 4 to get them back into the range 0..63
					// because keys returned by GetKey are masked but not shifted.
					c1 /= 4;
					c2 /= 4;
					ProcessKeys(c1, c2, iMethod.iFlags);
					}
				if (c1 != c2) // test equality again because case folding might have made them equal
					{
					order = c1 > c2 ? ERightComparesLessAndIsNotPrefix : ELeftComparesLessAndIsNotPrefix;
					TBool backwards = cur_level == 1 && (iMethod.iFlags & TCollationMethod::EAccentsBackwards);
					if (order && !backwards)
                        {
						break;
                        }
					}
				}
			}
		if (accumulatedOrder != order && order != EStringsIdentical)
			{
			if (accumulatedOrder == ERightIsPrefixOfLeft)
				{
				return ERightComparesLessAndIsNotPrefix;
				}
			else if (accumulatedOrder == ELeftIsPrefixOfRight)
				{
				return ELeftComparesLessAndIsNotPrefix;
				}
			else
				{
				// accumulatedOrder == EStringsIdentical
				if (order == ELeftComparesLessAndIsNotPrefix || order == ERightComparesLessAndIsNotPrefix)
					{
					return order;
					}
				}
			accumulatedOrder = order;
			}
		}

	if (accumulatedOrder == EStringsIdentical || accumulatedOrder == ERightIsPrefixOfLeft)
		{
		if (endOfLeft)
			{
			aLeft.SetStart(endOfLeft);
			}
		else if (accumulatedOrder == ERightIsPrefixOfLeft)
			{
			accumulatedOrder = ERightComparesLessAndIsNotPrefix;
			}
		}
	return accumulatedOrder;
	}

/**
Finds search term inside candidate string. Returns KErrNotFound if there
is no match, returns the offset into the candidate string at which the
search term was found. If a string was found, the search term iterator is left
pointing at the end of the search term, and the candidate iterator is
left pointing just after the matched keys. aMatchPos returns where in
the candidate string the match was found.

@internalComponent
*/
TInt TCollate::FindKeySequence(TUTF32Iterator& aCandidate, TUTF32Iterator& aSearchTerm, 
                               TInt aMaxLevel, TInt aWildChar, TInt aEscapeChar, TInt& aLengthFound) const
	{
    TInt matchOffset = 0;
    //Save the start of the candidate string
    const TText* candidateStart = aCandidate.CurrentPosition();
    //Create copies of aCandidate and aSearchTerm
    TUTF32Iterator candidateCopy(aCandidate);
    TUTF32Iterator searchTermCopy(aSearchTerm);
    aLengthFound = KErrNotFound;
    //Do the search
	for(;;)
		{
		TComparisonResult order = CompareKeySequences(aCandidate, aSearchTerm, aMaxLevel, aWildChar, aEscapeChar);
		if(order == ELeftIsPrefixOfRight)
            {
			return KErrNotFound;
            }
		if(order == ERightIsPrefixOfLeft || order == EStringsIdentical)
			{
			aLengthFound = (aCandidate.CurrentPosition() - candidateStart) - matchOffset;
			return matchOffset;
			}

        aCandidate = candidateCopy;
		aCandidate.Next();
        ::SkipCombiningCharacters(aCandidate);
        candidateCopy = aCandidate;

		matchOffset = aCandidate.CurrentPosition() - candidateStart;

		aSearchTerm = searchTermCopy;
		}
	}
