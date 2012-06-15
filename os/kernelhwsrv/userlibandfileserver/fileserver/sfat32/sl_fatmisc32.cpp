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
// f32\sfat\sl_fatmisc32.cpp
// 
//

#include "sl_std.h"
#include "sl_cache.h"

/**
@return ETrue if it is Fat32
*/
TBool CFatFormatCB::Is32BitFat() const
	{
	return(iFileSystemName==KFileSystemName32);
	}

/**
@return ETrue if it is Fat16
*/
TBool CFatFormatCB::Is16BitFat() const
	{
    return(iFileSystemName==KFileSystemName16);
    }

/**
Calculate the FAT size in sectors for a Fat32 volume

@return The number of sectors
*/
TUint32 CFatFormatCB::MaxFat32Sectors() const
	{
	TUint32 calc1 = iMaxDiskSectors - iReservedSectors;
	TUint32 calc2 = (256 * iSectorsPerCluster) + iNumberOfFats;
	calc2 = calc2 >> 1;
	return (calc1 + (calc2 - 1))/calc2;
	}


const TUint KDefFatResvdSec = 1;    ///< default number of FAT12/16 reserved sectors
const TUint KDefFat32ResvdSec = 32; ///< default number of FAT32 reserved sectors

//-------------------------------------------------------------------------------------------------------------------
void Dump_TLDFormatInfo(const TLDFormatInfo& aInfo)
{
    (void)aInfo;
#ifdef _DEBUG
    __PRINT(_L("----- TLDFormatInfo dump:"));
    __PRINT1(_L("iCapacity:%d"), aInfo.iCapacity);
    __PRINT1(_L("iSectorsPerCluster:%d"), aInfo.iSectorsPerCluster);
    __PRINT1(_L("iSectorsPerTrack:%d"), aInfo.iSectorsPerTrack);
    __PRINT1(_L("iFATBits:%d"), aInfo.iFATBits);
    __PRINT1(_L("iReservedSectors:%d"), aInfo.iReservedSectors);
    __PRINT1(_L("iFlags:%d"), aInfo.iFlags);
    __PRINT(_L("-----"));
#endif
}

//-------------------------------------------------------------------------------------------------------------------

