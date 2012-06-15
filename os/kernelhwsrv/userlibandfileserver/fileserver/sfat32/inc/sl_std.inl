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
// f32\sfat32\inc\sl_std.inl
// 
//

#ifndef SL_STD_INL
#define SL_STD_INL



//---------------------------------------------------------------------------------------------------------------------------------
// class TEntryPos
TUint32 TEntryPos::Cluster() const 
    {
    return (TUint32) iCluster;
    }

TUint32 TEntryPos::Pos() const 
    {
    return (TUint32) iPos;
    }

TBool TEntryPos::operator==(const TEntryPos& aRhs) const
    {
    ASSERT(this != &aRhs);
    return (iCluster == aRhs.iCluster && iPos == aRhs.iPos);
    }


//---------------------------------------------------------------------------------------------------------------------------------
// class CFatMountCB

inline TInt CFatMountCB::RootDirectorySector() const
    {return iVolParam.RootDirectorySector();}

inline TUint CFatMountCB::RootDirEnd() const
    {return iVolParam.RootDirEnd();}

inline TUint32 CFatMountCB::RootClusterNum() const
    {return iVolParam.RootClusterNum(); }        


inline TInt CFatMountCB::StartCluster(const TFatDirEntry & anEntry) const
	{
	if(Is32BitFat())	
		return anEntry.StartCluster();
	else
		return 0xFFFF&anEntry.StartCluster();
	}

/**
returns true for root dir on Fat12/16 (fixed root dir versions of Fat) false on fat32 
this function is used to handle special cases for reading/writing the root directory on FAT via the use of cluster zero.

@param aEntry position on volume being queried
@return Whether Root dir position or not
*/
TBool CFatMountCB::IsRootDir(const TEntryPos &aEntry) const
	{
	if(Is32BitFat())
		return EFalse;
	else
		return((aEntry.iCluster==0) ? (TBool)ETrue : (TBool)EFalse);	
	}
/**
Indicates the root directory cluster, For Fat12/16 root is always indicated by cluster number zero, on Fat32 the is a root cluster number
@return The root cluster indicator
*/
TInt CFatMountCB::RootIndicator() const
	{
	if(Is32BitFat())
        return iVolParam.RootClusterNum();
	else
		return 0;
	}


/** @return Log2 of cluster size on volume */
TInt CFatMountCB::ClusterSizeLog2() const
    {return(iVolParam.ClusterSizeLog2());}

/** @return Log2 of media sector size  */
TInt CFatMountCB::SectorSizeLog2() const
    {return(iVolParam.SectorSizeLog2());}

/** @return sector per cluster */
TInt CFatMountCB::SectorsPerCluster() const
    {return(1<<(iVolParam.ClusterSizeLog2()-iVolParam.SectorSizeLog2()));}

/** @return the base position of a cluster */
TInt CFatMountCB::ClusterBasePosition() const
	{return(iFirstFreeByte);}

/** @return the offset into a cluster of a byte address */
TInt CFatMountCB::ClusterRelativePos(TInt aPos) const
	{return(aPos&((1<<ClusterSizeLog2())-1));}

/**
Calculates the maximum number of clusters
@return  maximum number of clusters
*/
TUint32 CFatMountCB::MaxClusterNumber() const
    {return(TotalSectors()>>(ClusterSizeLog2()-SectorSizeLog2()));}

/** @return the the total sectors on volume */
TInt CFatMountCB::TotalSectors() const
    {return iVolParam.TotalSectors();}

/** @return total size of a Fat in bytes */
TInt CFatMountCB::FatSizeInBytes() const
    {return iVolParam.FatSizeInBytes();}

/** @return first sector of the Fat */
TUint32 CFatMountCB::FirstFatSector() const
    {return iVolParam.FirstFatSector();}

/** @return the byte offset of the Fat */
TInt CFatMountCB::StartOfFatInBytes() const
	{return(FirstFatSector()<<SectorSizeLog2());}

/** @return Number of Fats used by the volume */
TInt CFatMountCB::NumberOfFats() const
    {return iVolParam.NumberOfFats();}


/** @return refrence to the fat table owned by the mount */
CFatTable& CFatMountCB::FAT() const
	{return(*iFatTable);}
/**
    @return refrence to the file system object that has produced this CFatMountCB
*/
CFatFileSystem& CFatMountCB::FatFileSystem() const
	{
    return (CFatFileSystem&)(*FileSystem()); //-- CMountCB::FileSystem() provides correct answer
    }


/** @return  refrence to a raw disk object owned by the mount */
CRawDisk& CFatMountCB::RawDisk() const
	{return(*iRawDisk);}

/**
@return ETrue if aCluster value is bad cluster marker defined in FAT specification
*/
TBool CFatMountCB::IsBadCluster(TInt aCluster) const
	{return Is32BitFat() ? aCluster==0xFFFFFF7 : Is16BitFat() ? aCluster==0xFFF7 : aCluster==0xFF7;}

/**
Returns whether the current mount is running as rugged Fat or not, this is held in the file system object
@return Is rugged fat flag
*/
TBool CFatMountCB::IsRuggedFSys() const
	{return Drive().IsRugged();}

