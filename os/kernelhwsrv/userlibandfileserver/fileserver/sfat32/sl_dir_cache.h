// Copyright (c) 2008-2009 Nokia Corporation and/or its subsidiary(-ies).
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
// f32\sfat32\inc\sl_dir_cache.h
// 
//

/**
 @file
 @internalTechnology
*/

#ifndef SL_DIR_CACHE_H
#define SL_DIR_CACHE_H

#include "sf_memory_man.h"
#include "sf_memory_client.h"
#include "sl_cache.h"
#include <e32hashtab.h>

//---------------------------------------------------------------------------------------------------------------------------------
class 	CDynamicDirCache;


/**
The dynamic directory cache page class
*/
class TDynamicDirCachePage
    {
public:
	enum TPageType
		{
		EUnknown,
		ELocked,
		EUnlocked,
		};

public:
	~TDynamicDirCachePage();
	static TDynamicDirCachePage* NewL(CDynamicDirCache* aOwnerCache, TInt64 aStartMedPos, TUint8* aStartRamAddr);
	
	inline void	SetLocked(TBool);
	inline TBool	IsLocked() const;
	inline TUint8*	StartPtr()	const;
	inline void 	SetStartPtr(TUint8* aPtr);
	inline void	SetValid(TBool aIsValid);
	inline TBool	IsValid()	const;
	inline void	SetPageType(TPageType aType);
	inline TPageType PageType();

	inline TUint32	PageSizeInBytes() const;
    inline TUint32	PageSizeInSegs() const;

    inline void	SetPos(TInt64 aPos);
    inline void	ResetPos();
	inline TInt64	StartPos()	const;

	inline TUint8*	PtrInPage(TInt64 aPos) const;
	inline TBool	PosCachedInPage(TInt64 aPos) const;

	inline void	Deque();

private:
	// declared to disable copying and assignment
	TDynamicDirCachePage& operator=(const TDynamicDirCachePage&);
	TDynamicDirCachePage(const TDynamicDirCachePage&);

	// private constructor, as this class is not supposed to be created on stack
	TDynamicDirCachePage(CDynamicDirCache* aOwnerCache, TInt64 aStartMedPos, TUint8* aStartRamAddr);

public:
	TDblQueLink			iLink;			///< the embedded link object, see TCachePageList
    TInt64  			iStartMedPos;	///< the starting media address that this page caches
    TUint8*				iStartRamAddr;	///< the starting ram address that thsi page lives
    CDynamicDirCache* 	iOwnerCache;	///< pointer to the cache that owns this page
    TBool				iValid 	:1;		///< flag to indicate the validity of the page content
	TBool				iLocked :1;		///< flag to indicate if the page is locked or not
	TPageType			iType;			///< page type, see TPageType
    };

/**
The lookup table entry class
@see	CDynamicDirCache
*/
class TLookupEntry
	{
	public:
		TLookupEntry(): iPos(0), iRange(0), iPage(NULL) {};
		TLookupEntry(TInt64 aPos, TUint32 aRange, TDynamicDirCachePage* aPage): iPos(aPos), iRange(aRange), iPage(aPage) {};
	public:
		TInt64 					iPos;
		TUint32					iRange;
		TDynamicDirCachePage* 	iPage;
	};

