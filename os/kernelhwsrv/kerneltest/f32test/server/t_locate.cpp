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
// f32test\server\t_locate.cpp
// 
//
#define __E32TEST_EXTENSION__
#include <f32file.h>
#include <e32test.h>
#include "t_server.h"



GLDEF_D RTest test(_L("T_LOCATE"));

LOCAL_D TFileName gPath1;
LOCAL_D TFileName gPath2;
LOCAL_D TFileName gPath3;
LOCAL_D TFileName gPath4;
LOCAL_D TFileName gPath5;


LOCAL_D TFileName gRemovableDriveFile;
LOCAL_D TFileName gInternalDriveFile;
LOCAL_D TFileName gInternalDriveFile2;


LOCAL_D TChar removableDriveLetter;  
LOCAL_D TChar internalDriveLetter; 


LOCAL_D TInt removableFlag=0;
LOCAL_D TInt internalFlag=0;



LOCAL_C void Md(const TDesC& aDirName)
//
// Make a dir
//
	{

	TInt r=TheFs.MkDirAll(aDirName);
	if (r == KErrCorrupt)
		test.Printf(_L("Media corruption; previous test may have aborted; else, check hardware\n"));
	else if (r == KErrNotReady)
		test.Printf(_L("No medium present / drive not ready, previous test may have hung; else, check hardware\n"));
	test_Value(r, r == KErrNone || r == KErrAlreadyExists);
	}

LOCAL_C void Mf(const TDesC& aFileName)
//
// Make a file
//
	{

	RFile file;
	TInt r = file.Replace(TheFs,aFileName,0);
	if (r == KErrPathNotFound)
		{
		test.Printf(_L("Mf: Path Not Found\n"));
		Md(aFileName);
		r=file.Replace(TheFs,aFileName,0);
		}

	if (r == KErrCorrupt)
		test.Printf(_L("Media corruption; previous test may have aborted; else, check hardware\n"));
	else if (r == KErrNotReady)
		test.Printf(_L("No medium present / drive not ready, previous test may have hung; else, check hardware\n"));

	test_Value(r, r == KErrNone || r == KErrAlreadyExists);
	file.Close();
	}

LOCAL_C void MakeLocateTestDirectoryStructure()
//
// Create files for test
//
	{
	test.Next(_L("Create LOCTEST directories"));
	Md(_L("\\F32-TST\\LOCTEST\\BIN1\\"));
	Md(_L("\\F32-TST\\LOCTEST\\BIN2\\"));
	Md(_L("\\F32-TST\\LOCTEST\\BIN3\\"));
	Md(_L("\\F32-TST\\LOCTEST\\BIN1\\BIN4\\"));
		
	
	
#if defined(_DEBUG)
	TheFs.SetErrorCondition(-47,5);
	TDriveInfo drive;
	for (TInt i=0;i<5;i++)
		{
		TInt r=TheFs.Drive(drive);
		test(r==KErrNone);
		}
	TInt r=TheFs.MkDirAll(_L("alskdjfl"));
	test(r==-47);
	r=TheFs.MkDirAll(_L("alskdjfl"));
	test(r==-47);
	TheFs.SetErrorCondition(KErrNone);
	r=TheFs.Drive(drive);
	test(r==KErrNone);
#endif
//
	test.Next(_L("Create LOCTEST files"));
	Mf(_L("\\F32-TST\\LOCTEST\\FILE1.AAA"));
	Mf(_L("\\F32-TST\\LOCTEST\\FILE2.BBB"));
	Mf(_L("\\F32-TST\\LOCTEST\\FILE3.CCC"));
	Mf(_L("\\F32-TST\\LOCTEST\\WORK.AAA"));
	Mf(_L("\\F32-TST\\LOCTEST\\HOME.CCC"));
	Mf(_L("\\F32-TST\\LOCTEST\\FILE.AAA"));
	Mf(_L("C:\\F32-TST\\LOCTEST\\BIN1\\FILE1.AAA"));
	Mf(_L("C:\\F32-TST\\LOCTEST\\BIN1\\WORK.AAA"));
	Mf(_L("C:\\F32-TST\\LOCTEST\\BIN1\\WORK.BBB"));
	Mf(_L("\\F32-TST\\LOCTEST\\BIN1\\FILE1.AAA"));
	Mf(_L("\\F32-TST\\LOCTEST\\BIN1\\WORK.AAA"));
	Mf(_L("\\F32-TST\\LOCTEST\\BIN1\\WORK.BBB"));
	Mf(_L("\\F32-TST\\LOCTEST\\BIN1\\CONFUSED.DOG"));
	Mf(_L("\\F32-TST\\LOCTEST\\BIN2\\FILE1.BBB"));
	Mf(_L("\\F32-TST\\LOCTEST\\BIN2\\WORK.BBB"));
	Mf(_L("\\F32-TST\\LOCTEST\\BIN2\\FILE2.BBB"));
	Mf(_L("\\F32-TST\\LOCTEST\\BIN2\\FILE3.BBB"));
	Mf(_L("\\F32-TST\\LOCTEST\\BIN3\\FILE3.CCC"));
	Mf(_L("\\F32-TST\\LOCTEST\\BIN3\\WORK.CCC"));
	Mf(_L("\\F32-TST\\LOCTEST\\BIN3\\PLAY.CCC"));
	Mf(_L("\\F32-TST\\LOCTEST\\BIN1\\BIN4\\FILE1.AAA"));
	Mf(_L("\\F32-TST\\LOCTEST\\BIN1\\BIN4\\FILE2.BBB"));
	Mf(_L("\\F32-TST\\LOCTEST\\BIN1\\BIN4\\FILE3.CCC"));
	Mf(_L("\\F32-TST\\LOCTEST\\BIN1\\BIN4\\FILE4.DDD"));
	
	
	
	}
	

