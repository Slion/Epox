// Copyright (c) 2006-2009 Nokia Corporation and/or its subsidiary(-ies).
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
// f32test\server\t_findcapall.cpp
// PlatSec compatibility test. Application capability is ALLFILES
// Tests to confirm DEF088224 changes do not affect exisitng functionality
// TFindFile should NEVER return KErrPermissionDenied for FindByDir requests
// z:\sys\bin\t_findcaptestfile.txt is used for testing.
// If the file is not avialable in the location, test will panic.
// 
//

#include <e32test.h>
#include <f32file.h>

_LIT(KTestString,"t_findcapall");

LOCAL_C RTest test(KTestString);

LOCAL_C	RFs FileServer;

LOCAL_C TInt TestFind(const TPtrC16 aTestDesc, const TPtrC16 aFPath, const TPtrC16 aFName)
	{
	TInt Err;
	test.Next(aTestDesc);
	TFindFile FindFile(FileServer);
	Err=FindFile.FindByDir(aFName,aFPath);
	return Err;
	}


GLDEF_C TInt E32Main()
	{
	TInt Err;
	
	test.Title();
	
	Err=FileServer.Connect();
	
	test(Err==KErrNone);
	
    // RTest.Next is called from function "TestFind()".
    // RTest.Start is called here to start the test.
    test.Start(_L("Test Starts : Dummy Test"));
    
	// Test: Find non existing file in existing /sys folder
	//
	// Drive Name			: Z:
	// Path 				: sys\bin
	// File Name			: nonexistingfile.txt
	//
	// Expected return value: KErrNotFound
	Err=TestFind(_L("Drive specified & available Path exists File does not exist"),
			     _L("z:\\sys\\bin\\"),
			     _L("nonexistingfile.txt"));
	
	test(Err==KErrNotFound);
	
	// Test: Find existing file in existing /sys folder
	//
	// Drive Name			: Z:
	// Path 				: sys\bin
	// File Name			: t_findcaptestfile.txt
	//
	// Expected return value: KErrNone
	Err=TestFind(_L("Drive specified & available Path exists File exists"),
			     _L("z:\\sys\\bin\\"),
			     _L("t_findcaptestfile.txt"));
	
	test(Err==KErrNone);
	
	// Test: Find non existing file in existing / non existing /sys folder
	//
	// Drive Name			: C:
	// Path 				: sys
	// File Name			: nonexisting.txt
	//
	// Expected return value: KErrNotFound
	Err=TestFind(_L("Drive specified & available Path may exist File does not exist"),
			     _L("c:\\sys\\"),
			     _L("nonexisting.txt"));
	
	test(Err==KErrNotFound);
	
	// Test: Find existing file in /sys folder without specifying the path
	//
	// Drive Name			: Not specified.
	// Path 				: sys\bin
	// File Name			: t_findcaptestfile.txt
	//
	// Expected return value: KErrNone
	Err=TestFind(_L("Drive not specified Path exists File exists"),
			     _L("\\sys\\bin\\"),
			     _L("t_findcaptestfile.txt"));
	
	test(Err==KErrNone);
	
	// Test: Find non existing file in /sys folder without specifying the path
	//
	// Drive Name			: Not specified
	// Path 				: sys
	// File Name			: nonexistingfile.txt
	//
	// Expected return value: KErrNotFound
	Err=TestFind(_L("Drive not specified Path exists File does not exist"),
			     _L("\\sys\\"),
			     _L("nonexisting.txt"));
	
	test(Err==KErrNotFound);
	
	FileServer.Close();
	
    test.Printf(_L("Test completed\n"));
    
	test.End();
	test.Close();
	
	return KErrNone;
	}
