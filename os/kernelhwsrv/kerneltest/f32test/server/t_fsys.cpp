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
// f32test\server\t_fsys.cpp
// 
//

#define __E32TEST_EXTENSION__

#include <f32file.h>
#include <f32file_private.h>
#include <e32test.h>
#include "t_server.h"
#include "fat_utils.h"
#include "filesystem_fat.h"

using namespace Fat_Test_Utils;

RTest test(_L("T_FSYS"));

static void TestFileSystemNames()
    {
    test.Next(_L("Read file system names for all drives"));
    TFullName name;
    TInt r;
    for(TInt i=EDriveA;i<KMaxDrives;++i)
        {
        r=TheFs.FileSystemName(name,i);
        test(r==KErrNone || r==KErrNotFound);
        TChar c;
        r=RFs::DriveToChar(i,c);
        test(r==KErrNone);
        if(name.Length())       
            test.Printf(_L("File System Name on drive %c is %S\n"),(char)c,&name);
        else
            test.Printf(_L("No file system on drive %c\n"),(char)c);
        }
    }

static void CheckDismount(TDesC& aFs,TInt aDrive)
    {

    if (aDrive==EDriveC) // ??? Can't test on C: - see below
        return;
    TInt r;
    TFullName oldSess, newSess;
    r=TheFs.SessionPath(oldSess);
    test(r==KErrNone);
    TChar c;
    r=TheFs.DriveToChar(aDrive,c);
    test(r==KErrNone);
    newSess.Append(c);
    newSess.Append(':');
    newSess.Append('\\');

    TBuf<128> b;
    TDriveInfo di;
    r=TheFs.Drive(di,aDrive);
    test(r==KErrNone);
    b.Format(_L("Test dismounting of test file system on %c: (DrvAtt:%x MedAtt:%x)"),(TUint)c,di.iDriveAtt,di.iMediaAtt);
    test.Next(b);
    
    // Test cannot dismount on rom drive
    test.Next(_L("Test cannot dismount on Rom drive"));
    TFullName zName;
    r=TheFs.FileSystemName(zName,EDriveZ);
    test(r==KErrNone);
    r=TheFs.DismountFileSystem(zName,EDriveZ);
    test.Printf(_L("r=%d"),r);
    // NB if paging is enabled on a ROFS partition which is part of the composite file system then the 
    // likelihood is that there will be a at least one file clamped: in this case there error will be KErrInUse
    test(r==KErrAccessDenied || r==KErrInUse);

    // Test cannot dismount on wrong drive
    test.Next(_L("Test cannot dismount on wrong drive"));
    r=TheFs.DismountFileSystem(aFs,EDriveA);
    test(r==KErrNotReady);

    // Test cannot dismount with wrong name
    test.Next(_L("Test cannot dismount with wrong file system name"));
    r=TheFs.DismountFileSystem(_L("abc"),aDrive);
    test(r==KErrNotFound);
 
    // Test cannot dismount with a file open
    test.Next(_L("Test cannot dismount with a file open"));
    r=TheFs.SetSessionPath(newSess);
    RFile file;
    r=file.Replace(TheFs,_L("abc"),EFileShareAny);
    test(r==KErrNone);
    r=TheFs.SessionPath(newSess);
    TBool open;
    r=TheFs.IsFileOpen(_L("abc"),open);
    test(r==KErrNone);
    test(open);
    r=TheFs.DismountFileSystem(aFs,aDrive);
    test(r==KErrInUse);
    file.Close();

    // Now test dismount works
    test.Next(_L("Test dismounts OK"));
    r=TheFs.DismountFileSystem(aFs,aDrive);
    if(r!=KErrNone)
        {
        test.Printf(_L("Error = %d"),r);    
        test(EFalse);
        }
    TFullName n;
    r=TheFs.FileSystemName(n,aDrive);
    test(r==KErrNone || r==KErrNotFound);
    test(!n.Length());
    r=file.Replace(TheFs,_L("abc"),EFileShareAny);
    test(r==KErrNotReady);
    file.Close();

    r=TheFs.MountFileSystem(aFs,aDrive);
    if(r!=KErrNone) 
        {
        test.Printf(_L("error = %d\n"),r);
        test(EFalse);
        }
    r=TheFs.FileSystemName(n,aDrive);
    test(r==KErrNone);
    test(n.Compare(aFs)==0);
    r=file.Replace(TheFs,_L("abc"),EFileShareAny); // ??? bang
    test(r==KErrNone);
    file.Close();
    r=TheFs.SetSessionPath(oldSess);
    test(r==KErrNone);
    }

static void TestDismountFileSystem(TInt aDrive)
    {

    TInt r;
    TFullName name;
    r=TheFs.FileSystemName(name,aDrive);
    test(r==KErrNone || r==KErrNotFound);
    if(name.Length())
        CheckDismount(name,aDrive);
    }