LOCAL_C void CreateFilesInRemovableDrive()	
	{
	

    TInt err;
    TDriveList driveList;
    TDriveInfo info;

	 err = TheFs.DriveList(driveList);
    test( err == KErrNone );
    
    for (TInt i = 0; i < KMaxDrives; i++) 
        {
        
        if (driveList[i]) 
            {
            err = TheFs.Drive(info, i);
            test( err == KErrNone ); 
                        
            if( info.iDriveAtt  & KDriveAttRemovable )  
            	{
								
				if ( ( info.iType != EMediaNotPresent) && (info.iType != EMediaUnknown) && (info.iType != EMediaCdRom) )
					{
					TheFs.DriveToChar(i,removableDriveLetter) ;
					gRemovableDriveFile.Append (removableDriveLetter);
					gRemovableDriveFile.Append (_L(":\\F32-TST\\LOCTEST\\BIN\\FINDFILE.AAA") );

					Mf(gRemovableDriveFile);
					removableFlag=1;
					break;
					}
				else 
					continue;
      				
 
            	}
           				
            
            }
    
        }

	}
	
	

LOCAL_C void CreateFilesInInternalDrive()	
	{
	
    TInt err;
    TDriveList driveList;
    TDriveInfo info;

	 err = TheFs.DriveList(driveList);
    test( err == KErrNone );
    
    for (TInt i = 0; i < KMaxDrives; i++) 
        {
        
        if (driveList[i]) 
            {
            err = TheFs.Drive(info, i);
            test( err == KErrNone ); 
                        
            if( info.iDriveAtt  & KDriveAttInternal  ) 
            	{			
				
				TheFs.DriveToChar(i,internalDriveLetter) ;
				gInternalDriveFile.Append (internalDriveLetter);
				gInternalDriveFile.Append (_L(":\\F32-TST\\LOCTEST\\BIN\\INT\\FINDINTERNALFILE.AAA") );
				
				gInternalDriveFile2.Append (internalDriveLetter);
				gInternalDriveFile2.Append (_L(":\\F32-TST\\LOCTEST\\BIN\\INT\\FINDINTERNALFILE_B.AAA") );
				
				Mf(gInternalDriveFile);
				Mf(gInternalDriveFile2);
				internalFlag=1;	
 				
            	break;
            	}
            
            }
    
        }
	
	
	}
	



LOCAL_C void DeleteRemovableDirectory()
	{	

	//Delete the directory structure we created in the removalbe drive	
	if ( removableFlag == 1 )
		{
		CFileMan* fMan=CFileMan::NewL(TheFs);
		test(fMan!=NULL);
	
		TFileName gPathRem;
		gPathRem.Append (removableDriveLetter);
		gPathRem.Append (_L(":\\F32-TST\\") );
		TInt r=fMan->RmDir(gPathRem);
		test(r==KErrNone);
	
		delete fMan;
		}
	}

LOCAL_C void DeleteInternalDirectory()
	{	

	//Delete the directory structure we created in the internal drive		


	if( internalFlag == 1 )
		{
		CFileMan* fMan=CFileMan::NewL(TheFs);
		test(fMan!=NULL);
	
		TFileName gPathInt;
		gPathInt.Append (internalDriveLetter);
		gPathInt.Append (_L(":\\F32-TST\\") );
		TInt r=fMan->RmDir(gPathInt);
		test(r==KErrNone);
	
		delete fMan;
		}
	}

	
LOCAL_C void MountRemoteFilesystem()	
	{
		
  	test.Next(_L("Mount Remote Drive simulator on Q:\n"));
	
	
	TInt r=TheFs.AddFileSystem(_L("CFAFSDLY"));
	test.Printf(_L("Add remote file system\n"));
	test.Printf(_L("AddFileSystem returned %d\n"),r);
	test (r==KErrNone || r==KErrAlreadyExists);


	r=TheFs.MountFileSystem(_L("DELAYFS"),EDriveQ);

	
	test.Printf(_L("Mount remote file system\n"));
	test.Printf(_L("MountFileSystem returned %d\n"),r);
	test(r==KErrNone || r==KErrCorrupt || r==KErrNotReady || r==KErrAlreadyExists);

	
	Mf(_L("Q:\\F32-TST\\LOCTEST\\BIN\\FINDFILE.AAA"));

	}
	

	
