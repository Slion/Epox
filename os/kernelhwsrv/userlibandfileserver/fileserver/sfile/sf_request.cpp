// Copyright (c) 2002-2009 Nokia Corporation and/or its subsidiary(-ies).
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
// f32\sfile\sf_request.cpp
// 
//

#include "sf_std.h"
#include "sf_file_cache.h"
#ifdef SYMBIAN_F32_ENHANCED_CHANGE_NOTIFICATION	
#include "sf_notifier.h"
#endif //SYMBIAN_F32_ENHANCED_CHANGE_NOTIFICATION	


TParse dummyP;
RMessage2 dummyM;

CFsClientMessageRequest* RequestAllocator::iFreeHead;				
CFsClientMessageRequest* RequestAllocator::iCloseHead;
TInt RequestAllocator::iAllocNum;
TInt RequestAllocator::iAllocNumOperation;
TMsgOperation* RequestAllocator::iFreeHeadSupOp;

#if defined(_USE_CONTROLIO) || defined(_DEBUG) || defined(_DEBUG_RELEASE)
TInt RequestAllocator::iAllocated;
#endif
RFastLock RequestAllocator::iCacheLock;

void RequestAllocator::Initialise()
	{	
	iFreeHead=NULL;  
	iCloseHead=NULL; 
	iAllocNum=0; 
#if defined(_USE_CONTROLIO) || defined(_DEBUG) || defined(_DEBUG_RELEASE)
	iAllocated=0;
#endif
	iAllocNumOperation=0;
	iFreeHeadSupOp=NULL;
	}

TInt RequestAllocator::AllocRequest(TInt aNum)
//
//	Allocates a group of request objects
//
	{
    TInt i;
	if(iAllocNum < KMaxRequestAllocated)
		{
		__CACHE_PRINT(_L("RequestAllocator::AllocRequest() Not reached the limit"));
		CFsClientMessageRequest* list;
		CFsClientMessageRequest* start;
		list = new CFsClientMessageRequest[KAllocReqBlock];
		start = list;
		if(!list)
			return KErrNoMemory;
		
		// Make sure the constructors are called for every element in the array
		// - some compilers don't do this
		for(TInt j=0; j<KAllocReqBlock; j++)
			{
			CFsClientMessageRequest* request = &list[j];
			new(request) CFsClientMessageRequest();
			}

		iAllocNum += KAllocReqBlock;
		CFsClientMessageRequest* last;
		for(i=1;i<KAllocReqBlock;i++)
			{
			last = list;
			list++;
			last->iNext = list;
			}
		list->iNext = iFreeHead;
		iFreeHead = start;
		return KErrNone;
		}
	else
		{
		__CACHE_PRINT1(_L("RequestAllocator::AllocRequest() Limit exceeded Count = %d"),aNum);
		CFsClientMessageRequest* request;
		for(i=0;i<aNum;i++)
			{
			request=new CFsClientMessageRequest;
			if(!request)
				return KErrNoMemory;
#if defined(_USE_CONTROLIO) || defined(_DEBUG) || defined(_DEBUG_RELEASE)
			iAllocated++;
#endif
			request->SetAllocated();
			request->iNext=iFreeHead;
			iFreeHead=request;
			}
		return KErrNone;
		}
	}

void RequestAllocator::FreeRequest(CFsClientMessageRequest* aRequest)
//
//free request 
//
	{
	__CACHE_PRINT1(_L("PLUGIN: RequestAllocator::FreeRequest for %x"), aRequest);
	if(aRequest->IsAllocated())
		{
		__CACHE_PRINT(_L("RequestAllocator::FreeRequest() Allocated request"));
		delete(aRequest);
#if defined(_USE_CONTROLIO) || defined(_DEBUG) || defined(_DEBUG_RELEASE)
		iAllocated--;
#endif
		return;
		}
	
	__CACHE_PRINT(_L("RequestAllocator::FreeRequest() returning to free list"));
	iCacheLock.Wait();
	aRequest->iNext = iFreeHead;
	iFreeHead=aRequest;
	aRequest->SetSubstedDrive(NULL);
	iCacheLock.Signal();
	}

void RequestAllocator::OpenSubFailed(CSessionFs* aSession)
//
//	Move requst from closed list to free list
//
	{
	__ASSERT_DEBUG(iCloseHead!=NULL,Fault(ERequestAllocatorOpenSubFailed)); // On arriving here Close Queue is supposed to be empty
	__ASSERT_ALWAYS(aSession!=NULL,Fault(ERequestAllocatorOpenSubFailed));
	if (iCloseHead==NULL)
		{
		return;
		}
	iCacheLock.Wait();
	CFsClientMessageRequest* rp = iCloseHead;
	iCloseHead = rp->iNext;
	
	// dec the number of closed requests owned by this session
	aSession->CloseRequestCountDec();

	rp->iNext = NULL;
	if(rp->IsAllocated())
		{
		__CACHE_PRINT(_L("RequestAllocator::OpenSubFailed() Allocated request"));
		delete(rp);
#if defined(_USE_CONTROLIO) || defined(_DEBUG) || defined(_DEBUG_RELEASE)
		iAllocated--;
#endif
		}
	else
		{
		__CACHE_PRINT(_L("RequestAllocator::OpenSubFailed()"));
		if(iFreeHead)
			{
			rp->iNext = iFreeHead;
			}
		else
			{
			rp->iNext = NULL;
			}

		iFreeHead = rp;		
		}
	iCacheLock.Signal();
	}

TInt RequestAllocator::GetMessageRequest(const TOperation& aOperation,const RMessage2& aMessage,CFsClientMessageRequest* &aRequest)
//
//	tries to get a pre allocated message from the cache. Failing that allocates one indivisualy 
//	
	{
	if(aOperation.IsOpenSubSess())
		{
		__CACHE_PRINT(_L("++RequestAllocator::GetMessageRequest() Open sub-sess"));
		iCacheLock.Wait();
		if(iFreeHead == NULL || iFreeHead->iNext == NULL)
			{
			if(AllocRequest(2)!= KErrNone)
				{
				iCacheLock.Signal();
				return KErrNoMemory;
				}
			}
		aRequest= iFreeHead;						//get our request from free head
		iFreeHead = iFreeHead->iNext->iNext;	//set next but one as new free head read for next

		aRequest->iNext->iNext = NULL;				//seperate our request and close from free list
		CFsClientMessageRequest* CRp = aRequest->iNext;
		aRequest->iNext = NULL;
		if(iCloseHead)
			{
			CRp->iNext = iCloseHead;		//set second one as a reserved (tail) close request
			iCloseHead = CRp;
			}
		else
			iCloseHead = CRp;
		
		((CSessionFs*) aMessage.Session())->CloseRequestCountInc();
		}
	else if(aOperation.IsCloseSubSess())
		{
		__CACHE_PRINT(_L("++RequestAllocator::GetMessageRequest() Close sub-sess"));

		CFsObject* pO = SessionObjectFromHandle(aMessage.Int3(),0,reinterpret_cast<CSessionFs*>(aMessage.Session()));
		if(!pO)
			return KErrBadHandle;

		CObjPromotion* pm = (CObjPromotion*)pO;
		TInt function = aMessage.Function();
		switch(function & KIpcFunctionMask)
			{
			case EFsFormatSubClose:
				{
				if(pm->UniqueID() != Formats->UniqueID())
					{
					return KErrBadHandle;
					}
				break;
				}
			case EFsDirSubClose:
				{
				if(pm->UniqueID() != Dirs->UniqueID())
					{
					return KErrBadHandle;
					}
				break;
				}
			case EFsFileSubClose:
				{
				if(pm->UniqueID() != FileShares->UniqueID())
					{
					return KErrBadHandle;
					}
				break;
				}
			case EFsRawSubClose:
				{
				if(pm->UniqueID() != RawDisks->UniqueID())
					{
					return KErrBadHandle;
					}
				break;
				}
			case EFsPluginSubClose:
				{
				if(pm->UniqueID() != FsPluginManager::iPluginConns->UniqueID())
					{
					return KErrBadHandle;
					}
				break;
				}
#ifdef SYMBIAN_F32_ENHANCED_CHANGE_NOTIFICATION					
			case EFsNotificationSubClose:
				{
				if(pm->UniqueID() != FsNotificationManager::iNotifyRequests->UniqueID())
					{
					return KErrBadHandle;
					}
				break;
				}
#endif //SYMBIAN_F32_ENHANCED_CHANGE_NOTIFICATION					
			default:
				Fault(ECloseSubBadMessage);
			}

		iCacheLock.Wait();
		aRequest = iCloseHead;
		iCloseHead = aRequest->iNext;
		((CSessionFs*) aMessage.Session())->CloseRequestCountDec();
		aRequest->iNext = NULL;

		if((function & KIpcFunctionMask)!= EFsPluginSubClose &&
			(function & KIpcFunctionMask)!= EFsNotificationSubClose)
			{
			aRequest->SetDrive(&TheDrives[((CFsDispatchObject*)pO)->DriveNumber()]);
			}

		aRequest->SetScratchValue((TUint)pO);
		}
	else
		{
		__CACHE_PRINT(_L("++RequestAllocator::GetMessageRequest() "));
		iCacheLock.Wait();
		if(!iFreeHead)
			{
			if(AllocRequest(1) != KErrNone)
				{
				iCacheLock.Signal();
				return KErrNoMemory; 
				}
			}
		aRequest = iFreeHead;						
		iFreeHead = aRequest->iNext;
		aRequest->iNext= NULL;
		}

	aRequest->Init();

	iCacheLock.Signal();

	// pre-allocate any TParse objects needed by this request
	if (aRequest->AllocParseObjects(aOperation) != KErrNone)
		{
		aRequest->Free();
		return KErrNoMemory; 
		}

	__CACHE_PRINT1(_L("--RequestAllocator::GetMessageRequest() allocated %08x"), aRequest);

	return KErrNone;
	}	


