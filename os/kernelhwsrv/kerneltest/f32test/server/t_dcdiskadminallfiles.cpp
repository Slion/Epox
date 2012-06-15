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
// f32test\server\t_dcdiskadminallfiles.cpp
// 
//

#include <f32file.h>
#include <e32test.h>
#include <e32std.h>
#include <e32std_private.h>
#include <e32def.h>
#include <e32def_private.h>
#include <e32svr.h>
#include "t_server.h"

GLDEF_D RTest test(_L("T_DCDISKADMINALLFILES"));
GLDEF_D TTime gTimeNow;
LOCAL_D TInt gTheDriveNum;


const TInt KPathPosition = 2;
_LIT(KExpectedPrivatePath, "\\Private\\00000001\\");
_LIT(KResourcePath, "?:\\Resource\\");
_LIT(KSystemPath,	"?:\\Sys\\");
_LIT(KPrivatePath,	"?:\\Private\\");
_LIT(KPrivateFalseID,	"?:\\Private\\FFFFFFFF\\");
_LIT(KDriveName,	"Billy");
_LIT(KVolLable,		"Benny");
_LIT(KFileSys,	"systemfile.txt");
_LIT(KFileSys3, "sysfile.rna");
_LIT(KFilePri,	"privatefile.txt");
_LIT(KFilePri2,	"privatefile.tmp");
_LIT(KFilePri3,	"prifile.rna");
_LIT(KFileRes,	"resourcefile.txt");
_LIT(KFileRes3,	"resfile.rna");
_LIT(KMkDirSub,"Subdir\\");
_LIT(KOldFile,"?:\\Anyold.txt");

#ifndef __WINS__
_LIT(KWildPath, "Z:\\SYSTEM\\");   // Until Loader is modified to use /Sys/
#else
_LIT(KWildPath, "Z:\\SYS\\");
#endif
_LIT(KWildFile, "*");

TCapability TheCaps;
TBuf<4> driveBuf=_L("?:\\");
RFormat format;
TInt count;
RRawDisk rawdisk;
RFile file1;
RFile file2;
RDir	dir;

TInt r;
TBuf<40> fsname;
TBuf<40> systestname;
TBuf<40> pritestname;
TBuf<40> restestname;
TBuf<40> theprivatepath;
TBuf<40> pritestfalseidname;
TBuf<40> mkdirname;
TFileName fromTemp;

TBuf<25> sysfilename;
TBuf<30> realName;
TBuf<40> shortfilename;
TBuf<40> longfilename;

TRequestStatus aStat1;
TRequestStatus aStat2;
TRequestStatus aStat3;
TRequestStatus aStat4;

TVolumeInfo aVolInfo;
//	TDriveInfo adriveInfo;

TBuf<40> systestfile;
TBuf<40> pritestfile;
TBuf<40> restestfile;
TBuf<40> systestfile1;
TBuf<40> pritestfile1;
TBuf<40> restestfile1;

TTime testtime;
TBuf<20> oldName;

TBuf<25> temp;

TEntry entry;


LOCAL_C void systemRFsTest()
//
//	RFs test on system Directory
//
	{
	systestname=KSystemPath;
	systestname[0]=(TText)('A' + gTheDriveNum);
	
	mkdirname.Zero();
	mkdirname.Append(systestname);
	mkdirname.Append(KMkDirSub);
	r=TheFs.MkDirAll(mkdirname);	
	test(r==KErrPermissionDenied);

	TheFs.RmDir(mkdirname);
	test(r==KErrPermissionDenied);

	r=TheFs.SetSubst(systestname,EDriveO);
	test(r==KErrPermissionDenied);
	
	r=TheFs.SetSessionPath(systestname);
	test(r==KErrNone);
	
	TheFs.NotifyChange(ENotifyAll,aStat1,systestname);
	test(aStat1==KRequestPending);

	systestfile=KSystemPath;
	systestfile[0]=(TText)('A' + gTheDriveNum);
	systestfile1=systestfile;
	systestfile.Append(KFileSys);
	systestfile1.Append(KFileSys3);
	
	oldName=KOldFile;
	oldName[0]=(TText)gDriveToTest;

	r=TheFs.GetShortName(systestfile, shortfilename);
	test(r==KErrNone || r==KErrNotFound || r==KErrNotSupported);

	r=TheFs.GetLongName(systestfile1, longfilename);
	test(r==KErrNone || r==KErrNotFound || r==KErrNotSupported);

	r=file1.Create(TheFs,oldName,EFileWrite);
	test(r==KErrNone || r==KErrAlreadyExists);
	file1.Close();

	r=TheFs.Replace(oldName,systestfile);
	test(r==KErrPermissionDenied);
	
	r=TheFs.Rename(systestfile,systestfile1);
	test(r==KErrPermissionDenied);
	
	r=TheFs.Entry(systestfile1,entry);
	test(r==KErrNone || KErrNotFound);

	r=TheFs.SetEntry(systestfile1,testtime,KEntryAttNormal,KEntryAttReadOnly);
	test(r==KErrPermissionDenied);

	r=TheFs.Delete(systestfile1);
	test(r==KErrPermissionDenied);
	}

