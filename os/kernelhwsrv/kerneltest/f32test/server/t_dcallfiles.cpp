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
//

#include <f32file.h>
#include <e32test.h>
#include <e32std.h>
#include <e32std_private.h>
#include <e32def.h>
#include <e32def_private.h>
#include <e32svr.h>
#include "t_server.h"

GLDEF_D RTest test(_L("t_dcallfiles"));
GLDEF_D TTime gTimeNow;
LOCAL_D TInt gTheDriveNum;

//_LIT(KDefPath, "\\Default\\");

const TInt KPathPosition = 2;
_LIT(KExpectedPrivatePath, "\\Private\\00000001\\");

_LIT(KResourcePath, "?:\\Resource\\");
_LIT(KSystemPath,	"?:\\Sys\\");
_LIT(KPrivatePath,	"?:\\Private\\");
_LIT(KPrivateFalseID,	"?:\\Private\\FFFFFFFF\\");
_LIT(KDriveName,	"Billy");
//_LIT(KVolLable,		"Benny");

_LIT(KFileSys,	"systemfile.txt");
//_LIT(KFileSys2,	"systemfile.tmp");
_LIT(KFileSys3, "sysfile.rna");

_LIT(KFilePri,	"privatefile.txt");
_LIT(KFilePri2,	"privatefile.tmp");
_LIT(KFilePri3,	"prifile.rna");

_LIT(KFileRes,	"resourcefile.txt");
//_LIT(KFileRes2,	"resourcefile.tmp");
_LIT(KFileRes3,	"resfile.rna");

_LIT(KMkDirSub,"Subdir\\");
_LIT(KOldFile,"?:\\Anyold.txt");
//_LIT(KNullPath, "");
//_LIT(KLFFSName, "Lffs");

_LIT(KWildPath, "Z:\\SYS\\");
_LIT(KWildFile, "*");
_LIT(KSysBinFile, "z:\\sys\\bin\\t_findcaptestfile.txt");

TInt theDrive=0;
TCapability TheCaps;
TBuf<4> driveBuf=_L("?:\\");
RFormat format;
TInt count;
RRawDisk rawdisk;
RFile file1;
RFile file2;
RDir	dir;
CDir*	dirEntries;
TInt r;
TBuf<40> fsname;
TBuf<40> systestname;
TBuf<40> pritestname;
TBuf<40> restestname;
TBuf<40> theprivatepath;
TBuf<40> pritestfalseidname;
TBuf<40> mkdirname;
TFileName fromTemp;

TBuf<40> shortfilename;
TBuf<40> longfilename;
TBuf<30> dirNameBuf;

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
TBuf<30> realName;
TTime testtime;
TBuf<20> oldName;

TBuf<25> temp;

TEntry entry;


LOCAL_C void TestPathCheck()
//
// This test case is brought in by INC054580
// (NTT Renaming sys �Cfolder on C -drive on H2 allows user to access sys -files)
//
    {
    TInt r = TheFs.Rename(_L("\\sys"), _L("\\sysbad"));
    test(r == KErrPermissionDenied);
    r = TheFs.Rename(_L("\\resource"), _L("\\resourcebad"));
    test(r == KErrPermissionDenied);
    r = TheFs.Rename(_L("\\private"), _L("\\privatebad"));
    test(r == KErrNone);
    r = TheFs.Rename(_L("\\privatebad"), _L("\\private"));
    test(r == KErrNone);
    }

LOCAL_C void systemRFstest()
//
//
//
	{
//system		
	systestname=KSystemPath;
	systestname[0]=(TText)('A' + gTheDriveNum);
	
	mkdirname.Zero();
	mkdirname.Append(systestname);
	mkdirname.Append(KMkDirSub);
	r=TheFs.MkDirAll(mkdirname);	
	test(r==KErrPermissionDenied);

	r=TheFs.RmDir(mkdirname);	
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
	test(r==KErrNone || r==KErrNotFound);	

	r=TheFs.SetEntry(systestfile1,testtime,KEntryAttNormal,KEntryAttReadOnly);
	test(r==KErrPermissionDenied);

	r=TheFs.Delete(systestfile1);
	test(r==KErrPermissionDenied);

	}


