// Copyright (c) 2005-2009 Nokia Corporation and/or its subsidiary(-ies).
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
//

#include "memmodel.h"
#include "mm.h"
#include "mmu.h"

#include "mpager.h"
#include "mrom.h"
#include "mobject.h"
#include "mmapping.h"
#include "maddressspace.h"
#include "mmanager.h"
#include "mptalloc.h"
#include "mpagearray.h"
#include "mswap.h"
#include "mthrash.h"
#include "cache_maintenance.inl"


const TUint16 KDefaultYoungOldRatio = 3;
const TUint16 KDefaultMinPages = 256;
#ifdef _USE_OLDEST_LISTS
const TUint16 KDefaultOldOldestRatio = 3;
#endif

const TUint KMinOldPages = 1;

/*	On a 32 bit system without PAE can't have more than 2^(32-KPageShift) pages.
 *	Subtract 1 so it doesn't overflow when converted to bytes.
*/
const TUint	KAbsoluteMaxPageCount = (1u<<(32-KPageShift))-1u;



DPager ThePager;


DPager::DPager()
	: iMinimumPageCount(0), iMaximumPageCount(0), iYoungOldRatio(0),
	  iYoungCount(0),iOldCount(0),iNumberOfFreePages(0)
	{
	}


void DPager::Init2()
	{
	TRACEB(("DPager::Init2()"));

#if defined(__CPU_ARM)

/** Minimum number of young pages the demand paging live list may have.
	Need at least 4 mapped pages to guarantee to be able to execute all ARM instructions,
	plus enough pages for 4 page tables to map those pages, plus enough pages for the
	page table info structures of those page tables.
	(Worst case is a Thumb-2 STM instruction with both instruction and data straddling chunk
	boundaries.)
*/
	iMinYoungPages = 4											// pages
					+(4+KPtClusterSize-1)/KPtClusterSize		// page table pages
					+(4+KPageTableInfosPerPage-1)/KPageTableInfosPerPage;	// page table info pages

#elif defined(__CPU_X86)

/*	Need at least 6 mapped pages to guarantee to be able to execute all ARM instructions,
	plus enough pages for 6 page tables to map those pages, plus enough pages for the
	page table info structures of those page tables.
	(Worst case is (?) a MOV [X],[Y] instruction with instruction, 'X' and 'Y' all
	straddling chunk boundaries.)
*/
	iMinYoungPages = 6											// pages
					+(6+KPtClusterSize-1)/KPtClusterSize		// page table pages
					+(6+KPageTableInfosPerPage-1)/KPageTableInfosPerPage;	// page table info pages

#else
#error Unknown CPU
#endif

#ifdef __SMP__
	// Adjust min page count so that all CPUs are guaranteed to make progress.
	// NOTE: Can't use NKern::NumberOfCpus here because we haven't finished booting yet and will
	// always have only one CPU running at this point...

	// TODO: Before we can enable this the base test configuration needs
	// updating to have a sufficient minimum page size...
	//
	// iMinYoungPages *= KMaxCpus;
#endif

	// A minimum young/old ratio of 1 means that we need at least twice iMinYoungPages pages...
	iAbsoluteMinPageCount = 2*iMinYoungPages;

	__NK_ASSERT_DEBUG(KMinOldPages<=iAbsoluteMinPageCount/2);

	// initialise live list...
	TUint minimumPageCount = 0;
	TUint maximumPageCount = 0;

	SDemandPagingConfig config = TheRomHeader().iDemandPagingConfig;

	iMinimumPageCount = KDefaultMinPages;
	if(minimumPageCount)
		iMinimumPageCount = minimumPageCount;
	if(config.iMinPages)
		iMinimumPageCount = config.iMinPages;
	if(iMinimumPageCount<iAbsoluteMinPageCount)
		iMinimumPageCount = iAbsoluteMinPageCount;
	iInitMinimumPageCount = iMinimumPageCount;

	iMaximumPageCount = KMaxTInt;
	if(maximumPageCount)
		iMaximumPageCount = maximumPageCount;
	if(config.iMaxPages)
		iMaximumPageCount = config.iMaxPages;
	if (iMaximumPageCount > KAbsoluteMaxPageCount)
		iMaximumPageCount = KAbsoluteMaxPageCount;
	iInitMaximumPageCount = iMaximumPageCount;

	iYoungOldRatio = KDefaultYoungOldRatio;
	if(config.iYoungOldRatio)
		iYoungOldRatio = config.iYoungOldRatio;
	TInt ratioLimit = (iMinimumPageCount-KMinOldPages)/KMinOldPages;
	if(iYoungOldRatio>ratioLimit)
		iYoungOldRatio = ratioLimit;

#ifdef _USE_OLDEST_LISTS
	iOldOldestRatio = KDefaultOldOldestRatio;
	if(config.iSpare[2])
		iOldOldestRatio = config.iSpare[2];
#endif

	iMinimumPageLimit = (iMinYoungPages * (1 + iYoungOldRatio)) / iYoungOldRatio;
	if(iMinimumPageLimit<iAbsoluteMinPageCount)
		iMinimumPageLimit = iAbsoluteMinPageCount;

	TRACEB(("DPager::Init2() live list min=%d max=%d ratio=%d",iMinimumPageCount,iMaximumPageCount,iYoungOldRatio));

	if(iMaximumPageCount<iMinimumPageCount)
		__NK_ASSERT_ALWAYS(0);

	//
	// This routine doesn't acquire any mutexes because it should be called before the system
	// is fully up and running. I.e. called before another thread can preempt this.
	//

	// Calculate page counts
	TUint minOldAndOldest = iMinimumPageCount / (1 + iYoungOldRatio);
	if(minOldAndOldest < KMinOldPages)
		__NK_ASSERT_ALWAYS(0);
	if (iMinimumPageCount < minOldAndOldest)
		__NK_ASSERT_ALWAYS(0);
	TUint minYoung = iMinimumPageCount - minOldAndOldest;
	if(minYoung < iMinYoungPages)
		__NK_ASSERT_ALWAYS(0); // Need at least iMinYoungPages pages mapped to execute worst case CPU instruction
#ifdef _USE_OLDEST_LISTS
	// There should always be enough old pages to allow the oldest lists ratio.
	TUint oldestCount = minOldAndOldest / (1 + iOldOldestRatio);
	if (!oldestCount)
		__NK_ASSERT_ALWAYS(0);
#endif
	iNumberOfFreePages = 0;
	iNumberOfDirtyPages = 0;

	// Allocate RAM pages and put them all on the old list
	RamAllocLock::Lock();
	iYoungCount = 0;
	iOldCount = 0;
#ifdef _USE_OLDEST_LISTS
	iOldestCleanCount = 0;
	iOldestDirtyCount = 0;
#endif
	Mmu& m = TheMmu;
	for(TUint i=0; i<iMinimumPageCount; i++)
		{
		// Allocate a single page
		TPhysAddr pagePhys;
		TInt r = m.AllocRam(&pagePhys, 1, 
							(Mmu::TRamAllocFlags)(EMemAttNormalCached|Mmu::EAllocNoWipe|Mmu::EAllocNoPagerReclaim), 
							EPageDiscard);
		if(r!=KErrNone)
			__NK_ASSERT_ALWAYS(0);
		MmuLock::Lock();
		AddAsFreePage(SPageInfo::FromPhysAddr(pagePhys));
		MmuLock::Unlock();
		}
	RamAllocLock::Unlock();

#ifdef _USE_OLDEST_LISTS
	TRACEB(("DPager::Init2() end with young=%d old=%d oldClean=%d oldDirty=%d min=%d free=%d max=%d",iYoungCount,iOldCount,iOldestCleanCount,iOldestDirtyCount,iMinimumPageCount,iNumberOfFreePages,iMaximumPageCount));
#else
	TRACEB(("DPager::Init2() end with young=%d old=%d min=%d free=%d max=%d",iYoungCount,iOldCount,iMinimumPageCount,iNumberOfFreePages,iMaximumPageCount));
#endif
	}


#ifdef _DEBUG
TBool DPager::CheckLists()
	{
#if 0
	__NK_ASSERT_DEBUG(MmuLock::IsHeld());
	SDblQueLink* head = &iOldList.iA;
	TInt n = iOldCount;
	SDblQueLink* link = head;
	while(n--)
		{
		link = link->iNext;
		if(link==head)
			return false;
		}
	link = link->iNext;
	if(link!=head)
		return false;

	head = &iYoungList.iA;
	n = iYoungCount;
	link = head;
	while(n--)
		{
		link = link->iNext;
		if(link==head)
			return false;
		}
	link = link->iNext;
	if(link!=head)
		return false;

//	TRACEP(("DP: y=%d o=%d f=%d",iYoungCount,iOldCount,iNumberOfFreePages));
#endif
//	TraceCounts();
	return true;
	}

void DPager::TraceCounts()
	{
	TRACEP(("DP: y=%d o=%d f=%d min=%d max=%d ml=%d res=%d",
		iYoungCount,iOldCount,iNumberOfFreePages,iMinimumPageCount,
		iMaximumPageCount,iMinimumPageLimit,iReservePageCount));
	}

#endif


TBool DPager::HaveTooManyPages()
	{
	__NK_ASSERT_DEBUG(RamAllocLock::IsHeld());
	__NK_ASSERT_DEBUG(MmuLock::IsHeld());
	return iMinimumPageCount+iNumberOfFreePages > iMaximumPageCount;
	}


TBool DPager::HaveMaximumPages()
	{
	__NK_ASSERT_DEBUG(RamAllocLock::IsHeld());
	__NK_ASSERT_DEBUG(MmuLock::IsHeld());
	return iMinimumPageCount+iNumberOfFreePages >= iMaximumPageCount;
	}


void DPager::AddAsYoungestPage(SPageInfo* aPageInfo)
	{
	__NK_ASSERT_DEBUG(MmuLock::IsHeld());
	__NK_ASSERT_DEBUG(CheckLists());
	__NK_ASSERT_DEBUG(aPageInfo->PagedState()==SPageInfo::EUnpaged);

	aPageInfo->SetPagedState(SPageInfo::EPagedYoung);
	iYoungList.AddHead(&aPageInfo->iLink);
	++iYoungCount;
	}


void DPager::AddAsFreePage(SPageInfo* aPageInfo)
	{
	__NK_ASSERT_DEBUG(MmuLock::IsHeld());
	__NK_ASSERT_DEBUG(CheckLists());

	__NK_ASSERT_DEBUG(aPageInfo->PagedState()==SPageInfo::EUnpaged);
	TheMmu.PageFreed(aPageInfo);
	__NK_ASSERT_DEBUG(aPageInfo->PagedState()==SPageInfo::EUnpaged);

	// add as oldest page...
#ifdef _USE_OLDEST_LISTS
	aPageInfo->SetPagedState(SPageInfo::EPagedOldestClean);
	iOldestCleanList.Add(&aPageInfo->iLink);
	++iOldestCleanCount;
#else
	aPageInfo->SetPagedState(SPageInfo::EPagedOld);
	iOldList.Add(&aPageInfo->iLink);
	++iOldCount;
#endif

	Event(EEventPageInFree,aPageInfo);
	}