#if defined(_USE_CONTROLIO) || defined(_DEBUG) || defined(_DEBUG_RELEASE)
TInt RequestAllocator::CloseCount()
	{TInt count=0;
	CFsClientMessageRequest* list=iCloseHead;
	while(list!=NULL)
		{
		count++;
		list=list->iNext;
		}
	return(count);
	} 
TInt RequestAllocator::FreeCount()
	{
	TInt count=0;
	CFsClientMessageRequest* list=iFreeHead;
	while(list!=NULL)
		{
		count++;
		list=list->iNext;
		}
	return(count);}
#endif

TInt RequestAllocator::AllocOperation()
//
//	Allocates a group of TMsgOperation objects
//
// Must be called with iCacheLock held
	{
    TInt i;
	if(iAllocNumOperation < KMaxOperationAllocated)
		{
		__CACHE_PRINT(_L("RequestAllocator::AllocOperation() Not reached the limit"));
		TMsgOperation* list;
		TMsgOperation* start;
		list = new TMsgOperation[KAllocReqBlock];
		start = list;
		if(!list)
			return KErrNoMemory;
		
		for(TInt j=0; j<KAllocReqBlock; j++)
			{
			TMsgOperation* request = &list[j];
			request->iIsAllocated = EFalse;
			}

		iAllocNumOperation += KAllocReqBlock;
		TMsgOperation* last;
		for(i=1;i<KAllocReqBlock;i++)
			{
			last = list;
			list++;
			last->iNext = list;
			}
		list->iNext = iFreeHeadSupOp;
		iFreeHeadSupOp = start;
		return KErrNone;
		}
	else
		{
		__CACHE_PRINT(_L("RequestAllocator::AllocOperation() Limit exceeded"));
		TMsgOperation* request;

		request=new TMsgOperation;
		if(!request)
			return KErrNoMemory;
#if defined(_USE_CONTROLIO) || defined(_DEBUG) || defined(_DEBUG_RELEASE)
		iAllocated++;
#endif
		request->iIsAllocated = ETrue;
		request->iNext=iFreeHeadSupOp;
		iFreeHeadSupOp=request;

		return KErrNone;
		}
	}
TInt RequestAllocator::GetOperation(TMsgOperation* &aOperation)
//
//	tries to get a pre allocated subop from the cache. Failing that allocates one individualy 
//	
	{

	__CACHE_PRINT(_L("RequestAllocator::GetOperation() "));
	iCacheLock.Wait();
	if(!iFreeHeadSupOp)
		{
		if(AllocOperation() != KErrNone)
			{
			iCacheLock.Signal();
			return KErrNoMemory; 
			}
		}
	aOperation = iFreeHeadSupOp;						
	iFreeHeadSupOp = aOperation->iNext;
	aOperation->iNext = aOperation->iPrev = NULL;

	iCacheLock.Signal();
	return KErrNone;
	}	

void RequestAllocator::FreeOperation(TMsgOperation* aOperation)
//
// free Operation
//
	{
	if(aOperation->iIsAllocated)
		{
		__CACHE_PRINT(_L("RequestAllocator::FreeOperation() Allocated subop"));
		delete(aOperation);
#if defined(_USE_CONTROLIO) || defined(_DEBUG) || defined(_DEBUG_RELEASE)
		iAllocated--;
#endif
		return;
		}
	
	__CACHE_PRINT(_L("RequestAllocator::FreeOperation() returning to free list"));
	iCacheLock.Wait();
	aOperation->iNext = iFreeHeadSupOp;	// NB backward link only used when request in in use
	iFreeHeadSupOp = aOperation;

	iCacheLock.Signal();
	}


CFsRequest::CFsRequest()
//
//
//
	: iOperation(NULL),iDriveNumber(KDriveInvalid)
	{}

CFsRequest::~CFsRequest()
//
//
//
	{}

void CFsRequest::Set(const TOperation& aOperation,CSessionFs* aSession)
//
//
//
	{

	SetState(EReqStateInitialise);

	iOperation     = const_cast<TOperation*>(&aOperation);
	iSession       = aSession;
	iIsCompleted   = aOperation.IsCompleted();
	iError         = KErrNone;
	iDriveNumber   = KDriveInvalid;
	iCurrentPlugin = NULL;
	iOwnerPlugin   = NULL;
	iDirectToDrive = EFalse;
	iClientThreadId= 0;
	iFlags &= ~(EFreeChanged | EPostInterceptEnabled | EPostOperation | EFsObjectOpen);
	iScratchValue = 0;
	}

void CFsRequest::Set(CSessionFs* aSession)
//
//
//
	{
	__ASSERT_DEBUG(iOperation,Fault(EBaseRequestSet1));

	SetState(EReqStateInitialise);

	iSession       = aSession;
	iIsCompleted   = iOperation->IsCompleted();
	iError         = KErrNone;
	iDriveNumber   = KDriveInvalid;
	iCurrentPlugin = NULL;
	iOwnerPlugin   = NULL;
	iDirectToDrive = EFalse;
	iClientThreadId= 0;
	iFlags &= ~(EFreeChanged | EPostInterceptEnabled | EPostOperation | EFsObjectOpen);
	iScratchValue = 0;
	}


TParse& CFsRequest::Src()
//
//
//
	{
	Fault(EBaseRequestSrc);
	return(dummyP);
	}

TParse& CFsRequest::Dest()
//
//
//
	{
	Fault(EBaseRequestDest);
	return(dummyP);
	}

TDrive* CFsRequest::Drive()
//
//
//
	{
	Fault(EBaseRequestDrive);
	return(NULL);
	}

TDrive* CFsRequest::SubstedDrive()
//
//
//
	{
	Fault(EBaseRequestSubstedDrive);
	return(NULL);
	}

void CFsRequest::SetDrive(TDrive* /*aDrive*/)
//
//
//
	{
	Fault(EBaseRequestSetDrive);
	}

void CFsRequest::SetSubstedDrive(TDrive* /*aDrive*/)
//
//
//
	{
	Fault(EBaseRequestSetSubstedDrive);
	}

TInt CFsRequest::GetSlot(TFsPluginRequest::TF32ArgType aType)
	{
	if(iOperation->Arg(0) == aType) return 0;
	if(iOperation->Arg(1) == aType) return 1;
	if(iOperation->Arg(2) == aType) return 2;
	if(iOperation->Arg(3) == aType) return 3;
	
	return(-1);
	}


TBool CFsMessageRequest::IsNotifierSpecific() const
	{
#ifndef SYMBIAN_F32_ENHANCED_CHANGE_NOTIFICATION
	return EFalse;
#else
	TInt function = iOperation->iFunction;
	return(	function == EFsNotificationAdd      || 
			function == EFsNotificationBuffer ||
			function == EFsNotificationCancel ||
			function == EFsNotificationOpen ||
			function == EFsNotificationRemove ||
			function == EFsNotificationRequest ||
			function == EFsNotificationSubClose);
#endif //SYMBIAN_F32_ENHANCED_CHANGE_NOTIFICATION
	}


TBool CFsMessageRequest::IsNotifierSupported() const
	{
#ifndef SYMBIAN_F32_ENHANCED_CHANGE_NOTIFICATION
	return EFalse;
#else
	TInt function = iOperation->iFunction;
	return(	function == EFsDelete || 
			function == EFsRmDir || 
			function == EFsMkDir || 
			function == EFsFormatNext || 
			function == EFsFileCreate ||
			function == EFsFileReplace ||
			function == EFsFileRename ||
			function == EFsRename ||
			function == EFsReplace ||
			function == EFsSetDriveName ||
			function == EFsSetVolume ||
			function == EFsSetEntry ||
			function == EFsFileSetAtt ||
			function == EFsFileSet ||
			function == EFsMountFileSystem ||
			function == EFsDismountFileSystem ||
			function == EFsFileSetSize ||
			function == EFsFileWrite ||
			function == EFsFileWriteDirty ||
			function == EFsRawDiskWrite ||
			function == EFsMountFileSystemScan);
#endif //SYMBIAN_F32_ENHANCED_CHANGE_NOTIFICATION
	}


TInt CFsRequest::Read(TFsPluginRequest::TF32ArgType aType, TInt& aVal)
	{
	if(iOperation->Arg(0) == aType) { aVal = Message().Int0(); return KErrNone; }
	if(iOperation->Arg(1) == aType) { aVal = Message().Int1(); return KErrNone; }
	if(iOperation->Arg(2) == aType) { aVal = Message().Int2(); return KErrNone; }
	if(iOperation->Arg(3) == aType) { aVal = Message().Int3(); return KErrNone; }
	
	return KErrNotSupported;
	}


TInt CFsRequest::Read(TFsPluginRequest::TF32ArgType aType, TUint& aVal)
	{
	if(iOperation->Arg(0) == aType) { aVal = (TUint)Message().Int0(); return KErrNone; }
	if(iOperation->Arg(1) == aType) { aVal = (TUint)Message().Int1(); return KErrNone; }
	if(iOperation->Arg(2) == aType) { aVal = (TUint)Message().Int2(); return KErrNone; }
	if(iOperation->Arg(3) == aType) { aVal = (TUint)Message().Int3(); return KErrNone; }
	
	return KErrNotSupported;
	}

