// Copyright (c) 1995-2009 Nokia Corporation and/or its subsidiary(-ies).
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
// e32test\mmu\t_chunk.cpp
// Tests on RChunk objects in connection with demand paging.
// Tests exercise the locking, unlocking, commiting and decommiting of 
// pages to chunk objects.
// 1  Check Unlocked page gets counted as free memory
// 2  Check Unlock/Lock preserves page contents
// 3  Tests at chunk offset '0'
// 4    Check Lock is idempotent
// 5    Check non page aligned Unlock/Lock
// 6    Check unlocked pages get reclaimed for new memory allocation
// 7    Check reclaimed memory is unmapped from original location
// 8    Check Lock fails when memory is reclaimed
// 9    Check Lock failure Decommits memory
// 10   Recommit memory to chunk
// 11   Check Commit on unlocked pages
// 12   Check Commit on unlocked and reclaimed pages
// 13   Restore chunk
// 14 Tests at chunk offset 'PageSize'
// 15   Check Lock is idempotent
// 16   Check non page aligned Unlock/Lock
// 17   Check unlocked pages get reclaimed for new memory allocation
// 18   Check reclaimed memory is unmapped from original location
// 19   Check Lock fails when memory is reclaimed
// 20   Check Lock failure Decommits memory
// 21   Recommit memory to chunk
// 22   Check Commit on unlocked pages
// 23   Check Commit on unlocked and reclaimed pages
// 24   Restore chunk
// 25 Tests at chunk offset '0x100000-PageSize'
// 26   Check Lock is idempotent
// 27   Check non page aligned Unlock/Lock
// 28   Check unlocked pages get reclaimed for new memory allocation
// 29   Check reclaimed memory is unmapped from original location
// 30   Check Lock fails when memory is reclaimed
// 31   Check Lock failure Decommits memory
// 32   Recommit memory to chunk
// 33   Check Commit on unlocked pages
// 34   Check Commit on unlocked and reclaimed pages
// 35   Restore chunk
// 36 Tests at chunk offset '0x400000-PageSize'
// 37   Check Lock is idempotent
// 38   Check non page aligned Unlock/Lock
// 39   Check unlocked pages get reclaimed for new memory allocation
// 40   Check reclaimed memory is unmapped from original location
// 41   Check Lock fails when memory is reclaimed
// 42   Check Lock failure Decommits memory
// 43   Recommit memory to chunk
// 44 Check Commit on unlocked pages
// 45 Check Commit on unlocked and reclaimed pages
// 46 Restore chunk
// 47 Big Unlock/Lock
// 48 Benchmarks...
// 49 Close chunk with unlocked pages which have been flushed
// 
//

//! @SYMTestCaseID			KBASE-T_CACHECHUNK-0336
//! @SYMTestType			UT
//! @SYMPREQ				PREQ1110
//! @SYMTestCaseDesc		Demand Paging Loader Stress Tests
//! @SYMTestActions			0  Commit all of memory
//! @SYMTestExpectedResults All tests should pass.
//! @SYMTestPriority        High
//! @SYMTestStatus          Implemented

#define __E32TEST_EXTENSION__

#include <e32test.h>
#include <e32panic.h>
#include <e32svr.h>
#include <hal.h>
#include "mmudetect.h"
#include "d_memorytest.h"
#include "d_gobble.h"
#include <dptest.h>
#include "freeram.h"

LOCAL_D RTest test(_L("T_CACHECHUNK"));

RMemoryTestLdd MemoryTest;

RChunk TestChunk;
TUint8* TestChunkBase;
TInt CommitEnd;
TInt PageSize;
TInt NoFreeRam;
RTimer Timer;



void FillPage(TUint aOffset)
	{
	TUint8* ptr = TestChunkBase+aOffset;
	TUint8* ptrEnd = ptr+PageSize;
	do *((TUint32*&)ptr)++ = aOffset+=4;
	while(ptr<ptrEnd);
	}


TBool CheckPage(TUint aOffset)
	{
	TUint8* ptr = TestChunkBase+aOffset;
	TUint8* ptrEnd = ptr+PageSize;
	do if(*((TUint32*&)ptr)++ != (aOffset+=4)) break;
	while(ptr<ptrEnd);
	return ptr==ptrEnd;
	}


TBool CheckPages(TUint aOffset, TInt aNumPages)
	{
	while(aNumPages--)
		if(!CheckPage(aOffset+=PageSize))
			return EFalse;
	return ETrue;
	}