#if defined(__EPOC32__)
static void TestFileSystem(TInt aDrive)
//
// Mount a new CTestFileSystem on the drive under test
//
    {
    TBuf<64> b;
    TChar c;
    TInt r=TheFs.DriveToChar(aDrive,c);
    test(r==KErrNone);
    TDriveInfo di;
    r=TheFs.Drive(di,aDrive);
    test(r==KErrNone);
    b.Format(_L("Test mounting of test file system on %c: (DrvAtt:%x MedAtt:%x)"),(TUint)c,di.iDriveAtt,di.iMediaAtt);
    test.Next(b);

    test.Next(_L("Test mounting of test file system"));
    r=TheFs.AddFileSystem(_L("T_TFSYS"));
    if(r!=KErrNone && r!=KErrAlreadyExists)
        {
        test.Printf(_L("error=%d"),r);
        test(EFalse);
        }

    TFullName oldFs;
    r=TheFs.FileSystemName(oldFs,aDrive);
//  TFileName oldFs;
//  r=TheFs.FileSystemName(oldFs,aDrive);
    test(r==KErrNone);
    r=TheFs.DismountFileSystem(oldFs,aDrive);
    if(r!=KErrNone)
        {
        test.Printf(_L("Error = %d"),r);    
        test(EFalse);
        }
    r=TheFs.MountFileSystem(_L("Test"),aDrive);
    test(r==KErrNone);

    TFileName newFs;
    r=TheFs.FileSystemName(newFs,aDrive);
    test(r==KErrNone);
    test(newFs.Compare(_L("Test"))==0);

    // Check attributes
    TDriveInfo info;
    r=TheFs.Drive(info,aDrive);
    test(r==KErrNone);
 
    test.Printf(_L("iType=%d,iConnectionBusType=%d,iDriveAtt=%x,iMediaAtt=%x\n"),(TUint)info.iType,\
        (TUint)info.iConnectionBusType,info.iDriveAtt,info.iMediaAtt);

    //Try to remove filesystem without dismounting.
    r=TheFs.RemoveFileSystem(_L("Test"));
    if(r!=KErrInUse)
        {
        test.Printf(_L("error=%d"),r);
        test(EFalse);
        }
    r=TheFs.FileSystemName(newFs,aDrive);
    test(r==KErrNone);
    test(newFs.Compare(_L("Test"))==0);

    r=TheFs.DismountFileSystem(newFs,aDrive);
    test(r==KErrNone);

    r=TheFs.MountFileSystem(oldFs,aDrive);
    test(r==KErrNone);
    }
#endif

static void TestMountInvalidDrive()
//
// Attempt to mount FAT on non-local drive
    {
    test.Start(_L("TestMountInvalidDrive"));

    TInt r;

    test.Next(_L("Adding EFAT"));
#ifdef __WINS__
    _LIT(KFsNm, "EFAT32");
#else
    _LIT(KFsNm, "ELOCAL");
#endif

    r = TheFs.AddFileSystem(KFsNm);
    test.Printf(_L("afs: r = %d\n"), r);
    test(r == KErrNone || r == KErrAlreadyExists);
    test.Next(_L("mounting FAT on drive R"));
    r = TheFs.MountFileSystem(KFileSystemName_FAT, EDriveR);
    test(r == KErrArgument);
    
    test.End();
    }