TInt CFsRequest::Read(TFsPluginRequest::TF32ArgType aType, TInt64& aVal)
	{
	TPckg<TInt64> pkVal(aVal);
	// EFsFileLock, EFsFileUnLock, EFsFileSeek and EFsFileSetSize need special treatment
	if(iOperation->Arg(0) == aType)
		{
		if((iOperation->Function()==EFsFileLock)   ||
		   (iOperation->Function()==EFsFileUnLock)   )
			{
			if(IsDescData(KMsgPtr0))
				Read(KMsgPtr0,pkVal);
			else
				aVal = MAKE_TINT64(0, Message().Int0());	// Position is unsigned value
			return KErrNone;
			}
		
		if((iOperation->Function()==EFsFileSeek)    ||		// Seek offset (input paramater)
		   (iOperation->Function()==EFsFileSize)    ||		// Size (output paramater)
		   (iOperation->Function()==EFsFileSetSize)   )		// Size to be set (input paramater)
			{
			if(IsDescData(KMsgPtr0))
				Read(KMsgPtr0,pkVal);
			else
				aVal = Message().Int0();	// Seek offset / Size is signed value
			return KErrNone;
			}
		
		aVal = Message().Int0();
		return KErrNone; 
		}
	
	// EFsFileLock and EFsFileUnLock need special treatment
	if(iOperation->Arg(1) == aType) 
		{
		if((iOperation->Function()==EFsFileLock)   ||
		   (iOperation->Function()==EFsFileUnLock)   )
			{
			if(IsDescData(KMsgPtr1))
				Read(KMsgPtr1,pkVal);
			else
				aVal = MAKE_TINT64(0, Message().Int1());	// Length is unsigned value
			return KErrNone; 
			}
		
		aVal = Message().Int1();
		return KErrNone; 
		}
	
	// EFsFileRead, EFsFileWrite, EFsFileSeek and EFsReadFileSection need special treatment
	if(iOperation->Arg(2) == aType) 
		{
		if((iOperation->Function()==EFsFileRead)  ||
		   (iOperation->Function()==EFsFileWrite)   )
			{
			if(IsDescData(KMsgPtr2))
				Read(KMsgPtr2,pkVal);
			else
				{
				if(Message().Int2() == (TInt)I64LOW(KCurrentPosition64))
					aVal = KCurrentPosition64;				// Position is KCurrentPosition64 (-1)
				else
					aVal = MAKE_TINT64(0,Message().Int2());	// Position is unsigned value
				}
			return KErrNone; 
			}
		
		if(iOperation->Function()==EFsFileSeek)
			{
			if(IsDescData(KMsgPtr2))
				Read(KMsgPtr2,pkVal);
			else
				aVal = Message().Int2();	// New position is signed value
			return KErrNone;
			}
		
		if(iOperation->Function()==EFsReadFileSection)
			{
			if(IsDescData(KMsgPtr2))
			Read(KMsgPtr2,pkVal);
			else
			aVal = MAKE_TINT64(0,Message().Int2()); // Position is unsigned value
			return KErrNone;
			}
		
		aVal = Message().Int2();		
		return KErrNone; 
		}
	
	if(iOperation->Arg(3) == aType) 
		{
		aVal = Message().Int3();
		return KErrNone;
		}
	
	return KErrNotSupported;
	}

void CFsRequest::SetAndOpenScratchValue(const TInt64& aValue)
	{
	if (IsFsObjectOpen())
		{
		((CFsDispatchObject*) I64LOW(iScratchValue))->Close();
		SetFsObjectOpen(EFalse);
		}
	if (I64LOW(aValue) && iOperation && (iOperation->iFlags & EFileShare))
		{
		((CFsDispatchObject*) I64LOW(aValue))->Open();
		SetFsObjectOpen(ETrue);
		}
	iScratchValue = aValue;
	}


TInt CFsRequest::Read(TFsPluginRequest::TF32ArgType aType, TDes8& aDes, TInt aOffset)
	{
	TInt slot = GetSlot(aType);
	if(slot >= 0)
		return Read(slot, aDes, aOffset);
	return KErrNotSupported;
	}


TInt CFsRequest::Read(TFsPluginRequest::TF32ArgType aType, TDes16& aDes, TInt aOffset)
	{
	TInt slot = GetSlot(aType);
	if(slot >= 0)
		return Read(slot, aDes, aOffset);
	return KErrNotSupported;
	}


TInt CFsRequest::Write(TFsPluginRequest::TF32ArgType aType, const TDesC8& aDes, TInt aOffset)
	{
	TInt slot = GetSlot(aType);
	if(slot >= 0)
		return Write(slot, aDes, aOffset);
	return KErrNotSupported;
	}


TInt CFsRequest::Write(TFsPluginRequest::TF32ArgType aType, const TDesC16& aDes, TInt aOffset)
	{
	TInt slot = GetSlot(aType);
	if(slot >= 0)
		return Write(slot, aDes, aOffset);
	return KErrNotSupported;
	}


TInt CFsRequest::Read(const TInt aMsgPtr,TDes8 &aDes)
	{
	if (Message().Handle() == KLocalMessageHandle)
		{
		TDesC8* pDes = (TDesC8*) ((const RLocalMessage*) &Message())->Arg(aMsgPtr);
		if (aDes.MaxLength() < pDes->Length())
			return KErrTooBig;
		aDes.Copy(*pDes);
		return KErrNone;
		}

	return Message().Read(aMsgPtr,aDes,0);
	}


TInt CFsRequest::Read(const TInt aMsgPtr,TDes8 &aDes,TInt anOffset)
	{
	if (Message().Handle() == KLocalMessageHandle)
		{
		TDesC8* pDes = (TDesC8*) ((const RLocalMessage*) &Message())->Arg(aMsgPtr);
		if (aDes.MaxLength() < pDes->Length() + anOffset)
			return KErrTooBig;
		aDes.SetLength(pDes->Length() + anOffset);
		aDes.MidTPtr(anOffset).Copy(*pDes);
		return KErrNone;
		}

	return Message().Read(aMsgPtr,aDes,anOffset);
	}


TInt CFsRequest::Read(const TInt aMsgPtr,TDes16 &aDes)
	{
	if (Message().Handle() == KLocalMessageHandle)
		{
		TDesC16* pDes = (TDesC16*) ((const RLocalMessage*) &Message())->Arg(aMsgPtr);
		if (aDes.MaxLength() < pDes->Length())
			return KErrTooBig;
		aDes.Copy(*pDes);
		return KErrNone;
		}

	return Message().Read(aMsgPtr,aDes,0);
	}


TInt CFsRequest::Read(const TInt aMsgPtr,TDes16 &aDes,TInt anOffset)
	{
	if (Message().Handle() == KLocalMessageHandle)
		{
		TDesC16* pDes = (TDesC16*) ((const RLocalMessage*) &Message())->Arg(aMsgPtr);
		if (aDes.MaxLength() < pDes->Length() + anOffset)
			return KErrTooBig;
		aDes.SetLength(pDes->Length() + anOffset);
		aDes.MidTPtr(anOffset).Copy(*pDes);
		return KErrNone;
		}

	return Message().Read(aMsgPtr,aDes,anOffset);
	}


TInt CFsRequest::Write(const TInt aMsgNum,const TDesC8 &aDes)
	{
	if (Message().Handle() == KLocalMessageHandle)
		{
		TDes8* pDes = (TDes8*) ((const RLocalMessage*) &Message())->Arg(aMsgNum);
		if (pDes->MaxLength() < aDes.Length())
			return KErrTooBig;
		pDes->Copy(aDes);
		return KErrNone;
		}

	return Message().Write(aMsgNum,aDes,0);
	}


TInt CFsRequest::Write(const TInt aMsgNum,const TDesC8 &aDes,TInt anOffset)
	{
	if (Message().Handle() == KLocalMessageHandle)
		{
		TDes8* pDes = (TDes8*) ((const RLocalMessage*) &Message())->Arg(aMsgNum);
		if (pDes->MaxLength() < aDes.Length() + anOffset)
			return KErrTooBig;
		pDes->SetLength(aDes.Length() + anOffset);
		pDes->MidTPtr(anOffset).Copy(aDes);
		return KErrNone;
		}

	return Message().Write(aMsgNum,aDes,anOffset);
	}


TInt CFsRequest::Write(const TInt aMsgNum,const TDesC16 &aDes)
	{
	if (Message().Handle() == KLocalMessageHandle)
		{
		TDes16* pDes = (TDes16*) ((const RLocalMessage*) &Message())->Arg(aMsgNum);
		if (pDes->MaxLength() < aDes.Length())
			return KErrTooBig;
		pDes->Copy(aDes);
		return KErrNone;
		}

	return Message().Write(aMsgNum,aDes,0);
	}


TInt CFsRequest::Write(const TInt aMsgNum,const TDesC16 &aDes,TInt anOffset)
	{
	if (Message().Handle() == KLocalMessageHandle)
		{
		TDes16* pDes = (TDes16*) ((const RLocalMessage*) &Message())->Arg(aMsgNum);
		if (pDes->MaxLength() < aDes.Length() + anOffset)
			return KErrTooBig;
		pDes->SetLength(aDes.Length() + anOffset);
		pDes->MidTPtr(anOffset).Copy(aDes);
		return KErrNone;
		}

	return Message().Write(aMsgNum,aDes,anOffset);
	}


void CFsRequest::ReadL(const TInt aMsgPtr,TDes8 &aDes)
	{ User::LeaveIfError(Read(aMsgPtr,aDes)); }

void CFsRequest::ReadL(const TInt aMsgPtr,TDes8 &aDes,TInt anOffset)
	{ User::LeaveIfError(Read(aMsgPtr,aDes,anOffset)); }

