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
// f32\sfat\sl_file.cpp
// 
//

#include "sl_std.h"
#include "sl_cache.h"
#include <e32math.h>

const TInt KSeekIndexSize=128; // Cache 128 clusters
const TInt KSeekIndexSizeLog2=7;
const TInt KFirstClusterNum=2;

CFatFileCB::CFatFileCB()
	{

	__PRINT1(_L("CFatFileCB created 0x%x"),this);
	}

CFatFileCB::~CFatFileCB()
	{
	__PRINT1(_L("CFatFileCB deleted 0x%x"),this);

    //-- a nasty trick to find out if the CFatFileCB is in consistent state on the moment of destruction.
    //-- Because of OOM conditions CFatFileCB might not be fully constructed and to be deleted, while FlushAll()
    //-- implies valid iMount.
    const CMountCB* pMount  = &Mount();
    if(pMount)
        {//-- do some finalisation work if CMountCB is valid
        if (iAtt&KEntryAttModified)
            TRAP_IGNORE(FlushAllL());
        }

    delete[] iSeekIndex;
	}


void CFatFileCB::CreateSeekIndex()
//
// Create a seek index
//
	{

	iSeekIndex = new TUint32[KSeekIndexSize];
	if (iSeekIndex == NULL)
		return;

	Mem::FillZ(iSeekIndex, sizeof(TUint32) * KSeekIndexSize);

	iSeekIndexSize=CalcSeekIndexSize(Size());
	}

TInt CFatFileCB::SeekToPosition(TInt aNewRelCluster,TInt aClusterOffset)
//
// Use the seek index to set iCurrentPos.iCluster as close as possible to aNewRelCluster
// Return aNewRelCluster-aCurrentPos.iCluster
//
	{
	TInt clusterOffset=aClusterOffset;
	TInt seekPos=(aNewRelCluster>>iSeekIndexSize)-1;
	__ASSERT_DEBUG(seekPos<KSeekIndexSize,Fault(EFatFileSeekIndexTooSmall));

	while(seekPos>=0 && iSeekIndex[seekPos]==0 && clusterOffset!=0)
		{
		seekPos--;
		clusterOffset--;
		}
	if (clusterOffset==0) // Counted back to the current cluster
		return(aClusterOffset);
	if (seekPos<0)
		{
		iCurrentPos.iCluster=iStartCluster;
		return(aNewRelCluster);
		}

	iCurrentPos.iCluster=iSeekIndex[seekPos];
	return(aNewRelCluster-((seekPos+1)<<iSeekIndexSize));
	}

void CFatFileCB::SetSeekIndexValueL(TInt aRelCluster,TInt aStoredCluster)
//
// Sets a value in the seekindex
//
	{

	TInt seekPos=(aRelCluster>>iSeekIndexSize)-1;
	__ASSERT_DEBUG(seekPos<KSeekIndexSize,Fault(EFatFileSeekIndexTooSmall));
	__ASSERT_DEBUG(seekPos>=0,Fault(EFatFileSeekIndexTooSmall2));
	iSeekIndex[seekPos] = aStoredCluster;
	}

TBool CFatFileCB::IsSeekBackwards(TUint aPos)
//
// Return true if aPos<currentPos
//
	{
	
	TUint cluster=iCurrentPos.iCluster<<ClusterSizeLog2();
	TInt offset=ClusterRelativePos(iCurrentPos.iPos);
	TUint currentPos=cluster+offset;
	return(aPos<currentPos);
	}

