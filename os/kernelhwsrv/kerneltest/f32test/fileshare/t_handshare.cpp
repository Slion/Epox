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
// f32test\server\t_handshare.cpp
// 
//

#include <f32file.h>
#include <e32test.h>
#include "t_server.h"
#include "handshare.h"

GLDEF_D RTest test(_L("T_HANDSHARE"));

LOCAL_D TInt drivenum;



// test of (deprecated) RFile::Adopt()
//
// Request an open file (read only)
//
GLDEF_C void RequestFileDeprecatedAdopt()
	{
	test.Next(_L("RFile::Adopt()"));

	TInt r;
	RFileHandleSharer handsvr;
	do
		{
		r=handsvr.Connect();
		}
		while(r==KErrNotFound);
	test(r==KErrNone);
	r=handsvr.SetTestDrive(drivenum);
	test(r==KErrNone);
	TInt ssh;
	RFs fs1;
	TInt fsh = handsvr.GetFileHandle(ssh, EFileRead);

	r = fs1.SetReturnedHandle(fsh);
	test(r==KErrNone);
	
	RFile file;
	r=file.Adopt(fs1,ssh);
	test(r==KErrNone);
	
	TBuf8<100> rbuf;
	r=file.Read(0,rbuf);
	test(r==KErrNone);
	
	r=rbuf.CompareF(KTestData1());
	test(r==KErrNone);
	
	r=file.Write(KTestData());
	test(r==KErrAccessDenied);
	
	r=file.ChangeMode(EFileWrite);
	test(r==KErrArgument);
	
	r=file.Rename(_L("\\newname.txt"));
	test(r==KErrPermissionDenied || r==KErrAccessDenied);

	file.Close();

	fsh = handsvr.GetFileHandle(ssh, EFileRead);
	r = fs1.SetReturnedHandle(fsh);
	test(r==KErrNone);

	// Adopt a bad sub-session handle

	r=file.Adopt(fs1, KNullHandle);
	test(r==KErrBadHandle);

	r=file.Adopt(fs1, -1);
	test(r==KErrBadHandle);


	handsvr.Close();
	file.Close();
	RDebug::Print(_L("End Of Tests"));
	fs1.Close();
	}



// Cloning tests
GLDEF_C void Duplicate()
	{
	test.Next(_L("RFile::Duplicate()"));

	RFs fs;
	TInt r=fs.Connect();
	test(r==KErrNone);

	// Check the number of open file handles
	TInt resCount = fs.ResourceCount();
	test(resCount == 0);

	// create a file & fill it with data	
	RFile file1;
	r=file1.Replace(fs,KCliFileName,EFileWrite);
	test(r==KErrNone);
	r=file1.Write(KTestData());
	test(r==KErrNone);
	file1.Close();

	// open the file for read
	r = file1.Open(fs,KCliFileName,EFileRead);
	test(r==KErrNone);
	TBuf8<100> rbuf;
	r = file1.Read(0,rbuf);
	test(r==KErrNone);
	r=rbuf.CompareF(KTestData);
	test(r==KErrNone);


	// clone the file
	RFile file2;
	r = file2.Duplicate(file1);
	test(r==0);

	// make sure file positions are the same
	TInt pos1 = 0;
	r = file1.Seek(ESeekCurrent, pos1);
	test(r==KErrNone);
	TInt pos2 = 0;
	r = file2.Seek(ESeekCurrent, pos2);
	test(r==KErrNone);
	test(pos1 == pos2);

	// change the position on the duplcated file handle & 
	// verify that the original file handle's position is unchanged
	TInt oldPos1 = pos1;
	
	const TInt newPos2 = 5;
	pos2 = newPos2;
	r = file2.Seek(ESeekStart, pos2);
	test(r==KErrNone);
	
	pos1 = 0;
	r = file1.Seek(ESeekCurrent, pos1);
	test(r==KErrNone);
	test(pos1 == oldPos1);
	
	pos2 = 0;
	r = file2.Seek(ESeekCurrent, pos2);
	test(r==KErrNone);
	test(pos2 == newPos2);
	test(pos1 != pos2);

	// close the parent file and check we can still use the duplicated one.
	file1.Close();

	// Check the number of open file handles - should be 1 (the duplicated one)
	resCount = fs.ResourceCount();
	test(resCount == 1);

	fs.Close();

	rbuf.FillZ();

	// reset to pos 0
	pos2 = 0;
	r = file2.Seek(ESeekStart, pos2);
	test(r==KErrNone);
	
	r=file2.Read(0,rbuf);
	test(r==KErrNone);
	r=rbuf.CompareF(KTestData);
	test(r==KErrNone);
	file2.Close();

	// start again - this time we're going to close the duplicated file first
	// and check we can still use the parent file

	r = fs.Connect();
	test(r==KErrNone);

	// Make a note of the number of open resources
	fs.ResourceCountMarkStart();

	// open the file for read
	r = file1.Open(fs,KCliFileName,EFileRead);
	test(r==KErrNone);


	// clone the file & check we can read it
	r = file2.Duplicate(file1, EOwnerThread);
	test(r==0);
	rbuf.FillZ();
	r = file2.Read(0,rbuf);
	test(r==KErrNone);
	r=rbuf.CompareF(KTestData);
	test(r==KErrNone);


	// close the duplicated file and check we can still use the parent one.
	file2.Close();

	rbuf.FillZ();

	// check we can read the parent file
	r=file1.Read(0,rbuf);
	test(r==KErrNone);
	r=rbuf.CompareF(KTestData);
	test(r==KErrNone);

	// close the parent
	file1.Close();

	// Check the number of open file handles
	fs.ResourceCountMarkEnd();
	resCount = fs.ResourceCount();
	test(resCount == 0);

	fs.Close();
	}