//---------------------------------------------------------------------------------------------------------------------------------
typedef TDblQue<TDynamicDirCachePage> TCachePageList;
/**
Dynamic directory cache.
For now it is directly derived from MWTCacheInterface.
Provides caching FAT directory data.
*/
class CDynamicDirCache : public CBase, public MWTCacheInterface
    {
public:
	~CDynamicDirCache();
	static CDynamicDirCache* NewL(TDriveInterface& aDrive, TUint32 aMinPageNum, TUint32 aMaxPageNum, TUint32 aPageSizeLog2, const TDesC& aClientName);

	//-- overloads from the base class
	void    ReadL (TInt64 aPos, TInt aLength, TDes8& aDes);
	void    WriteL(TInt64 aPos, const TDesC8& aDes);
	void    InvalidateCache(void);
    void    InvalidateCachePage(TUint64 aPos);

	TUint32 PosCached(const TInt64& aPosToSearch, TInt64& aCachedPosStart);
	TUint32 CacheSizeInBytes()  const;
	TInt    Control(TUint32 aFunction, TUint32 aParam1, TAny* aParam2);
	void 	SetCacheBasePos(TInt64 aBasePos);
	void 	MakePageMRU(TInt64 aPos);
	TUint32	PageSizeInBytesLog2() const;
	
	TUint32 PageSizeInSegs() const;

    // Debugging functions
	void Dump();
	void Info() const;

protected:
	CDynamicDirCache(TDriveInterface& aDrive, TUint32 aMinSizeInBytes, TUint32 aMaxSizeInBytes, TUint32 aPageSizeInBytesLog2);
	void ConstructL(const TDesC& aClientName);

	void ReadDataFromSinglePageL(TInt64 aPos, TInt aLength, TDes8& aDes);
	void WriteDataOntoSinglePageL(TInt64 aPos, const TUint8* aData, TUint32 aDataLen);
	TDynamicDirCachePage* FindPageByPos(TInt64 aPos);
	TDynamicDirCachePage* UpdateActivePageL(TInt64 aPos);
	TDynamicDirCachePage* AllocateAndLockNewPageL(TInt64 aStartMedPos);
	TUint8* LockPage(TDynamicDirCachePage* aPage);
	TInt 	UnlockPage(TDynamicDirCachePage* aPage);
	TInt 	DecommitPage(TDynamicDirCachePage* aPage);
	inline TInt64  CalcPageStartPos(TInt64 aPos) const;
	void CheckThresholds();
	inline TBool CacheIsFull() const;
	inline TUint32 MaxCacheSizeInPages() const;
	TInt DeQueue(TDynamicDirCachePage* aPage);
	TInt AddFirstOntoQueue(TDynamicDirCachePage* aPage, TDynamicDirCachePage::TPageType aType);
	TInt LookupTblRemove(TInt64 aPagePos);
	TInt LookupTblAdd(TDynamicDirCachePage* aPage);
	TDynamicDirCachePage* LookupTblFind(TInt64 aPos);
	TInt ResetPagePos(TDynamicDirCachePage* aPage);
	void MakePageLastLocked(TDynamicDirCachePage* aPage);
	void DoMakePageMRU(TInt64 aPos);
	void DoInvalidateCache(void);
	
private:
	TUint32				iPageSizeLog2;		///< log2 value of cache pages size in bytes
	TUint32				iMinCacheSizeInBytes;	///< minimum cache data size
	TUint32				iMaxCacheSizeInBytes;	///< maximum cache data size
	TUint32				iMinSizeInPages;	///< minimum cache page number
	TUint32				iMaxSizeInPages;	///< maximum cache page number
	TUint32             iPageSizeInBytes;	///< cache page size in bytes
	TInt64              iCacheBasePos; 		///< cache pages base position, used to align them at cluster size

	TDriveInterface&    iDrive;        		///< reference to the driver for media access
	TUint32             iCacheDisabled : 1; ///< if not 0 the cache is disabled totally and all reads and writes go via TDriveInterface directly

	
	// data structures for LRU page list	
	TCachePageList 	iLockedQ;				///< the locked queue that manages all locked pages, limited by minimum page number
	TCachePageList 	iUnlockedQ;				///< the unlocked queue that manages all locked pages, limited by maximum page number - minimum page number
	TUint32			iLockedQCount;
	TUint32			iUnlockedQCount;

	// data structures for look up table
	THashFunction32<TLookupEntry> 	iHashFunction;
	TIdentityRelation<TLookupEntry>	iIdentityFunction;
	RHashSet<TLookupEntry> 			iLookupTable;	///< a lookup table that used to speed up page look up

	CCacheMemoryClient*	iCacheMemoryClient;	///< interface to cache memory manager
	TUint32 iPermanentlyAllocatedPageCount;	///< count of pages in locked queue that are never unlocked

    };

#include"sl_dir_cache.inl"

#endif //SL_DIR_CACHE_H