/**
Sets the rugged flag in the file system object
@param Flag to set or clear the rugged flag
*/
void CFatMountCB::SetRuggedFSys(TBool aVal)
	{Drive().SetRugged(aVal);}

/** @return the usable clusters count for a volume */
TUint32 CFatMountCB::UsableClusters() const
	{return(iUsableClusters);}


TUint CFatMountCB::StartOfRootDirInBytes() const
    {return iVolParam.RootDirectorySector()<<SectorSizeLog2();}


/** @return FAT type for this mount */
TFatType CFatMountCB::FatType() const
{
    return iFatType;
}

/** @return ETrue if the mount has 16bit FAT */
TBool CFatMountCB::Is16BitFat() const
{
    return FatType() == EFat16;
} 

/** @return ETrue if the mount has 32bit FAT */
TBool CFatMountCB::Is32BitFat() const
{   
    return FatType() == EFat32;
}

CAsyncNotifier* CFatMountCB::Notifier() const
	{return iNotifier;}	



/**
    Set or reset Read Only mode for the mount.
    @param    aReadOnlyMode if ETrue, the mount will be set RO.
*/
void  CFatMountCB::SetReadOnly(TBool aReadOnlyMode) 
    {
    iReadOnly = aReadOnlyMode;
    }


/**
    @return ETrue if the volume is in Read-Only state
*/
TBool CFatMountCB::ReadOnly(void) const
    {
    return iReadOnly;
    }

/** @return state of the CFatMountCB*/
CFatMountCB::TFatMntState CFatMountCB::State() const 
    {
    return iState;
    }

/** 
    Set state of the CFatMountCB
    @param  aState state to set
*/
void CFatMountCB::SetState(TFatMntState aState)
    {
    __PRINT3(_L("#- CFatMountCB::SetState() drv:%d, %d->%d\n"),DriveNumber(),iState,aState);
    iState = aState;
    }


TDriveInterface& CFatMountCB::DriveInterface() const 
    {
    return (TDriveInterface&)iDriverInterface; 
    }

const TFatConfig& CFatMountCB::FatConfig() const 
    {
    return iFatConfig;
    }

//---------------------------------------------------------------------------------------------------------------------------------
/** 
Check if the XFileCreationHelper object is initialised.
*/
TBool CFatMountCB::XFileCreationHelper::IsInitialised() const 
	{
	return isInitialised;
	}

/** 
Get number of new entries for file creation.
*/
TUint16	CFatMountCB::XFileCreationHelper::NumOfAddingEntries() const
	{
	ASSERT(isInitialised); 
	return iNumOfAddingEntries;
	}

/** 
Get position of new entries for file creation.
*/
TEntryPos CFatMountCB::XFileCreationHelper::EntryAddingPos() const 
	{
	ASSERT(isInitialised); 
	return iEntryAddingPos;
	}

/** 
Check if position of new entries has been found.
*/
TBool CFatMountCB::XFileCreationHelper::IsNewEntryPosFound() const 
	{
	ASSERT(isInitialised); 
	return isNewEntryPosFound;
	}

/** 
Check if file name of the new file is a legal dos name.
*/
TBool CFatMountCB::XFileCreationHelper::IsTrgNameLegalDosName() const
	{
	ASSERT(isInitialised); 
	return isTrgNameLegalDosName;
	}

/** 
Set entry position for new entries to be added.
*/
void CFatMountCB::XFileCreationHelper::SetEntryAddingPos(const TEntryPos& aEntryPos) 
	{
	iEntryAddingPos = aEntryPos;
	}

/** 
Set condition if position of new entries has been found.
*/
void CFatMountCB::XFileCreationHelper::SetIsNewEntryPosFound(TBool aFound) 
	{
	isNewEntryPosFound = aFound;
	}


/**
    Checks for "EOC" for all Fat types
    @param  aCluster FAT table entry (cluster number) to check
    @return ETrue    if aCluster is a EOC for the FAT type being used by CFatMountCB
*/
TBool CFatMountCB::IsEndOfClusterCh(TInt aCluster) const
	{
    ASSERT(iFatEocCode);

    if((TUint32)aCluster >= iFatEocCode)
        return ETrue;

    ASSERT((TUint32)aCluster <= iFatEocCode+7);

	return EFalse;
    }

/**
    Sets "End of Cluster Chain" value in aCluster depending on the FAT type.
    @param aCluster cluster to set to end of chain marker
*/
void CFatMountCB::SetEndOfClusterCh(TInt &aCluster) const
	{
    ASSERT(iFatEocCode);
    aCluster = iFatEocCode+7;
	}


//-------  debug methods
#ifdef  _DEBUG
/**
Debug function indicates whether write fails are active or not, for test
@return ETrue if write fails on or not
*/
TBool CFatMountCB::IsWriteFail()const
	{return(iIsWriteFail);}
/**
Switches write fails on or off, for test
@param aIsWriteFail set true or false to set write fails on or off
*/
void CFatMountCB::SetWriteFail(TBool aIsWriteFail)
	{iIsWriteFail=aIsWriteFail;}

