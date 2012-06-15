// Copyright (c) 1996-2009 Nokia Corporation and/or its subsidiary(-ies).
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
// f32test\server\b_osfil.cpp
// 
//

#include <f32file.h>
#include <e32test.h>
#include "t_server.h"

const TInt KSizeExistsFile=26;
const TInt KSizeExistsFile2=25;

GLDEF_D RTest test(_L("B_OSFIL"));

/* these buffers get written to in order to change the file system */	
//LOCAL_D TBuf<5> fileSystemName =_L("LOC::");   /* default file system name */
LOCAL_D TBuf<20> noDevice = _L("L:\\tmpfile.tmp");
LOCAL_D TBuf<15> noDevicePath = _L("L:\\");
LOCAL_D TBuf<28> noDirectory = _L("C:\\nodirect\\tmpfile.tmp");
LOCAL_D TBuf<30> noDirectoryPath = _L("C:\\nodirect\\");
LOCAL_D TBuf<20> noFile = _L("C:\\notafile.tmp");
LOCAL_D TBuf<19> fileName = _L("C:\\tmpfile.tmp");
LOCAL_D TBuf<20> existsFile = _L("C:\\tmpexst1.tmp");
LOCAL_D TBuf<20> existsFile2 = _L("C:\\tmpexst2.tmp");
LOCAL_D TBuf<21> renameFile1 = _L("C:\\tmpren1.tmp");
LOCAL_D TBuf<21> renameFile2 = _L("C:\\tmpren2.tmp");
LOCAL_D TBuf<34> directoryFile = _L("C:\\nodirect\\nodirect\\nodirect\\");
LOCAL_D TBuf<14> openUniqueDir = _L("C:\\nodir2\\");

/*************************************************************************/
LOCAL_C void TestInfo()
/*
Test the file info function
*/
	{
	TEntry entry;
	TInt r=TheFs.Entry(noDevice,entry);
	test((r==KErrNotReady)||(r==KErrPathNotFound));
	r=TheFs.Entry(noDirectory,entry);
	test(r==KErrPathNotFound);
	r=TheFs.Entry(noFile,entry);
	test(r==KErrNotFound);
	r=TheFs.Entry(existsFile,entry);
	test(r==KErrNone);
	test(entry.iSize==KSizeExistsFile);
	test(!entry.IsDir());
	test(!(entry.iAtt&KEntryAttDir));
	r=TheFs.Entry(existsFile2,entry);
	test(r==KErrNone);
	test(entry.iSize==KSizeExistsFile2);
	test(!entry.IsDir());
	test(!(entry.iAtt&KEntryAttDir));
	}

LOCAL_C void TestDelete()
/*
Test the delete
*/
	{
	TInt r=TheFs.Delete(noDevice);
	test((r==KErrNotReady)||(r==KErrPathNotFound));
	r=TheFs.Delete(noDirectory);
	test(r==KErrPathNotFound);
	r=TheFs.Delete(noFile);
	test(r==KErrNotFound);
	r=TheFs.Delete(existsFile);
	test(r==KErrNone);
	r=TheFs.Delete(existsFile2);
	test(r==KErrNone);
	TEntry entry;
	r=TheFs.Entry(existsFile,entry);
	test(r==KErrNotFound);
	r=TheFs.Entry(existsFile2,entry);
	test(r==KErrNotFound);
	}

LOCAL_C void TestRename()
/*
Test the rename
*/
	{
	TInt r=TheFs.Rename(noDevice,fileName);
	test(r==KErrArgument);
	r=TheFs.Rename(noDirectory,fileName);
	test(r==KErrPathNotFound);
	r=TheFs.Rename(noFile,fileName);
	test(r==KErrNotFound);
	r=TheFs.Rename(existsFile,existsFile2);
	test(r==KErrAlreadyExists);
	r=TheFs.Delete(existsFile);
	test(r==KErrNone);
	r=TheFs.Rename(existsFile,existsFile);
	test(r==KErrNotFound);
	r=TheFs.Rename(existsFile2,existsFile);
	test(r==KErrNone);
	TEntry entry;
	r=TheFs.Entry(existsFile2,entry);
	test(r==KErrNotFound);
	r=TheFs.Entry(existsFile,entry);
	test(r==KErrNone);
	r=TheFs.Delete(existsFile);
	test(r==KErrNone);
	}

LOCAL_C void TestMkDir()
	{
	TParse fparse;
	TInt r=TheFs.MkDirAll(directoryFile);
	test(r==KErrNone);
	TEntry entry;
	r=TheFs.Entry(directoryFile.Left(directoryFile.Length()-1),entry);
	test(r==KErrNone);
	test(entry.IsDir());
	test(entry.iAtt&KEntryAttDir);
	r=TheFs.RmDir(directoryFile);
	test(r==KErrNone);
	r=fparse.Set(directoryFile,NULL,NULL);
	test(r==KErrNone);
	r=fparse.PopDir();
	test(r==KErrNone);
	r=TheFs.RmDir(fparse.DriveAndPath());
	test(r==KErrNone);
	r=fparse.PopDir();
	test(r==KErrNone);
	r=TheFs.RmDir(fparse.DriveAndPath());
	test(r==KErrNone);
	}


