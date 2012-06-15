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
// f32\sfat32\sl_fatcache.cpp
// FAT12 and FAT16 cache implementation
// 
//

/**
 @file
*/

#include "sl_std.h"
#include "sl_fatcache.h"


//#################################################################################################################################
//  CFatCacheBase implementation
//  Base class for all types of FAT cache
//#################################################################################################################################

CFatCacheBase::~CFatCacheBase()
    {
    Close(ETrue); //-- deallocate cache's memory discarding any dirty data
    }

CFatCacheBase::CFatCacheBase()
    {
    iCurrentFatNo = KInvalidFatNo;
    SetDirty(EFalse);
    }


/**
    FAT cache initialisation.

    @param  aOwner pointer to the owning FAT mount
*/
void CFatCacheBase::InitialiseL(CFatMountCB* aOwner)
    {
    ASSERT(aOwner);
    
    Close(ETrue); //-- deallocate cache's memory discarding any dirty data
    
    //-- populate parameters from the owning mount
    iFatType = aOwner->FatType();
    __ASSERT_ALWAYS((iFatType == EFat12 || iFatType == EFat16 || iFatType == EFat32), User::Leave(KErrCorrupt));  
    
    ipDrive = &aOwner->DriveInterface();
    iFatStartPos = aOwner->FirstFatSector() << aOwner->SectorSizeLog2(); 
    iFatSize = aOwner->FatSizeInBytes();
    iNumFATs = (TUint16)aOwner->NumberOfFats();
    iFatSecSzLog2   = (TUint16)aOwner->SectorSizeLog2(); 
    iFatClustSzLog2 = (TUint16)aOwner->ClusterSizeLog2();

    __ASSERT_ALWAYS(iNumFATs >=1, User::Leave(KErrCorrupt));

    __PRINT3(_L("#-CFatCacheBase::InitialiseL() FatStart:%u, FatSz:%d, drv:%d"),iFatStartPos, iFatSize, aOwner->DriveNumber());
    }

//-----------------------------------------------------------------------------
/**
    This method shall be called to check if we are allowed to invalidate dirty cache, i.e. discard non-flushed data.
    The behaviour is hardcoded (see KAllowInvalidateDirtyCache constant)

    @return ETrue if invalidating dirty cache is allowed. Otherwise panics the current thread
*/
TBool CFatCacheBase::CheckInvalidatingDirtyCache() const
    {
    
    //-- If not EFalse, invalidating dirty cache (pages) is allowed. This shall be OK, because
    //-- invalidating the cache is required only after direct media writes to the FAT by RawWrite, which can corrupt it anyway. 
    TBool KAllowInvalidateDirtyCache = ETrue;
    
    if(!IsDirty())
        return KAllowInvalidateDirtyCache;        

    __PRINT(_L("#-CFatCacheBase::Invalidating dirty cache !"));
    
    if(!KAllowInvalidateDirtyCache)
        {
        __ASSERT_ALWAYS(0, Fault(EFatCache_DiscardingDirtyData));
        }

    return KAllowInvalidateDirtyCache;        
    }

//-----------------------------------------------------------------------------

/**
    Read portion of raw data from 1st FAT copy.
    
    @param  aPos   media position in the _FIRST_ FAT to start reading with
    @param  aLen   number of bytes to read
    @param  aData  data descriptor

    @return standard error code.
*/
TInt CFatCacheBase::ReadFatData(TUint32 aPos, TUint32 aLen, TDes8& aData) const
    {
    //__PRINT2(_L("#-CFatCacheNew::ReadFatData() pos:%u, Len:%d"), aPos, aLen);

    //-- this method can pick up data corresponding to invalid FAT entries, like FAT[0], FAT[1] and
    //-- the last portion beyond FAT because of read granularity. This isn't a problem, because the data there
    //-- won't be written on disk.
    ASSERT(aPos >= FatStartPos()); 

    return ipDrive->ReadNonCritical(aPos, aLen, aData);
    }

//-----------------------------------------------------------------------------

/** 
    Writes data to the FAT table, which number is set in iCurrentFatNo member variable. 
    @param  aPos   data media position in the _FIRST_ FAT copy
    @param  aData  data descriptor
    @return standard error code.
*/
TInt CFatCacheBase::WriteFatData(TUint32 aPos, const TDesC8& aData) const
    {
    //__PRINT3(_L("#-CFatCacheBase::WriteFatData() pos:%u, Len:%d, FAT:%d"), aPos, aData.Length(), iCurrentFatNo);

#ifdef _DEBUG    
    //-- FAT[0] and FAT[1] entries are reserved and we must not write data there. It's up to the caller of this method to
    //-- calculate correct data position in FAT
    TInt reserved_Entries_Offset=0;
    switch(iFatType)
        {
        case EFat32: reserved_Entries_Offset = KFatFirstSearchCluster*sizeof(TFat32Entry); break;  //-- FAT32  
        case EFat16: reserved_Entries_Offset = KFatFirstSearchCluster*sizeof(TFat16Entry); break;  //-- FAT16  
        case EFat12: reserved_Entries_Offset = 3;   break;                                         //-- FAT12
        default: ASSERT(0); break;
        }
    ASSERT(aPos >= FatStartPos()+reserved_Entries_Offset);
    ASSERT((aPos+aData.Length()) <= FatStartPos()+FatSize());
    ASSERT(iCurrentFatNo < iNumFATs);
#endif    

    //-- goto the required FAT copy. iCurrentFatNo shall contain FAT number we are writing to.
    aPos+=iCurrentFatNo*FatSize(); 

    return ipDrive->WriteCritical(aPos, aData);
    }