// Additional step for INC083446: Corrupted miniSD not detected as corrupted by phone 
static void TestMountingBrokenMedia(TInt aDrive)
//
// Mount a new CTestFileSystem on the drive under test
//
    {
    if (aDrive==EDriveC) // ??? Can't test on C:
        return;

    TBuf<64> b;
    TChar c;
    TInt r=TheFs.DriveToChar(aDrive,c);
    test(r==KErrNone);
    TDriveInfo di;
    r=TheFs.Drive(di,aDrive);
    test(r==KErrNone);
    b.Format(_L("Test mounting of test file system on %c: (DrvAtt:%x MedAtt:%x)"),(TUint)c,di.iDriveAtt,di.iMediaAtt);
    test.Next(b);

    test.Next(_L("Test mounting of test file system"));
    r=TheFs.AddFileSystem(_L("T_TFSYS2"));
    if(r!=KErrNone && r!=KErrAlreadyExists)
        {
        test.Printf(_L("error=%d"),r);
        test(EFalse);
        }

    TFullName oldFs;
    r=TheFs.FileSystemName(oldFs,aDrive);
    test(r==KErrNone);
    r=TheFs.DismountFileSystem(oldFs,aDrive);
    if(r!=KErrNone)
        {
        test.Printf(_L("Error = %d"),r);    
        test(EFalse);
        }
    r=TheFs.MountFileSystem(_L("Test2"),aDrive);
    test(r == KErrCorrupt);

    TFileName newFs;
    r=TheFs.FileSystemName(newFs,aDrive);
    test(r==KErrNone);
    test(newFs.Compare(_L("Test2"))==0);

    // Get the number of remounts by checking the volume attributes -
    // T_TFSYS2 hijacks the iBattery member to report back the number of times MountL() has been called
    TDriveInfo info;
    TInt remounts;
    r=TheFs.Drive(info,aDrive);
    test(r==KErrNone);
    test.Printf(_L("iType=%d,iBattery=%d,iDriveAtt=%x,iMediaAtt=%x\n"),(TUint)info.iType,\
        (TUint)info.iBattery,info.iDriveAtt,info.iMediaAtt);
    remounts = (TInt) info.iBattery;
    test.Printf(_L("Initial remounts = %d"), remounts);

    // Make the file server attempt to remount the drive by looking for a non-existant DLL
    // The file server should setop trying to remount the driver after KMaxMountFailures attempts
    const TInt KMaxMountFailures = 3;   // copied from sf_drv.cpp
    const TInt KEntryAttempts = 10;
    TInt entryAttempts;
    for (entryAttempts=0; entryAttempts < KEntryAttempts; entryAttempts++)
        {
        TEntry entry;
        _LIT(KNonExistantFilename, "NONEXISTANT_FILENAME.DLL");
        r = TheFs.Entry(KNonExistantFilename, entry);
        test(r == KErrCorrupt);
        }
    r=TheFs.Drive(info,aDrive);
    test(r==KErrNone);
    test.Printf(_L("iType=%d,iBattery=%d,iDriveAtt=%x,iMediaAtt=%x\n"),(TUint)info.iType,\
        (TUint)info.iBattery,info.iDriveAtt,info.iMediaAtt);
    remounts = (TInt) info.iBattery;
    test.Printf(_L("Remounts = %d"), remounts);
    test(remounts ==  KMaxMountFailures);
    
    // simulate a media change to reset failure count
    r = TheFs.RemountDrive(aDrive, NULL, 0);

    // now try mounting again & verify the the file server attempts to mount the drive again
    for (entryAttempts=0; entryAttempts < KEntryAttempts; entryAttempts++)
        {
        TEntry entry;
        _LIT(KNonExistantFilename, "NONEXISTANT_FILENAME.DLL");
        r = TheFs.Entry(KNonExistantFilename, entry);
        test(r == KErrCorrupt);
        }
    r=TheFs.Drive(info,aDrive);
    test(r==KErrNone);
    test.Printf(_L("iType=%d,iBattery=%d,iDriveAtt=%x,iMediaAtt=%x\n"),(TUint)info.iType,\
        (TUint)info.iBattery,info.iDriveAtt,info.iMediaAtt);
    remounts = (TInt) info.iBattery;
    test.Printf(_L("Remounts = %d"), remounts);
    test(remounts ==  KMaxMountFailures * 2);
    


    r=TheFs.DismountFileSystem(newFs,aDrive);
    test(r==KErrNone);
    r=TheFs.MountFileSystem(oldFs,aDrive);
    test(r==KErrNone);
    
    r=TheFs.RemoveFileSystem(_L("Test2"));
    if(r!=KErrNone)
        {
        test.Printf(_L("error=%d"),r);
        test(EFalse);
        }
    }


/**
    Testing obtaining media serial number for the substituted drives
*/
static void TestSubstDriveMediaSerialNumber()
{
    test.Next(_L("Test obtaining media serial number for the substituted drives"));

    TInt  nRes;
    const TInt currDrvNum=CurrentDrive();
    
    TDriveInfo drvInfo;
    nRes=TheFs.Drive(drvInfo, currDrvNum);
    test(nRes==KErrNone);

    if(drvInfo.iDriveAtt & (KDriveAttRom | KDriveAttRedirected | KDriveAttSubsted))
    {
        test.Printf(_L("Can't test on this drive!\n"));
        return;
    }

    TMediaSerialNumber serNum;

    //-- test Media Serial Number on unexisting drive
    {
    for(TInt drvNum=EDriveA; drvNum<=EDriveZ; ++drvNum)
        {
        TDriveInfo drvInfo;
        if(KErrNone==TheFs.Drive(drvInfo, drvNum) && drvInfo.iType==EMediaNotPresent)
            {
            // found a non-extant drive, test it...
            nRes = TheFs.GetMediaSerialNumber(serNum, drvNum);
            test(nRes == KErrNotReady);
            break;
            }
        }
    }
    
    nRes = TheFs.GetMediaSerialNumber(serNum, currDrvNum);
    if(nRes != KErrNone)
    {
        test.Printf(_L("Test is inconsintent on this drive!\n"));
        return;
    }

    TFileName substPath;                //-- path to the directory to substitute
    const TInt KSubstDrv = EDriveO;     //-- drive to be substituted

    //-- make directory, which will be substituted ad a drive
    substPath.Format(_L("%c:\\SubstDrv1\\"), (TUint8)'A'+currDrvNum);
    MakeDir(substPath);
  
    nRes = TheFs.SetSubst(substPath, KSubstDrv);
    test(nRes == KErrNone);

    //-- an attempt to obtain Media Serial Number on a substed drive shall result in KErrNotSupported
    nRes = TheFs.GetMediaSerialNumber(serNum, KSubstDrv);
    test(nRes == KErrNotSupported);

    //-- delete substed drive
    nRes = TheFs.SetSubst(_L(""), KSubstDrv);
    test(nRes == KErrNone);
}