TInt DPager::PageFreed(SPageInfo* aPageInfo)
	{
	__NK_ASSERT_DEBUG(MmuLock::IsHeld());
	__NK_ASSERT_DEBUG(CheckLists());

	switch(aPageInfo->PagedState())
		{
	case SPageInfo::EUnpaged:
		return KErrNotFound;

	case SPageInfo::EPagedYoung:
		__NK_ASSERT_DEBUG(iYoungCount);
		aPageInfo->iLink.Deque();
		--iYoungCount;
		break;

	case SPageInfo::EPagedOld:
		__NK_ASSERT_DEBUG(iOldCount);
		aPageInfo->iLink.Deque();
		--iOldCount;
		break;

#ifdef _USE_OLDEST_LISTS
	case SPageInfo::EPagedOldestClean:
		__NK_ASSERT_DEBUG(iOldestCleanCount);
		aPageInfo->iLink.Deque();
		--iOldestCleanCount;
		break;

	case SPageInfo::EPagedOldestDirty:
		__NK_ASSERT_DEBUG(iOldestDirtyCount);
		aPageInfo->iLink.Deque();
		--iOldestDirtyCount;
		break;
#endif

	case SPageInfo::EPagedPinned:
		// this can occur if a pinned mapping is being unmapped when memory is decommitted.
		// the decommit will have succeeded because the the mapping no longer vetoes this,
		// however the unpinning hasn't yet got around to changing the page state.
		// When the state change happens the page will be put back on the live list so
		// we don't have to do anything now...
		return KErrNone;

	case SPageInfo::EPagedPinnedMoved:
		// This page was pinned when it was moved but it has not been returned 
		// to the free pool yet so make sure it is...
		aPageInfo->SetPagedState(SPageInfo::EUnpaged);	// Must be unpaged before returned to free pool.
		return KErrNotFound;

	default:
		__NK_ASSERT_DEBUG(0);
		return KErrNotFound;
		}

	// Update the dirty page count as required...
	if (aPageInfo->IsDirty())
		SetClean(*aPageInfo);

	// add as oldest page...
#ifdef _USE_OLDEST_LISTS
	aPageInfo->SetPagedState(SPageInfo::EPagedOldestClean);
	iOldestCleanList.Add(&aPageInfo->iLink);
	++iOldestCleanCount;
#else
	aPageInfo->SetPagedState(SPageInfo::EPagedOld);
	iOldList.Add(&aPageInfo->iLink);
	++iOldCount;
#endif

	return KErrNone;
	}


extern TBool IsPageTableUnpagedRemoveAllowed(SPageInfo* aPageInfo);

void DPager::RemovePage(SPageInfo* aPageInfo)
	{
	__NK_ASSERT_DEBUG(MmuLock::IsHeld());
	__NK_ASSERT_DEBUG(CheckLists());

	switch(aPageInfo->PagedState())
		{
	case SPageInfo::EPagedYoung:
		__NK_ASSERT_DEBUG(iYoungCount);
		aPageInfo->iLink.Deque();
		--iYoungCount;
		break;

	case SPageInfo::EPagedOld:
		__NK_ASSERT_DEBUG(iOldCount);
		aPageInfo->iLink.Deque();
		--iOldCount;
		break;

#ifdef _USE_OLDEST_LISTS
	case SPageInfo::EPagedOldestClean:
		__NK_ASSERT_DEBUG(iOldestCleanCount);
		aPageInfo->iLink.Deque();
		--iOldestCleanCount;
		break;

	case SPageInfo::EPagedOldestDirty:
		__NK_ASSERT_DEBUG(iOldestDirtyCount);
		aPageInfo->iLink.Deque();
		--iOldestDirtyCount;
		break;
#endif

	case SPageInfo::EPagedPinned:
		__NK_ASSERT_DEBUG(0);
	case SPageInfo::EUnpaged:
#ifdef _DEBUG
		if (!IsPageTableUnpagedRemoveAllowed(aPageInfo))
			__NK_ASSERT_DEBUG(0);
		break;
#endif
	default:
		__NK_ASSERT_DEBUG(0);
		return;
		}

	aPageInfo->SetPagedState(SPageInfo::EUnpaged);
	}


void DPager::ReplacePage(SPageInfo& aOldPageInfo, SPageInfo& aNewPageInfo)
	{
	__NK_ASSERT_DEBUG(MmuLock::IsHeld());
	__NK_ASSERT_DEBUG(CheckLists());

	__NK_ASSERT_DEBUG(aOldPageInfo.PagedState() == aNewPageInfo.PagedState());
	switch(aOldPageInfo.PagedState())
		{
		case SPageInfo::EPagedYoung:
		case SPageInfo::EPagedOld:
		case SPageInfo::EPagedOldestClean:
		case SPageInfo::EPagedOldestDirty:
			{// Update the list links point to the new page.
			__NK_ASSERT_DEBUG(iYoungCount);
			SDblQueLink* prevLink = aOldPageInfo.iLink.iPrev;
#ifdef _DEBUG
			SDblQueLink* nextLink = aOldPageInfo.iLink.iNext;
			__NK_ASSERT_DEBUG(prevLink == aOldPageInfo.iLink.iPrev);
			__NK_ASSERT_DEBUG(prevLink->iNext == &aOldPageInfo.iLink);
			__NK_ASSERT_DEBUG(nextLink == aOldPageInfo.iLink.iNext);
			__NK_ASSERT_DEBUG(nextLink->iPrev == &aOldPageInfo.iLink);
#endif
			aOldPageInfo.iLink.Deque();
			aNewPageInfo.iLink.InsertAfter(prevLink);
			aOldPageInfo.SetPagedState(SPageInfo::EUnpaged);
#ifdef _DEBUG
			__NK_ASSERT_DEBUG(prevLink == aNewPageInfo.iLink.iPrev);
			__NK_ASSERT_DEBUG(prevLink->iNext == &aNewPageInfo.iLink);
			__NK_ASSERT_DEBUG(nextLink == aNewPageInfo.iLink.iNext);
			__NK_ASSERT_DEBUG(nextLink->iPrev == &aNewPageInfo.iLink);
#endif
			}
			break;
		case SPageInfo::EPagedPinned:
			// Mark the page as 'pinned moved' so that when the page moving invokes 
			// Mmu::FreeRam() it returns this page to the free pool.
			aOldPageInfo.ClearPinCount();
			aOldPageInfo.SetPagedState(SPageInfo::EPagedPinnedMoved);
			break;
		case SPageInfo::EPagedPinnedMoved:
			// Shouldn't happen as the ram alloc mutex will be held for the 
			// entire time the page's is paged state == EPagedPinnedMoved.
		case SPageInfo::EUnpaged:
			// Shouldn't happen as we only move pinned memory and unpinning will 
			// atomically add the page to the live list and it can't be removed 
			// from the live list without the ram alloc mutex.
			__NK_ASSERT_DEBUG(0);
			break;
		}	
	}


SPageInfo* DPager::StealOldestPage()
	{
	__NK_ASSERT_DEBUG(RamAllocLock::IsHeld());
	__NK_ASSERT_DEBUG(MmuLock::IsHeld());

	for(;;)
		{
		// find oldest page in list...
		SDblQueLink* link;
#ifdef _USE_OLDEST_LISTS
		if (iOldestCleanCount)
			{
			__NK_ASSERT_DEBUG(!iOldestCleanList.IsEmpty());
			link = iOldestCleanList.Last();
			}
		else if (iOldestDirtyCount)
			{
			__NK_ASSERT_DEBUG(!iOldestDirtyList.IsEmpty());
			link = iOldestDirtyList.Last();
			}
		else if (iOldCount)
#else
		if (iOldCount)
#endif
			{
			__NK_ASSERT_DEBUG(!iOldList.IsEmpty());
			link = iOldList.Last();
			}
		else
			{
			__NK_ASSERT_DEBUG(iYoungCount);
			__NK_ASSERT_ALWAYS(!iYoungList.IsEmpty());
			link = iYoungList.Last();
			}
		SPageInfo* pageInfo = SPageInfo::FromLink(link);

		// steal it from owning object...
		TInt r = StealPage(pageInfo);

		BalanceAges();

		if(r==KErrNone)
			return pageInfo; // done

		// loop back and try again
		}
	}


TInt DPager::RestrictPage(SPageInfo* aPageInfo, TRestrictPagesType aRestriction)
	{
	TRACE(("DPager::RestrictPage(0x%08x,%d)",aPageInfo,aRestriction));
	__NK_ASSERT_DEBUG(MmuLock::IsHeld());

	TInt r;
	if(aPageInfo->Type()==SPageInfo::EUnused)
		{
		// page was unused, so nothing to do...
		r = KErrNone;
		}
	else
		{
		// get memory object which owns the page...
		__NK_ASSERT_DEBUG(aPageInfo->Type()==SPageInfo::EManaged);
		DMemoryObject* memory = aPageInfo->Owner();
		memory->Open();

		// try restricting access to page...
		r = memory->iManager->RestrictPage(memory,aPageInfo,aRestriction);
		__NK_ASSERT_DEBUG(r!=KErrNotSupported);

		// close memory object...
		MmuLock::Unlock();
		memory->AsyncClose();
		MmuLock::Lock();
		}

	TRACE(("DPager::RestrictPage returns %d",r));
	return r;
	}


TInt DPager::StealPage(SPageInfo* aPageInfo)
	{
	TRACE(("DPager::StealPage(0x%08x)",aPageInfo));
	__NK_ASSERT_DEBUG(RamAllocLock::IsHeld());
	__NK_ASSERT_DEBUG(MmuLock::IsHeld());
 
	__UNLOCK_GUARD_START(MmuLock);
	RemovePage(aPageInfo);

	TInt r;
	if(aPageInfo->Type()==SPageInfo::EUnused)
		{
		// page was unused, so nothing to do...
		r = KErrNone;
		__UNLOCK_GUARD_END(MmuLock);
		MmuLock::Unlock();
		}
	else
		{
		// get memory object which owns the page...
		__NK_ASSERT_DEBUG(aPageInfo->Type()==SPageInfo::EManaged);
		DMemoryObject* memory = aPageInfo->Owner();
		memory->Open();

		// try and steal page from memory object...
		__UNLOCK_GUARD_END(MmuLock); // StealPage must be called without releasing the MmuLock
		r = memory->iManager->StealPage(memory,aPageInfo);
		__NK_ASSERT_DEBUG(r!=KErrNotSupported);

		// close memory object...
		MmuLock::Unlock();
		memory->AsyncClose();
		}

	MmuLock::Lock();

	if(r==KErrNone)
		Event(EEventPageOut,aPageInfo);

	TRACE(("DPager::StealPage returns %d",r));
	return r;
	}


