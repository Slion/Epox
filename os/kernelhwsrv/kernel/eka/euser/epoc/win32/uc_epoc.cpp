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
// e32\euser\epoc\win32\uc_epoc.cpp
// 
//

/*
#include <e32std.h>
#include <e32std_private.h>
#include <e32wins.h>
*/

#ifdef __SYMC__

//That stuff comes mostly from estub and uc_stub.cpp
//We had to put it in there since we don't know how to do an MMP firstlib on Windows

/*
extern "C"
GLDEF_C TInt _E32Startup()
//
// Unused in the stub
//
	{
	return KErrNone;
	}



//
#include <windows.h>


int CALLBACK _E32Bootstrap(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
	{
	BootEpoc(EFalse);
	return 0;
	};

*/

#endif







/*
#if defined __SYMC__

//#include <e32cmn.h>
#include <nwdl.h>


//#include <e32hashtab.h>

//SL: Empty on FCL ?
//For now we use this for basic testing on our SYMC implementation


TInt IdleCallBack(TAny* aParam)
   {
   static TInt count=0;

   count++;

   if (count>5)
      {
      //Stop it there
      CActiveScheduler::Stop();
      return EFalse;
      }

   //Keep playing
   return ETrue;		
   }



class CBaseTest: public CBase
	{

	};


GLDEF_C void MainL()
	{

   //Testing CBase object
	CBase* other=new(ELeave) CBase();
	CleanupStack::PushL(other);
	CBase* base=new(ELeave) CBase();
	CleanupStack::PushL(base);
	CleanupStack::PopAndDestroy(2,other);
	//delete base;
	
	//Testing cleanup stack
	TRAPD(err,
	base=new(ELeave) CBase();
	CleanupStack::PushL(base);
	User::Leave(KErrCancel);
	);

	ASSERT(err==KErrCancel);

	//Testing alloc failure
   //See TProcess
	TRAP(err,
	TUint8* shouldFail=new(ELeave) TUint8[1024*1024*10];
	delete[] shouldFail;
	);

   //
	ASSERT(err==KErrNoMemory);

   //Testing alloc failure without leave
   TUint8* tooLarge=new TUint8[1024*1024*10];
   ASSERT(tooLarge==NULL);
   delete[] tooLarge;

   //Testing simple alloc of T class
   TUint8* tinyAlloc=new TUint8;
   ASSERT(tinyAlloc!=NULL);
   delete tinyAlloc;

   //TODO:
   //RHashSet<TUint32> hash;
	
   //Testing AOs
   //Install an active scheduler
   CActiveScheduler* activeScheduler = new(ELeave) CActiveScheduler;
   CActiveScheduler::Install(activeScheduler);
   CleanupStack::PushL(activeScheduler);

   CIdle* idle = CIdle::NewL(CActive::EPriorityIdle);	
   CleanupStack::PushL(idle);

   idle->Start(TCallBack(IdleCallBack,NULL));

   CActiveScheduler::Start();

   CleanupStack::PopAndDestroy(idle);
   CActiveScheduler::Install(NULL);
   CleanupStack::PopAndDestroy(activeScheduler);


	//Testing unbalanced cleanup stack
	//base=new(ELeave) CBase();
	//CleanupStack::PushL(base);
	}


GLDEF_C TInt E32Main()
	{
	//What do we do then
	//SetReturnedHandle
	
	__UHEAP_MARK;

	//CBase* base=new(ELeave) CBase();
	CBase* base=new CBase();
	delete base;

	CBaseTest* baseTest=new CBaseTest();
	delete baseTest;

	HBufC* buf=HBufC::New(10);
	delete buf;

	CArrayFix<TInt>* active=new CArrayFixFlat<TInt>(10);
	delete active;

	TUint8* test=new TUint8[1024*9];
	delete[] test;

	CTrapCleanup* cleanupStack = CTrapCleanup::New();
	if (!cleanupStack)
		{
		return KErrNoMemory;
		}





	TInt err=KErrNone;
	TRAP(err,MainL());
	
	delete cleanupStack;

	__UHEAP_MARKEND;

	return err;
	}



#include <windows.h>

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine,	int nCmdShow)
	{
	//User::InitProcess();
	//E32Main();
	//User::Exit(0);

	BootEpoc(ETrue);

	return 0;
	}

#endif
*/