void CFatFileCB::CheckPosL(TUint aPos)
//
// Check that the file is positioned correctly.
// If aPos<currentPos attempt to guess the new position.
//
	{
	__PRINT1(_L("CFatFileCB::CheckPosL(%d)"), aPos);
	if (aPos==iCurrentPos.iPos)
		return;
    __ASSERT_DEBUG(aPos <= (TUint)Size(), Fault(EFatFilePosBeyondEnd));

	if (iFileSizeModified && IsSeekBackwards(aPos))
		FlushDataL(); 
	
	TUint newRelCluster=aPos>>ClusterSizeLog2();
	if ( aPos && (aPos==(newRelCluster<<ClusterSizeLog2())) )
		newRelCluster--;
	TUint oldRelCluster=iCurrentPos.iPos>>ClusterSizeLog2();
	if ( iCurrentPos.iPos && (iCurrentPos.iPos==(oldRelCluster<<ClusterSizeLog2())) )
		oldRelCluster--;	
	TInt clusterOffset=newRelCluster-oldRelCluster;
	TInt oldCluster=iCurrentPos.iCluster;
	iCurrentPos.iPos=aPos;
	if (clusterOffset==0)
		return;
	TInt seekOffset=clusterOffset;
	if (iSeekIndex!=NULL)
		{ // Can alter iCurrentPos.iCluster
		seekOffset=SeekToPosition(newRelCluster,seekOffset);
		if (seekOffset==0)
			return;
		}
	if (clusterOffset==-1 && seekOffset!=1)
		{ // Check previous cluster
		TInt cluster=oldCluster-1;
		if (FAT().GetNextClusterL(cluster) && cluster==oldCluster)
			{
            iCurrentPos.iCluster=oldCluster-1;
			return;
			}
		}
	if (seekOffset<0)
		{
		seekOffset=newRelCluster;
		iCurrentPos.iCluster=iStartCluster;
		}
	while (seekOffset--)
		{
        if (!FAT().GetNextClusterL(iCurrentPos.iCluster))
            {
            __PRINT(_L("CFatFileCB::CheckPosL() corrupt#1"));
            User::Leave(KErrCorrupt);
            }
        TInt cluster=newRelCluster-seekOffset;
		if (iSeekIndex!=NULL && cluster && (cluster>>iSeekIndexSize)<<iSeekIndexSize==cluster)
			SetSeekIndexValueL(cluster,iCurrentPos.iCluster);
		}
	}

void CFatFileCB::SetL(const TFatDirEntry& aFatDirEntry,TShare aShare,const TEntryPos& aPos)
//
// Initialize FileCB from entry data
//
	{

	__PRINT(_L("CFatFileCB::SetL"));
	SetSize(aFatDirEntry.Size()); 
	iCurrentPos.iCluster= FatMount().StartCluster(aFatDirEntry);
	iStartCluster=iCurrentPos.iCluster;
	iCurrentPos.iPos=0;
	iAtt=aFatDirEntry.Attributes();
	iModified= aFatDirEntry.Time(FatMount().TimeOffset());
	iShare=aShare;
	iFileDirPos=aPos;

    SetMaxSupportedSize(KMaxSupportedFatFileSize);
	}

//-----------------------------------------------------------------------------
// from CFileCB::MExtendedFileInterface
void CFatFileCB::ReadL(TInt64 aPos,TInt& aLength, TDes8* aDes, const RMessagePtr2& aMessage, TInt aOffset)
	{
	__PRINT2(_L("CFatFileCB::ReadL aFilePos=%LU aLength=%d"),aPos,aLength);
	
    if((TUint64)aPos > KMaxSupportedFatFileSize-1)
        User::Leave(KErrNotSupported);  //-- max. position in the file is 0xFFFFFFFE

    FatMount().CheckStateConsistentL();
    
	CheckPosL(I64LOW(aPos));
	
	const TUint startPos = iCurrentPos.iPos;
	const TUint curSize  = (TUint)Size();
	const TUint length   = (TUint)aLength;
	
	if((startPos + length > curSize) || (startPos > startPos + length) )
		aLength=curSize-startPos;
	
    FatMount().ReadFromClusterListL(iCurrentPos,aLength,aDes,aMessage,aOffset);
	aLength=iCurrentPos.iPos-startPos;
	}


void CFatFileCB::ReadL(TInt aFilePos,TInt& aLength,const TAny* aTrg,const RMessagePtr2& aMessage)
	{
	ReadL(TInt64(aFilePos),aLength,(TDes8*) aTrg,aMessage, 0);
	}

