/*
* Copyright (c) 2006-2009 Nokia Corporation and/or its subsidiary(-ies).
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
* Copied from messaging IMAP tests
*
*/


#ifndef ACTIVEWAITER_H
#define ACTIVEWAITER_H

#include <test/tefunit.h>

/**
@internalTechnology
@test
*/
class MActiveWaiterObserver
	{
	public:
		virtual void DoCancel() =0;
	};

/**
An generic active object that can be used to wait for an iStatus to be signalled.
This has a similar effect to User::WaitForRequest(), but works without
blocking the thread (which allows other active objects within the thread
to continue).  Instead, a nested active scheduler loop is used.

To use a CActiveWaiter, pass its iStatus to an asynchronous method, as usual.
Then call WaitActiveL() instead of SetActive().

The call will block until the asynchronous completes.

E.g.

void CMyClass::Foo()
{
	CActiveWaiter* waiter = new(ELeave)CActiveWaiter;

	iAsyncObj->Bar(waiter->iStatus);
	waiter->WaitActiveL();
	
	delete waiter;
}
@internalTechnology
@test
*/
class CActiveWaiter : public CActive
	{
public:
	CActiveWaiter();
	~CActiveWaiter();
	
	void WaitActiveL(TInt aExpectedStatus = KErrNone);
	void SetActiveAndCancel(MActiveWaiterObserver& aCancelObserver);
	void CancelWaitActive(MActiveWaiterObserver& aCancelObserver);

protected:
	// from CActive;
	void RunL();
	void DoCancel();
	
private:
	CActiveSchedulerWait iScheduler;
	MActiveWaiterObserver* iCancelObserver;
	};
	
#endif // ACTIVEWAITER_H