LOCAL_C void resourceRFstest()
//
//
//
	{
//resource		
	restestname=KResourcePath;
	restestname[0]=(TText)('A' + gTheDriveNum);
	
	mkdirname.Zero();
	mkdirname.Append(restestname);
	mkdirname.Append(KMkDirSub);
	r=TheFs.MkDirAll(mkdirname);	
	test(r==KErrPermissionDenied);

	TheFs.RmDir(mkdirname);
	test(r==KErrPermissionDenied);


	r=TheFs.SetSubst(restestname,EDriveO);
	test(r==KErrPermissionDenied);
	
	r=TheFs.RealName(_L("O:\\File.XXX"),realName);
	test(r==KErrNone);

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
	test(r==KErrNone || r==KErrNotFound);

	r=TheFs.SetEntry(restestfile1,testtime,KEntryAttNormal,KEntryAttReadOnly);
	test(r==KErrPermissionDenied);

	r=TheFs.Delete(restestfile1);
	test(r==KErrPermissionDenied);

	}


LOCAL_C void privatefalseIDRFstest()
//
//
//
	{
//private//false ID
	pritestfalseidname=KPrivateFalseID;
	pritestfalseidname[0]=(TText)('A' + gTheDriveNum);

	mkdirname.Zero();
	mkdirname.Append(pritestfalseidname);
	mkdirname.Append(KMkDirSub);

	r=TheFs.MkDirAll(mkdirname);	
	test(r==KErrNone );

	r=TheFs.RmDir(mkdirname);	
	test(r==KErrNone );

	r=TheFs.SetSubst(pritestfalseidname,EDriveO);
	test(r==KErrPermissionDenied); 

	r=TheFs.RealName(_L("O:\\File.XXX"),realName);
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
	test(r==KErrNone || r==KErrAlreadyExists);
	
	r=TheFs.Entry(pritestfile1,entry);
	test(r==KErrNone);

	r=TheFs.SetEntry(pritestfile1,testtime,KEntryAttNormal,KEntryAttReadOnly);
	test(r==KErrNone);

	r=TheFs.Delete(pritestfile1);
	test(r==KErrNone);

	}


LOCAL_C void privateRFstest()
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
	test(r==KErrNone );

	r=TheFs.RmDir(mkdirname);	
	test(r==KErrNone );

	r=TheFs.SetSubst(pritestname,EDriveO);
	test(r==KErrPermissionDenied); 

	r=TheFs.RealName(_L("O:\\File.XXX"),realName);
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


LOCAL_C void privateSIDRFstest()
//
//
//
	{
//private/UID

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
	test(r==KErrPermissionDenied);

	r=TheFs.RealName(_L("O:\\File.XXX"),realName);
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


	TBuf<25> sysfilename;
	sysfilename.Append(systestname);
	sysfilename.Append(KFileSys);

	r=file1.Create(TheFs,sysfilename,EFileWrite);
	test(r==KErrPermissionDenied);

	r=file1.Open(TheFs,sysfilename,EFileWrite);
	test(r==KErrPermissionDenied);

	// DEF113117
	r=file1.Open(TheFs, KSysBinFile, EFileShareReadersOnly | EFileRead | EFileReadBuffered);
	test(r==KErrNone);
	file1.Close();

	r=file1.Open(TheFs, KSysBinFile, EFileStreamText | EFileReadBuffered | EFileReadAheadOn);
	test(r==KErrNone);
	file1.Close();

	r=file1.Open(TheFs, KSysBinFile, EFileStreamText | EFileShareReadersOnly);
	test(r==KErrNone);
	file1.Close();

	r=file1.Open(TheFs,sysfilename,EFileRead);
	test(r==KErrNone || r==KErrNotFound);

	r=file1.Replace(TheFs,sysfilename,EFileWrite);
	test(r==KErrPermissionDenied);

    TFindFile finder(TheFs);
    CDir* dir = NULL;
    r=finder.FindWildByDir(KWildFile, KWildPath, dir);
	if (!(r==KErrNone))
        test.Printf(_L("T_DCALLFILES: test find wildcards r = %d (expected KErrNone)\n"), r);
	test(r==KErrNone || r==KErrNotFound);
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

	r=file1.Open(TheFs,KFileRes,EFileWrite);
	test(r==KErrPermissionDenied);
	file1.Close();

	r=file1.Open(TheFs,KFileRes,EFileRead|EFileShareReadersOnly);
	test(r==KErrNone || r==KErrNotFound);
	file1.Close();

	r=file1.Open(TheFs,KFileRes,EFileShareReadersOrWriters|EFileRead);
	test(r==KErrNone || r==KErrNotFound);
	file1.Close();

	r=file1.Open(TheFs,KFileRes,EFileShareReadersOrWriters|EFileWrite);
	test(r==KErrPermissionDenied);
	file1.Close();

	r=file1.Open(TheFs,KFileRes,EFileShareReadersOnly);
	test(r==KErrNone || r==KErrNotFound);

	r=file1.ChangeMode(EFileShareExclusive);	//this is not illegal though will prevent shared access to resource which is nit my fault but may be desirable to prevent
	test(r==KErrNone);

	//this operation is prevented as you can not open a file for write access in the resource directory
	r=file1.Rename(KFileRes3);
	test(r==KErrPermissionDenied);
	file1.Close();

	r=file1.Replace(TheFs,KFileRes,EFileWrite);
	test(r==KErrPermissionDenied);
	file1.Close();	

	}


