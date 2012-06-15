/*
* Copyright (c) 2009 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
* This component and the accompanying materials are made available
* under the terms of "Eclipse Public License v1.0"
* which accompanies this distribution, and is available
* at the URL "http://www.eclipse.org/legal/epl-v10.html".
*
* Initial Contributors:
* Nokia Corporation - initial contribution.
*
* Contributors:
*
* Description:
*
*/

#include <stdio.h>
#include<e32std.h>
#include "libutils.h"
#include"std_log_result.h"
#define LOG_FILENAME_LINE __FILE__, __LINE__
int main()
{
    int retval =ESuccess;
    string s("Hello World");
    __UHEAP_MARK;	
    TPtr16  buf((unsigned short *)"Some Unused data", 10);
    wchar_t *wptr = new wchar_t[15];
    retval = StringToTptr16(s, wptr, buf);

    if (retval == EInsufficientMemory)
    {
    printf("stringtotptr16 Passed");
    }
    else
    {
    assert_failed = true;
    printf("stringtotbuf16 Failed");
    }
    delete[] wptr;
    wptr = NULL;
    __UHEAP_MARKEND;
    testResultXml("test_stringtotptr16_negative1");
	return 0;
}