//----------------------------------------------------------------------------------------------
//! @SYMTestCaseID      PBASE-t_fsys-0317
//! @SYMTestType        CIT
//! @SYMPREQ            CR0882
//! @SYMTestCaseDesc    This test case is testing querying file system sub type name using 
//!                     RFs::QueryVolumeInfoExt() API.
//! @SYMTestActions     1   querys sub type of file system on volumes mounted with 'Fat' file system
//!                     2   querys sub type of file system on volumes mounted with 'Lffs' file system
//!                     3   querys sub type of file system on volumes mounted with 'rofs' file system
//!                     4   querys sub type of file system on volumes mounted with other file systems
//! @SYMTestExpectedResults 
//!                     1   returned error code should be KErrNone, descriptor should match 'FAT12' or 'FAT16' or 'FAT32'
//!                     2   returned error code should be KErrNotSupported, descriptor should match 'Lffs'
//!                     3   returned error code should be KErrNotSupported, descriptor should match 'rofs'
//!                     4   returned error code should be KErrNotSupported, descriptor length should not be zero
//! @SYMTestPriority        High
//! @SYMTestStatus          Implemented
//----------------------------------------------------------------------------------------------
static void TestFileSystemSubTypeQuery()
    {
    test.Next(_L("Test querying sub type of the mounted file system"));
    TFSName fsName;
    TPckgBuf<TFSName> subName;
    TInt i, r;
    TDriveInfo driveInfo;
    TPckgBuf<TBool> fDrvSyncBuf;


    for(i = EDriveA; i <= EDriveZ; ++i, subName.Zero())
        {
        r = TheFs.FileSystemName(fsName, i);
        if (r == KErrNone)
            {
            test.Printf(_L("Tested on drive: %c.\n"), (char)(i+'A'));
            r=TheFs.Drive(driveInfo, i);
            test(r==KErrNone);
            
            if (driveInfo.iType==EMediaNotPresent)
                {
                test.Printf(_L("The media is not present.\n"));
                r = TheFs.QueryVolumeInfoExt(i, EFileSystemSubType, subName);
                test(r == KErrNone || r == KErrNotReady);
                }
            else if (driveInfo.iType==EMediaCdRom)
                {
                test.Printf(_L("CD ROM with no media will report not ready!\n"));
                r = TheFs.QueryVolumeInfoExt(i, EFileSystemSubType, subName);
                test(r == KErrNotReady);
                }
            else
                {
                r = TheFs.QueryVolumeInfoExt(i, EFileSystemSubType, subName);
                test_KErrNone(r);

                //-- test EIsDriveSync command
                r = TheFs.QueryVolumeInfoExt(i, EIsDriveSync, fDrvSyncBuf);
                test(r == KErrNone);
                if(fDrvSyncBuf())
                    test.Printf(_L("The drive is Synchronous.\n"));
                else
                    test.Printf(_L("The drive is Asynchronous.\n"));

                //-----------------
                
                // if Fat, testing returning sub type name
                if (fsName.CompareF(KFileSystemName_FAT)==0)
                    {
                    test(r == KErrNone);
                    test(subName().CompareF(KFSSubType_FAT12)==0 ||
                         subName().CompareF(KFSSubType_FAT16)==0 ||
                         subName().CompareF(KFSSubType_FAT32)==0);
                    continue;
                    }
                
                // if Lffs, testing returning file system name
                if (fsName.CompareF(_L("Lffs"))==0)
                    {
                    test(r == KErrNone);
                    test(subName().CompareF(_L("Lffs"))==0);
                    continue;
                    }
                // if rofs, testing returning file system name
                if (fsName.CompareF(_L("rofs"))==0)
                    {
                    test(r == KErrNone);
                    test(subName().CompareF(_L("rofs"))==0);
                    continue;
                    }
                // if Composite, testing returning file system name
                if (fsName.CompareF(_L("Composite"))==0)
                    {
                    test(r == KErrNone);
                    test(subName().CompareF(_L("Composite"))==0);
                    continue;
                    }

                // else
                test(r == KErrNone);
                test(subName().Length()!=0);
                
                }
            }
        }
    }