TBool IsPageMapped(TUint aOffset)
	{
	TUint32 value;
	TInt r=MemoryTest.ReadMemory(TestChunkBase+aOffset,value);
	return r==KErrNone;
	}


void Tests(TInt aOffset)
	{
	if(aOffset+5*PageSize>=CommitEnd)
		{
		test.Start(_L("TEST NOT RUN - Not enough system RAM"));
		test.End();
		return;
		}
	TInt r;
	TInt freeRam;

	TUint origChunkSize = TestChunk.Size();

	test.Start(_L("Check Unlock is idempotent"));
	r = TestChunk.Unlock(aOffset+PageSize,PageSize);
	test_KErrNone(r);
	freeRam = FreeRam();
	test(freeRam==NoFreeRam+PageSize);
	r = TestChunk.Unlock(aOffset+PageSize,PageSize);
	test_KErrNone(r);
	test_Equal(FreeRam(), freeRam);
	// Ensure unlock on reclaimed pages is idempotent
	TInt flushSupported = UserSvr::HalFunction(EHalGroupVM,EVMHalFlushCache,0,0);
	r = TestChunk.Unlock(aOffset+PageSize,PageSize);
	test_KErrNone(r);
	test_Equal(FreeRam(), freeRam);
	test_Equal(origChunkSize, TestChunk.Size());
	
	if (flushSupported == KErrNotSupported)
		{// Flush cache not supported so lock won't fail so no need to recommit the pages.
		test_KErrNone(TestChunk.Lock(aOffset+PageSize,PageSize));
		}
	else
		{// Recommit the reclaimed pages.
		test_KErrNone(flushSupported);
		test_Equal(KErrNotFound, TestChunk.Lock(aOffset+PageSize,PageSize));
		test_KErrNone(TestChunk.Commit(aOffset+PageSize,PageSize));
		}

	test.Next(_L("Check Lock is idempotent"));
	r = TestChunk.Lock(aOffset,3*PageSize);
	test_KErrNone(r);
	freeRam = FreeRam();
	test(freeRam==NoFreeRam);
	CheckPages(aOffset,3);
	r = TestChunk.Lock(aOffset,3*PageSize);
	test_KErrNone(r);
	CheckPages(aOffset,3);
	freeRam = FreeRam();
	test(freeRam==NoFreeRam);
	test_Equal(origChunkSize, TestChunk.Size());

	test.Next(_L("Check non page aligned Unlock/Lock"));
	r = TestChunk.Unlock(aOffset+PageSize-1,1);
	test_KErrNone(r);
	freeRam = FreeRam();
	test(freeRam==NoFreeRam+PageSize);
	r = TestChunk.Lock(aOffset+PageSize-1,1);
	test_KErrNone(r);
	freeRam = FreeRam();
	test(freeRam==NoFreeRam);
	r = TestChunk.Unlock(aOffset+PageSize-1,2);
	test_KErrNone(r);
	freeRam = FreeRam();
	test(freeRam==NoFreeRam+PageSize*2);
	r = TestChunk.Lock(aOffset+PageSize-1,2);
	test_KErrNone(r);
	freeRam = FreeRam();
	test(freeRam==NoFreeRam);
	test_Equal(origChunkSize, TestChunk.Size());

	test.Next(_L("Check unlocked pages get reclaimed for new memory allocation"));
	r=TestChunk.Commit(CommitEnd,PageSize);
	test(r==KErrNoMemory);
	r = TestChunk.Unlock(aOffset,4*PageSize);
	test_KErrNone(r);
	freeRam = FreeRam();
	test(freeRam==NoFreeRam+PageSize*4);
	r=TestChunk.Commit(CommitEnd,PageSize);
	test_KErrNone(r);
	freeRam = FreeRam();
	test(freeRam<NoFreeRam+PageSize*4);
	r=TestChunk.Decommit(CommitEnd,PageSize);
	test_KErrNone(r);
	freeRam = FreeRam();
	test(freeRam==NoFreeRam+PageSize*4);
	UserSvr::HalFunction(EHalGroupVM,EVMHalFlushCache,0,0); // make sure unlocked page is gone
	freeRam = FreeRam();
	test(freeRam==NoFreeRam+PageSize*4);

#ifndef __WINS__ // wins fakery doesn't actually do this
	test.Next(_L("Check reclaimed memory is unmapped and decommitted from original location"));
	TInt mappedPages = IsPageMapped(aOffset+PageSize*0);
	mappedPages += IsPageMapped(aOffset+PageSize*1);
	mappedPages += IsPageMapped(aOffset+PageSize*2);
	mappedPages += IsPageMapped(aOffset+PageSize*3);
	test(mappedPages<4);
#endif
	if(aOffset>PageSize)
		{
		test(CheckPage(aOffset+PageSize*-1)); // should be left mapped
		}
	test(CheckPage(aOffset+PageSize*4)); // should be left mapped

	test.Next(_L("Check Lock fails when memory is reclaimed"));
	r = TestChunk.Lock(aOffset,4*PageSize);
	test(r==KErrNotFound);
	freeRam = FreeRam();
	test(freeRam==NoFreeRam+PageSize*4);

	test.Next(_L("Check Lock failure Decommits memory"));
	test(!IsPageMapped(aOffset+PageSize*0));
	test(!IsPageMapped(aOffset+PageSize*1));
	test(!IsPageMapped(aOffset+PageSize*2));
	test(!IsPageMapped(aOffset+PageSize*3));
	test_Equal(origChunkSize-PageSize*4, TestChunk.Size());

	test.Next(_L("Recommit memory to chunk"));
	TInt offset;
	for(offset=aOffset; offset<aOffset+PageSize*4; offset+=PageSize)
		{
		r=TestChunk.Commit(offset,PageSize);
		test_KErrNone(r);
		FillPage(offset);
		}
	freeRam = FreeRam();
	test(freeRam==NoFreeRam);
	test_Equal(origChunkSize, TestChunk.Size());

	test.Next(_L("Check Commit on unlocked pages"));
	r = TestChunk.Unlock(aOffset,4*PageSize);
	test_KErrNone(r);
	freeRam = FreeRam();
	test(freeRam>=NoFreeRam+PageSize*4);
	r=TestChunk.Commit(aOffset,4*PageSize);
	test(r==KErrAlreadyExists);
	freeRam = FreeRam();
	test(freeRam>=NoFreeRam+PageSize*4);
	test_Equal(origChunkSize, TestChunk.Size());

	test.Next(_L("Check Commit on unlocked and reclaimed pages"));
	// unlock and force a page to be reclaimed...
	r=TestChunk.Commit(CommitEnd,PageSize);
	test_KErrNone(r);
	r=TestChunk.Decommit(CommitEnd,PageSize);
	test_KErrNone(r);
	UserSvr::HalFunction(EHalGroupVM,EVMHalFlushCache,0,0); // make sure unlocked page is gone
	freeRam = FreeRam();
	test(freeRam>=NoFreeRam+PageSize*4);
	// check can't commit any pages (because they are unlocked, not decommitted)...
	r=TestChunk.Commit(aOffset+PageSize*0,PageSize);
	test(r==KErrAlreadyExists);
	r=TestChunk.Commit(aOffset+PageSize*1,PageSize);
	test(r==KErrAlreadyExists);
	r=TestChunk.Commit(aOffset+PageSize*2,PageSize);
	test(r==KErrAlreadyExists);
	r=TestChunk.Commit(aOffset+PageSize*3,PageSize);
	test(r==KErrAlreadyExists);
	freeRam = FreeRam();
	test(freeRam>=NoFreeRam+PageSize*4);
	test_Equal(origChunkSize, TestChunk.Size());
	// Restore the chunk to original size.
	r = TestChunk.Lock(aOffset,4*PageSize);
	test_Equal(r, KErrNotFound);
	freeRam = FreeRam();
	test_Compare(freeRam, >=, NoFreeRam+PageSize*4);
	test_Equal(origChunkSize - PageSize*4, TestChunk.Size());
	r = TestChunk.Commit(aOffset, PageSize*4);
	test_KErrNone(r);

	test.Next(_L("Check Decommit on unlocked pages"));
	r = TestChunk.Unlock(aOffset,PageSize*4);
	test_KErrNone(r);
	test(FreeRam() >= NoFreeRam+PageSize*4);
	r=TestChunk.Decommit(aOffset, PageSize*4);
	test_KErrNone(r);
	freeRam = FreeRam();
	test_Compare(freeRam, >=, NoFreeRam+PageSize*4);
	test_Equal(origChunkSize - PageSize*4, TestChunk.Size());
	// Restore chunk back to original state
	r = TestChunk.Commit(aOffset, PageSize*4);
	test_KErrNone(r);
	test(FreeRam() == NoFreeRam);

	test.Next(_L("Check Decommit on unlocked and reclaimed pages"));
	r = TestChunk.Unlock(aOffset,PageSize*4);
	test_KErrNone(r);
	freeRam = FreeRam();
	test_Compare(freeRam, >=, NoFreeRam+PageSize*4);
	r=TestChunk.Commit(CommitEnd,PageSize);
	test_KErrNone(r);
	r=TestChunk.Decommit(CommitEnd,PageSize);
	test_KErrNone(r);
	UserSvr::HalFunction(EHalGroupVM,EVMHalFlushCache,0,0); // make sure unlocked page is gone
	freeRam = FreeRam();
	test_Compare(freeRam, >=, NoFreeRam+PageSize*4);
	r=TestChunk.Decommit(aOffset, PageSize*4);
	test_KErrNone(r);
	freeRam = FreeRam();
	test_Compare(freeRam, >=, NoFreeRam+PageSize*4);
	test_Equal(origChunkSize - PageSize*4, TestChunk.Size());

	test.Next(_L("Restore chunk"));
	test_Equal(origChunkSize-PageSize*4, TestChunk.Size());
	for(offset=aOffset; offset<aOffset+PageSize*4; offset+=PageSize)
		{
		r=TestChunk.Commit(offset,PageSize);
		test_KErrNone(r);
		FillPage(offset);
		}
	freeRam = FreeRam();
	test(freeRam==NoFreeRam);
	test_Equal(origChunkSize, TestChunk.Size());

	test.End();
	}