LOCAL_C void DisMountRemoteFilesystem()	
	{

	test.Printf(_L("Dismounting the remote Drives \n"));
 	
 	TInt r=TheFs.DismountFileSystem(_L("DELAYFS"),EDriveQ);
 	 
 	test.Printf(_L("Dismounting the Remote Drive returned %d\n"),r);
 	
 	test(r==KErrNone );
	}


	

LOCAL_C void Test1()
//
// Do simple tests
//
	{
	test.Next(_L("Test FindByPath"));
	
	TAutoClose<RFs> fs;
	TInt r=fs.iObj.Connect();
	test(r==KErrNone);
	TFindFile finder(fs.iObj);
	TPtrC path=gPath1;
	r=finder.FindByPath(_L("file1.aaa"),&path);
	test(r==KErrNone);
	TParse fileParse;
	fileParse.Set(finder.File(),NULL,NULL);
	test(fileParse.Path()==_L("\\F32-TST\\LOCTEST\\BIN1\\"));
	test(fileParse.NameAndExt()==_L("file1.aaa"));
	r=finder.Find();
	test(r==KErrNotFound);


	path.Set(gPath2);
	r=finder.FindByPath(_L("file1.aaa"),&path);
	test(r==KErrNone);
	fileParse.Set(finder.File(),NULL,NULL);
	test(fileParse.Path()==_L("\\F32-TST\\LOCTEST\\BIN1\\"));
	test(fileParse.NameAndExt()==_L("file1.aaa"));
	r=finder.Find();
	test(r==KErrNone);
	fileParse.Set(finder.File(),NULL,NULL);
	test(fileParse.Path()==_L("\\F32-TST\\LOCTEST\\BIN1\\BIN4\\"));
	test(fileParse.NameAndExt()==_L("file1.aaa"));
	r=finder.Find();
	test(r==KErrNotFound);
//
	test.Next(_L("Test FindByDir"));
	TPtrC dir=_L("\\F32-TST\\LOCTEST\\BIN2\\");
	r=finder.FindByDir(_L("file2.bbb"),dir);
	test(r==KErrNone);
	TFileName defaultPath;
	r=TheFs.SessionPath(defaultPath);
	defaultPath.SetLength(2);
	test(r==KErrNone);
	fileParse.Set(finder.File(),NULL,NULL);
	test(fileParse.Drive()==defaultPath);
	test(fileParse.Path()==_L("\\F32-TST\\LOCTEST\\BIN2\\"));
	test(_L("file2.bbb").MatchF(fileParse.NameAndExt())!=KErrNotFound); // MatchF only sees wildcards in its argument
	r=finder.Find();
	if (r==KErrNone)
		{
		fileParse.Set(finder.File(),NULL,NULL);
		if (defaultPath==_L("C:"))
			test(fileParse.Drive()==_L("Y:"));
		else
			test(fileParse.Drive()==_L("C:"));
		test(fileParse.Path()==_L("\\F32-TST\\LOCTEST\\BIN2\\"));
		test(_L("file2.bbb").MatchF(fileParse.NameAndExt())!=KErrNotFound);
		r=finder.Find();
		}
	test(r==KErrNotFound);
	}

LOCAL_C void Test2()
//
// Test extremes
//
	{

	test.Next(_L("Test extremes"));
	TAutoClose<RFs> fs;
	TInt r=fs.iObj.Connect();
	test(r==KErrNone);
	TBuf<4> temp=_L("");
	TFindFile finder(fs.iObj);
	r=finder.FindByPath(_L("file1.aaa"),&temp);
	test(r==KErrNotFound);
	r=finder.Find();
	test(r==KErrNotFound);
//
	TPtrC path=_L("blarg.7");
	r=finder.FindByPath(_L(""),&path);	
	test(r==KErrArgument);
	r=finder.FindByPath(_L("*"),&path);
	test(r==KErrNotFound);
	r=finder.FindByPath(_L("xmvid"),&path);
	test(r==KErrNotFound);
	r=finder.Find();
	test(r==KErrNotFound);
//
	path.Set(_L("C:\\F32-TST\\LOCTEST\\BIN1\\;\\F32-TST\\LOCTEST\\BIN2\\;Z:\\F32-TST\\LOCTEST\\BIN1\\BIN4\\;\\F32-TST\\LOCTEST\\BIN3\\;"));
	r=finder.FindByPath(_L(""),&path);
	test(r==KErrArgument);
	r=finder.FindByPath(_L("xyz.abc"),&path);
	test(r==KErrNotFound);
	r=finder.Find();
	test(r==KErrNotFound);
	
	test.Next(_L("Test FindByDir with empty file spec"));
	TPtrC dir2=_L("\\F32-TST\\LOCTEST\\");
	r=finder.FindByDir(_L(""),dir2);
	test(r==KErrArgument);		
	
	}