TInt DPager::DiscardPage(SPageInfo* aOldPageInfo, TUint aBlockZoneId, TBool aBlockRest)
	{
	__NK_ASSERT_DEBUG(RamAllocLock::IsHeld());
	__NK_ASSERT_DEBUG(MmuLock::IsHeld());

	TInt r;
	// If the page is pinned or if the page is dirty and a general defrag is being 
	// performed then don't attempt to steal it.
	if (aOldPageInfo->Type() != SPageInfo::EUnused && 
		(aOldPageInfo->PagedState() == SPageInfo::EPagedPinned ||
		(aBlockRest && aOldPageInfo->IsDirty())))
		{// The page is pinned or is dirty and this is a general defrag so move the page.
		DMemoryObject* memory = aOldPageInfo->Owner();
		// Page must be managed if it is pinned or dirty.
		__NK_ASSERT_DEBUG(aOldPageInfo->Type()==SPageInfo::EManaged);
		__NK_ASSERT_DEBUG(memory);
		MmuLock::Unlock();
		TPhysAddr newAddr;
		return memory->iManager->MovePage(memory, aOldPageInfo, newAddr, aBlockZoneId, aBlockRest);
		}

	if (!iNumberOfFreePages)
		{
		// Allocate a new page for the live list as it has reached its minimum size.
		MmuLock::Unlock();
		SPageInfo* newPageInfo = GetPageFromSystem((Mmu::TRamAllocFlags)(EMemAttNormalCached|Mmu::EAllocNoWipe),
													aBlockZoneId, aBlockRest);
		 if (!newPageInfo)
			return KErrNoMemory;

		// Re-acquire the mmulock and re-check that the page is not pinned or dirty.
		MmuLock::Lock();
		if (aOldPageInfo->Type() != SPageInfo::EUnused && 
			(aOldPageInfo->PagedState() == SPageInfo::EPagedPinned ||
			(aBlockRest && aOldPageInfo->IsDirty())))
			{// Page is now pinned or dirty so give up as it is inuse.
			ReturnPageToSystem(*newPageInfo);
			MmuLock::Unlock();
			return KErrInUse;
			}

		// Attempt to steal the page
		r = StealPage(aOldPageInfo);
		__NK_ASSERT_DEBUG(MmuLock::IsHeld());

		if (r == KErrCompletion)
			{// This was a page table that has been freed but added to the 
			// live list as a free page.  Remove from live list and continue.
			__NK_ASSERT_DEBUG(!aOldPageInfo->IsDirty());
			RemovePage(aOldPageInfo);
			r = KErrNone;
			}

		if (r == KErrNone)
			{// Add the new page to the live list as discarding the old page 
			// will reduce the live list below the minimum.
			AddAsFreePage(newPageInfo);
			// We've successfully discarded the page so return it to the free pool.
			ReturnPageToSystem(*aOldPageInfo);
			BalanceAges();
			}
		 else
			{
			// New page not required so just return it to the system.  This is safe as 
			// iNumberOfFreePages will have this page counted but as it is not on the live list
			// noone else can touch it.
			ReturnPageToSystem(*newPageInfo);
			}
		}
	else
		{
		// Attempt to steal the page
		r = StealPage(aOldPageInfo);

		__NK_ASSERT_DEBUG(MmuLock::IsHeld());

		if (r == KErrCompletion)
			{// This was a page table that has been freed but added to the 
			// live list as a free page.  Remove from live list.
			__NK_ASSERT_DEBUG(!aOldPageInfo->IsDirty());
			RemovePage(aOldPageInfo);
			r = KErrNone;
			}

		if (r == KErrNone)
			{// We've successfully discarded the page so return it to the free pool.
			ReturnPageToSystem(*aOldPageInfo);
			BalanceAges();
			}
		}
	MmuLock::Unlock();
	return r;	
	}


TBool DPager::TryGrowLiveList()
	{
	__NK_ASSERT_DEBUG(RamAllocLock::IsHeld());
	__NK_ASSERT_DEBUG(MmuLock::IsHeld());

	MmuLock::Unlock();
	SPageInfo* sparePage = GetPageFromSystem((Mmu::TRamAllocFlags)(EMemAttNormalCached|Mmu::EAllocNoWipe));
	MmuLock::Lock();

	if(!sparePage)
		return false;

	// add page to live list...
	AddAsFreePage(sparePage);
	return true;
	}


SPageInfo* DPager::GetPageFromSystem(Mmu::TRamAllocFlags aAllocFlags, TUint aBlockZoneId, TBool aBlockRest)
	{
	__NK_ASSERT_DEBUG(RamAllocLock::IsHeld());

	TPhysAddr pagePhys;
	TInt r = TheMmu.AllocRam(&pagePhys, 1, 
							(Mmu::TRamAllocFlags)(aAllocFlags|Mmu::EAllocNoPagerReclaim), 
							EPageDiscard, aBlockZoneId, aBlockRest);
	if(r!=KErrNone)
		return NULL;

	MmuLock::Lock();
	++iNumberOfFreePages;
	MmuLock::Unlock();

	return SPageInfo::FromPhysAddr(pagePhys);
	}


void DPager::ReturnPageToSystem()
	{
	__NK_ASSERT_DEBUG(RamAllocLock::IsHeld());
	__NK_ASSERT_DEBUG(MmuLock::IsHeld());

	ReturnPageToSystem(*StealOldestPage());
	}


void DPager::ReturnPageToSystem(SPageInfo& aPageInfo)
	{
	__NK_ASSERT_DEBUG(RamAllocLock::IsHeld());
	__NK_ASSERT_DEBUG(MmuLock::IsHeld());

	__NK_ASSERT_DEBUG(iNumberOfFreePages>0);
	--iNumberOfFreePages;

	MmuLock::Unlock();

	TPhysAddr pagePhys = aPageInfo.PhysAddr();
	TheMmu.FreeRam(&pagePhys, 1, EPageDiscard);

	MmuLock::Lock();
	}


SPageInfo* DPager::PageInAllocPage(Mmu::TRamAllocFlags aAllocFlags)
	{
	SPageInfo* pageInfo;
	TPhysAddr pagePhys;

	RamAllocLock::Lock();
	MmuLock::Lock();

	// try getting a free page from our live list...
#ifdef _USE_OLDEST_LISTS
	if (iOldestCleanCount)
		{
		pageInfo = SPageInfo::FromLink(iOldestCleanList.Last());
		if(pageInfo->Type()==SPageInfo::EUnused)
			goto get_oldest;
		}
#else
	if(iOldCount)
		{
		pageInfo = SPageInfo::FromLink(iOldList.Last());
		if(pageInfo->Type()==SPageInfo::EUnused)
			goto get_oldest;
		}
#endif

	// try getting a free page from the system pool...
	if(!HaveMaximumPages())
		{
		MmuLock::Unlock();
		pageInfo = GetPageFromSystem(aAllocFlags);
		if(pageInfo)
			goto done;
		MmuLock::Lock();
		}

	// as a last resort, steal a page from the live list...
get_oldest:
#ifdef _USE_OLDEST_LISTS
	__NK_ASSERT_ALWAYS(iOldestCleanCount|iOldestDirtyCount|iOldCount|iYoungCount);
#else
	__NK_ASSERT_ALWAYS(iOldCount|iYoungCount);
#endif
	pageInfo = StealOldestPage();
	MmuLock::Unlock();

	// make page state same as a freshly allocated page...
	pagePhys = pageInfo->PhysAddr();
	TheMmu.PagesAllocated(&pagePhys,1,aAllocFlags);

done:
	RamAllocLock::Unlock();
	return pageInfo;
	}


TBool DPager::GetFreePages(TInt aNumPages)
	{
	TRACE(("DPager::GetFreePages(%d)",aNumPages));

	__NK_ASSERT_DEBUG(RamAllocLock::IsHeld());

	MmuLock::Lock();
	while(aNumPages>0 && (TInt)NumberOfFreePages()>=aNumPages)
		{
		ReturnPageToSystem();
		--aNumPages;
		}
	MmuLock::Unlock();

	TRACE(("DPager::GetFreePages returns %d",!aNumPages));
	return !aNumPages;
	}


void DPager::DonatePages(TUint aCount, TPhysAddr* aPages)
	{
	TRACE(("DPager::DonatePages(%d,?)",aCount));
	__ASSERT_CRITICAL;
	RamAllocLock::Lock();
	MmuLock::Lock();

	TPhysAddr* end = aPages+aCount;
	while(aPages<end)
		{
		TPhysAddr pagePhys = *aPages++;
		if(RPageArray::State(pagePhys)!=RPageArray::ECommitted)
			continue; // page is not present

#ifdef _DEBUG
		SPageInfo* pi = SPageInfo::SafeFromPhysAddr(pagePhys&~KPageMask);
		__NK_ASSERT_DEBUG(pi);
#else
		SPageInfo* pi = SPageInfo::FromPhysAddr(pagePhys);
#endif
		switch(pi->PagedState())
			{
		case SPageInfo::EUnpaged:
			// Change the type of this page to discardable and 
			// then add it to live list.
			// Only the DDiscardableMemoryManager should be invoking this and
			// its pages will be movable before they are donated.
			__NK_ASSERT_DEBUG(pi->Owner()->iManager->PageType() == EPageMovable);
			TheMmu.ChangePageType(pi, EPageMovable, EPageDiscard);
			break;

		case SPageInfo::EPagedYoung:
		case SPageInfo::EPagedOld:
#ifdef _USE_OLDEST_LISTS
		case SPageInfo::EPagedOldestDirty:
		case SPageInfo::EPagedOldestClean:
#endif
			continue; // discard already been allowed

		case SPageInfo::EPagedPinned:
			__NK_ASSERT_DEBUG(0);
		default:
			__NK_ASSERT_DEBUG(0);
			continue;
			}

		// put page on live list...
		AddAsYoungestPage(pi);
		++iNumberOfFreePages;

		Event(EEventPageDonate,pi);

		// re-balance live list...
		RemoveExcessPages();
		BalanceAges();
		}

	MmuLock::Unlock();
	RamAllocLock::Unlock();
	}


TInt DPager::ReclaimPages(TUint aCount, TPhysAddr* aPages)
	{
	TRACE(("DPager::ReclaimPages(%d,?)",aCount));
	__ASSERT_CRITICAL;
	RamAllocLock::Lock();
	MmuLock::Lock();

	TInt r = KErrNone;
	TPhysAddr* end = aPages+aCount;
	while(aPages<end)
		{
		TPhysAddr pagePhys = *aPages++;
		TBool changeType = EFalse;

		if(RPageArray::State(pagePhys)!=RPageArray::ECommitted)
			{
			r = KErrNotFound; // too late, page has gone
			continue;
			}

#ifdef _DEBUG
		SPageInfo* pi = SPageInfo::SafeFromPhysAddr(pagePhys&~KPageMask);
		__NK_ASSERT_DEBUG(pi);
#else
		SPageInfo* pi = SPageInfo::FromPhysAddr(pagePhys);
#endif
		switch(pi->PagedState())
			{
		case SPageInfo::EUnpaged:
			continue; // discard already been disallowed

		case SPageInfo::EPagedYoung:
		case SPageInfo::EPagedOld:
#ifdef _USE_OLDEST_LISTS
		case SPageInfo::EPagedOldestClean:
		case SPageInfo::EPagedOldestDirty:
#endif
			changeType = ETrue;
			break; // remove from live list

		case SPageInfo::EPagedPinned:
			__NK_ASSERT_DEBUG(0);
		default:
			__NK_ASSERT_DEBUG(0);
			break;
			}

		// check paging list has enough pages before we remove one...
		if(iNumberOfFreePages<1)
			{
			// need more pages so get a page from the system...
			if(!TryGrowLiveList())
				{
				// out of memory...
				r = KErrNoMemory;
				break;
				}
			// retry the page reclaim...
			--aPages;
			continue;
			}

		if (changeType)
			{// Change the type of this page to movable, wait until any retries
			// have been attempted as we can't change a page's type twice.
			// Only the DDiscardableMemoryManager should be invoking this and
			// its pages should be movable once they are reclaimed.
			__NK_ASSERT_DEBUG(pi->Owner()->iManager->PageType() == EPageMovable);
			TheMmu.ChangePageType(pi, EPageDiscard, EPageMovable);
			}

		// remove page from paging list...
		__NK_ASSERT_DEBUG(iNumberOfFreePages>0);
		--iNumberOfFreePages;
		RemovePage(pi);

		Event(EEventPageReclaim,pi);

		// re-balance live list...
		BalanceAges();
		}

	// we may have added a spare free page to the live list without removing one,
	// this could cause us to have too many pages, so deal with this...
	RemoveExcessPages();

	MmuLock::Unlock();
	RamAllocLock::Unlock();
	return r;
	}