void CFsRequest::ReadL(const TInt aMsgPtr,TDes16 &aDes)
	{ User::LeaveIfError(Read(aMsgPtr,aDes)); }

void CFsRequest::ReadL(const TInt aMsgPtr,TDes16 &aDes,TInt anOffset)
	{ User::LeaveIfError(Read(aMsgPtr,aDes,anOffset)); }

void CFsRequest::WriteL(const TInt aMsgNum,const TDesC8 &aDes)
	{ User::LeaveIfError(Write(aMsgNum,aDes)); }

void CFsRequest::WriteL(const TInt aMsgNum,const TDesC8 &aDes,TInt anOffset)
	{ User::LeaveIfError(Write(aMsgNum,aDes,anOffset)); }

void CFsRequest::WriteL(const TInt aMsgNum,const TDesC16 &aDes)
	{ User::LeaveIfError(Write(aMsgNum,aDes)); }

void CFsRequest::WriteL(const TInt aMsgNum,const TDesC16 &aDes,TInt anOffset)
	{ User::LeaveIfError(Write(aMsgNum,aDes,anOffset)); }


/**
    @param  aMsgNum message argument index 
    @return client side descriptor length 
*/
TInt CFsRequest::GetDesLength(const TInt aMsgNum)
	{
	if (Message().Handle() == KLocalMessageHandle)
		{
		TDes8* pDes = (TDes8*) ((const RLocalMessage*) &Message())->Arg(aMsgNum);
		return pDes->Length();
		}
	else
		{
		return Message().GetDesLength(aMsgNum);
		}
	}

const RMessage2& CFsRequest::Message()
//
//
//
	{
	Fault(EBaseRequestMessage);
	return(dummyM);
	}



void CFsMessageRequest::Set(const RMessage2& aMessage,CSessionFs* aSession)
//
// For reuseable request
//
	{
	iMessage=aMessage;
	iDrive=NULL;
	iSubstedDrive=NULL;
	CFsRequest::Set(aSession);
	SetFreeChanged(EFalse);
	EnablePostIntercept(ETrue);
	}

	
void CFsMessageRequest::Set(const RMessage2& aMessage,const TOperation& aOperation,CSessionFs* aSession)
//
//
//
	{
	iCurrentPlugin=NULL;
	iMessage=aMessage;
	iDrive=NULL;
	iSubstedDrive=NULL;
	CFsRequest::Set(aOperation,aSession);
	SetFreeChanged(EFalse);
	EnablePostIntercept(ETrue);
	}

void CFsMessageRequest::Set(const TOperation& aOperation)
	{
	iOperation=const_cast<TOperation*>(&aOperation);
	}

void CFsMessageRequest::Process()
//
// Process the request - (passing to a plugin or a drive thread)
//
	{
	__THRD_PRINT3(_L("CFsMessageRequest::Process() req %08x state %d plugin 0x%x"), this, iReqState, iCurrentPlugin);
	
	// initialise request - if not initialised already
	if (iReqState == EReqStateInitialise)
		{
		TInt r = DoInitialise();
		if (r == EReqActionComplete)
			{
			return;
			}
		else if (r == EReqActionBusy)	// request postponed ?
			{
			SetState(EReqStateInitialise);
			Dispatch(EFalse);
			return;
			}
		else if (r == KErrNone && iCurrentPlugin)		// dispatch to plugin thread ?
			{
			Dispatch(EFalse);
			return;
			}
		}

	if(iCurrentPlugin)
		{
		if (IsPostOperation())
			{
			ProcessPostOperation();
			return;
			}
			
		if(!IsPluginSpecific())
			{
			ProcessPreOperation();
			return;
			}
		}

	
	// Is there a PostInitialise function ?
	if (iReqState == EReqStatePostInitialise)
		{
		TInt r = PostInitialise();
		if (r == EReqActionComplete)
			{
			return;
			}
		else if (r == EReqActionBusy)	// request postponed ?
			{
			SetState(EReqStatePostInitialise);
			Dispatch(EFalse);
			return;
			}
		}

	ProcessDriveOperation();
	}
	
void CFsMessageRequest::ProcessPostOperation()
//
// Process the message in post operation mode (handled by the current plugin)
//
	{
	TInt err = KErrNone;
	if(!iCurrentPlugin->IsPluginThread(*this))
		{
		// The request hasn't come from this plugin so it's safe to dispatch		
		TFsPluginRequest request(this);
		TRAPD(leaveValue, err = iCurrentPlugin->DoRequestL(request));
		if(leaveValue != KErrNone)
			{
			Panic(KFsClient,leaveValue);
			if(iOperation->IsOpenSubSess())		
				RequestAllocator::OpenSubFailed(Session());
			Free();
			return;
			}

		if(!IsExpectedResult(err))
			{
			Complete(err);
			return;
			}
		}
	
	// Find the previous plugin in the chain and dispatch
	//  - If no more plugins are interested in this message, then complete
	FsPluginManager::PrevPlugin(iCurrentPlugin, this, ETrue);
	if(iCurrentPlugin == NULL)
		{
		Complete(GetError());
		return;
		}

	Dispatch();
	return;
	}
	

void CFsMessageRequest::ProcessPreOperation()
//
// Process the message in pre operation mode (handled by the current plugin)
//
	{
	TInt err = KErrNone;
	if(!iCurrentPlugin->IsPluginThread(*this))
		{
		// The request hasn't come from this plugin so it's safe to dispatch		
		TFsPluginRequest request(this);		
		TRAPD(leaveValue, err = iCurrentPlugin->DoRequestL(request));
		__PLUGIN_PRINT1(_L("PLUGIN: CFsMessageRequest:: %x processed by plugin"), this);

		if((iOperation->Function() == EFsDismountPlugin) && (err !=  KErrPermissionDenied))
			{
			TRAP(leaveValue, err = iOperation->DoRequestL(this));
			}

		if(leaveValue != KErrNone)
			{
			Panic(KFsClient,leaveValue);
			if(iOperation->IsOpenSubSess())		//this should be close subsession
				RequestAllocator::OpenSubFailed(Session());	//need a close subsession fail
			Free();
			return;
			}
		}

	if(err == KErrNone)
		{
		// Find the next plugin in the chain and dispatch
		//  - If no more plugins are interested in this message, 
		//	  then Dispatch() will process the request in drive/main thread context.
		FsPluginManager::NextPlugin(iCurrentPlugin, this,(TBool)ETrue);
		if(iCurrentPlugin && IsPostOperation())
			SetPostOperation(EFalse);
		Dispatch();
		return;
		}
	// KErrCompletion may be returned by the plugin to 
	// indicate that it has process the message itself (do post-intercept)
	else if (err == KErrCompletion)
		{
		// Find the previous plugin in the chain and dispatch
		//  - If no more plugins are interested in this message, then complete
		FsPluginManager::PrevPlugin(iCurrentPlugin, this,(TBool)ETrue);
		if(iCurrentPlugin != NULL)
			{
			SetPostOperation(ETrue);
			err = KErrNone;
			Dispatch();
			return;
			}
		else
			{
			err = KErrNone;	
			}
		}
		
	Complete(err);
	return;
	}


void CFsMessageRequest::ProcessDriveOperation()
//
// Process the message in drive (or main thread) context
//
	{

	// A new request is to be processed - kick off the inactivity/finalisation timer...
	FsThreadManager::StartFinalisationTimer(iDriveNumber);

	TInt err = KErrNone;
	
	TRAPD(leaveValue, err = iOperation->DoRequestL(this));

	// Cancel hung state if a request from the drive thread has finished
	FsThreadManager::SetDriveHung(DriveNumber(), EFalse);

	if(leaveValue != KErrNone)
		{
		Panic(KFsClient,leaveValue);
		if(iOperation->IsOpenSubSess()) 
			RequestAllocator::OpenSubFailed(Session());
		Free();
		return;
		}
		
	// request postponed ? (e.g. if a page used by file cache is busy,
	// or because of fair scheduling).
	if (err == EReqActionBusy)
		{
		Dispatch();
		return;
		}

	iLastError = err;

	if(!IsExpectedResult(err) || IsPluginSpecific())
		{
		// no need to call DoNotify here since that requires err==KErrNone
		Complete(err);
		return;
		}

	SetError(err);


	// Start issuing the post-operation requests starting from the bottom of the chain
	iCurrentPlugin = NULL;
	if (PostInterceptEnabled())
		{
		FsPluginManager::PrevPlugin(iCurrentPlugin, this,(TBool)ETrue);
		if(iCurrentPlugin && !iCurrentPlugin->IsPluginThread(*this))
			{
			SetPostOperation(ETrue);
			if (DispatchToPlugin())
				return;
			}
		}		

	Complete(GetError());
	return;
	}