//-----------------------------------------------------------------------------
/** 
    get a pointer to the CFatBitCache interface. 
    @return NULL because it is not present here 
*/
CFatBitCache* CFatCacheBase::BitCacheInterface() 
    {
    return NULL;
    }
    

//#################################################################################################################################
//  CFatPagedCacheBase implementation
//  Base class for all paged FAT caches
//#################################################################################################################################

CFatPagedCacheBase::CFatPagedCacheBase()
               :CFatCacheBase() 
    {
    }


//#################################################################################################################################
//  CFatCachePageBase implementation
//  Base class for FAT cache pages (FAT16 fixed and FAT32 LRU)
//#################################################################################################################################

CFatCachePageBase::CFatCachePageBase(CFatPagedCacheBase& aCache)
                  :iCache(aCache)
    {
    ASSERT(IsPowerOf2(aCache.PageSize()));
    iStartIndexInFAT = KMaxTUint;

    //-- calculate number of FAT entries in the page, it depends on FAT type
    switch(aCache.FatType())
        {
        case EFat32:
            iFatEntriesInPage = PageSize() >> KFat32EntrySzLog2;
        break;
    
        case EFat16:
            iFatEntriesInPage = PageSize() >> KFat16EntrySzLog2;
        break;
    
        default:
            ASSERT(0);
            Fault(EFatCache_BadFatType);
        break;

        };

    SetState(EInvalid); 
    }

CFatCachePageBase::~CFatCachePageBase()
    {
    iData.Close();
    }

//-----------------------------------------------------------------------------
/**
    Mark the page as "invalid". I.e containing inalid data.
    On the first read/write access to such page it will be re-read from the media

    @param aIgnoreDirtyData if ETrue, it is allowed to ignore the fact that the page contains dirty (not flushed) data.
*/
void CFatCachePageBase::Invalidate(TBool aIgnoreDirtyData /*= EFalse*/)
    {
    if(!aIgnoreDirtyData && IsDirty())
        {
        __PRINT1(_L("#-CFatCachePageBase::Invalidate() dirty page! FAT idx:%d"), iStartIndexInFAT);
        __ASSERT_ALWAYS(0, Fault(EFatCache_DiscardingDirtyData));
        }

    iDirtySectors.Clear(); //-- clear dirty sectors bitmap
    SetState(EInvalid);
    }

//-----------------------------------------------------------------------------
/**
    Flush all dirty page sectors to the media and mark the page as "clean" if required.
    If the page is "clean" i.e doesn't contain changed data, does nothing.

    @param  aKeepDirty  if ETrue, the "dirty" flag isn't reset after page flushing.
*/
void CFatCachePageBase::FlushL(TBool aKeepDirty)
    {
    if(!IsDirty())
        return;

    if(!IsValid())
        {
        __PRINT1(_L("#-CFatCachePageBase::FlushL() Invalid page! FAT idx:%d"), iStartIndexInFAT);
        ASSERT(0);
        User::Leave(KErrCorrupt);
        return;
        }

    //__PRINT1(_L("#-CFatCachePageBase::FlushL() FAT idx:%d"), iStartIndexInFAT);

    //-- write dirty FAT sectors  to the media one by one. 
    //-- merging adjacent dirty subsectors into larger clusters and writing them at once looks like a good idea, but
    //-- in reality it showed FAT performance degradation, at least on MMC/SD media.
    
    const TInt MaxSectors = iCache.SectorsInPage();
    
    for(TInt i=0; i<MaxSectors; ++i)
        {
        if(iDirtySectors[i])
            {
            DoWriteSectorL(i);
            }
        }

    //-- All data flushed; mark page as clean if it isn't required not to do.
    if(!aKeepDirty)
        SetClean(); 

    }


//#################################################################################################################################
//  CFat16FixedCache implementation
//  Fixed cache (caches all FAT16) but organised as an array of pages
//#################################################################################################################################

CFat16FixedCache::CFat16FixedCache()
                 :CFatPagedCacheBase(),iPages(1) //-- array granularity is 1
    {
    }