/**
Initialize the format parameters for a normal fixed sized disk
Setting set to adhere to Rules of Count of clusters for FAT type

@param  aDiskSizeInSectors Size of volume in sectors
@return system-wide error code
*/
TInt  CFatFormatCB::InitFormatDataForFixedSizeDiskNormal(TInt aDiskSizeInSectors, const TLocalDriveCapsV6& aCaps)
	{
	__PRINT1(_L("CFatFormatCB::InitFormatDataForFixedSizeDiskNormal() sectors:%d"), aDiskSizeInSectors);
    
    if( Drive().IsRemovable() )
		iNumberOfFats = KNumberOfFatsExternal;
	else
		iNumberOfFats = KNumberOfFatsInternal;	
 	
	iReservedSectors=KDefFatResvdSec;		
	if (aDiskSizeInSectors <=4084*1)	// 2MB
		{
		iRootDirEntries=128;
		iSectorsPerCluster=1;
		iFileSystemName=KFileSystemName12;
		iSectorsPerFat=MaxFat12Sectors();
   		}
	else if (aDiskSizeInSectors<4084*2) // < 4MB (8168 sectors)
		{
		iRootDirEntries=256; 
		iSectorsPerCluster=2;
		iFileSystemName=KFileSystemName12;
		iSectorsPerFat=MaxFat12Sectors();
		}
	else if (aDiskSizeInSectors<4084*4) // < 8MB (16336 sectors)
		{
		iRootDirEntries=512;
		iSectorsPerCluster=4;
		iFileSystemName=KFileSystemName12;
		iSectorsPerFat=MaxFat12Sectors();
		}
	else if (aDiskSizeInSectors<4084*8) // < 16MB (32672 sectors)
		{
		iRootDirEntries=512;
		iSectorsPerCluster=8;
		iFileSystemName=KFileSystemName12;
		iSectorsPerFat=MaxFat12Sectors();
		}
	else if(aDiskSizeInSectors<1048576) // >= 16Mb - FAT16   < (1048576) 512MB
		{
		iFileSystemName=KFileSystemName16;
		TInt minSectorsPerCluster=(aDiskSizeInSectors+KMaxFAT16Entries-1)/KMaxFAT16Entries;
		iRootDirEntries=512;
		iSectorsPerCluster=1;
		while (minSectorsPerCluster>iSectorsPerCluster)
			iSectorsPerCluster<<=1;
		iSectorsPerFat=MaxFat16Sectors();
		}
	else	//use FAT32
		{
		iFileSystemName=KFileSystemName32;
		iRootDirEntries=0;						//this is always the case for fat32
		if(aDiskSizeInSectors < 16777216)		//8GB in 512byte sectors
			iSectorsPerCluster=8;
		else if(aDiskSizeInSectors < 33554432)	//16GB in 512byte sectors
			iSectorsPerCluster=16;
		else if(aDiskSizeInSectors < 67108864)	//32GB in 512byte sectors 
			iSectorsPerCluster=32;
		else
			iSectorsPerCluster=64;				//Anything >= 32GB uses a 32K cluster size
		iReservedSectors=KDefFat32ResvdSec;
		iRootClusterNum=2;						//As recomended in the document
		iSectorsPerFat=MaxFat32Sectors();
		
		}

	// Ensure cluster size is a multiple of the block size
	TInt blockSizeInSectors = aCaps.iBlockSize >> iSectorSizeLog2;
	__PRINT1(_L("blockSizeInSectors: %d"),blockSizeInSectors);
	ASSERT(blockSizeInSectors == 0 || IsPowerOf2(blockSizeInSectors));
	if (blockSizeInSectors != 0 && IsPowerOf2(blockSizeInSectors))
		{
		__PRINT1(_L("iSectorsPerCluster	(old): %d"),iSectorsPerCluster);
		AdjustClusterSize(blockSizeInSectors);
		__PRINT1(_L("iSectorsPerCluster	(new): %d"),iSectorsPerCluster);
		}

	// Align first data sector on an erase block boundary if
	// (1) the iEraseBlockSize is specified
	// (2) the start of the partition is already aligned to an erase block boundary, 
	//     i.e. iHiddenSectors is zero or a multiple of iEraseBlockSize
	__PRINT1(_L("iHiddenSectors: %d"),iHiddenSectors);
	TInt eraseblockSizeInSectors = aCaps.iEraseBlockSize >> iSectorSizeLog2;
	__PRINT1(_L("eraseblockSizeInSectors: %d"),eraseblockSizeInSectors);
	ASSERT(eraseblockSizeInSectors == 0 || IsPowerOf2(eraseblockSizeInSectors));	
	ASSERT(eraseblockSizeInSectors == 0 || eraseblockSizeInSectors >= blockSizeInSectors);
	if ((eraseblockSizeInSectors != 0) &&
		(iHiddenSectors % eraseblockSizeInSectors == 0) &&	
		(IsPowerOf2(eraseblockSizeInSectors)) &&
		(eraseblockSizeInSectors >= blockSizeInSectors))
		{
		TInt r = AdjustFirstDataSectorAlignment(eraseblockSizeInSectors);
		ASSERT(r == KErrNone);
		(void) r;
		}
	__PRINT1(_L("iReservedSectors: %d"),iReservedSectors);
	__PRINT1(_L("FirstDataSector: %d"), FirstDataSector());

    return KErrNone;
	}

TInt CFatFormatCB::FirstDataSector() const
	{
	TInt rootDirSectors = (iRootDirEntries * KSizeOfFatDirEntry + (iBytesPerSector-1)) / iBytesPerSector;
    return iHiddenSectors + iReservedSectors + iNumberOfFats*iSectorsPerFat + rootDirSectors;
	}

void CFatFormatCB::AdjustClusterSize(TInt aRecommendedSectorsPerCluster)
	{
    const TInt KMaxSecPerCluster = 64;	// 32K
	while (aRecommendedSectorsPerCluster > iSectorsPerCluster && iSectorsPerCluster <= (KMaxSecPerCluster/2))
		iSectorsPerCluster<<= 1;
	}