//----------------------------------------------------------------------------------------------
//! @SYMTestCaseID      PBASE-t_fsys-0318
//! @SYMTestType        CIT
//! @SYMPREQ            CR0882
//! @SYMTestCaseDesc    This test case is testing querying file system's cluster size using 
//!                     RFs::QueryVolumeInfoExt() API.
//! @SYMTestActions     1   querys cluster size of file system on volumes mounted with 'Fat' file system
//!                     2   querys cluster size of file system on volumes mounted with 'Lffs' file system
//!                     3   querys cluster size of file system on volumes mounted with other file systems
//! @SYMTestExpectedResults 
//!                     1   returned error code should be KErrNone, cluster size should be non-zero
//!                     2   returned error code should be KErrNone, cluster size should be 512
//!                     3   returned error code should be KErrNone, cluster size should be KErrNotSupported
//! @SYMTestPriority        High
//! @SYMTestStatus          Implemented
//----------------------------------------------------------------------------------------------
static void TestFileSystemClusterSizeQuery()
    {
    test.Next(_L("Test querying cluster size information of the mounted file system"));
    TFullName fsName;
    TPckgBuf<TVolumeIOParamInfo> ioInfo;
    TInt i, r;
    TDriveInfo driveInfo;
    for(i = EDriveA; i <= EDriveZ; ++i)
        {
        r = TheFs.FileSystemName(fsName, i);
        if (r == KErrNone)
            {
            test.Printf(_L("Tested on drive: %c.\n"), (char)(i+'A'));

            r=TheFs.Drive(driveInfo, i);
            test(r==KErrNone);
            // if no media present
            if (driveInfo.iType==EMediaNotPresent)
                {
                r = TheFs.QueryVolumeInfoExt(i, EIOParamInfo, ioInfo);
                test(r == KErrNone || r == KErrNotReady);
                }
            else if (driveInfo.iType==EMediaCdRom)
                {
                test.Printf(_L("CD ROM with no media!\n"));
                r = TheFs.QueryVolumeInfoExt(i, EIOParamInfo, ioInfo);
                test(r == KErrNone || r == KErrNotReady);
                }
            else
                {
                r = TheFs.QueryVolumeInfoExt(i, EIOParamInfo, ioInfo);
                test(KErrNone == r);
                // if Fat
                if (fsName.CompareF(KFileSystemName_FAT)==0)
                    {
                    test(ioInfo().iClusterSize != 0);
                    continue;
                    }
                // else if Lffs
                if (fsName.CompareF(_L("Lffs"))==0)
                    {
                        TBusLocalDrive drive;   
                        TBool changeFlag = EFalse;  
                        TInt locDriveNumber;
                        TLocalDriveCaps DriveCaps;
                        TLocalDriveCapsV7 DriveCapsV7;
                        for(locDriveNumber = 0; locDriveNumber < KMaxLocalDrives; locDriveNumber++)
                        {
                            r = drive.Connect(locDriveNumber,changeFlag);
                            if(r==KErrNone)
                            {
                                
                                TPckg<TLocalDriveCaps> capsPckg(DriveCaps);
                                r=drive.Caps(capsPckg);
                                if((r==KErrNone) && (DriveCaps.iFileSystemId==KDriveFileSysLFFS))
                                {
                                    break;
                                }
                                drive.Disconnect();
                            }
                        }
                        TPckg<TLocalDriveCapsV7> capsPckg(DriveCapsV7);
                        r=drive.Caps(capsPckg);
                        test(r==KErrNone);
                        drive.Disconnect();
                        if(DriveCapsV7.iObjectModeSize == 0)
                        {
                    test(ioInfo().iClusterSize == 512);
                    continue;
                        }
                        else
                        {
                            test((TUint32)(ioInfo().iClusterSize) == DriveCapsV7.iObjectModeSize);
                            continue;
                        }
                    }
                // else
                //-- we can not suggest anything about unknown filesystem, thus do not check the result.
                //test(ioInfo().iClusterSize == KErrNotSupported);
                
                }
            }
        }
    }

