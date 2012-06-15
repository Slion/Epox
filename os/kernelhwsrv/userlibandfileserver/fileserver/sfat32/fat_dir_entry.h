// Copyright (c) 1998-2009 Nokia Corporation and/or its subsidiary(-ies).
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
// f32\sfat\inc\fat_dir_entry.h
// FAT directory entry related stuff definitions.
// 
//

/**
 @file
 @internalTechnology
*/

#if !defined(FAT_DIR_ENTRY_H)
#define FAT_DIR_ENTRY_H


//-------------------------------------------------------------------------------------------------------------------


const TInt      KFatDirNameSize         = 11;   ///< Dos directory/File name length
const TInt      KVFatEntryAttribute     = 0x0F;  ///< VFat entry attribute setting
const TUint8    KDotEntryByte           = 0x2e;  ///< Dot value for self and parent pointer directory entries
const TUint8    KBlankSpace             = 0x20;  ///< Blank space in a directory entry
const TInt      KSizeOfFatDirEntryLog2  = 5;     ///< Log2 of size in bytes of a Fat directry entry 
const TUint     KSizeOfFatDirEntry      = 1 << KSizeOfFatDirEntryLog2;    ///< Size in bytes of a Fat directry entry 

const TUint16 KReservedIdOldEntry = 1;	///< Rugged FAT "OldEntry" id
const TUint16 KReservedIdNewEntry = 0;  ///< Rugged FAT "ReservedIdNewEntry" id


typedef TBuf8<KFatDirNameSize> TShortName;  ///< Buffer type fot short names in dos entries

//-------------------------------------------------------------------------------------------------------------------

/**
    Fat DOS directory entry structure
*/
struct SFatDirEntry
    {
    TUint8  iName[KFatDirNameSize]; ///< :0  File/Directory name
    TUint8  iAttributes;            ///< :11 File/Directory attributes
    TUint8  iReserved1[2];          ///< :12 2 reserved bytes(in our implementation), some versions of Windows may use them
    TUint16 iTimeC;                 ///< :14 Creation time
    TUint16 iDateC;                 ///< :16 Creation date
    TUint16 iReserved2;             ///< :18 2 reserved bytes(in our implementation), FAT specs say that this is "last access date". Rugged FAT uses them as a special entry ID
    TUint16 iStartClusterHi;        ///< :20 High 16 bits of the File/Directory cluster number (Fat32 only)
    TUint16 iTime;                  ///< :22 last write access time 
    TUint16 iDate;                  ///< :24 last write access date 
    TUint16 iStartClusterLo;        ///< :26 Low 16 bits of the File/Directory cluster number 
    TUint32 iSize;                  ///< :28 File/Directory size in bytes
    };


//-------------------------------------------------------------------------------------------------------------------

/**
Provides access to the Fat directory entry parameters
*/
class TFatDirEntry
    {
public:
    inline TFatDirEntry();
    inline void InitZ();

    inline const TPtrC8 Name() const;
    inline TInt Attributes() const;
    inline TTime Time(TTimeIntervalSeconds aOffset) const;
    inline TInt StartCluster() const;
    inline TUint32 Size() const;
    inline TBool IsErased() const;
    inline TBool IsCurrentDirectory() const;
    inline TBool IsParentDirectory() const;
    inline TBool IsEndOfDirectory() const;
    inline TBool IsGarbage() const;
    inline void SetName(const TDesC8& aDes);
    inline void SetAttributes(TInt anAtt);
    inline void SetTime(TTime aTime, TTimeIntervalSeconds aOffset);
    inline void SetCreateTime(TTime aTime, TTimeIntervalSeconds aOffset);
    inline void SetStartCluster(TInt aStartCluster);
    inline void SetSize(TUint32 aFilesize);
    inline void SetErased();
    inline void SetCurrentDirectory();
    inline void SetParentDirectory();
    inline void SetEndOfDirectory();
    inline TUint RuggedFatEntryId() const;
    inline void  SetRuggedFatEntryId(TUint16 aId);

public:
    void InitializeAsVFat(TUint8 aCheckSum);
    void SetVFatEntry(const TDesC& aName,TInt aRemainderLen);
    void ReadVFatEntry(TDes16& aVBuf) const;
    inline TBool IsLongNameStart() const;
    inline TBool IsVFatEntry() const;
    inline TInt NumFollowing() const;
    inline TUint8 CheckSum() const;


public:
    TUint8 iData[KSizeOfFatDirEntry]; ///< The directory entry data
    };



#endif //FAT_DIR_ENTRY_H