LOCAL_C void ResourceRFsTest()
//
//
//
	{
	restestname=KResourcePath;
	restestname[0]=(TText)('A' + gTheDriveNum);
	
	mkdirname.Zero();
	mkdirname.Append(restestname);
	mkdirname.Append(KMkDirSub);
	r=TheFs.MkDirAll(mkdirname);	
	test(r==KErrPermissionDenied);

	TheFs.RmDir(mkdirname);
	test(r==KErrPermissionDenied);

	// Change due to defect DEF099546 fix
	// TCB capability is required for following operation.
	r=TheFs.SetSubst(restestname,EDriveO);
	//test(r==KErrNone);
	test(r==KErrPermissionDenied);
	
	// SetSubst will fail.
	//r=TheFs.RealName(_L("O:\\File.XXX"),realName);
	//test(r==KErrNone);

	//r=TheFs.SetSubst(_L(""),EDriveO); 
	//test(r==KErrNone);

	r=TheFs.SetSessionPath(restestname);
	test(r==KErrNone);
	
	TheFs.NotifyChange(ENotifyAll,aStat4,restestname);
	test(aStat4==KRequestPending);

	restestfile=KResourcePath;
	restestfile[0]=(TText)('A' + gTheDriveNum);
	restestfile1=restestfile;
	restestfile.Append(KFileRes);
	restestfile1.Append(KFileRes3);
	
	oldName=KOldFile;
	oldName[0]=(TText)gDriveToTest;

	r=TheFs.GetShortName(restestfile, shortfilename);
	test(r==KErrNone || r==KErrNotFound || r==KErrNotSupported);

	r=TheFs.GetLongName(restestfile1, longfilename);
	test(r==KErrNone || r==KErrNotFound || r==KErrNotSupported);

	r=file1.Create(TheFs,oldName,EFileWrite);
	test(r==KErrNone || r==KErrAlreadyExists);
	file1.Close();

	r=TheFs.Replace(oldName,restestfile);
	test(r==KErrPermissionDenied);
	
	r=TheFs.Rename(restestfile,restestfile1);
	test(r==KErrPermissionDenied);
	
	r=TheFs.Entry(restestfile1,entry);
	test(r==KErrNone || r==KErrPathNotFound || r==KErrNotFound);

	r=TheFs.SetEntry(restestfile1,testtime,KEntryAttNormal,KEntryAttReadOnly);
	test(r==KErrPermissionDenied);

	r=TheFs.Delete(restestfile1);
	test(r==KErrPermissionDenied);

	}