LOCAL_C void privateFalseIDRFiletest()
//
//RFile testing with session path set to //Private//FalseID
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
	test(r==KErrNone || r== KErrAlreadyExists);
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
	test(r==KErrNone || r== KErrAlreadyExists);
	file1.Close();
	}


LOCAL_C void privateSIDRFiletest()
//
//
//
	{
//Rfile Testing with session path set to //Private//UID//

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
	test(r==KErrNone || r==KErrAlreadyExists);
	file1.Close();

	}


LOCAL_C void RDirtest()
//
//
//
	{
	//system
	dirNameBuf.Zero();
	dirNameBuf = KSystemPath;
	dirNameBuf[0]=(TText)gDriveToTest;
	r=dir.Open(TheFs,dirNameBuf,KEntryAttNormal);
	test(r==KErrNone || r==KErrNotFound);
	dir.Close();

	r=TheFs.GetDir(dirNameBuf,KEntryAttMatchMask,ESortByName,dirEntries);
	test(r==KErrNone);
	delete dirEntries;

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
	test(r==KErrNone || r==KErrNotFound);

	r=TheFs.GetDir(dirNameBuf,KEntryAttMatchMask,ESortByName,dirEntries);
	test(r==KErrNone);
	delete dirEntries;
	dir.Close();
	}





LOCAL_C void testAllFiles()
//
//	Test with only AllFiles Capability
//
	{
	r=TheFs.FileSystemName(fsname,gTheDriveNum);
	test(r==KErrNone);
	r=TheFs.DismountFileSystem(fsname,gTheDriveNum);
	test(r==KErrPermissionDenied);
//	r=TheFs.RemoveFileSystem(fsname);	//can not test due to bug else where fix exists
//	test(r==KErrPermissionDenied);
//	r=TheFs.AddFileSystem(fsname);
//	test(r==KErrPermissionDenied);
	r=TheFs.MountFileSystem(fsname,gTheDriveNum);
	test(r==KErrPermissionDenied);
	r=TheFs.SetDriveName(gTheDriveNum,KDriveName);
	test(r==KErrPermissionDenied);
//	r=TheFs.SetVolumeLabel(KVolLable, gTheDriveNum);
//	test(r==KErrNone);

	systemRFstest();
	resourceRFstest();
	privateRFstest();
	privateSIDRFstest();
	privatefalseIDRFstest();

	systemRFiletest();
	resourceRFiletest();
	privateRFiletest();
	privateSIDRFiletest();
	privateFalseIDRFiletest();

	test(aStat1 == KRequestPending);
	TheFs.NotifyChangeCancel(aStat1);
	test(aStat1==KErrCancel);
	
	test(aStat4 == KRequestPending);
	TheFs.NotifyChangeCancel(aStat4);
	test(aStat4==KErrCancel);
	
	User::WaitForRequest(aStat2);
	User::WaitForRequest(aStat3);
	
	test(aStat2==KErrNone);
	test(aStat3==KErrNone);
	

	r=TheFs.SetSessionPath(systestname);
	test(r==KErrNone);
	
//Test RRawDisk class
	r=rawdisk.Open(TheFs,gTheDriveNum);
	test(r==KErrPermissionDenied);

	r=format.Open(TheFs,driveBuf,EHighDensity,count);
	test(r==KErrPermissionDenied);

	RDirtest();

	driveBuf[0]=(TText)gDriveToTest;
	r=TheFs.ScanDrive(driveBuf);
	test(r==KErrPermissionDenied);
	r=TheFs.CheckDisk(driveBuf);
	test(r==KErrPermissionDenied);
	}

LOCAL_C void TestCaps()
//
//	test format etc that require certain capabilities
//
	{
#ifndef __REMOVE_PLATSEC_DIAGNOSTIC_STRINGS__
	const char myDiagMsg[] = "Capability Check Failure";
#endif //!__REMOVE_PLATSEC_DIAGNOSTIC_STRINGS__
	r=RProcess().HasCapability(ECapabilityAllFiles, __PLATSEC_DIAGNOSTIC_STRING(myDiagMsg));
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

	testAllFiles();

	TFileName thesessionpath;
	r=TheFs.SetSessionToPrivate(gTheDriveNum);
	test(r==KErrNone);
	r=TheFs.SessionPath(thesessionpath);
	test(r==KErrNone);
	test(thesessionpath == theprivatepath);
	}