// AdjustFirstDataSectorAlignment()
// Attempts to align the first data sector on an erase block boundary by modifying the
// number of reserved sectors.
TInt CFatFormatCB::AdjustFirstDataSectorAlignment(TInt aEraseBlockSizeInSectors)
	{
	const TBool bFat16 = Is16BitFat();
    const TBool bFat32 = Is32BitFat();

	// Save these 2 values in the event of a convergence failure; this should 
	// hopefully never happen, but we will cater for this in release mode to be safe,
	TInt reservedSectorsSaved = iReservedSectors;
	TInt sectorsPerFatSaved = iSectorsPerFat;

	TInt reservedSectorsOld = 0;

	// zero for FAT32
	TInt rootDirSectors = (iRootDirEntries * KSizeOfFatDirEntry + (iBytesPerSector-1)) / iBytesPerSector;
	TInt fatSectors = 0;

	TInt KMaxIterations = 10;
	TInt n;
	for (n=0; n<KMaxIterations && reservedSectorsOld != iReservedSectors; n++)
		{
		reservedSectorsOld = iReservedSectors;

		iSectorsPerFat = bFat32 ? MaxFat32Sectors() : bFat16 ? MaxFat16Sectors() : MaxFat12Sectors();

		fatSectors = iSectorsPerFat * iNumberOfFats;

		// calculate number of blocks
		TInt  nBlocks = (iReservedSectors + fatSectors + rootDirSectors + aEraseBlockSizeInSectors-1) / aEraseBlockSizeInSectors;

		iReservedSectors = (nBlocks * aEraseBlockSizeInSectors) - rootDirSectors - fatSectors;
		}
	
	ASSERT(iReservedSectors >= (TInt) (bFat32 ? KDefFat32ResvdSec : KDefFatResvdSec));

	if ((FirstDataSector() & (aEraseBlockSizeInSectors-1)) == 0)
		{
		return KErrNone;
		}
	else
		{
		iReservedSectors = reservedSectorsSaved;
		iSectorsPerFat = sectorsPerFatSaved;
		return KErrGeneral;
		}
	}


//-------------------------------------------------------------------------------------------------------------------

/**
Create the boot sector on media for the volume. For FAT32 also creates a backup copy of the boot sector.

@leave System wide error codes
*/
void CFatFormatCB::CreateBootSectorL()
	{
	__PRINT1(_L("CFatFormatCB::CreateBootSector() drive:%d"),DriveNumber());

	
    const TBool bFat32 = Is32BitFat();
    
    TFatBootSector bootSector;

	bootSector.SetVendorID(KDefaultVendorID);
	bootSector.SetBytesPerSector(iBytesPerSector);
	bootSector.SetSectorsPerCluster(iSectorsPerCluster);
	bootSector.SetReservedSectors(iReservedSectors);
	bootSector.SetNumberOfFats(iNumberOfFats);
	iCountOfClusters=iMaxDiskSectors/iSectorsPerCluster;
	if (!bFat32)
		{
		if (iCountOfClusters>(TInt)KMaxTUint16)
			User::Leave(KErrTooBig);
		}

	bootSector.SetReservedByte(0);
	TTime timeID;
	timeID.HomeTime();						//	System time in future?
	bootSector.SetUniqueID(I64LOW(timeID.Int64()));	//	Generate UniqueID from time
	bootSector.SetVolumeLabel(_L8(""));
	bootSector.SetFileSysType(iFileSystemName);
// Floppy specific info:
	bootSector.SetJumpInstruction();
	bootSector.SetMediaDescriptor(KBootSectorMediaDescriptor);
	bootSector.SetNumberOfHeads(iNumberOfHeads);
	bootSector.SetHiddenSectors(iHiddenSectors);
	bootSector.SetSectorsPerTrack(iSectorsPerTrack);
	bootSector.SetPhysicalDriveNumber(128);
	bootSector.SetExtendedBootSignature(0x29);

	if(bFat32)
		{
		bootSector.SetFatSectors(0);
		bootSector.SetFatSectors32(iSectorsPerFat);
		bootSector.SetRootDirEntries(0);
		bootSector.SetTotalSectors(0);
		bootSector.SetHugeSectors(iMaxDiskSectors);
		bootSector.SetFATFlags(0);			
		bootSector.SetVersionNumber(0x00);	
		bootSector.SetRootClusterNum(iRootClusterNum);
		bootSector.SetFSInfoSectorNum(KFSInfoSectorNum);	
        bootSector.SetBkBootRecSector(KBkBootSectorNum);
		}
	else//fat12 and 16
		{
		bootSector.SetFatSectors32(0);
		bootSector.SetFatSectors(iSectorsPerFat);
		bootSector.SetRootDirEntries(iRootDirEntries);

		if (iMaxDiskSectors<=(TInt)KMaxTUint16)
			{
			bootSector.SetTotalSectors(iMaxDiskSectors);
			bootSector.SetHugeSectors(0);
			}
		else
			{
			bootSector.SetTotalSectors(0);
			bootSector.SetHugeSectors(iMaxDiskSectors);
			}
		}

	//-- write main boot sector to the first sector on media
    User::LeaveIfError(FatMount().DoWriteBootSector(KBootSectorNum*bootSector.BytesPerSector(), bootSector));
	
    //-- for FAT32 write backup copy of the boot sector
    if(bFat32)
        {
        User::LeaveIfError(FatMount().DoWriteBootSector(KBkBootSectorNum*bootSector.BytesPerSector(), bootSector));    
        }

    }

//-------------------------------------------------------------------------------------------------------------------

