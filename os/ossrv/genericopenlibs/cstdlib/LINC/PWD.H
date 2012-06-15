/* PWD.H
 * 
 * Portions Copyright (c) 1997-1999 Nokia Corporation and/or its subsidiary(-ies).
 * All rights reserved.
 */

/*-
 * Copyright (c) 1989 The Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *	@(#)pwd.h	5.13 (Berkeley) 5/28/91
 */

/** @file
@publishedAll
@released
*/

#ifndef _PWD_H_
#ifdef __cplusplus
extern "C" {
#endif
#define	_PWD_H_

#include <sys/types.h>

#ifndef _POSIX_SOURCE
#define	_PATH_PASSWD		"/etc/passwd"
/** max length, not counting NULL
 */
#define	_PASSWORD_LEN		128
#endif

/** password
*/
struct passwd {
	/**	user name
	*/
	char	*pw_name;	
	
	/** encrypted password
	 */	
	char	*pw_passwd;		

	/** user uid
	 */
	int	pw_uid;			

	/** user gid 
	*/
	int	pw_gid;		
	
	/**	comment
	*/
	char	*pw_comment;	
	
	/** Honeywell login info 
	*/	
	char	*pw_gecos;		
	
	/**	home directory 
	*/
	char	*pw_dir;	

	/** default shell 
	*/
	char	*pw_shell;		
};

extern struct passwd	*getpwuid (uid_t);
extern struct passwd	*getpwnam (const char *);
#ifndef _POSIX_SOURCE
extern struct passwd	*getpwent (void);
extern void		 setpwent (void);
extern void		 endpwent (void);
#endif

#ifdef __cplusplus
}
#endif
#endif /* _PWD_H_ */
