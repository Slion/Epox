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
* Values appropriate for the formatting of monetary and other
* numberic quantities.
* 
*
*/



/**
 @file
 @publishedAll
 @released
*/
#ifndef _LOCALE_H_
#define _LOCALE_H_
#ifdef __cplusplus
extern "C" {
#endif

#include "_ansi.h"

#ifndef NULL
#define NULL    0L
#endif

#define LC_ALL	    0
#define LC_COLLATE  1
#define LC_CTYPE    2
#define LC_MONETARY 3
#define LC_NUMERIC  4
#define LC_TIME     5

struct lconv
{
  char *decimal_point;
  char *thousands_sep;
  char *grouping;
  char *int_curr_symbol;
  char *currency_symbol;
  char *mon_decimal_point;
  char *mon_thousands_sep;
  char *mon_grouping;
  char *positive_sign;
  char *negative_sign;
  char int_frac_digits;
  char frac_digits;
  char p_cs_precedes;
  char p_sep_by_space;
  char n_cs_precedes;
  char n_sep_by_space;
  char p_sign_posn;
  char n_sign_posn;
};

IMPORT_C char *setlocale(int category, const char *locale);
IMPORT_C struct lconv *localeconv(void);

#ifdef __cplusplus
}
#endif
#endif /* _LOCALE_H_ */