/**
Format a disk section, called iteratively to erase whole of media, on last iteration
creates an empty volume. If called with quick formatonly erases the Fat leaving the
rest of the volume intact.

@leave System wide error code
*/
void CFatFormatCB::DoFormatStepL()
	{
	if (iFormatInfo.iFormatIsCurrent==EFalse)
		{ // Only done first time through
		if (iMode & EForceErase)
			{
			TInt r = FatMount().ErasePassword();
			User::LeaveIfError(r);
			// CFatMountCB::ErasePassword() calls TBusLocalDrive::ForceRemount(),
			// so need to stop a remount from occurring in next call to :
			// TFsFormatNext::DoRequestL((), TDrive::CheckMount().
			FatMount().Drive().SetChanged(EFalse);
			}

        RecordOldInfoL();
		InitializeFormatDataL();
		FatMount().DoDismount();
		if (iVariableSize)
			FatMount().ReduceSizeL(0,I64LOW(FatMount().iSize));
		}
    //
    // Blank disk if not EQuickFormat
    //
	if (!iVariableSize && !(iMode & EQuickFormat) && iCurrentStep)
		{
		if (iFormatInfo.iFormatIsCurrent == EFalse)
			{//-- firstly invalidate sectors 0-6 inclusive, they may contain main boot sector, backup boot sector and FSInfo sector.
	        DoZeroFillMediaL(0, (KBkBootSectorNum+1)*iBytesPerSector);
            }
		TInt ret=FatMount().LocalDrive()->Format(iFormatInfo);
		if (ret!=KErrNone && ret!=KErrEof) // Handle format error
            ret = HandleCorrupt(ret);

        if (ret!=KErrNone && ret!=KErrEof) // KErrEof could be set by LocalDrive()->Format()
		    User::Leave(ret);

		if (ret==KErrNone)
			{
			iCurrentStep = I64LOW( 100 - (100 * TInt64(iFormatInfo.i512ByteSectorsFormatted)) / iMaxDiskSectors );
			if (iCurrentStep<=0)
				iCurrentStep=1;
			return;
			}
		}

	// ReMount since MBR may have been rewritten and partition may have moved / changed size
	TInt ret = LocalDrive()->ForceRemount(0);
	if (ret != KErrNone && ret != KErrNotSupported)
		User::Leave(ret);

	// MBR may have changed, so need to re-read iHiddenSectors etc.before BPB is written
	InitializeFormatDataL();

    // Translate bad sector number to cluster number which contains that sector
    // This only happens in full format, in quick format they are already cluster numbers
    if (!iVariableSize && !(iMode & EQuickFormat))
        User::LeaveIfError(BadSectorToCluster());

	//Check if root cluster is bad and update as required
	if(Is32BitFat() && !iVariableSize && (iMode & EQuickFormat))
		{
		if(iBadClusters.Find(iRootClusterNum) !=  KErrNotFound)
			{
			iRootClusterNum++;
            while(iBadClusters.Find(iRootClusterNum) != KErrNotFound)
				{
				iRootClusterNum++;
				}
			}
		}

    //
    // Do the rest of the disk in one lump
    //
	iCurrentStep=0;

    //-- zero-fill media from position 0 to the FAT end, i.e main & backup boot sector, FSInfo and its copy and all FATs
    const TUint32 posFatEnd = ((iSectorsPerFat*iNumberOfFats) + iReservedSectors) * iBytesPerSector; //-- last FAT end position
	
    if (iVariableSize)
		FatMount().EnlargeL(posFatEnd); 

    DoZeroFillMediaL(0, posFatEnd);
    
    if(Is32BitFat())
		{//create an empty root directory entry here
		
        const TUint KFat32EntrySz = 4; //-- FAT32 entry size, bytes
        const TInt  startFAT1   = iReservedSectors;              //-- FAT1 start sector
        const TInt  entryOffset = iRootClusterNum*KFat32EntrySz; //-- Root dir entry offset in the FAT, bytes

		TBuf8<KFat32EntrySz> EOF(KFat32EntrySz);
		EOF[0]=0xFF;		
		EOF[1]=0xFF;
		EOF[2]=0xFF;
		EOF[3]=0x0F;

        //-- write EOF mark to the every FAT copy
    	for(TInt i=0; i<iNumberOfFats; i++)
            {
		    const TInt rootDirEntryPos = iBytesPerSector*(startFAT1 + i*iSectorsPerFat) + entryOffset;
            User::LeaveIfError(LocalDrive()->Write(rootDirEntryPos, EOF));
            }

        //-- zero-fill FAT32 root directory (just 1 cluster)
		const TInt firstDataSector = iReservedSectors + (iNumberOfFats * iSectorsPerFat); //+RootDirSectors (not required for fat32)
        const TInt firstSectorOfCluster = ((iRootClusterNum - KFatFirstSearchCluster) * iSectorsPerCluster) + firstDataSector;
	
        const TUint32 posRootDirStart = firstSectorOfCluster * iBytesPerSector;
        const TUint32 posRootDirEnd = posRootDirStart + iSectorsPerCluster*iBytesPerSector;

        DoZeroFillMediaL(posRootDirStart, posRootDirEnd);
        }
	else
		{//-- FAT12/16
		    //-- Zero fill root directory
            const TInt rootDirSector = iReservedSectors + (iNumberOfFats * iSectorsPerFat); 
            const TInt rootDirSize   = iRootDirEntries * KSizeOfFatDirEntry; //-- size in bytes
            
            const TUint32 posRootDirStart = rootDirSector * iBytesPerSector;
            const TUint32 posRootDirEnd   = posRootDirStart + rootDirSize;

            const TInt numOfRootSectors=(rootDirSize%iBytesPerSector) ? (rootDirSize/iBytesPerSector+1) : (rootDirSize/iBytesPerSector);
		    if (iVariableSize)
			    FatMount().EnlargeL(iBytesPerSector*numOfRootSectors);

            DoZeroFillMediaL(posRootDirStart, posRootDirEnd);

		// Enlarge ram drive to take into account rounding of
		// data start to cluster boundary
		if(iVariableSize && iSectorsPerCluster!=1)
			{
			const TInt firstFreeSector=rootDirSector+numOfRootSectors;
			const TInt firstFreeCluster=firstFreeSector%iSectorsPerCluster ? firstFreeSector/iSectorsPerCluster+1 : firstFreeSector/iSectorsPerCluster;
			const TInt alignedSector=firstFreeCluster*iSectorsPerCluster;
			if(alignedSector!=firstFreeSector)
				FatMount().EnlargeL((alignedSector-firstFreeSector)*iBytesPerSector);
			}
		}

    //-- FAT[0] must contain media descriptor in the low byte, FAT[1] for fat16/32 may contain some flags
	TBuf8<8> startFat(8);
    startFat.Fill(0xFF);
   
    if(Is32BitFat()) //-- FAT32
        {//-- FAT32 uses only low 28 bits in FAT entry. 
        startFat[3] = 0x0F;
        startFat[7] = 0x0F;
        } 
    else if(iVariableSize||Is16BitFat()) //-- FAT16 or RAM drive which is always FAT16
        {
        startFat.SetLength(4);
        }
    else //-- FAT12
        {
        startFat.SetLength(3);
        }

    startFat[0]=KBootSectorMediaDescriptor;

    //-- write FAT[0] and FAT[1] entries to all copies of FAT
	for(TInt i=0;i<iNumberOfFats;i++)
        {
		User::LeaveIfError(LocalDrive()->Write(iBytesPerSector*(iReservedSectors+(iSectorsPerFat*i)),startFat));
        }

	//-- create boot sectors
    CreateBootSectorL();

    //-- create FSInfo sectors
    if (Is32BitFat())
		{
		CreateReservedBootSectorL();
        CreateFSInfoSectorL();
		}

    //-- here we have bad clusters numbers saved by the quick format
    //-- Interpret old bad cluster number to new cluster number and mark new bad clusters
    if (!iVariableSize && iBadClusters.Count()>0)
        {
 	
        //-- Here we need fully mounted volume, so mount it normally.
	    FatMount().MountL(EFalse);

        iBadClusters.Sort();
        TranslateL();
        const TInt mark = FatMount().Is32BitFat() ? KBad_32Bit : (FatMount().Is16BitFat() ? KBad_16Bit : KBad_12Bit);
        
        for (TInt i=0; i<iBadClusters.Count(); ++i)
            FatMount().FAT().WriteL(iBadClusters[i], mark);
        
        FatMount().FAT().FlushL();
        
        //-- indicate that the volume is "dirty" in order to the next Mount evend not to use FSInfo, which
        //-- contains incorrect value of free clusters because we already have bad ones saved.  
        //-- This is a very rare condition.
        FatMount().SetVolumeCleanL(EFalse); 

#if defined(_DEBUG)
	TInt r=FatMount().CheckDisk();
	__PRINT1(_L("CFatFormatCB::DoFormatStepL() CheckDisk res: %d"),r);
#endif
        }
        else
        {
        //-- We do not need to perform full mount in this case, the TDrive object will be marked as changed in ~CFormatCB and the
        //-- mount will be closed. Therefore on the first access to it it will be mounted normally.
        FatMount().MountL(ETrue); //-- force mount
        }

    __PRINT1(_L("CFatFormatCB::DoFormatStepL() Format complete drv:%d"), DriveNumber());
	}