LOCAL_C void Test3()
//
// Test FindByDrives in a path=_L("c:\xyz;z:\lmnop;\abc;\y:\help");
//
	{

	test.Next(_L("Test FindInDrivesByPath"));
	TPtrC path=_L("\\F32-TST\\LOCTEST\\BIN2\\");
	TFileName defaultPath;
	TInt r=TheFs.SessionPath(defaultPath);
	defaultPath.SetLength(2);
//
	TAutoClose<RFs> fs;
	r=fs.iObj.Connect();
	test(r==KErrNone);
	TFindFile finder(fs.iObj);
	r=finder.FindByPath(_L("file1.aaa"),&path);
	test(r==KErrNotFound);
	r=finder.Find();
	test(r==KErrNotFound);
//
	path.Set(_L("\\F32-TST\\LOCTEST\\BIN2\\"));
	r=finder.FindByPath(_L("file2.bbb"),&path);
	test(r==KErrNone);
	TParse fileParse;
	fileParse.Set(finder.File(),NULL,NULL);
	test(fileParse.Drive()==defaultPath);
	test(fileParse.Path()==_L("\\F32-TST\\LOCTEST\\BIN2\\"));
	test(fileParse.NameAndExt()==_L("file2.bbb"));
	r=finder.Find();
	test(r==KErrNotFound || r==KErrNone);
	if (r==KErrNone)
		{
		fileParse.Set(finder.File(),NULL,NULL);
		test(fileParse.Drive()!=defaultPath);
		test(fileParse.Path()==_L("\\F32-TST\\LOCTEST\\BIN2\\"));
		test(fileParse.NameAndExt()==_L("file2.bbb"));
		r=finder.Find();
		test(r==KErrNotFound);
		}
//
	path.Set(_L("C:\\F32-TST\\LOCTEST\\BIN1\\;;\\F32-TST\\LOCTEST\\BIN2\\;Z:\\F32-TST\\LOCTEST\\BIN1\\BIN4\\;\\F32-TST\\LOCTEST\\BIN3\\;"));
	r=finder.FindByPath(_L("xyz.abc"),&path);
	test(r==KErrNotFound);
	r=finder.Find();
	test(r==KErrNotFound);
//
	r=finder.FindByPath(_L("file2.bbb"),&path);
	test(r==KErrNone);
	fileParse.Set(finder.File(),NULL,NULL);
	test(fileParse.Drive()==defaultPath);
	test(fileParse.Path()==_L("\\F32-TST\\LOCTEST\\BIN2\\"));
	test(fileParse.NameAndExt()==_L("file2.bbb"));
	r=finder.Find();
	test(r==KErrNotFound || r==KErrNone);
	if (r==KErrNone)
		{
		fileParse.Set(finder.File(),NULL,NULL);
		test(fileParse.Drive()!=defaultPath);
		test(fileParse.Path()==_L("\\F32-TST\\LOCTEST\\BIN2\\"));
		test(fileParse.NameAndExt()==_L("file2.bbb"));
		r=finder.Find();
		test(r==KErrNotFound);
		}
	}