// Request an open file (write mode) from the server
// using RFile::AdoptFromServer()
GLDEF_C void RequestFileWrite()
	{
	TInt r;
	RFileHandleSharer handsvr;
	do
		{
		r=handsvr.Connect();
		}
	while(r==KErrNotFound);
	test(r==KErrNone);

	r=handsvr.SetTestDrive(drivenum);
	test(r==KErrNone);

	TInt ssh;
	TInt fsh = handsvr.GetFileHandle2(ssh, EFileWrite);
	test (fsh >= 0);

	// Closing the handle to the server ensures the server has closed it's
	// RFs and RFile handles - this provides a means of testing whether we 
	// can still adopt the RFile even if the server has closed it's one.

	handsvr.Sync(); // make sure server has finished doing what it's doing
	handsvr.Close();

	// adopt the file handle from FHServer
	test.Next(_L("RFile::AdoptFromServer()"));

	RFile file;
	r=file.AdoptFromServer(fsh, ssh);
	test(r==KErrNone);

	TBuf8<100> rbuf;
	r=file.Read(0,rbuf);
	test(r==KErrNone);

	// server should write KTestData1 ("Server Write Server Write") to file
	r=rbuf.CompareF(KTestData1);
	test(r==KErrNone);

	// reset to pos 0
	TInt pos = 0;
	r = file.Seek(ESeekStart, pos);
	test(r==KErrNone);
	
	// overwrite with KTestData ("Client Write Client Write") to file
	r=file.Write(KTestData());
	test(r==KErrNone);
	rbuf.FillZ();
	r=file.Read(0,rbuf);
	test(r==KErrNone);
	r=rbuf.CompareF(KTestData);
	test(r==KErrNone);

// !!! Disable platform security tests until we get the new APIs
//	r=file.Rename(_L("\\newname.txt"));
//	test(r==KErrPermissionDenied);

	test.Next(_L("RFile::Name()"));

	// retrieve the file name from the server
	TFileName name;
	r = file.Name(name);
	test(r==KErrNone);
	r = name.Compare(KSvrFileName());
	test(r==0);
	

	test.Next(_L("RFile::Duplicate()"));
	RFile file2;
	r = file2.Duplicate(file);
	test(r==0);


	TInt pos1 = 0;
	r = file.Seek(ESeekCurrent, pos1);
	test(r==KErrNone);
	TInt pos2 = 0;
	r = file2.Seek(ESeekCurrent, pos2);
	test(r==KErrNone);
	test(pos1 == pos2);

	// close the parent file and check we can still use the duplicated one.
	file.Close();

	rbuf.FillZ();

	// reset to pos 0
	pos2 = 0;
	r = file2.Seek(ESeekStart, pos2);
	test(r==KErrNone);
	
	r=file2.Read(0,rbuf);
	test(r==KErrNone);
	r=rbuf.CompareF(KTestData);
	test(r==KErrNone);
	file2.Close();
	}