//-------------------------------------------------------------------------------------------------------------------

/**
    Initialize the user specific format parameters for fixed sized disk.
    
    @param  aDiskSizeInSectors disk size in sectors
    @return system-wide error code
*/
TInt CFatFormatCB::InitFormatDataForFixedSizeDiskUser(TInt aDiskSizeInSectors)
	{
    __PRINT1(_L("CFatFormatCB::InitFormatDataForFixedSizeDiskUser() sectors:%d"), aDiskSizeInSectors);
    Dump_TLDFormatInfo(iSpecialInfo());

    //-- KErrArgument will be returned if iSpecialInfo().iFATBits isn't one of EFB32, EFB16, EFB32

    if(iSpecialInfo().iFlags & TLDFormatInfo::EOneFatTable)
		iNumberOfFats = 1;
    else if(iSpecialInfo().iFlags & TLDFormatInfo::ETwoFatTables)
		iNumberOfFats = 2;
    else if(Drive().IsRemovable())
		iNumberOfFats = KNumberOfFatsExternal;
	else
		iNumberOfFats = KNumberOfFatsInternal;


    if(iSpecialInfo().iReservedSectors == 0)
        iReservedSectors = KDefFatResvdSec; //-- user hasn't specified reserved sectors count, use default (FAT12/16)
    else
        iReservedSectors = iSpecialInfo().iReservedSectors;


    const TInt KMaxSecPerCluster    = 64; 
	const TInt KDefaultSecPerCluster= 8;   //-- default value, if the iSpecialInfo().iSectorsPerCluster isn't specified

    iSectorsPerCluster = iSpecialInfo().iSectorsPerCluster;
    if(iSectorsPerCluster <= 0)
        {//-- default value, user hasn't specified TLDFormatInfo::iSectorsPerCluster
        iSectorsPerCluster = KDefaultSecPerCluster; //-- will be adjusted later
        }
    else
        {
        iSectorsPerCluster = Min(1<<Log2(iSectorsPerCluster), KMaxSecPerCluster);
	    }

    //-----------------------------------------

    if (aDiskSizeInSectors < 4096) // < 2MB
        {
        iSectorsPerCluster = 1;
		iRootDirEntries = 128;
        }
	else if (aDiskSizeInSectors < 8192) // < 4MB
        {
        iSectorsPerCluster = Min(iSectorsPerCluster, 2);
		iRootDirEntries = 256;
        }
	else if (aDiskSizeInSectors < 32768) // < 16MB
        {
        iSectorsPerCluster = Min(iSectorsPerCluster, 4);
		iRootDirEntries = 512;
        }
	else if (aDiskSizeInSectors < 1048576) // < 512MB
        {
        iSectorsPerCluster = Min(iSectorsPerCluster, 8);
		iRootDirEntries = 512;
        }
    else // FAT32
		{
        iRootDirEntries = 512;
        iSectorsPerCluster = Min(iSectorsPerCluster, KMaxSecPerCluster);
        }


    //-----------------------------------------

	TLDFormatInfo::TFATBits fatBits = iSpecialInfo().iFATBits;
	if (fatBits == TLDFormatInfo::EFBDontCare)
		{
        const TFatType fatType = SuggestFatType();
		switch(fatType)
			{
			case EFat12:
				fatBits = TLDFormatInfo::EFB12;
				break;
			case EFat16:
				fatBits = TLDFormatInfo::EFB16;
				break;
			case EFat32:
				fatBits = TLDFormatInfo::EFB32;
				break;
			case EInvalid:
				ASSERT(0);
			}
		}

    TFatType reqFatType(EInvalid); //-- requested FAT type

    switch (fatBits)
		{
		case TLDFormatInfo::EFB12:
			iFileSystemName=KFileSystemName12;
			iSectorsPerFat=MaxFat12Sectors();
			reqFatType = EFat12;
            break;

		case TLDFormatInfo::EFB16:
			iFileSystemName=KFileSystemName16;
			iSectorsPerFat=MaxFat16Sectors();
			reqFatType = EFat16;
            break;

		case TLDFormatInfo::EFB32:
			iFileSystemName=KFileSystemName32;
			iSectorsPerFat=MaxFat32Sectors();
	        
			iRootDirEntries = 0;
			iRootClusterNum = 2;
			
            if(iSpecialInfo().iReservedSectors == 0)
                iReservedSectors = KDefFat32ResvdSec; //-- user hasn't specified reserved sectors count, use default (FAT32)
            else
                iReservedSectors = iSpecialInfo().iReservedSectors;

			reqFatType = EFat32;
            break;

        default:
            __PRINT(_L("CFatFormatCB::InitFormatDataForFixedSizeDiskUser() Incorrect FAT type specifier!"));
            return KErrArgument;
		}
	
        //-- check if we can format the volume with requested FAT type
        const TFatType fatType = SuggestFatType();
        if(fatType != reqFatType)
			{
			//-- volume metrics don't correspond to the requested FAT type
            __PRINT(_L("CFatFormatCB::InitFormatDataForFixedSizeDiskUser() FAT type mismatch!"));
            return KErrArgument;
			}

        return KErrNone;
    }