//-----------------------------------------------------------------------------
// from CFileCB::MExtendedFileInterface
void CFatFileCB::WriteL(TInt64 aPos,TInt& aLength,const TDesC8* aSrc,const RMessagePtr2& aMessage, TInt aOffset)
	{
	__PRINT2(_L("CFatFileCB::WriteL aFilePos=%LU aLength=%d"),aPos,aLength);
	// FAT supports 32 bits only for file size
   	TUint64 endPos = aPos + aLength;
   	if(endPos > KMaxSupportedFatFileSize)
   		User::Leave(KErrNotSupported);
   	
    FatMount().CheckStateConsistentL();
    FatMount().CheckWritableL();
    const TUint pos = I64LOW(aPos);
  	CheckPosL(pos);
  	
	const TUint startCluster = (TUint)iStartCluster;
	const TUint length       = (TUint)aLength;
	
	endPos = iCurrentPos.iPos + length; 
	if ((endPos           > (TUint)Size()) ||
	    (iCurrentPos.iPos > endPos)         ) // Overflow condition 
		DoSetSizeL(iCurrentPos.iPos+length,EFalse);
   	
	TUint startPos=iCurrentPos.iPos;
	TInt badcluster=0;
	TInt goodcluster=0;
   	
	TRAPD(ret, FatMount().WriteToClusterListL(iCurrentPos,aLength,aSrc,aMessage,aOffset,badcluster, goodcluster));
   	
	if (ret == KErrCorrupt || ret == KErrDied)
		{
        if(startCluster == 0)
			{ //Empty File, revert all the clusters allocated.
			TInt cluster = iStartCluster;
			iStartCluster = 0;
			SetSize(0);
			FlushAllL();

			iCurrentPos.iCluster = 0;
			iCurrentPos.iPos = 0;

			FAT().FreeClusterListL(cluster);
			FAT().FlushL();
			}
		else
			{ //Calculate the clusters required based on file size, revert extra clusters if allocated.
			const TUint curSize = (TUint)Size();
			TUint ClustersNeeded = curSize >> ClusterSizeLog2();
			if(curSize > (ClustersNeeded << ClusterSizeLog2()))
				{
				ClustersNeeded++;
				}

			TInt cluster = iStartCluster;
			while(--ClustersNeeded)
				{
				FAT().GetNextClusterL(cluster);
				}
                
			iCurrentPos.iCluster = cluster;

			if (FAT().GetNextClusterL(cluster))
				{
				FAT().FreeClusterListL(cluster);
				}

			FAT().WriteFatEntryEofL(iCurrentPos.iCluster);
			FAT().FlushL();
			}
		}

	User::LeaveIfError(ret);

	if(badcluster != 0)
		{
		if(iStartCluster == badcluster)
			{
			iStartCluster = goodcluster;
			FlushStartClusterL();
			}
		else
			{
			TInt aCluster = iStartCluster;
			do
				{
                if((TUint)badcluster == FAT().ReadL(aCluster))
					{
					FAT().WriteL(aCluster, goodcluster);
					FAT().FlushL();
					break;
					}
				}
			while(FAT().GetNextClusterL(aCluster));
			}
		}
	aLength=iCurrentPos.iPos-startPos;

	if(FatMount().IsRuggedFSys() && pos+(TUint)aLength>(TUint)Size())
		{
		WriteFileSizeL(pos+aLength);
		}

	}


void CFatFileCB::WriteL(TInt aFilePos,TInt& aLength,const TAny* aSrc,const RMessagePtr2& aMessage)
	{
	WriteL(TInt64(aFilePos),aLength,(TDesC8*) aSrc,aMessage, 0);
	}



//-----------------------------------------------------------------------------

void CFatFileCB::ResizeIndex(TInt aNewMult,TUint aNewSize)
//
// Resize the seek index to accomodate a larger or smaller filesize
// Assumes KSeekIndexSize is a power of 2.
//
	{

	TInt maxNewIndex=aNewSize>>(ClusterSizeLog2()+aNewMult);


	TInt    index=0;
	TInt	indexEnd=KSeekIndexSize;
	TInt	newValEnd=maxNewIndex;

	if (iSeekIndexSize<aNewMult)
		{
		TInt newVal=index;
		TInt step=1<<(aNewMult-iSeekIndexSize);
		index+=step-1;
		while(index<indexEnd && newVal<newValEnd)
			{
			iSeekIndex[newVal] =  iSeekIndex[index];
			newVal++;
			index+=step;
			}
		while(newVal<indexEnd)
			iSeekIndex[newVal++] =  0;
		}
	else
		{
		TInt diffSize = iSeekIndexSize-aNewMult;
		TInt oldVal=(KSeekIndexSize>>diffSize) - 1;
		TInt newVal=indexEnd-1;
		TInt skip=(1<<diffSize)-1;

		if ((iSeekIndexSize - aNewMult) > KSeekIndexSizeLog2)
			{
            ClearIndex(0); //-- Invalidate every entry.
			}
		else
			{
			while(newVal>=index)
				{

				iSeekIndex[newVal--] =  iSeekIndex[oldVal--];


				for(TInt i=skip;i>0;i--)
					{	
					iSeekIndex[newVal--] = 0;

					}
				}
			}
		}
	iSeekIndexSize=aNewMult;
	}