LOCAL_C void TestOpenUnique()
/*
Test unique file name generator
*/
	{
	RFile p1;
	RFile p2;

	TInt r=p1.Open(TheFs,noDevice,EFileStreamText);
	test((r==KErrNotReady)||(r==KErrPathNotFound));
	r=p1.Open(TheFs,noDirectory,EFileStreamText);
	test(r==KErrPathNotFound);
	TFileName generated1;
	r=p1.Temp(TheFs,noDevicePath,generated1,EFileStreamText);
	test((r==KErrNotReady)||(r==KErrPathNotFound));
	r=p1.Temp(TheFs,noDirectoryPath,generated1,EFileStreamText);
	test(r==KErrPathNotFound);

	r=p1.Temp(TheFs,_L(""),generated1,EFileStreamText);
	test(r==KErrNone);
	TFileName generated2;
	r=p2.Temp(TheFs,_L(""),generated2,EFileStreamText);
	test(r==KErrNone);
	test(generated1!=generated2);
	p1.Close();
	TFileName generated3;
	r=p1.Temp(TheFs,_L(""),generated3,EFileStream);
	test(r==KErrNone);
	test(generated3!=generated2);
	test(generated3!=generated1);
	p2.Close();
	p1.Close();

	r=TheFs.Delete(generated1);
	test(r==KErrNone);
	r=TheFs.Delete(generated2);
	test(r==KErrNone);
	r=TheFs.Delete(generated3);
	test(r==KErrNone);

	r=p1.Temp(TheFs,_L(""),generated1,EFileStreamText);
	test(r==KErrNone);
	r=p2.Temp(TheFs,_L(""),generated2,EFileStreamText);
	test(r==KErrNone);
	test(generated1!=generated2);
	p1.Close();
	r=p1.Temp(TheFs,_L(""),generated3,EFileStream);
	test(r==KErrNone);
	test(generated3!=generated2);
	test(generated3!=generated1);
	p2.Close();
	p1.Close();

	r=TheFs.Delete(generated1);
	test(r==KErrNone);
	r=TheFs.Delete(generated2);
	test(r==KErrNone);
	r=TheFs.Delete(generated3);
	test(r==KErrNone);

	r=TheFs.MkDir(openUniqueDir);
	test(r==KErrNone);
	r=p1.Temp(TheFs,openUniqueDir,generated1,EFileStream);
	test(r==KErrNone);
	r=p2.Temp(TheFs,openUniqueDir,generated2,EFileStreamText|EFileWrite);
	test(r==KErrNone);
	r=p1.Write(_L8("junk"),4);
	test(r==KErrNone);
	TInt pos=0;
	r=p1.Seek(ESeekCurrent,pos);//P_FSET
	test(r==KErrNone);
	p1.Close();
	p2.Close();
	
	r=TheFs.Delete(generated1);
	test(r==KErrNone);
	r=TheFs.Delete(generated2);
	test(r==KErrNone);

	r=p1.Temp(TheFs,openUniqueDir,generated1,EFileStream);
	test(r==KErrNone);
	r=p2.Temp(TheFs,openUniqueDir,generated2,EFileStreamText);
	test(r==KErrNone);
	p1.Close();

	pos=0;
	r=p2.Seek(ESeekStart,pos);
	test(r==KErrNone);
	p2.Close();
	r=TheFs.Delete(generated1);
	test(r==KErrNone);
	r=TheFs.Delete(generated2);
	test(r==KErrNone);
	r=TheFs.RmDir(openUniqueDir);
	test(r==KErrNone);
	}

LOCAL_C void TestPaths()
	{
	TFileName path1;
	TInt r=TheFs.SessionPath(path1);
	test(r==KErrNone);
	r=TheFs.SetSessionPath(path1);
	test(r==KErrNone);
	TFileName path2;
	r=TheFs.SessionPath(path2);
	test(r==KErrNone);
	test(path1==path2);
	RDir dir;
	r=dir.Open(TheFs,path1,0);
	test(r==KErrNone);
	dir.Close();
	}