void CFsMessageRequest::Complete(TInt aError)
//
//
//
	{
	__THRD_PRINT2(_L("----- CFsMessageRequest::Complete() req %08x with %d"), this, aError);

	if (aError==KErrNoMemory)
		{
		if (iDrive)	// Not all message requests are associated with a drive!
			{
			TDriveInfo di;
			iDrive->DriveInfo(di);
			if (di.iType == EMediaRam)
				aError = KErrNoMemory;
			}
		}
	if(aError!=KErrNone)
		{
		if(iOperation->IsOpenSubSess())
			RequestAllocator::OpenSubFailed(Session());
		}

	iLastError = aError;

	// Call the current MessageOp's completion routine - if this returns EReqActionComplete, 
	// then pop the request and call the next MessageOps's completion routine.
	// Do this until there are no more MessageOp's on the stack.
	//
	// Completion return codes:
	// EReqActionOwnedByPlugin:			DON'T dispatch message again, DON'T complete message 
	// EReqActionComplete:				DON'T dispatch message again, DO complete message
	// EReqActionContinue:				DO dispatch message again, DON'T complete message
	// EReqActionBusy					DO dispatch message again, DON'T complete message, 
	TInt completeErr;

	for (completeErr = EReqActionComplete; CurrentOperationPtr() != NULL && completeErr == EReqActionComplete; )
		{
		// Get the completion routine from the current TMsgOperation and then pop it
		TFsRequestFunc doComplete = CurrentOperation().iComplete;
		PopOperation();
		
		// Check that a completion routine is supplied if this isn't the last TMsgOperation
		__ASSERT_ALWAYS(iCurrentOperation == NULL || doComplete != NULL, Fault(EBadOperationIndex));

		// Call the completion routine
		if (doComplete)
			completeErr = doComplete(this);

		// Check return code is valid
		__ASSERT_DEBUG(completeErr == EReqActionContinue || completeErr == EReqActionBusy || completeErr == EReqActionComplete || completeErr == EReqActionOwnedByPlugin, Fault(EBadOperationCompletionCode));
		}

	// a set of flags to determine what actions to take for each return code
	enum 
		{
		EDispatch			= 0x01,	// dispatch request
		EComplete			= 0x02,	// complete request
		EFree				 =0x04,	// free request
		EResetPostInitialised	= 0x08,	// call iPostInitialise again
		EDispatchToFront	= 0x10	// dispatch to back of drive thread queue
		};
	const TUint actions[8] = 
		{
		// 0 - EReqActionContinue
		EDispatch,											
		// 1 - unused
		0,
		// 2 - unused
		0,
		// 3 - unused
		0,
		// 4 - EReqActionCompleteAndDispatch
		EDispatch | EComplete | EResetPostInitialised | EDispatchToFront,
		// 5 - EReqActionOwnedByPlugin
		0,													
		// 6 - EReqActionBusy
		EDispatch | EResetPostInitialised,					
		// 7 - EReqActionComplete
		EComplete | EFree,
		};

	TUint actionFlags = actions[((TUint32) completeErr) & 0x00000007];

	// To dispatch to drive thread and intercept before DoRequestL() 
	// we must re-call iPostInitialise(), so reset iPostInitialised flag
	if (actionFlags & EResetPostInitialised)
		{
		// can only postpone the request if there's a PostInitialise() function
		__ASSERT_DEBUG(iOperation->iPostInitialise , Fault(EBadOperationCompletionCode));
		SetState(EReqStatePostInitialise);
		}



	TInt dispatchError = KErrNone;
	TBool completeMessage = (iLastError != KErrNone) || (iIsCompleted && (actionFlags & EComplete));

	// Need to lock drive to prevent both file server and drive thread from completing the same message (!)
	// or the dispatched thread from freeing the message before the first thread has completed it
	TDrive* drive = iDrive;
	if (drive) 
		drive->Lock();

	// don't go back up plugin chain once message has been completed
	if (completeMessage)
		EnablePostIntercept(EFalse);

	if (actionFlags & EDispatch)
		{
		__ASSERT_DEBUG(((actionFlags & EFree) == 0), Fault(EInvalidCompletionFlags));
		__ASSERT_DEBUG (iCurrentPlugin == NULL, Fault(EInvalidMsgState));
		dispatchError = DispatchToDrive(EFalse, actionFlags & EDispatchToFront);
		}

	
	if ((actionFlags & EComplete) && IsExpectedResult(iLastError) && !IsPluginSpecific() && !IsNotifierSpecific())
		DoNotify(KErrNone);


	if (completeMessage)
		{
		iIsCompleted = EFalse;
		TInt msgHandle = Message().Handle();
		if ((msgHandle != KLocalMessageHandle) && (msgHandle != 0))
			Message().Complete(iLastError);
		if (IsPluginRequest())
			CFsPlugin::Complete(this, iLastError);
		}

	if(iOperation->Function() == EFsFileSubClose)
		{
		HBufC* pFileName = (HBufC*) I64HIGH(ScratchValue64());
		if(pFileName)
			{
			delete pFileName;
			SetScratchValue(NULL);
			}
		}

	if (drive) 
		drive->UnLock();

	if (actionFlags & EFree)
		Free();

	// if the dispatch failed, then continue with popping MessageOps from the stack.
	if (dispatchError != KErrNone)
		Complete(dispatchError);
	}

void CFsMessageRequest::DoNotify(TInt aError)
//
//
//
	{
	__PRINT1(_L("----- CFsMessageRequest::DoNotify() with %d"),aError);

	TInt driveNumber = DriveNumber();

	if(aError==KErrNone)
		{
		if(!(FsNotify::IsChangeQueEmpty(driveNumber)))
			FsNotify::HandleChange(this,driveNumber);	
		if ((driveNumber != KDriveInvalid) && !(FsNotify::IsDiskSpaceQueEmpty(driveNumber)))
			FsNotify::HandleDiskSpace(this, DriveNumber());
	
#ifdef SYMBIAN_F32_ENHANCED_CHANGE_NOTIFICATION
		if 	(iOperation->iFunction == EFsFileWrite)
			{
			CFileShare* share = (CFileShare*) this->ScratchValue();
			CFileCB* file = &share->File();
			CFileCache* fileCache = file->FileCache();
			
			// Manage notifications for write with no cache or a write-through
			if (!fileCache || !fileCache->IsDirty())
				{
				FsNotificationManager::HandleChange((CFsClientMessageRequest&)*this);
				}
			}
		else if((iOperation->iFunction == EFsFileWriteDirty) && FsNotificationManager::IsInitialised())
			{
			CFileShare* share;
			CFileCB* file;
			GetFileFromScratch(this, share, file);

			TFileName path;
			path.Append(file->DriveNumber() + 'A');
			path.Append(':');
			path.Append(file->FileName().Des());
			
			// Manage notifications for write with caching enabled
			FsNotificationManager::HandleChange((CFsClientMessageRequest*)this, path, TFsNotification::EFileChange);
			}
		else if(IsNotifierSupported())
			{
			FsNotificationManager::HandleChange((CFsClientMessageRequest&)*this);
			}
#endif //SYMBIAN_F32_ENHANCED_CHANGE_NOTIFICATION
		}
	}


void CFsMessageRequest::Free()
//
//
//
	{
	__THRD_PRINT1(_L("CFsMessageRequest::Free() isAllocated=%d"), IsAllocated());

	SetScratchValue(0);	// this should close the CFsObject

	if(!IsAllocated())
		delete(this);
	else iOperation = NULL;

	}

TInt CFsMessageRequest::DoInitialise()
//
// returns KErrNone if Initialise succeeded normally
// or      EReqActionComplete if request completed already
// or      EReqActionBusy if request needs dispatching & initialising again
//
	{
	TInt r = KErrNone;

	SetState(iOperation->iPostInitialise ? EReqStatePostInitialise : EReqStateDoRequest);
	r=Initialise();
	if(r==KErrBadHandle)
		{
		// bad subsession handle so panic client
		_LIT(KPanic,"Panic");
		Panic(KPanic,r);
		if(iOperation->IsOpenSubSess())			//this should be close subsession
			RequestAllocator::OpenSubFailed(Session());	//need a close subsession fail
		Free();
		r = EReqActionComplete;
		}
	else if (r == EReqActionComplete)	// completed synchronously in Initialise() function ?
		{
		Complete(KErrNone);
		}
	else if (r == EReqActionBusy || r == EReqActionPending)	// request postponed or owned by file share ?
		{
		}
	else if (r != KErrNone)			// error 
		{
		Complete(r);
		r = EReqActionComplete;
		}

	return r;
	}

TInt CFsMessageRequest::PostInitialise()
//
// returns KErrNone if PostInitialise() succeeded normally
// or      EReqActionComplete if request completed already
// or      EReqActionBusy if request needs dispatching & initialising again
//
	{
	TInt r = KErrNone;

	SetState(EReqStateDoRequest);
	if (iOperation->iPostInitialise)
		r = iOperation->iPostInitialise(this);
	if (r == EReqActionComplete)		// completed early ?
		{

		// Start issuing the post-operation requests starting from the bottom of the chain
		iCurrentPlugin = NULL;
		if (PostInterceptEnabled())
			{
			FsPluginManager::PrevPlugin(iCurrentPlugin, this,(TBool)ETrue);
			if(iCurrentPlugin && !iCurrentPlugin->IsPluginThread(*this))
				{
				SetPostOperation(ETrue);
				Dispatch();
				return r;	// EReqActionComplete
				}
			}		

		Complete(KErrNone);
		}
	else if (r == EReqActionBusy)	// request postponed ?
		{
		}
	else if (r != KErrNone)			// error 
		{
		Complete(r);
		r = EReqActionComplete;
		}

	return r;
	}


