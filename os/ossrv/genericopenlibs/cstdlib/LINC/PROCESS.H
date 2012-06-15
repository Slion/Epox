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
*
*/



/**
 @file
 @publishedAll
 @released
*/

#ifndef __PROCESS_H_
#define __PROCESS_H_

#ifdef __cplusplus
extern "C" {
#endif

extern int execl(const char *path, const char *argv0, ...);
extern int execle(const char *path, const char *argv0, ... /*, char * const *envp */);
extern int execlp(const char *path, const char *argv0, ...);
extern int execlpe(const char *path, const char *argv0, ... /*, char * const *envp */);

extern int execv(const char *path, char * const *argv);
extern int execve(const char *path, char * const *argv, char * const *envp);
extern int execvp(const char *path, char * const *argv);
extern int execvpe(const char *path, char * const *argv, char * const *envp);


#ifdef __cplusplus
}
#endif

#endif