void TestBenchmarks()
	{
	TInt r = TestChunk.Unlock(0,CommitEnd); // start with everthing unlocked
	test_KErrNone(r);
	TInt testSizes[] = { PageSize,1<<16,1<<20,0 };
	TInt* sizes = testSizes;
	TInt size;
	while((size=*sizes++)!=0)
		{
		TRequestStatus status;
		Timer.After(status,1);
		User::WaitForRequest(status);
		TInt KRunTime = 1*1000*1000;
		Timer.After(status,KRunTime);
		TInt count = 0;
		while(status==KRequestPending)
			{
			r = TestChunk.Lock(0,size);
			test_KErrNone(r);
			r = TestChunk.Unlock(0,size);
			test_KErrNone(r);
			++count;
			}
		User::WaitForRequest(status);
		test.Printf(_L("Unlock/Lock of %d kB takes %d us\n"),size>>10,KRunTime/count);
		}
	}



void TestUnlockOld()
	{
	// we start with TestChunk being locked and no or little free RAM
	// (hence live list should be close to minimum size.)

	// get sizes...
	TUint min = 0;
	TUint max = 0;
	TUint cur = 0;
	TInt r = DPTest::CacheSize(min,max,cur);

	// manipulate demand paging live list so we end up with zero old pages...

	r = TestChunk.Unlock(0,min*2); // dump 2*min bytes at start of live list
	test_KErrNone(r);
	// live list now cur+2*min bytes

	r = TestChunk.Commit(CommitEnd,cur); // use up 'cur' bytes of RAM from end of live list
	test_KErrNone(r);
	// live list now 2*min bytes of pages which were unlocked from our test chunk

	// lock pages until free RAM is <= 2 pages.
	// this should remove all of the 'old' pages
	TUint i = 0;
	while(FreeRam()>2*PageSize)
		{
		TestChunk.Lock(i,PageSize);
		i += PageSize;
		test(i<=min);
		}
	// live list now min+2*PageSize bytes, with no old pages

	// now commit memory, which forces allocation from the demand paging live list
	// which doesn't have any old pages (the purpose of this test)...
	TUint extra = 0;
	for(;;)
		{
		r = TestChunk.Commit(CommitEnd+min+extra,PageSize);
		if(r==KErrNoMemory)
			break;
		extra += PageSize;
		}
	test(extra>0);

	// restore commit state...
	r = TestChunk.Decommit(CommitEnd,min+extra);
	test_KErrNone(r);
	r = TestChunk.Decommit(0,min*2);
	test_KErrNone(r);
	r = TestChunk.Commit(0,min*2);
	test_KErrNone(r);
	}