LOCAL_C void privatefalseIDRFsTest()
//
//
//
	{
	pritestfalseidname=KPrivateFalseID;
	pritestfalseidname[0]=(TText)('A' + gTheDriveNum);
	
	mkdirname.Zero();
	mkdirname.Append(pritestfalseidname);
	mkdirname.Append(KMkDirSub);

	r=TheFs.MkDirAll(mkdirname);	
	test(r==KErrNone);

	r=TheFs.RmDir(mkdirname);	
	test(r==KErrNone);

	r=TheFs.SetSubst(pritestfalseidname,EDriveO);
	test(r==KErrNone);

	r=TheFs.RealName(_L("O:\\File.XXX"),realName);
	test(r==KErrNone);

	r=TheFs.SetSubst(_L(""),EDriveO);
	test(r==KErrNone);

	r=TheFs.SetSessionPath(pritestfalseidname);
	test(r==KErrNone);

	TheFs.NotifyChange(ENotifyAll,aStat2,pritestfalseidname);
	test(aStat2==KRequestPending);


	pritestfile=KPrivateFalseID;
	pritestfile[0]=(TText)('A' + gTheDriveNum);
	pritestfile1=pritestfile;
	pritestfile.Append(KFilePri2);
	pritestfile1.Append(KFilePri3);
	
	oldName=KOldFile;
	oldName[0]=(TText)gDriveToTest;

	r=TheFs.GetShortName(pritestfile, shortfilename);
	test(r==KErrNone || r==KErrNotFound || r==KErrNotSupported);

	r=TheFs.GetLongName(pritestfile1, longfilename);
	test(r==KErrNone || r==KErrNotFound || r==KErrNotSupported);

	r=file1.Create(TheFs,oldName,EFileWrite);
	test(r==KErrNone || r==KErrAlreadyExists);
	file1.Close();

	r=TheFs.Replace(oldName,pritestfile);
	test(r==KErrNone);
	
	r=TheFs.Rename(pritestfile,pritestfile1);
	test(r==KErrNone);
	
	r=TheFs.Entry(pritestfile1,entry);
	test(r==KErrNone);

	r=TheFs.SetEntry(pritestfile1,testtime,KEntryAttNormal,KEntryAttReadOnly);
	test(r==KErrNone);

	r=TheFs.Delete(pritestfile1);
	test(r==KErrNone);

	}


LOCAL_C void privateRFsTest()
//
//
//
	{
	//private
	pritestname=KPrivatePath;
	pritestname[0]=(TText)('A' + gTheDriveNum);
	
	mkdirname.Zero();
	mkdirname.Append(pritestname);
	mkdirname.Append(KMkDirSub);

	r=TheFs.MkDirAll(mkdirname);	
	test(r==KErrNone);

	r=TheFs.RmDir(mkdirname);	
	test(r==KErrNone);

	r=TheFs.SetSubst(pritestname,EDriveO);
	test(r==KErrNone);

	r=TheFs.RealName(_L("O:\\File.XXX"),realName);
	test(r==KErrNone);

	r=TheFs.SetSubst(_L(""),EDriveO);
	test(r==KErrNone);

	r=TheFs.SetSessionPath(pritestname);
	test(r==KErrNone);

	TheFs.NotifyChange(ENotifyAll,aStat2,pritestname);
	test(aStat2==KRequestPending);


	pritestfile=KPrivatePath;
	pritestfile[0]=(TText)('A' + gTheDriveNum);
	pritestfile1=pritestfile;
	pritestfile.Append(KFilePri2);
	pritestfile1.Append(KFilePri3);
	
	oldName=KOldFile;
	oldName[0]=(TText)gDriveToTest;

	r=TheFs.GetShortName(pritestfile, shortfilename);
	test(r==KErrNone || r==KErrNotFound || r==KErrNotSupported);

	r=TheFs.GetLongName(pritestfile1, longfilename);
	test(r==KErrNone || r==KErrNotFound || r==KErrNotSupported);

	r=file1.Create(TheFs,oldName,EFileWrite);
	test(r==KErrNone || r==KErrAlreadyExists);
	file1.Close();

	r=TheFs.Replace(oldName,pritestfile);
	test(r==KErrNone || r==KErrNotFound);
	
	r=TheFs.Rename(pritestfile,pritestfile1);
	test(r==KErrNone);
	
	r=TheFs.Entry(pritestfile1,entry);
	test(r==KErrNone);

	r=TheFs.SetEntry(pritestfile1,testtime,KEntryAttNormal,KEntryAttReadOnly);
	test(r==KErrNone);

	r=TheFs.Delete(pritestfile1);
	test(r==KErrNone);

	}