// CFsMessageRequest::Dispatch()
//
// If aInitialise is EFalse, just disptach request to appropriate thread - 
// don't call DoInitialise() or PostInitialise()
//
void CFsMessageRequest::Dispatch(TBool aInitialise, TBool aLowPriority, TBool aDispatchToFront)
	{
	__THRD_PRINT1(_L("CFsMessageRequest::Dispatch() req %08x"), this);

	TInt r = KErrNone;

	if (iReqState == EReqStateInitialise && aInitialise)
		{
		r = DoInitialise();
		if (r == EReqActionComplete)
			{
			return;
			}
		else if (r == EReqActionBusy)		// request postponed ?
			{
			SetState(EReqStateInitialise);	// reinitialize when request is next processed
			}
		else if (r == EReqActionPending)	// owned by file share ?
			{
			SetState(EReqStateInitialise);	// reinitialize when request is next processed
			return;
			}
		if(!IsPluginSpecific() && (iOwnerPlugin == NULL))
			{
			iCurrentPlugin = NULL;
			iClientThreadId = 0;
			FsPluginManager::NextPlugin(iCurrentPlugin, this, (TBool)ETrue);

			// find out whether there is a plugin registered to post intercept this message
			CFsPlugin* postInterceptPlugin = NULL;
			if (iCurrentPlugin == NULL)
				FsPluginManager::PrevPlugin(postInterceptPlugin, this, (TBool)ETrue);

			// Save the client's thread Id for subsequent testing by CFsPlugin::IsPluginThread() - doing so on the fly 
			// is risky because some messages are completed early in which case Message().Client() will result in a panic
			if ((iCurrentPlugin || postInterceptPlugin) && Message().Handle() != NULL && Message().Handle() != KLocalMessageHandle)
				{
				RThread thread;
				Message().Client(thread, EOwnerThread);
				iClientThreadId = thread.Id();
				thread.Close();
				}
			} 
		} // initialise
		

		// dispatch to plugin thread if initialised (otherwise dispatch to drive thread)
	if (iReqState > EReqStateInitialise && DispatchToPlugin())
		{
		__PLUGIN_PRINT1(_L("PLUGIN: CFsMessageRequest %x dispatched to plugin (async)"), this);
		// The request has been delivered to the plugin thread
		//  - leave the main thread now and await asynchronous completion
		return;
		}


	// Is there a PostInitialise function ?
	if (iReqState ==  EReqStatePostInitialise && aInitialise && r == KErrNone)
		{
		TInt r = PostInitialise();
		if (r == EReqActionComplete)
			return;
		else if (r == EReqActionBusy)				// request postponed ?
			SetState(EReqStatePostInitialise);		// reinitialize when request is next processed
		}


	if(!IsSeparateThread() || FsThreadManager::IsDriveSync(DriveNumber(),EFalse))
		{
		__PLUGIN_PRINT1(_L("PLUGIN: CFsMessageRequest %x dispatched to plugin (sync)"), this);
		FsPluginManager::DispatchSync(this);
		return;
		}

	r = DispatchToDrive(aLowPriority, aDispatchToFront);
	if (r != KErrNone)
		Complete(r);
	}

inline TInt CFsMessageRequest::DispatchToDrive(TBool aLowPriority, TBool aDispatchToFront)
	{
	TInt drivenumber = DriveNumber();
	
	if (drivenumber < EDriveA || drivenumber > EDriveZ)
		return KErrNotSupported;
	
	FsThreadManager::LockDrive(drivenumber);

	CDriveThread* dT=NULL;
	TInt r = FsThreadManager::GetDriveThread(drivenumber,&dT);

	if(r == KErrNone)
		{
		CRequestThread* pT = (CRequestThread*)dT;

		if (FsThreadManager::IsDriveHung(drivenumber))
			r = KErrNotReady;
		else if (aDispatchToFront)
			pT->DeliverFront(this);
		else
			pT->DeliverBack(this, aLowPriority);
		}
	else if (r == KErrAccessDenied)
		{
		Process();
		}

	FsThreadManager::UnlockDrive(drivenumber);

	return r;
	}

void CFsMessageRequest::Dispatch()
	{
	Dispatch(ETrue);
	}

TBool CFsMessageRequest::DispatchToPlugin()
//
// Common route: Receive -> Process -> Dispatch -> DispatchToPlugin
//
	{
	TInt drivenumber = DriveNumber();
	if(iCurrentPlugin)
		{		
		FOREVER
			{
			// if we've reached a plugin which is waiting for Complete(), stop going up the plugin chain
			if(drivenumber >= 0 && IsPostOperation() && CurrentPluginWaiting())
				{
				// No more plug-ins need to process this request
				iCurrentPlugin = NULL;
				}

			while(iCurrentPlugin && iCurrentPlugin->IsPluginThread(*this))
				{
				// Skip the current plugin if the request originated from the plugin
				if(IsPostOperation())
					{
					FsPluginManager::PrevPlugin(iCurrentPlugin, this,(TBool)ETrue);
					}
				else
					{
					FsPluginManager::NextPlugin(iCurrentPlugin, this,(TBool)ETrue);
					}
				}
				
			if(iCurrentPlugin)
				{
				TFsPluginRequest request(this);
				 __THRD_PRINT1(_L("CFsMessageRequest::DispatchToPlugin() req %08x"), this);

				TInt err = iCurrentPlugin->Deliver(request);
				if(err == KErrNone)
					{

					// The request has been delivered to the plugin thread
					//  - leave the main thread now
					return(ETrue);
					}
				else if(err < KErrNone)
					{
					// An error has occurred
					//  - complete the message
					Complete(err);
					return(ETrue);
					}
				else if(err == KPluginMessageForward)
					{
					// The plugin has processed synchronously (case 1)
					//  - Pass the message on to the next plugin
					if(FsFunction() != EFsPluginOpen)
					    {
	                    FsPluginManager::NextPlugin(iCurrentPlugin, this,(TBool)ETrue);
	                    continue;
					    }
					else // FsFunction == EFsPluginOpen
					    {
					    /* 
					     * PluginOpen requests should not be passed down the plugin stack.
					     * 

					     */
					    iCurrentPlugin = NULL;
					    continue;
					    }
					}
				else if(err == KPluginMessageComplete)
					{
					// The plugin has processed synchronously (case 2)
					//  - Pass the message back up the stack
					SetPostOperation(ETrue);
					FsPluginManager::PrevPlugin(iCurrentPlugin, this,(TBool)ETrue);
					continue;
					}
				_LIT(KPanic,"Panic: F32-BAD-PLUGIN-ERROR");
				Panic(KPanic, err);
				}
			else if (IsPostOperation())
				{
				// No more plugins are interested in this request
				//	 - If in post operation, complete the request
				Complete(GetError());
				return(ETrue);
				}
			else
				{
				// No plugin is registered for pre-operation interception this message
				//	- pass the request onto the drive thread.
				return EFalse;
				}
			}			
		}

	if(iOperation->Function() == EFsDismountPlugin)
		{
		// Don't pass plugin dismounts to the drive thread
		Process();
		return(ETrue);
		}
		
	return EFalse;
	}

TDrive* CFsMessageRequest::Drive()
//
//
//
	{
	return(iDrive);
	}

TDrive* CFsMessageRequest::SubstedDrive()
//
//
//
	{
	return(iSubstedDrive);
	}

void CFsMessageRequest::SetDrive(TDrive* aDrive)
//
//
//
	{
	iDrive=aDrive;
	if(aDrive)
		{
		iDriveNumber=aDrive->DriveNumber();
		}
	}

void CFsMessageRequest::SetSubstedDrive(TDrive* aDrive)
//
//
//
	{
	iSubstedDrive=aDrive;
	}

const RMessage2& CFsMessageRequest::Message()
//
//
//
	{
	return(iMessage);
	}



void CFsMessageRequest::ReStart()
	{
	__ASSERT_ALWAYS(CurrentOperationPtr() != NULL, Fault(EBadOperationIndex));

	// restore original settings :
	TMsgOperation& currentOperation = CurrentOperation();
	TInt offset = currentOperation.iReadWriteArgs.iOffset;
	currentOperation.iReadWriteArgs.iPos-= offset;
	currentOperation.iReadWriteArgs.iTotalLength+= offset;
	currentOperation.iReadWriteArgs.iLength = currentOperation.iReadWriteArgs.iTotalLength;
	currentOperation.iReadWriteArgs.iOffset = 0;
	currentOperation.iState = 0;
	TInt function = iOperation->Function();
	__ASSERT_ALWAYS(function == EFsFileRead || function == EFsFileWrite,  Fault(EMsgRestartBadFunction));
	}


void CFsMessageRequest::SetOperationFunc(TInt aFunction)
	{
	const TOperation& oP = OperationArray[aFunction];
	Set(oP);
	// modified because some requests were set to PostInitialise. They are set to DoRequest in Dispatch anyway, so this is possibly ok?
	//__ASSERT_ALWAYS(iReqState == EReqStateDoRequest, Fault(EInvalidMsgState));
	}

TInt CFsMessageRequest::PushOperation(TFsRequestFunc aCallback, TInt aNextState, TInt aFunction)
	{
	TMsgOperation* nextOperation;
	TInt r = RequestAllocator::GetOperation(nextOperation);
	if (r != KErrNone)
		return r;

	
	// Store the caller's state etc in the CURRENT TMsgOperation (if there is one)
	if (iCurrentOperation != NULL)
		{
		__ASSERT_ALWAYS(aCallback, Fault(EPushOpNoCallback));
		iCurrentOperation->iState = aNextState;
		iCurrentOperation->iCurrentPlugin = iCurrentPlugin;
		iCurrentOperation->iFunction = iOperation->Function();
		iCurrentOperation->iNext = nextOperation;
		nextOperation->iPrev = iCurrentOperation;
		}

	// store the call-back routine in the NEW TMsgOperation
	nextOperation->iComplete = aCallback;
	nextOperation->iState = 0;
	if (aFunction != KOperationFunctionUnaltered && aFunction != iOperation->Function())
		SetOperationFunc(aFunction);


	iCurrentOperation = nextOperation;
	
	// reset post operation state: as a plugin may push more than one TMsgOperation
	SetPostOperation(EFalse);

	return KErrNone;
	}