//----------------------------------------------------------------------------------------------
//! @SYMTestCaseID      PBASE-t_fsys-0319
//! @SYMTestType        CIT
//! @SYMPREQ            CR0882
//! @SYMTestCaseDesc    This test case is testing querying block size of underlying media using 
//!                     RFs::QueryVolumeInfoExt() API.
//! @SYMTestActions     1   querys block size on volumes mounted with MMC card type of media
//!                     2   querys block size on volumes mounted with RAM type of media
//!                     3   querys block size on volumes mounted with NOR flash type of media
//!                     4   querys block size on volumes mounted with Nand flash (code) type of media
//!                     5   querys block size on volumes mounted with Nand flash (data) type of media
//! @SYMTestExpectedResults 
//!                     1   returned error code should be KErrNone, block size should be 512
//!                     2   returned error code should be KErrNone, block size should be KDefaultVolumeBlockSize
//!                     3   returned error code should be KErrNone, block size should be KDefaultVolumeBlockSize
//!                     4   returned error code should be KErrNone, block size should be 512
//!                     5   returned error code should be KErrNone, block size should be 512
//! @SYMTestPriority        High
//! @SYMTestStatus          Implemented
//----------------------------------------------------------------------------------------------
static void TestMediaBlockSizeQuery()
    {
    test.Next(_L("Test querying block size information of the underlying media"));
    #if defined(__WINS__)
        test.Printf(_L("This test case runs on hardware only"));
        return;
    
    #else   // test runs on hardware only.
    TFSName fsName;
    TPckgBuf<TVolumeIOParamInfo> ioInfo;
    TInt i, r;
    TDriveInfo driveInfo;
    for(i = EDriveA; i <= EDriveZ; ++i)
        {
        r = TheFs.FileSystemName(fsName, i);
        if (r == KErrNone)
            {
            test.Printf(_L("Tested on drive: %c.\n"), (char)(i+'A'));
            r=TheFs.Drive(driveInfo, i);
            test(r==KErrNone);
            // if no media present
            if (driveInfo.iType==EMediaNotPresent)
                {
                r = TheFs.QueryVolumeInfoExt(i, EIOParamInfo, ioInfo);
                test(r == KErrNone || r == KErrNotReady);
                }
            else if (driveInfo.iType==EMediaCdRom)
                {
                test.Printf(_L("CD ROM with no media will report not ready!\n"));
                r = TheFs.QueryVolumeInfoExt(i, EIOParamInfo, ioInfo);
                test(r == KErrNotReady);
                }
            else
                {
                r = TheFs.QueryVolumeInfoExt(i, EIOParamInfo, ioInfo);
                test(KErrNone == r);
                // if MMC, test block size >= 512;
                // (Version 4.3 MMC cards introduce the concept of a "Super Page" which should be used as
                // guide when calculating the cluster size. For these cards the reported block size may be 
                // any multiple of 512).
                if ((driveInfo.iType == EMediaHardDisk) && 
                    (driveInfo.iDriveAtt & KDriveAttRemovable) &&
                    (driveInfo.iDriveAtt & KDriveAttLocal))
                    {
                    test(ioInfo().iBlockSize >= 512);
                    continue;
                    }
                // if RAM, test block size == 1;
                if ((driveInfo.iType == EMediaRam) && 
                    (driveInfo.iDriveAtt & KDriveAttLocal) &&
                    (driveInfo.iDriveAtt & KDriveAttInternal))
                    {
                    test(ioInfo().iBlockSize == 1);
                    continue;
                    }
                // if NOR flash, test block size == 512 (default block size);
                if ((driveInfo.iType == EMediaFlash) && 
                    (driveInfo.iDriveAtt & KDriveAttLocal) &&
                    (driveInfo.iDriveAtt & KDriveAttInternal))
                    {
                    TBusLocalDrive drive;   
                    TBool changeFlag = EFalse;  
                    TInt locDriveNumber;
                    TLocalDriveCaps DriveCaps;
                    TLocalDriveCapsV7 DriveCapsV7;
                    for(locDriveNumber = 0; locDriveNumber < KMaxLocalDrives; locDriveNumber++)
                        {
                        r = drive.Connect(locDriveNumber,changeFlag);
                        if(r==KErrNone)
                            {
                            TPckg<TLocalDriveCaps> capsPckg(DriveCaps);
                            r=drive.Caps(capsPckg);
                            if((r==KErrNone) && (DriveCaps.iFileSystemId==KDriveFileSysLFFS))
                                {
                                break;
                                }
                            drive.Disconnect();
                            }
                        }
                    TPckg<TLocalDriveCapsV7> capsPckg(DriveCapsV7);
                    r=drive.Caps(capsPckg);
                    test(r==KErrNone);
                    if ((fsName.CompareF(_L("Lffs"))==0) && (DriveCapsV7.iObjectModeSize != 0))
                        {                   
                        test(ioInfo().iBlockSize == (TInt) DriveCapsV7.iObjectModeSize);
                        continue;
                        }
                    else
                        {
                        test(ioInfo().iBlockSize == (TInt) KDefaultVolumeBlockSize);
                        continue;
                        }
                    }
                // if Nand flash (with Fat file system), test block size == 512 (small-block) or 2048 (large-block)
                if ((driveInfo.iType == EMediaNANDFlash) &&
                    (driveInfo.iDriveAtt & KDriveAttLocal) &&
                    (driveInfo.iDriveAtt & KDriveAttInternal))
                    {
                    test(ioInfo().iBlockSize == 512 || ioInfo().iBlockSize == 2048);
                    continue;
                    }
                }
            }
        }
    #endif // __WINS__
    }

//----------------------------------------------------------------------------------------------
//! @SYMTestCaseID      PBASE-t_fsys-0320
//! @SYMTestType        CIT
//! @SYMPREQ            CR0882
//! @SYMTestCaseDesc    This test case is testing wrapper API RFs::FileSystemSubType() has the same
//!                     behaviours as RFs::QueryVolumeInfoExt()
//! @SYMTestActions     1   querys file system sub type name by both APIs
//! @SYMTestExpectedResults 
//!                     1   returned error codes and descriptors of both API should be identical
//! @SYMTestPriority        High
//! @SYMTestStatus          Implemented
//----------------------------------------------------------------------------------------------
static void TestFileSystemSubType()
    {
    test.Next(_L("Test wrapper API RFs::FileSystemSubType()'s behaviour"));
    TFSName fsName;
    TPckgBuf<TFSName> subName;
    TInt r;
    TFSName subName1;
    TInt r1;
    
    for(TInt i = EDriveA; i <= EDriveZ; ++i)
        {
        r = TheFs.FileSystemName(fsName, i);
        if (r == KErrNone)
            {
            test.Printf(_L("Tested on drive: %c.\n"), (char)(i+'A'));
            r = TheFs.QueryVolumeInfoExt(i, EFileSystemSubType, subName);
            r1 = TheFs.FileSystemSubType(i, subName1);
            test(r==r1);
            if (subName().Length())
                {
                test(subName().CompareF(subName1)==0);
                }
            else
                {
                test(subName1.Length()==0);
                }
            }
        }
    }