TInt VMHalFunction(TAny*, TInt aFunction, TAny* a1, TAny* a2);

void DPager::Init3()
	{
	TRACEB(("DPager::Init3()"));
	TheRomMemoryManager->Init3();
	TheDataPagedMemoryManager->Init3();
	TheCodePagedMemoryManager->Init3();
	TInt r = Kern::AddHalEntry(EHalGroupVM, VMHalFunction, 0);
	__NK_ASSERT_ALWAYS(r==KErrNone);
	}


void DPager::Fault(TFault aFault)
	{
	Kern::Fault("DPager",aFault);
	}


void DPager::BalanceAges()
	{
	__NK_ASSERT_DEBUG(MmuLock::IsHeld());
	TBool restrictPage = EFalse;
	SPageInfo* pageInfo = NULL;
#ifdef _USE_OLDEST_LISTS
	TUint oldestCount = iOldestCleanCount + iOldestDirtyCount;
	if((iOldCount + oldestCount) * iYoungOldRatio < iYoungCount)
#else
	if (iOldCount * iYoungOldRatio < iYoungCount)
#endif
		{
		// Need more old pages so make one young page into an old page...
		__NK_ASSERT_DEBUG(!iYoungList.IsEmpty());
		__NK_ASSERT_DEBUG(iYoungCount);
		SDblQueLink* link = iYoungList.Last()->Deque();
		--iYoungCount;

		pageInfo = SPageInfo::FromLink(link);
		pageInfo->SetPagedState(SPageInfo::EPagedOld);

		iOldList.AddHead(link);
		++iOldCount;

		Event(EEventPageAged,pageInfo);
		// Delay restricting the page until it is safe to release the MmuLock.
		restrictPage = ETrue;
		}

#ifdef _USE_OLDEST_LISTS
	// Check we have enough oldest pages.
	if (oldestCount * iOldOldestRatio < iOldCount)
		{
		__NK_ASSERT_DEBUG(!iOldList.IsEmpty());
		__NK_ASSERT_DEBUG(iOldCount);
		SDblQueLink* link = iOldList.Last()->Deque();
		--iOldCount;

		SPageInfo* oldestPageInfo = SPageInfo::FromLink(link);
		if (oldestPageInfo->IsDirty())
			{
			oldestPageInfo->SetPagedState(SPageInfo::EPagedOldestDirty);
			iOldestDirtyList.AddHead(link);
			++iOldestDirtyCount;
			Event(EEventPageAgedDirty,oldestPageInfo);
			}
		else
			{
			oldestPageInfo->SetPagedState(SPageInfo::EPagedOldestClean);
			iOldestCleanList.AddHead(link);
			++iOldestCleanCount;
			Event(EEventPageAgedClean,oldestPageInfo);
			}
		}
#endif
	if (restrictPage)
		{
		// Make the recently aged old page inaccessible.  This is done last as it 
		// will release the MmuLock and therefore the page counts may otherwise change.
		RestrictPage(pageInfo,ERestrictPagesNoAccessForOldPage);
		}
	}


void DPager::RemoveExcessPages()
	{
	__NK_ASSERT_DEBUG(RamAllocLock::IsHeld());
	__NK_ASSERT_DEBUG(MmuLock::IsHeld());
	while(HaveTooManyPages())
		ReturnPageToSystem();
	}


void DPager::RejuvenatePageTable(TPte* aPt)
	{
	SPageInfo* pi = SPageInfo::FromPhysAddr(Mmu::PageTablePhysAddr(aPt));

	SPageTableInfo* pti = SPageTableInfo::FromPtPtr(aPt);
	if(!pti->IsDemandPaged())
		{
		__NK_ASSERT_DEBUG(pi->PagedState()==SPageInfo::EUnpaged);
		return;
		}

	TRACE2(("DP: %O Rejuvenate PT 0x%08x 0x%08x",TheCurrentThread,pi->PhysAddr(),aPt));
	switch(pi->PagedState())
		{
	case SPageInfo::EPagedYoung:
	case SPageInfo::EPagedOld:
#ifdef _USE_OLDEST_LISTS
	case SPageInfo::EPagedOldestClean:
	case SPageInfo::EPagedOldestDirty:
#endif
		RemovePage(pi);
		AddAsYoungestPage(pi);
		BalanceAges();
		break;

	case SPageInfo::EUnpaged:
		AddAsYoungestPage(pi);
		BalanceAges();
		break;

	case SPageInfo::EPagedPinned:
		break;

	default:
		__NK_ASSERT_DEBUG(0);
		break;
		}
	}

TInt DPager::PteAndInfoFromLinAddr(	TInt aOsAsid, TLinAddr aAddress, DMemoryMappingBase* aMapping, 
									TUint aMapInstanceCount, TPte*& aPte, SPageInfo*& aPageInfo)
	{
	__NK_ASSERT_DEBUG(MmuLock::IsHeld());	

	// Verify the mapping is still mapped and has not been reused.
	if (aMapInstanceCount != aMapping->MapInstanceCount() || aMapping->BeingDetached())
		return KErrAbort;

	aPte = Mmu::SafePtePtrFromLinAddr(aAddress,aOsAsid);
	if(!aPte)
		return KErrNotFound;

	TPte pte = *aPte;
	if(pte==KPteUnallocatedEntry)
		return KErrNotFound;

	SPageInfo* pi = SPageInfo::SafeFromPhysAddr(pte & ~KPageMask);
	if(!pi)
		return KErrNotFound;
	aPageInfo = pi;

	return KErrNone;
	}

TInt DPager::TryRejuvenate(	TInt aOsAsid, TLinAddr aAddress, TUint aAccessPermissions, TLinAddr aPc,
							DMemoryMappingBase* aMapping, TUint aMapInstanceCount, DThread* aThread, 
							TAny* aExceptionInfo)
	{
	__NK_ASSERT_DEBUG(MmuLock::IsHeld());

	SPageInfo* pi;
	TPte* pPte;
	TPte pte;
	TInt r = PteAndInfoFromLinAddr(aOsAsid, aAddress, aMapping, aMapInstanceCount, pPte, pi);
	if (r != KErrNone)
		{
		if (aThread->IsRealtime())
			{// This thread is real time so it shouldn't be accessing paged out paged memory
			// unless there is a paging trap.
			MmuLock::Unlock();
			// Ensure that we abort when the thread is not allowed to access paged out pages.
			if (CheckRealtimeThreadFault(aThread, aExceptionInfo) != KErrNone)
				r = KErrAbort;
			MmuLock::Lock();
			}
		return r;
		}
	pte = *pPte;
	SPageInfo::TType type = pi->Type();
	SPageInfo::TPagedState state = pi->PagedState();

	if (aThread->IsRealtime() && 
		state != SPageInfo::EPagedPinned && 
		state != SPageInfo::EPagedPinnedMoved)
		{// This thread is real time so it shouldn't be accessing unpinned paged memory
		// unless there is a paging trap.
		MmuLock::Unlock();
		r = CheckRealtimeThreadFault(aThread, aExceptionInfo);
		MmuLock::Lock();
		if (r != KErrNone)
			return r;
		// We had to release the MmuLock have to reverify the status of the page and mappings.
		r = PteAndInfoFromLinAddr(aOsAsid, aAddress, aMapping, aMapInstanceCount, pPte, pi);
		if (r != KErrNone)
			return r;
		pte = *pPte;
		type = pi->Type();
		state = pi->PagedState();
		}

	if (type != SPageInfo::EManaged)
		return KErrNotFound;

	if(state==SPageInfo::EUnpaged)
		return KErrNotFound;

	DMemoryObject* memory = pi->Owner();
	TUint index = pi->Index();

	TPhysAddr page = memory->iPages.Page(index);
	if(!RPageArray::IsPresent(page))
		return KErrNotFound;

	TPhysAddr physAddr = pi->PhysAddr();
	if ((page^physAddr) >= (TPhysAddr)KPageSize)
		{// Page array entry should contain same physical address as PTE unless the 
		// page has or is being moved and this mapping accessed the page.
		// Get the page info for the page that we should be using.
		physAddr = page & ~KPageMask;
		pi = SPageInfo::SafeFromPhysAddr(physAddr);
		if(!pi)
			return KErrNotFound;

		type = pi->Type();
		if (type!=SPageInfo::EManaged)
			return KErrNotFound;

		state = pi->PagedState();
		if(state==SPageInfo::EUnpaged)
			return KErrNotFound;

		memory = pi->Owner();
		index = pi->Index();

		// Update pte to point to the correct physical address for this memory object's page.
		pte = (pte & KPageMask) | physAddr;
		}

	if(aAccessPermissions&EReadWrite)
		{// The mapping that took the fault permits writes and is still attached 
		// to the memory object therefore the object can't be read only.
		__NK_ASSERT_DEBUG(!memory->IsReadOnly());
		SetWritable(*pi);
		}

	pte = Mmu::MakePteAccessible(pte,aAccessPermissions&EReadWrite);
	TRACE2(("!PTE %x=%x",pPte,pte));
	*pPte = pte;
	CacheMaintenance::SinglePteUpdated((TLinAddr)pPte);
	InvalidateTLBForPage((aAddress&~KPageMask)|aOsAsid);

	Event(EEventPageRejuvenate,pi,aPc,aAddress,aAccessPermissions);

	TBool balance = false;
#ifdef _USE_OLDEST_LISTS
	if(	state==SPageInfo::EPagedYoung || state==SPageInfo::EPagedOld || 
		state==SPageInfo::EPagedOldestClean || state==SPageInfo::EPagedOldestDirty)
#else
	if(state==SPageInfo::EPagedYoung || state==SPageInfo::EPagedOld)
#endif
		{
		RemovePage(pi);
		AddAsYoungestPage(pi);
		// delay BalanceAges because we don't want to release MmuLock until after
		// RejuvenatePageTable has chance to look at the page table page...
		balance = true;
		}
	else
		{// Clear the modifier so that if this page is being moved then this 
		// access is detected. For non-pinned pages the modifier is cleared 
		// by RemovePage().
		__NK_ASSERT_DEBUG(state==SPageInfo::EPagedPinned);
		pi->SetModifier(0);
		}

	RejuvenatePageTable(pPte);

	if(balance)
		BalanceAges();

	return KErrNone;
	}