LOCAL_C void privateSIDRFstest()
	{
	theprivatepath[0]=(TText)gDriveToTest;	
	test.Printf(_L("the Private Path = %S"),&theprivatepath);

	mkdirname.Zero();
	mkdirname.Append(theprivatepath);
	mkdirname.Append(KMkDirSub);
	r=TheFs.MkDirAll(mkdirname);	
	test(r==KErrNone);

	r=TheFs.RmDir(mkdirname);	
	test(r==KErrNone);

	r=TheFs.SetSubst(theprivatepath,EDriveO);	
	test(r==KErrNone);

	r=TheFs.RealName(_L("O:\\File.XXX"),realName);
	test(r==KErrNone);

	r=TheFs.SetSubst(_L(""),EDriveO);	
	test(r==KErrNone);

	r=TheFs.SetSessionPath(theprivatepath);
	test(r==KErrNone);

	TheFs.NotifyChange(ENotifyAll,aStat3,theprivatepath);
	test(aStat3==KRequestPending);

	pritestfile=theprivatepath;
	pritestfile[0]=(TText)('A' + gTheDriveNum);
	pritestfile1=pritestfile;
	pritestfile.Append(KFilePri2);
	pritestfile1.Append(KFilePri3);
	
	oldName=KOldFile;
	oldName[0]=(TText)gDriveToTest;

	r=TheFs.GetShortName(pritestfile, shortfilename);
	test(r==KErrNone || r==KErrNotFound || r==KErrNotSupported);

	r=TheFs.GetLongName(pritestfile1, longfilename);
	test(r==KErrNone || r==KErrNotFound || r==KErrNotSupported);

	r=file1.Create(TheFs,oldName,EFileWrite);
	test(r==KErrNone || r==KErrAlreadyExists);
	file1.Close();

	r=TheFs.Replace(oldName,pritestfile);
	test(r==KErrNone);
	
	r=TheFs.Rename(pritestfile,pritestfile1);
	test(r==KErrNone || r==KErrAlreadyExists);
	
	r=TheFs.Entry(pritestfile1,entry);
	test(r==KErrNone);

	r=TheFs.SetEntry(pritestfile1,testtime,KEntryAttNormal,KEntryAttReadOnly);
	test(r==KErrNone);

	r=TheFs.Delete(pritestfile1);
	test(r==KErrNone);
	}


LOCAL_C void systemRFiletest()
//
//RFile testing with session path set to //system//
//
	{

	r=TheFs.SetSessionPath(systestname);
	test(r==KErrNone);

	r=file1.Temp(TheFs,systestname,fromTemp,EFileWrite);
	test(r==KErrPermissionDenied);
	file1.Close();
	
	systestfile=systestname;
	systestfile.Append(KFileSys);
	r=file1.Create(TheFs,systestfile,EFileWrite);
	test(r==KErrPermissionDenied);
	file1.Close();

	r=file1.Open(TheFs,systestfile,EFileWrite);
	test(r==KErrPermissionDenied);

	r=file1.Open(TheFs,systestfile,EFileRead);
	test(r==KErrNone|| r==KErrNotFound);
	
	r=file1.Replace(TheFs,systestfile,EFileWrite);
	test(r==KErrPermissionDenied);
	
    TFindFile finder(TheFs);
    CDir* dir = NULL;
    r=finder.FindWildByDir(KWildFile, KWildPath, dir);
	if (!(r==KErrNone))
        test.Printf(_L("T_DCDISKADMINALLFILES: test find wildcards r = %d (expected KErrNone)\n"), r);
	test(r==KErrNone);
	delete dir;
	}

LOCAL_C void resourceRFiletest()
//
//RFile testing with session path set to //resource//
//
	{
	r=TheFs.SetSessionPath(restestname);
	test(r==KErrNone);

	r=file1.Temp(TheFs,restestname,fromTemp,EFileWrite);
	test(r==KErrPermissionDenied);
	file1.Close();

	r=file1.Create(TheFs,KFileRes,EFileWrite);
	test(r==KErrPermissionDenied);
	file1.Close();

	r=file1.Open(TheFs,KFileRes,EFileWrite|EFileShareExclusive );
	test(r==KErrPermissionDenied);
	
	r=file1.Open(TheFs,KFileRes,EFileShareReadersOnly);
	test(r==KErrNone || KErrPathNotFound || KErrNotFound);
	file1.Close();

	r=file1.Open(TheFs,KFileRes,EFileShareReadersOrWriters|EFileRead);
	test(r==KErrNone || KErrPathNotFound || KErrNotFound);
	file1.Close();

	r=file1.Open(TheFs,KFileRes,EFileShareReadersOrWriters|EFileWrite);
	test(r==KErrPermissionDenied);
	file1.Close();
	
	r=file1.Replace(TheFs,KFileRes3,EFileWrite);
	test(r==KErrPermissionDenied);
	file1.Close();
	}