//-----------------------------------------------------------------------------
/**
    FAT16 fixed cache factory function.
    @param  aOwner              pointer to the owning FAT mount
    @param  aFatSize            size of the FAT table in bytes
    @param  aRdGranularityLog2  Log2(read granularity)
    @param  aWrGranularityLog2  Log2(write granularity)

    @return pointer to the constructed object.
*/
CFat16FixedCache* CFat16FixedCache::NewL(CFatMountCB* aOwner, TUint32 aFatSize, TUint32 aRdGranularityLog2, TUint32 aWrGranularityLog2)
    {
    __PRINT(_L("#-CFat16FixedCache::NewL()"));

    CFat16FixedCache* pSelf = NULL;
    pSelf = new (ELeave) CFat16FixedCache;

    CleanupStack::PushL(pSelf);
    pSelf->InitialiseL(aOwner, aFatSize, aRdGranularityLog2, aWrGranularityLog2);
    CleanupStack::Pop();
    
    return pSelf;
    }

//-----------------------------------------------------------------------------
/**
    FAT16 fixed cache initialisation.
    @param  aOwner              pointer to the owning FAT mount
    @param  aFatSize            size of the FAT table in bytes
    @param  aRdGranularityLog2  Log2(read granularity)
    @param  aWrGranularityLog2  Log2(write granularity)
*/
void CFat16FixedCache::InitialiseL(CFatMountCB* aOwner, TUint32 aFatSize, TUint32 aRdGranularityLog2, TUint32 aWrGranularityLog2)
    {
    const TUint32 ReadGranularity = Pow2(aRdGranularityLog2);
    const TUint32 WriteGranularity = Pow2(aWrGranularityLog2);

    __PRINT3(_L("#-CFat16FixedCache::InitialiseL FatSz:%u, RdGr:%d, WrGr:%d"),aFatSize, ReadGranularity, WriteGranularity);
    (void)ReadGranularity;
    (void)WriteGranularity;

    TBool bParamsValid = (aRdGranularityLog2 >= aWrGranularityLog2) && (aWrGranularityLog2 >= KDefSectorSzLog2);
    __ASSERT_ALWAYS(bParamsValid, Fault(EFatCache_BadGranularity));

    CFatPagedCacheBase::InitialiseL(aOwner);
    
    ASSERT(FatType() == EFat16);

    //-- See FAT specs, and round up the limit to the FAT sector boundary
    const TUint32 KMaxFat16Size = ((65524*sizeof(TFat16Entry)+FAT_SectorSz()-1) >> FAT_SectorSzLog2()) << FAT_SectorSzLog2(); 
    const TUint32 KMinFat16Size = 4086*sizeof(TFat16Entry);  //-- See FAT specs
    
    bParamsValid = aFatSize >= KMinFat16Size && aFatSize <= KMaxFat16Size;
	__ASSERT_ALWAYS(bParamsValid, User::Leave(KErrCorrupt));

    //-- cache page size is (2^aRdGranularityLog2) bytes and consists of 2^(aRdGranularityLog2-aWrGranularity) sectors.
    iPageSizeLog2 = aRdGranularityLog2;
    iSectorSizeLog2 = aWrGranularityLog2; //-- Log2(number of sectors in cache page)
    
    __ASSERT_ALWAYS(SectorsInPage() < KMaxSectorsInPage, Fault(EFatCache_BadGranularity));

    const TUint numPages = (aFatSize+(PageSize()-1)) >> iPageSizeLog2;
    __PRINT1(_L("#-CFat16FixedCache Num Pages:%d"), numPages);

    //-- prepare pointer array for pages. NULL entry in the array means that the page at this index isn't allocated.
    for(TUint i=0; i<numPages; ++i)
        iPages.Append(NULL);
    
    }


//-----------------------------------------------------------------------------
/**
    Close the cache and deallocate its memory.
    @param  aDiscardDirtyData if ETrue, will ignore dirty data. If EFalse, will panic on atempt to close dirty cache.  
*/
void CFat16FixedCache::Close(TBool aDiscardDirtyData)
    {
    __PRINT1(_L("#-CFat16FixedCache::Close(%d)"), aDiscardDirtyData);

    TInt cnt = iPages.Count();
    while(cnt--)
        {//-- delete pages
        CFat16FixedCachePage *pPage = iPages[cnt];
        if(pPage && (pPage->IsDirty()))
            {//-- trying to destroy the cache that has dirty pages
            __PRINT1(_L("#-CFat16FixedCache::Close() The page is dirty! Start idx:%d"), pPage->StartFatIndex());
            if(!aDiscardDirtyData)
                {
                __ASSERT_ALWAYS(0, Fault(EFatCache_DiscardingDirtyData));
                }
            //-- ignore this fact if requested.
            }
        
        delete pPage;
        }

    iPages.Close();
    SetDirty(EFalse);
    }

//-----------------------------------------------------------------------------
/**
    Read FAT entry from the cache. 

    @param  aIndex FAT entry index to read
    @return FAT entry value at the index "aIndex" 
*/
TUint32 CFat16FixedCache::ReadEntryL(TUint32 aIndex)
    {
    //__PRINT1(_L("#-CFat16FixedCache::ReadEntryL() FAT idx:%d"), aIndex);
    ASSERT(aIndex >= KFatFirstSearchCluster &&  aIndex < (FatSize() >> KFat16EntrySzLog2));

    //-- calculate page index in the array
    const TInt pgIdx = aIndex >> (PageSizeLog2()-KFat16EntrySzLog2);
    CFat16FixedCachePage *pPage = iPages[pgIdx];
    
    TUint32 entry = KMaxTUint;

    if(!pPage)
        {//-- page at this position isn't allocated yet
        pPage = CFat16FixedCachePage::NewL(*this);
        iPages[pgIdx] = pPage;
        
        //-- read the page from media
        entry = pPage->ReadFromMediaL(aIndex);
        }
    else
        {//-- get cached entry from the page
        TBool bRes = pPage->ReadCachedEntryL(aIndex, entry);
        ASSERT(bRes);
        (void)bRes;
        }

    return entry;
    }