TInt DPager::PageInAllocPages(TPhysAddr* aPages, TUint aCount, Mmu::TRamAllocFlags aAllocFlags)
	{
	TUint n = 0;
	while(n<aCount)
		{
		SPageInfo* pi = PageInAllocPage(aAllocFlags);
		if(!pi)
			goto fail;
		aPages[n++] = pi->PhysAddr();
		}
	return KErrNone;
fail:
	PageInFreePages(aPages,n);
	return KErrNoMemory;
	}


void DPager::PageInFreePages(TPhysAddr* aPages, TUint aCount)
	{
	while(aCount--)
		{
		MmuLock::Lock();
		SPageInfo* pi = SPageInfo::FromPhysAddr(aPages[aCount]);
		switch(pi->PagedState())
			{
		case SPageInfo::EPagedYoung:
		case SPageInfo::EPagedOld:
#ifdef _USE_OLDEST_LISTS
		case SPageInfo::EPagedOldestClean:
		case SPageInfo::EPagedOldestDirty:
#endif
			RemovePage(pi);
			// fall through...
		case SPageInfo::EUnpaged:
			AddAsFreePage(pi);
			break;

		case SPageInfo::EPagedPinned:
			__NK_ASSERT_DEBUG(0);
			break;
		default:
			__NK_ASSERT_DEBUG(0);
			break;
			}
		MmuLock::Unlock();
		}
	}


void DPager::PagedInUnneeded(SPageInfo* aPageInfo)
	{
	__NK_ASSERT_DEBUG(MmuLock::IsHeld());
	Event(EEventPageInUnneeded,aPageInfo);
	AddAsFreePage(aPageInfo);
	}


void DPager::PagedIn(SPageInfo* aPageInfo)
	{
	__NK_ASSERT_DEBUG(MmuLock::IsHeld());
	switch(aPageInfo->PagedState())
		{
	case SPageInfo::EPagedYoung:
	case SPageInfo::EPagedOld:
#ifdef _USE_OLDEST_LISTS
	case SPageInfo::EPagedOldestClean:
	case SPageInfo::EPagedOldestDirty:
#endif
		RemovePage(aPageInfo);
		AddAsYoungestPage(aPageInfo);
		BalanceAges();
		break;

	case SPageInfo::EUnpaged:
		AddAsYoungestPage(aPageInfo);
		BalanceAges();
		break;

	case SPageInfo::EPagedPinned:
		// Clear the modifier so that if this page is being moved then this 
		// access is detected. For non-pinned pages the modifier is cleared by RemovePage().
		aPageInfo->SetModifier(0);
		break;

	default:
		__NK_ASSERT_DEBUG(0);
		break;
		}
	}


void DPager::PagedInPinned(SPageInfo* aPageInfo, TPinArgs& aPinArgs)
	{
	__NK_ASSERT_DEBUG(MmuLock::IsHeld());
	Pin(aPageInfo,aPinArgs);
	}


void DPager::Pin(SPageInfo* aPageInfo, TPinArgs& aPinArgs)
	{
	__ASSERT_CRITICAL;
	__NK_ASSERT_DEBUG(MmuLock::IsHeld());
	__NK_ASSERT_DEBUG(aPinArgs.HaveSufficientPages(1));

	aPageInfo->IncPinCount();
	Event(EEventPagePin,aPageInfo);

	// remove page from live list...
	switch(aPageInfo->PagedState())
		{
	case SPageInfo::EPagedYoung:
		__NK_ASSERT_DEBUG(iYoungCount);
		aPageInfo->iLink.Deque();
		--iYoungCount;
		__NK_ASSERT_DEBUG(aPageInfo->PinCount()==1);
		break;

	case SPageInfo::EPagedOld:
		__NK_ASSERT_DEBUG(iOldCount);
		aPageInfo->iLink.Deque();
		--iOldCount;
		__NK_ASSERT_DEBUG(aPageInfo->PinCount()==1);
		break;

#ifdef _USE_OLDEST_LISTS
	case SPageInfo::EPagedOldestClean:
		__NK_ASSERT_DEBUG(iOldestCleanCount);
		aPageInfo->iLink.Deque();
		--iOldestCleanCount;
		__NK_ASSERT_DEBUG(aPageInfo->PinCount()==1);
		break;

	case SPageInfo::EPagedOldestDirty:
		__NK_ASSERT_DEBUG(iOldestDirtyCount);
		aPageInfo->iLink.Deque();
		--iOldestDirtyCount;
		__NK_ASSERT_DEBUG(aPageInfo->PinCount()==1);
		break;
#endif

	case SPageInfo::EPagedPinned:
		// nothing more to do...
		__NK_ASSERT_DEBUG(aPageInfo->PinCount()>1);
		return;

	case SPageInfo::EUnpaged:
		__NK_ASSERT_DEBUG(aPageInfo->PinCount()==1);
		TRACE2(("DPager::PinPage page was unpaged"));
		// This could be a page in the process of being stolen.
		// Could also be page for storing page table infos, which aren't necessarily
		// on the live list.
		break;

	default:
		__NK_ASSERT_DEBUG(0);
		return;
		}

	// page has now been removed from the live list and is pinned...
	aPageInfo->SetPagedState(SPageInfo::EPagedPinned);

	if(aPinArgs.iReplacementPages==TPinArgs::EUseReserveForPinReplacementPages)
		{
		// pinned paged counts as coming from reserve pool...
		aPageInfo->SetPinnedReserve();
		}
	else
		{
		// we used up a replacement page...
		--aPinArgs.iReplacementPages;
		}

	BalanceAges();
	}


void DPager::Unpin(SPageInfo* aPageInfo, TPinArgs& aPinArgs)
	{
	__ASSERT_CRITICAL;
	__NK_ASSERT_DEBUG(MmuLock::IsHeld());
	__NK_ASSERT_DEBUG(aPageInfo->PagedState()==SPageInfo::EPagedPinned);
	__NK_ASSERT_DEBUG(aPageInfo->PinCount()>0);

	TUint pinCount = aPageInfo->DecPinCount();
	Event(EEventPageUnpin,aPageInfo);

	if(pinCount)
		return;

	aPageInfo->SetPagedState(SPageInfo::EUnpaged);

	if(!aPageInfo->ClearPinnedReserve())
		{
		// was not a pinned reserve page, so we how have a spare replacement page,
		// which can be used again or freed later ...
		__NK_ASSERT_DEBUG(aPinArgs.iReplacementPages!=TPinArgs::EUseReserveForPinReplacementPages);
		++aPinArgs.iReplacementPages;
		}

	AddAsYoungestPage(aPageInfo);
	BalanceAges();
	}


TInt TPinArgs::AllocReplacementPages(TUint aNumPages)
	{
	if(iUseReserve)
		{
		__NK_ASSERT_DEBUG(iReplacementPages==0 || iReplacementPages==EUseReserveForPinReplacementPages);
		iReplacementPages = EUseReserveForPinReplacementPages;
		}
	else
		{
		if(aNumPages>iReplacementPages)
			{
			if(!ThePager.AllocPinReplacementPages(aNumPages-iReplacementPages))
				return KErrNoMemory;
			iReplacementPages = aNumPages;
			}
		}
	return KErrNone;
	}


void TPinArgs::FreeReplacementPages()
	{
	if(iReplacementPages!=0 && iReplacementPages!=EUseReserveForPinReplacementPages)
		ThePager.FreePinReplacementPages(iReplacementPages);
	iReplacementPages = 0;
	}


TBool DPager::AllocPinReplacementPages(TUint aNumPages)
	{
	TRACE2(("DPager::AllocPinReplacementPages(0x%x)",aNumPages));
	__ASSERT_CRITICAL;
	RamAllocLock::Lock();
	MmuLock::Lock();

	TBool ok = false;
	do
		{
		if(iNumberOfFreePages>=aNumPages)
			{
			iNumberOfFreePages -= aNumPages;
			ok = true;
			break;
			}
		}
	while(TryGrowLiveList());

	MmuLock::Unlock();
	RamAllocLock::Unlock();
	return ok;
	}


void DPager::FreePinReplacementPages(TUint aNumPages)
	{
	TRACE2(("DPager::FreePinReplacementPage(0x%x)",aNumPages));
	__ASSERT_CRITICAL;

	RamAllocLock::Lock();
	MmuLock::Lock();

	iNumberOfFreePages += aNumPages;
	RemoveExcessPages();

	MmuLock::Unlock();
	RamAllocLock::Unlock();
	}


TBool DPager::ReservePage()
	{
	__NK_ASSERT_DEBUG(RamAllocLock::IsHeld());
	__NK_ASSERT_DEBUG(MmuLock::IsHeld());
	__ASSERT_CRITICAL;
	__NK_ASSERT_DEBUG(iMinimumPageCount >= iMinimumPageLimit+iReservePageCount);
	while(iMinimumPageCount==iMinimumPageLimit+iReservePageCount && iNumberOfFreePages==0)
		{
		if(!TryGrowLiveList())
			return false;
		}
	if(iMinimumPageCount==iMinimumPageLimit+iReservePageCount)
		{
		++iMinimumPageCount;
		--iNumberOfFreePages;
		if(iMinimumPageCount>iMaximumPageCount)
			iMaximumPageCount = iMinimumPageCount;
		}
	++iReservePageCount;
	__NK_ASSERT_DEBUG(iMinimumPageCount >= iMinimumPageLimit+iReservePageCount);
	__NK_ASSERT_DEBUG(iMinimumPageCount+iNumberOfFreePages <= iMaximumPageCount);
	return ETrue;
	}


TBool DPager::ReservePages(TUint aRequiredCount, TUint& aCount)
	{
	__ASSERT_CRITICAL;

	RamAllocLock::Lock();
	MmuLock::Lock();
	while(aCount<aRequiredCount)
		{
		if(!ReservePage())
			break;
		++aCount;
		MmuLock::Flash();
		}
	TBool enoughPages = aCount==aRequiredCount;
	MmuLock::Unlock();
	RamAllocLock::Unlock();

	if(!enoughPages)
		UnreservePages(aCount);

	return enoughPages;
	}


void DPager::UnreservePages(TUint& aCount)
	{
	MmuLock::Lock();
	iReservePageCount -= aCount;
	aCount = 0;
	MmuLock::Unlock();
	}