LOCAL_C void Test4()
//
// Test wildcard findbypath
//
	{

	test.Next(_L("FindByPath with wild filenames"));
	TFindFile finder(TheFs);
	CDir* dir;
	TInt count;
	TEntry entry;
	TFileName path;

	TInt r=finder.FindWildByPath(_L("*.aaa"),&gPath3,dir);
	test(r==KErrNone);
	count=dir->Count();
	test(count==3);
	entry=(*dir)[0];
	test(entry.iName.MatchF(_L("FILE.AAA"))!=KErrNotFound);
	entry=(*dir)[1];
	test(entry.iName.MatchF(_L("FILE1.AAA"))!=KErrNotFound);
	entry=(*dir)[2];
	test(entry.iName.MatchF(_L("WORK.AAA"))!=KErrNotFound);
	TParse fileParse;
	fileParse.Set(finder.File(),NULL,NULL);
	path=fileParse.FullName();
	test(path==_L("*.aaa"));
	delete dir;

	r=finder.FindWild(dir);
	test(r==KErrNone);
	count=dir->Count();
	test(count==2);
	entry=(*dir)[0];
	test(entry.iName.MatchF(_L("FILE1.AAA"))!=KErrNotFound);
	entry=(*dir)[1];
	test(entry.iName.MatchF(_L("WORK.AAA"))!=KErrNotFound);
	fileParse.Set(finder.File(),NULL,NULL);
	path=fileParse.FullName();
	test(path==_L("C:\\F32-TST\\LOCTEST\\BIN1\\*.aaa"));
	delete dir;

	r=finder.FindWild(dir);
	test(r==KErrNotFound);
	r=finder.FindWild(dir);
	test(r==KErrNotFound);

	r=finder.FindWildByPath(_L("*FILE.AAA*"), &gPath1, dir);
	test(r==KErrNone);
	test(dir->Count()==1);
	entry=(*dir)[0];
	test(entry.iName.MatchF(_L("FILE.AAA"))!=KErrNotFound);
	delete dir;
	r=finder.FindWildByPath(_L("*FILE.AAA"), &gPath1, dir);
	test(r==KErrNone);
	test(dir->Count()==1);
	entry=(*dir)[0];
	test(entry.iName.MatchF(_L("FILE.AAA"))!=KErrNotFound);
	delete dir;
	r=finder.FindWildByPath(_L("FILE.AAA*"), &gPath1, dir);
	test(r==KErrNone);
	test(dir->Count()==1);
	entry=(*dir)[0];
	test(entry.iName.MatchF(_L("FILE.AAA"))!=KErrNotFound);
	delete dir;
	r=finder.FindWildByPath(_L("CONFUSED.DOG"), &gPath1, dir);
	test(r==KErrNone);
	test(dir->Count()==1);
	entry=(*dir)[0];
	test(entry.iName.MatchF(_L("CONFUSED.DOG"))!=KErrNotFound);
	delete dir;
	r=finder.FindWildByPath(_L("*CONFUSED.DOG"), &gPath1, dir);
	test(r==KErrNone);
	test(dir->Count()==1);
	entry=(*dir)[0];
	test(entry.iName.MatchF(_L("CONFUSED.DOG"))!=KErrNotFound);
	delete dir;
	r=finder.FindWildByPath(_L("CONFUSED.DOG*"), &gPath1, dir);
	test(r==KErrNone);
	test(dir->Count()==1);
	entry=(*dir)[0];
	test(entry.iName.MatchF(_L("CONFUSED.DOG"))!=KErrNotFound);
	delete dir;
	r=finder.FindWildByPath(_L("*CONFUSED.DOG*"), &gPath1, dir);
	test(r==KErrNone);
	test(dir->Count()==1);
	entry=(*dir)[0];
	test(entry.iName.MatchF(_L("CONFUSED.DOG"))!=KErrNotFound);
	delete dir;
	}

LOCAL_C void Test5()
//
// Test wildcard findbydir
//
	{

	test.Next(_L("FindByDir with wild filenames"));
	TFindFile finder(TheFs);
	CDir* dir;
	TInt count;
	TEntry entry;
	TFileName path;

	TInt r=finder.FindWildByDir(_L("FILE*"),_L("\\F32-TST\\LOCTEST\\BIN3\\"),dir);
	test(r==KErrNone);
	count=dir->Count();
	test(count==1);
	entry=(*dir)[0];
	test(entry.iName.MatchF(_L("FILE3.CCC"))!=KErrNotFound);
	TParse fileParse;
	fileParse.Set(finder.File(),NULL,NULL);
	path=fileParse.FullName();
	TFileName tpath=_L("?:\\F32-TST\\LOCTEST\\BIN3\\FILE*");
    tpath[0]=gSessionPath[0];
	test(path.CompareF(tpath)==0);
	delete dir;

	r=finder.FindWild(dir);
	if (r==KErrNotFound)
		return;
	test(r==KErrNone);
	entry=(*dir)[0];
	test(entry.iName.MatchF(_L("FILE3.CCC"))!=KErrNotFound);
	fileParse.Set(finder.File(),NULL,NULL);
	path=fileParse.FullName();
	test(path.CompareF(tpath)==0);
	delete dir;

	r=finder.FindWild(dir);
	test(r==KErrNotFound);
	r=finder.FindWild(dir);
	test(r==KErrNotFound);
	}

LOCAL_C void Test6()
//
// Test file not found
//
	{

	test.Next(_L("Test file not found"));
	TFindFile ff(TheFs);
	TInt r=ff.FindByDir(_L("NOEXIST.EXE"),_L("\\System\\Programs\\"));
	test(r==KErrNotFound);
	}




//  The following test has the requirement that the only remote drive is the one we mount 
//  during the test(DELAYFS) and which doesn't have any other attributes set. If this is not the
//  case then test conditions must be changed, in order for the test to stop failing.
// 	Even more if a removable drive is not present in the target platform then findfile.aaa
//	only exists in the remote one and this is why we have a distinction in the test results.
//



  	//--------------------------------------------- 
	//! @SYMTestCaseID			PBASE-T_LOCATE-0553
	//! @SYMTestType			UT 
	//! @SYMREQ					CR909
	//! @SYMTestCaseDesc		When using the various Find functions of class TFindFile,by default remote drives are 
	//!							excluded from the list of drives that are searched. Using function 
	//!							SetFindMask(TUint aMask) it is possible to specify a combination of attributes that
	//!						    the drives to be searched must match.
	//! @SYMTestActions			Call function FindByPath/Find without specifying a mask. Check that remote drives are not 
	//!							included. Then call SetFindMask(TUint aMask) using various combinations and verify 
	//!							that FindByPath or Find return appopriate results.
	//! @SYMTestExpectedResults	Test that file findfile.aaa is found or not depending on the specified mask.
	//! @SYMTestPriority		High
	//! @SYMTestStatus			Implemented 
	//--------------------------------------------- 



