// Copyright (c) 1997-2009 Nokia Corporation and/or its subsidiary(-ies).
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

#include <f32file.h>
#include <e32test.h>
#include <e32svr.h>
#include <e32math.h>

#include "fat_utils.h"
#include "t_server.h"
#include "t_chlffs.h"

using namespace Fat_Test_Utils;

RTest test(_L("t_falsespace"));

const TInt KNumberThreads=2;
const TInt KHeapSize=0x2000;

static TInt RsrvSpaceThread(TAny* aArg);
static TInt SessCloseThread(TAny* aArg);
static void GetFreeDiskSpace(TInt64 &aFree);


TInt gCount;		//count of files used to fill up the disk
TInt gTestDrive;	//drive number of the drive currently being tested

TChar gCh;

_LIT(KBasePath,"\\F32-TST\\FILLDIR\\");
_LIT(KBaseName,"\\F32-TST\\FILLDIR\\FILE");

_LIT(KTestFile,"?:\\test.txt");
_LIT8(KTestData, "12345678123456781234567812345678123456781234567812345678123456781234567812345678123456781234567812345678123456781234567812345678");
_LIT(KDir, "?:\\adodgydir\\");
_LIT(KDrv,"?:\\");	
_LIT(KNewName,"?:\\newname.txt");


void FormatDrive()
{
    TInt nRes;

    
    #if 0
    //-- FAT32 SPC:1; for the FAT32 testing on the emulator 
    TFatFormatParam fp;
    fp.iFatType = EFat32;
    fp.iSecPerCluster = 1;

	nRes = FormatFatDrive(TheFs, gTestDrive, ETrue, &fp);	
    #else

    nRes = FormatFatDrive(TheFs, gTestDrive, ETrue);	

    #endif

    test(nRes == KErrNone);
}

void SynchronousClose(RFs &aSession)
	{
	TRequestStatus s;
	aSession.NotifyDestruction(s);
	test(s.Int()==KRequestPending);
	aSession.Close();
	User::WaitForRequest(s);
	}


static TInt CreateFileX(const TDesC& aBaseName,TInt aX, TInt aFileSize)
//
// Create a large file. Return KErrEof or KErrNone
//
	{

	TBuf<128> fileName=aBaseName;
	fileName.AppendNum(aX);
	RFile file;

	TInt r=file.Replace(TheFs,fileName,EFileWrite);
	if (r==KErrDiskFull)
		return(r);
	if (r!=KErrNone)
		{
		test.Printf(_L("ERROR:: Replace returned %d\n"),r);
		test(0);
		return(KErrDiskFull);
		}

	if (!IsTestingLFFS())
		r=file.SetSize(aFileSize);
	else
		{
    	TBuf8<1024> testdata(1024);
    	TInt count=(aFileSize/testdata.Length());
    	r=KErrNone;
    	while (count-- && r==KErrNone) 
        	r=file.Write(testdata);
		}
	if (r==KErrDiskFull)
		{
		file.Close();
		return(r);
		}
	if (r!=KErrNone)
		{
		test.Printf(_L("ERROR:: SetSize/Write returned %d\n"),r);
		test(0);
		//test.Getch();
		file.Close();
		return(KErrDiskFull);
		}

	file.Close();

	test.Printf(_L("Created file %d size %d\n"),aX,aFileSize);
	return(KErrNone);
	}

LOCAL_C TInt DeleteFileX(const TDesC& aBaseName,TInt aX)
//
// Delete a large file
//
	{
	TBuf<128> fileName=aBaseName;
	fileName.AppendNum(aX);
	return TheFs.Delete(fileName);
	}


