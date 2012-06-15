/***************************************************************************
 *
 * _exception.h - Declarations for the Standard Library exception class
 *
 * This is an internal header file used to implement the C++ Standard
 * Library. It should never be #included directly by a program.
 *
 * $Id: _exception.h 225375 2005-07-26 19:04:10Z sebor $
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

#ifndef _RWSTD_EXCEPTION_H_INCLUDED
#define _RWSTD_EXCEPTION_H_INCLUDED


#include <exception>
#include <rw/_defs.h>


_RWSTD_NAMESPACE (std) { 


// forward declarations avoid circular dependencies
template <class _CharT, class _Traits, class _Allocator>
class basic_string;

template <class _CharT>
struct char_traits;

template <class _TypeT>
class allocator;


typedef basic_string<char, char_traits<char>, allocator<char> > string;

}  // namespace std
_RWSTD_NAMESPACE (__rw) { 
	
// base exception class inherited by all other exceptions
#ifndef __SYMBIAN32__
class _RWSTD_EXPORT __rw_exception: public _STD::exception
{
public:

    __rw_exception () _THROWS (());

    __rw_exception (const __rw_exception&);

    _EXPLICIT __rw_exception (const _STD::string&);
    
    _EXPLICIT __rw_exception (const char*);

    virtual ~__rw_exception () _THROWS (());

    // empty exception specification necessary in order to preserve
    // the no-exception guarantee provided by std::exception to
    // derived classes (e.g., logic_error)
    __rw_exception& operator= (const __rw_exception&) _THROWS (());

    virtual const char* what () const _THROWS (());

    __rw_exception&
    _C_assign (const char*, _RWSTD_SIZE_T = _RWSTD_SIZE_MAX);

private:
    char *_C_what;   // description string
};
#else
class __rw_exception: public _STD::exception
{
public:

    IMPORT_C __rw_exception () _THROWS (());

    IMPORT_C __rw_exception (const __rw_exception&);

   IMPORT_C _EXPLICIT __rw_exception (const _STD::string&);
    
    IMPORT_C _EXPLICIT __rw_exception (const char*);

    IMPORT_C virtual ~__rw_exception () _THROWS (());

    // empty exception specification necessary in order to preserve
    // the no-exception guarantee provided by std::exception to
    // derived classes (e.g., logic_error)
IMPORT_C    __rw_exception& operator= (const __rw_exception&) _THROWS (());

 IMPORT_C   virtual const char* what () const _THROWS (());

 IMPORT_C   __rw_exception&  _C_assign (const char*, _RWSTD_SIZE_T /*= _RWSTD_SIZE_MAX*/);

private:
    char *_C_what;   // description string
};
#endif //__SYMBIAN32__

}   // namespace __rw


#endif   // _RWSTD_EXCEPTION_H_INCLUDED
