/*-
 * � Portions copyright (c) 2006 Nokia Corporation.  All rights reserved.
 * 
 * Copyright (c) 2002-2004 Tim J. Robbins.
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
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <sys/cdefs.h>
__FBSDID("$FreeBSD: src/lib/libc/stdio/fputwc.c,v 1.10 2004/07/20 08:27:27 tjr Exp $");

#include "namespace.h"
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>
#include "un-namespace.h"
#include "libc_private.h"
#include "local.h"
#ifndef __SYMBIAN32__
#include "mblocal.h"
#endif //__SYMBIAN32__

/*
 * Non-MT-safe version.
 */
wint_t
__fputwc(wchar_t wc, FILE *fp)
{
	char buf[MB_LEN_MAX];
	size_t i, len;

	if (MB_CUR_MAX == 1 && wc > 0 && wc <= UCHAR_MAX) {
		/*
		 * Assume single-byte locale with no special encoding.
		 * A more careful test would be to check
		 * _CurrentRuneLocale->encoding.
		 */
		*buf = (unsigned char)wc;
		len = 1;
	} else {
#ifndef __SYMBIAN32__
		if ((len = __wcrtomb(buf, wc, &fp->_extra->mbstate)) ==
		    (size_t)-1) {
#else //__SYMBIAN32__
		if ((len = wcrtomb(buf, wc, &fp->_extra->mbstate)) ==
		    (size_t)-1) {
#endif //__SYMBIAN32__
			fp->_flags |= __SERR;
			return (WEOF);
		}
	}

	for (i = 0; i < len; i++)
		if (__sputc((unsigned char)buf[i], fp) == EOF)
			return (WEOF);

	return ((wint_t)wc);
}

/*
 * MT-safe version.
 */
EXPORT_C wint_t
fputwc(wchar_t wc, FILE *fp)
{
	wint_t r;

	FLOCKFILE(fp);
	ORIENT(fp, 1);
	r = __fputwc(wc, fp);
	FUNLOCKFILE(fp);

	return (r);
}