// Request a test file & test what we can do with it
// i.e CFileMan::Copy, RFile::Rename() and RFs:Delete()
//
void RequestFileTest()
	{
	TInt r;

	RFs fs;
	r=fs.Connect();
	test(r==KErrNone);
	r=fs.ShareProtected();
	test(r==KErrNone);

	
	// define a filename in our private path
	TPath newPath;
	fs.PrivatePath(newPath);
	TFileName newFileName;
	newFileName = newPath;
	newFileName.Append(_L("newname.txt"));
	
	r=fs.CreatePrivatePath(drivenum);
	test(r==KErrNone);
	r=fs.SetSessionToPrivate(drivenum);

	RFileHandleSharer handsvr;
	do
		{
		r=handsvr.Connect();
		}
	while(r==KErrNotFound);
	test(r==KErrNone);

	r=handsvr.SetTestDrive(drivenum);
	test(r==KErrNone);

	// Next verify that we can copy a file retrieved from the server
	// using CFileMan::Copy()

	test.Next(_L("CFileMan::Copy()"));

	TInt ssh;
	TInt fsh = handsvr.GetFileHandle2(ssh, EFileRead);
	test (fsh >= 0);

	RFile file;
	r=file.AdoptFromServer(fsh, ssh);
	test(r==KErrNone);


	CFileMan* fileMan = NULL;
	TRAP(r, fileMan = CFileMan::NewL(fs));
	test(r == KErrNone && fileMan != NULL);

	// copy to file, overwrite 
	r = fileMan->Copy(file, newFileName, CFileMan::EOverWrite);
	test(r == KErrNone);

	// copy to file, don't overwrite 
	r = fileMan->Copy(file, newFileName, 0);	
	test(r == KErrAlreadyExists);

	// copy to file, overwrite
	r = fileMan->Copy(file, newFileName, CFileMan::EOverWrite);
	test(r == KErrNone);

	// copy to path
	r = fileMan->Copy(file, newPath, CFileMan::EOverWrite);	
	test(r == KErrNone);

	// copy to file, overwrite, asynchnonous
	TRequestStatus status(KRequestPending);
	r = fileMan->Copy(file, newFileName, CFileMan::EOverWrite, status);
	test(r == KErrNone);
	User::WaitForRequest(status);
	test(status == KErrNone);


	// Negative tests...
	TPath newLongPath;
	TInt len;

	// copy to very long but valid path (no filename) which will overflow
	// when the filename is appended to it
	newLongPath = newPath;
	for (len=newLongPath.Length(); len< KMaxPath -4; len = newLongPath.Length())
		newLongPath.Append(_L("x\\"));
	r = fileMan->Copy(file, newLongPath, CFileMan::EOverWrite);	
	test(r == KErrBadName);

	// copy to very long but valid path (no filename) which will overflow
	// when drive letter is pre-pended to it
	newLongPath = newPath;
	for (len=newLongPath.Length(); len< KMaxPath -2; len = newLongPath.Length())
		newLongPath.Append(_L("x\\"));
	r = fileMan->Copy(file, newLongPath, CFileMan::EOverWrite);	
	test(r == KErrBadName);

	// copy to very long but valid path and filename which will overflow
	// when drive letter is pre-pended to it
	newLongPath.Append(_L("y"));
	r = fileMan->Copy(file, newLongPath, CFileMan::EOverWrite);	
	test(r == KErrBadName);

	// copy to badly formed path 
	newLongPath = newPath;
	newLongPath.Append(_L("\\y"));
	r = fileMan->Copy(file, newLongPath, CFileMan::EOverWrite);	
	test(r == KErrBadName);

	// copy to correctly formed path which doesn't exist
	newLongPath = newPath;
	newLongPath.Append(_L("x\\y\\z"));
	r = fileMan->Copy(file, newLongPath, CFileMan::EOverWrite);	
	test(r == KErrPathNotFound);

	delete fileMan; fileMan = NULL;

	file.Close();

	
	// First verify that we CANNOT rename a file retrieved from the server
	// that has not been opened in EFileShareExclusive mode

	test.Next(_L("negative test of RFile::Rename()"));

	fsh = handsvr.GetFileHandle2(ssh, TFileMode(EFileShareAny | EFileWrite));
	test (fsh >= 0);

	// adopt the file handle from FHServer
	r=file.AdoptFromServer(fsh, ssh);
	test(r==KErrNone);

	r=file.Rename(_L("newname.txt"));
	test(r==KErrPermissionDenied || r==KErrAccessDenied);

	// delete the file before we try to rename anything to it
	r = fs.Delete(newFileName);
	test(r == KErrNone || r == KErrNotFound);

	r=file.Rename(newFileName);
	test(r==KErrPermissionDenied || r==KErrAccessDenied);

	file.Close();


	// Next verify that we CAN rename a file retrieved from the server
	// that HAS been opened in EFileShareExclusive mode

	test.Next(_L("RFile::Rename()"));

	fsh = handsvr.GetFileHandle2(ssh, EFileWrite);
	test (fsh >= 0);

	r=file.AdoptFromServer(fsh, ssh);
	test(r==KErrNone);

	// delete the file before we try to rename anything to it
	r = fs.Delete(newFileName);
	test(r == KErrNone || r == KErrNotFound);

	r=file.Rename(newFileName);
	test(r==KErrNone);

	file.Close();

	// Next verify that we can delete the file (which should now 
	// have been moved to our private directory)
	test.Next(_L("RFs::Delete()"));
	r = fs.Delete(newFileName);
	test(r == KErrNone);

	handsvr.Close();

	fs.Close();
	}