//----------------------------------------------------------------------------------------------
//! @SYMTestCaseID      PBASE-t_fsys-0321
//! @SYMTestType        CIT
//! @SYMPREQ            CR0882
//! @SYMTestCaseDesc    This test case is testing wrapper API RFs::VolumeIOParam() has the same
//!                     behaviours as RFs::QueryVolumeInfoExt()
//! @SYMTestActions     1   querys volume IO params by both APIs
//! @SYMTestExpectedResults 
//!                     1   returned error codes and IO param values of both API should be identical
//! @SYMTestPriority        High
//! @SYMTestStatus          Implemented
//----------------------------------------------------------------------------------------------
static void TestVolumeIOParam()
    {
    test.Next(_L("Test wrapper API RFs::VolumeIOParam()'s behaviour"));
    TFSName fsName;
    TPckgBuf<TVolumeIOParamInfo> ioInfo;
    TInt r;
    TVolumeIOParamInfo ioInfo1;
    TInt r1;
    
    for(TInt i = EDriveA; i <= EDriveZ; ++i)
        {
        r = TheFs.FileSystemName(fsName, i);
        if (r == KErrNone)
            {
            test.Printf(_L("Tested on drive: %c.\n"), (char)(i+'A'));
            r = TheFs.QueryVolumeInfoExt(i, EIOParamInfo, ioInfo);
            r1 = TheFs.VolumeIOParam(i, ioInfo1);
            test(r==r1);
            test(ioInfo().iBlockSize == ioInfo1.iBlockSize);
            test(ioInfo().iClusterSize == ioInfo1.iClusterSize);
            test(ioInfo().iRecReadBufSize == ioInfo1.iRecReadBufSize);
            test(ioInfo().iRecWriteBufSize == ioInfo1.iRecWriteBufSize);
            }
        }
    }


//----------------------------------------------------------------------------------------------
//! @SYMTestCaseID      PBASE-t_fsys-0322
//! @SYMTestType        CIT
//! @SYMPREQ            CR0882
//! @SYMTestCaseDesc    This test case is testing RFs::QueryVolumeInfoExt() API on a testing file system
//! @SYMTestActions     0   mounts testing file system on a certain drive
//!                     1   querys file system's sub type name on the drive under testing
//!                     2   querys file system's cluster size on the drive under testing
//! @SYMTestExpectedResults 
//!                     1   returned error code should be KErrNone, sub type name should match 'Test3SubType'
//!                     2   returned error code should be KErrNone, cluster size should equal 1024
//! @SYMTestPriority        High
//! @SYMTestStatus          Implemented
//----------------------------------------------------------------------------------------------
static void TestQueryVolumeInfoExtOnTestFS(TInt aDrive)
    {
    if (aDrive==EDriveC) // Can't test on C:
        return;

    TInt r;

    test.Printf(_L("Tested on drive: %c.\n"), (char)(aDrive+'A'));

    // Mount a new CTestFileSystem on the drive under test
    test.Next(_L("Test RFs::QueryVolumeInfoExt() on Testing File System"));
    r = TheFs.AddFileSystem(_L("T_TFSYS3"));
    if (r != KErrNone && r != KErrAlreadyExists)
        {
        test.Printf(_L("error=%d"),r);
        test(EFalse);
        }
    TFSName oldFs;
    r = TheFs.FileSystemName(oldFs,aDrive);
    test(r==KErrNone);
    r = TheFs.DismountFileSystem(oldFs,aDrive);
    if (r != KErrNone)
        {
        test.Printf(_L("Error = %d"),r);    
        test(EFalse);
        }
    r = TheFs.MountFileSystem(_L("Test3"),aDrive);
    test(r==KErrNone);
    TFSName newFs;
    r = TheFs.FileSystemName(newFs,aDrive);
    test(r==KErrNone);
    test(newFs.Compare(_L("Test3"))==0);

    // Sub type name query: 
    TPckgBuf<TFSName> subNameP;
    r = TheFs.QueryVolumeInfoExt(aDrive, EFileSystemSubType, subNameP);
    test(r==KErrNone);
    test(subNameP() == _L("Test3SubType"));

    // Cluster size querys:
    TPckgBuf<TVolumeIOParamInfo> ioInfoP;
    r = TheFs.QueryVolumeInfoExt(aDrive, EIOParamInfo, ioInfoP);
    test(r==KErrNone);
    test(ioInfoP().iClusterSize==1024);

    // Mount the original file system back
    r=TheFs.DismountFileSystem(newFs,aDrive);
    test(r==KErrNone);
    r=TheFs.MountFileSystem(oldFs,aDrive);
    test(r==KErrNone);
    
    r=TheFs.RemoveFileSystem(_L("Test3"));
    if(r!=KErrNone)
        {
        test.Printf(_L("error=%d"),r);
        test(EFalse);
        }
    }