/**
    Initialize the format parameters for a custom fixed sized disk

    @param  aFormatInfo The custom format parameters
    @return system-wide error code
*/
TInt CFatFormatCB::InitFormatDataForFixedSizeDiskCustom(const TLDFormatInfo& aFormatInfo)
	{
    __PRINT(_L("CFatFormatCB::InitFormatDataForFixedSizeDiskCustom()"));
    Dump_TLDFormatInfo(aFormatInfo);

	if(aFormatInfo.iFlags & TLDFormatInfo::EOneFatTable)
		iNumberOfFats = 1;
    else if(aFormatInfo.iFlags & TLDFormatInfo::ETwoFatTables)
		iNumberOfFats = 2;
    else if(Drive().IsRemovable())
		iNumberOfFats = KNumberOfFatsExternal;
	else
		iNumberOfFats = KNumberOfFatsInternal;	

	iRootDirEntries=512;

	iSectorsPerCluster = aFormatInfo.iSectorsPerCluster;
	iSectorsPerTrack   = aFormatInfo.iSectorsPerTrack;
	iNumberOfHeads	   = aFormatInfo.iNumberOfSides;
	iReservedSectors   = aFormatInfo.iReservedSectors ? aFormatInfo.iReservedSectors : KDefFatResvdSec;
	
    switch (aFormatInfo.iFATBits)
		{
		case TLDFormatInfo::EFB12:
			iFileSystemName = KFileSystemName12;
			iSectorsPerFat  = MaxFat12Sectors();
			break;

		case TLDFormatInfo::EFB16:
			iFileSystemName = KFileSystemName16;
			iSectorsPerFat  = MaxFat16Sectors();
            break;

		case TLDFormatInfo::EFB32:
			iFileSystemName  = KFileSystemName32;
			iReservedSectors = aFormatInfo.iReservedSectors ? aFormatInfo.iReservedSectors : KDefFat32ResvdSec;
			iSectorsPerFat   = MaxFat32Sectors();
			iRootDirEntries  = 0;
			iRootClusterNum  = 2;
            break;

		default:
			{
			TInt64 clusters64 = (aFormatInfo.iCapacity / KDefaultSectorSize) / iSectorsPerCluster;
			TInt clusters = I64LOW(clusters64);
			if (clusters < 4085)
				{
				iFileSystemName = KFileSystemName12;
				iSectorsPerFat  = MaxFat12Sectors();
				}
			else if(clusters < 65525)
				{
				iFileSystemName = KFileSystemName16;
				iSectorsPerFat  = MaxFat16Sectors();
                }
			else
				{
				iFileSystemName  = KFileSystemName32;
				iReservedSectors = aFormatInfo.iReservedSectors ? aFormatInfo.iReservedSectors : KDefFat32ResvdSec;
				iSectorsPerFat   = MaxFat32Sectors();
				iRootDirEntries  = 0;
				iRootClusterNum  = 2;
				}
			}
		}

    return KErrNone;
	}