//
// Pass an open file to the server 
// The server will use RFile::AdoptFromClient()
//
GLDEF_C void PassFile()
	{
	RFs fs;
	TInt r=fs.Connect();
	test(r==KErrNone);

	// Check the number of open file handles
	TInt resCount = fs.ResourceCount();
	test(resCount == 0);

	r=fs.ShareProtected();
	test(r==KErrNone);

	r=fs.CreatePrivatePath(drivenum);
	test(r==KErrNone);
	r=fs.SetSessionToPrivate(drivenum);
	
	
	RFile file1;
	r=file1.Replace(fs,KCliFileName,EFileWrite);
	test(r==KErrNone);
	r=file1.Write(KTestData());
	test(r==KErrNone);
	file1.Close();

	RFileHandleSharer handsvr;
	do
		{
		r=handsvr.Connect();
		}
	while(r==KErrNotFound);
	test(r==KErrNone);

	r=handsvr.SetTestDrive(drivenum);
	test(r==KErrNone);

	r=fs.SetSessionToPrivate(drivenum);
	test(r==KErrNone);

	r=file1.Open(fs,KCliFileName,EFileRead);
	test(r==KErrNone);
	
	// pass the file handle to FHServer
	test.Next(_L("RFile::TransferToServer()"));

	TIpcArgs ipcArgs;
	file1.TransferToServer(ipcArgs, 0, 1);

	r = handsvr.PassFileHandle(ipcArgs);


	// verify that the original file handle's position is unchanged
	TInt pos = 0;
	r = file1.Seek(ESeekCurrent, pos);
	test(r==KErrNone);
	test(pos == 0);

	// make sure we can still use it
	TBuf8<100> rbuf;
	r=file1.Read(0,rbuf);
	test(r==KErrNone);

	// Close the file
	file1.Close();	

	handsvr.Close();

	r=fs.MkDir(_L("C:\\mdir"));
	test(r==KErrNone || r==KErrAlreadyExists);
	
	// Check the number of open file handles
	resCount = fs.ResourceCount();
	test(resCount == 0);

	fs.Close();
	}