/** @return number of write fails to occur, for test */
TInt CFatMountCB::WriteFailCount()const
	{return(iWriteFailCount);}

/**
Set the number of Write fails 
@param aFailCount number of write fails, for test
*/
void CFatMountCB::SetWriteFailCount(TInt aFailCount)
	{iWriteFailCount=aFailCount;}

/** Decrement the number of write fails, for test */
void CFatMountCB::DecWriteFailCount()
	{--iWriteFailCount;}

/** @return Error for a write failure, for test */
TInt CFatMountCB::WriteFailError()const
	{return iWriteFailError;}

/**
Set the write fail error code, for test
@param aErrorValue The Error for a write fails
*/
void CFatMountCB::SetWriteFailError(TInt aErrorValue)
	{iWriteFailError=aErrorValue;}

#endif





//---------------------------------------------------------------------------------------------------------------------------------
// class CFatFormatCB

/** @return pointer to the owning mount object */
CFatMountCB& CFatFormatCB::FatMount()
	{return *(CFatMountCB*)&Mount();}

/**
Returns the local drive used by the file systems from the owning mount
@return Pointer to the local drive 
*/
CProxyDrive* CFatFormatCB::LocalDrive()
	{return(FatMount().LocalDrive());}


//---------------------------------------------------------------------------------------------------------------------------------
// class CFatFileCB

/**
Returns the owning mount from file object

@return pointer to the owning mount object
*/
CFatMountCB& CFatFileCB::FatMount() const
	{return((CFatMountCB&)Mount());}

/**
Returns the fat table used by the file system for this mount

@return Refrence to the Fat table owned by the mount
*/
CFatTable& CFatFileCB::FAT()
	{return(FatMount().FAT());}

/**
Position with in a cluster for a given address

@param aPos Byte position 
*/
TInt CFatFileCB::ClusterRelativePos(TInt aPos)
	{return(FatMount().ClusterRelativePos(aPos));}
/**
Returns Log2 of cluster size from mount

@return cluster size
*/
TInt CFatFileCB::ClusterSizeLog2()
	{return(FatMount().ClusterSizeLog2());}

/*
 Note: this replaces SeekIndex() which was only used in sl_mnt
 to verify whether the seek index had been created/initialised
*/
inline TBool CFatFileCB::IsSeekIndex() const
{return (iSeekIndex==NULL?(TBool)EFalse:(TBool)ETrue); }


//---------------------------------------------------------------------------------------------------------------------------------
// class CFatDirCB

/**
Returns the owning mount from directory object

@return pointer to the owning mount object
*/
CFatMountCB& CFatDirCB::FatMount()
	{return((CFatMountCB&)Mount());}



//---------------------------------------------------------------------------------------------------------------------------------
// class CFatTable

TUint32 CFatTable::FreeClusters() const 
    {
    return iFreeClusters;
    }


//---------------------------------------------------------------------------------------------------------------------------------

inline TFatType CFatTable::FatType() const 
    {
    return iFatType;
    }

inline TBool CFatTable::IsFat12() const
    {
    return iFatType == EFat12;
    }

inline TBool CFatTable::IsFat16() const
    {
    return iFatType == EFat16;
    }

inline TBool CFatTable::IsFat32() const
    {
    return iFatType == EFat32;
    }


/**
    Checks for "EOC" for all Fat types
    @param  aCluster FAT table entry (cluster number) to check
    @return ETrue    if aCluster is a EOC for the FAT type being used by CFatMountCB that owns the CFatTable
*/
inline TBool CFatTable::IsEndOfClusterCh(TUint32 aCluster) const
    {
    ASSERT(iFatEocCode);

    if(aCluster >= iFatEocCode)
        return ETrue;

    ASSERT((TUint32)aCluster <= iFatEocCode+7);
	return EFalse;
    }


/**
@return Maximal number of addresable FAT entries. This value is taken from the owning mount
*/
inline TUint32 CFatTable::MaxEntries() const
    {
        ASSERT(iMaxEntries > 0);
        return iMaxEntries;
    }


// class TDriveInterface
TBool TDriveInterface::NotifyUser() const
	{return(iMount->GetNotifyUser());}


//----------------------------------------------------------------------------------------------------
// class CRawDisk

/**
    Get pointer to the directory cache interface. Any client that reads/writes directory entries 
    MUST do it via this interface.
    Default implementation returns NULL

    @return     pointer to the MWTCacheInterface interface, or NULL if it is not present.
*/
MWTCacheInterface* CRawDisk::DirCacheInterface()
    {
    return NULL;
    }

//---------------------------------------------------------------------------------------------------------------------------------	

/**
    Calculate offset of the page starting position in the cluster 
    @param aPos  the current entry position in bytes in the cluster
    @param aPageSzLog2	page size in log2
    @return		the starting position of the page that contains aPos
*/
inline TUint32 CalculatePageOffsetInCluster(TUint32 aPos, TUint aPageSzLog2)
	{
	return (aPos >> aPageSzLog2) << aPageSzLog2;
	}

#endif //SL_STD_INL