LOCAL_C void Test7()

	{
	
	TAutoClose<RFs> fs;
	TInt r=fs.iObj.Connect();
	test(r==KErrNone);
	TFindFile finder(fs.iObj);
	TPtrC path=gPath4;
	r=finder.FindByPath(_L("findfile.aaa"),&path); 	
	
	TParse fileParse;
	
	test.Next(_L("Test FindByPath without specifying any mask"));
	
	if (removableFlag == 1)
		{
			test(r==KErrNone); 
			fileParse.Set(finder.File(),NULL,NULL);
			test(fileParse.Path()==_L("\\F32-TST\\LOCTEST\\BIN\\"));
			test(fileParse.NameAndExt()==_L("findfile.aaa")); //The filename.aaa in the removable Drive
			r=finder.Find();
			test(r==KErrNotFound);     //remote drives are excluded by default
		
		}
	else
		test(r==KErrNotFound);

	

	test.Next(_L("Search for the specified file in all Drives, including remotes ones \n"));


	r=finder.SetFindMask(	KDriveAttAll) ;
	test(r==KErrNone);
	r=finder.FindByPath(_L("findfile.aaa"),&path);
	test(r==KErrNone);
	fileParse.Set(finder.File(),NULL,NULL);
	test(fileParse.Path()==_L("\\F32-TST\\LOCTEST\\BIN\\"));   
	test(fileParse.NameAndExt()==_L("findfile.aaa"));      //either the remote or removable one.
	r=finder.Find();
	
	
	if (removableFlag == 1)
		{	
		test(r==KErrNone);
	
		fileParse.Set(finder.File(),NULL,NULL);

		test(fileParse.Path()==_L("\\F32-TST\\LOCTEST\\BIN\\"));
		test(fileParse.NameAndExt()==_L("findfile.aaa"));         //either the remote or removable one.

		r=finder.Find();
		test(r==KErrNotFound);
		}
	else 
		{
		test(r==KErrNotFound);	
			
		}
		
		
	test.Next(_L("Search exclusively in remote drives \n"));

	r=finder.SetFindMask(	KDriveAttExclusive| KDriveAttRemote); 
	test(r==KErrNone);
	r=finder.FindByPath(_L("findfile.aaa"),&path);
	test(r==KErrNone);
	fileParse.Set(finder.File(),NULL,NULL);
	test(fileParse.Path()==_L("\\F32-TST\\LOCTEST\\BIN\\"));
	test(fileParse.NameAndExt()==_L("findfile.aaa"));
	r=finder.Find();
	test(r==KErrNotFound);

	
	test.Next(_L("Search excluding removables and remote \n"));

	r=finder.SetFindMask(	KDriveAttExclude | KDriveAttRemovable |KDriveAttRemote ); 
	test(r==KErrNone);
	r=finder.FindByPath(_L("findfile.aaa"),&path);   
	test(r==KErrNotFound);   //filename.aaa exists in the remote drive and if present to the removable one


	test.Next(_L("Search in Internal Drives \n"));

	r=finder.SetFindMask(KDriveAttInternal ) ;
	test(r==KErrNone);
	r=finder.FindByPath(_L("findfile.aaa"),&path);   
	test(r==KErrNotFound);   //filename.aaa exists only in the Removable drive and the remote one.


	}

	
	 //--------------------------------------------- 
	//! @SYMTestCaseID			PBASE-T_LOCATE-0554
	//! @SYMTestType			UT 
	//! @SYMREQ					CR909
	//! @SYMTestCaseDesc		Test that SetFindMask(TUint aMask) returns the correct value for all combinations of matching masks.						
	//!							
	//! @SYMTestActions			Call SetFindMask for every combination of mask and check that the correct value is returned.   
	//!							A structure is used to store the expected value for each combination.
	//! @SYMTestExpectedResults	For every combination either KErrNone or KErrArgument must be returned.
	//! @SYMTestPriority		High.
	//! @SYMTestStatus			Implemented 
	//--------------------------------------------- 
	