static void FillUpDisk()
//
// Test that a full disk is ok
//
	{

	test.Start(_L("Fill disk to capacity"));
	TInt r=TheFs.MkDirAll(KBasePath);
	test(r==KErrNone || r==KErrAlreadyExists);
	gCount=0;
	TFileName sessionPath;
	r=TheFs.SessionPath(sessionPath);
	test(r==KErrNone);
	TBuf<128> fileName=KBaseName();
	
	TInt64 freespace=0;
	TInt64 freespaceBeforeScanDrive = 0;
	TInt64 freespaceAfterScanDrive = 0;
	
	do
		{
		GetFreeDiskSpace(freespace);
		TInt fillfilesize=0;
		if (I64HIGH(freespace))
			fillfilesize=KMaxTInt;
		else
			fillfilesize=I64LOW(freespace)* 7/8;

		FOREVER
			{
			TInt r=CreateFileX(fileName,gCount,fillfilesize);
			if (r==KErrDiskFull)
				{
				if(fillfilesize <= 2)
					break;
				else
					fillfilesize=fillfilesize/2;
				}
			test(r==KErrNone || r==KErrDiskFull);
			if(r==KErrNone)
				gCount++;
			}

		r=TheFs.CheckDisk(fileName);
		if (r!=KErrNone && r!=KErrNotSupported)
			{
			test.Printf(_L("ERROR:: CheckDisk returned %d\n"),r);
			test(0);

			}

		// Test that scan drive passes on a full disk
		// DEF071696 - KErrCorrupt on Scan Drive 
		GetFreeDiskSpace(freespaceBeforeScanDrive);
		test.Printf(_L("Before ScanDrive freeSpace = %08X:%08X\n"),
			I64HIGH(freespaceBeforeScanDrive), I64LOW(freespaceBeforeScanDrive));
		r = TheFs.ScanDrive(fileName);
		if (r!=KErrNone && r!=KErrNotSupported)
			{
			test.Printf(_L("ScanDrive returned %d\n"), r);
			test(0);
			}
		GetFreeDiskSpace(freespaceAfterScanDrive);
		test.Printf(_L("After ScanDrive freeSpace = %08X:%08X\n"),
			I64HIGH(freespaceAfterScanDrive), I64LOW(freespaceAfterScanDrive));
		}
		while (freespaceBeforeScanDrive != freespaceAfterScanDrive );

	gCount--;

	test.End();
	}

static void GetFreeDiskSpace(TInt64 &aFree)
//
//	Get free disk space
//
	{
	TVolumeInfo v;

	TInt r=TheFs.Volume(v,gTestDrive);
	test(r==KErrNone);
	aFree=v.iFree;
	}


static void Test1()
//
//	Test the API fundamentaly works for one session
//
	{
	test.Next(_L("Test Disk Space reserve APIs"));
	TInt r=0;
	
    FormatDrive();
	
	TInt64 free2;
	TInt64 free1; 
	TInt64 diff;

	r=TheFs.GetReserveAccess(gTestDrive);
	test(r==KErrPermissionDenied);
	
	//make sure nothing odd happens if we didnt already have access
	r=TheFs.ReleaseReserveAccess(gTestDrive);
	test(r==KErrNone);

	
	GetFreeDiskSpace(free2);

	r=TheFs.ReserveDriveSpace(gTestDrive,0x1000);
	test(r==KErrNone);

	GetFreeDiskSpace(free1);
	diff = free2 - free1;
	test(I64INT(diff) > 0xfe0 && I64INT(diff) < 0x1100); 
	
	r=TheFs.GetReserveAccess(gTestDrive);
	test(r==KErrNone);

	GetFreeDiskSpace(free1);
	TInt64 temp = free2-free1;
	test(I64INT(temp)>(-0x90) && I64INT(temp)<0x90);
	
	r=TheFs.ReleaseReserveAccess(gTestDrive);
	test(r==KErrNone);
	GetFreeDiskSpace(free1);

	diff = free2 - free1;
	test(I64INT(diff) > 0xfe0 && I64INT(diff) < 0x1100);
	
	
	//test reallocation of reserved space is possible
	r=TheFs.ReserveDriveSpace(gTestDrive,0x2000);
	test(r==KErrNone);

	//test upper limit of reserved space 
	r=TheFs.ReserveDriveSpace(gTestDrive,0x2000000);
	test(r==KErrArgument);

	r=TheFs.ReserveDriveSpace(gTestDrive,0);
	test(r==KErrNone);
	
	r=TheFs.GetReserveAccess(gTestDrive);
	test(r==KErrPermissionDenied);

	//make sure nothing odd happens if we didnt already have access
	r=TheFs.ReleaseReserveAccess(gTestDrive);
	test(r==KErrNone);
	
	r=TheFs.ReserveDriveSpace(gTestDrive,-45);
	test(r==KErrArgument);
	}