LOCAL_C void TestFileStatus()
	{
	TEntry blk;
	TInt r=TheFs.Entry(existsFile,blk);
	test(r==KErrNone);
	TEntry blk1=blk;

/* Toggle the write attribute */
	TUint clear,set; /* were modifying write attrib */
	if (blk1.IsReadOnly())
		{
		clear=KEntryAttReadOnly;
		set=0;
		}
	else
		{
		clear=0;
		set=KEntryAttReadOnly;
		}
	r=TheFs.SetEntry(existsFile,blk1.iModified,set,clear);
	test(r==KErrNone);
	r=TheFs.Entry(existsFile,blk);
	test(r==KErrNone);
	if (blk1.IsReadOnly())
		test(!blk.IsReadOnly());
	else
		test(blk.IsReadOnly());

/* set write attrib back */
	r=TheFs.SetEntry(existsFile,blk1.iModified,clear,set);
	test(r==KErrNone);
	r=TheFs.Entry(existsFile,blk);
	test(r==KErrNone);
	if (blk1.IsReadOnly())
		test(blk.IsReadOnly());
	else
		test(!blk.IsReadOnly());

/* toggle the archive bit */
	if (blk1.IsArchive())
		{
		clear=KEntryAttArchive;
		set=0;
		}
	else
		{
		clear=0;
		set=KEntryAttArchive;
		}
	r=TheFs.SetEntry(existsFile,blk1.iModified,set,clear);
	test(r==KErrNone);
	r=TheFs.Entry(existsFile,blk);
	test(r==KErrNone);
	if (blk1.IsArchive())
		test(!blk.IsArchive());
	else
		test(blk.IsArchive());
/* set archive attrib back */
	r=TheFs.SetEntry(existsFile,blk1.iModified,clear,set);
	test(r==KErrNone);
	r=TheFs.Entry(existsFile,blk);
	test(r==KErrNone);
	if (blk1.IsArchive())
		test(blk.IsArchive());
	else
		test(!blk.IsArchive());

/* toggle the hidden bit */
	if (blk1.IsHidden())
		{
		clear=KEntryAttHidden;
		set=0;
		}
	else
		{
		clear=0;
		set=KEntryAttHidden;
		}
	r=TheFs.SetEntry(existsFile,blk1.iModified,set,clear);
	test(r==KErrNone);
	r=TheFs.Entry(existsFile,blk);
	test(r==KErrNone);
	if (blk1.IsHidden())
		test(!blk.IsHidden());
	else
		test(blk.IsHidden());
/* set hidden attrib back */
	r=TheFs.SetEntry(existsFile,blk1.iModified,clear,set);
	test(r==KErrNone);
	r=TheFs.Entry(existsFile,blk);
	test(r==KErrNone);
	if (blk1.IsHidden())
		test(blk.IsHidden());
	else
		test(!blk.IsHidden());

/* toggle the system bit */
	if (blk1.IsSystem())
		{
		clear=KEntryAttSystem;
		set=0;
		}
	else
		{
		clear=0;
		set=KEntryAttSystem;
		}
	r=TheFs.SetEntry(existsFile,blk1.iModified,set,clear);
	test(r==KErrNone);
	r=TheFs.Entry(existsFile,blk);
	test(r==KErrNone);
	if (blk1.IsSystem())
		test(!blk.IsSystem());
	else
		test(blk.IsSystem());
/* set system attrib back */
	r=TheFs.SetEntry(existsFile,blk1.iModified,clear,set);
	test(r==KErrNone);
	r=TheFs.Entry(existsFile,blk);
	test(r==KErrNone);
	if (blk1.IsSystem())
		test(blk.IsSystem());
	else
		test(!blk.IsSystem());
	r=TheFs.Delete(existsFile);
	test(r==KErrNone);
	r=TheFs.Delete(existsFile2);
	test(r==KErrNone);
	}


LOCAL_C void CreateTestFiles()
	{
	RFile fcb;
	TInt r=fcb.Replace(TheFs,existsFile,EFileWrite);
	test(r==KErrNone);
	r=fcb.Write(_L8("ABCDEFGHIJKLMNOPQRSTUVWXYZ"),KSizeExistsFile);
	test(r==KErrNone);
	fcb.Close();

	r=fcb.Replace(TheFs,existsFile2,EFileWrite);
	test(r==KErrNone);
	r=fcb.Write(_L8("bcdefghijklmnopqrstuvwxyz"),KSizeExistsFile2);
	test(r==KErrNone);
	fcb.Close();
	}

GLDEF_C void CallTestsL(void)
//
// Do testing on aDrive
//
	{
	test.Title();

	TVolumeInfo di;
	TInt r=TheFs.Volume(di);
	test(r==KErrNone);
	test.Start(_L("Info()"));
	CreateTestFiles();
	TestInfo();
	test.Next(_L("Delete()"));
	CreateTestFiles();
	TestDelete();
	test.Next(_L("Rename()"));
	CreateTestFiles();
	TestRename();
	test.Next(_L("MkDir()"));
	TestMkDir();
	test.Next(_L("Temp()"));
	TestOpenUnique();
	test.Next(_L("Paths"));	
	TestPaths();
	test.Next(_L("File status"));
	CreateTestFiles();
	TestFileStatus();
	test.End();
	}
