/*
* Copyright (c) 2009 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
* This component and the accompanying materials are made available
* under the terms of the License "Eclipse Public License v1.0"
* which accompanies this distribution, and is available
* at the URL "http://www.eclipse.org/legal/epl-v10.html".
*
* Initial Contributors:
* Nokia Corporation - initial contribution.
*
* Contributors:
*
* Description: 
*
*/


#include "activewaiter.h"

CActiveWaiter::CActiveWaiter()
	: CActive(CActive::EPriorityStandard)
	{
	CActiveScheduler::Add(this);
	}
	
CActiveWaiter::~CActiveWaiter()
	{ Cancel(); }


void CActiveWaiter::WaitActiveL(TInt aExpectedStatus /*=KErrNone*/)
	{ 
	ASSERT(!iScheduler.IsStarted());
	SetActive();
	
	iScheduler.Start();
	
	if(iStatus.Int() != aExpectedStatus)
		{
		User::Leave(iStatus.Int());
		}
	}

/**
Sets the active scheduler active and immidiately Cancels, calling the 
supplied cancel observer during DoCancel();
@param aCancelObserver performs and DoCancel() operations on behalf of this object
*/
void CActiveWaiter::SetActiveAndCancel(MActiveWaiterObserver& aCancelObserver)
	{
	ASSERT(!iScheduler.IsStarted());
	ASSERT(iCancelObserver == NULL);
	SetActive();
	
	iCancelObserver = &aCancelObserver;
	
	Cancel();
	}

/**
Cancels the current operation, and stops the active scheduler loop,
allowing a previous call to WaitActiveL() to return.
@param aCancelObserver performs and DoCancel() operations on behalf of this object
*/
void CActiveWaiter::CancelWaitActive(MActiveWaiterObserver& aCancelObserver)
	{ 
	ASSERT(iScheduler.IsStarted());
	ASSERT(iCancelObserver == NULL);
		
	iCancelObserver = &aCancelObserver;
	
	Cancel();
	
	ASSERT(iScheduler.CanStopNow());
	iScheduler.AsyncStop();
	}

void CActiveWaiter::RunL()
	{
	if (iScheduler.IsStarted())
		{
		ASSERT(iScheduler.CanStopNow());
		iScheduler.AsyncStop();
		}
	}

void CActiveWaiter::DoCancel()
	{
	ASSERT(iCancelObserver != NULL);
	
	iCancelObserver->DoCancel();
	iCancelObserver = NULL;
	}
