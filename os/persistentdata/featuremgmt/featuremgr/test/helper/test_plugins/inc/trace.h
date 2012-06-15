/*
* Copyright (c) 2002-2009 Nokia Corporation and/or its subsidiary(-ies).
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



#ifndef TRACE_H
#define TRACE_H

// INCLUDES
#include <e32debug.h>

// PC-LINT OPTIONS
// Removes null statement not in line by itself warnings from
// COMPONENT_TRACE macros
//lint -esym(960,54)

// Removes "area too small" pointer cast warnings.
//lint -e826



// CONSTANTS
// MACROS
#ifdef _DEBUG

    #ifdef TEST_PLUGIN_TRACE_FLAG

        #define TEST_PLUGIN_TRACE( a ) RDebug::Print a

    #else // #ifdef TEST_PLUGIN_TRACE_FLAG

        #define TEST_PLUGIN_TRACE( a )

    #endif //#ifdef TEST_PLUGIN_TRACE_FLAG


    #ifdef TEST_PLUGIN_API_TRACE_FLAG

        #define TEST_PLUGIN_API_TRACE( a ) RDebug::Print a

    #else //#ifdef TEST_PLUGIN_API_TRACE_FLAG

        #define TEST_PLUGIN_API_TRACE( a )

    #endif //#ifdef TEST_PLUGIN_API_TRACE_FLAG

#else // #ifdef _DEBUG

    #define TEST_PLUGIN_TRACE( a )
    #define TEST_PLUGIN_API_TRACE( a )

#endif //#ifdef _DEBUG


// DATA TYPES
// FUNCTION PROTOTYPES
// FORWARD DECLARATIONS

#endif //#ifndef TRACE_H