//-----------------------------------------------------------------------------
/**
    Write FAT entry to the cache. 
    Appropriate FAT cache sector will be marked as "dirty" and will be eventually flushed to the media.

    @param  aIndex FAT entry index
    @param  aEntry FAT entry value
*/
void CFat16FixedCache::WriteEntryL(TUint32 aIndex, TUint32 aEntry)
    {
    //__PRINT2(_L("#-CFat16FixedCache::WriteEntryL() FAT idx:%d, val:%d"), aIndex, aEntry);

    ASSERT(aIndex >= KFatFirstSearchCluster &&  aIndex < (FatSize() >> KFat16EntrySzLog2));

    SetDirty(ETrue);

    //-- calculate page index in the array
    const TInt pgIdx = aIndex >> (PageSizeLog2()-KFat16EntrySzLog2);
    CFat16FixedCachePage *pPage = iPages[pgIdx];

    if(!pPage)
        {//-- page at this position isn't allocated yet
        pPage = CFat16FixedCachePage::NewL(*this);
        iPages[pgIdx] = pPage;
        
        //-- read the page from media
        pPage->ReadFromMediaL(aIndex);
        }

    //-- overwrite entry in cache
    TBool bRes = pPage->WriteCachedEntryL(aIndex, aEntry);
    ASSERT(bRes);
    (void)bRes;
    }

/**
    A debug method that asserts that the cache is really clean
*/
void CFat16FixedCache::AssertCacheReallyClean() const
    {
#ifdef _DEBUG 
        for(TUint i=0; i<NumPages(); ++i)
        {
            CFat16FixedCachePage* pPage = iPages[i];
            if(pPage && pPage->IsDirty())
                {
                __PRINT(_L("#-CFat16FixedCache::AssertCacheReallyClean()"));
                ASSERT(0);
                }
        }
#endif   
    }


//-----------------------------------------------------------------------------
/**
    Flushes all dirty data to the media.
*/
void CFat16FixedCache::FlushL()
    {
    if(!IsDirty())
        {
        AssertCacheReallyClean();
        return;
        }


    //-- flush dirty data to all copies of FAT
    for(iCurrentFatNo=0; iCurrentFatNo < NumFATs(); ++iCurrentFatNo)
        {
        const TInt nPages = NumPages();
        for(TInt i=0; i<nPages; ++i)
            {
            const TBool keepDirty = iCurrentFatNo < (NumFATs() - 1);

            CFat16FixedCachePage* pPage = iPages[i];
            if(pPage)
                pPage->FlushL(keepDirty);
            }
       
        }
   
    iCurrentFatNo = KInvalidFatNo;
    SetDirty(EFalse);
    }

//-----------------------------------------------------------------------------
/**
    Invalidate whole cache. All pages will be marked as invalid and will be re-read from the media on first access to them.
    @return always KErrNone
*/
TInt CFat16FixedCache::Invalidate()
    {
    __PRINT(_L("#-CFat16FixedCache::Invalidate()"));
    const TBool bIgnoreDirtyData = CheckInvalidatingDirtyCache();

    //-- iterate through the array of pages marking invalidating every page
    TInt cnt = iPages.Count();
    while(cnt--)
        {//-- delete pages
        CFat16FixedCachePage *pPage = iPages[cnt];
        if(pPage)
            pPage->Invalidate(bIgnoreDirtyData);
        }


    SetDirty(EFalse);

    return KErrNone;
    }

//-----------------------------------------------------------------------------
/**
    Invalidate FAT cache pages that contain FAT entries from aStartIndex to (aStartIndex+aNumEntries)
    These pages will be marked as invalid and will be re-read from the media on first access to them.
    
    @param  aStartIndex FAT start index of the region being invalidated
    @param  aNumEntries number of entries to invalidate
    @return always KErrNone
*/
TInt CFat16FixedCache::InvalidateRegion(TUint32 aStartIndex, TUint32 aNumEntries)
    {
    __PRINT2(_L("#-CFat16FixedCache::InvalidateRegion() startIndex:%d, entries:%d"),aStartIndex, aNumEntries);
    ASSERT(aStartIndex >= KFatFirstSearchCluster &&  aStartIndex < (FatSize() >> KFat16EntrySzLog2));

    if(!aNumEntries)
        {
        ASSERT(0);
        return KErrNone;
        }

    const TBool bIgnoreDirtyData = CheckInvalidatingDirtyCache();
    const TUint startPgIdx  = aStartIndex >> (PageSizeLog2()-KFat16EntrySzLog2);
    const TUint nPagesToInv = 1+(aNumEntries >> (PageSizeLog2()-KFat16EntrySzLog2));

    TUint i;
    //-- invalidate pages that contain [aStartIndex ... aStartIndex+aNumEntries] entries
    for(i=0; i<nPagesToInv; ++i)
        {
        const TUint pageIdx = i+startPgIdx;
        if(pageIdx >= NumPages())
            break;
        
        CFat16FixedCachePage* pPage = iPages[pageIdx];
        if(pPage)
            pPage->Invalidate(bIgnoreDirtyData);
        } 

    SetDirty(EFalse);
   
    //-- check if the cache still has dirty pages
    for(i=0; i<NumPages(); ++i)
        {
        CFat16FixedCachePage* pPage = iPages[i];
        if(pPage && pPage->IsDirty()) 
            {
            SetDirty(ETrue);
            break;
            }      
        }

    return KErrNone;
    }