TInt DPager::CheckRealtimeThreadFault(DThread* aThread, TAny* aExceptionInfo)
	{
	// realtime threads shouldn't take paging faults...
	DThread* client = aThread->iIpcClient;

	// If iIpcClient is set then we are accessing the address space of a remote thread.  If we are
	// in an IPC trap, this will contain information the local and remote addresses being accessed.
	// If this is not set then we assume than any fault must be the fault of a bad remote address.
	TIpcExcTrap* ipcTrap = (TIpcExcTrap*)aThread->iExcTrap;
	if (ipcTrap && !ipcTrap->IsTIpcExcTrap())
		ipcTrap = 0;
	if (client && (!ipcTrap || ipcTrap->ExcLocation(aThread, aExceptionInfo) == TIpcExcTrap::EExcRemote))
		{
		// kill client thread...
		if(K::IllegalFunctionForRealtimeThread(client,"Access to Paged Memory (by other thread)"))
			{
			// treat memory access as bad...
			return KErrAbort;
			}
		// else thread is in 'warning only' state so allow paging...
		}
	else
		{
		// kill current thread...
		if(K::IllegalFunctionForRealtimeThread(NULL,"Access to Paged Memory"))
			{
			// if current thread is in critical section, then the above kill will be deferred
			// and we will continue executing. We will handle this by returning an error
			// which means that the thread will take an exception (which hopefully is XTRAPed!)
			return KErrAbort;
			}
		// else thread is in 'warning only' state so allow paging...
		}
	return KErrNone;
	}


TInt DPager::HandlePageFault(	TLinAddr aPc, TLinAddr aFaultAddress, TUint aFaultAsid, TUint aFaultIndex,
								TUint aAccessPermissions, DMemoryObject* aMemory, DMemoryMapping* aMapping,
								TUint aMapInstanceCount, DThread* aThread, TAny* aExceptionInfo)
	{
	MmuLock::Lock();
	TInt r = TryRejuvenate(	aFaultAsid, aFaultAddress, aAccessPermissions, aPc, aMapping, aMapInstanceCount,
							aThread, aExceptionInfo);
	if(r == KErrNone || r == KErrAbort)
		{
		MmuLock::Unlock();
		}
	else
		{
		// rejuvenate failed, call memory manager to page in memory...
		Event(EEventPageInStart, 0, aPc, aFaultAddress, aAccessPermissions);
		MmuLock::Unlock();
		TheThrashMonitor.NotifyStartPaging();

		DMemoryManager* manager = aMemory->iManager;
		r = manager->HandleFault(aMemory, aFaultIndex, aMapping, aMapInstanceCount, aAccessPermissions);

		TheThrashMonitor.NotifyEndPaging();
		}
	return r;
	}


TInt DPager::ResizeLiveList()
	{
	MmuLock::Lock();
	TUint min = iMinimumPageCount;
	TUint max = iMaximumPageCount;
	MmuLock::Unlock();
	return ResizeLiveList(min,max);
	}


TInt DPager::ResizeLiveList(TUint aMinimumPageCount, TUint aMaximumPageCount)
	{
	TRACE(("DPager::ResizeLiveList(%d,%d) current young=%d old=%d min=%d free=%d max=%d",aMinimumPageCount,aMaximumPageCount,iYoungCount,iOldCount,iMinimumPageCount,iNumberOfFreePages,iMaximumPageCount));
	if(!aMaximumPageCount)
		{
		aMinimumPageCount = iInitMinimumPageCount;
		aMaximumPageCount = iInitMaximumPageCount;
		}
	if (aMaximumPageCount > KAbsoluteMaxPageCount)
		aMaximumPageCount = KAbsoluteMaxPageCount;

	// Min must not be greater than max...
	if(aMinimumPageCount>aMaximumPageCount)
		return KErrArgument;

	NKern::ThreadEnterCS();
	RamAllocLock::Lock();

	MmuLock::Lock();

	// Make sure aMinimumPageCount is not less than absolute minimum we can cope with...
	iMinimumPageLimit = iMinYoungPages * (1 + iYoungOldRatio) / iYoungOldRatio
						+ DPageReadRequest::ReservedPagesRequired();
	if(iMinimumPageLimit<iAbsoluteMinPageCount)
		iMinimumPageLimit = iAbsoluteMinPageCount;
	if(aMinimumPageCount<iMinimumPageLimit+iReservePageCount)
		aMinimumPageCount = iMinimumPageLimit+iReservePageCount;
	if(aMaximumPageCount<aMinimumPageCount)
		aMaximumPageCount=aMinimumPageCount;

	// Increase iMaximumPageCount?
	TInt extra = aMaximumPageCount-iMaximumPageCount;
	if(extra>0)
		iMaximumPageCount += extra;

	// Reduce iMinimumPageCount?
	TInt spare = iMinimumPageCount-aMinimumPageCount;
	if(spare>0)
		{
		iMinimumPageCount -= spare;
		iNumberOfFreePages += spare;
		}

	// Increase iMinimumPageCount?
	TInt r=KErrNone;
	while(iMinimumPageCount<aMinimumPageCount)
		{
		TUint newMin = aMinimumPageCount;
		TUint maxMin = iMinimumPageCount+iNumberOfFreePages;
		if(newMin>maxMin)
			newMin = maxMin;

		TUint delta = newMin-iMinimumPageCount;
		if(delta)
			{
			iMinimumPageCount = newMin;
			iNumberOfFreePages -= delta;
			continue;
			}

		if(!TryGrowLiveList())
			{
			r=KErrNoMemory;
			break;
			}
		}

	// Reduce iMaximumPageCount?
	while(iMaximumPageCount>aMaximumPageCount)
		{
		TUint newMax = aMaximumPageCount;
		TUint minMax = iMinimumPageCount+iNumberOfFreePages;
		if(newMax<minMax)
			newMax = minMax;

		TUint delta = iMaximumPageCount-newMax;
		if(delta)
			{
			iMaximumPageCount = newMax;
			continue;
			}

		ReturnPageToSystem();
		}

	TRACE(("DPager::ResizeLiveList end with young=%d old=%d min=%d free=%d max=%d",iYoungCount,iOldCount,iMinimumPageCount,iNumberOfFreePages,iMaximumPageCount));

#ifdef BTRACE_KERNEL_MEMORY
	BTrace4(BTrace::EKernelMemory,BTrace::EKernelMemoryDemandPagingCache,iMinimumPageCount << KPageShift);
#endif

	MmuLock::Unlock();

	RamAllocLock::Unlock();
	NKern::ThreadLeaveCS();

	return r;
	}


void DPager::FlushAll()
	{
	NKern::ThreadEnterCS();
	RamAllocLock::Lock();

	TRACE(("DPager::FlushAll() live list young=%d old=%d min=%d free=%d max=%d",iYoungCount,iOldCount,iMinimumPageCount,iNumberOfFreePages,iMaximumPageCount));

	// look at all RAM pages in the system, and unmap all those used for paging
	const TUint32* piMap = (TUint32*)KPageInfoMap;
	const TUint32* piMapEnd = piMap+(KNumPageInfoPages>>5);
	SPageInfo* pi = (SPageInfo*)KPageInfoLinearBase;
	MmuLock::Lock();
	do
		{
		SPageInfo* piNext = pi+(KPageInfosPerPage<<5);
		for(TUint32 piFlags=*piMap++; piFlags; piFlags>>=1)
			{
			if(!(piFlags&1))
				{
				pi += KPageInfosPerPage;
				continue;
				}
			SPageInfo* piEnd = pi+KPageInfosPerPage;
			do
				{
				SPageInfo::TPagedState state = pi->PagedState();
#ifdef _USE_OLDEST_LISTS
				if (state==SPageInfo::EPagedYoung || state==SPageInfo::EPagedOld ||
					state==SPageInfo::EPagedOldestClean || state==SPageInfo::EPagedOldestDirty)
#else
				if(state==SPageInfo::EPagedYoung || state==SPageInfo::EPagedOld)
#endif
					{
					if (pi->Type() != SPageInfo::EUnused)
						{
						TInt r = StealPage(pi);
						if(r==KErrNone)
							AddAsFreePage(pi);
						MmuLock::Flash();
						}
					}
				++pi;
				if(((TUint)pi&(0xf<<KPageInfoShift))==0)
					MmuLock::Flash(); // every 16 page infos
				}
			while(pi<piEnd);
			}
		pi = piNext;
		}
	while(piMap<piMapEnd);
	MmuLock::Unlock();

	// reduce live page list to a minimum
	while(GetFreePages(1)) {}; 

	TRACE(("DPager::FlushAll() end with young=%d old=%d min=%d free=%d max=%d",iYoungCount,iOldCount,iMinimumPageCount,iNumberOfFreePages,iMaximumPageCount));

	RamAllocLock::Unlock();
	NKern::ThreadLeaveCS();
	}


void DPager::GetLiveListInfo(SVMCacheInfo& aInfo)
	{
	MmuLock::Lock(); // ensure consistent set of values are read...
	aInfo.iMinSize = iMinimumPageCount<<KPageShift;
	aInfo.iMaxSize = iMaximumPageCount<<KPageShift;
	aInfo.iCurrentSize = (iMinimumPageCount+iNumberOfFreePages)<<KPageShift;
	aInfo.iMaxFreeSize = iNumberOfFreePages<<KPageShift;
	MmuLock::Unlock();
	}


void DPager::GetEventInfo(SVMEventInfo& aInfoOut)
	{
	MmuLock::Lock(); // ensure consistent set of values are read...
	aInfoOut = iEventInfo;
	MmuLock::Unlock();
	}


void DPager::ResetEventInfo()
	{
	MmuLock::Lock();
	memclr(&iEventInfo, sizeof(iEventInfo));
	MmuLock::Unlock();
	}


TInt TestPageState(TLinAddr aAddr)
	{
	DMemModelProcess* process = (DMemModelProcess*)TheCurrentThread->iOwningProcess;
	// Get the os asid of current thread's process so no need to open a reference on it.
	TInt osAsid = process->OsAsid();
	TPte* ptePtr = 0;
	TPte pte = 0;
	TInt r = 0;
	SPageInfo* pageInfo = NULL;

	NKern::ThreadEnterCS();

	TUint offsetInMapping;
	TUint mapInstanceCount;
	DMemoryMapping* mapping = MM::FindMappingInAddressSpace(osAsid, aAddr, 1, offsetInMapping, mapInstanceCount);

	MmuLock::Lock();

	if(mapping)
		{
		DMemoryObject* memory = mapping->Memory();
		if(mapInstanceCount == mapping->MapInstanceCount() && memory)
			{
			DMemoryManager* manager = memory->iManager;
			if(manager==TheCodePagedMemoryManager)
				r |= EPageStateInRamCode|EPageStatePaged;
			}
		}

	ptePtr = Mmu::SafePtePtrFromLinAddr(aAddr,osAsid);
	if (!ptePtr)
		goto done;
	pte = *ptePtr;
	if (pte == KPteUnallocatedEntry)
		goto done;		
	r |= EPageStatePtePresent;
	if (pte!=Mmu::MakePteInaccessible(pte,0))
		r |= EPageStatePteValid;
	
	pageInfo = SPageInfo::SafeFromPhysAddr(pte&~KPageMask);
	if(pageInfo)
		{
		r |= pageInfo->Type();
		r |= pageInfo->PagedState()<<8;
		}
done:
	MmuLock::Unlock();
	if(mapping)
		mapping->Close();
	NKern::ThreadLeaveCS();
	return r;
	}