static void Test2()
//
//	Test multiple sessions, ensure the drive limit is not exceeded
//	
	{

	test.Next(_L("Test Session and total reserve limits"));
	
    FormatDrive();
	
	TInt i=0;
	TInt r=0;
	RFs sessions[17];
	TVolumeInfo v;

	//Connect Sessions
	for(i=0; i<17; i++)
		{
		r = sessions[i].Connect();
		test(r==KErrNone);
		}

	test.Next(_L("Test breaching sesson reserve limit"));
	r=sessions[0].ReserveDriveSpace(gTestDrive,0x10001);
	test(r==KErrArgument);

	//Get Volume Free Space
	r = sessions[0].Volume(v, gTestDrive);

	if(v.iFree > 0x100000)
		{
		test.Next(_L("Test breaching drive reserve limit"));

		for (i=0; i<16; i++)
			{
			r=sessions[i].ReserveDriveSpace(gTestDrive,0x10000);
			test(r==KErrNone);
			}

		//The straw
		r=sessions[16].ReserveDriveSpace(gTestDrive,0x10);
		test(r==KErrTooBig);
		}
	else
		{
		test.Printf(_L("Drive too small: breaching drive reserve limit test skipped\n"));
		test.Next(_L("Testing exhausting available drive free space instead"));

		for(i=0; (v.iFree -= 0x10000) >= 0; i++)
			{
			r=sessions[i].ReserveDriveSpace(gTestDrive,0x10000);
			test(r==KErrNone);
			}

		//The straw
		r=sessions[i].ReserveDriveSpace(gTestDrive,0x10000);
		test(r==KErrDiskFull);
		}

	//Close Sessions
	for(i=0; i<17; i++)
		{
		SynchronousClose(sessions[i]);
		}
	}

static void Test3()
//
//	Test session cleanup
//		
	{
	test.Next(_L("Test session close and clean up of resrved space"));

	FormatDrive();
	
	RFs fs1;
	RFs fs2;
	TInt64 free2(0);
	TInt64 free1(0); 
	TInt64 diff(0);
	
	TInt r=0;
	r = fs1.Connect();
	test(r==KErrNone);
	r = fs2.Connect();
	test(r==KErrNone);

	GetFreeDiskSpace(free1);

	r=fs1.ReserveDriveSpace(gTestDrive,0x10000);
	test(r==KErrNone);
	r=fs2.ReserveDriveSpace(gTestDrive,0x10000);
	test(r==KErrNone);

	GetFreeDiskSpace(free2);
	diff = free1 - free2;
	test(I64INT(diff)>0x1FBD0 && I64INT(diff)<0x21000); 

	SynchronousClose(fs1);

	GetFreeDiskSpace(free2);
	diff = free1-free2;
	test(I64INT(diff)>0xFA00 && I64INT(diff)<0x103C4); 

	r = fs1.Connect();
	test(r==KErrNone);

	GetFreeDiskSpace(free1);
	diff= free1-free2;
	test(I64INT(diff)== 0 || I64INT(diff)<0xFA0 ); 

	r=fs1.ReserveDriveSpace(gTestDrive,0x10000);
	test(r==KErrNone);

	GetFreeDiskSpace(free2);
	diff = free1 - free2;
	test(I64INT(diff)>0xFA00 && I64INT(diff)<0x103C4); 

	// Make sure no reserve space is allocated
	r=fs1.ReserveDriveSpace(gTestDrive,0);
	test(r==KErrNone);
	r=fs2.ReserveDriveSpace(gTestDrive,0);
	test(r==KErrNone);

	// Now fill up the disk
	FillUpDisk();
	
	// Should fail as there is no space
	r=fs1.ReserveDriveSpace(gTestDrive,0x10000);
	test(r==KErrDiskFull);

	SynchronousClose(fs1);
	SynchronousClose(fs2);
	}