LOCAL_C void Test8()

	{

	test.Next(_L("Test SetFindMask with all mask combinations \n"));	
	
	
	TAutoClose<RFs> fs;
	TInt r=fs.iObj.Connect();
	test(r==KErrNone);
	TFindFile finder(fs.iObj);
	TPtrC path=gPath4;	
	TParse fileParse;
		

	r=finder.SetFindMask(KDriveAttAll) ;
	test(r==KErrNone);
	r=finder.FindByPath(_L("findfile.aaa"),&path);
	test(r==KErrNone);
	fileParse.Set(finder.File(),NULL,NULL);
	test(fileParse.Path()==_L("\\F32-TST\\LOCTEST\\BIN\\"));   
	test(fileParse.NameAndExt()==_L("findfile.aaa"));


	struct TCombinations
		{
		TUint iMatchMask;			  // The Match Mask to be combined with drive attributes
		TInt  iExpectedResultNoAtts;	  // Expected result when flag used on it's own
		TInt  iExpectedResultWithAtts;  // Expected result when flag used in combination with drive flags
		};

	TCombinations testCombinations[] = {
		{ 0,														KErrNone,     KErrNone},
		{ KDriveAttAll,												KErrNone,     KErrArgument },
		{ KDriveAttExclude,											KErrArgument, KErrNone },
		{ KDriveAttExclusive,										KErrArgument, KErrNone },
		{ KDriveAttExclude | KDriveAttExclusive,					KErrArgument, KErrNone },
		{ KDriveAttAll	   | KDriveAttExclude,						KErrArgument, KErrArgument },
		{ KDriveAttAll     | KDriveAttExclusive,					KErrArgument, KErrArgument},
		{ KDriveAttAll     | KDriveAttExclude | KDriveAttExclusive, KErrArgument, KErrArgument}};


	
	for(TUint matchIdx = 0; matchIdx < sizeof(testCombinations) / sizeof(TCombinations); matchIdx++)
		{
		test.Printf(_L("\nTest mask : KDriveAttAll[%c] KDriveAttExclude[%c] KDriveAttExclusive[%c]\n"), testCombinations[matchIdx].iMatchMask & KDriveAttAll       ? 'X' : ' ',
																										 testCombinations[matchIdx].iMatchMask & KDriveAttExclude   ? 'X' : ' ',
																										 testCombinations[matchIdx].iMatchMask & KDriveAttExclusive ? 'X' : ' ');
		for(TUint testAtt = 0; testAtt <= KMaxTUint8; testAtt++)
			{
			r= finder.SetFindMask( testCombinations[matchIdx].iMatchMask | testAtt ) ;
	 		
		//	test.Printf(_L("            ATT : 0x%08x \n"), testAtt);
		//	test.Printf(_L("Expected Result : %d     \n"), testAtt == 0 ? testCombinations[matchIdx].iExpectedResultNoAtts : testCombinations[matchIdx].iExpectedResultWithAtts);
		//	test.Printf(_L("  Actual Result : 0x%08x \n"), err);
		
		//	test.Printf(_L("\nTest mask : %d \n"),testCombinations[matchIdx].iMatchMask | testAtt );
			
			test( r == (testAtt == 0 ? testCombinations[matchIdx].iExpectedResultNoAtts : testCombinations[matchIdx].iExpectedResultWithAtts) );
			
			
			if (r== KErrNone)
				{
				r  = finder.FindByPath(_L("findfile.aaa"),&path);
				test (r== KErrNone || r ==KErrNotFound);
				}
			
			}
		}

	}
	
	