//
// Pass an invalid file handle to the server 
// The server will use RFile::AdoptFromClient()
//
GLDEF_C void PassInvalidFile()
	{

	RFs fs;
	TInt r=fs.Connect();
	test(r==KErrNone);

	// Check the number of open file handles
	TInt resCount = fs.ResourceCount();
	test(resCount == 0);

	r=fs.ShareProtected();
	test(r==KErrNone);

	r=fs.CreatePrivatePath(drivenum);
	test(r==KErrNone);
	r=fs.SetSessionToPrivate(drivenum);
	
	
	RFile file1;
	r=file1.Replace(fs,KCliFileName,EFileWrite);
	test(r==KErrNone);
	r=file1.Write(KTestData());
	test(r==KErrNone);
	file1.Close();

	RFileHandleSharer handsvr;
	do
		{
		r=handsvr.Connect();
		}
	while(r==KErrNotFound);
	test(r==KErrNone);

	r=handsvr.SetTestDrive(drivenum);
	test(r==KErrNone);

	r=fs.SetSessionToPrivate(drivenum);
	test(r==KErrNone);

	r=file1.Open(fs,KCliFileName,EFileRead);
	test(r==KErrNone);
	
	// check the resoure count - there should be 1 open file handle
	resCount = fs.ResourceCount();
	test(resCount == 1);

	// pass an invalid file handle to FHServer
	// by overwriting the IPC slots
	test.Next(_L("PassInvalidFileHandle - RFile::TransferToServer()"));

	TIpcArgs ipcArgs;

	// Pass a bad RFs handle
	file1.TransferToServer(ipcArgs, 0, 1);

	// check the resoure count - there should be 2 open file handles
	resCount = fs.ResourceCount();
	test(resCount == 2);

	ipcArgs.Set(0, 0);	// invalidate the RFs handle
	r = handsvr.PassInvalidFileHandle(ipcArgs);
	test (r == KErrBadHandle);

	// Pass a bad RFile handle
	file1.TransferToServer(ipcArgs, 0, 1);

	// check the resoure count - there should be 3 open file handles
	resCount = fs.ResourceCount();
	test(resCount == 3);

	ipcArgs.Set(1, 0);	// invalidate the RFile handle
	r = handsvr.PassInvalidFileHandle(ipcArgs);
	test (r == KErrBadHandle);

	// Pass bad RFs and RFile handles
	file1.TransferToServer(ipcArgs, 0, 1);

	// check the resoure count - there should be 4 open file handles
	resCount = fs.ResourceCount();
	test(resCount == 4);


	ipcArgs.Set(0, 0);	// invalidate the RFs handle
	ipcArgs.Set(1, 0);	// invalidate the RFile handle
	r = handsvr.PassInvalidFileHandle(ipcArgs);
	test (r == KErrBadHandle);

	// Close the file
	handsvr.Close();
	file1.Close();	

	// Check the number of open file handles
	resCount = fs.ResourceCount();
	test(resCount == 3);

	fs.Close();
	}