static void Test4()
//
//	Test real out of disk space conditions i.e. properly run out of disk space and try to 
//	reserve an area etc
//	
	{
	test.Next(_L("Test Filling disk and using APIs"));

	if(IsTestingLFFS())
		{
		//-- This test is not valid for LFFS, because free space on this FS can change itself because of some 
        //-- internal FS activities
		test.Printf(_L("This test is inconsistent on LFFS\n"));
		return;
		}
	
    FormatDrive();

	RFs fs;
	TInt r=fs.Connect();
	test(r==KErrNone);
	TInt64 freeA(0);
	TInt64 freeB(0);
	RFile file;

									//start with known amount of space

	//create a single file to use for futher tests
	TBuf<20> buf;
	buf=KTestFile;
	buf[0]=(TUint16)gCh;

	r=file.Replace(fs, buf, EFileWrite);
	test(r==KErrNone);

	r=file.Write(KTestData());
	test(r==KErrNone);

	file.Close();

	r=fs.ReserveDriveSpace(gTestDrive,0x10000);		//reserve some disk space
	test(r==KErrNone);
		
	FillUpDisk();									//fill up the disk

	TVolumeInfo v;									//get disk space
	r=fs.Volume(v,gTestDrive);
	test(r==KErrNone);
	freeA=v.iFree;

	r=fs.GetReserveAccess(gTestDrive);				//get access to reserve space
	test(r==KErrNone);

	r=fs.Volume(v,gTestDrive);						//get disk space
	test(r==KErrNone);
	freeB=v.iFree;
	
	r=fs.ReleaseReserveAccess(gTestDrive);			//release reserve space
	test(r==KErrNone);
	
	test(freeA == (freeB - 0x10000));				//test difference in space is equal to the amount reserved

	r=fs.Volume(v,gTestDrive);						//get disk space
	test(r==KErrNone);
	freeB=v.iFree;
	test(freeA == freeB);							//check reading is still correct
	
	TBuf <20> dir = KDir();
	dir[0]=(TUint16)gCh;
	r=fs.MkDir(dir);
	test(r==KErrDiskFull);

	r=fs.MkDirAll(dir);
	test(r==KErrDiskFull);

	TFileName temp;
	TBuf<5> drv = KDrv();
	drv[0]=(TUint16)gCh;
	r=file.Temp(fs, drv, temp, EFileWrite);
	test(r==KErrDiskFull);

	r=file.Replace(fs, buf, EFileWrite);
	test(r==KErrDiskFull);

	r=file.Create(fs, buf, EFileWrite);
	test(r==KErrDiskFull);

	r=file.Open(fs, buf, EFileWrite);
	test(r==KErrNone);

	r=file.Write(128, KTestData());

	if ((gDriveCacheFlags & EFileCacheWriteOn) && (r == KErrNone))
		r = file.Flush();
	
	test(r==KErrDiskFull);

	r=file.SetSize(0x1000);
	test(r==KErrDiskFull);

	r=file.SetAtt(KEntryAttHidden,0); 
	test(r==KErrDiskFull);

	TTime dtime;
	r=file.SetModified(dtime); 
	test(r==KErrDiskFull);

	r=file.Set(dtime,KEntryAttHidden,0);
	test(r==KErrDiskFull);

	r=file.Rename(buf);
	test(r==KErrDiskFull);

	file.Close();


	// Test that we can create a temporary file & write to it after acquiring reserved access, 
	r=fs.GetReserveAccess(gTestDrive);				//get access to reserve space
	test(r==KErrNone);

	r=fs.Volume(v,gTestDrive);						//get disk space
	test(r==KErrNone);
	freeA = v.iFree;

	r=file.Temp(fs, drv, temp, EFileWrite);
	test(r==KErrNone);

	r = file.Write(KTestData());
	test (r == KErrNone);

	// If write caching is enabled, call RFs::Entry() to flush the file "anonymously"
	if ((gDriveCacheFlags & EFileCacheWriteOn) && (r == KErrNone))
		{
		r = file.Flush();
		test (r == KErrNone);
		}

	r=fs.Volume(v,gTestDrive);						//get disk space
	test(r==KErrNone);
	freeB = v.iFree;
	test (freeB < freeA);

	file.Close();

	r=fs.ReleaseReserveAccess(gTestDrive);			//release reserve space
	test(r==KErrNone);


	TBuf<20> newname =KNewName();
	newname[0]=(TUint16)gCh;
	r=fs.Rename(buf, newname);
	test(r==KErrDiskFull);

	r=fs.Replace(buf, newname);
	test(r==KErrDiskFull);

	r=fs.SetEntry(buf, dtime, KEntryAttHidden, 0);
	test(r==KErrDiskFull);

	r=fs.CreatePrivatePath(gTestDrive);
	test(r==KErrDiskFull);

	r=fs.SetVolumeLabel(_L("Moooo"), gTestDrive);
	test(r==KErrDiskFull);	

	r=fs.SetModified(buf, dtime);
	test(r==KErrDiskFull);	

	SynchronousClose(fs);
	}

	