/**
    Zero freed clusters in the index

    @param  aNewSize new size of the file that the index corresponds to.
            if = 0  all existing index will be zero filled
*/ 
void CFatFileCB::ClearIndex(TUint aNewSize)
	{

	if (!iSeekIndex)
	    return;

    if(aNewSize==0)
    	{
    	//-- zero fill all the array
        Mem::FillZ(iSeekIndex, KSeekIndexSize*sizeof(TUint32));
		return;
    	}

	// Files that fill up a cluster exactly do not have a trailing empty
	// cluster. So the entry for that position must also be invalidated
	aNewSize--;
	TInt firstInvalidIndex=aNewSize>>(iSeekIndexSize+ClusterSizeLog2());
		
	TInt indexLen=KSeekIndexSize-firstInvalidIndex;

	Mem::FillZ(iSeekIndex+firstInvalidIndex, indexLen * sizeof(TUint32));
	}

TInt CFatFileCB::CalcSeekIndexSize(TUint aSize)
//
// Find the nearest power of 2 > aSize
//
	{
	TInt count = 0;
	const TUint indexSize=KSeekIndexSize<<ClusterSizeLog2();//KSeekIndexSize=128
	if (aSize<=indexSize)
	  return(count);
	
	while((aSize>>=1)>0)
		{
		count++;
		}
	return (count - (KSeekIndexSizeLog2 + ClusterSizeLog2()) + 1);
	}

//-----------------------------------------------------------------------------

void CFatFileCB::SetSizeL(TInt64 aSize)
	{
	__PRINT(_L("CFatFileCB::SetSizeL"));
	
	// FAT supports 32 bits only for file size
	if (I64HIGH(aSize))
		User::Leave(KErrNotSupported);

	if(FatMount().IsRuggedFSys())
		DoSetSizeL(I64LOW(aSize),ETrue);
	else
		DoSetSizeL(I64LOW(aSize),EFalse);
	}


void CFatFileCB::SetSizeL(TInt aSize)
//
// Envelope function around DoSetSizeL to enable aSize to
// be written to disk for rugged fat file system
//
	{
	SetSizeL(TInt64(aSize));
	}

void CFatFileCB::DoSetSizeL(TUint aSize,TBool aIsSizeWrite)
//
// Extend or truncate the file.
// Expects the modified attribute and iSize are set afterwards.
// Does not alter iCurrentPos, the current file position.
// Writes size of file to disk if aIsSizeWrite set
//
	{
	__PRINT2(_L("CFatFileCB::DoSetSizeL sz:%d, fileWrite=%d"),aSize ,aIsSizeWrite);

    FatMount().CheckStateConsistentL();
    FatMount().CheckWritableL();

	
	// Can not change the file size if it is clamped
	if(Mount().IsFileClamped(MAKE_TINT64(0,iStartCluster)) > 0)
		User::Leave(KErrInUse);
	
	iFileSizeModified=ETrue;

	TInt newIndexMult=CalcSeekIndexSize(aSize);
	if (iSeekIndex!=NULL && newIndexMult!=iSeekIndexSize)
		ResizeIndex(newIndexMult,aSize);
	if (aSize == 0)
		{
		if (Size() != 0)
			{
            ClearIndex(0); //-- clear seek index array
			TInt cluster=iStartCluster;
			iStartCluster = 0;
			SetSize(0);
			FlushAllL();
			CheckPosL(0);
			FAT().FreeClusterListL(cluster);
			FAT().FlushL();
			}
		return;
		}
	if (aSize<(TUint)Size())
		{
		if(aIsSizeWrite)		// write file size if decreasing
				WriteFileSizeL(aSize);
		CheckPosL(aSize);
		TInt cluster=iCurrentPos.iCluster;
		if (FAT().GetNextClusterL(cluster))
			{
			FAT().WriteFatEntryEofL(iCurrentPos.iCluster);
			FAT().FreeClusterListL(cluster);
			}
		ClearIndex(aSize);
		FAT().FlushL();
		return;
		}
	
	TUint newSize=aSize>>ClusterSizeLog2();	//	Number of clusters we now need
	if (aSize > (newSize<<ClusterSizeLog2()))
		newSize++;	//	File size is not an exact multiple of cluster size
					//	Increment the number of clusters required to accomodate tail
	
	if (iStartCluster==0)
		{
        //-- FAT().FreeClusterHint() will give us a hint of the last free cluster
        ClearIndex(0); //-- clear seek index array
        TInt tempStartCluster=FAT().AllocateClusterListL(newSize, FAT().FreeClusterHint());
		FAT().FlushL();
		iCurrentPos.iCluster=tempStartCluster;
		iStartCluster=tempStartCluster;
		SetSize(aSize);
		FlushAllL();
		}
	else
		{
		const TUint curSize = (TUint)Size(); 
		TUint oldSize=curSize>>ClusterSizeLog2();	//	Number of clusters we had previously
		if (curSize>(oldSize<<ClusterSizeLog2()))
			oldSize++;
	
		TInt newClusters=newSize-oldSize;	//	Number of clusters we need to prepare
		if (newClusters)
			{
			TEntryPos currentPos=iCurrentPos;
			CheckPosL(Size());
			FAT().ExtendClusterListL(newClusters,iCurrentPos.iCluster);
			iCurrentPos=currentPos;
			}
		FAT().FlushL();
		if(aIsSizeWrite)			// write file size if increasing
			WriteFileSizeL(aSize);
		}
	}