//#################################################################################################################################
//  CFat16FixedCachePage implementation
//  Page for the FAT16 fixed cache
//#################################################################################################################################

//-----------------------------------------------------------------------------

CFat16FixedCachePage::CFat16FixedCachePage(CFatPagedCacheBase& aCache)
                     :CFatCachePageBase(aCache)
    {
    ASSERT(IsPowerOf2(EntriesInPage()));
    }


/**
    Factory function.
    @param aCache reference to the owning cache.
    @return pointer to the constructed object or NULL on error
*/
CFat16FixedCachePage* CFat16FixedCachePage::NewL(CFatPagedCacheBase& aCache)
    {
    CFat16FixedCachePage* pSelf = NULL;
    pSelf = new (ELeave) CFat16FixedCachePage(aCache);

    CleanupStack::PushL(pSelf);
    
    pSelf->iData.CreateMaxL(aCache.PageSize()); //-- allocate memory for the page
   
    CleanupStack::Pop();

    return pSelf;
    }


//-----------------------------------------------------------------------------
/**
    Read FAT16 entry from the cache. 
    
    1. If page's data are valid, just extracts data from the page buffer.
    2. If page's data are invalid firstly reads data from the media and goto 1
    
    @param  aFatIndex entry's absolute FAT index (from the FAT start)
    @param  aResult on sucess there will be FAT16 entry value
    @return ETrue, because FAT16 cache pages never get eviched.
*/
TBool CFat16FixedCachePage::ReadCachedEntryL (TUint32 aFatIndex, TUint32& aResult)
    {
    if(IsValid())
        {//-- read entry directly from page buffer, the cached data are valid
        aResult = (*GetEntryPtr(aFatIndex)) & KFat16EntryMask;
        }
    else
        {//-- aFatIndex belongs to this page, but the page is invalid and needs to be read from the media
        //__PRINT(_L("#-CFat16FixedCachePage::ReadCachedEntry() The page is invalid, reading from the media"));
        aResult = ReadFromMediaL(aFatIndex);
        }

    return ETrue;
    }

//-----------------------------------------------------------------------------

/**
    Writes FAT cache page sector to the media (to all copies of the FAT)
    @param  aSector sector number winthin this page
*/
void CFat16FixedCachePage::DoWriteSectorL(TUint32 aSector)
    {
    //__PRINT1(_L("#-CFat16FixedCachePage::DoWriteSectorL() startSec:%d, cnt:%d"), aSector);

    ASSERT(aSector < iCache.SectorsInPage());

    TInt offset = 0;

    if(iStartIndexInFAT == 0 && aSector == 0)
        {//-- this is the very beginning of FAT16. We must skip FAT[0] & FAT[1] entries and do not write them to media.    
        offset = KFatFirstSearchCluster << KFat16EntrySzLog2; 
        }    
    
    const TUint8* pData = iData.Ptr()+offset+(aSector << iCache.SectorSizeLog2());
    
    TUint32 dataLen = (1 << iCache.SectorSizeLog2()) - offset;

    const TUint32 mediaPosStart = iCache.FatStartPos() + (iStartIndexInFAT << KFat16EntrySzLog2) + (aSector << iCache.SectorSizeLog2()) + offset; 
    const TUint32 mediaPosEnd = mediaPosStart + dataLen; 

    //-- check if we are going to write beyond FAT. It can happen if the write granularity is bigger that the sector size.
    const TUint32 posFatEnd = iCache.FatStartPos() + iCache.FatSize();
    if(mediaPosEnd > posFatEnd)
        {//-- correct the leength of the data to write.
        dataLen -= (mediaPosEnd-posFatEnd);
        }

    TPtrC8 ptrData(pData, dataLen); //-- source data descriptor 

    TInt nRes = iCache.WriteFatData(mediaPosStart, ptrData);
    
    if(nRes != KErrNone)
        {
        __PRINT1(_L("#-CFat16FixedCachePage::DoWriteSectorsL() failed! code:%d"), nRes);
        User::Leave(nRes);
        }

    }