TInt VMHalFunction(TAny*, TInt aFunction, TAny* a1, TAny* a2)
	{
	switch(aFunction)
		{
	case EVMHalFlushCache:
		if(!TheCurrentThread->HasCapability(ECapabilityWriteDeviceData,__PLATSEC_DIAGNOSTIC_STRING("Checked by VMHalFunction(EVMHalFlushCache)")))
			K::UnlockedPlatformSecurityPanic();
		ThePager.FlushAll();
		return KErrNone;

	case EVMHalSetCacheSize:
		{
		if(!TheCurrentThread->HasCapability(ECapabilityWriteDeviceData,__PLATSEC_DIAGNOSTIC_STRING("Checked by VMHalFunction(EVMHalSetCacheSize)")))
			K::UnlockedPlatformSecurityPanic();
		TUint min = TUint(a1)>>KPageShift;
		if(TUint(a1)&KPageMask)
			++min;
		TUint max = TUint(a2)>>KPageShift;
		if(TUint(a2)&KPageMask)
			++max;
		return ThePager.ResizeLiveList(min,max);
		}

	case EVMHalGetCacheSize:
		{
		SVMCacheInfo info;
		ThePager.GetLiveListInfo(info);
		kumemput32(a1,&info,sizeof(info));
		}
		return KErrNone;

	case EVMHalGetEventInfo:
		{
		SVMEventInfo info;
		ThePager.GetEventInfo(info);
		Kern::InfoCopy(*(TDes8*)a1,(TUint8*)&info,sizeof(info));
		}
		return KErrNone;

	case EVMHalResetEventInfo:
		ThePager.ResetEventInfo();
		return KErrNone;

#ifdef __SUPPORT_DEMAND_PAGING_EMULATION__
	case EVMHalGetOriginalRomPages:
		RomOriginalPages(*((TPhysAddr**)a1), *((TUint*)a2));
		return KErrNone;
#endif

	case EVMPageState:
		return TestPageState((TLinAddr)a1);

	case EVMHalGetSwapInfo:
		{
		if ((K::MemModelAttributes & EMemModelAttrDataPaging) == 0)
			return KErrNotSupported;
		SVMSwapInfo info;
		GetSwapInfo(info);
		kumemput32(a1,&info,sizeof(info));
		}
		return KErrNone;

	case EVMHalGetThrashLevel:
		return TheThrashMonitor.ThrashLevel();

	case EVMHalSetSwapThresholds:
		{
		if(!TheCurrentThread->HasCapability(ECapabilityWriteDeviceData,__PLATSEC_DIAGNOSTIC_STRING("Checked by VMHalFunction(EVMHalSetSwapThresholds)")))
			K::UnlockedPlatformSecurityPanic();
		if ((K::MemModelAttributes & EMemModelAttrDataPaging) == 0)
			return KErrNotSupported;
		SVMSwapThresholds thresholds;
		kumemget32(&thresholds,a1,sizeof(thresholds));
		return SetSwapThresholds(thresholds);
		}

	case EVMHalSetThrashThresholds:
		if(!TheCurrentThread->HasCapability(ECapabilityWriteDeviceData,__PLATSEC_DIAGNOSTIC_STRING("Checked by VMHalFunction(EVMHalSetThrashThresholds)")))
			K::UnlockedPlatformSecurityPanic();
		return TheThrashMonitor.SetThresholds((TUint)a1, (TUint)a2);

#ifdef __DEMAND_PAGING_BENCHMARKS__
	case EVMHalGetPagingBenchmark:
		{
		TUint index = (TInt) a1;
		if (index >= EMaxPagingBm)
			return KErrNotFound;
		NKern::LockSystem();
		SPagingBenchmarkInfo info = ThePager.iBenchmarkInfo[index];
		NKern::UnlockSystem();
		kumemput32(a2,&info,sizeof(info));
		}		
		return KErrNone;
		
	case EVMHalResetPagingBenchmark:
		{
		TUint index = (TInt) a1;
		if (index >= EMaxPagingBm)
			return KErrNotFound;
		NKern::LockSystem();
		ThePager.ResetBenchmarkData((TPagingBenchmark)index);
		NKern::UnlockSystem();
		}
		return KErrNone;
#endif

	default:
		return KErrNotSupported;
		}
	}


#ifdef __DEMAND_PAGING_BENCHMARKS__

void DPager::ResetBenchmarkData(TPagingBenchmark aBm)
    {
    SPagingBenchmarkInfo& info = iBenchmarkInfo[aBm];
    info.iCount = 0;
    info.iTotalTime = 0;
    info.iMaxTime = 0;
    info.iMinTime = KMaxTInt;
    }
 
void DPager::RecordBenchmarkData(TPagingBenchmark aBm, TUint32 aStartTime, TUint32 aEndTime)
    {
    SPagingBenchmarkInfo& info = iBenchmarkInfo[aBm];
    ++info.iCount;
#if !defined(HIGH_RES_TIMER) || defined(HIGH_RES_TIMER_COUNTS_UP)
    TInt64 elapsed = aEndTime - aStartTime;
#else
    TInt64 elapsed = aStartTime - aEndTime;
#endif
    info.iTotalTime += elapsed;
    if (elapsed > info.iMaxTime)
        info.iMaxTime = elapsed;
    if (elapsed < info.iMinTime)
        info.iMinTime = elapsed;
    }

#endif //__DEMAND_PAGING_BENCHMARKS__


//
// Paging request management...
//

//
// DPagingRequest
//

DPagingRequest::DPagingRequest(DPagingRequestPool::TGroup& aPoolGroup)
	: iPoolGroup(aPoolGroup), iUseRegionMemory(0), iUseRegionIndex(0), iUseRegionCount(0)
	{
	}


FORCE_INLINE void DPagingRequest::SetUse(DMemoryObject* aMemory, TUint aIndex, TUint aCount)
	{
	__ASSERT_SYSTEM_LOCK;
	iUseRegionMemory = aMemory;
	iUseRegionIndex = aIndex;
	iUseRegionCount = aCount;
	}


TBool DPagingRequest::CheckUse(DMemoryObject* aMemory, TUint aIndex, TUint aCount)
	{
	return aMemory==iUseRegionMemory
		&& TUint(aIndex-iUseRegionIndex) < iUseRegionCount
		&& TUint(iUseRegionCount-TUint(aIndex-iUseRegionIndex)) <= aCount;
	}


void DPagingRequest::Release()
	{
	NKern::LockSystem();
	SetUse(0,0,0);
	Signal();
	}


void DPagingRequest::Wait()
	{
	__ASSERT_SYSTEM_LOCK;
	++iUsageCount;
	TInt r = iMutex->Wait();
	__NK_ASSERT_ALWAYS(r == KErrNone);
	}


void DPagingRequest::Signal()
	{
	__ASSERT_SYSTEM_LOCK;
	iPoolGroup.Signal(this);
	}


FORCE_INLINE TBool DPagingRequest::IsCollision(DMemoryObject* aMemory, TUint aIndex, TUint aCount)
	{
	__ASSERT_SYSTEM_LOCK;
	DMemoryObject* memory = iUseRegionMemory;
	TUint index = iUseRegionIndex;
	TUint count = iUseRegionCount;
	// note, this comparison would fail if either region includes page number KMaxTUint,
	// but it isn't possible to create a memory object which is > KMaxTUint pages...
	return memory == aMemory && index+count > aIndex && index < aIndex+aCount;
	}


TLinAddr DPagingRequest::MapPages(TUint aColour, TUint aCount, TPhysAddr* aPages)
	{
	__NK_ASSERT_DEBUG(iMutex->iCleanup.iThread == &Kern::CurrentThread());
	return iTempMapping.Map(aPages,aCount,aColour);
	}


void DPagingRequest::UnmapPages(TBool aIMBRequired)
	{
	__NK_ASSERT_DEBUG(iMutex->iCleanup.iThread == &Kern::CurrentThread());
	iTempMapping.Unmap(aIMBRequired);
	}


//
// DPageReadRequest
//

TInt DPageReadRequest::iAllocNext = 0;

TInt DPageReadRequest::Construct()
	{
	// allocate id and mutex...
	TUint id = (TUint)__e32_atomic_add_ord32(&iAllocNext, 1);
	_LIT(KLitPagingRequest,"PageReadRequest-");
	TBuf<sizeof("PageReadRequest-")+10> mutexName(KLitPagingRequest);
	mutexName.AppendNum(id);
	TInt r = K::MutexCreate(iMutex, mutexName, NULL, EFalse, KMutexOrdPageIn);
	if(r!=KErrNone)
		return r;

	// allocate space for mapping pages whilst they're being loaded...
	iTempMapping.Alloc(EMaxPages);

	// create memory buffer...
	TUint bufferSize = EMaxPages+1;
	DMemoryObject* bufferMemory;
	r = MM::MemoryNew(bufferMemory,EMemoryObjectUnpaged,bufferSize,EMemoryCreateNoWipe);
	if(r!=KErrNone)
		return r;
	MM::MemorySetLock(bufferMemory,iMutex);
	TPhysAddr physAddr;
	r = MM::MemoryAllocContiguous(bufferMemory,0,bufferSize,0,physAddr);
	(void)physAddr;
	if(r!=KErrNone)
		return r;
	DMemoryMapping* bufferMapping;
	r = MM::MappingNew(bufferMapping,bufferMemory,ESupervisorReadWrite,KKernelOsAsid);
	if(r!=KErrNone)
		return r;
	iBuffer = MM::MappingBase(bufferMapping);

	// ensure there are enough young pages to cope with new request object...
	r = ThePager.ResizeLiveList();
	if(r!=KErrNone)
		return r;

	return r;
	}


//
// DPageWriteRequest
//

TInt DPageWriteRequest::iAllocNext = 0;

TInt DPageWriteRequest::Construct()
	{
	// allocate id and mutex...
	TUint id = (TUint)__e32_atomic_add_ord32(&iAllocNext, 1);
	_LIT(KLitPagingRequest,"PageWriteRequest-");
	TBuf<sizeof("PageWriteRequest-")+10> mutexName(KLitPagingRequest);
	mutexName.AppendNum(id);
	TInt r = K::MutexCreate(iMutex, mutexName, NULL, EFalse, KMutexOrdPageOut);
	if(r!=KErrNone)
		return r;

	// allocate space for mapping pages whilst they're being loaded...
	iTempMapping.Alloc(EMaxPages);

	return r;
	}


//
// DPagingRequestPool
//

DPagingRequestPool::DPagingRequestPool(TUint aNumPageReadRequest,TUint aNumPageWriteRequest)
	: iPageReadRequests(aNumPageReadRequest), iPageWriteRequests(aNumPageWriteRequest)
	{
	TUint i;

	for(i=0; i<aNumPageReadRequest; ++i)
		{
		DPageReadRequest* req = new DPageReadRequest(iPageReadRequests);
		__NK_ASSERT_ALWAYS(req);
		TInt r = req->Construct();
		__NK_ASSERT_ALWAYS(r==KErrNone);
		iPageReadRequests.iRequests[i] = req;
		iPageReadRequests.iFreeList.Add(req);
		}

	for(i=0; i<aNumPageWriteRequest; ++i)
		{
		DPageWriteRequest* req = new DPageWriteRequest(iPageWriteRequests);
		__NK_ASSERT_ALWAYS(req);
		TInt r = req->Construct();
		__NK_ASSERT_ALWAYS(r==KErrNone);
		iPageWriteRequests.iRequests[i] = req;
		iPageWriteRequests.iFreeList.Add(req);
		}
	}


