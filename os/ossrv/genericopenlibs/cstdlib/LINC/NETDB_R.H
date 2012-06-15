/*
* Copyright (c) 1997-2009 Nokia Corporation and/or its subsidiary(-ies).
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
* Sundry networking-related functions which uses static data areas or other bits of
* nastiness.
* 
*
*/



/**
 @file
 @publishedAll
 @released
*/

#ifndef _NETDB_R_H_
#define _NETDB_R_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <netdb.h>
#include <reent.h>
#include <sys/socket.h>
#include <libc/netinet/in.h>

IMPORT_C char*	_inet_ntoa_r (struct _reent*, struct in_addr);
IMPORT_C struct hostent* _gethostbyaddr_r (struct _reent* rp, const char* addr, int length, int format);
IMPORT_C struct hostent* _gethostbyname_r (struct _reent* rp, const char* name);

#ifdef __cplusplus
}
#endif

#endif /* !_NETDB_R_H_ */
