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
    __UHEAP_MARK;
    _LIT16(Kname,"qqqqqqqqqqqqqqqqqqqjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjj");
    TBuf <300>src;
    src.Copy(Kname);
    wchar_t * des= new wchar_t [300]; 
    int size=20000;
    int retval =ESuccess;
    retval= Tlitc16ToWchar(src,des,size);

    if(retval ==ESuccess)
    {
    printf(" Test_tlit16towchar_maxsize PASSED\n");
    }
    else
    {
    assert_failed = true;
    printf(" Test_tlit16towchar_maxsize FAILURE\n");
    }
    delete[] des;
    des = NULL;	
    __UHEAP_MARKEND;
    testResultXml("Test_tlit16towchar_maxsize");
}