static void Test5()
//
//
//
	{
	test.Next(_L("Test Session limits"));

	if(IsTestingLFFS())
		{
		//-- This test is not valid for LFFS, because free space on this FS can change itself because of some 
        //-- internal FS activities
		test.Printf(_L("This test is inconsistent on LFFS\n"));
		return;
		}


	RFs fs1;
	RFs fs2;
	TInt r=KErrNone;

	r=fs1.Connect();
	test(r==KErrNone);
	r=fs2.Connect();
	test(r==KErrNone);

	FormatDrive();

	r=fs1.ReserveDriveSpace(gTestDrive,0x10000);		
	test(r==KErrNone);

	r=fs2.ReserveDriveSpace(gTestDrive,0x10000);		
	test(r==KErrNone);

	FillUpDisk();									

	r=fs1.GetReserveAccess(gTestDrive);				
	test(r==KErrNone);

	TBuf<20> dir = KDir();
	dir[0]=(TUint16)gCh;


	r=fs2.MkDir(dir);
	test(r==KErrDiskFull);

	r=fs1.ReserveDriveSpace(gTestDrive,0); //can not release reserve space while you have reserve access
	test(r==KErrInUse);

	r=fs1.ReleaseReserveAccess(gTestDrive);				
	test(r==KErrNone);

	r=fs1.ReserveDriveSpace(gTestDrive,0); 
	test(r==KErrNone);

	r=fs2.MkDir(dir);
	test(r==KErrNone);

	SynchronousClose(fs1);
	SynchronousClose(fs2);
	}

static TInt RsrvSpaceThread(TAny* aArg)
	{
	TInt r=KErrNone;
	TInt64 fr1;
	TInt64 fr2;
	TInt64 diff;

	TVolumeInfo v;
	r=((RFs*)aArg)->Volume(v,gTestDrive);
	if(r!=KErrNone)
		return(r);

	fr1=v.iFree;

	r=((RFs*)aArg)->ReserveDriveSpace(gTestDrive,0x10000); 
	if(r!=KErrNone)
		return(r);

	r=((RFs*)aArg)->Volume(v,gTestDrive);
	if(r!=KErrNone)
		return(r);
	fr2=v.iFree;
	
	diff=fr1-fr2;
	if(!(I64INT(diff)> 0xef38 && I64INT(diff)<0xf100))
		return(KErrGeneral);
	return r;
	}

