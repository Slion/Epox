/*
 * Copyright (c) 1999-2004 Damien Miller <djm@mindrot.org>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 * * Portions Copyright (c) 2006 Nokia Corporation and/or its subsidiary(-ies). All rights reserved. 
 */




#include <sys/time.h>
#include <sys/timespec.h>
#include <sys/select.h>
#include <errno.h>

//#if !defined(HAVE_NANOSLEEP) && !defined(HAVE_NSLEEP)

EXPORT_C 
int nanosleep(const struct timespec *req, struct timespec *rem)
{
        int rc, saverrno;
        extern int errno;
        struct timeval tstart, tstop, tremain, time2wait;

        if(!req)
        	{ 
        		return 0 ;
        	} 	 
        TIMESPEC_TO_TIMEVAL(&time2wait, req);
        (void) gettimeofday(&tstart, NULL);
        rc = select(0, NULL, NULL, NULL, &time2wait);
        if (rc == -1) {
                saverrno = errno;
                (void) gettimeofday (&tstop, NULL);
                errno = saverrno;
                tremain.tv_sec = time2wait.tv_sec -
                        (tstop.tv_sec - tstart.tv_sec);
                tremain.tv_usec = time2wait.tv_usec -
                        (tstop.tv_usec - tstart.tv_usec);
                tremain.tv_sec += tremain.tv_usec / 1000000L;
                tremain.tv_usec %= 1000000L;
        } else {
                tremain.tv_sec = 0;
                tremain.tv_usec = 0;
        }
        if(rem)
        	{
        	TIMEVAL_TO_TIMESPEC(&tremain, rem);
        	}

        return(rc);
}
//#endif
