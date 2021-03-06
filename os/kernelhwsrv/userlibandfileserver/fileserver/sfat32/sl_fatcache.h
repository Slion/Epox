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
// f32\sfat\sl_facache.h
// FAT cache base classes definition
// FAT12 and FAT16 cache classes definition
// 
//

/**
 @file
 @internalTechnology
*/

#ifndef SL_FAT_CACHE_H
#define SL_FAT_CACHE_H


//-----------------------------------------------------------------------------

class CFatBitCache;

/**
    An abstract base class for all types of FAT caches.
    Provides user interface and some common for all types of FAT caches functionality.
*/
class CFatCacheBase : public CBase
{
 public:

    virtual ~CFatCacheBase();

    //-- public interface
    virtual void Close(TBool /*aDiscardDirtyData*/) {};
    virtual void FlushL() = 0;

    virtual TUint32 ReadEntryL(TUint32 aIndex) = 0;
    virtual void WriteEntryL(TUint32 aIndex, TUint32 aEntry) = 0;
    
    virtual TInt Invalidate() = 0;
    virtual TInt InvalidateRegion(TUint32 aStartEntry, TUint32 aNumEntries) = 0;

    TInt ReadFatData(TUint32 aPos, TUint32 aLen, TDes8& aData) const;
    TInt WriteFatData(TUint32 aPos, const TDesC8& aData) const;

    inline TUint32  FatStartPos() const;
    inline TUint32  FatSize() const;
    inline TFatType FatType() const;

 public:
    
    //-- auxilary interface to additional bit supercache (it may exist only in FAT32 cache implementation)
    virtual CFatBitCache* BitCacheInterface();


 protected:
    CFatCacheBase();

    virtual void InitialiseL(CFatMountCB* aOwner);

    inline TBool IsDirty() const;
    inline void SetDirty(TBool aDirty);
    inline TUint NumFATs() const;

    TBool CheckInvalidatingDirtyCache() const;

    inline TUint FAT_SectorSzLog2() const;
    inline TUint FAT_SectorSz() const; 
    inline TUint FAT_ClusterSzLog2() const;

 protected:
    
    enum {KInvalidFatNo = 0xFF}; ///< used to invalidate current FAT no.
    TUint   iCurrentFatNo;       ///< current FAT number WriteFatData will write to.

 private:    
    //-- values cached from owning mount.
    TUint32     iFatStartPos;   ///< media position of FAT1 start 
    TUint32     iFatSize;       ///< size of FAT in bytes
    TUint16     iNumFATs;       ///< number of FATs on the volume
    TUint16     iFatSecSzLog2;  ///< Log2(FAT Sector size)
    TUint16     iFatClustSzLog2;///< Log2(FAT cluster size)
    TFatType    iFatType;       ///< FAT type
    TDriveInterface* ipDrive;///< interface to the media driver
    //---

    TBool       iDirty;         ///< ETrue if the cache is dirty
};


//-----------------------------------------------------------------------------

/**
    Fixed FAT12 cache. This is a contiguous cache that caches whole FAT12.
    This cache is logically divided to sectors, maximal number of sectors in this cache is KMaxSectorsInCache (32).
    
    Read granularity: whole cache; anyway it can't be larger than 6126 bytes.
    Write granularity: cache sector size, which is always "FAT Sector Size" and non-configurable.
*/
class CFat12Cache : public CFatCacheBase
{
 public:
    static CFat12Cache* NewL(CFatMountCB* aOwner, TUint32 aFatSize);

    //-- overrides from base class
    virtual void Close(TBool aDiscardDirtyData);
    virtual void FlushL();

    virtual TUint32 ReadEntryL(TUint32 aIndex);
    virtual void WriteEntryL(TUint32 aIndex, TUint32 aEntry);

    virtual TInt Invalidate();
    virtual TInt InvalidateRegion(TUint32 aStartEntry, TUint32 aNumEntries);
    //------------------------------------

 private:
    
    void InitialiseL(CFatMountCB* aOwner, TUint32 aFatSize); 

    CFat12Cache();
    CFat12Cache(const CFat12Cache&);
    CFat12Cache& operator=(const CFat12Cache&);


    inline TUint32 NumSectors() const;
    void AssertCacheReallyClean() const;

 private:

    enum {KMaxSectorsInCache = 32};  ///< maximal number sectors in FAT12 cache
    enum {KFat12EntryMask = 0x0FFF}; ///< FAT12 entry mask

    TUint32 iSectorsInCache;    ///< total number sectors in the cache, KMaxSectorsInCache max.
    T32Bits iDirtySectors;      ///< dirty sectors bitmap. '1' bit corresponds to the dirty sector;
    RBuf8   iData;              ///< Whole FAT12 cache data.
};


//-----------------------------------------------------------------------------

/**
    Abstract base class for paged caches, i.e. those that consist of some number of cache pages.
    In this case the most of the functionality is implemented in page classes and this is just a page container.
    Each cache page in turn is logically divided to sectors. The sector is a logical unit of write granularity
    See also CFatCachePageBase et al.
*/
class CFatPagedCacheBase : public CFatCacheBase
{
 public:

    inline TUint PageSizeLog2()  const;
    inline TUint PageSize()      const;
    
    inline TUint SectorSizeLog2() const;
    inline TUint SectorsInPage()  const;

 protected:
    CFatPagedCacheBase();

 protected:
    
    enum {KMaxSectorsInPage = 32}; ///< maximal number sectors in FAT cache page

    TUint iPageSizeLog2;    ///< Log2(page size)
    TUint iSectorSizeLog2;  ///< Log2(page sector size)
 
};

//-----------------------------------------------------------------------------

class CFat16FixedCachePage;