static TInt SessCloseThread(TAny* aArg)
	{
	TInt r=KErrNone;
	TInt64 fr1;
	TInt64 fr2;
	TInt64 diff;

	TVolumeInfo v;
	r=((RFs*)aArg)->Volume(v,gTestDrive);
	if(r!=KErrNone)
		return(r);
	fr1=v.iFree;

	((RFs*)aArg)->ReserveDriveSpace(gTestDrive,0x1000);
	
	r=((RFs*)aArg)->Volume(v,gTestDrive);
	if(r!=KErrNone)
		return(r);
	fr2=v.iFree;

	diff=fr2-fr1;
	if(!(I64INT(diff)> 0xef38 && I64INT(diff)<0xf100))
		return(KErrGeneral);

	SynchronousClose(*((RFs*)aArg));

	return r;
	}

static void Test6()
//
//	Test sharabale session
//
	{
	
	test.Next(_L("Test sharable session"));

	RFs fsess;
	TInt r=KErrNone;
	TInt64 free1(0);
	TInt64 free2(0);
	TInt64 diff(0);
	RThread t[KNumberThreads];
	TRequestStatus tStat[KNumberThreads];

	r=fsess.Connect();
	test(r==KErrNone);

	FormatDrive();

	r= fsess.ShareAuto();
	test(r==KErrNone);

	GetFreeDiskSpace(free1);

	fsess.ReserveDriveSpace(gTestDrive,0x1000);
		
	r = t[0].Create(_L("Sub_Thread1"),RsrvSpaceThread,KDefaultStackSize,KHeapSize,KHeapSize,&fsess); 
	test(r==KErrNone);

	t[0].Rendezvous(tStat[0]);
	t[0].Resume();

	User::WaitForRequest(tStat[0]);

	t[0].Close();
	test(tStat[0]==KErrNone);

	r = t[1].Create(_L("Sub_Thread2"),SessCloseThread,KDefaultStackSize,KHeapSize,KHeapSize,&fsess); 
	test(r==KErrNone);

	t[1].Rendezvous(tStat[1]);
	t[1].Resume();

	User::WaitForRequest(tStat[1]);

	t[1].Close();
	test(tStat[1]==KErrNone);

	GetFreeDiskSpace(free2);

	diff = free1-free2;
	test(I64INT(diff)== 0 || I64INT(diff)<0xFA0 );
	}