LOCAL_C void Test9()
//
// Test wildcard findbydir and FindByWildPath in Removable and Internal Drives
//

	//--------------------------------------------- 
	//! @SYMTestCaseID			PBASE-T_LOCATE-0555
	//! @SYMTestType			UT 
	//! @SYMREQ					CR909
	//! @SYMTestCaseDesc		Check that FindWildByDir and FindByDir functions behave correctly when a mask has been specified 
	//! 						through SetFindMask.
	//! @SYMTestActions			Call FindWildByDir with a filename containing wildchars and a specific path. Then call SetFindMask
	//!							to exclude Removable drives and call FindWildByDir again.Even more call FindByDir for the file in
	//!							the removable drive and for the same directory as before. 
	//! @SYMTestExpectedResults The number of files found when excluding the removable drive(if a removable drive exists in the
	//!							target platform) must differ by one. The FinByDir must find the same results.
	//! @SYMTestPriority		High
	//! @SYMTestStatus			Implemented 
	//--------------------------------------------- 


	{		

	TAutoClose<RFs> fs;
	TInt r=fs.iObj.Connect();
	test(r==KErrNone);
	
	TFindFile finder(fs.iObj);
	
	CDir* dir;
	CDir* dir3;
	
	TInt count;
	TEntry entry;	



	if ( removableFlag == 1 )
		{
		
		test.Next(_L("FindByDir with wild filenames when a find mask is specified"));
		
		TInt r=finder.SetFindMask(KDriveAttRemovable);
		test(r==KErrNone);
		r=finder.FindWildByDir(_L("FIND*"),_L("\\F32-TST\\LOCTEST\\BIN\\"),dir);
		test(r==KErrNone);
		count=dir->Count();
		test(count==1);
		entry=(*dir)[0];
		test(entry.iName.MatchF(_L("FINDFILE.AAA"))!=KErrNotFound); 
		delete dir;	

		r=finder.FindWild(dir);
		test(r==KErrNotFound);	
		
		
		r=finder.SetFindMask(KDriveAttExclude| KDriveAttRemovable);
		test(r==KErrNone);
		r=finder.FindWildByDir(_L("FIND*"),_L("\\F32-TST\\LOCTEST\\BIN\\"),dir);
		test(r==KErrNotFound);
		
		
		test.Next(_L("Test FindByDir when a find mask is specified"));
		
		
		TPtrC dir2=_L("\\F32-TST\\LOCTEST\\BIN\\");
		
		r=finder.SetFindMask(KDriveAttExclude | KDriveAttRemote );
		test(r==KErrNone);
		r=finder.FindByDir(_L("findfile.aaa"),dir2);
		test(r==KErrNone);
	
		r=finder.Find();
		test(r==KErrNotFound);	
		

		}
	


	//--------------------------------------------- 
	//! @SYMTestCaseID			PBASE-T_LOCATE-0556
	//! @SYMTestType			UT 
	//! @SYMREQ					CR909
	//! @SYMTestCaseDesc		FindByWildPath and FindByPath functions when supplied with a path that also contains 
	//!							a Drive letter, they will not need to check other Drives. Therefore calling SetFindMask 
	//!							does not affect the drives returned. 
	//! @SYMTestActions			Call FindWildByPath with an appropriate path in the internal drive. Then call SetFindMask
	//!							to exclude Internal drives and call FindWildByPath again.
	//! @SYMTestExpectedResults The number of files found in both cases must be the same since no other drive is searched.
	//! @SYMTestPriority		High
	//! @SYMTestStatus			Implemented 
	//--------------------------------------------- 




	if( internalFlag == 1 )
		{
		
		
		test.Next(_L("Test that SetFindMask does not affect Find functions that have a drive letter specified"));
		

		gPath5.Append (internalDriveLetter);
		gPath5.Append (_L(":\\F32-TST\\LOCTEST\\BIN\\INT\\") );

		
		r=finder.FindWildByPath(_L("FIND*.AAA"), &gPath5, dir3);
 		test(r==KErrNone);
		test(dir3->Count()==2);
		
		entry=(*dir3)[0];		
		test(  (entry.iName.MatchF(_L("FINDINTERNALFILE_B.AAA"))!=KErrNotFound)  || (entry.iName.MatchF(_L("FINDINTERNALFILE.AAA"))!=KErrNotFound)  );
		
				
		entry=(*dir3)[1];
		test(  (entry.iName.MatchF(_L("FINDINTERNALFILE_B.AAA"))!=KErrNotFound )  || (entry.iName.MatchF(_L("FINDINTERNALFILE.AAA"))!=KErrNotFound)  );		

		
		delete dir3;
		
		
		
		r=finder.SetFindMask(KDriveAttExclude| KDriveAttInternal);
		test(r==KErrNone);
		r=finder.FindWildByPath(_L("FIND*.AAA"), &gPath5, dir3);	
 		test(r==KErrNone);
		test(dir3->Count()==2);
		
		delete dir3;
		
		
		r=finder.FindWild(dir3);
		test(r==KErrNotFound);
				
				
		}


	}




GLDEF_C void CallTestsL()
//
// Do all tests
//
	{

		
		gPath3=_L("C:\\F32-TST\\LOCTEST\\BIN1\\;C:\\F32-TST\\LOCTEST\\BIN2\\");

		gPath1=_L("");
		gPath1.Append(gSessionPath[0]);
		gPath1.Append(_L(":\\F32-TST\\LOCTEST\\BIN1\\;"));
		gPath1.Append(gSessionPath[0]);
		gPath1.Append(_L(":\\F32-TST\\LOCTEST\\BIN2\\"));
	
		gPath2=gPath1;
		gPath2.Append(';');
		gPath2.Append(gSessionPath[0]);
		gPath2.Append(_L(":\\F32-TST\\LOCTEST\\BIN1\\BIN4\\;"));
		if (gSessionPath[0]!='C')
			gPath2.Append(gSessionPath.Left(2));
		gPath2.Append(_L("\\F32-TST\\LOCTEST\\BIN3\\;"));
		
		gPath4=_L("");
		gPath4.Append(_L("\\F32-TST\\LOCTEST\\BIN\\"));
		
	

		CreateTestDirectory(_L("\\F32-TST\\LOCTEST\\"));
		MakeLocateTestDirectoryStructure();
		Test1();
		Test2();
		Test3();
		Test4();
		Test5();
		Test6();
	
		MountRemoteFilesystem();      
		CreateFilesInRemovableDrive();  //used in Test7/8/9   	
	
		Test7();
		Test8();
	
		CreateFilesInInternalDrive();  //used in Test9
		Test9();
			
		DisMountRemoteFilesystem();	
		
		DeleteTestDirectory();
		
		//Explicity delete the directories created
		DeleteRemovableDirectory();
		DeleteInternalDirectory();


	
	}