TInt E32Main()
	{
	test.Title();

	if (!HaveVirtMem())
		{
		test.Printf(_L("This test requires an MMU\n"));
		return KErrNone;
		}
	test.Start(_L("Initialise test"));
	test.Next(_L("Load gobbler LDD"));
	TInt r = User::LoadLogicalDevice(KGobblerLddFileName);
	test(r==KErrNone || r==KErrAlreadyExists);
	RGobbler gobbler;
	r = gobbler.Open();
	test(r==KErrNone);
	TUint32 taken = gobbler.GobbleRAM(496*1024*1024);
	test.Printf(_L("Gobbled: %dK\n"), taken/1024);
	test.Printf(_L("Free RAM 0x%08X bytes\n"),FreeRam());

	test_KErrNone(HAL::Get(HAL::EMemoryPageSize,PageSize));
	TInt totalRAM;
	test_KErrNone(HAL::Get(HAL::EMemoryRAM, totalRAM));
	totalRAM -= taken;
	test.Printf(_L("totalRAM=%dK\n"), totalRAM/1024);

	test(KErrNone==MemoryTest.Open());
	// Create the test chunk.  It must not be paged otherwise
	// unlocking its pages will have no effect.
	TChunkCreateInfo createInfo;
	createInfo.SetCache(totalRAM);
	test_KErrNone(TestChunk.Create(createInfo));
	TestChunkBase = TestChunk.Base();

	test(KErrNone==Timer.CreateLocal());
	UserSvr::HalFunction(EHalGroupVM,EVMHalFlushCache,0,0);

	test.Next(_L("Commit all of memory"));
	CommitEnd = 0;
	while(KErrNone==(r=TestChunk.Commit(CommitEnd,PageSize)))
		{
		FillPage(CommitEnd);
		CommitEnd += PageSize;
		}
	test(r==KErrNoMemory);
	NoFreeRam = FreeRam();
	test(NoFreeRam<=PageSize);

	test.Next(_L("Check Unlocked page gets counted as free memory"));
	r = TestChunk.Unlock(0,PageSize);
	test_KErrNone(r);
	TInt freeRam = FreeRam();
	test(freeRam==NoFreeRam+PageSize);
	r = TestChunk.Lock(0,PageSize);
	test_KErrNone(r);
	freeRam = FreeRam();
	test(freeRam==NoFreeRam);

	test.Next(_L("Check Unlock/Lock preserves page contents"));
	TInt offset;
	for(offset=0; offset<CommitEnd; offset+=PageSize)
		{
		test(CheckPage(offset));
		r = TestChunk.Unlock(offset,PageSize);
		test_KErrNone(r);
		r = TestChunk.Lock(offset,PageSize);
		test_KErrNone(r);
		test(CheckPage(offset));
		freeRam = FreeRam();
		test(freeRam==NoFreeRam);
		}

	test.Next(_L("Tests at chunk offset '0'"));
	Tests(0);
	test.Next(_L("Tests at chunk offset 'PageSize'"));
	Tests(PageSize);
	test.Next(_L("Tests at chunk offset '0x100000-PageSize'"));
	Tests(0x100000-PageSize);
	test.Next(_L("Tests at chunk offset '0x400000-PageSize'"));
	Tests(0x400000-PageSize);

	// Remove limit on max size of live list
	TUint originalMin = 0;
	TUint originalMax = 0;
	TUint currentSize = 0;
	r = DPTest::CacheSize(originalMin, originalMax, currentSize);
	test(r == KErrNone || r == KErrNotSupported);
	TBool resizeCache = r == KErrNone;
	if (resizeCache)
		test_KErrNone(DPTest::SetCacheSize(originalMin, KMaxTUint));

	test.Next(_L("Big Unlock/Lock"));
	r = TestChunk.Unlock(0,CommitEnd);
	test_KErrNone(r);
	freeRam = FreeRam();
	test(freeRam>=NoFreeRam+CommitEnd);
	r = TestChunk.Lock(0,CommitEnd);
	test_KErrNone(r);
	freeRam = FreeRam();
	test_Equal(NoFreeRam, freeRam);

	if (resizeCache)
		{
		test.Next(_L("Check Unlock of old pages doesn't cause problems"));
		TestUnlockOld();
		}

	test.Next(_L("Benchmarks..."));
	TestBenchmarks();

	test.Next(_L("Close chunk with unlocked pages which have been flushed"));
	r = TestChunk.Unlock(0,CommitEnd);
	test_KErrNone(r);
	UserSvr::HalFunction(EHalGroupVM,EVMHalFlushCache,0,0);
	TestChunk.Close();

	test.Next(_L("Check can't lock/unlock non-cache chunks"));
	RChunk chunk;
	test_KErrNone(chunk.CreateDisconnectedLocal(0,PageSize,2*PageSize));
	test_Equal(KErrGeneral,chunk.Lock(PageSize,PageSize));
	test_Equal(KErrGeneral,chunk.Unlock(0,PageSize));
	chunk.Close();

	// Restore original settings for live list size
	if (resizeCache)
		test_KErrNone(DPTest::SetCacheSize(originalMin, originalMax));

	// end...
	test.End();
	MemoryTest.Close();
	gobbler.Close();
	test.Close();

	return KErrNone;
	}