static void Test7()
//
// Tests notifier events for sessions with and without reserved access
//
	{
	if(IsTestingLFFS())
		{
		// This test is not valid for LFFS...
		test.Printf(_L("Test reserved access notification not run for LFFS\n"));
		return;
		}

	
	test.Next(_L("Test reserved access notification"));
	
	FormatDrive();

	RFs theNrm;
	RFs theRes;

	TInt err = theNrm.Connect();
	test(KErrNone == err);

	err = theRes.Connect();
	test(KErrNone == err);


	TInt64 freeSpace(0);
	GetFreeDiskSpace(freeSpace);

	RFs theTestSession;
	theTestSession.Connect();

	_LIT(KFileFiller, "?:\\t_falseSpaceFiller");
	TBuf<25> fileName;
	fileName = KFileFiller;
	fileName[0] = (TUint16)gCh;

	err = theTestSession.Connect();
	test(err == KErrNone);

	// determine the cluster size
	RFile theFile;
	err=theFile.Replace(theTestSession, fileName, EFileShareAny | EFileWrite);
	test(err==KErrNone);

	// Neither notifier should be triggered here
	err = theFile.SetSize(1);
	test(KErrNone == err);
	theFile.Close();

	TInt64 newFreeSpace;
	GetFreeDiskSpace(newFreeSpace);
	TInt clusterSize = TInt(freeSpace - newFreeSpace);
	theTestSession.Delete(fileName);
	GetFreeDiskSpace(newFreeSpace);
	test (newFreeSpace == freeSpace);

	TInt resSpace = Max(0x1000, clusterSize);
		
	TVolumeInfo volInfo;
	theNrm.Volume(volInfo, gTestDrive);
	test(volInfo.iFree == freeSpace);

	err = theRes.ReserveDriveSpace(gTestDrive, resSpace);
	test(KErrNone == err);
	err = theRes.GetReserveAccess(gTestDrive);
	test(KErrNone == err);

	theRes.Volume(volInfo, gTestDrive);
	test(volInfo.iFree == freeSpace);

	theNrm.Volume(volInfo, gTestDrive);
	test(volInfo.iFree == freeSpace - resSpace);


	//
	// Register the notifiers and verify that the only the "Normal"
	// and not the "Reserved" session is triggered.
	//
	TRequestStatus statNrm;
	TRequestStatus statRes;

	TInt64 threshold(freeSpace - resSpace*2);
	theNrm.NotifyDiskSpace(threshold, gTestDrive, statNrm);
	theRes.NotifyDiskSpace(threshold, gTestDrive, statRes);
	test((statNrm == KRequestPending) && (statRes == KRequestPending));


	//
	// Main part of the test starts here.
	// First we create a new file, then we increase its size to cause the
	// "Normal" notifier to trigger but not the "Reserved" notifier
	//
	err=theFile.Replace(theTestSession, fileName, EFileShareAny | EFileWrite);
	test(err==KErrNone);
	test((statNrm == KRequestPending) && (statRes == KRequestPending));

	// Neither notifier should be triggered here
	err = theFile.SetSize(resSpace);
	test(KErrNone == err);
	test((statNrm == KRequestPending) && (statRes == KRequestPending));

	// This should trigger the "Normal" notifier, but not the "Reserved" one
	err = theFile.SetSize(2*resSpace);
	test(KErrNone == err);
	test((statNrm == KErrNone) && (statRes == KRequestPending));


	//
	// Reset the "Normal" notifier then increase the amount of reserved space
	// on the drive. This should re-trigger the "Normal" notifier but leave
	// the "Reserved" notifier untouched.
	//
	theNrm.NotifyDiskSpace(threshold - resSpace, gTestDrive, statNrm);
	test((statNrm == KRequestPending) && (statRes == KRequestPending));

	err = theTestSession.ReserveDriveSpace(gTestDrive, resSpace * 3);
	if (err != KErrArgument)	// will have exceeded limit if resSpace = 32K
		{
		test(err == KErrNone);
		test((statNrm == KErrNone) && (statRes == KRequestPending));
		}

	//
	// All done - tidy up.
	//
	theFile.Close();
	theTestSession.Delete(fileName);
	theTestSession.Close();
	theNrm.Close();
	theRes.Close();
	}

LOCAL_C void TestForDEF142554()
    {
    test.Next(_L("Test for DEF142554: test RFile::Modified and RFile::Att when disk full"));
    
    Format(gTestDrive);
    
    TUint att;
    TTime time;
    
    RFs fs;
    TInt err = fs.Connect();
    test(err == KErrNone);

    RFile file;
    TBuf<20> fileName;
    fileName = KTestFile;
    fileName[0] = (TUint16)gCh;
    
    err = fs.ReserveDriveSpace(gTestDrive,0x10000); 
    test(err == KErrNone);

    err = file.Replace(fs, fileName, EFileWrite);
    test(err == KErrNone);

    err = file.Write(KTestData);
    test(err == KErrNone);
    
    err = file.Flush();
    test(err == KErrNone);
    
    file.Close();
    
    err = file.Open(fs, fileName, EFileRead);
    test(err == KErrNone);
    
    err = file.Att(att);
    test(err == KErrNone);
    
    err = file.Modified(time);
    test(err == KErrNone);
    
    file.Close();
    
    FillUpDisk();
    
    err = file.Open(fs, fileName, EFileRead);
    test(err == KErrNone);
    
    TUint att1;
    err = file.Att(att1);
    test(err == KErrNone);
    test(att1 == att);
    
    TTime time1;
    err = file.Modified(time1);
    test(err == KErrNone);
    test(time1 == time);
    
    file.Close();
    fs.Close();
    
    }


