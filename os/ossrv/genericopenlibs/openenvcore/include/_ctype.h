/*
 * Copyright (c) 1989, 1993
 *	The Regents of the University of California.  All rights reserved.
 * (c) UNIX System Laboratories, Inc.
 * All or some portions of this file are derived from material licensed
 * to the University of California by American Telephone and Telegraph
 * Co. or Unix System Laboratories, Inc. and are reproduced herein with
 * the permission of UNIX System Laboratories, Inc.
 *
 * This code is derived from software contributed to Berkeley by
 * Paul Borman at Krystal Technologies.
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
 *
 * From @(#)ctype.h	8.4 (Berkeley) 1/21/94
 * From FreeBSD: src/include/ctype.h,v 1.27 2004/06/23 07:11:39 tjr Exp
 * $FreeBSD: src/include/_ctype.h,v 1.30 2004/08/21 07:00:40 tjr Exp $
 *
 *  Portions Copyright (c) 2006-2007 Nokia Corporation and/or its subsidiary(-ies). All rights reserved.
 */

#ifndef __CTYPE_H_
#define	__CTYPE_H_

#include <sys/cdefs.h>
#include <sys/_types.h>

#ifdef __SYMBIAN32__
#include <_ansi.h>

#ifdef __cplusplus
extern "C" {
#endif
#endif // __SYMBIAN32__

#define	_CTYPE_A	0x00000100L		/* Alpha */
#define	_CTYPE_C	0x00000200L		/* Control */
#define	_CTYPE_D	0x00000400L		/* Digit */
#define	_CTYPE_G	0x00000800L		/* Graph */
#define	_CTYPE_L	0x00001000L		/* Lower */
#define	_CTYPE_P	0x00002000L		/* Punct */
#define	_CTYPE_S	0x00004000L		/* Space */
#define	_CTYPE_U	0x00008000L		/* Upper */
#define	_CTYPE_X	0x00010000L		/* X digit */
#define	_CTYPE_B	0x00020000L		/* Blank */
#define	_CTYPE_R	0x00040000L		/* Print */
#define	_CTYPE_I	0x00080000L		/* Ideogram */
#define	_CTYPE_T	0x00100000L		/* Special */
#define	_CTYPE_Q	0x00200000L		/* Phonogram */
#define	_CTYPE_SW0	0x20000000L		/* 0 width character */
#define	_CTYPE_SW1	0x40000000L		/* 1 width character */
#define	_CTYPE_SW2	0x80000000L		/* 2 width character */
#define	_CTYPE_SW3	0xc0000000L		/* 3 width character */
#define	_CTYPE_SWM	0xe0000000L		/* Mask for screen width data */
#define	_CTYPE_SWS	30			/* Bits to shift to get width */


#ifdef __SYMBIAN32__
#ifdef __cplusplus
}
#endif
#endif //__SYMBIAN32__

#endif /* !__CTYPE_H_ */