//-----------------------------------------------------------------------------
/**
    Set the entry's attributes and modified time.
*/
void CFatFileCB::SetEntryL(const TTime& aTime,TUint aSetAttMask,TUint aClearAttMask)
	{
	__PRINT(_L("CFatFileCB::SetEntryL"));
    
    FatMount().CheckStateConsistentL();
    FatMount().CheckWritableL();

	TUint setAttMask=aSetAttMask&KEntryAttMaskSupported;
	if (setAttMask|aClearAttMask)
		{
		iAtt|=setAttMask;
		iAtt&=(~aClearAttMask);
		}
	if (aSetAttMask&KEntryAttModified)
		iModified=aTime;
	iAtt|=KEntryAttModified;
	}

/**
    This is a RuggedFAT - specific method. Writes file size to the corresponding field of this
    file direcrory entry.
*/
void CFatFileCB::WriteFileSizeL(TUint aSize)
	{
	__PRINT(_L("CFatFileCB::WriteFileSizeL"));
	TEntryPos entryPos=iFileDirPos;
	entryPos.iPos+=_FOFF(SFatDirEntry,iSize);
	TPtrC8 size((TUint8*)&aSize,sizeof(TUint));
	
    //-- use directory cache when dealing with directories
    FatMount().DirWriteL(entryPos,size);
	iFileSizeModified=EFalse;
    }

//-----------------------------------------------------------------------------
/** 
    Flush file size, attributes, time etc. to the media.
    It doesn't matter if whole directory entry is being written of only part of it. Anyway, a single DOS
    dir. entry always fits into 1 sector.
*/
void CFatFileCB::FlushDataL()
	{
	__PRINT(_L("CFatFileCB::FlushDataL"));
    FlushAllL();
	}

//-----------------------------------------------------------------------------
/** 
    Flush the fide directory entry data: files size, attributes, time etc. 
*/
void CFatFileCB::FlushAllL()
	{
	__PRINT(_L("CFatFileCB::FlushAllL()"));

    if (Mount().IsCurrentMount()==EFalse)
		User::Leave(KErrDisMounted);

    FatMount().CheckStateConsistentL();
    FatMount().CheckWritableL();

	TFatDirEntry entry;
	FatMount().ReadDirEntryL(iFileDirPos,entry);
	__ASSERT_ALWAYS(entry.IsEndOfDirectory()==EFalse,User::Leave(KErrCorrupt));
	entry.SetAttributes(iAtt&KEntryAttMaskSupported);
	entry.SetSize(Size());
	entry.SetTime(iModified, FatMount().TimeOffset());
	entry.SetStartCluster(iStartCluster);

	TBool setNotify = FatMount().GetNotifyUser();
	if(setNotify)
		{
		FatMount().SetNotifyOff();	// do not launch a notifier
		}

	TRAPD(ret, FatMount().WriteDirEntryL(iFileDirPos,entry));
	
	if(setNotify)
		{
		FatMount().SetNotifyOn();
		}

	User::LeaveIfError(ret);
	iAtt&=(~KEntryAttModified);
	iFileSizeModified=EFalse;
	}