LOCAL_C void privatefalseIDRFiletest()
//
//RFile testing with session path set to //Private//false ID
//
	{
	r=TheFs.SetSessionPath(pritestfalseidname);
	test(r==KErrNone);

	r=file1.Temp(TheFs,pritestfalseidname,fromTemp,EFileWrite);
	test(r==KErrNone);
	file1.Close();

	r=file1.Create(TheFs,KFilePri,EFileWrite);
	test(r==KErrNone);
	file1.Close();

	r=file1.Open(TheFs,KFilePri,EFileWrite);
	test(r==KErrNone);
	file1.Close();
	
	r=file1.Open(TheFs,KFilePri,EFileRead);
	test(r==KErrNone);
	file1.Close();
	
	r=file1.Replace(TheFs,KFilePri,EFileWrite);
	test(r==KErrNone);
	
	r=file1.Rename(KFilePri3);
	test(r==KErrNone);
	file1.Close();
	}


LOCAL_C void privateRFiletest()
//
//RFile testing with session path set to //Private//
//
	{
	r=TheFs.SetSessionPath(pritestname);
	test(r==KErrNone);

	r=file1.Temp(TheFs,pritestname,fromTemp,EFileWrite);
	test(r==KErrNone);
	file1.Close();

	r=file1.Create(TheFs,KFilePri,EFileWrite);
	test(r==KErrNone);
	file1.Close();

	r=file1.Open(TheFs,KFilePri,EFileWrite);
	test(r==KErrNone);
	file1.Close();
	
	r=file1.Open(TheFs,KFilePri,EFileRead);
	test(r==KErrNone);
	file1.Close();

	r=file1.Replace(TheFs,KFilePri,EFileWrite);
	test(r==KErrNone);
	
	r=file1.Rename(KFilePri3);
	test(r==KErrNone);
	file1.Close();
	}


LOCAL_C void privateSIDRFiletest()
//
//Rfile Testing with session path set to //Private//UID//
//
	{
	r=TheFs.SetSessionToPrivate(gTheDriveNum);
	test(r==KErrNone);
	
	r=file1.Temp(TheFs,theprivatepath,fromTemp,EFileWrite);
	test(r==KErrNone);
	file1.Close();

	r=file1.Create(TheFs,KFilePri,EFileWrite);
	test(r==KErrNone);
	file1.Close();

	r=file1.Open(TheFs,KFilePri,EFileWrite);
	test(r==KErrNone);
	file1.Close();

	r=file1.Open(TheFs,KFilePri,EFileRead);
	test(r==KErrNone);
	file1.Close();


	r=file1.Replace(TheFs,KFilePri,EFileWrite);
	test(r==KErrNone);
	
	r=file1.Rename(KFilePri3);
	test(r==KErrNone);
	file1.Close();
	}