// Pass RFs/RFile handles to the server using
// process parameter slots (RProcess::SetParameter()),
// resume the process and wait for it to read the file...
// The server will use RFile::AdoptFromCreator()
GLDEF_C void PassFile(RProcess& aProcess)
	{
	
	RFs fs;
	TInt r=fs.Connect();
	test(r==KErrNone);

	// Check the number of open file handles
	TInt resCount = fs.ResourceCount();
	test(resCount == 0);

	r=fs.ShareProtected();
	test(r==KErrNone);

	r=fs.CreatePrivatePath(drivenum);
	test(r==KErrNone);
	r=fs.SetSessionToPrivate(drivenum);
	
	
	RFile file1;
	r=file1.Replace(fs,KCliFileName,EFileWrite);
	test(r==KErrNone);
	r=file1.Write(KTestData());
	test(r==KErrNone);
	file1.Close();

	r=file1.Open(fs, KCliFileName, EFileWrite);

	test(r==KErrNone);
	
	// NB slot 0 is reserved for the command line

	test.Next(_L("RFile::TransferToProcess()"));

	r = file1.TransferToProcess(aProcess, 1, 2);

	r = aProcess.SetParameter(3, drivenum);
	test(r==KErrNone);

	r=fs.SetSessionToPrivate(drivenum);
	test(r==KErrNone);

	// make sure we can still read from the file
	TBuf8<100> rbuf;
	r=file1.Read(0,rbuf);
	test(r==KErrNone);
	r=rbuf.CompareF(KTestData());
	test(r==KErrNone);
	file1.Close();

	r=fs.MkDir(_L("C:\\mdir"));
	test(r==KErrNone || r==KErrAlreadyExists);
	
	// Check the number of open file handles - 
	// should be 1 (the one duplicated for the other process)
	resCount = fs.ResourceCount();
	test(resCount == 1);

	fs.Close();

	// Start the server thread
	aProcess.Resume();



	// connect to the server
	RFileHandleSharer handsvr;
	do
		{
		r=handsvr.Connect();
		}
	while(r==KErrNotFound);
	test(r==KErrNone);


	r=handsvr.SetTestDrive(drivenum);
	test(r==KErrNone);

	// wait for server to read the file
	r = handsvr.PassFileHandleProcess();
	test (r == KErrNone);
	
	handsvr.Close();
	}



GLDEF_C void CallTestsL(void)
    {
	RFs::CharToDrive(gDriveToTest,drivenum);

	// make sure the session path exists
	RFs fs;
	TInt r = fs.Connect();
	test(r==KErrNone);

	TFileName sessionp;
	fs.SessionPath(sessionp);
	r = fs.MkDirAll(sessionp);
	test(r==KErrNone || r==KErrAlreadyExists);
	fs.Close();

	// Remember the number of open handles. Just for a sanity check ....
	TInt start_thc, start_phc;
	RThread().HandleCount(start_phc, start_thc);
	test.Printf(_L("Handles: start_phc %d, start_thc %d\n"), start_phc, start_thc);

	//create test server
	RProcess p;
	r = p.Create(_L("FHServer.exe"), KNullDesC);
	test(r==KErrNone);

	// RFile::Duplicate() tests
	Duplicate();

	// Pass RFs/RFile handles to the server using RFile::TransferToProcess()
	// process parameter slots (RProcess::SetParameter()),
	// resume the server process and wait for it to read the file...
	PassFile(p);
	
	// Transfer the file handle to FHServer using RFile::TransferToServer(). 
	// Get FHServer to transfer it to FHServer2
	PassFile();

	
	// Get an open writeable file from FHServer2 via FHServer 
	// using RFile::AdoptFromServer()
	RequestFileWrite();

	// Deprecated RFile::Adopt() test
	RequestFileDeprecatedAdopt();

	// negative test 
	// Pass an invalid file handle to the server 
	PassInvalidFile();


	// Get an open file handle from FHServer2 via FHServer 
	// and test whether we can copy/rename/delete it etc
	// be able to rename it.
	RequestFileTest();

	// stop the servers
	RFileHandleSharer handsvr;
	r=handsvr.Connect();
	test(r==KErrNone);
	r = handsvr.Exit();
	test(r == KErrNone);
	handsvr.Close();
	
	// delete the test file
	RFs cleanupfs;
	r=cleanupfs.Connect();
	test(r==KErrNone);
	r=cleanupfs.SetSessionToPrivate(drivenum);
	test(r==KErrNone);
	r=cleanupfs.Delete(KCliFileName);
	test(r==KErrNone || r==KErrNotFound);
	cleanupfs.Close();


	// wait for server process to end
	TRequestStatus status;
	p.Logon(status);
	User::WaitForRequest(status);
	test (status == KErrNone);

	// cleanup
	p.Close();

	// Sanity check for open handles
	TInt end_thc, end_phc;
	RThread().HandleCount(end_phc, end_thc);
	test.Printf(_L("Handles: end_phc %d, end_thc %d\n"), end_phc, end_thc);

	test(start_thc == end_thc);
	test(start_phc == end_phc);
	
	// and also for pending requests ...
	test(RThread().RequestCount() == 0);
	
	
	RDebug::Print(_L("End Of Tests"));
	}
	