//-----------------------------------------------------------------------------

/**
    Rename already opened file.
    @param  aNewName new file name; all trailing dots from the name will be removed
*/
void CFatFileCB::RenameL(const TDesC& aNewName)
	{
    __PRINT2(_L("CFatFileCB::RenameL[0x%x], name:%S"),this, &aNewName);

    FatMount().CheckStateConsistentL();
    FatMount().CheckWritableL();

    const TPtrC fileName = RemoveTrailingDots(aNewName); //-- remove trailing dots from the name


	FatMount().DoRenameOrReplaceL(*iFileName, fileName, CFatMountCB::EModeRename,iFileDirPos);
	
    AllocBufferL(iFileName, fileName);
	
	if(!FatMount().IsRuggedFSys())
		FAT().FlushL();
	}


//***********************************************************
//* BlockMap interface
//***********************************************************
	
TInt CFatFileCB::BlockMap(SBlockMapInfo& aInfo, TInt64& aStartPos, TInt64 aEndPos)
//
// Retrieves the block map of a given section of the file, in the FAT file system.
//	
	{
	__PRINT2(_L("CFatFileCB::BlockMap aStartPos=%ld aEndPos=%ld"), aStartPos, aEndPos);
	
	if ( I64HIGH(aStartPos) || I64HIGH(aEndPos) )
		return KErrNotSupported;

    TUint startPos = I64LOW(aStartPos);
	TUint endPos = I64LOW(aEndPos);

	// aEndPos will always be >=0 at this point
	const TUint length = endPos - startPos;
	
	// Store the position of cluster zero in aInfo
	CFatMountCB& fatMount = FatMount();

	TInt drvNo=-1;
	TBusLocalDrive* locDrv;
	if((fatMount.LocalDrive()->GetLocalDrive(locDrv)==KErrNone) && ((drvNo=GetLocalDriveNumber(locDrv))>=0) && (drvNo<KMaxLocalDrives))
		aInfo.iLocalDriveNumber=drvNo;
	else
		return KErrNotSupported;

	// Fetch the address of cluster 0
	aInfo.iStartBlockAddress = fatMount.FAT().DataPositionInBytes(KFirstClusterNum);

	TRAPD(r, CheckPosL(startPos));
	if (r != KErrNone)
		return r;

	aInfo.iBlockStartOffset = fatMount.ClusterRelativePos(iCurrentPos.iPos);
	aInfo.iBlockGranularity = 1 << FatMount().ClusterSizeLog2();
	const TUint myStartPos = iCurrentPos.iPos;
	if ( myStartPos + length > (TUint)Size())
		return KErrArgument;

	TRAP(r, FatMount().BlockMapReadFromClusterListL(iCurrentPos, length, aInfo));
	if (r != KErrNone)
		return r;

	aStartPos = iCurrentPos.iPos;
	if ((I64LOW(aStartPos) == (TUint)Size()) || ( I64LOW(aStartPos) == (myStartPos + length)))
		return KErrCompletion;
	else
		return KErrNone;
	}



TInt CFatFileCB::GetInterface(TInt aInterfaceId,TAny*& aInterface,TAny* aInput)
	{
	switch(aInterfaceId)
		{
		case EExtendedFileInterface:
			((CFileCB::MExtendedFileInterface*&) aInterface) = this;
			return KErrNone;

		case EBlockMapInterface:
			aInterface = (CFileCB::MBlockMapInterface*) this;
			return KErrNone;

		case EGetLocalDrive:
			return FatMount().LocalDrive()->GetLocalDrive((TBusLocalDrive*&) aInterface);

		default:
			return CFileCB::GetInterface(aInterfaceId,aInterface,aInput);
		}
	}




/**
    Overwrites file's start cluster (iStartCluster) in its directory entry.
*/
void CFatFileCB::FlushStartClusterL()
	{
	__PRINT(_L("CFatFileCB::FlushStartClusterL"));

    CFatMountCB& mount = FatMount();
    TFatDirEntry dirEntry;
    
    mount.ReadDirEntryL(iFileDirPos, dirEntry);      //-- read this file's dir. entry
    dirEntry.SetStartCluster(iStartCluster);         //-- set new start cluster
    mount.WriteDirEntryL(iFileDirPos, dirEntry);//-- write the entry back
	}