LOCAL_C void RDirtest()
//
//
//
	{

	//system
	CDir*	dirEntries;
	TBuf<30> dirNameBuf(KSystemPath);
	dirNameBuf[0]=(TText)gDriveToTest;
	r=dir.Open(TheFs,dirNameBuf,KEntryAttNormal);
	test(r==KErrNone || r==KErrPathNotFound || r==KErrNotFound);
	dir.Close();
	if(r==KErrNone)
		{
		r=TheFs.GetDir(dirNameBuf,KEntryAttMatchMask,ESortByName,dirEntries);
		test(r==KErrNone);
		delete dirEntries;
		}
	dirNameBuf.Zero();
	//Private//falseID
	dirNameBuf=KPrivateFalseID;
	dirNameBuf[0]=(TText)gDriveToTest;
	r=dir.Open(TheFs,dirNameBuf,KEntryAttNormal);
	test(r==KErrNone);
	dir.Close();
	r=TheFs.GetDir(dirNameBuf,KEntryAttMatchMask,ESortByName,dirEntries);
	test(r==KErrNone);
	dirNameBuf.Zero();
	delete dirEntries;
	//Private
	dirNameBuf=KPrivatePath;
	dirNameBuf[0]=(TText)gDriveToTest;
	r=dir.Open(TheFs,dirNameBuf,KEntryAttNormal);
	test(r==KErrNone);
	dir.Close();
	r=TheFs.GetDir(dirNameBuf,KEntryAttMatchMask,ESortByName,dirEntries);
	test(r==KErrNone);
	dirNameBuf.Zero();
	delete dirEntries;
	//Private/uid
	TheFs.PrivatePath(dirNameBuf);
	dirNameBuf.Insert(0,_L("?:"));
	dirNameBuf[0]=(TText)gDriveToTest;
	r=dir.Open(TheFs,dirNameBuf,KEntryAttNormal);
	test(r==KErrNone);
	dir.Close();
	r=TheFs.GetDir(dirNameBuf,KEntryAttMatchMask,ESortByName,dirEntries);
	test(r==KErrNone);
	dirNameBuf.Zero();
	delete dirEntries;
	//Resource
	dirNameBuf=KResourcePath;
	dirNameBuf[0]=(TText)gDriveToTest;
	r=dir.Open(TheFs,dirNameBuf,KEntryAttNormal);
	test(r==KErrNone || r==KErrPathNotFound);
	if(r==KErrNone)
		{
		r=TheFs.GetDir(dirNameBuf,KEntryAttMatchMask,ESortByName,dirEntries);
		test(r==KErrNone);
		dir.Close();
		delete dirEntries;
		}
	}


LOCAL_C void TestDiskAdminAllfiles()
//
//	Test with root capabilities
//
	{
	r=TheFs.FileSystemName(fsname,gTheDriveNum);
	test(r==KErrNone);	
	r = DismountFileSystem(TheFs, fsname, gTheDriveNum);
	test(r==KErrNone);
//	r=TheFs.RemoveFileSystem(fsname);	//can not test due to bug else where fix exists
//	test(r==KErrNone);
//	r=TheFs.AddFileSystem(fsname);
//	test(r==KErrNone);
	r = MountFileSystem(TheFs, fsname, gTheDriveNum);
	test(r==KErrNone);
	r=TheFs.SetDriveName(gTheDriveNum,KDriveName);
	test(r==KErrNone);
#ifndef __WINS__
 	r=TheFs.SetVolumeLabel(KVolLable, gTheDriveNum);
	test(r==KErrNone);
#endif

	systemRFsTest();
	ResourceRFsTest();
	privateRFsTest();	
	privateSIDRFstest();
	privatefalseIDRFsTest();

	systemRFiletest();
	resourceRFiletest();
	privateRFiletest();
	privateSIDRFiletest();
	privatefalseIDRFiletest();

	//check notifier return values set in rfs testing
	//User::WaitForRequest(aStat1);
	User::WaitForRequest(aStat2);
	User::WaitForRequest(aStat3);
	//User::WaitForRequest(aStat4);
	test(aStat2==KErrNone);
	test(aStat3==KErrNone);
		
	test(aStat1 == KRequestPending);
	TheFs.NotifyChangeCancel(aStat1);
	test(aStat1==KErrCancel);
	
	test(aStat4 == KRequestPending);
	TheFs.NotifyChangeCancel(aStat4);
	test(aStat4==KErrCancel);

	r=TheFs.SetSessionPath(systestname);
	test(r==KErrNone);
	
//Test RRawDisk class
	r=rawdisk.Open(TheFs,gTheDriveNum);
	test(r==KErrPermissionDenied);
	rawdisk.Close();

	RDirtest();

#ifdef __WINS__
	if (User::UpperCase(driveBuf[0]) != 'C')
#endif
		{
		//Test RFormat class
		r=format.Open(TheFs,driveBuf,EHighDensity,count);
		test(r==KErrNone);
		while(count)	
			{
			TInt r=format.Next(count);
			test(r==KErrNone);
			}
		format.Close();
		}

	driveBuf[0]=(TText)gDriveToTest;
	r=TheFs.ScanDrive(driveBuf);
	test(r==KErrNone || r==KErrNotSupported || KErrInUse);
	r=TheFs.CheckDisk(driveBuf);
	test(r==KErrNone || r==KErrNotSupported || KErrInUse);
	}