/**
PushOperation()

Pushes a new TMsgOperation onto this request's "stack". After this function completes the new 
TMsgOperation object becomes the "current" one (pointed at by CFsMessageRequest::iCurrentOperation). 

After the request has been dispatched to the drive thread, TOperation::DoRequestL() will be called.
When this has completed, CFsMessage::Complete() is called which then pops the new TMsgOperation 
off the stack; and calls the callback routine (aCallback).

@param aPos			The position to read / write. Stored in the NEW TMsgOperation.

@param aLength		The length to read / write. Stored in the NEW TMsgOperation.

@param aData		A buffer to read the data from / write the data to. Stored in the NEW TMsgOperation.
					The buffer must belong to the file server or to a plugin

@param aOffset		The offset onto the buffer to read from / write to. Stored in the NEW TMsgOperation.

@param aCallback	An optional callback routine. Stored in the OLD TMsgOperation.
					MUST be supplied if there are one or more TMsgOperation's already on the stack

@param aNextState	State information private to the caller. Defaults to zero. Stored in the OLD TMsgOperation.

@param aFunction	An optional TFsMessage. Supplied if the caller wishes to change the TOperation
					associated with this request. When the new TMsgOperation is popped off the stack, the
					previous TOperation is restored. Used primarally for read-modify-write.
					
*/
TInt CFsMessageRequest::PushOperation(TInt64 aPos, TInt aLength, TUint8* aData, TInt aOffset, TFsRequestFunc aCallback, TInt aNextState, TInt aFunction)
	{
	TInt r = PushOperation(aCallback, aNextState, aFunction);
	if (r == KErrNone)
		CurrentOperation().Set(aPos, aLength, aData, aOffset, 0);
	return r;
	}


/**
PushOperation()

Pushes a new TMsgOperation onto this request's "stack". After this function completes the new 
TMsgOperation object becomes the "current" one (pointed at by CFsMessageRequest::iCurrentOperation). 

After the request has been dispatched to the drive thread, TOperation::DoRequestL() will be called.
When this has completed, CFsMessage::Complete() is called which then pops the new TMsgOperation 
off the stack; and calls the callback routine (aCallback).

@param aPos			The position to read / write. Stored in the NEW TMsgOperation.

@param aLength		The length to read / write. Stored in the NEW TMsgOperation.

@param aData		A buffer to read the data from / write the data to. Stored in the NEW TMsgOperation.
					The buffer belongs to a client of the file server on the other side of an IPC boundary.

@param aOffset		The offset onto the buffer to read from / write to. Stored in the NEW TMsgOperation.

@param aCallback	An optional callback routine. Stored in the OLD TMsgOperation.
					MUST be supplied if there are one or more TMsgOperation's already on the stack

@param aNextState	State information private to the caller. Defaults to zero. Stored in the OLD TMsgOperation.

@param aFunction	An optional TFsMessage. Supplied if the caller wishes to change the TOperation
					associated with this request. When the new TMsgOperation is popped off the stack, the
					previous TOperation is restored. Used primarally for read-modify-write.
					
*/
TInt CFsMessageRequest::PushOperation(TInt64 aPos, TInt aLength, TDesC8* aData, TInt aOffset, TFsRequestFunc aCallback, TInt aNextState, TInt aFunction)
	{
	TInt r = PushOperation(aCallback, aNextState, aFunction);
	if (r == KErrNone)
		CurrentOperation().Set(aPos, aLength, aData, aOffset, 0);
	return r;
	}

void CFsMessageRequest::PopOperation()
	{
	__ASSERT_ALWAYS(iCurrentOperation != NULL, Fault(EInvalidOperationIndex));
	
	TMsgOperation* currentOperation = iCurrentOperation;
	
	if (iCurrentOperation->iPrev == NULL)
		{
		iCurrentOperation = NULL;
		}
	else
		{
		TMsgOperation* prevOperation = iCurrentOperation->iPrev;
		prevOperation->iNext = NULL;
		iCurrentOperation = prevOperation;

		iCurrentPlugin = iCurrentOperation->iCurrentPlugin;
		
		if (iCurrentOperation->iFunction != iOperation->Function())
			SetOperationFunc(iCurrentOperation->iFunction);
		}

	RequestAllocator::FreeOperation(currentOperation);
	}

TMsgOperation& CFsMessageRequest::CurrentOperation()
	{
	__ASSERT_ALWAYS(iCurrentOperation != NULL, Fault(EInvalidOperationIndex));
	return *iCurrentOperation;
	}


CFsClientMessageRequest::CFsClientMessageRequest()
: iPoolSrc (0),
  iPoolDest(0)
	{
	}

// Return ETrue if there are ANY TMsgOperation's in the stack which were pushed by iCurrentPlugin
// If there are any, then it's NOT OK to dispatch this message to the plugin thread as it will be 
// waiting on a semaphore which is only cleared by Complete()
TBool CFsMessageRequest::CurrentPluginWaiting()
	{
	if (iOwnerPlugin == iCurrentPlugin)
		return ETrue;

	// find out which plugin (if any) was the last to push a TMsgOperation
	TMsgOperation* currentOperation;

	for (currentOperation = iCurrentOperation ? iCurrentOperation->iPrev : NULL;
		 currentOperation != NULL;
		 currentOperation = currentOperation->iPrev)
			{
			if (currentOperation->iCurrentPlugin == iCurrentPlugin)
				return ETrue;
			}

	return EFalse;
	}


TBool CFsMessageRequest::IsPluginRequest()
	{
	if(iOwnerPlugin) // Check the previous operation had a plugin)
		{
		return ETrue;
		}
	return EFalse;
	}


void CFsClientMessageRequest::Free()
//
//
//
	{
	__THRD_PRINT(_L("CFsClientMessageRequest::Free()"));
	__ASSERT_ALWAYS(CurrentOperationPtr() == NULL, Fault(EFreeingMsgWithMsgOp));
	if (iPoolSrc)
		{
		TParsePool::Release(iPoolSrc);
		iPoolSrc = 0;
		}

	if (iPoolDest)
		{
		TParsePool::Release(iPoolDest);
		iPoolDest = 0;
		}

	SetScratchValue(0);	// this should close the CFsObject
	iOperation = NULL;
	RequestAllocator::FreeRequest(this);
	}


TInt CFsClientMessageRequest::AllocParseObjects(const TOperation& aOperation)
	{
	__ASSERT_ALWAYS(iPoolSrc == NULL && iPoolDest == NULL, Fault(ETParsePoolGet));

	if (aOperation.iFlags & EParseSrc)
		{
		iPoolSrc = TParsePool::Get();
		if (iPoolSrc == NULL)
			return KErrNoMemory;
		}
	if (aOperation.iFlags & EParseDst)
		{
		iPoolDest = TParsePool::Get();
		if (iPoolDest == NULL)
			return KErrNoMemory;
		}
	return KErrNone;
	}

TParse& CFsClientMessageRequest::Src()
//
//
	{
	__ASSERT_ALWAYS(iPoolSrc, Fault(ETParsePoolGet));
	return (iPoolSrc->GetObject());
	}

TParse& CFsClientMessageRequest::Dest()
//
//
//
	{
	__ASSERT_ALWAYS(iPoolDest, Fault(ETParsePoolGet));
	return (iPoolDest->GetObject());
	}



TParsePool* TParsePool::iFreeHead   = 0;
TParsePool* TParsePool::iClosedHead = 0;
TInt        TParsePool::iCountFree  = 0;
RFastLock	TParsePool::iLock;


TParsePool::TParsePool()
:	iNext   (iFreeHead),
	iPrev   (0),
	iFree   (ETrue)
	{
	}

TInt TParsePool::Init()
	{
	return iLock.CreateLocal();
	}

TParsePool* TParsePool::Get()
// Gets a TParsePool object from the free list. If the free list does 
// not contain any TParsePool objects, additional objects are manufactured.
// Returns NULL if unable to allocate a new object
	{
	iLock.Wait();

	TParsePool* pObject = 0;

	// If we don't have anything in the free list, allocate some more...
	if (iCountFree < 1)
		{    
		__ASSERT_DEBUG(iFreeHead == NULL, Fault(ETParsePoolGet));
		for (TInt i = 0; i < KEBlockSize; i++)
			{
			// if allocation fails break out of the loop.
			pObject = new TParsePool;
			if (pObject == NULL)
				break;
			
			iCountFree++;
			
			if (iFreeHead)
				iFreeHead->iPrev = pObject;

			iFreeHead = pObject;
			}
		}

	// if we failed to allocate even a single object, return NULL
	if (iCountFree < 1)
		{
		iLock.Signal();
		return NULL;
		}


	__ASSERT_ALWAYS(iFreeHead != NULL, Fault(ETParsePoolGet));

	pObject = iFreeHead;
	pObject->iFree = EFalse;


	// Remove the head of the free list...
	iFreeHead = pObject->iNext;
	if (iFreeHead)
		iFreeHead->iPrev = 0;
	iCountFree--;

	// ... and add it to the closed list
	pObject->iNext = iClosedHead;
	pObject->iPrev = 0;
	if (iClosedHead)
		iClosedHead->iPrev = pObject;
	iClosedHead = pObject;

	iLock.Signal();

	return pObject;
	}


