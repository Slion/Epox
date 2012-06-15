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
#include <e32base.h>
#include "libutils.h"
#include"std_log_result.h"
#define LOG_FILENAME_LINE __FILE__, __LINE__
void main()
{
    __UHEAP_MARK;
    {
    int retval =ESuccess;
    wstring str;
    _LIT16(KTxt, "hellohello");
    TBuf<30> tbuf(KTxt);
    int i=0;
    while (i<=1000)
    {
  	retval = Tbuf16ToWstring(tbuf,str);
    i++;
    printf("%d\n",i);
    }    

    if (retval ==ESuccess)
    {
    printf("tbuf16towstring Passed");
    }
    else
    {
    assert_failed = true;
    printf("tbuf16towstring failed");
    }      
    }
    __UHEAP_MARKEND;
    testResultXml("test_tbuf16towstring_positive");
}