//-----------------------------------------------------------------------------
/**
    Write FAT16 entry at aFatIndex to the cache. Note that the data are not written to the media, only to the cache page.
    Corresponding page sector is marked as dirty and will be flushed on FlushL() call later.

    1. If page's data are valid, copies data to the page buffer and marks sector as dirty.
    2. If page's data are invalid, firstly reads data from the media and goto 1

    @param  aFatIndex entry's absolute FAT index (from the FAT start)
    @param  aFatEntry FAT16 entry value
    @return ETrue because FAT16 cache pages never get eviched.
*/
TBool CFat16FixedCachePage::WriteCachedEntryL(TUint32 aFatIndex, TUint32 aFatEntry)
    {
    
    ASSERT(IsEntryCached(aFatIndex));

    if(!IsValid())
        {//-- we are trying to write data to the page that has invalid data. //-- read the data from the media first.
        ReadFromMediaL(aFatIndex);
        }

    TFat16Entry* pEntry = GetEntryPtr(aFatIndex);
    
    const TFat16Entry orgEntry = *pEntry;
    *pEntry = (TFat16Entry)((orgEntry & ~KFat16EntryMask) | (aFatEntry & KFat16EntryMask));
    
    //-- mark corresponding sector of the cache page as dirty
    const TUint entryIndexInPage = aFatIndex & (EntriesInPage()-1); //-- number of entries in page is always a power of 2
    const TUint dirtySectorNum   = entryIndexInPage >> (iCache.SectorSizeLog2() - KFat16EntrySzLog2);

    ASSERT(dirtySectorNum < iCache.SectorsInPage());

    iDirtySectors.SetBit(dirtySectorNum);
    SetState(EDirty); //-- mark page as dirty.

    return ETrue;
    }

//-----------------------------------------------------------------------------

/**
    Get a pointer to the FAT16 entry in the page buffer.
    The page 's data shall be valid and the entry shall belong to this page.
    
    @param aFatIndex absolute FAT index (from the FAT start) of the entry
    @return pointer to the FAT16 entry in the page buffer.
*/
TFat16Entry* CFat16FixedCachePage::GetEntryPtr(TUint32 aFatIndex) const
    {
    ASSERT(IsValid() && IsEntryCached(aFatIndex));

    const TUint KEntryIndexInPage = aFatIndex & (EntriesInPage()-1); //-- number of entries in page is always a power of 2
    TFat16Entry* pEntry = ((TFat16Entry*)iData.Ptr()) + KEntryIndexInPage;

    return  pEntry;
    }

//-----------------------------------------------------------------------------
/**
    Read the FAT16 cache page from the media and returns required FAT16 entry.    

    @param  aFatIndex entry's absolute FAT index (from the FAT start)
    @return entry value at aFatIndex.
*/
TUint32 CFat16FixedCachePage::ReadFromMediaL(TUint32 aFatIndex)
    {
    //__PRINT1(_L("#-CFat16FixedCachePage::ReadFromMediaL() FAT idx:%d"), aFatIndex);
    const TUint KFat16EntriesInPageLog2 = iCache.PageSizeLog2()-KFat16EntrySzLog2; //-- number of FAT16 entries in page is always a power of 2

    //-- find out index in FAT this page starts from
    iStartIndexInFAT = (aFatIndex >> KFat16EntriesInPageLog2) << KFat16EntriesInPageLog2;
    SetState(EInvalid); //-- mark the page as invalid just in case if the read fails.

    //-- read page from the media
    const TUint32 pageStartPos = iCache.FatStartPos() + (iStartIndexInFAT << KFat16EntrySzLog2);
    
    TInt nRes = iCache.ReadFatData(pageStartPos, iCache.PageSize(), iData);
    if(nRes != KErrNone)
        {
        __PRINT1(_L("#-CFat16FixedCachePage::ReadFromMediaL() failed! code:%d"), nRes);
        User::Leave(nRes);
        }

    SetClean(); //-- mark this page as clean

    const TFat16Entry entry = (TFat16Entry)((*GetEntryPtr(aFatIndex)) & KFat16EntryMask);

    return entry;
    }


//-----------------------------------------------------------------------------

//#################################################################################################################################
//  CFat12Cache implementation
//  FAT12 non-paged fixed cache. This cache consists from only 1 page, logically divided up to 32 sectors (write granularity unit)
//#################################################################################################################################

CFat12Cache::CFat12Cache()
            :CFatCacheBase()
    {
    }

//-----------------------------------------------------------------------------
/**
    FAT12 fixed cache factory function.
    @param  aOwner              pointer to the owning FAT mount
    @param  aFatSize            size of the FAT table in bytes

    @return pointer to the constructed object.
*/
CFat12Cache* CFat12Cache::NewL(CFatMountCB* aOwner, TUint32 aFatSize)
    {
    __PRINT(_L("#-CFat12Cache::NewL()"));
    CFat12Cache* pSelf = NULL;
    pSelf = new (ELeave) CFat12Cache;

    CleanupStack::PushL(pSelf);
    pSelf->InitialiseL(aOwner, aFatSize);
    CleanupStack::Pop();
    
    return pSelf;
    }

