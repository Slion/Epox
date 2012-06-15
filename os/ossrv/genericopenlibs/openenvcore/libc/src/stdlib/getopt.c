/*	$NetBSD: getopt.c,v 1.26 2003/08/07 16:43:40 agc Exp $	*/

/*
 * Copyright (c) 1987, 1993, 1994
 *	The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
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
 * � Portions copyright (c) 2006 Nokia Corporation.  All rights reserved.
 */

#if defined(LIBC_SCCS) && !defined(lint)
static char sccsid[] = "@(#)getopt.c	8.3 (Berkeley) 4/27/95";
#endif /* LIBC_SCCS and not lint */
#include <sys/cdefs.h>
__FBSDID("$FreeBSD: src/lib/libc/stdlib/getopt.c,v 1.7 2004/03/06 17:05:45 ache Exp $");

#include "namespace.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "un-namespace.h"

#include "libc_private.h"

#if (defined(__SYMBIAN32__) && (defined(__WINSCW__) || defined(__WINS__)))
#include "libc_wsd_defs.h"
#endif

#ifndef __SYMBIAN32__

int	opterr = 1,		/* if error message should be printed */
	optind = 1,		/* index into parent argv vector */
	optopt,			/* character checked for validity */
	optreset;		/* reset getopt */
char	*optarg;	/* argument associated with option */

#else //__SYMBIAN32__

#ifndef EMULATOR

int	opterrp = 1,	/* if error message should be printed */
	optindp = 1,	/* index into parent argv vector */
	optoptp,		/* character checked for validity */
	optresetp;		/* reset getopt */
char	*optargp;	/* argument associated with option */

#else //EMULATOR
GET_GLOBAL_VAR_FROM_TLS(opterrp, int)
GET_GLOBAL_VAR_FROM_TLS(optindp, int)
GET_GLOBAL_VAR_FROM_TLS(optoptp, int)
GET_GLOBAL_VAR_FROM_TLS(optresetp, int)
GET_GLOBAL_VAR_FROM_TLS(optargp, char *)
GET_STATIC_VAR_FROM_TLS(getopt_place, char *)

#define opterrp (*GET_WSD_VAR_NAME(opterrp, g)())
#define optindp (*GET_WSD_VAR_NAME(optindp, g)())
#define optoptp (*GET_WSD_VAR_NAME(optoptp, g)())
#define optresetp (*GET_WSD_VAR_NAME(optresetp, g)())
#define optargp (*GET_WSD_VAR_NAME(optargp, g)())
#define place (*GET_WSD_VAR_NAME(getopt_place, s)())

#endif //EMULATOR

#endif //__SYMBIAN32__

#define	BADCH	(int)'?'
#define	BADARG	(int)':'
#define	EMSG	""


EXPORT_C int *__optind(void)
	{
	return &optindp;
	}

EXPORT_C int *__opterr(void)
	{
	return &opterrp;
	}
		
EXPORT_C int *__optopt(void)
	{
	return &optoptp;
	}

EXPORT_C int *__optreset(void)
	{
	return &optresetp;
	}
	
EXPORT_C char **__optarg(void)
	{
	return &optargp;
	}
	
/*
 * getopt --
 *	Parse argc/argv argument vector.
 */

EXPORT_C
int
getopt(nargc, nargv, ostr)
	int nargc;
	char * const nargv[];
	const char *ostr;
{
#ifndef EMULATOR
	static char *place = EMSG;		/* option letter processing */
#endif //EMULATOR
	char *oli;				/* option letter list index */

	if (optreset || *place == 0) {		/* update scanning pointer */
		optreset = 0;
		place = nargv[optind];
		if (optind >= nargc || *place++ != '-') {
			/* Argument is absent or is not an option */
			place = EMSG;
			return (-1);
		}
		optopt = *place++;
		if (optopt == '-' && *place == 0) {
			/* "--" => end of options */
			++optind;
			place = EMSG;
			return (-1);
		}
		if (optopt == 0) {
			/* Solitary '-', treat as a '-' option
			   if the program (eg su) is looking for it. */
			place = EMSG;
			if (strchr(ostr, '-') == NULL)
				return (-1);
			optopt = '-';
		}
	} else
		optopt = *place++;

	/* See if option letter is one the caller wanted... */
	if (optopt == ':' || (oli = strchr(ostr, optopt)) == NULL) {
		if (*place == 0)
			++optind;
		if (opterr && *ostr != ':')
			(void)fprintf(stderr,
			    "%s: illegal option -- %c\n", _getprogname(),
			    optopt);
		return (BADCH);
	}

	/* Does this option need an argument? */
	if (oli[1] != ':') {
		/* don't need argument */
		optarg = NULL;
		if (*place == 0)
			++optind;
	} else {
		/* Option-argument is either the rest of this argument or the
		   entire next argument. */
		if (*place)
			optarg = place;
		else if (nargc > ++optind)
			optarg = nargv[optind];
		else {
			/* option-argument absent */
			optarg = NULL;
			place = EMSG;
			if (*ostr == ':')
				return (BADARG);
			if (opterr)
				(void)fprintf(stderr,
				    "%s: option requires an argument -- %c\n",
				    _getprogname(), optopt);
			return (BADCH);
		}
		place = EMSG;
		++optind;
	}
	return (optopt);			/* return option letter */
}
