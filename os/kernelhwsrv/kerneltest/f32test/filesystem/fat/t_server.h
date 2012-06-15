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
// f32test\server\t_server.h
//
//

#if !defined(__T_STD_H__)
#define __T_STD_H__

GLREF_C void CallTestsL();
GLREF_C void CreateTestDirectory(const TDesC& aTestPath);
GLREF_C void DeleteTestDirectory();
GLREF_C TInt CurrentDrive();
GLREF_C void MakeFile(const TDesC& aFileName);
GLREF_C void MakeFile(const TDesC& aFileName,const TDesC8& aContents);
GLREF_C void MakeFile(const TDesC& aFileName,const TUidType& aUidType,const TDesC8& aContents);
GLREF_C void MakeFile(const TDesC& aFileName,TInt anAttributes);
GLREF_C void MakeDir(const TDesC& aDirName);
GLREF_C void ReportCheckDiskFailure(TInt aRet);
GLREF_C void CheckDisk();
GLREF_C void CreateLongName(TDes& aFileName,TInt64& aSeed,TInt aLength=-1);
GLREF_C void CreateShortName(TDes& aFileName,TInt64& aSeed);

GLREF_C TInt DismountFileSystem(RFs& aFs, const TDesC& aFileSystemName,TInt aDrive);
GLREF_C TInt MountFileSystem(RFs& aFs, const TDesC& aFileSystemName,TInt aDrive, TBool aIsSync=EFalse);


GLREF_D RTest test;
GLREF_D RFs TheFs;

GLREF_D TFileName gSessionPath;
GLREF_D TInt64 gSeed;
GLREF_D TChar gDriveToTest;
GLREF_D TFileCacheFlags gDriveCacheFlags;

#endif