void TParsePool::Release(TParsePool* aObject)
//
// Release a sub-sequence of TParsePool objects back to the free list.
//
	{
	__ASSERT_ALWAYS(aObject != NULL && !aObject->iFree, Fault(ETParsePoolGet));

	iLock.Wait();

	aObject->iFree = ETrue;

	// Get the objects either side of the one we're interested in
	TParsePool* pPrevious = aObject->iPrev;
	TParsePool* pNext     = aObject->iNext;

	// Remove it from the closed list
	if (pPrevious)
		pPrevious->iNext = pNext;
	else
		iClosedHead = pNext;

	if (pNext)
		pNext->iPrev = pPrevious;


	// Now add it to the free list
	aObject->iNext = iFreeHead;
	aObject->iPrev = 0;
	if (iFreeHead)
		iFreeHead->iPrev = aObject;

	iFreeHead = aObject;
	iCountFree++;

	iLock.Signal();
	}


CFsInternalRequest::CFsInternalRequest()
//
//
//
	{
	// iStatus=NULL;
	// iIsAllocated=EFalse;
	}

void CFsInternalRequest::Set(const TOperation& aOperation,CSessionFs* aSession)
//
//
// 
	{
	CFsRequest::Set(aOperation,aSession);
	}

void CFsInternalRequest::Process()
//
//
//
	{
	__THRD_PRINT(_L("CFsInternalRequest::Process()"));
	TInt r=KErrNone;
	TRAPD(leaveValue,r=iOperation->DoRequestL(this));
	// internal requests should never fail
	__ASSERT_ALWAYS(leaveValue==KErrNone && (r==KErrNone||r==EReqActionBusy),Fault(EInternalRequestProcess));

	// request postponed ? (e.g. if a page used by file cache is busy).
	if (r == EReqActionBusy)
		{
		Dispatch();
		return;
		}

	Complete(r);
	}


void CFsInternalRequest::Complete(TInt aError)
//
//
//
	{
	__PRINT1(_L("CFsInternalRequest::Complete() with %d"),aError);
	TInt func = Operation()->Function();
	if(func==KCancelSession || func==KCancelPlugin || func==KFlushDirtyData)
		{
		__ASSERT_DEBUG(ThreadHandle()!=0 && !FsThreadManager::IsDisconnectThread(),Fault(EInternalRequestComplete1));
		RThread t;
		t.SetHandle(ThreadHandle());
		TRequestStatus* s=&Status();
		t.RequestComplete(s,aError);
		Free();
		}
	else if(func == KDispatchObjectClose)
		{
		TFsCloseObject::Complete(this);
		Free();
		}
	else if(func==KFileShareClose)
		{
		if (aError == EReqActionBusy)
			{
			Dispatch();
			}
		else
			{
			TFsCloseFileShare::Complete(this);
			Free();
			}
		}
	else
		Fault(EInternalRequestComplete3);
	}

void CFsInternalRequest::Dispatch()
//
//
//
	{
	__THRD_PRINT(_L("CFsInternalRequest::Dispatch()"));
	__ASSERT_ALWAYS(Initialise()==KErrNone,Fault(EInternalRequestDispatch1));

	if(iCurrentPlugin && Operation()->Function() == KCancelPlugin)
		{
		TFsPluginRequest request(this);
		TInt r = iCurrentPlugin->Deliver(request);
		__ASSERT_ALWAYS(r == KErrNone, Fault(EInternalRequestDispatchCancelPlugin));
		}
	else
		{
		TInt drivenumber = DriveNumber();
		FsThreadManager::LockDrive(drivenumber);
		// shouldn't dispath if no drive available
		__ASSERT_ALWAYS(FsThreadManager::IsDriveAvailable(drivenumber,EFalse) && !FsThreadManager::IsDriveSync(drivenumber,EFalse),Fault(EInternalRequestDispatch2));
		CDriveThread* dT=NULL;
		TInt r=FsThreadManager::GetDriveThread(drivenumber,&dT);
		__THRD_PRINT2(_L("deliver to thread 0x%x, drive number %d"),dT,drivenumber);
		__ASSERT_ALWAYS(r==KErrNone && dT,Fault(EInternalRequestDispatch3));
		CRequestThread* pT = (CRequestThread*)dT;
		TInt func = Operation()->Function();
		if(func == KDispatchObjectClose || func == KFileShareClose || func == KFlushDirtyData)
			pT->DeliverBack(this);
		else
			pT->DeliverFront(this);
		FsThreadManager::UnlockDrive(drivenumber);
		}
	}

void CFsInternalRequest::Free()
//
//
//
	{
	__THRD_PRINT1(_L("CFsInternalRequest::Free() isAllocated=%d"),IsAllocated());

	SetScratchValue(0);	// this should close the CFsObject

	if(!IsAllocated())
		delete(this);
	}

void CFsDisconnectRequest::Dispatch()
//
//
//
	{
	__THRD_PRINT(_L("CFsDisconnectRequest::Dispatch()"));
	// no need to lock
	TInt r=Initialise();
	__ASSERT_ALWAYS(r==KErrNone,Fault(EDisconnectRequestDispatch1));
	CRequestThread* pT=FsThreadManager::GetDisconnectThread();
	__ASSERT_ALWAYS(pT,Fault(EDisconnectRequestDispatch2));
	pT->DeliverBack(this);
	}

void CFsDisconnectRequest::Process()
//
//
//
	{
	__THRD_PRINT(_L("CFsDisconnectRequest::Process()"));
	TInt r=KErrNone;
	TRAPD(leaveValue,r=iOperation->DoRequestL(this));
	leaveValue=leaveValue; // just to make compiler happy
	__ASSERT_DEBUG(leaveValue==KErrNone && r==KErrNone,Fault(EDisonncectRequestProcess));
	Complete(r);
	}

void CFsDisconnectRequest::Complete(TInt aError)
//
//
//
	{
	__PRINT1(_L("CFsDisconnectRequest::Complete() with %d"),aError);
	__ASSERT_ALWAYS(aError==KErrNone,Fault(EDisconnectRequestComplete));
	// set session disconnect reqeust to NULL
	// will be freed in CFsMessageRequest::Free()
	Session()->iDisconnectRequest=NULL;
	// now delete session
	TheFileServer->SessionQueueLockWait();
	delete(Session());
	TheFileServer->SessionQueueLockSignal();
	// NB Must complete the message AFTER the session has been deleted...
	Message().Complete(aError);
	delete(this);	
	}


/**
Create a new synchronous message scheduler
*/
CFsSyncMessageScheduler* CFsSyncMessageScheduler::NewL()
	{
	__PRINT(_L("CFsSyncMessageScheduler::NewL()"));

	CFsSyncMessageScheduler* pSelf = new(ELeave)CFsSyncMessageScheduler();
	
	CleanupStack::PushL(pSelf);
	pSelf->ConstructL();
	CleanupStack::Pop();
	
	return(pSelf);
	}


/**
Construct a synchronous message scheduler
*/
CFsSyncMessageScheduler::CFsSyncMessageScheduler()
 : CActive(EPriorityHigh),
   iList(_FOFF(CFsRequest,iLink))
	{
	__PRINT(_L("CFsSyncMessageScheduler::CFsSyncMessageScheduler()"));
	}


/**
Second-phase constructor
*/

void CFsSyncMessageScheduler::ConstructL()
	{
	__PRINT(_L("CFsSyncMessageScheduler::ConstructL()"));
	
	User::LeaveIfError(iLock.CreateLocal());
	User::LeaveIfError(iThread.Open(RThread().Id()));
	
	CActiveScheduler::Add(this);
	}
	
	
/**
Synchronous message scheduler 
	- dispatch any synchronous requests that were queued via ::Dispatch
*/
void CFsSyncMessageScheduler::RunL()
	{
	__PRINT(_L(">> CFsSyncMessageScheduler::RunL()"));
	
	FOREVER
		{
		iLock.Wait();
		if(iList.IsEmpty())
			{
			break;
			}
		else
			{
			CFsRequest* request = iList.First();
			request->iLink.Deque();
			iLock.Signal();
			__PRINT1(_L("   CFsSyncMessageScheduler - Dispatching message %08x"), request);
			request->Process();
			}
		}
	
	iSignalled = EFalse;
	iStatus = KRequestPending;
	SetActive();
	iLock.Signal();
	
	__PRINT(_L("<< CFsSyncMessageScheduler::RunL()"));
	}


/**
DoCancel - Not Implemented
*/
void CFsSyncMessageScheduler::DoCancel()
	{
	__PRINT(_L("CFsSyncMessageScheduler::DoCancel"));
	}


/**
Queue synchronous requests to be processed in main thread context
	- called when synchronous messages are passed from plugin threads
*/
void CFsSyncMessageScheduler::Dispatch(CFsRequest* aRequest)
	{
	__PRINT1(_L("CFsSyncMessageScheduler::Dispatch(%08x)"), aRequest);

	// Accquire lock and add request to queue.
	iLock.Wait();
	iList.AddFirst(*aRequest);

	if(!iSignalled)
		{
		// set iSignalled in case another thread (plug-in) is calling dispatch, then release lock.
		iSignalled = ETrue;
		iLock.Signal();

		// signal main thread.
		TRequestStatus* s = &iStatus;
		iThread.RequestComplete(s, KErrNone);
		}
	else
		{
		// main thread already signalled, just release lock.
		iLock.Signal();
		}
	}


/**
Complete outstanding requests for the specified session
*/
void CFsSyncMessageScheduler::CompleteSessionRequests(CSessionFs* aSession, TInt aValue)
	{
	__PRINT2(_L("FsPluginManager::CompleteSessionRequests(%08x, %d)"), aSession, aValue);
	
	iLock.Wait();
	TDblQueIter<CFsRequest> q(iList);
	CFsRequest* pR;
	while((pR=q++)!=NULL)
		{
		if(pR->Session()==aSession)
			{
			pR->iLink.Deque();
			pR->Complete(aValue);
			}
		}
	iLock.Signal();
	}