TFileName dirName;

LOCAL_C void ScanDir(const TDesC& aName, CDirScan::TScanDirection aDirection, TInt aError)
	{
	CDirScan* scanner = NULL;
	TRAP(r, scanner = CDirScan::NewL(TheFs));
	test(r == KErrNone && scanner);

	TRAP(r, scanner->SetScanDataL(aName,KEntryAttDir,ESortByName|EAscending,aDirection));
	test(r == KErrNone);
	
	CDir *entryList=NULL;
	for (;;)
		{
		TRAP(r, scanner->NextL(entryList));
		test(r == aError);
		if (entryList==NULL)
			break;
		TInt count=entryList->Count();
		while (count--)
			{
			TEntry data=(*entryList)[count];
			TBuf<KMaxFileName> path=scanner->AbbreviatedPath();
			dirName = path;
			dirName.Append(data.iName);
			test.Printf(_L("    %S\n"),&dirName);
			}
		delete entryList;
		entryList=NULL;
		}
	delete scanner;

	}

/**
The following test, tests CFileMan and CDirScan API on folders private and sys
to confirm that any operation on these folders for any app with incorrect capability
returns KErrPermissionDenied. This test step was added as a result of DEF051428
("PlatSec: Incorrect errors returned by f32")
*/
LOCAL_C void TestCaging()
	{
	CFileMan* fMan=CFileMan::NewL(TheFs);
	TInt r;
	if(fMan!=NULL)
		{		
		
		// Checking the private path
		TBuf<30> privatepath;
		r=TheFs.PrivatePath(privatepath);
		test.Printf(_L("Private Path is=%S"),&privatepath);
		
		r = TheFs.MkDir(_L("\\Caged\\"));
		test(r==KErrNone || r==KErrAlreadyExists);
		
		CDir* entryCount=NULL;
		r=TheFs.GetDir(_L("\\*.*"),KEntryAttNormal,ESortNone,entryCount);
		test(r==KErrNone);
		TInt rootCount= entryCount->Count();
		
		delete entryCount;
		entryCount=NULL;


		//Testing Copy
		CDir* entryCount2=NULL;
		r=fMan->Copy(_L("\\sys\\"),_L("\\Caged\\"));
		test(r == KErrNotFound);
		r=fMan->Copy(_L("\\*"),_L("\\Caged\\"));
		test(r == KErrNone);
		
		r=TheFs.GetDir(_L("\\Caged\\*.*"),KEntryAttNormal,ESortNone,entryCount2);
		test(r==KErrNone);
		TInt cagedCount= entryCount2->Count();
		
		test(cagedCount==rootCount);
		
		delete entryCount2;
		entryCount2=NULL;

	
		r=fMan->Copy(_L("\\private\\two\\moo"),_L("\\private\\two\\mew")); 
		test(r == KErrPathNotFound);
	
		// Create a test file
		RFile testFile;
		r = testFile.Replace(TheFs, _L("\\capTest"),EFileWrite);
		test(r==KErrNone || r==KErrAlreadyExists);
		testFile.Close();
		
		TFileName name;
		name = privatepath;
		name.Append(_L("privateFile.tst"));
		RFile privateFile;
		r = privateFile.Replace(TheFs, name,EFileWrite);
		test(r==KErrNone || r==KErrAlreadyExists);
		privateFile.Close();

	
		r=fMan->Copy(_L("\\capTest"),_L("\\private\\to\\moo")); 
		test(r == KErrPathNotFound);
		r=fMan->Copy(_L("\\capTest"),_L("\\sys\\bin\\moo")); 
		test(r == KErrPermissionDenied); 
		r=fMan->Copy(_L("\\sys\\bin\\capTest"),_L("\\sys\\bin\\moo"));
		test(r == KErrPathNotFound); 
		r=fMan->Copy(_L("\\sys\\*"),_L("\\"));
		test (r==KErrNone || r==KErrNotFound);
		r=fMan->Copy(name,_L("\\sys\\"));
		test(r==KErrPermissionDenied);
		
		// Testing Move
		r=fMan->Move(_L("\\capTest"),_L("\\private\\wst\\moo"), CFileMan::ERecurse); // Recurse flag needed as destination path does not exist.
		test(r == KErrNone);
		r=fMan->Move(_L("\\sys\\bin\\capTest"),_L("\\sys\\bin\\moo"));
		test(r == KErrPathNotFound); 
		r=fMan->Move(_L("\\sys\\*"),_L("\\"));
		test (r==KErrNone || r==KErrNotFound);
		r=fMan->Move(name,_L("\\sys\\"));
		test(r==KErrPermissionDenied);
		r=fMan->Move(_L("\\private\\two\\moo"),_L("\\private\\one\\moo"));
		test(r == KErrPathNotFound);
		r=fMan->Move(_L("\\private\\two\\moo.."),_L("\\private\\one\\moo"));
		test(r == KErrPathNotFound);
		r=fMan->Move(_L("\\private\\two\\moo"),_L("\\private\\one\\moo.."));
		test(r == KErrPathNotFound);
		r=fMan->Move(name,_L("\\privateFile.tst"));
		test(r == KErrNone);
		r=fMan->Move(_L("\\privateFile.tst"),name);
		test(r == KErrNone);

		
		// Testing Attribs
		r=fMan->Attribs(_L("\\private\\two\\moo"),KEntryAttReadOnly,0,TTime(0)); 
		test(r == KErrPathNotFound);
		r=fMan->Attribs(_L("\\private\\moo"),KEntryAttReadOnly,0,TTime(0)); 
		test(r == KErrNotFound);
		r=fMan->Attribs(name,KEntryAttReadOnly,0,TTime(0));
		test(r == KErrNone);
		r=fMan->Attribs(name,0,KEntryAttReadOnly,TTime(0));
		test(r == KErrNone);		

		// Testing RmDir
		r=fMan->RmDir(_L("\\private\\"));
		test(r == KErrNone);
		// put it back where it was
		r = TheFs.MkDirAll(_L("\\private\\00000001\\"));
		test(r == KErrNone);
		r=fMan->RmDir(_L("\\private\\two\\"));
		test(r == KErrPathNotFound);
		r=fMan->RmDir(_L("\\private\\tw?\\"));
		test(r == KErrBadName);
		r=fMan->RmDir(_L("\\sys\\"));
		test(r == KErrPermissionDenied);
		
		// Testing Rename
		r=fMan->Rename(_L("\\private\\two\\moo"),_L("\\private\\two\\mew")); 
		test(r == KErrPathNotFound);
		
		// Testing Delete
		r=fMan->Delete(_L("\\private\\two\\test")); 
		test(r == KErrPathNotFound);
		r=fMan->Delete(_L("\\private\\moo")); 
		test(r == KErrNotFound);
		
		//Something that actually exists in Private
		r=fMan->Rename(name,_L("\\private\\00000001\\moo")); 
		test(r == KErrNotFound); //deleted the file previously
		r=fMan->Rename(_L("\\private\\00000001\\moo"),name); 
		test(r == KErrNotFound);
		r=fMan->Copy(name,_L("\\private\\00000001\\moo")); 
		test(r == KErrNotFound);
		r=fMan->Delete(_L("\\private\\00000001\\moo")); 
		test(r == KErrNotFound);

		// Clean up the test data
		r=fMan->RmDir(_L("\\Caged\\")); 
		test(r == KErrNone);
		r=fMan->Delete(_L("\\capTest")); 
		test(r == KErrNone || KErrNotFound);
		delete(fMan);
		}
	
	// CDirScan tests
	ScanDir(_L("\\"), CDirScan::EScanUpTree, KErrNone);
	ScanDir(_L("\\"), CDirScan::EScanDownTree, KErrNone);
	ScanDir(_L("\\private\\"), CDirScan::EScanDownTree, KErrNone);
	ScanDir(_L("\\private\\"), CDirScan::EScanUpTree, KErrNone);
	ScanDir(_L("\\sys\\"), CDirScan::EScanDownTree, KErrNone);
	ScanDir(_L("\\sys\\"), CDirScan::EScanUpTree,KErrNone);
	
	}
LOCAL_C void CleanupL()
//
// Clean up tests
//
	{
	test.Next(_L("Delete test directory"));
	CFileMan* fMan=CFileMan::NewL(TheFs);
	TInt r=fMan->RmDir(gSessionPath);
	test(r==KErrNone);
	delete fMan;
	}

GLDEF_C void CallTestsL(/*TChar aDriveLetter*/)
//
// Do all tests
//
	{
	if(!PlatSec::IsCapabilityEnforced(ECapabilityAllFiles))
		{
		test.Printf(_L("Capability ECapabilityAllFiles not enabled - leaving t_dcallfiles"));
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
	TestCaging();
    TestPathCheck();

	test.Printf(_L("No of files open=%d"), TheFs.ResourceCount());

	CleanupL();
	}