//-----------------------------------------------------------------------------
/**
    FAT16 fixed cache initialisation.
    @param  aOwner              pointer to the owning FAT mount
    @param  aFatSize            size of the FAT table in bytes
*/
void CFat12Cache::InitialiseL(CFatMountCB* aOwner, TUint32 aFatSize)
    {
    __PRINT1(_L("#-CFat12Cache::InitialiseL FatSz:%u"),aFatSize);

    CFatCacheBase::InitialiseL(aOwner);
    ASSERT(FatType() == EFat12);

    //-- see FAT specs; 4084 is a max. number of clusters, fat12 entry is 1.5 bytes; but we need to round up FAT12 size to the sector size
    const TUint32 KMaxFat12Size = ( ((TUint32)(4084*1.5+FAT_SectorSz()-1)) >> FAT_SectorSzLog2()) << FAT_SectorSzLog2();
    const TUint32 KMinFat12Size = FAT_SectorSz();  //-- 1 FAT sector
	__ASSERT_ALWAYS(aFatSize >= KMinFat12Size && aFatSize <= KMaxFat12Size, User::Leave(KErrCorrupt));
    (void)KMaxFat12Size;
    (void)KMinFat12Size;

    //-- as soon as FAT12 max size is 4084 entries or 6126 bytes, the cache is contiguous and divided 
    //-- to logical sectors (write granularity). 

    //-- calculate number write cache sector in the cache
    iSectorsInCache = (aFatSize + (FAT_SectorSz()-1)) >> FAT_SectorSzLog2();
    __ASSERT_ALWAYS(NumSectors() <= KMaxSectorsInCache, Fault(EFatCache_BadGranularity));

    //-- round up cache size to write granularity (sector size)
    const TUint32 cacheSize = NumSectors() << FAT_SectorSzLog2();
    
    //-- create buffer for the whole FAT12
    iData.CreateMaxL(cacheSize);

    //-- this will read whole FAT into the cache 
    User::LeaveIfError(Invalidate()); 
    }

//-----------------------------------------------------------------------------
/**
    Close the cache and deallocate its memory.
    @param  aDiscardDirtyData if ETrue, will ignore dirty data. If EFalse, will panic on atempt to close dirty cache.  
*/
void CFat12Cache::Close(TBool aDiscardDirtyData)
    {
    __PRINT1(_L("#-CFat12Cache::Close(%d)"), aDiscardDirtyData);    
    
    for(TUint32 i=0; i<NumSectors(); ++i)
        {
        if(iDirtySectors[i])
            {//-- trying to destroy the cache that has dirty sectors
            __PRINT1(_L("#-CFat12Cache::Close() The cache is dirty! cache sector:%d"), i);
            if(!aDiscardDirtyData)
                {
                __ASSERT_ALWAYS(0, Fault(EFatCache_DiscardingDirtyData));
                }
            //-- ignore this fact if requested.
            }
        }

    iData.Close();
    SetDirty(EFalse);
    }

//-----------------------------------------------------------------------------
/**
    Read FAT entry from the cache. 

    @param  aIndex FAT entry index to read
    @return FAT entry value at the index "aIndex" 
*/
TUint32 CFat12Cache::ReadEntryL(TUint32 aIndex)
    {
    //__PRINT1(_L("#-CFat12Cache::ReadEntryL() FAT idx:%d"), aIndex);
    ASSERT(aIndex >= KFatFirstSearchCluster &&  aIndex <  (FatSize() + FatSize()/2)); //-- FAT12 entry is 1.5 bytes long

    TUint32 entry;

    if(aIndex & 0x01)
        {//-- odd index
        --aIndex;
        const TUint32 byteIdx = 1 + aIndex + (aIndex >> 1); //-- byteIdx = 1+(aIndex-1)*1.5
        Mem::Copy(&entry, iData.Ptr()+byteIdx, 2); 
        entry >>= 4;   
        }
    else
        {//-- even index
        const TUint32 byteIdx = aIndex + (aIndex >> 1); //-- byteIdx = aIndex*1.5
        Mem::Copy(&entry, iData.Ptr()+byteIdx, 2);
        }

    entry &= KFat12EntryMask; 

    return entry;
    }