DPagingRequestPool::~DPagingRequestPool()
	{
	__NK_ASSERT_ALWAYS(0); // deletion not implemented
	}


DPageReadRequest* DPagingRequestPool::AcquirePageReadRequest(DMemoryObject* aMemory, TUint aIndex, TUint aCount)
	{
	NKern::LockSystem();

	DPagingRequest* req;

	// if we collide with page write operation...
	req = iPageWriteRequests.FindCollision(aMemory,aIndex,aCount);
	if(req)
		{
		// wait until write completes...
		req->Wait();
		req->Signal();
		return 0; // caller expected to retry if needed
		}

	// get a request object to use...
	req = iPageReadRequests.GetRequest(aMemory,aIndex,aCount);

	// check no new requests collide with us...
	if(iPageWriteRequests.FindCollision(aMemory,aIndex,aCount)
		|| iPageReadRequests.FindCollision(aMemory,aIndex,aCount))
		{
		// another operation is colliding with this region, give up and retry...
		req->Signal();
		return 0; // caller expected to retry if needed
		}

	// we have a request object which we can use...
	req->SetUse(aMemory,aIndex,aCount);

	NKern::UnlockSystem();
	return (DPageReadRequest*)req;
	}


DPageWriteRequest* DPagingRequestPool::AcquirePageWriteRequest(DMemoryObject* aMemory, TUint aIndex, TUint aCount)
	{
	NKern::LockSystem();

	DPagingRequest* req;

	for(;;)
		{
		// get a request object to use...
		req = iPageWriteRequests.GetRequest(aMemory,aIndex,aCount);

		if(iPageWriteRequests.FindCollision(aMemory,aIndex,aCount))
			{
			// another write operation is colliding with this region, give up and retry...
			req->Signal();
			// Reacquire the system lock as Signal() above will release it.
			NKern::LockSystem();
			continue;
			}

		break;
		}

	// we have a request object which we can use...
	req->SetUse(aMemory,aIndex,aCount);

	NKern::UnlockSystem();
	return (DPageWriteRequest*)req;
	}


DPagingRequestPool::TGroup::TGroup(TUint aNumRequests)
	{
	iNumRequests = aNumRequests;
	iRequests = new DPagingRequest*[aNumRequests];
	__NK_ASSERT_ALWAYS(iRequests);
	}


DPagingRequest* DPagingRequestPool::TGroup::FindCollision(DMemoryObject* aMemory, TUint aIndex, TUint aCount)
	{
	__ASSERT_SYSTEM_LOCK;
	DPagingRequest** ptr = iRequests;
	DPagingRequest** ptrEnd = ptr+iNumRequests;
	while(ptr<ptrEnd)
		{
		DPagingRequest* req = *ptr++;
		if(req->IsCollision(aMemory,aIndex,aCount))
			return req;
		}
	return 0;
	}


static TUint32 RandomSeed = 33333;

DPagingRequest* DPagingRequestPool::TGroup::GetRequest(DMemoryObject* aMemory, TUint aIndex, TUint aCount)
	{
	__NK_ASSERT_DEBUG(iNumRequests > 0);

	// try using an existing request which collides with this region...
	DPagingRequest* req  = FindCollision(aMemory,aIndex,aCount);
	if(!req)
		{
		// use a free request...
		req = (DPagingRequest*)iFreeList.GetFirst();
		if(req)
			{
			// free requests aren't being used...
			__NK_ASSERT_DEBUG(req->iUsageCount == 0);
			}
		else
			{
			// pick a random request...
			RandomSeed = RandomSeed*69069+1; // next 'random' number
			TUint index = (TUint64(RandomSeed) * TUint64(iNumRequests)) >> 32;
			req = iRequests[index];
			__NK_ASSERT_DEBUG(req->iUsageCount > 0); // we only pick random when none are free
			}
		}

	// wait for chosen request object...
	req->Wait();

	return req;
	}


void DPagingRequestPool::TGroup::Signal(DPagingRequest* aRequest)
	{
	// if there are no threads waiting on the mutex then return it to the free pool...
	__NK_ASSERT_DEBUG(aRequest->iUsageCount > 0);
	if (--aRequest->iUsageCount==0)
		iFreeList.AddHead(aRequest);

	aRequest->iMutex->Signal();
	}


/**
Register the specified paging device with the kernel.

@param aDevice	A pointer to the paging device to install

@return KErrNone on success
*/
EXPORT_C TInt Kern::InstallPagingDevice(DPagingDevice* aDevice)
	{
	TRACEB(("Kern::InstallPagingDevice(0x%08x) name='%s' type=%d",aDevice,aDevice->iName,aDevice->iType));

	__NK_ASSERT_ALWAYS(aDevice->iReadUnitShift <= KPageShift);

	TInt r = KErrNotSupported;	// Will return this if unsupported device type is installed

	// create the pools of page out and page in requests...
	const TInt writeReqs = (aDevice->iType & DPagingDevice::EData) ? KPagingRequestsPerDevice : 0;
	aDevice->iRequestPool = new DPagingRequestPool(KPagingRequestsPerDevice,writeReqs);
	if(!aDevice->iRequestPool)
		{
		r = KErrNoMemory;
		goto exit;
		}

	if(aDevice->iType & DPagingDevice::ERom)
		{
		r = TheRomMemoryManager->InstallPagingDevice(aDevice);
		if(r!=KErrNone)
			goto exit;
		}

	if(aDevice->iType & DPagingDevice::ECode)
		{
		r = TheCodePagedMemoryManager->InstallPagingDevice(aDevice);
		if(r!=KErrNone)
			goto exit;
		}

	if(aDevice->iType & DPagingDevice::EData)
		{
		r = TheDataPagedMemoryManager->InstallPagingDevice(aDevice);
		if(r!=KErrNone)
			goto exit;
		}

 	if (K::MemModelAttributes & (EMemModelAttrRomPaging | EMemModelAttrCodePaging | EMemModelAttrDataPaging))
		TheThrashMonitor.Start();

exit:
	TRACEB(("Kern::InstallPagingDevice returns %d",r));
	return r;
	}



//
// DDemandPagingLock
//

EXPORT_C DDemandPagingLock::DDemandPagingLock()
	: iReservedPageCount(0), iLockedPageCount(0), iPinMapping(0)
	{
	}


EXPORT_C TInt DDemandPagingLock::Alloc(TInt aSize)
	{
	TRACEP(("DDemandPagingLock[0x%08x]::Alloc(0x%x)",this,aSize));
	iMaxPageCount = ((aSize-1+KPageMask)>>KPageShift)+1;

	TInt r = KErrNoMemory;

	NKern::ThreadEnterCS();

	TUint maxPt = DVirtualPinMapping::MaxPageTables(iMaxPageCount);
	// Note, we need to reserve whole pages even for page tables which are smaller
	// because pinning can remove the page from live list...
	TUint reserve = iMaxPageCount+maxPt*KNumPagesToPinOnePageTable;
	if(ThePager.ReservePages(reserve,(TUint&)iReservedPageCount))
		{
		iPinMapping = DVirtualPinMapping::New(iMaxPageCount);
		if(iPinMapping)
			r = KErrNone;
		else
			ThePager.UnreservePages((TUint&)iReservedPageCount);
		}

	NKern::ThreadLeaveCS();
	TRACEP(("DDemandPagingLock[0x%08x]::Alloc returns %d, iMaxPageCount=%d, iReservedPageCount=%d",this,r,iMaxPageCount,iReservedPageCount));
	return r;
	}


EXPORT_C void DDemandPagingLock::Free()
	{
	TRACEP(("DDemandPagingLock[0x%08x]::Free()"));
	Unlock();
	NKern::ThreadEnterCS();
	DVirtualPinMapping* pinMapping = (DVirtualPinMapping*)__e32_atomic_swp_ord_ptr(&iPinMapping, 0);
	if (pinMapping)
		pinMapping->Close();
	NKern::ThreadLeaveCS();
	ThePager.UnreservePages((TUint&)iReservedPageCount);
	}


EXPORT_C TInt DDemandPagingLock::Lock(DThread* aThread, TLinAddr aStart, TInt aSize)
	{
//	TRACEP(("DDemandPagingLock[0x%08x]::Lock(0x%08x,0x%08x,0x%08x)",this,aThread,aStart,aSize));
	if(iLockedPageCount)
		__NK_ASSERT_ALWAYS(0); // lock already used

	// calculate the number of pages that need to be locked...
	TUint mask=KPageMask;
	TUint offset=aStart&mask;
	TInt numPages = (aSize+offset+mask)>>KPageShift;
	if(numPages>iMaxPageCount)
		__NK_ASSERT_ALWAYS(0);

	NKern::ThreadEnterCS();

	// find mapping which covers the specified region...
	TUint offsetInMapping;
	TUint mapInstanceCount;
	DMemoryMapping* mapping = MM::FindMappingInThread((DMemModelThread*)aThread, aStart, aSize, offsetInMapping, mapInstanceCount);
	if(!mapping)
		{
		NKern::ThreadLeaveCS();
		return KErrBadDescriptor;
		}

	MmuLock::Lock(); 
	DMemoryObject* memory = mapping->Memory();
	if(mapInstanceCount != mapping->MapInstanceCount() || !memory)
		{// Mapping has been reused or no memory.
		MmuLock::Unlock();
		mapping->Close();
		NKern::ThreadLeaveCS();
		return KErrBadDescriptor;
		}

	if(!memory->IsDemandPaged())
		{
		// memory not demand paged, so we have nothing to do...
		MmuLock::Unlock();
		mapping->Close();
		NKern::ThreadLeaveCS();
		return KErrNone;
		}

	// Open a reference on the memory so it doesn't get deleted.
	memory->Open();
	MmuLock::Unlock();

	// pin memory...
	TUint index = (offsetInMapping>>KPageShift)+mapping->iStartIndex;
	TUint count = ((offsetInMapping&KPageMask)+aSize+KPageMask)>>KPageShift;
	TInt r = ((DVirtualPinMapping*)iPinMapping)->Pin(	memory,index,count,mapping->Permissions(),
														mapping, mapInstanceCount);

	if(r==KErrNotFound)
		{
		// some memory wasn't present, so treat this as an error...
		memory->Close();
		mapping->Close();
		NKern::ThreadLeaveCS();
		return KErrBadDescriptor;
		}

	// we can't fail to pin otherwise...
	__NK_ASSERT_DEBUG(r!=KErrNoMemory); // separate OOM assert to aid debugging
	__NK_ASSERT_ALWAYS(r==KErrNone);

	// indicate that we have actually pinned...
	__NK_ASSERT_DEBUG(iLockedPageCount==0);
	iLockedPageCount = count;

	// cleanup...
	memory->Close();
	mapping->Close();
	NKern::ThreadLeaveCS();

	return 1;
	}


EXPORT_C void DDemandPagingLock::DoUnlock()
	{
	NKern::ThreadEnterCS();
	((DVirtualPinMapping*)iPinMapping)->Unpin();
	__NK_ASSERT_DEBUG(iLockedPageCount);
	iLockedPageCount = 0;
	NKern::ThreadLeaveCS();
	}