void CFatFormatCB::RecordOldInfoL()
    {
    // Check if mount or disk is corrupt
    // This should be stored in member variable because FatMount is remounted
    //  every time RFormat::Next() gets called thus FatMount().Initialised()
    //  will be inconsistent with previous state.
	TLocalDriveCapsV3Buf caps;
	User::LeaveIfError(LocalDrive()->Caps(caps));
	iVariableSize=((caps().iMediaAtt)&KMediaAttVariableSize) ? (TBool)ETrue : (TBool)EFalse;
    iDiskCorrupt = !FatMount().ConsistentState();
    iBadClusters.Reset();
    iBadSectors.Reset();
    if (!iVariableSize && !iDiskCorrupt && (iMode&EQuickFormat))
        {
        iOldFirstFreeSector = FatMount().iFirstFreeByte>>FatMount().SectorSizeLog2();
        iOldSectorsPerCluster = FatMount().SectorsPerCluster();

        FatMount().FAT().InvalidateCacheL(); //-- invalidate whole FAT cache

        // Collect bad cluster information from current FAT table
        const TInt maxClusterNum = FatMount().UsableClusters() + KFatFirstSearchCluster;
        const TUint32 mark = FatMount().Is32BitFat() ? KBad_32Bit : (FatMount().Is16BitFat() ? KBad_16Bit : KBad_12Bit);
        
        for (TInt i=KFatFirstSearchCluster; i<maxClusterNum; ++i)
            {
            if (FatMount().FAT().ReadL(i) == mark)
                iBadClusters.AppendL(i);
            }
        }
    }