//-----------------------------------------------------------------------------
/**
    Write FAT entry to the cache. 
    Appropriate FAT cache sector will be marked as "dirty" and will be eventually flushed to the media.

    @param  aIndex FAT entry index
    @param  aEntry FAT entry value
*/
void CFat12Cache::WriteEntryL(TUint32 aIndex, TUint32 aEntry)
    {
    //__PRINT2(_L("#-CFat12Cache::WriteEntryL() FAT idx:%d, entry:%u"), aIndex, aEntry);
    ASSERT(aIndex >= KFatFirstSearchCluster &&  aIndex <  (FatSize() + FatSize()/2)); //-- FAT12 entry is 1.5 bytes long

    aEntry &= KFat12EntryMask; 
 
    TUint32 byteIdx = 0;
    TUint8 tmp;

    if(aIndex & 0x01)
        {//-- odd index
        --aIndex;
        byteIdx = 1 + aIndex + (aIndex >> 1); //-- byteIdx = 1+(aIndex-1)*1.5
        tmp = (TUint8)(iData[byteIdx] & 0x0F); //-- we modifying a higher nibble 
        tmp |= (TUint8) ((aEntry & 0x0F)<<4);
        iData[byteIdx] = tmp;

        iData[byteIdx+1] = (TUint8)(aEntry >> 4);  
        }
    else
        {//-- even index
        byteIdx = aIndex + (aIndex >> 1); //-- byteIdx = aIndex*1.5
        iData[byteIdx] = (TUint8)aEntry;  

        const TUint32 nextIdx = byteIdx+1;
        tmp = (TUint8)(iData[nextIdx] & 0xF0); //-- we modifying a lower nibble 
        tmp |= (TUint8)((aEntry >> 8) & 0x0F);
        iData[nextIdx] = tmp;

        }

    //-- mark changed sectors dirty. We modified 2 bytes at [byteIdx] and [byteIdx+1]
    iDirtySectors.SetBit(byteIdx >> FAT_SectorSzLog2());
    iDirtySectors.SetBit((byteIdx+1) >> FAT_SectorSzLog2());

    SetDirty(ETrue);
    }

//-----------------------------------------------------------------------------
/**
    A debug method that asserts that the cache is really clean
*/
void CFat12Cache::AssertCacheReallyClean() const
    {
#ifdef _DEBUG 
    if(iDirtySectors.HasBitsSet())
        {
        __PRINT(_L("#-CFat12Cache::AssertCacheReallyClean()"));
        ASSERT(0);
        }

#endif   
    }

//-----------------------------------------------------------------------------
/**
    Flushes all dirty data to the media.
    Walks through all sectors in this cache and flushes dirty ones.
*/
void CFat12Cache::FlushL()
    {
    if(!IsDirty())
        {
        AssertCacheReallyClean();
        return;
        }

    //-- write all dirty sectors to the media (into all copies of FAT)
    for(iCurrentFatNo=0; iCurrentFatNo < NumFATs(); ++iCurrentFatNo)
        {
        for(TUint secNo=0; secNo<NumSectors(); ++secNo)
            {
            if(iDirtySectors[secNo])
                {//-- this sector is dirty, write it to the media

                TInt offset = 0;
                if(secNo == 0)
                    {//-- this is a first sector in FAT. We must skip FAT[0] & FAT[1] entries and do not write them to the media.    
                    offset = 3; //-- 2 FAT12 entries
                    }

                const TUint32 secPos = secNo << FAT_SectorSzLog2(); //-- relative sector position in FAT
                const TUint8* pData = iData.Ptr()+offset+secPos;    //-- pointer to the data in cache buffer
                const TUint32 len = FAT_SectorSz() - offset;          
                TPtrC8 ptrData(pData, len);                         //-- source data descriptor 
                const TUint32 mediaPos = FatStartPos() + secPos + offset;
                
                TInt nRes = WriteFatData(mediaPos, ptrData);

                if(nRes != KErrNone)
                    {
                    __PRINT1(_L("#-CFat12Cache::FlushL() failed! code:%d"), nRes);
                    User::Leave(nRes);
                    }

                }//if(iDirtySectors[secNo])
            }

        }

    iCurrentFatNo = KInvalidFatNo;

    //-- mark the cache as clean
    iDirtySectors.Clear();
    SetDirty(EFalse);
    
    }

//-----------------------------------------------------------------------------
/**
    Invalidates whole cache. Because FAT12 is tiny, just re-reads data from the media to the cache
    @return Media read result code.
*/
TInt CFat12Cache::Invalidate()
    {
    __PRINT(_L("#-CFat12Cache::Invalidate()"));
    CheckInvalidatingDirtyCache();
    
    //-- read whole cache from the media
    const TUint32 posStart = FatStartPos();
    const TUint32 len      = NumSectors() << FAT_SectorSzLog2();
     
    TInt nRes = ReadFatData(posStart, len, iData);
    if(nRes != KErrNone)
        return nRes;

    //-- mark the cache as clean
    SetDirty(EFalse);
    iDirtySectors.Clear();

    return KErrNone;
    }

//-----------------------------------------------------------------------------
/**
    Invalidate wholes cache. Because FAT12 is tiny, just re-reads data from the media to the cache
    @param  aStartIndex ignored
    @param  aNumEntries ignored
    @return Media read result code.
*/
TInt CFat12Cache::InvalidateRegion(TUint32 aStartIndex, TUint32 aNumEntries)
    {
    __PRINT2(_L("#-CFat12Cache::InvalidateRegion() startIndex:%d, entries:%d"),aStartIndex, aNumEntries);
    ASSERT(aStartIndex >= KFatFirstSearchCluster &&  aStartIndex <  (FatSize() + FatSize()/2)); //-- FAT12 entry is 1.5 bytes long
    (void)aStartIndex;
    (void)aNumEntries;

    //-- just re-read all FAT12, it is just 6K max and isn't worth calculating invalid sectors
    return Invalidate();
    }











































