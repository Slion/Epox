/**************************************************************************
 *
 * _error.h - Definitions for error messages
 *
 * This is an internal header file used to implement the C++ Standard
 * Library. It should never be #included directly by a program.
 *
 * $Id: _error.h 225375 2005-07-26 19:04:10Z sebor $
 *
 ***************************************************************************
 *
 * Copyright (c) 1994-2005 Quovadx,  Inc., acting through its  Rogue Wave
 * Software division. Licensed under the Apache License, Version 2.0 (the
 * "License");  you may  not use this file except  in compliance with the
 * License.    You    may   obtain   a   copy   of    the   License    at
 * http://www.apache.org/licenses/LICENSE-2.0.    Unless   required    by
 * applicable law  or agreed to  in writing,  software  distributed under
 * the License is distributed on an "AS IS" BASIS,  WITHOUT WARRANTIES OR
 * CONDITIONS OF  ANY KIND, either  express or implied.  See  the License
 * for the specific language governing permissions  and limitations under
 * the License.
 * 
 **************************************************************************/

#ifndef _RWSTD_ERROR_H_INCLUDED
#define _RWSTD_ERROR_H_INCLUDED

#ifndef _RWSTD_NO_INCLUDES

   // prevent inclusion if compiling a .rc file with MSVC
#  include <rw/_defs.h>

_RWSTD_NAMESPACE (__rw) { 

// throws an exception identified by first argument, optional arguments
// (if any) used to format the exception object's what() string
#ifndef __SYMBIAN32__
void _RWSTD_EXPORT __rw_throw (int, ...);
#else
void __rw_throw (int, ...){}
#endif

// throws an exception identified by first argument with the second
// argument containing the exception object's what() string, which
// if non-0, is dynamically allocated and must be delete[]'d
// may be assigned to a user-defined handler (e.g., to prevent
// the library from throwing exceptions or to implement logging)
extern void _RWSTD_EXPORT (*__rw_throw_proc)(int, char*);

}   // namespace __rw

#endif

#ifndef _RWSTD_ERROR_CATALOG
   // name of catalog optionally followed by a colon and a set number
   // catalog looked up according to rules of catopen(3)
   // may be customized at lib build time (pathname okay)
#  define _RWSTD_ERROR_CATALOG "rwstderr:1"
#endif   // _RWSTD_ERROR_CATALOG

#ifndef _RWSTD_ERROR_ENVVAR
   // environment variable name - overrides _RWSTD_ERROR_CATALOG
   // may be customized at lib build time
#  define _RWSTD_ERROR_ENVVAR "RWSTDERR"
#endif   // _RWSTD_ERROR_ENVVAR

#ifndef _RWSTD_ERROR_FIRST
   // id of first message - 1, may be customized at lib build time
#  define _RWSTD_ERROR_FIRST 0
#endif   // _RWSTD_ERROR_FIRST


// these must be macros to accomodate MSVC's resource compiler, with values
// expected to be consecutive starting with _RWSTD_ERROR_FIRST + 1

// #  define _RWSTD_ERROR_EXCEPTION                   (_RWSTD_ERROR_FIRST +  1)
// #  define _RWSTD_ERROR_BAD_EXCEPTION               (_RWSTD_ERROR_FIRST +  2)
// #  define _RWSTD_ERROR_BAD_ALLOC                   (_RWSTD_ERROR_FIRST +  3)
#  define _RWSTD_ERROR_BAD_CAST                    (_RWSTD_ERROR_FIRST +  4)
#  define _RWSTD_ERROR_LOGIC_ERROR                 (_RWSTD_ERROR_FIRST +  5)
#  define _RWSTD_ERROR_DOMAIN_ERROR                (_RWSTD_ERROR_FIRST +  6)
#  define _RWSTD_ERROR_INVALID_ARGUMENT            (_RWSTD_ERROR_FIRST +  7)
#  define _RWSTD_ERROR_LENGTH_ERROR                (_RWSTD_ERROR_FIRST +  8)
#  define _RWSTD_ERROR_OUT_OF_RANGE                (_RWSTD_ERROR_FIRST +  9)
#  define _RWSTD_ERROR_RUNTIME_ERROR               (_RWSTD_ERROR_FIRST + 10)
#  define _RWSTD_ERROR_RANGE_ERROR                 (_RWSTD_ERROR_FIRST + 11)
#  define _RWSTD_ERROR_OVERFLOW_ERROR              (_RWSTD_ERROR_FIRST + 12)
#  define _RWSTD_ERROR_UNDERFLOW_ERROR             (_RWSTD_ERROR_FIRST + 13)

#  define _RWSTD_ERROR_FAILBIT_SET                 (_RWSTD_ERROR_FIRST + 14)
#  define _RWSTD_ERROR_BADBIT_SET                  (_RWSTD_ERROR_FIRST + 15)
#  define _RWSTD_ERROR_EOFBIT_SET                  (_RWSTD_ERROR_FIRST + 16)
#  define _RWSTD_ERROR_IOSTATE_BIT_SET             (_RWSTD_ERROR_FIRST + 17)

#  define _RWSTD_ERROR_FACET_NOT_FOUND             (_RWSTD_ERROR_FIRST + 18)
#  define _RWSTD_ERROR_LOCALE_BAD_NAME             (_RWSTD_ERROR_FIRST + 19)
#  define _RWSTD_ERROR_LOCALE_ERROR_NAME           (_RWSTD_ERROR_FIRST + 20)
#  define _RWSTD_ERROR_CODECVT                     (_RWSTD_ERROR_FIRST + 21)

#  define _RWSTD_ERROR_BAD_POINTER                 (_RWSTD_ERROR_FIRST + 22)
#  define _RWSTD_ERROR_TRANSFORM                   (_RWSTD_ERROR_FIRST + 23)
#  define _RWSTD_ERROR_LOCALE_BAD_CAT              (_RWSTD_ERROR_FIRST + 24)

#  define _RWSTD_ERROR_STRINGS                                  \
          "%s: %s: unspecified error",                          \
          "%s: %s: exception",                                  \
          "%s: %s: unexpected exception",                       \
          "%s: %s: bad_alloc: out of memory",                   \
          "%s: %s: bad cast",                                   \
          "%s: %s: logic error",                                \
          "%s: %s: domain error",                               \
          "%s: %s: invalid argument",                           \
          "%s: %s: length error: size %u out of range [0, %u)", \
          "%s: %s: argument value %u out of range [0, %u)",     \
          "%s: %s: runtime error",                              \
          "%s: %s: range error: invalid range [%d, %d)",        \
          "%s: %s: overflow error",                             \
          "%s: %s: underflow error",                            \
          /* the following take a single arg because */         \
          /* they are generated in a .cpp file       */         \
          "%s: stream object has set ios::failbit",             \
          "%s: stream object has set ios::badbit",              \
          "%s: stream object has set ios::eofbit",              \
          "%s: stream object has set %s",                       \
          "%s: %s: facet %u not found in locale (\"%s\")",      \
          "%s: %s: bad locale name: \"%s\"",                    \
          "%s: %s: failed to construct locale name",            \
          "%s: %s: conversion failed",                          \
          "%s: %s: invalid pointer %p",                         \
          "%s: %s: transformation failed",                      \
          "%s: %s: bad category value: %#x"

#define _RWSTD_ERROR_EXCEPTION     "exception"
#define _RWSTD_ERROR_BAD_EXCEPTION "unexpected exception"
#define _RWSTD_ERROR_BAD_ALLOC     "bad_alloc: out of memory"


#endif   // _RWSTD_ERROR_H_INCLUDED
