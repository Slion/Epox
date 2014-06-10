/**

*/


//#include <e32std.h>
#include <e32base.h>
#include <f32file.h>

#define TEST_SUSTAIN

TTime startTime;



TInt IdleCallBack(TAny* aParam)
	{
	static TInt count=0;

	count++;

    //Only do this on first run
	if (count==1)
		{
        startTime.HomeTime();
		//Quick file system test
		RFs fs;
		User::LeaveIfError(fs.Connect());
		CleanupClosePushL(fs);

		_LIT(KFileName,"c:\\tsymc\\tsymc.txt");
		_LIT(KFileNameToCreate,"c:\\tsymc\\created.txt");
		_LIT8(KWriteThat,"Write is working too!\n");

		RFile file;
		TInt err=file.Open(fs,KFileName,EFileRead|EFileWrite);
		if (!err)
			{
			TInt size=0;
			TInt sizeErr=file.Size(size);
			if (!sizeErr)
				{
				HBufC8* content=HBufC8::NewLC(size);
				TPtr8 ptr=content->Des();
				TInt readErr=file.Read(ptr,size);
				CleanupStack::PopAndDestroy(1,content);
				//file.Close();
				}

			//Test write
			TInt writeErr=file.Write(KWriteThat);
			file.Close();
			}

		//Test file creation
		TInt createErr=file.Create(fs,KFileNameToCreate,EFileWrite);
		if (!createErr)
			{
			TInt writeErr=file.Write(KWriteThat);
			file.Close();
			}



		CleanupStack::PopAndDestroy(1,&fs);
		}

    //Check our exit condition
    TTime time;
    time.HomeTime();
    TTimeIntervalSeconds seconds;
    User::LeaveIfError(time.SecondsFrom(startTime,seconds));

    //Just run for a few seconds
    if (seconds.Int()>5)
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