//----------------------------------------------------------------------------------------------
/**
    Test remounting the file system with objects opened.
    scenario:
    1. create a file
    2. open it.
    3. forcedly remount the file system 
    4. read this file (this will imply remounting the filesystem)
*/
static void TestRemountFSWithOpenedObjects()
{
    test.Next(_L("Testing forcedly remounting FS with objects opened.\n"));
    
    TInt nRes;
    
    //-- 1. create a file
    _LIT(KFile, "\\test_file.file");
    const TUint KFileSz = 5000;
    
    nRes = CreateCheckableStuffedFile(TheFs, KFile, KFileSz);
    test_KErrNone(nRes);

    RFile file;

    //-- 2. open this file
    nRes = file.Open(TheFs, KFile, EFileRead);
    test_KErrNone(nRes);

    //-- 2.1 try to dismount the FS, it must fail because of the opened object.
    TBuf<40> fsName;
    nRes = TheFs.FileSystemName(fsName, CurrentDrive());
    test_KErrNone(nRes);

    nRes = TheFs.DismountFileSystem(fsName, CurrentDrive());
    test(nRes == KErrInUse);


    //-- 3. forcedly remount the drive
    nRes = TheFs.RemountDrive(CurrentDrive());
    if(nRes == KErrNotSupported)
    {//-- this feature is not supported and the test is inconsistent.
        test.Printf(_L("RemountDrive() is not supported, the test is inconsistent!"));
        
        //-- remounting must work at least on MMC drives
        const TBool isFAT = Is_Fat(TheFs, CurrentDrive());

        TDriveInfo  driveInfo;
        nRes = TheFs.Drive(driveInfo, CurrentDrive());
        test_KErrNone(nRes);

        test(!isFAT || (!(driveInfo.iDriveAtt & KDriveAttRemovable)));

    }
    else
    {
        test_KErrNone(nRes);
    }

    User::After(500*K1mSec);

    //-- 4. read this file. The FS will be remounted and the read must be OK.
    TBuf8<40> buf;
    nRes = file.Read(0, buf, 30);
    test_KErrNone(nRes);
    
    file.Close();

    //-- 5. verify the file, just in case.
    nRes = VerifyCheckableFile(TheFs, KFile);
    test_KErrNone(nRes);

    //-- 6. delete the file
    TheFs.Delete(KFile);

}
//----------------------------------------------------------------------------------------------
static void TestFileSystem_MaxSupportedFileSizeQuery()
{
    test.Next(_L("Test querying max. supported file size on this file system"));
    TFullName fsName;
    TPckgBuf<TVolumeIOParamInfo> ioInfo;
    TVolumeIOParamInfo& volInfo = ioInfo();

    const TInt drvNo=CurrentDrive();

    TInt nRes;

    nRes = TheFs.FileSystemName(fsName, drvNo);
    test_KErrNone(nRes);

    nRes = TheFs.QueryVolumeInfoExt(drvNo, EIOParamInfo, ioInfo);
    test_KErrNone(nRes);

    test.Printf(_L("FS:'%S' Max File Size:0x%LX\n"), &fsName, volInfo.iMaxSupportedFileSize);
    if(volInfo.iMaxSupportedFileSize == KMaxTUint64)
    {
        test.Printf(_L("Max File Size query isn't supported by this FS\n"));
    }


    //-- check the value for FAT FS only. 
    if(Is_Fat(TheFs, drvNo))
    {
        test(volInfo.iMaxSupportedFileSize == KMaxSupportedFatFileSize);
    }

}

//----------------------------------------------------------------------------------------------
GLDEF_C void CallTestsL()
//
// Do all tests
//
    {

    //-- set up console output 
    Fat_Test_Utils::SetConsole(test.Console()); 

    TInt drive=CurrentDrive();

    PrintDrvInfo(TheFs, drive);

    //Do not run this test on the NAND drive, as
    //this has the FTL mounted as a primary extension
    //which causes the test to fail
    #if defined(__WINS__)
        if (drive==EDriveU)
            return;
    #else
        TDriveInfo driveInfo;
        TheFs.Drive(driveInfo,drive);
        if (driveInfo.iType == EMediaNANDFlash)
            {
            return;
            }
    #endif

    //---------------------------------------

    TestFileSystemNames();
    TestDismountFileSystem(CurrentDrive());
#if defined(__EPOC32__)
    TestFileSystem(CurrentDrive());
#endif

    TestMountInvalidDrive();
    
    TestMountingBrokenMedia(CurrentDrive());
    TestSubstDriveMediaSerialNumber();

    TestFileSystemSubTypeQuery();
    TestFileSystemClusterSizeQuery();
    TestMediaBlockSizeQuery();
    TestFileSystemSubType();
    TestVolumeIOParam();
    TestQueryVolumeInfoExtOnTestFS(CurrentDrive());

    TestFileSystem_MaxSupportedFileSizeQuery();

    TestRemountFSWithOpenedObjects();
    
    
    }