LOCAL_C void TestCaps()
//
//	test format etc that require certain capabilities
//
	{
#ifndef __REMOVE_PLATSEC_DIAGNOSTIC_STRINGS__
	const char myDiagMsg[] = "Capability Check Failure";
#endif //__REMOVE_PLATSEC_DIAGNOSTIC_STRINGS__
	r=RProcess().HasCapability(ECapabilityAllFiles, __PLATSEC_DIAGNOSTIC_STRING(myDiagMsg));
	test(r);
	r=RProcess().HasCapability(ECapabilityDiskAdmin, __PLATSEC_DIAGNOSTIC_STRING(myDiagMsg));
	test(r);

	driveBuf[0]=(TText)gDriveToTest;
	r=TheFs.SessionPath(temp);
	test(r==KErrNone);

	test.Printf(_L("Session path: %S"),&temp);

	r=TheFs.CreatePrivatePath(gTheDriveNum);
	test(r==KErrNone || r== KErrAlreadyExists);

	TBuf<18> tempPri;
	r=TheFs.PrivatePath(tempPri);
	test(r==KErrNone);
	theprivatepath = _L("?:");
	theprivatepath.Append(tempPri);

	TestDiskAdminAllfiles();
	
	TFileName thesessionpath;
	r=TheFs.SetSessionToPrivate(gTheDriveNum);
	test(r==KErrNone);
	r=TheFs.SessionPath(thesessionpath);
	test(r==KErrNone);
	test(thesessionpath == theprivatepath);
	}


LOCAL_C void CleanupL()
//
// Clean up tests
//
	{
	test.Next(_L("Delete test directory"));
	CFileMan* fMan=CFileMan::NewL(TheFs);
	TInt r=fMan->RmDir(gSessionPath);
	test(r==KErrNone || KErrPathNotFound);
	delete fMan;
	}

GLDEF_C void CallTestsL(/*TChar aDriveLetter*/)
//
// Do all tests
//
	{
	if( !PlatSec::IsCapabilityEnforced(ECapabilityAllFiles) ||
		!PlatSec::IsCapabilityEnforced(ECapabilityDiskAdmin))
		{
		test.Printf(_L("Capabilities insufficient - leaving t_dcdiskadminallfiles"));
		test.Printf(_L("\n")); // Prevent overwrite by next print
		return;
		}
	TurnAllocFailureOff();
	TheFs.CharToDrive(gDriveToTest,gTheDriveNum);
	
	TBuf<30> sesspath;
	sesspath=_L("?:\\");
	sesspath[0] = (TText)gDriveToTest;

	TInt r= TheFs.SetSessionPath(sesspath);
	test(r==KErrNone);

	TBuf<2> cmd;
	cmd.SetLength(1);
	cmd[0] = (TText)gDriveToTest;
	RProcess tp;
	r=tp.Create(_L("clean_prepdc.exe"),sesspath);
	test(r==KErrNone);
	{
	TRequestStatus ps;
	tp.Logon(ps);
	tp.Resume();
	tp.Close();
	User::WaitForRequest(ps);
	}

	//check double mode ie that Defpath still works	
	RFs fs1;
	RFs fs2;
	
	r=fs1.Connect();
	test(r==KErrNone);
	r=fs1.SessionPath(sesspath);
	test(r==KErrNone);
	test.Printf(_L("session1 Path=%S"),&sesspath);

	TBuf<30> privatepath;
	r=fs1.SetSessionToPrivate(gTheDriveNum);
	test(r==KErrNone);
	r=fs1.PrivatePath(privatepath);
	test(r==KErrNone);
	r=privatepath.Compare(KExpectedPrivatePath());
	test(r==0);
	r=fs1.SessionPath(sesspath);
	test(r==KErrNone);
	r=privatepath.Compare(sesspath.Mid(KPathPosition));
	test(r==0);
	r=fs1.CreatePrivatePath(gTheDriveNum);
	test(r==KErrNone);
	fs1.Close();

	r=fs2.Connect();
	test(r==KErrNone);
	r=fs2.SessionPath(sesspath);
	test(r==KErrNone);
	test.Printf(_L("session2 Path=%S"),&sesspath);
	fs2.Close();

	TestCaps();

	test.Printf(_L("No of files open=%d"), TheFs.ResourceCount());

	CleanupL();
	}