//-----------------------------------------------------------------------------

/**
    test creation of the the file that crosses 4G boundary on the FAT media

*/
static void TestFAT4G_Boundary()
	{
    const TInt64 K4Gig = 4*(TInt64)K1GigaByte;

	test.Next(_L("Test files crossing 4G boundary on FAT"));

    if(!Is_Fat32(TheFs, gTestDrive))
		{
		test.Printf(_L("This test requires FAT32. Skipping.\n"));
		return;
		}

    TVolumeInfo volInfo;
	
	TInt nRes = TheFs.Volume(volInfo,gTestDrive);
	test(nRes == KErrNone);
	
    if(volInfo.iSize < K4Gig+K1MegaByte)
		{
		test.Printf(_L("This test requires volume > 4G. Skipping.\n"));
		return;
		}
	
    //-- 1. format the volume
    FormatDrive();

    //-- find out media position of the data region start
    TFatBootSector bootSector;
    nRes = ReadBootSector(TheFs, gTestDrive, 0, bootSector);
    test(nRes == KErrNone);
    test(bootSector.IsValid());

    const TInt64 dataStartPos = bootSector.FirstDataSector() << KDefaultSectorLog2;
    const TInt64 lowRegion = K4Gig - dataStartPos - K1MegaByte; 


    //-- 2. create several empty files that take a bit less that 4gig
    //-- the drive is freshly formatted and the files will expand linearry
    _LIT(KBaseFN, "\\LargeFile");
    
    const TInt MaxDummyFiles = 5;
    const TUint32 DummyFileLen = (TUint32)(lowRegion / MaxDummyFiles);
	TInt i;
    for(i=0; i<MaxDummyFiles; ++i)
		{
        nRes = CreateFileX(KBaseFN, i, DummyFileLen); 
        test(nRes == KErrNone);
		}

    //-- 3. create a real file that crosses 4G boundary
    nRes = CreateCheckableStuffedFile(TheFs, KBaseFN, 5*K1MegaByte);
    test(nRes == KErrNone);
    
    test.Printf(_L("Verifying the file that crosses 4G boundary.\n"));

    nRes = VerifyCheckableFile(TheFs, KBaseFN);
    test(nRes == KErrNone);

	
	nRes = TheFs.Delete(KBaseFN);
	test(nRes == KErrNone);
    for(i=0; i<MaxDummyFiles; ++i)
	    {
        nRes = DeleteFileX(KBaseFN, i); 
        test(nRes == KErrNone);
		}
	}

//-----------------------------------------------------------------------------

GLDEF_C void CallTestsL()
//
// Do tests relative to session path
//
	{
    //-- set up console output 
    Fat_Test_Utils::SetConsole(test.Console()); 


	if (gSessionPath[0]=='C')	//only test on non C drives
		{
		test.Printf(_L("TEST NOT RUN FOR THIS DRIVE"));
		return;
		}
	
	if (UserSvr::DebugMask(2)&0x00000002) // TESTFAST mode set? (for automated test builds)
		if(IsTestingLFFS())
			{
			// Don't run on LFFS (to increase speed of automated testing)
			test.Printf(_L("TEST NOT RUN FOR THIS DRIVE"));
			return;
			}

	//get the number of the drive we are currently testing
	TInt r=0;
	r=RFs::CharToDrive(gSessionPath[0],gTestDrive);
    test(r==KErrNone);

	r=RFs::DriveToChar(gTestDrive,gCh);
	test(r==KErrNone);

    //-- print drive information
    PrintDrvInfo(TheFs, gTestDrive);

	Test1();	//General test for new APIs
	Test2();	//Test to ensure drive and session reserve limits are not exceeded
	Test3();
	Test4();	//test filling the drive and that each checked API fails
	Test5();
	Test6();
	Test7();
	TestForDEF142554();
	Test2();	//run this test to check reserves are being cleared correctly

	TestFAT4G_Boundary();
    
    TurnAllocFailureOff();
	}