TInt CFatFormatCB::BadSectorToCluster()
    {
    TInt sizeofFatAndRootDir;
    if (iFileSystemName != KFileSystemName32)
		sizeofFatAndRootDir = iSectorsPerFat*iNumberOfFats + ((iRootDirEntries*KSizeOfFatDirEntry+(1<<iSectorSizeLog2)-1)>>iSectorSizeLog2);
    else
        sizeofFatAndRootDir = (iRootClusterNum-2) * iSectorsPerCluster;
    TInt firstFreeSector = iReservedSectors + sizeofFatAndRootDir;

    // Check in rare case that corrupt in critical area
    // which includes bootsector, FAT table, (and root dir if not FAT32)
    TInt i, r;
    for (i=0; i<iBadSectors.Count(); ++i)
        {
        TInt badSector = iBadSectors[i];
        // Check in rare case that corrupt in critical area
        // which includes bootsector, FAT table, (and root dir if not FAT32)
        if (firstFreeSector > badSector)
            {
            if (badSector == 0) // Boot sector corrupt
                return KErrCorrupt;
            if (iFileSystemName==KFileSystemName32 && badSector==1) // FSInfo corrupt
                return KErrCorrupt;
            if (badSector < iReservedSectors) // Harmless in reserved area
                continue;
            // Extend reserved area to cover bad sector
            iReservedSectors = badSector + 1;
            firstFreeSector = iReservedSectors + sizeofFatAndRootDir;
            continue;
            }

        // Figure out bad cluster number and record it
        TUint cluster = (badSector-firstFreeSector)/iSectorsPerCluster+2;
        if (iBadClusters.Find(cluster) == KErrNotFound)
            {
            if ((r=iBadClusters.Append(cluster)) != KErrNone)
                return r;
            if (iFileSystemName==KFileSystemName32 && iRootClusterNum==cluster)
                iRootClusterNum++;
            }
        }
    return KErrNone;
    }

/**
Create the File system information sector and its backup copy on a disk. 
Note that CFatMountCB is still not in mounted state, so we can not rely on it.

@leave System wide error codes
*/
void CFatFormatCB::CreateFSInfoSectorL()
	{
	__PRINT1(_L("CFatFormatCB::CreateFSInfoSectorL() drv:%d"), DriveNumber());
	
    ASSERT(Is32BitFat()); //-- Actually, CFatMount shall be in a consistent state.

    TFSInfo fsInfo;
	TBuf8<KSizeOfFSInfo> fsInfoSecBuf;
    
    const TUint32 freeSectors  = iMaxDiskSectors - (iReservedSectors + (iNumberOfFats * iSectorsPerFat));
    const TUint32 freeClusters = (freeSectors / iSectorsPerCluster) - 1; //-- 1st cluster is taken by empty Root Dir on FAT32
    const TUint32 nextFreeClust = iRootClusterNum+1; 

    fsInfo.SetFreeClusterCount(freeClusters);
    fsInfo.SetNextFreeCluster(nextFreeClust);

    fsInfo.Externalize(fsInfoSecBuf); //-- put data to the sector buffer

    User::LeaveIfError(LocalDrive()->Write(KFSInfoSectorNum*iBytesPerSector, fsInfoSecBuf)); //-- main FSInfo Sector
    User::LeaveIfError(LocalDrive()->Write(KBkFSInfoSectorNum*iBytesPerSector, fsInfoSecBuf)); //-- Backup FSInfo Sector

	}

/**
Create the reserved boot sector and its backup copy on a disk. 
These are located at sectors 2 & 8

@leave System wide error codes
*/
void CFatFormatCB::CreateReservedBootSectorL()
	{
	__PRINT1(_L("CFatFormatCB::CreateReserveBootSectorL() drv:%d"), DriveNumber());
	
    ASSERT(Is32BitFat());

    TFatBootSector bootSector;

	User::LeaveIfError(FatMount().DoWriteBootSector(KReservedBootSectorNum*KDefaultSectorSize, bootSector));
	User::LeaveIfError(FatMount().DoWriteBootSector(KBkReservedBootSectorNum*KDefaultSectorSize, bootSector));    
	}