/**
    FAT16 fixed paged cache. Used for FAT16 only and caches whole FAT16 (its max size is 131048 bytes).
    Consists of the fixed array of cache pages; Pages are allocated on demand and never get evicted.
    Each page is logically divided to page sectors. The number of pages depends on the FAT16 size.

    Read granularity: One page, which size is 2^aRdGranularityLog2
    Write granularity: cache's page sector; its size is 2^aWrGranularityLog2
*/
class CFat16FixedCache : public CFatPagedCacheBase
{
 public:

    static CFat16FixedCache* NewL(CFatMountCB* aOwner, TUint32 aFatSize, TUint32 aRdGranularityLog2, TUint32 aWrGranularityLog2);

    //-- overrides from base class
    virtual void Close(TBool aDiscardDirtyData);
    virtual void FlushL();

    virtual TUint32 ReadEntryL(TUint32 aIndex);
    virtual void WriteEntryL(TUint32 aIndex, TUint32 aEntry);
    

    virtual TInt Invalidate();
    virtual TInt InvalidateRegion(TUint32 aStartEntry, TUint32 aNumEntries);
    //------------------------------------

 private:

    void InitialiseL(CFatMountCB* aOwner, TUint32 aFatSize, TUint32 aRdGranularityLog2, TUint32 aWrGranularityLog2); 
    
    CFat16FixedCache();
    CFat16FixedCache(const CFat16FixedCache&);
    CFat16FixedCache& operator=(const CFat16FixedCache&);

    inline TUint NumPages() const;
    void AssertCacheReallyClean() const;

 private:    
    RPointerArray<CFat16FixedCachePage> iPages;  ///< array of pointer to the cahe pages; if the entry is NULL, it means that the page isn't allocated yet.

};


//-----------------------------------------------------------------------------


/**
    An abstract base class for the cache page. Paged caches, i.e derived form CFatPagedCacheBase uses this functionality.
    Provides an interface and common functionality for all types of cache pages.

    The FAT cache page contains a number of FAT16 or FAT32 entries, their number is always the power of 2.
    The page is logically divided into sectors, the maximal number of sectors in the page is KMaxSectorsInPage (32).
    The page read granularity is whole page and the write granularity is the sector  (see aRdGranularityLog2, aWrGranularityLog2 from the cache)

    The caching is write-back, i.e WriteCachedEntryL() modifies data in the cache and marks corresponding page sector as dirty.
    FlushL() shall be called to flust all dirty sectors in page to the media

*/
class CFatCachePageBase : public CBase
{
public:
    
    ~CFatCachePageBase();

    //----------------
    virtual TBool ReadCachedEntryL (TUint32 aFatIndex, TUint32& aResult) = 0;
    virtual TBool WriteCachedEntryL(TUint32 aFatIndex, TUint32 aFatEntry) = 0; 
    virtual TUint32 ReadFromMediaL(TUint32 aFatIndex) = 0;
    virtual void FlushL(TBool aKeepDirty);
    
    //----------------
    inline TBool IsEntryCached(TUint32 aFatIndex) const ;
    void Invalidate(TBool aIgnoreDirtyData = EFalse);
    
    inline TBool IsDirty() const;
    inline TBool IsValid() const;
    
    inline TUint32 StartFatIndex() const;

protected:
    CFatCachePageBase(CFatPagedCacheBase& aCache);

    /** possible states of the page */
    enum TState
        {
        EInvalid, ///< the page's data are invalid
        EClean,   ///< the page is clean, data valid and the same as on the media  
        EDirty    ///< the page is dirty, there are data eventually to be flushed to the media, iDirtySectors contains dirty sectors bitmap.
        };

    inline void SetState(TState aState);
    inline TState State() const;
    inline void SetClean();
    inline TUint32 PageSize() const; 
    inline TUint32 NumSectors() const; 
    
    virtual void DoWriteSectorL(TUint32 aSector)=0;
    inline TUint32 EntriesInPage() const;

protected:
    TUint32 iStartIndexInFAT;   ///< FAT index this page starts from
    T32Bits iDirtySectors;      ///< dirty sectors bitmap. '1' bit corresponds to the dirty sector;
    CFatPagedCacheBase& iCache; ///< reference to the owher cache
    RBuf8   iData;              ///< page Data

private:
    TState  iState;             ///< page state
    TUint32 iFatEntriesInPage;  ///< number of FAT entries in the page. 

};


//---------------------------------------------------------------------------------------------------------------------------------

/**
    FAT16 cache page. Used only by CFat16FixedCache.
*/
class CFat16FixedCachePage : public CFatCachePageBase
{
 public:
    ~CFat16FixedCachePage() {}
    
    static CFat16FixedCachePage* NewL(CFatPagedCacheBase& aCache);

    //-- overrides
    virtual TBool ReadCachedEntryL (TUint32 aFatIndex, TUint32& aResult);
    virtual TBool WriteCachedEntryL(TUint32 aFatIndex, TUint32 aFatEntry); 
    virtual TUint32 ReadFromMediaL(TUint32 aFatIndex);
    //----

 private:
    CFat16FixedCachePage(CFatPagedCacheBase& aCache);

    //-- outlaws here
    CFat16FixedCachePage();
    CFat16FixedCachePage(const CFat16FixedCachePage&);
    CFat16FixedCachePage& operator=(const CFat16FixedCachePage&);

    virtual void DoWriteSectorL(TUint32 aSector);

    inline TFat16Entry* GetEntryPtr(TUint32 aFatIndex) const;

 private:
    enum {KFat16EntryMask = 0xFFFF}; ///< FAT16 entry mask
};



//---------------------------------------------------------------------------------------------------------------------------------



#include "sl_fatcache.inl"


#endif //SL_FAT_CACHE_H























