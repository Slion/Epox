

/***************************************************************************
 *
 * 22.locale.num.get.cpp - test exercising the std::num_get facet
 *
 * $Id: 22.locale.num.get.cpp 290831 2005-09-21 21:15:42Z sebor $
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

#include <cstdlib>

// Usage:
//   #define NO_GET_${T}, where ${T} is the name of the arithmetic
//   type the num_get<charT>::get(..., ${T}&) overload, to disable
//   the corresponding test
//
//   #define NO_GET_INTEGER to disable tests of all integer types
//   #define NO_GET_FLOATING to disable testst of all floating types
//   #define NO_GET_ERRNO to disable errno tests

#define	SAME_AS_UNIX_COMMENT	
#define	FLT_MIN_NOT_SAME		
#define	FLT_MAX_NOT_SAME

#ifdef NO_GET_INTEGER
#  define NO_GET_BOOL
// no NO_GET_USHRT
#  define NO_GET_SHRT
// no NO_GET_UINT
#  define NO_GET_INT
#  define NO_GET_ULONG
#  define NO_GET_LONG
#  define NO_GET_ULLONG
#  define NO_GET_LLONG
#endif   // NO_GET_INTEGER

#ifdef NO_GET_FLOATING
#  define NO_GET_FLT
#  define NO_GET_DBL
#  define NO_GET_LDBL
#endif   // NO_GET_FLOATING
#define	_RWSTD_DBL_MIN	DBL_MIN
#define	_RWSTD_DBL_MAX	DBL_MAX
#define	_RWSTD_ULONG_MAX ULONG_MAX

#define	_RWSTD_LONG_MAX	LONG_MAX
/**************************************************************************/

#include <cfloat>
#include <climits>   // XXX_MAX, XXX_MIN
#include <clocale>   // for localeconv(), setlocale()
#include <cstdio>    // for sprintf(), sscanf()
#include <cstring>   // for memset(), strerror(), strlen()
#include <cstddef>
#include <cerrno>    // for ERANGE, errno

#include <ios>
#include <limits>
#include <locale>

#include <any.h>         // for rw_any_t
#include <cmdopt.h>      // for rw_enabled()
#include <driver.h>      // for rw_test(), ...
#include <localedef.h>   // for rw_locales()
#include <valcmp.h>      // for rw_equal()

#include"std_log_result.h"
#define LOG_FILENAME_LINE __FILE__, __LINE__
int failures=0;

#ifdef __ARMCC__
#pragma diag_suppress 940
#pragma diag_suppress 815
#pragma diag_suppress 177
#endif

#ifdef __SYMBIAN32__
#include<assert.h>
#endif //__SYMBIAN32__
/**************************************************************************/
//#if 0
// set by the command line option handler in response to:
static int rw_opt_no_errno       = 0;   // --no-errno
static int rw_opt_no_grouping    = 0;   // --no-grouping
static int rw_opt_no_widen       = 0;   // --no-widen
static int rw_opt_no_uflow       = 0;   // --no-underflow

/**************************************************************************/

// replacement ctype facet

template <class charT>
struct Ctype: std::ctype<charT>
{
    typedef std::ctype<charT> Base;

    typedef typename Base::char_type char_type;

    static int n_widen_;

    Ctype (): Base (0, 0, 1) { }

    virtual char_type do_widen (char c) const {
        ++n_widen_;

        switch (c) {
        case '0': c = '9'; break;
        case '1': c = '8'; break;
        case '2': c = '7'; break;
        case '3': c = '6'; break;
        case '4': c = '5'; break;
        case '5': c = '4'; break;
        case '6': c = '3'; break;
        case '7': c = '2'; break;
        case '8': c = '1'; break;
        case '9': c = '0'; break;
        default: break;
        }

        return char_type (c);
    }

    virtual const char_type*
    do_widen (const char *lo, const char *hi, char_type *dst) const {
        return Base::do_widen (lo, hi, dst);
    }
};

template <class charT>
int Ctype<charT>::n_widen_;


/**************************************************************************/

// replacement numpunct facet

template <class charT>
struct Punct: std::numpunct<charT>
{
    typedef typename std::numpunct<charT>::char_type   char_type;
    typedef typename std::numpunct<charT>::string_type string_type;

    static char_type        decimal_point_;
    static char_type        thousands_sep_;
    static const char      *grouping_;
    static const char_type *truename_;
    static const char_type *falsename_;

    static int n_objs_;            // number of facet objects in existence
    static int n_thousands_sep_;   // number of calls to do_thousands_sep()

    Punct (std::size_t ref = 0)
        : std::numpunct<charT>(ref) {
        ++n_objs_;
    }

    ~Punct () {
        --n_objs_;
    }

    virtual char_type do_decimal_point () const {
        return decimal_point_;
    }

    virtual std::string do_grouping () const {
        return grouping_;
    }

    virtual char_type do_thousands_sep () const {
        ++n_thousands_sep_;
        return thousands_sep_;
    }

    virtual string_type do_truename () const {
        return truename_ ? string_type (truename_) : string_type ();
    }

    virtual string_type do_falsename () const {
        return falsename_ ? string_type (falsename_) : string_type ();
    }
};

template <class charT>
const char* Punct<charT>::grouping_ = "";

template <class charT>
typename Punct<charT>::char_type Punct<charT>::decimal_point_ = '.';

template <class charT>
typename Punct<charT>::char_type Punct<charT>::thousands_sep_ = ',';

template <class charT>
const typename Punct<charT>::char_type* Punct<charT>::truename_;

template <class charT>
const typename Punct<charT>::char_type* Punct<charT>::falsename_;

template <class charT>
int Punct<charT>::n_thousands_sep_;

template <class charT>
int Punct<charT>::n_objs_;

/**************************************************************************/

template <class charT>
struct Ios: std::basic_ios<charT>
{
    Ios () { this->init (0); }
};


template <class charT>
struct NumGet: std::num_get<charT, const charT*>
{
    NumGet () { /* working around a bug in older versions of EDG eccp */ }
};


/**************************************************************************/

#define TEST   do_test
#define T      __LINE__, charT ()

// if non-zero expected to point to a maximum valid value
// of type T that's being tested below (used for floating
// point ranges)
// on function return, the pointed to value is overwritten
// with the actual extracted value
void *pmax = 0;

template <class nativeT>
class rw_any_template_native_t
{
public:
	rw_any_template_native_t(nativeT aNativeT) : iNativeT(aNativeT)
	{
	}
	const char* const type_name ()
	{
	iNativeT.type_name();
	}
private:
	nativeT iNativeT;
};



template <class charT, class nativeT>
int do_test (int         lineno,          // line number
             charT       /* dummy */,
             nativeT     val,             // value expected to be extracted
             const char *str,             // input sequence
             int         consumed = -1,   // number of consumed characters
             int         flags = 0,       // initial set of flags
             int         err_expect = -1, // expected iostate
             const char *grouping = "")   // optional grouping string
{
    if (!rw_enabled (lineno)) {
        rw_note (0, __FILE__, __LINE__, "test on line %d disabled", lineno);
        return 0;
    }
    static const char* const cname = rw_any_t (charT ()).type_name ();
    static const char* const tname =cname;//rw_any_template_native_t<nativeT>(nativeT ()).type_name (); //FIX

    // create a distinct punctuation facet for each iteration to make sure
    // any data cached in between successive calls to the facet's public
    // member functions are flushed
    const Punct<charT> pun (1);

    Ios<charT> io;
    NumGet<charT> ng;

    io.imbue (std::locale (io.getloc (),
                           (const std::numpunct<charT>*)&pun));

    io.flags (std::ios_base::fmtflags (flags));
    pun.grouping_ = grouping;

    // initialize x to a value other than the expected one except when
    // get() is expected to fail (in which case the value isn't supposed
    // to be modified, except when the failure is caused by an invalid
    // grouping)
    // doing so exercises the requirement that the get() functions not
    // modify their argument on failure (except when the failure is due
    // to bad grouping)
    nativeT x =
        err_expect & std::ios::failbit || *grouping ? val : nativeT (!val);

    charT wstr_buf [256];   // small buffer to widen `str' to
    charT *wstr = 0;        // points to wstr_buf if large enough

    const charT *next;    // iterator to the next character where to parse

    char nbuf [4096];    // narrow character buffer

    // if the string `str' starts with a '%', treat it as a printf()
    // format specifier and take the string produced by sprintf()
    // below as the expected result, otherwise treat it literally
    // as the expected result

#ifdef __SYMBIAN32__
    if ('%' == *str && 0 < sprintf (nbuf, str, val))
        str = nbuf;
     const size_t str_len = strlen (str);   
#else
    if ('%' == *str && 0 < std::sprintf (nbuf, str, val))
        str = nbuf;
   const std::size_t str_len = std::strlen (str);     
#endif //__SYMBIAN32__
    // convert the narrow character string `str' to a wide character
    // string `wstr' if charT is wider than char

    

    if (sizeof (charT) > sizeof (char)) {

        // widen `str' to the wide character buffer if the latter
        // is large enough, or to a dynamically allocated buffer
        // otherwise
        wstr = str_len < sizeof wstr_buf / sizeof *wstr_buf ?
            wstr_buf : new charT [str_len + 1];

        typedef unsigned char UChar;

        for (std::size_t j = 0; j != str_len + 1; ++j)
            wstr [j] = charT (UChar (str [j]));

        next = wstr;

    }
    else {

        // charT == char, just point next at `str'
        next = (const charT*)str;
    }

    std::ios_base::iostate err = std::ios_base::goodbit;

    const charT *last = next + std::char_traits<charT>::length (next);

    last = ng.get (next, last, io, err, x);

    // do not test the number of extracted characters
    // if (consumed == -1) holds
    int success = -1 == consumed || last - next == consumed;

    int nfailures = !success;
#ifndef __SYMBIAN32__
    rw_assert (success, 0, lineno,
               "line %d: num_get<%s>::get (%#s, ..., %s&); "
               "fmtflags = %{If}; "
               "%{?}grouping = %#s; %{;}"
               "ate %td, expected %d",
               __LINE__, cname, str, tname,
               flags,
               *grouping && '%' != *grouping, grouping,
               last - next, consumed);
#else
 if(!success)
 {
  failures++;
  std_log(LOG_FILENAME_LINE,"Reason:Failing");
  }
  #endif
    const nativeT val_max = pmax ? *(nativeT*)pmax : val;
    // if pmax is non-zero it points to the upper bound of the range
    // that the extracted value can be in
    success =
           err == err_expect
        && (rw_equal (x, val) || val <= x && x <= val_max);

    nfailures += !success;
   #ifndef __SYMBIAN32__
    rw_assert (success, 0, lineno,
               "line %d: num_get<%s>::get (%{*Ac}, ..., %s&); "
               "fmtflags = %{If}; "
               "%{?}grouping = %#s; %{;}"
               "got %s (%{Is}); "
               "expected %s%{?} <= %s %{;} "
               "(%{Is})",
               __LINE__, cname, sizeof *str, str, tname,
               flags,
               *grouping && '%' != *grouping, grouping,
               TOSTR (x), err,
               TOSTR (val), 0 != pmax, TOSTR (val_max),
               err_expect);
   #else
    if(!success)
    {
     failures++;
     std_log(LOG_FILENAME_LINE,"Reason:Failing");
     }
    #endif //__SYMBIAN32__ 
    if (pmax) {
        // store the extracted value into the pointed to location
        *(nativeT*)pmax = x;
    }

    if (*grouping && '%' == *grouping) {

        // if the `grouping' string starts with a '%', treat it as
        // a scanf() format specifier and use sscanf() to parse the
        // tested string `str'; then compare for equality the number
        // parsed with sscanf() with that parsed with num_get::get()
        const char* const scanspec = grouping;

        nativeT y = nativeT ();

        #ifdef __SYMBIAN32__
        const int n = sscanf (str, scanspec, &y);
        #else
        const int n = std::sscanf (str, scanspec, &y);
        #endif //__SYMBIAN32__

        success =
            !(   err & std::ios::failbit    && n > 0
              || !(err & std::ios::failbit) && 1 != n
              || !rw_equal (x, y));

        nfailures += !success;

      #ifndef __SYMBIAN32__
        rw_assert (success, 0, lineno,
                   "line %d: num_get<%s>::get (%#s, ..., %s&); "
                   "got %s, but scanf (..., \"%s\", ...) extracted %s "
                   "and returned %d; fmtflags = %{If}, iostate = %{Is}",
                   __LINE__, cname, str, tname,
                   TOSTR (x), scanspec, TOSTR (y),
                   n, flags, err);
       #else
        if(!success)
        {
         failures++;
         std_log(LOG_FILENAME_LINE,"Reason:Failing") ;
         }  
         #endif//__SYMBIAN32__         
    }

    if (wstr != wstr_buf)
        delete[] wstr;

    return nfailures;
}

/**************************************************************************/

// for convenience
#define boolalpha   std::ios_base::boolalpha
#define dec         std::ios_base::dec
#define fixed       std::ios_base::fixed
#define hex         std::ios_base::hex
#define internal    std::ios_base::internal
#define left        std::ios_base::left
#define oct         std::ios_base::oct
#define right       std::ios_base::right
#define scientific  std::ios_base::scientific
#define showbase    std::ios_base::showbase
#define showpoint   std::ios_base::showpoint
#define showpos     std::ios_base::showpos
#define skipws      std::ios_base::skipws
#define unitbuf     std::ios_base::unitbuf
#define uppercase   std::ios_base::uppercase
#define bin         std::ios_base::bin
#define adjustfield std::ios_base::adjustfield
#define basefield   std::ios_base::basefield
#define floatfield  std::ios_base::floatfield
#define nolock      std::ios_base::nolock
#define nolockbuf   std::ios_base::nolockbuf

#define Bad         std::ios_base::badbit
#define Eof         std::ios_base::eofbit
#define Fail        std::ios_base::failbit
#define Good        std::ios_base::goodbit

/*************************************************************************/
template <class numT>
class rw_any_template_t
{
public:
	rw_any_template_t(numT aNumT) : iNumT(aNumT)
	{
	}
	const char* const type_name ()
	{
	iNumT.type_name();
	}
private:
	numT iNumT;
};

template <class charT, class numT>
void test_errno (charT, numT)
{
    // verify that errno doesn't change after, or affects
    // a successful extraction

    const char* const cname = rw_any_t (charT ()).type_name ();
    const char* const tname = cname;//rw_any_template_t <numT>(numT()).type_name (); //FIX

    rw_info (0, 0, 0, "std::num_get<%s>::get (..., %s&) and errno",
             cname, tname);

    const int errnos[] = {
        ERANGE, -4, -3, -2, -1, 0, 1, 2, 3, 4,
        Bad, Eof, Fail, Bad | Eof, Bad | Fail, Eof | Fail, Bad | Eof | Fail
    };

    int i;
#define TEST_ERRNO(val)                                              \
    for (i = 0; i != sizeof errnos / sizeof *errnos; ++i) {          \
        errno = errnos [i];                                          \
        TEST (T, numT (val), _RWSTD_STR (val),                       \
              sizeof (_RWSTD_STR (val)) -1, 0, Eof);                 \
         if(errnos [i] != errno)                                     \
         {                                                           \
           failures++;                                               \
         std_log(LOG_FILENAME_LINE,"%d. errno unexpectedly changed from %d to %d",   \
                    i, errnos [i], errno);                           \
                    }                                                 \
    }
    
   /* 
  void test1(int val)  
  {
     int i;
       for (i = 0; i != sizeof errnos / sizeof *errnos; ++i) {       
               errno = errnos [i];                                          
        TEST (T, numT (val), _RWSTD_STR (val),                       
              sizeof (_RWSTD_STR (val)) -1, 0, Eof);                 
        if(errnos [i] != errno)
        {
         failures++;
         std_log(LOG_FILENAME_LINE,"%d. errno unexpectedly changed from %d to %d",   i, errnos [i], errno);                                        
        }
                 
    }
        
  };
  
  #define TEST_ERRNO (val)   test1(val) 
      
  */
    // prevent warnings when numT is unsigned
    numT zero      = numT ();
    numT minus_one = numT (~0);

   std_log(LOG_FILENAME_LINE,"Test start");
    TEST_ERRNO (0);
   std_log(LOG_FILENAME_LINE,"Test end");
   std_log(LOG_FILENAME_LINE,"Test start"); 
    TEST_ERRNO (1);
   std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST_ERRNO (12);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST_ERRNO (123);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST_ERRNO (1234);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST_ERRNO (12345);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST_ERRNO (12346);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST_ERRNO (12347);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST_ERRNO (12348);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST_ERRNO (12349);
 std_log(LOG_FILENAME_LINE,"Test end");
    if (minus_one > zero)
        return;
std_log(LOG_FILENAME_LINE,"Test start");
    TEST_ERRNO (-1);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST_ERRNO (-12);
     std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST_ERRNO (-123);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST_ERRNO (-1234);
     std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST_ERRNO (-12345);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
     TEST_ERRNO (-12346);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST_ERRNO (-12347);
     std_log(LOG_FILENAME_LINE,"Test end");
     std_log(LOG_FILENAME_LINE,"Test start");
    TEST_ERRNO (-12348);
     std_log(LOG_FILENAME_LINE,"Test end");
 std_log(LOG_FILENAME_LINE,"Test start");
    TEST_ERRNO (-12349);
  std_log(LOG_FILENAME_LINE,"Test end");
  std_log(LOG_FILENAME_LINE,"Test start");
}

/**************************************************************************/

template <class charT>
void test_bool (charT, const char *cname)
{
#ifndef NO_GET_BOOL

    const char* const tname = "bool";

    rw_info (0, 0, 0, "std::num_get<%s>::get (..., %s&)", cname, tname);

    Punct<charT>::decimal_point_ = '.';
    Punct<charT>::thousands_sep_ = ',';

    {
        static const charT names[][6] = {
            { 'f', 'a', 'l', 's', 'e', '\0' },
            { 't', 'r', 'u', 'e', '\0' }
        };

        Punct<charT>::falsename_ = names [0];
        Punct<charT>::truename_  = names [1];
    }

    // lwg issue 17: special treatment for bool:

    // The in iterator is always left pointing one position beyond
    // the last character successfully matched. If val is set, then
    // err is set to str.good; or to str.eof if, when seeking
    // another character to match, it is found that (in==end). If
    // val is not set, then err is set to str.fail; or to
    // (str.fail|str.eof) if the reason for the failure was
    // that (in==end). [Example: for targets true:"a" and false:"abb",
    // the input sequence "a" yields val==true and err==str.eof;
    // the input sequence "abc" yields err=str.fail, with in ending
    // at the 'c' element. For targets true:"1" and false:"0", the
    // input sequence "1" yields val==true and err=str.good. For
    // empty targets (""), any input sequence yields err==str.fail.
    // --end example]

    // arguments are passed in the following order:
    //
    // [v]   (out)      N/A   expected value
    // [s]   (in)       N/A   character string to parse or a printf
    //                        specifier to format [v] with
    // [n]   (out)       -1   number of characters consumed
    // [f]   (in)         0   iosflags
    // [e]   (in|out)    -1   expected iostate after parsing
    // [g]   (in)        ""   grouping string, or a scanf
    //                        specifier to parse [s] with

    // bool, numeric parsing
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, false, "0", 1, 0, Eof);
     std_log(LOG_FILENAME_LINE,"Test end");
     std_log(LOG_FILENAME_LINE,"Test start");

    TEST (T, true,  "1", 1, 0, Eof);
     std_log(LOG_FILENAME_LINE,"Test end");
     std_log(LOG_FILENAME_LINE,"Test start");

    TEST (T, false, "2", 1, 0, Fail | Eof);
     std_log(LOG_FILENAME_LINE,"Test end");
     std_log(LOG_FILENAME_LINE,"Test start");

    TEST (T, false, "0x0",  3, 0, Eof);
     std_log(LOG_FILENAME_LINE,"Test end");
     std_log(LOG_FILENAME_LINE,"Test start");

    TEST (T, true,  "0x1",  3, 0, Eof);
     std_log(LOG_FILENAME_LINE,"Test end");
     std_log(LOG_FILENAME_LINE,"Test start");

    TEST (T, true,  "0x01", 4, 0, Eof);
     std_log(LOG_FILENAME_LINE,"Test end");
     std_log(LOG_FILENAME_LINE,"Test start");

    TEST (T, false, "0x20", 4, 0, Fail | Eof);
     std_log(LOG_FILENAME_LINE,"Test end");
     std_log(LOG_FILENAME_LINE,"Test start");

    TEST (T, false, "",     0, 0, Fail | Eof);
     std_log(LOG_FILENAME_LINE,"Test end");
     std_log(LOG_FILENAME_LINE,"Test start");

    TEST (T, false, "0+", 1, 0, Good);
     std_log(LOG_FILENAME_LINE,"Test end");
     std_log(LOG_FILENAME_LINE,"Test start");

    TEST (T, true,  "1+", 1, 0, Good);
     std_log(LOG_FILENAME_LINE,"Test end");
     std_log(LOG_FILENAME_LINE,"Test start");

    TEST (T, false, "2+", 1, 0, Fail);
     std_log(LOG_FILENAME_LINE,"Test end");
     std_log(LOG_FILENAME_LINE,"Test start");

    TEST (T, false, "0-", 1, 0, Good);
     std_log(LOG_FILENAME_LINE,"Test end");
     std_log(LOG_FILENAME_LINE,"Test start");

    TEST (T, true,  "1-", 1, 0, Good);
     std_log(LOG_FILENAME_LINE,"Test end");
     std_log(LOG_FILENAME_LINE,"Test start");

    TEST (T, false, "2-", 1, 0, Fail);
     std_log(LOG_FILENAME_LINE,"Test end");
     std_log(LOG_FILENAME_LINE,"Test start");

    TEST (T, false, "00",  2, 0, Eof);
     std_log(LOG_FILENAME_LINE,"Test end");
     std_log(LOG_FILENAME_LINE,"Test start");

    TEST (T, true,  "01",  2, 0, Eof);
     std_log(LOG_FILENAME_LINE,"Test end");
     std_log(LOG_FILENAME_LINE,"Test start");

    TEST (T, false, "02",  2, 0, Fail | Eof);
     std_log(LOG_FILENAME_LINE,"Test end");
     std_log(LOG_FILENAME_LINE,"Test start");

    TEST (T, false, "012", 3, 0, Fail | Eof);
     std_log(LOG_FILENAME_LINE,"Test end");
     std_log(LOG_FILENAME_LINE,"Test start");

    TEST (T, false, "+0 ", 2, 0, Good);
     std_log(LOG_FILENAME_LINE,"Test end");
     std_log(LOG_FILENAME_LINE,"Test start");

    TEST (T, true,  "+1 ", 2, 0, Good);
     std_log(LOG_FILENAME_LINE,"Test end");
     std_log(LOG_FILENAME_LINE,"Test start");

    TEST (T, false, "+2 ", 2, 0, Fail);
     std_log(LOG_FILENAME_LINE,"Test end");
     std_log(LOG_FILENAME_LINE,"Test start");

    TEST (T, false, "-0 ", 2, 0, Good);
     std_log(LOG_FILENAME_LINE,"Test end");
     std_log(LOG_FILENAME_LINE,"Test start");

    TEST (T, false, "-1 ", 2, 0, Fail);
     std_log(LOG_FILENAME_LINE,"Test end");
     std_log(LOG_FILENAME_LINE,"Test start");

    TEST (T, false, "-2 ", 2, 0, Fail);

     std_log(LOG_FILENAME_LINE,"Test end");
     std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, false, "++1", 1, 0, Fail);
     std_log(LOG_FILENAME_LINE,"Test end");
     std_log(LOG_FILENAME_LINE,"Test start");

    TEST (T, false, "+-1", 1, 0, Fail);
     std_log(LOG_FILENAME_LINE,"Test end");
     std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, false, "-+1", 1, 0, Fail);
     std_log(LOG_FILENAME_LINE,"Test end");
     std_log(LOG_FILENAME_LINE,"Test start");

    TEST (T, false, "--1", 1, 0, Fail);
     std_log(LOG_FILENAME_LINE,"Test end");
     std_log(LOG_FILENAME_LINE,"Test start");

    TEST (T, false, "+000+",  4, 0, Good);
     std_log(LOG_FILENAME_LINE,"Test end");
     std_log(LOG_FILENAME_LINE,"Test start");

    TEST (T, false, "+000-",  4, 0, Good);
     std_log(LOG_FILENAME_LINE,"Test end");
     std_log(LOG_FILENAME_LINE,"Test start");

    TEST (T, true,  "+001-",  4, 0, Good);
     std_log(LOG_FILENAME_LINE,"Test end");
     std_log(LOG_FILENAME_LINE,"Test start");

    TEST (T, false, "+002",   4, 0, Eof | Fail);
     std_log(LOG_FILENAME_LINE,"Test end");
     std_log(LOG_FILENAME_LINE,"Test start");

    TEST (T, false, "-000-",  4, 0, Good);

     std_log(LOG_FILENAME_LINE,"Test end");
     std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, false, "-000+",  4, 0, Good);

     std_log(LOG_FILENAME_LINE,"Test end");
     std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, false, "-001- ", 4, 0, Fail);
     std_log(LOG_FILENAME_LINE,"Test end");
     std_log(LOG_FILENAME_LINE,"Test start");

    TEST (T, false, "-002- ", 4, 0, Fail);
     std_log(LOG_FILENAME_LINE,"Test end");
     std_log(LOG_FILENAME_LINE,"Test start");

    TEST (T, false, "-012- ", 4, 0, Fail);
     std_log(LOG_FILENAME_LINE,"Test end");
     std_log(LOG_FILENAME_LINE,"Test start");

    TEST (T, false, "0.0", 1, 0, Good);
     std_log(LOG_FILENAME_LINE,"Test end");
     std_log(LOG_FILENAME_LINE,"Test start");

    TEST (T, true,  "1.1", 1, 0, Good);
     std_log(LOG_FILENAME_LINE,"Test end");
     std_log(LOG_FILENAME_LINE,"Test start");

    TEST (T, false, "-0.00 ", 2, 0, Good);
     std_log(LOG_FILENAME_LINE,"Test end");
     std_log(LOG_FILENAME_LINE,"Test start");

    TEST (T, false, "+00.01", 3, 0, Good);
     std_log(LOG_FILENAME_LINE,"Test end");
     std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, true,  "+1.12.", 2, 0, Good);
     std_log(LOG_FILENAME_LINE,"Test end");
     std_log(LOG_FILENAME_LINE,"Test start");

    // bool, numeric parsing with grouping

    Punct<charT>::thousands_sep_ = ';';

    TEST (T, false,        "0;0",  3, 0, Eof, "\1");

     std_log(LOG_FILENAME_LINE,"Test end");
     std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, false,       "+0;0",  4, 0, Eof, "\1");

     std_log(LOG_FILENAME_LINE,"Test end");
     std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, false,       "-0;0",  4, 0, Eof, "\1");
     std_log(LOG_FILENAME_LINE,"Test end");
     std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, false,      "0;0;0",  5, 0, Eof, "\1");
     std_log(LOG_FILENAME_LINE,"Test end");
     std_log(LOG_FILENAME_LINE,"Test start");

    TEST (T, false,    "0;0;0;0",  7, 0, Eof, "\1");
     std_log(LOG_FILENAME_LINE,"Test end");
     std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, true,          "01",  2, 0, Eof, "\2");
     std_log(LOG_FILENAME_LINE,"Test end");
     std_log(LOG_FILENAME_LINE,"Test start");

    TEST (T, true,        "0;01",  4, 0, Eof, "\2");
     std_log(LOG_FILENAME_LINE,"Test end");
     std_log(LOG_FILENAME_LINE,"Test start");

    TEST (T, true,       "00;01",  5, 0, Eof, "\2");
        std_log(LOG_FILENAME_LINE,"Test end");
     std_log(LOG_FILENAME_LINE,"Test start");

    TEST (T, true,        "0;01",  4, 0, Eof, "\2");
     std_log(LOG_FILENAME_LINE,"Test end");
     std_log(LOG_FILENAME_LINE,"Test start");

    TEST (T, false,       "00;0",  4, 0, Eof, "\1\2");
     std_log(LOG_FILENAME_LINE,"Test end");
     std_log(LOG_FILENAME_LINE,"Test start");

    TEST (T, false,     "0;00;0",  6, 0, Eof, "\1\2\3");
     std_log(LOG_FILENAME_LINE,"Test end");
     std_log(LOG_FILENAME_LINE,"Test start");

    TEST (T, false,   "000;00;0",  8, 0, Eof, "\1\2\3");
     std_log(LOG_FILENAME_LINE,"Test end");
     std_log(LOG_FILENAME_LINE,"Test start");

    TEST (T, false, "0;000;00;0", 10, 0, Eof, "\1\2\3");
     std_log(LOG_FILENAME_LINE,"Test end");
     std_log(LOG_FILENAME_LINE,"Test start");

    TEST (T, true,        "00;1",  4, 0, Eof, "\1\2");
     std_log(LOG_FILENAME_LINE,"Test end");
     std_log(LOG_FILENAME_LINE,"Test start");

    TEST (T, true,      "0;00;1",  6, 0, Eof, "\1\2\3");
     std_log(LOG_FILENAME_LINE,"Test end");
     std_log(LOG_FILENAME_LINE,"Test start");

    TEST (T, true,    "000;00;1",  8, 0, Eof, "\1\2\3");
     std_log(LOG_FILENAME_LINE,"Test end");
     std_log(LOG_FILENAME_LINE,"Test start");

    TEST (T, true,  "0;000;00;1", 10, 0, Eof, "\1\2\3");
     std_log(LOG_FILENAME_LINE,"Test end");
     std_log(LOG_FILENAME_LINE,"Test start");

    TEST (T, true, "+0;000;00;1", 11, 0, Eof, "\1\2\3");
     std_log(LOG_FILENAME_LINE,"Test end");
     std_log(LOG_FILENAME_LINE,"Test start");

    // bad groups, invalid input: do not store value, set failbit
    // the format of integers in 22.2.3.1, p2 is:
    //
    //   integer   ::= [sign] units
    //   sign      ::= plusminus [whitespace]
    //   plusminus ::= '+' | '-'
    //   units     ::= digits [thousands-sep units]
    //   digits    ::= digit [digits]
    //
    // i.e., thousands-sep may only appear at most one at a time and
    // only when enclosed within a sequence of digits and not first
    // or last

    TEST (T, false,      ";0", 0, 0, Fail,       "\1");
     std_log(LOG_FILENAME_LINE,"Test end");
     std_log(LOG_FILENAME_LINE,"Test start");

    TEST (T, false,      "0;", 2, 0, Eof | Fail, "\1");
     std_log(LOG_FILENAME_LINE,"Test end");
     std_log(LOG_FILENAME_LINE,"Test start");

    TEST (T, false,     "0;;", 2, 0, Fail,       "\1");
     std_log(LOG_FILENAME_LINE,"Test end");
     std_log(LOG_FILENAME_LINE,"Test start");

    TEST (T, false,    ";0;0", 0, 0, Fail,       "\1");
     std_log(LOG_FILENAME_LINE,"Test end");
     std_log(LOG_FILENAME_LINE,"Test start");

    TEST (T, false,     ";01", 0, 0, Fail,       "\2");
     std_log(LOG_FILENAME_LINE,"Test end");
     std_log(LOG_FILENAME_LINE,"Test start");

    TEST (T, false,     "01;", 3, 0, Eof | Fail, "\2");
     std_log(LOG_FILENAME_LINE,"Test end");
     std_log(LOG_FILENAME_LINE,"Test start");

    TEST (T, false,    "01;;", 3, 0, Fail,       "\2");
     std_log(LOG_FILENAME_LINE,"Test end");
     std_log(LOG_FILENAME_LINE,"Test start");

    TEST (T, false,  ";;0;01", 0, 0, Fail,       "\2");
     std_log(LOG_FILENAME_LINE,"Test end");
     std_log(LOG_FILENAME_LINE,"Test start");

    TEST (T, false,  "00;;01", 3, 0, Fail,       "\2");
     std_log(LOG_FILENAME_LINE,"Test end");
     std_log(LOG_FILENAME_LINE,"Test start");

    TEST (T, false, "00;;;01", 3, 0, Fail,       "\2");
     std_log(LOG_FILENAME_LINE,"Test end");
     std_log(LOG_FILENAME_LINE,"Test start");

    TEST (T, false,    ";;11", 0, 0, Fail,       "\3");
     std_log(LOG_FILENAME_LINE,"Test end");
     std_log(LOG_FILENAME_LINE,"Test start");

    TEST (T, false,   "+;;12", 1, 0, Fail,       "\3");
     std_log(LOG_FILENAME_LINE,"Test end");
     std_log(LOG_FILENAME_LINE,"Test start");

    TEST (T, false,   "-;;13", 1, 0, Fail,       "\3");
     std_log(LOG_FILENAME_LINE,"Test end");
     std_log(LOG_FILENAME_LINE,"Test start");

    TEST (T, false,   "01;;4", 3, 0, Fail,       "\3");
     std_log(LOG_FILENAME_LINE,"Test end");
     std_log(LOG_FILENAME_LINE,"Test start");

    // bad groups, valid input: store value and set failbit
    // this is different from the above in that the integer grammar
    // is satisifed but the positions of the thousands_sep characters
    // in otherwise valid input do not match the specified grouping

    TEST (T, true, "00;0;1", 6, 0, Eof | Fail, "\1");
     std_log(LOG_FILENAME_LINE,"Test end");
#ifndef	SAME_AS_UNIX_COMMENT     
     std_log(LOG_FILENAME_LINE,"Test start");

    // bool, boolalpha parsing
    TEST (T, false, "false", 5, boolalpha, Good);
     std_log(LOG_FILENAME_LINE,"Test end");
     std_log(LOG_FILENAME_LINE,"Test start");

    TEST (T, true,  "true",  4, boolalpha, Good);
     std_log(LOG_FILENAME_LINE,"Test end");
#endif          
     std_log(LOG_FILENAME_LINE,"Test start");

    TEST (T, false, "falsee", 5, boolalpha, Good);
     std_log(LOG_FILENAME_LINE,"Test end");
     std_log(LOG_FILENAME_LINE,"Test start");

    TEST (T, true,  "truee",  4, boolalpha, Good);
     std_log(LOG_FILENAME_LINE,"Test end");
     std_log(LOG_FILENAME_LINE,"Test start");

    TEST (T, false, "False", 0, boolalpha, Fail);
     std_log(LOG_FILENAME_LINE,"Test end");
     std_log(LOG_FILENAME_LINE,"Test start");

    TEST (T, false, "True",  0, boolalpha, Fail);
     std_log(LOG_FILENAME_LINE,"Test end");
     std_log(LOG_FILENAME_LINE,"Test start");

    TEST (T, false, "falsE", 4, boolalpha, Fail);
     std_log(LOG_FILENAME_LINE,"Test end");
     std_log(LOG_FILENAME_LINE,"Test start");

    TEST (T, false, "truE",  3, boolalpha, Fail);
     std_log(LOG_FILENAME_LINE,"Test end");
     std_log(LOG_FILENAME_LINE,"Test start");

    TEST (T, false, "fals", 4, boolalpha, Eof | Fail);
     std_log(LOG_FILENAME_LINE,"Test end");
     std_log(LOG_FILENAME_LINE,"Test start");

    TEST (T, false, "tru",  3, boolalpha, Eof | Fail);
     std_log(LOG_FILENAME_LINE,"Test end");
     std_log(LOG_FILENAME_LINE,"Test start");

    TEST (T, false, "fal", 3, boolalpha, Eof | Fail);
     std_log(LOG_FILENAME_LINE,"Test end");
     std_log(LOG_FILENAME_LINE,"Test start");

    TEST (T, false, "tr",  2, boolalpha, Eof | Fail);
     std_log(LOG_FILENAME_LINE,"Test end");
     std_log(LOG_FILENAME_LINE,"Test start");

    TEST (T, false, "fa", 2, boolalpha, Eof | Fail);
     std_log(LOG_FILENAME_LINE,"Test end");
     std_log(LOG_FILENAME_LINE,"Test start");

    TEST (T, false, "t",  1, boolalpha, Eof | Fail);
     std_log(LOG_FILENAME_LINE,"Test end");
     std_log(LOG_FILENAME_LINE,"Test start");

    TEST (T, false, "f", 1, boolalpha, Eof | Fail);
     std_log(LOG_FILENAME_LINE,"Test end");
     std_log(LOG_FILENAME_LINE,"Test start");

    TEST (T, false, "",  0, boolalpha, Eof | Fail);
     std_log(LOG_FILENAME_LINE,"Test end");


    {
        static const charT names[][7] = {
            { 'b', 'o', 'o', 'l', ':', '-', '\0' },
            { 'b', 'o', 'o', 'l', ':', '+', '\0' }
        };

        Punct<charT>::falsename_ = names [0];
        Punct<charT>::truename_  = names [1];
    }

     std_log(LOG_FILENAME_LINE,"Test start");

    TEST (T, false, "false", 0, boolalpha, Fail);
     std_log(LOG_FILENAME_LINE,"Test end");
     std_log(LOG_FILENAME_LINE,"Test start");

    TEST (T, false, "true",  0, boolalpha, Fail);
     std_log(LOG_FILENAME_LINE,"Test end");
     std_log(LOG_FILENAME_LINE,"Test start");
#ifndef	SAME_AS_UNIX_COMMENT 
    TEST (T, false, "bool:-", 6, boolalpha, Good);
     std_log(LOG_FILENAME_LINE,"Test end");
     std_log(LOG_FILENAME_LINE,"Test start");

    TEST (T, true,  "bool:+", 6, boolalpha, Good);
     std_log(LOG_FILENAME_LINE,"Test end");
#endif     
     std_log(LOG_FILENAME_LINE,"Test start");

    TEST (T, false, "bool: ", 5, boolalpha, Fail);
     std_log(LOG_FILENAME_LINE,"Test end");
     std_log(LOG_FILENAME_LINE,"Test start");

    TEST (T, false, "bool: ", 5, boolalpha, Fail);
     std_log(LOG_FILENAME_LINE,"Test end");



    {
        static const charT names[][8] = {
            { 'B', 'o', 'o', 'l', '\0' },
            { 'B', 'o', 'o', 'l', 'e', 'a', 'n', '\0' }
        };

        Punct<charT>::falsename_ = names [0];
        Punct<charT>::truename_  = names [1];
    }

     std_log(LOG_FILENAME_LINE,"Test start");

    TEST (T, false, "false", 0, boolalpha, Fail);
     std_log(LOG_FILENAME_LINE,"Test end");
     std_log(LOG_FILENAME_LINE,"Test start");

    TEST (T, false, "true",  0, boolalpha, Fail);
     std_log(LOG_FILENAME_LINE,"Test end");
     std_log(LOG_FILENAME_LINE,"Test start");

    TEST (T, false, "Bool",    4, boolalpha, Eof);
     std_log(LOG_FILENAME_LINE,"Test end");
#ifndef	SAME_AS_UNIX_COMMENT 
     std_log(LOG_FILENAME_LINE,"Test start");

    TEST (T, true,  "Boolean", 7, boolalpha, Good);
     std_log(LOG_FILENAME_LINE,"Test end");
     std_log(LOG_FILENAME_LINE,"Test start");

    TEST (T, false, "Boole",  5, boolalpha, Eof | Fail);
     std_log(LOG_FILENAME_LINE,"Test end");
     std_log(LOG_FILENAME_LINE,"Test start");

    TEST (T, false, "Boolea", 6, boolalpha, Eof | Fail);
     std_log(LOG_FILENAME_LINE,"Test end");
#endif


    {
        static const charT names[][8] = {
            { 'B', 'O', 'O', 'L', 'E', 'A', 'N', '\0' },
            { 'B', 'O', 'O', 'L', '\0' }
        };

        Punct<charT>::falsename_ = names [0];
        Punct<charT>::truename_  = names [1];
    }

     std_log(LOG_FILENAME_LINE,"Test start");

    TEST (T, false, "false", 0, boolalpha, Fail);
     std_log(LOG_FILENAME_LINE,"Test end");
     std_log(LOG_FILENAME_LINE,"Test start");

    TEST (T, false, "true",  0, boolalpha, Fail);
     std_log(LOG_FILENAME_LINE,"Test end");
#ifndef	SAME_AS_UNIX_COMMENT 
     std_log(LOG_FILENAME_LINE,"Test start");

    TEST (T, false, "BOOLEAN", 7, boolalpha, Good);
     std_log(LOG_FILENAME_LINE,"Test end");
#endif     
     std_log(LOG_FILENAME_LINE,"Test start");

    TEST (T, true,  "BOOL",    4, boolalpha, Eof);
     std_log(LOG_FILENAME_LINE,"Test end");
     std_log(LOG_FILENAME_LINE,"Test start");

    TEST (T, true,  "BOOL ",   4, boolalpha, Good);
     std_log(LOG_FILENAME_LINE,"Test end");
#ifndef	SAME_AS_UNIX_COMMENT      
     std_log(LOG_FILENAME_LINE,"Test start");


    // the parsing algorithm is greedy, partial matches fail even
    // if one keyword is a subset of the other and the shorter one
    // is a subset of the characters matched so far
    TEST (T, false, "BOOLEA", 6, boolalpha, Eof | Fail);
     std_log(LOG_FILENAME_LINE,"Test end");
     std_log(LOG_FILENAME_LINE,"Test start");

    TEST (T, false, "BOOLE",  5, boolalpha, Eof | Fail);
     std_log(LOG_FILENAME_LINE,"Test end");
#endif


    {
        static const charT names[][8] = {
            { '1', '\0' },
            { '0', '\0' }
        };

        Punct<charT>::falsename_ = names [0];
        Punct<charT>::truename_  = names [1];
    }

     std_log(LOG_FILENAME_LINE,"Test start");

    TEST (T, false, "false", 0, boolalpha, Fail);
     std_log(LOG_FILENAME_LINE,"Test end");
     std_log(LOG_FILENAME_LINE,"Test start");

    TEST (T, false, "true",  0, boolalpha, Fail);
     std_log(LOG_FILENAME_LINE,"Test end");
#ifndef	SAME_AS_UNIX_COMMENT  
     std_log(LOG_FILENAME_LINE,"Test start");

    TEST (T, false, "1", 1, boolalpha, Good);
     std_log(LOG_FILENAME_LINE,"Test end");
     std_log(LOG_FILENAME_LINE,"Test start");

    TEST (T, true,  "0", 1, boolalpha, Good);
     std_log(LOG_FILENAME_LINE,"Test end");
#endif     
     std_log(LOG_FILENAME_LINE,"Test start");

    // numeric parsing with "0" and "1" as keywords
    TEST (T, false, "0", 1, 0, Eof);
     std_log(LOG_FILENAME_LINE,"Test end");
     std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, true,  "1", 1, 0, Eof);
     std_log(LOG_FILENAME_LINE,"Test end");
     std_log(LOG_FILENAME_LINE,"Test start");

    TEST (T, false, "11", 1, boolalpha, Good);
     std_log(LOG_FILENAME_LINE,"Test end");
     std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, true,  "00", 1, boolalpha, Good);
     std_log(LOG_FILENAME_LINE,"Test end");
     std_log(LOG_FILENAME_LINE,"Test start");

    TEST (T, false, "+1", 0, boolalpha, Fail);
     std_log(LOG_FILENAME_LINE,"Test end");
     std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, false, "+0", 0, boolalpha, Fail);
     std_log(LOG_FILENAME_LINE,"Test end");


#else   // if defined (NO_GET_BOOL)

    _RWSTD_UNUSED (t);

#endif   // NO_GET_BOOL

}

/**************************************************************************/
//#ifndef __SYMBIAN32__   // Bug -> 481

template <class charT>
void test_shrt (charT, const char *cname)
{
#ifndef _RWSTD_NO_EXT_NUM_GET
#  ifndef NO_GET_SHRT

    const char* const tname = "short";

    rw_info (0, 0, 0, "extension: std::num_get<%s>::get (..., %s&)",
             cname, tname);

    // short parsing of 0, no base specified
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, short (0), "%hi",  1, 0, Eof, "%hi");
     std_log(LOG_FILENAME_LINE,"Test end");     
     std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, short (0), "%ho",  1, 0, Eof, "%hi");
     std_log(LOG_FILENAME_LINE,"Test end");     
     std_log(LOG_FILENAME_LINE,"Test start");

    TEST (T, short (0), "%hx",  1, 0, Eof, "%hi");
     std_log(LOG_FILENAME_LINE,"Test end");     
     std_log(LOG_FILENAME_LINE,"Test start");

    TEST (T, short (0), "%#hx", 1, 0, Eof, "%hi");
     std_log(LOG_FILENAME_LINE,"Test end");     
     std_log(LOG_FILENAME_LINE,"Test start");

    TEST (T, short (1),        "%hi",  1, 0, Eof, "%hi");
     std_log(LOG_FILENAME_LINE,"Test end");     
     std_log(LOG_FILENAME_LINE,"Test start");

    TEST (T, short (SHRT_MAX), "%hi", -1, 0, Eof, "%hi");
     std_log(LOG_FILENAME_LINE,"Test end");     
     std_log(LOG_FILENAME_LINE,"Test start");

    TEST (T, short (SHRT_MIN), "%hi", -1, 0, Eof, "%hi");
     std_log(LOG_FILENAME_LINE,"Test end");     


    rw_info (0, 0, 0, "extension: base 1 (Roman)");

#define BASE(n)   ((n)  << _RWSTD_IOS_BASEOFF)
     std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, short ( 0), "0",     1, BASE (1), Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
#ifndef	SAME_AS_UNIX_COMMENT  
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, short ( 1), "i",     1, BASE (1), Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");

    TEST (T, short ( 2), "ii",    2, BASE (1), Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");

    TEST (T, short ( 3), "iii",   3, BASE (1), Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");

    TEST (T, short ( 4), "iiii",  4, BASE (1), Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");

    TEST (T, short ( 4), "iv",    2, BASE (1), Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");

    TEST (T, short ( 5), "v",     1, BASE (1), Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");

    TEST (T, short ( 6), "vi",    2, BASE (1), Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");

    TEST (T, short ( 7), "vii",   3, BASE (1), Eof);

    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, short ( 8), "viii",  4, BASE (1), Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");

    TEST (T, short ( 9), "viiii", 5, BASE (1), Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");

    TEST (T, short ( 9), "ix",    2, BASE (1), Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
   TEST (T, short (10), "x",     1, BASE (1), Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");

    TEST (T, short (  12), "XII",         3, BASE (1), Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");

    TEST (T, short ( 123), "CXXIII",      6, BASE (1), Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");

    TEST (T, short (1234), "MCCXXXIV",    8, BASE (1), Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");

    TEST (T, short (2345), "MMCCCXLV",    8, BASE (1), Eof); 
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, short (3456), "MMMCDLVI",    8, BASE (1), Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, short (4567), "MMMMDLXVII", 10, BASE (1), Eof); 
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, short (4999), "MMMMCMXCIX", 10, BASE (1), Eof); 
    std_log(LOG_FILENAME_LINE,"Test end");
#endif    
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, short (5000), "5000",        4, BASE (1), Eof); 
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, short (5678), "5678",        4, BASE (1), Eof); 
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");

    TEST (T, short (5001), "5001i",       4, BASE (1), Good); 
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, short (5002), "5002v",       4, BASE (1), Good); 
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, short (5003), "5003x",       4, BASE (1), Good); 
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, short (5004), "5004l",       4, BASE (1), Good); 
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, short (5005), "5005d",       4, BASE (1), Good); 
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, short (5006), "5006m",       4, BASE (1), Good); 
    std_log(LOG_FILENAME_LINE,"Test end");


    rw_info (0, 0, 0, "extension: base 2 (ios_base::bin, binary)");

    std_log(LOG_FILENAME_LINE,"Test start");

    TEST (T, short (  0), "0",      1, BASE (2), Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");

    TEST (T, short (  1), "1",      1, BASE (2), Eof);

    std_log(LOG_FILENAME_LINE,"Test end");
#ifndef	SAME_AS_UNIX_COMMENT      
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, short (  2), "10",     2, BASE (2), Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");

    TEST (T, short (  3), "11",     2, BASE (2), Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, short (  4), "100",    3, BASE (2), Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");

    TEST (T, short (  5), "101",    3, BASE (2), Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, short (  6), "110",    3, BASE (2), Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, short (  7), "111",    3, BASE (2), Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, short (  8), "1000",   4, BASE (2), Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, short (  9), "1001",   4, BASE (2), Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, short ( 10), "1010",   4, BASE (2), Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, short ( 11), "1011",   4, BASE (2), Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, short ( 12), "1100",   4, BASE (2), Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, short ( 13), "1101",   4, BASE (2), Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, short ( 14), "1110",   4, BASE (2), Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, short ( 15), "1111",   4, BASE (2), Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, short ( 16), "10000",  5, BASE (2), Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, short ( 17), "+10001", 6, BASE (2), Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, short (-18), "-10010", 6, BASE (2), Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");

    TEST (T, short (17), "100012",  5, BASE (2), Good);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, short (18), "100103",  5, BASE (2), Good);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, short (19), "100114",  5, BASE (2), Good);
    std_log(LOG_FILENAME_LINE,"Test end");
#endif    
    std_log(LOG_FILENAME_LINE,"Test start");

    rw_info (0, 0, 0, "extension: base 3");

    TEST (T, short ( 0), "0",     1, BASE (3), Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, short ( 1), "1",     1, BASE (3), Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, short ( 2), "2",     1, BASE (3), Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
#ifndef	SAME_AS_UNIX_COMMENT      
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, short ( 3), "10",    2, BASE (3), Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, short ( 4), "11",    2, BASE (3), Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, short ( 5), "12",    2, BASE (3), Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, short ( 6), "20",    2, BASE (3), Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, short ( 7), "21",    2, BASE (3), Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, short ( 8), "22",    2, BASE (3), Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");

    TEST (T, short ( 8), "223",   2, BASE (3), Good);
    std_log(LOG_FILENAME_LINE,"Test end");
#endif

    if (rw_opt_no_errno) {
        rw_note (0, 0, 0, "errno test disabled");
    }
    else {
        test_errno (charT (), short ());
    }

#  else   // if defined (NO_GET_SHRT)

    _RWSTD_UNUSED (t);

#  endif   // NO_GET_SHRT
#endif   // _RWSTD_NO_EXT_NUM_GET

}
//#endif
/**************************************************************************/
//#ifndef __SYMBIAN32__ // Bug 481

template <class charT>
void test_int (charT, const char *cname)
{
#ifndef _RWSTD_NO_EXT_NUM_GET
#  ifndef NO_GET_INT

    const char* const tname = "int";

    rw_info (0, 0, 0, "extension: std::num_get<%s>::get (..., %s&)",
             cname, tname);

    std_log(LOG_FILENAME_LINE,"Test start");

    TEST (T, 0, "%i",  1, 0, Eof, "%i");
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, 0, "%o",  1, 0, Eof, "%i");
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, 0, "%x",  1, 0, Eof, "%i");
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, 0, "%#x", 1, 0, Eof, "%i");
    std_log(LOG_FILENAME_LINE,"Test end");


    if (rw_opt_no_errno) {
        rw_note (0, 0, 0, "errno test disabled");
    }
    else {
        test_errno (charT (), int ());
    }

#  else   // if defined (NO_GET_INT)

    _RWSTD_UNUSED (t);

#  endif   // NO_GET_INT
#endif   // _RWSTD_NO_EXT_NUM_GET

}
//#endif //__SYMBIAN32__
/**************************************************************************/


template <class charT>
void test_long (charT, const char *cname)
{
#ifndef NO_GET_LONG

    const char* const tname = "long";

    rw_info (0, 0, 0, "std::num_get<%s>::get (..., %s&)",
             cname, tname);

    // 22.2.3.1, p2 -- integer grammar:
    // 
    // integer   ::= [sign] units
    // sign      ::= plusminus [whitespace]
    // plusminus ::= '+' | '-'
    // units     ::= digits [thousands-sep units]
    // digits    ::= digit [digits]
    // digit     ::=   '0' | '1' | '2' | '3' | '4' | '5' | '6' | '7'
    //               | '8' | '9' | 'A' | 'B' | 'C' | 'D' | 'E' | 'F'
    //               | 'a' | 'b' | 'c' | 'd' | 'e' | 'f'


    // long parsing of 0, no base specified

    std_log(LOG_FILENAME_LINE,"Test start");

    TEST (T, 0L, "%li",  1, 0, Eof, "%li");
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, 0L, "%lo",  1, 0, Eof, "%li");
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, 0L, "%lx",  1, 0, Eof, "%li");
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, 0L, "%#lx", 1, 0, Eof, "%li");
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");

    TEST (T, 0L, "",            0, 0, Eof | Fail);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, 0L, "+",           1, 0, Eof | Fail);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, 0L, "-",           1, 0, Eof | Fail);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, 0L, "0",           1, 0, Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, 0L, "00",          2, 0, Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, 0L, "+0",          2, 0, Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, 0L, "-0",          2, 0, Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, 0L, "0x0",         3, 0, Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, 0L, "0X0",         3, 0, Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, 0L, "+0x0",        4, 0, Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, 0L, "-0x0",        4, 0, Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, 0L, "0x0x",        3, 0, Good);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, 0L, "00000000",    8, 0, Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, 0L, "0x00000000", 10, 0, Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, 0L, "0-",          1, 0, Good);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, 0L, "0+",          1, 0, Good);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, 0L, "0.",          1, 0, Good);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    
    // long parsing of 0, base 10
    TEST (T, 0L, "0",          1, dec, Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, 0L, "00",         2, dec, Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, 0L, "+0",         2, dec, Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, 0L, "-0",         2, dec, Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, 0L, "0x0",        1, dec, Good);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, 0L, "+0x0",       2, dec, Good);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, 0L, "-0x0",       2, dec, Good);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, 0L, "00000000",   8, dec, Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, 0L, "0x00000000", 1, dec, Good);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, 0L, "0-",         1, dec, Good);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, 0L, "0+",         1, dec, Good);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, 0L, "0.",         1, dec, Good);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");

    // long parsing of 0, base 8
    TEST (T, 0L, "0",          1, oct, Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, 0L, "00",         2, oct, Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, 0L, "+0",         2, oct, Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, 0L, "-0",         2, oct, Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, 0L, "0x0",        1, oct, Good);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, 0L, "+0x0",       2, oct, Good);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, 0L, "-0x0",       2, oct, Good);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, 0L, "00000000",   8, oct, Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, 0L, "0x00000000", 1, oct, Good);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, 0L, "0-",         1, oct, Good);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, 0L, "0+",         1, oct, Good);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, 0L, "0.",         1, oct, Good);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");

    // long parsing of 0, base 16
    TEST (T, 0L, "0",           1, hex, Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");

    TEST (T, 0L, "00",          2, hex, Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, 0L, "+0",          2, hex, Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, 0L, "-0",          2, hex, Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, 0L, "0x0",         3, hex, Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, 0L, "0X00",        4, hex, Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, 0L, "+0x0",        4, hex, Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, 0L, "-0x0",        4, hex, Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, 0L, "00000000",    8, hex, Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, 0L, "0x00000000", 10, hex, Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, 0L, "0-",          1, hex, Good);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, 0L, "0+",          1, hex, Good);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, 0L, "0.",          1, hex, Good);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");

    // long parsing, no base specified
    //     assumes sizeof(long) >= 4

    _RWSTD_ASSERT (sizeof (long) >= 4);

    TEST (T,         1L,         "1",  1, 0, Eof, "%li");

    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T,        12L,        "12",  2, 0, Eof, "%li");
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T,       123L,       "123",  3, 0, Eof, "%li");
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T,      1234L,      "1234",  4, 0, Eof, "%li");
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T,     12345L,     "12345",  5, 0, Eof, "%li");
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T,    123456L,    "123456",  6, 0, Eof, "%li");
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T,   1234567L,   "1234567",  7, 0, Eof, "%li");
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T,  12345678L,  "12345678",  8, 0, Eof, "%li");
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, 134217728L, "134217728",  9, 0, Eof, "%li");
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, 134217728L, "%li",        9, 0, Eof, "%li");
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T,        ~0L, "%li",       -1, 0, Eof, "%li");
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T,        10L, "%li",       -1, 0, Eof, "%li");
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");

    // autodetection of oct and hex format
    TEST (T,   01234567L,   "01234567",  8, 0, Eof, "%li");
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T,  0x1234567L,  "0x1234567",  9, 0, Eof, "%li");
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, 0X79aBcDeFL, "0X79aBcDeF", 10, 0, Eof, "%li");
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");

    TEST (T,   01234567L,  "+01234567",  9, 0, Eof, "%li");
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T,  0x1234567L, "+0x1234567", 10, 0, Eof, "%li");
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T,  -01234567L,  "-01234567",  9, 0, Eof, "%li");
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, -0x1234567L, "-0x1234567", 10, 0, Eof, "%li");
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");

    TEST (T, 0xaL, "0x00000000000000a", 17, 0, Eof, "%li");
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, 0xabL, "0x000000000000ab", 16, 0, Eof, "%li");
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, 0xabcL, "0x0000000000abc", 15, 0, Eof, "%li");
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, 0xabcdL, "0x00000000abcd", 14, 0, Eof, "%li");
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, 0xabcdeL, "0x000000abcde", 13, 0, Eof, "%li");
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, 0xabcdefL, "0x0000abcdef", 12, 0, Eof, "%li");
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, 0xabcdef0L, "0x00abcdef0", 11, 0, Eof, "%li");
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, 0xabcdef1L,   "0xabcdef1",  9, 0, Eof, "%li");
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");

    TEST (T, 0L, "00x", 2, 0, Good, "%li");
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, 1L, "01x", 2, 0, Good, "%li");
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, 0L, "0x",  2, 0, Eof | Fail);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, 0L, "x",   0, 0, Fail, "%li");
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");

    // long parsing, dec base
    TEST (T,           2L,           "2",   1, dec, Eof,  "%ld");
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T,          23L,          "23",   2, dec, Eof,  "%ld");
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T,         234L,         "234",   3, dec, Eof,  "%ld");
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T,        2345L,        "2345",   4, dec, Eof,  "%ld");
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T,       23456L,       "23456",   5, dec, Eof,  "%ld");
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T,      234567L,      "234567",   6, dec, Eof,  "%ld");
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T,     2345678L,     "2345678",   7, dec, Eof,  "%ld");
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T,    23456789L,    "23456789",   8, dec, Eof,  "%ld");
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T,   134217728L,   "134217728",   9, dec, Eof,  "%ld");
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T,   134217728L,         "%ld",   9, dec, Eof,  "%ld");
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T,          ~0L,         "%ld",  -1, dec, Eof,  "%ld");
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T,     LONG_MAX,         "%ld",  -1, dec, Eof,  "%ld");
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T,     LONG_MIN,         "%ld",  -1, dec, Eof,  "%ld");
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T,          -3L,         "%ld",   2, dec, Eof,  "%ld");
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T,         -34L,         "%ld",   3, dec, Eof,  "%ld");
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T,        -345L,         "%ld",   4, dec, Eof,  "%ld");
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T,       -3456L,       "-3456",   5, dec, Eof,  "%ld");
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T,      -34567L,      "-34567",   6, dec, Eof,  "%ld");
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T,     -345678L,   "-00345678",   9, dec, Eof,  "%ld");
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T,     3456789L,   "+03456789",   9, dec, Eof,  "%ld");
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T,     4567890L,   "4567890.1",   7, dec, Good, "%ld");
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T,      567890L,   "567890e+1",   6, dec, Good, "%ld");

    Punct<charT>::thousands_sep_ = ';';

    // long parsing, dec base, with grouping (which is optional, empty
    // grouping, "", causes extraction to terminate at the first occurrence
    // of the thousands_sep character)
    TEST (T, 2147483647L,         "2147483647;", 10, dec, Good, "\0");
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T,  214748364L,         "214748364;7",  9, dec, Good, "\0");
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T,   21474836L,         "21474836;47",  8, dec, Good, "\0");
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T,    2147483L,         "2147483;647",  7, dec, Good, "\0");
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T,     214748L,         "214748;3647",  6, dec, Good, "\0");
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T,      21474L,         "21474;83647",  5, dec, Good, "\0");
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T,       2147L,         "2147;483647",  4, dec, Good, "\0");
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T,        214L,         "214;7483647",  3, dec, Good, "\0");
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T,         21L,         "21;47483647",  2, dec, Good, "\0");
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T,          2L,         "2;147483647",  1, dec, Good, "\0");
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T,          0L,         ";2147483647",  0, dec, Fail, "\0");
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");

    TEST (T, 2147483647L,          "2147483647", 10, dec, Eof, "\1");
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, 2147483647L, "2;1;4;7;4;8;3;6;4;7", 19, dec, Eof, "\1");
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, 2147483646L,      "21;47;48;36;46", 14, dec, Eof, "\2");
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, 2147483645L,       "2;147;483;645", 13, dec, Eof, "\3");
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, 2147483644L,        "21;4748;3644", 12, dec, Eof, "\4");
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, 2147483643L,         "21474;83643", 11, dec, Eof, "\5");
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, 2147483642L,         "2147;483642", 11, dec, Eof, "\6");
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, 2147483641L,         "214;7483641", 11, dec, Eof, "\7");
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, 2147483640L,         "21;47483640", 11, dec, Eof, "\10");
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, 2147483639L,         "2;147483639", 11, dec, Eof, "\11");
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, 2147483638L,          "2147483638", 10, dec, Eof, "\12");
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");

    TEST (T, 2147483637L, "2;14;74;83;63;7", 15, dec, Eof, "\1\2");
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, 2147483636L,   "214;748;363;6", 13, dec, Eof, "\1\3");
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, 2147483635L,   "2;1474;8363;5", 13, dec, Eof, "\1\4");
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, 2147483634L,    "2147;48363;4", 12, dec, Eof, "\1\5");
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, 2147483633L,    "214;748363;3", 12, dec, Eof, "\1\6");
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, 2147483632L,    "21;4748363;2", 12, dec, Eof, "\1\7");
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, 2147483631L,    "2;14748363;1", 12, dec, Eof, "\1\10");
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, 2147483630L,     "214748363;0", 11, dec, Eof, "\1\11");
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");

    TEST (T, 2147483629L,   "2;147;483;62;9", 14, dec, Eof, "\1\2\3");
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, 2147483628L, "2;1;4;7;4;83;628", 16, dec, Eof, "\3\2\1");
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, 2147483627L,    "2;147;483;627", 13, dec, Eof, "\3\3\3");
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, 2147483626L,    "+2147;483;626", 13, dec, Eof, "\3\3\4");
    std_log(LOG_FILENAME_LINE,"Test end");


    for (int i = 256; i >= 0; i -= 32) {

        // exercise parsing of input with excessively many groups
        static const char many_groups[] = {
            // each row below consists of 32 thousands_sep's (numbered 0-31)
            // and 32 zeros for a total of 64 characters
            //  . . . . : . . . . 1 . . . . : . . . . 2 . . . . : . . . . 3 .
            "0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;"
            "0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;"
            "0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;"
            "0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;"
            "0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;"
            "0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;"
            "0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;"
            "0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;"
            "1"
        };

        static const std::size_t NC = sizeof many_groups - 1;

        const int nfail = TEST (T, 1L, many_groups + i, NC - i, dec, Eof, "\1");

        // increase input length and break out after the first failure
        if (nfail)
            break;
    }

    // long parsing, hex base, with grouping
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");

    TEST (T,        0x0L, "0",        1, hex, Eof, "\1");
        std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
TEST (T,        0x0L, "00",       2, hex, Eof, "\1");
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T,        0x0L, "000",      3, hex, Eof, "\1");
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T,        0x0L, "0;0;0",    5, hex, Eof, "\1");
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T,        0x0L, "0x0;0;0",  7, hex, Eof, "\1");
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T,        0x0L, "0X0;0;0",  7, hex, Eof, "\1");
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T,        0x0L, "+0;0;0",   6, hex, Eof, "\1");
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T,        0x0L, "-0;0;0",   6, hex, Eof, "\1");
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T,        0x0L, "+0x0;0;0", 8, hex, Eof, "\1");
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T,        0x0L, "+0X0;0;0", 8, hex, Eof, "\1");
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T,        0x0L, "-0x0;0;0", 8, hex, Eof, "\1");
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T,        0x0L, "-0X0;0;0", 8, hex, Eof, "\1");
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");

    TEST (T,        0xAL, "A",               1, hex, Eof, "\1");
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T,       0xABL, "AB",              2, hex, Eof, "\1");
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T,       0xABL, "A;B",             3, hex, Eof, "\1");
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T,       0xABL, "A;b",             3, hex, Eof, "\1");
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T,       0xABL, "a;B",             3, hex, Eof, "\1");
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T,      0xABCL, "ABC",             3, hex, Eof, "\1");
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T,      0xABCL, "A;B;C",           5, hex, Eof, "\1");
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T,     0xABCDL, "ABCD",            4, hex, Eof, "\1");
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T,     0xABCDL, "A;B;C;D",         7, hex, Eof, "\1");
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T,    0xABCDEL, "ABCDE",           5, hex, Eof, "\1");
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T,    0xABCDEL, "A;B;C;D;E",       9, hex, Eof, "\1");
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T,   0xABCDEFL, "ABCDEF",          6, hex, Eof, "\1");
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T,   0xABCDEFL, "A;B;C;D;E;F",    11, hex, Eof, "\1");
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T,   0xABCDEFL, "a;B;c;D;e;F",    11, hex, Eof, "\1");
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T,   0xABCDEFL, "+A;B;C;D;E;F",   12, hex, Eof, "\1");
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T,  -0xABCDEFL, "-A;B;C;D;E;F",   12, hex, Eof, "\1");
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T,   0xABCDEFL, "0xA;B;C;D;E;F",  13, hex, Eof, "\1");
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T,   0xABCDEFL, "+0xA;B;C;D;E;F", 14, hex, Eof, "\1");
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T,  -0xABCDEFL, "-0xA;B;C;D;E;F", 14, hex, Eof, "\1");
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");

    // long parsing, dec base, with bad groups: store value, set failbit
    TEST (T, 2147483624L,   "21;474;83624; ", 13, dec, Fail, "\5\3");
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, 2147483623L,    "21;47;483623 ", 12, dec, Fail, "\6\3");
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, 2147483622L,     "21;47483622 ", 11, dec, Fail, "\7\3");
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, 2147483621L,    "214;748362;1 ", 12, dec, Fail, "\7\3");
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, 2147483619L,   "2;147;483;619 ", 13, dec, Fail, "\1");
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, 2147483618L,   "2147;48;36;18 ", 13, dec, Fail, "\2");
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, 2147483617L,    "2147;483;617 ", 12, dec, Fail, "\3");
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, 2147483616L,     "21;47483616 ", 11, dec, Fail, "\4");
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, 2147483615L,   "21;47;4836;15 ", 13, dec, Fail, "\2\2");
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, 2147483614L,    "21474;836;14 ", 12, dec, Fail, "\2\3");
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");

    // thousands_sep at illegal positions, do not store value, set failbit
    TEST (T, 2147483625L,   ";214;748;3625 ",  0, dec, Fail, "\4\3");
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, 2147483620L,   "+214;;7483620 ",  5, dec, Fail, "\7\4");
    std_log(LOG_FILENAME_LINE,"Test end");
    
    // unusual thousands_sep
    Punct<charT>::thousands_sep_ = '.';

    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, 2147483618L, "214.74836.18", 12, dec, Eof, "\2\5");
    std_log(LOG_FILENAME_LINE,"Test end");
    Punct<charT>::thousands_sep_ = 'A';
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, 2147483617L, "2A14A7483A617", 13, dec, Eof, "\3\4\2");
    std_log(LOG_FILENAME_LINE,"Test end");
    // Stage 2 parsing algorithm in 22.2.2.1.2, p8 removes thousands_sep
    // from the sequence before checking for sign and other valid chars
    // but since, according to the grammar, a well-formed thousands_sep
    // may not appear first, when the leading thousands_sep matches
    // the sign character it is taken as the sign
    // (these are totally anal-retentive tests)
    Punct<charT>::thousands_sep_ = '-';

    std_log(LOG_FILENAME_LINE,"Test start");

    TEST (T,  2147483617L, "21-47-4-83-617", 14, dec, Eof, "\3\2\1\2");
    TEST (T, -2147483616L, "-214-748-3616 ", 13, dec, Good, "\4\3\3");

    Punct<charT>::thousands_sep_ = '+';
    TEST (T, 2147483615L, "2+147+48+3+615", 14, dec, Eof, "\3\1\2\3");
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");

    // leading '+' taken as the plus sign and not a thousands_sep
    TEST (T, 2147483614L, "+21+4748361+4 ", 13, dec, Good, "\1\7\2");
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");

    // a digit as a thousands_sep is removed from the input sequence
    Punct<charT>::thousands_sep_ = '0';
    TEST (T, 2147483613L, "21047048306013", 14, dec, Eof, "\2\1\3\2");
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");

    Punct<charT>::thousands_sep_ = '1';
    TEST (T,   24748362L,     "2147483612", 10, dec, Eof, "\1\6");
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");

    // exercise lwg issue 338 (whitespace after sign not allowed)
    TEST (T, 0L, "+ 1", 1, 0, Fail);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, 0L, "- 2", 1, 0, Fail);
    std_log(LOG_FILENAME_LINE,"Test end");



    // exercise signed integer overflow
#if LONG_MAX == 0x7fffL
    const char lmin[]         = "-32768";
    const char lmin_minus_1[] = "-32769";
    const char lmin_minus_2[] = "-32770";
    const char lmin_minus_3[] = "-32771";

    const char lmax[]         = "32767";
    const char lmax_plus_1[]  = "32768";
    const char lmax_plus_2[]  = "32769";
    const char lmax_plus_3[]  = "32770";
    const char lmax_x_f[]     = "0x7ffff";

    const char vflow_1[]      = "";
#elif LONG_MAX == 0x7fffffffL
    const char lmin[]         = "-2147483648";
    const char lmin_minus_1[] = "-2147483649";
    const char lmin_minus_2[] = "-2147483650";
    const char lmin_minus_3[] = "-2147483651";

    const char lmax[]         = "2147483647";   // 0x7fffffffL
    const char lmax_plus_1[]  = "2147483648";   // 0x7fffffffL + 1
    const char lmax_plus_2[]  = "2147483649";   // 0x7fffffffL + 2
    const char lmax_plus_3[]  = "2147483650";   // 0x7fffffffL + 3
    const char lmax_x_f[]     = "0x7ffffffff";  // extra 'f' (times 0xf)
    const char vflow_1[]      = "";
#elif LONG_MAX > 0x7fffffffL
    const char lmin[]         = "-9223372036854775808";
    const char lmin_minus_1[] = "-9223372036854775809";
    const char lmin_minus_2[] = "-9223372036854775810";
    const char lmin_minus_3[] = "-9223372036854775811";

    const char lmax[]         = "9223372036854775807";
    const char lmax_plus_1[]  = "9223372036854775808";
    const char lmax_plus_2[]  = "9223372036854775809";
    const char lmax_plus_3[]  = "9223372036854775810";
    const char lmax_x_f[]     = "0x7ffffffffffffffff";
    const char vflow_1[]      = "";
#else
    // working around a SunPro bug (PR #28279)

    const char lmin[]         = "-999999999999999999990";
    const char lmin_minus_1[] = "-999999999999999999991";
    const char lmin_minus_2[] = "-999999999999999999992";
    const char lmin_minus_3[] = "-999999999999999999993";

    const char lmax[]         = "0";
    const char lmax_plus_1[]  = "999999999999999999997";
    const char lmax_plus_2[]  = "999999999999999999998";
    const char lmax_plus_3[]  = "999999999999999999999";
    const char lmax_x_f[]     = "0xfffffffffffffffff";
    const char vflow_1[]      = "999999999999999999990";
#endif

    std_log(LOG_FILENAME_LINE,"Test start");

    TEST (T, LONG_MIN, lmin,         sizeof lmin - 1,         0, Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, LONG_MIN, lmin_minus_1, sizeof lmin_minus_1 - 1, 0, Eof | Fail);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, LONG_MIN, lmin_minus_2, sizeof lmin_minus_2 - 1, 0, Eof | Fail);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, LONG_MIN, lmin_minus_3, sizeof lmin_minus_3 - 1, 0, Eof | Fail);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");

    TEST (T, LONG_MAX, lmax,         sizeof lmax - 1,         0, Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, LONG_MAX, lmax_plus_1,  sizeof lmax_plus_1 - 1,  0, Eof | Fail);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, LONG_MAX, lmax_plus_2,  sizeof lmax_plus_2 - 1,  0, Eof | Fail);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, LONG_MAX, lmax_plus_3,  sizeof lmax_plus_3 - 1,  0, Eof | Fail);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, LONG_MAX, lmax_x_f,     sizeof lmax_x_f - 1,     0, Eof | Fail);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");

    TEST (T, LONG_MAX, vflow_1,      sizeof vflow_1 - 1,      0, Eof | Fail);
    std_log(LOG_FILENAME_LINE,"Test end");
#ifndef	SAME_AS_UNIX_COMMENT
    std_log(LOG_FILENAME_LINE,"Test start");


    // verify that the combination of ERANGE and LONG_MAX
    // doesn't trip the facet up if it uses strtol()
    errno = ERANGE;
    TEST (T, _RWSTD_LONG_MAX, _RWSTD_STRSTR (_RWSTD_LONG_MAX), -1, 0, Good);
    std_log(LOG_FILENAME_LINE,"Test end");

#ifndef __SYMBIAN32__
    rw_assert (ERANGE == errno, 0, __LINE__,
               "errno unexpectedly changed from %d (%{#*m}) to %d (%{#m})",
               ERANGE, ERANGE, errno);
#else
 if(ERANGE != errno)
 {
   failures++;
   std_log(LOG_FILENAME_LINE,"Reason:Failing");
   }
  #endif //__SYMBIAN32__
#endif  
    rw_info (0, 0, 0, "extension: base 1 (Roman)");

#define BASE(n)   ((n)  << _RWSTD_IOS_BASEOFF)


    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T,    0L, "0",           1, BASE (1), Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
#ifndef	SAME_AS_UNIX_COMMENT    
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T,    1L, "i",           1, BASE (1), Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T,    2L, "ii",          2, BASE (1), Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T,    3L, "iii",         3, BASE (1), Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T,    4L, "iiii",        4, BASE (1), Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T,    4L, "iv",          2, BASE (1), Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T,    5L, "v",           1, BASE (1), Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T,    6L, "vi",          2, BASE (1), Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T,    7L, "vii",         3, BASE (1), Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T,    8L, "viii",        4, BASE (1), Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T,    9L, "viiii",       5, BASE (1), Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T,    9L, "ix",          2, BASE (1), Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T,   10L, "x",           1, BASE (1), Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");

    TEST (T,   12L, "XII",         3, BASE (1), Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T,  123L, "CXXIII",      6, BASE (1), Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, 1234L, "MCCXXXIV",    8, BASE (1), Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, 2345L, "MMCCCXLV",    8, BASE (1), Eof); 
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, 3456L, "MMMCDLVI",    8, BASE (1), Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, 4567L, "MMMMDLXVII", 10, BASE (1), Eof); 
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, 4999L, "MMMMCMXCIX", 10, BASE (1), Eof); 
    std_log(LOG_FILENAME_LINE,"Test end");
#endif    
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, 5000L, "5000",        4, BASE (1), Eof); 
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, 5678L, "5678",        4, BASE (1), Eof); 
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");

    TEST (T, 5001L, "5001i",       4, BASE (1), Good); 
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, 5002L, "5002v",       4, BASE (1), Good); 
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, 5003L, "5003x",       4, BASE (1), Good); 
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, 5004L, "5004l",       4, BASE (1), Good); 
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, 5005L, "5005d",       4, BASE (1), Good); 
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, 5006L, "5006m",       4, BASE (1), Good); 

    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    rw_info (0, 0, 0, "extension: base 2 (ios_base::bin, binary)");

    TEST (T,    0L, "0",           1, BASE (2), Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T,    1L, "1",           1, BASE (2), Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
#ifndef	SAME_AS_UNIX_COMMENT    
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T,    2L, "10",          2, BASE (2), Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T,    3L, "11",          2, BASE (2), Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T,    4L, "100",         3, BASE (2), Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T,    5L, "101",         3, BASE (2), Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T,    6L, "110",         3, BASE (2), Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T,    7L, "111",         3, BASE (2), Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T,    8L, "1000",        4, BASE (2), Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T,    9L, "1001",        4, BASE (2), Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T,   10L, "1010",        4, BASE (2), Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T,   11L, "1011",        4, BASE (2), Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T,   12L, "1100",        4, BASE (2), Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T,   13L, "1101",        4, BASE (2), Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T,   14L, "1110",        4, BASE (2), Eof);
    TEST (T,   15L, "1111",        4, BASE (2), Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T,   16L, "10000",       5, BASE (2), Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T,   17L, "+10001",      6, BASE (2), Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T,  -18L, "-10010",      6, BASE (2), Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");

    TEST (T,   17L, "100012",      5, BASE (2), Good);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T,   18L, "100103",      5, BASE (2), Good);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T,   19L, "100114",      5, BASE (2), Good);

    std_log(LOG_FILENAME_LINE,"Test end");
#endif    
    std_log(LOG_FILENAME_LINE,"Test start");
    rw_info (0, 0, 0, "extension: base 3");

    TEST (T,    0L, "0",           1, BASE (3), Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T,    1L, "1",           1, BASE (3), Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T,    2L, "2",           1, BASE (3), Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
#ifndef	SAME_AS_UNIX_COMMENT    
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T,    3L, "10",          2, BASE (3), Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T,    4L, "11",          2, BASE (3), Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T,    5L, "12",          2, BASE (3), Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T,    6L, "20",          2, BASE (3), Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T,    7L, "21",          2, BASE (3), Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T,    8L, "22",          2, BASE (3), Eof);

    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T,    8L, "223",         2, BASE (3), Good);

    std_log(LOG_FILENAME_LINE,"Test end");
#endif    
    std_log(LOG_FILENAME_LINE,"Test start");
    rw_info (0, 0, 0, "extension: base 4");

    TEST (T,    0L, "0",           1, BASE (4), Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T,    1L, "1",           1, BASE (4), Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T,    2L, "2",           1, BASE (4), Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T,    3L, "3",           1, BASE (4), Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
#ifndef	SAME_AS_UNIX_COMMENT    
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T,    4L, "10",          2, BASE (4), Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T,    5L, "11",          2, BASE (4), Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T,    6L, "12",          2, BASE (4), Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T,    7L, "13",          2, BASE (4), Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T,    8L, "20",          2, BASE (4), Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T,    9L, "21",          2, BASE (4), Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T,   10L, "22",          2, BASE (4), Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T,   11L, "23",          2, BASE (4), Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");

    TEST (T,   11L, "234",         2, BASE (4), Good);
    std_log(LOG_FILENAME_LINE,"Test end");
#endif    
    std_log(LOG_FILENAME_LINE,"Test start");

    rw_info (0, 0, 0, "extension: base 5");

    TEST (T,    0L, "0",           1, BASE (5), Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T,    1L, "1",           1, BASE (5), Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T,    2L, "2",           1, BASE (5), Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T,    3L, "3",           1, BASE (5), Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T,    4L, "4",           1, BASE (5), Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
#ifndef	SAME_AS_UNIX_COMMENT
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T,    5L, "10",          2, BASE (5), Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T,    6L, "11",          2, BASE (5), Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T,    7L, "12",          2, BASE (5), Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T,    8L, "13",          2, BASE (5), Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T,    9L, "14",          2, BASE (5), Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T,   10L, "20",          2, BASE (5), Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T,   11L, "21",          2, BASE (5), Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T,   12L, "22",          2, BASE (5), Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T,   13L, "23",          2, BASE (5), Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T,   14L, "24",          2, BASE (5), Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");

    TEST (T,   14L, "245",         2, BASE (5), Good);
    std_log(LOG_FILENAME_LINE,"Test end");
#endif

    if (rw_opt_no_errno) {
        rw_note (0, 0, 0, "errno test disabled");
    }
    else {
        test_errno (charT (), long ());
    }

#endif   // NO_GET_LONG

}

/**************************************************************************/


template <class charT>
void test_ulong (charT, const char *cname)
{
#ifndef NO_GET_ULONG

    const char* const tname = "unsigned long";

    rw_info (0, 0, 0, "std::num_get<%s>::get (..., %s&)",
             cname, tname);

    // negative numbers are conventionally parsed and interpreted
    // according to the following algorithm:

    //   1.  extract sign S
    //   2.  extract unsigned number N
    //   3.  if (S == '-' && N <= ULONG_MAX) X = ULONG_MAX - N + 1
    //   4.  if (S != '-' && N <= ULONG_MAX) X = N
    //   5.  if (N > ULONG_MAX) X = ULONG_MAX, set failbit
    //   6.  return X

    std_log(LOG_FILENAME_LINE,"Test start");

    TEST (T,       0UL,   "0",  1, 0, Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T,       0UL,   "+0", 2, 0, Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T,       0UL,   "-0", 2, 0, Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T,       1UL,   "+1", 2, 0, Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T,    1078UL, "1078", 4, 0, Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, ULONG_MAX,   "-1", 2, 0, Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");

    TEST (T,             65535UL,       "65535",  5, 0, Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, ULONG_MAX - 65535UL + 1UL, "-65535", 6, 0, Eof);
    std_log(LOG_FILENAME_LINE,"Test end");


    // exercise integer overflow
#if ULONG_MAX == 0xffffUL
    const char ulmax[]        = "65535";
    const char ulmax_plus_1[] = "65536";
    const char ulmax_plus_2[] = "65537";
    const char ulmax_plus_3[] = "65538";
    const char ulmax_x_f[]    = "0xfffff";
    const char vflow_1[]      = "131061";
#elif ULONG_MAX == 0xffffffffUL
    const char ulmax[]        = "4294967295";   // 0xffffffffUL
    const char ulmax_plus_1[] = "4294967296";   // 0xffffffffUL + 1
    const char ulmax_plus_2[] = "4294967297";   // 0xffffffffUL + 2
    const char ulmax_plus_3[] = "4294967298";   // 0xffffffffUL + 3
    const char ulmax_x_f[]    = "0xfffffffff";  // extra 'f' (times 0xf)
    const char vflow_1[]      = "8589934581";
#elif ULONG_MAX > 0xffffffffUL
    const char ulmax[]        = "18446744073709551615";
    const char ulmax_plus_1[] = "18446744073709551616";
    const char ulmax_plus_2[] = "18446744073709551617";
    const char ulmax_plus_3[] = "18446744073709551618";
    const char ulmax_x_f[]    = "0xfffffffffffffffff";
    const char vflow_1[]      = "36893488147419103221";
#else
    // working around a SunPro bug (PR #28279)
    const char ulmax[]        = "0";
    const char ulmax_plus_1[] = "999999999999999999997";
    const char ulmax_plus_2[] = "999999999999999999998";
    const char ulmax_plus_3[] = "999999999999999999999";
    const char ulmax_x_f[]    = "0xfffffffffffffffff";
    const char vflow_1[]      = "999999999999999999990";
#endif
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, ULONG_MAX, ulmax,        sizeof ulmax - 1,        0, Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, ULONG_MAX, ulmax_plus_1, sizeof ulmax_plus_1 - 1, 0, Eof | Fail);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, ULONG_MAX, ulmax_plus_2, sizeof ulmax_plus_2 - 1, 0, Eof | Fail);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, ULONG_MAX, ulmax_plus_3, sizeof ulmax_plus_3 - 1, 0, Eof | Fail);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, ULONG_MAX, ulmax_x_f,    sizeof ulmax_x_f - 1,    0, Eof | Fail);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");

    TEST (T, ULONG_MAX, vflow_1,      sizeof vflow_1 - 1,      0, Eof | Fail);
    std_log(LOG_FILENAME_LINE,"Test end");

#ifndef	SAME_AS_UNIX_COMMENT 
    // verify that the combination of ERANGE and ULONG_MAX
    // doesn't trip the facet up if it uses strtoul()
    errno = ERANGE;
    TEST (T, ULONG_MAX, _RWSTD_STRSTR (_RWSTD_ULONG_MAX), -1, 0, Good);
#ifndef __SYMBIAN32__
    rw_assert (ERANGE == errno, 0, __LINE__,
               "errno unexpectedly changed from %d (%{#*m}) to %d (%{#m})",
               ERANGE, ERANGE, errno);
#else
  if(ERANGE != errno)
  {
   failures++;
   std_log(LOG_FILENAME_LINE,"Reason:Failing");
   }
 #endif//__SYMBIAN32__
   
 #endif

    if (rw_opt_no_errno) {
        rw_note (0, 0, 0, "errno test disabled");
    }
    else {
        test_errno (charT (), (unsigned long)0);
    }

#endif   // NO_GET_ULONG
}

/**************************************************************************/

template <class charT>
void test_llong (charT, const char *cname)
{
#ifndef NO_GET_LLONG

    const char* const tname = "long long";

    rw_info (0, 0, 0, "extension: std::num_get<%s>::get (..., %s&)",
             cname, tname);

#  ifndef _MSC_VER
#    define LL(number)   number ## LL
#  else   // if defined (_MSC_VER)
     // MSVC 7.0 doesn't recognize the LL suffix
#    define LL(number)   number ## I64
#  endif   // _MSC_VER

    std_log(LOG_FILENAME_LINE,"Test start");

    TEST (T, LL (                   0),                    "0",  1, 0, Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, LL (                   0),                   "+0",  2, 0, Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");

    TEST (T, LL (                   0),                   "-0",  2, 0, Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, LL (                   1),                   "+1",  2, 0, Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, LL (                  -1),                   "-1",  2, 0, Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, LL (                1079),                 "1079",  4, 0, Eof);

    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    // LLONG_MAX for a 64-bit long long
    TEST (T, LL (+9223372036854775807), "+9223372036854775807", 20, 0, Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");

    // compute LLONG_MIN for a 64-bit long long using integer arithmetic
    // to avoid warnings due to 9223372036854775808 being too large for
    // long long
    // e.g., gcc 4.0 issues the following:
    // warning: integer constant is so large that it is unsigned
    // warning: this decimal constant is unsigned only in ISO C90
    TEST (T, LL (-9223372036854775807) - 1, "-9223372036854775808", 20, 0, Eof);
    std_log(LOG_FILENAME_LINE,"Test end");


    if (rw_opt_no_errno) {
        rw_note (0, 0, 0, "errno test disabled");
    }
    else {
        test_errno (charT (), (long long)0);
    }

#endif   // NO_GET_LLONG

}

/**************************************************************************/

template <class charT>
void test_ullong (charT, const char *cname)
{
#ifndef NO_GET_ULLONG

    const char* const tname = "unsigned long long";

    rw_info (0, 0, 0, "extension: std::num_get<%s>::get (..., %s&)",
             cname, tname);

#  ifndef _MSC_VER
#    define ULL(number)   number ## ULL
#  else   // if defined (_MSC_VER)
     // MSVC 7.0 doesn't recognize the LL suffix
#    define LL(number)   number ## UI64
#  endif   // _MSC_VER

    std_log(LOG_FILENAME_LINE,"Test start");

    TEST (T, ULL (                  0),                     "0",  1, 0, Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, ULL (                  0),                    "+0",  2, 0, Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, ULL (                  0),                    "-0",  2, 0, Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, ULL (                  1),                    "+1",  2, 0, Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, ULL (                1080),                 "1080",  4, 0, Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, ULL (18446744073709551615), "18446744073709551615", 20, 0, Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    

    if (rw_opt_no_errno) {
        rw_note (0, 0, 0, "errno test disabled");
    }
    else {
        test_errno (charT (), (unsigned long long)0);
    }

#endif   // NO_GET_ULLONG

}

/**************************************************************************/

template <class charT>
void test_pvoid (charT, const char *cname)
{
#ifndef NO_GET_PVOID

    const char* const tname = "void*";

    rw_info (0, 0, 0, "std::num_get<%s>::get (..., %s&)",
             cname, tname);

    typedef void* PVoid;

    std_log(LOG_FILENAME_LINE,"Test start");

    TEST (T, PVoid (         0),    "0", 1, 0, Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, PVoid (         1),    "1", 1, 0, Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, PVoid (         2),    "2", 1, 0, Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, PVoid (         3),   "03", 2, 0, Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, PVoid (0x000000ff),   "ff", 2, 0, Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, PVoid (0x00000fff),  "FFF", 3, 0, Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, PVoid (0x0000ffff), "FfFf", 4, 0, Eof);
    std_log(LOG_FILENAME_LINE,"Test end");


    // exercise overflow conditions
#if ULONG_MAX == 0xffffUL
    const char pvmax[]        = "0xffff";
    const char pvmax_plus_1[] = "0x10000";
#elif ULONG_MAX == 0xffffffffUL
    const char pvmax[]        = "0xffffffff";
    const char pvmax_plus_1[] = "0x100000000";
#elif ULONG_MAX > 0xffffffffUL
    const char pvmax[]        = "0xffffffffffffffff";
    const char pvmax_plus_1[] = "0x10000000000000000";
#else
    // working around a SunPro bug (PR #28279)
    const char pvmax[]        = "0";
    const char pvmax_plus_1[] = "0xffffffffffffffffffffffffffffffff";
#endif 

#define PVOIDSTR(name)   name, sizeof name - 1

    std_log(LOG_FILENAME_LINE,"Test start");

    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, PVoid (~0), PVOIDSTR (pvmax),        0, Eof);
    TEST (T, PVoid (~0), PVOIDSTR (pvmax_plus_1), 0, Eof | Fail);
    std_log(LOG_FILENAME_LINE,"Test end");


#undef PVOIDSTR

    // exercise error conditions
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, PVoid (0),  "", 0, 0, Eof | Fail);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, PVoid (0), "-", 1, 0, Eof | Fail);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, PVoid (0), "+", 1, 0, Eof | Fail);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, PVoid (0), "x", 0, 0, Fail);
    std_log(LOG_FILENAME_LINE,"Test end");


#endif   // NO_GET_PVOID
}

/**************************************************************************/


template <class charT, class floatT>
void test_floating_point (charT, floatT, bool test_locale)
{
    const char* const cname = rw_any_t (charT ()).type_name ();
    const char* const tname = rw_any_t (floatT ()).type_name ();

    Punct<charT>::thousands_sep_ = ';';

    typedef floatT F;

    std_log(LOG_FILENAME_LINE,"Test start");

    TEST (T, F (0.0),  "0",     1, 0, Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, F (0.0),  "0.",    2, 0, Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, F (0.0),  ".0",    2, 0, Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, F (0.0),  "0.0",   3, 0, Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, F (1.0),  "1",     1, 0, Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, F (2.0),  "2",     1, 0, Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, F (3.0),  "3",     1, 0, Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, F (4.0),  "4 ",    1, 0, Good);   // trailing ' ' not extracted
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, F (5.0),  "5.",    2, 0, Eof);    // trailing '.' extracted
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, F (5.0),  "5.0",   3, 0, Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, F (5.1),  "5.1",   3, 0, Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, F (5.2),  "5.2 ",  3, 0, Good);   // trailing ' ' not extracted
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, F (5.3),  "5.3;",  3, 0, Good);   // trailing ';' not extracted
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, F (5.4),  "5.4; ", 3, 0, Good);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, F (5.41), "5.41",  4, 0, Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, F (5.42), "5.42.", 4, 0, Good);   // trailing '.' not extracted
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, F (5.43), "5.43+", 4, 0, Good);   // trailing '+' not extracted
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, F (5.44), "5.44-", 4, 0, Good);   // trailing '-' not extracted
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");

    TEST (T, F (1.0e+00), "1",                               1, 0, Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, F (1.0e+01), "10",                              2, 0, Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, F (1.0e+02), "100",                             3, 0, Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, F (1.0e+03), "1000",                            4, 0, Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, F (1.0e+04), "10000",                           5, 0, Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, F (1.0e+05), "100000",                          6, 0, Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, F (1.0e+06), "1000000",                         7, 0, Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, F (1.0e+07), "10000000",                        8, 0, Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, F (1.0e+08), "100000000",                       9, 0, Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, F (1.0e+09), "1000000000",                     10, 0, Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, F (1.0e+10), "10000000000",                    11, 0, Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, F (1.0e+11), "100000000000",                   12, 0, Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, F (1.0e+12), "1000000000000",                  13, 0, Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, F (1.0e+13), "10000000000000",                 14, 0, Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, F (1.0e+14), "100000000000000",                15, 0, Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, F (1.0e+15), "1000000000000000",               16, 0, Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, F (1.0e+16), "10000000000000000",              17, 0, Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, F (1.0e+17), "100000000000000000",             18, 0, Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, F (1.0e+18), "1000000000000000000",            19, 0, Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, F (1.0e+19), "10000000000000000000",           20, 0, Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, F (1.0e+20), "100000000000000000000",          21, 0, Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, F (1.0e+21), "1000000000000000000000",         22, 0, Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, F (1.0e+22), "10000000000000000000000",        23, 0, Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, F (1.0e+23), "100000000000000000000000",       24, 0, Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, F (1.0e+24), "1000000000000000000000000",      25, 0, Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, F (1.0e+25), "10000000000000000000000000",     26, 0, Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, F (1.0e+26), "100000000000000000000000000",    27, 0, Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, F (1.0e+27), "1000000000000000000000000000",   28, 0, Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, F (1.0e+28), "10000000000000000000000000000",  29, 0, Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, F (1.0e+29), "100000000000000000000000000000", 30, 0, Eof);

    std_log(LOG_FILENAME_LINE,"Test end");

#ifdef __SYMBIAN32__
 #define VALSTR(x)   floatT (x), #x, strlen (#x)
#else 
#define VALSTR(x)   floatT (x), #x, std::strlen (#x)
#endif //__SYMBIAN32__

    // exercise various forms of floating point 0

    std_log(LOG_FILENAME_LINE,"Test start");

    TEST (T, VALSTR ( 0.),      0, Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, VALSTR (+0.),      0, Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, VALSTR (-0.),      0, Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");

    TEST (T, VALSTR ( .0),      0, Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, VALSTR (+.0),      0, Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, VALSTR (-.0),      0, Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");

    TEST (T, VALSTR ( 0e0),     0, Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, VALSTR (+0e0),     0, Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, VALSTR (-0e0),     0, Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");

    TEST (T, VALSTR ( 0E+0),    0, Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, VALSTR (+0E-0),    0, Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, VALSTR (-0E+0),    0, Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");

    TEST (T, VALSTR ( 0.0e+0),  0, Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, VALSTR (+0.0e+0),  0, Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, VALSTR (-0.0e+0),  0, Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");

    TEST (T, VALSTR ( 0.0E+0),  0, Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, VALSTR (+0.0E+0),  0, Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, VALSTR (-0.0E+0),  0, Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");

    // exercise 'e' immediately following decimal_point
    TEST (T, VALSTR ( 0.e+0),   0, Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, VALSTR (+0.e+0),   0, Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, VALSTR (-0.e+0),   0, Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");

    // same as above but with an 'E'
    TEST (T, VALSTR ( 0.E+0),   0, Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, VALSTR (+0.E+0),   0, Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, VALSTR (-0.E+0),   0, Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");

    // exercise 0 with a non-zero exponent
    TEST (T, VALSTR ( 0.0e+1),  0, Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, VALSTR ( 0.0e2),   0, Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, VALSTR (+0.0e+3),  0, Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, VALSTR (+0.0e4),   0, Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, VALSTR (-0.0e+5),  0, Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, VALSTR (-0.0e+6),  0, Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");

    TEST (T, VALSTR ( 0.0E+7),  0, Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, VALSTR ( 0.0E8),   0, Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, VALSTR (+0.0E+9),  0, Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, VALSTR (+0.0E10),  0, Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, VALSTR (-0.0E+11), 0, Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, VALSTR (-0.0E+12), 0, Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");

    // exercise 0 with a negative exponent
    TEST (T, VALSTR ( 0.0e-1),  0, Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, VALSTR (+0.0e-2),  0, Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, VALSTR (-0.0e-3),  0, Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");

    // exercise formats with no digits before decimal_point
    TEST (T, VALSTR ( .0e1),    0, Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, VALSTR ( .0e+2),   0, Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, VALSTR (+.0e+3),   0, Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, VALSTR (-.0e+4),   0, Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");

    TEST (T, VALSTR (1.0e+0),   0, Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, VALSTR (1.0e+2),   0, Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, VALSTR (1.2e+3),   0, Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, VALSTR (1.3e-4),   0, Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, VALSTR (1.4e5),    0, Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, VALSTR (1.e+6),    0, Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, VALSTR (1.e-7),    0, Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, VALSTR (1e+8),     0, Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, VALSTR (1e-9),     0, Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, VALSTR (1e10),     0, Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");

    TEST (T, VALSTR (+2.0e+0),  0, Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, VALSTR (+2.0e+2),  0, Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, VALSTR (+2.2e+3),  0, Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, VALSTR (+2.3e-4),  0, Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, VALSTR (+2.4e5),   0, Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, VALSTR (+2.e+6),   0, Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, VALSTR (+2.e-7),   0, Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, VALSTR (+2e+8),    0, Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, VALSTR (+2e-9),    0, Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, VALSTR (+2e10),    0, Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");

    TEST (T, VALSTR (-3.0e+0),  0, Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, VALSTR (-3.0e+2),  0, Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, VALSTR (-3.2e+3),  0, Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, VALSTR (-3.3e-4),  0, Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, VALSTR (-3.4e5),   0, Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, VALSTR (-3.e+6),   0, Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, VALSTR (-3.e-7),   0, Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, VALSTR (-3e+8),    0, Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, VALSTR (-3e-9),    0, Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, VALSTR (-3e10),    0, Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");

    // test with grouping (thousands_sep are optional in that they either
    // may or may not appear; if one appears, correct grouping is enforced)
    TEST (T, F (1.0e+29), "100000000000000000000000000000", 30, 0, Eof, "\1");

    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    // empty grouping, the first thousands_sep terminates parsing
    TEST (T, F (1.0),
          "1;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0",
          1, 0, Good, "\0");
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");

    TEST (T, F (1.0e+29),
          "1;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0",
          59, 0, Eof, "\1");
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");

    // lwg issue 358: thousands_sep is only interpreted as such and extracted
    // in the integral part of a floating point number; it is considered as
    // an invalid character that terminates input once the decimal point has
    // been encountered
    //                    0....:....1....
    TEST (T, F (123456.789), "123;456.789;012", 11, 0, Good, "\3");
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, F (123456.780), "123;456.78;9012", 10, 0, Good, "\3");
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, F (123456.700), "123;456.7;89012",  9, 0, Good, "\3");
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, F (123456.000), "123;456.;789012",  8, 0, Good, "\3");
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, F (123456e+1),  "123;456e+1;2345", 10, 0, Good, "\3");
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, F (123456e+2),  "123;456e2;34567",  9, 0, Good, "\3");
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, F (123456e+3),  "123456e3;456789",  8, 0, Good, "\3");
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, F (123456.e+2), "123;456.e+2;345", 11, 0, Good, "\3");
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, F (123456.e+3), "123;456.e3;4567", 10, 0, Good, "\3");
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, F (123456.e+4), "123456.e4;56789",  9, 0, Good, "\3");
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");

    // exercise behavior on invalid sequences disallowed by the grammar
    TEST (T, F (0.0), "",      0, 0, Eof | Fail);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, F (0.0), ".",     1, 0, Eof | Fail);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, F (0.0), "..",    1, 0, Fail);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, F (0.0), ".+",    1, 0, Fail);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, F (0.0), ".-",    1, 0, Fail);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, F (0.0), ".e",    1, 0, Fail);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, F (0.0), ".E",    1, 0, Fail);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");

    TEST (T, F (0.0), "+",     1, 0, Eof | Fail);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, F (0.0), "+.",    2, 0, Eof | Fail);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, F (0.0), "++",    1, 0, Fail);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, F (0.0), "+-",    1, 0, Fail);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, F (0.0), "+e",    1, 0, Fail);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, F (0.0), "+E",    1, 0, Fail);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");

    TEST (T, F (0.0), "-",     1, 0, Eof | Fail);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, F (0.0), "-.",    2, 0, Eof | Fail);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, F (0.0), "-+",    1, 0, Fail);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, F (0.0), "--",    1, 0, Fail);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, F (0.0), "-e",    1, 0, Fail);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, F (0.0), "-E",    1, 0, Fail);

    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, F (0.0), ".;",    1, 0, Fail);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, F (0.0), ";.;",   0, 0, Fail);

    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    // exercise lwg issue 338 (whitespace after sign not allowed)
    TEST (T, F (0.0), "+ 1",     1, 0, Fail);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, F (0.0), "- 2",     1, 0, Fail);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, F (0.0), "3.0e+ 1", 5, 0, Fail);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, F (0.0), "4.0e- 2", 5, 0, Fail);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");

    // exercise the ability to parse input in locales where thousands_sep
    // is the plus or minus character (e.g., on AIX, the Romanian locale,
    // "RO_RO" -- see bug #609)

    Punct<charT>::thousands_sep_ = '+';

    TEST (T, F ( 123e+4),  "123e+4", 6, 0, Eof, "\1");
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, F ( 124e+5),  "124e+5", 6, 0, Eof, "\1");
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, F (+125e+6), "+125e+6", 7, 0, Eof, "\1");
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, F (-126e+7), "-126e+7", 7, 0, Eof, "\1");
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, F (-127e-8), "-127e-8", 7, 0, Eof, "\1");

    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    Punct<charT>::thousands_sep_ = '-';

    TEST (T, F ( 123e-4),  "123e-4", 6, 0, Eof, "\1");
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, F ( 124e-5),  "124e-5", 6, 0, Eof, "\1");
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, F (+125e-6), "+125e-6", 7, 0, Eof, "\1");
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, F (+126e-7), "+126e-7", 7, 0, Eof, "\1");
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, F (+127e+8), "+127e+8", 7, 0, Eof, "\1");
    std_log(LOG_FILENAME_LINE,"Test end");


    // restore to a saner value
    Punct<charT>::thousands_sep_ = ';';

    if (test_locale) {
        // verify that the global LC_NUMERIC locale setting
        // has no impact on the facet (in case it uses scanf())
    #ifdef __SYMBIAN32__
#if 0    //what is this test?
            for (const char *name = rw_locales (LC_NUMERIC, 0); *name;
             name += strlen (name) + 1) {
                   // find the first locale whose decimal_point character
            // is different than in the classic C locale (i.e., than '.')
            if (0 == setlocale (LC_NUMERIC, name))
                continue;

            const lconv* const conv = localeconv ();

            if (!conv)
                continue;

            if (conv->decimal_point && '.' != *conv->decimal_point) {
                rw_info (0, 0, 0, 
                         "num_get<%s>::get(..., %s&) when LC_NUMERIC=\"%s\"",
                         cname, tname, name);
                break;
            }
        }
#endif //#if 0    
      #else  
       
        for (const char *name = rw_locales (LC_NUMERIC, 0); *name;
             name += std::strlen (name) + 1) {
                   // find the first locale whose decimal_point character
            // is different than in the classic C locale (i.e., than '.')
            if (0 == std::setlocale (LC_NUMERIC, name))
                continue;

            const std::lconv* const conv = std::localeconv ();

            if (!conv)
                continue;

            if (conv->decimal_point && '.' != *conv->decimal_point) {
                rw_info (0, 0, 0, 
                         "num_get<%s>::get(..., %s&) when LC_NUMERIC=\"%s\"",
                         cname, tname, name);
                break;
            }
        }

    #endif

    std_log(LOG_FILENAME_LINE,"Test start");
    
        TEST (T, F ( 1.2e+34),  "1.2e+34", 7, 0, Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
        TEST (T, F (+1.3e+33), "+1.3e+33", 8, 0, Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
        TEST (T, F (-1.4e+32), "-1.4e+32", 8, 0, Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
        TEST (T, F ( 0.5e+31), ".5e+31",   6, 0, Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
        TEST (T, F (+0.6e+30), "+.6e+30",  7, 0, Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
        TEST (T, F (-0.7e+29), "-.7e+29",  7, 0, Eof);
    std_log(LOG_FILENAME_LINE,"Test end");


        // reset the global locale
     #ifdef __SYMBIAN32__
      setlocale (LC_NUMERIC, "C");
     #else 
        std::setlocale (LC_NUMERIC, "C");
     #endif 
    }
    else
        rw_warn (0, 0, 0,
                 "num_get<%s>::get (..., %s&) known to fail when LC_NUMERIC "
                 "is set to a locale where decimal_point != '.'",
                 cname, tname);

    //////////////////////////////////////////////////////////////////
    // exercise errno
    if (rw_opt_no_errno) {
        rw_note (0, 0, 0, "errno test disabled");
    }
    else {
        test_errno (charT (), floatT ());
    }

}


/**************************************************************************/

template <class charT>
void test_flt_uflow (charT, const char *cname)
{
#ifndef NO_GET_FLT

    rw_info (0, 0, 0,
             "std::num_get<%s>::get (..., float&) on underflow", cname);

    // exercise bahvior on underflow
    float val = 0;
    pmax = &val;

    // on underflow, get() follows C99 requirements on strtof()
    // i.e., it stores a value in the range [0, +/-FLT_MIN]
    // the function does NOT set failbit on underflow (regardless
    // of whether strtof() sets errno)
    //     If the result underflows (7.12.1), the functions return
    //     a value whose magnitude is no greater than the smallest
    //     normalized positive number in the return type; whether
    //     errno acquires the value ERANGE is implementation-defined.
#ifndef	SAME_AS_UNIX_COMMENT 
    TEST (T, (val = FLT_MIN, 0.0f), "1.111111e-9999", 14, 0, Eof);
    #ifndef __SYMBIAN32__
    rw_assert (!(val < 0.0), 0, __LINE__,
               "correct sign after positive underflow");
    #else
     if((val < 0.0))
     {
      failures++;
      std_log(LOG_FILENAME_LINE,"Reason:Failing");
      }
    #endif//__SYMBIAN32__
    TEST (T, (val = 0.0f, -FLT_MIN), "-1.111111e-9999", 15, 0, Eof);
#endif    
    #ifndef __SYMBIAN32__
    rw_assert (!(val > 0.0), 0, __LINE__,
               "correct sign after negative underflow");
    #else
     if((val > 0.0))
     {
      failures++;
      std_log(LOG_FILENAME_LINE,"Reason:Failing");
      }
    #endif //__SYMBIAN32__
    if (1.234567e-39 < FLT_MIN) {

    std_log(LOG_FILENAME_LINE,"Test start");
 TEST (T, (val = FLT_MIN, 0.0f),   "1.234567e-39", 12, 0, Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
#ifndef	FLT_MIN_NOT_SAME 
    std_log(LOG_FILENAME_LINE,"Test start");
        TEST (T, (val = 0.0f, -FLT_MIN), "-1.234567e-39", 13, 0, Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
#endif    
    }

    if (1.234567e-49 < FLT_MIN) {

    std_log(LOG_FILENAME_LINE,"Test start");
        TEST (T, (val = FLT_MIN, 0.0f),   "1.234567e-49", 12, 0, Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
#ifndef	SAME_AS_UNIX_COMMENT 
    std_log(LOG_FILENAME_LINE,"Test start");
        TEST (T, (val = 0.0f, -FLT_MIN), "-1.234567e-49", 13, 0, Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
#endif    

    }

    if (1.234567e-99 < FLT_MIN) {

    std_log(LOG_FILENAME_LINE,"Test start");
        TEST (T, (val = FLT_MIN, 0.0f),   "1.234567e-99", 12, 0, Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
#ifndef	SAME_AS_UNIX_COMMENT     
    std_log(LOG_FILENAME_LINE,"Test start");
        TEST (T, (val = 0.0f, -FLT_MIN), "-1.234567e-99", 13, 0, Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
#endif    

    }

    // exercise facet's behavior on underflow:
    //   parsing succeeds (fail is clear), +/-min is stored

    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T,  FLT_MIN, _RWSTD_STRSTR ( _RWSTD_DBL_MIN), -1, 0, Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
#ifndef	SAME_AS_UNIX_COMMENT     
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, -FLT_MIN, _RWSTD_STRSTR (-_RWSTD_DBL_MIN), -1, 0, Eof);
    std_log(LOG_FILENAME_LINE,"Test end");
#endif

    pmax = 0;   // reset before next test


    rw_info (0, 0, 0, "get (..., float&) on overflow");

    const float inf = std::numeric_limits<float>::infinity ();

    // on overflow, get() follows C99 requirements on strtof()
    // it sets failbit

    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T,  inf, _RWSTD_STRSTR ( _RWSTD_DBL_MAX), -1, 0, Eof | Fail);
    std_log(LOG_FILENAME_LINE,"Test end");
#ifndef	SAME_AS_UNIX_COMMENT     
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, -inf, _RWSTD_STRSTR (-_RWSTD_DBL_MAX), -1, 0, Eof | Fail);
    std_log(LOG_FILENAME_LINE,"Test end");
#endif

    if (9.876543e+39 > FLT_MAX) {

    std_log(LOG_FILENAME_LINE,"Test start");
        TEST (T,  inf,  "9.876543e+39", 12, 0, Eof | Fail);
    std_log(LOG_FILENAME_LINE,"Test end");
#ifndef	FLT_MAX_NOT_SAME    
    std_log(LOG_FILENAME_LINE,"Test start");
        TEST (T, -inf, "-9.876543e+39", 13, 0, Eof | Fail);
    std_log(LOG_FILENAME_LINE,"Test end");
#endif

    }

    if (9.876543e+49 > FLT_MAX) {

    std_log(LOG_FILENAME_LINE,"Test start");
        TEST (T,  inf,  "9.876543e+49", 12, 0, Eof | Fail);
    std_log(LOG_FILENAME_LINE,"Test end");
#ifndef	FLT_MAX_NOT_SAME    
    std_log(LOG_FILENAME_LINE,"Test start");
        TEST (T, -inf, "-9.876543e+49", 13, 0, Eof | Fail);
    std_log(LOG_FILENAME_LINE,"Test end");
#endif    

    }

    if (9.876543e+99 > FLT_MAX) {

    std_log(LOG_FILENAME_LINE,"Test start");
        TEST (T,  inf,  "9.876543e+99", 12, 0, Eof | Fail);
    std_log(LOG_FILENAME_LINE,"Test end");
#ifndef	FLT_MAX_NOT_SAME    
    std_log(LOG_FILENAME_LINE,"Test start");
        TEST (T, -inf, "-9.876543e+99", 13, 0, Eof | Fail);
    std_log(LOG_FILENAME_LINE,"Test end");
#endif    

    }
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T,  inf,  "9.999999e+9999", 14, 0, Eof | Fail);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, -inf, "-9.999999e+9999", 15, 0, Eof | Fail);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");


    // exercise facet's behavior on overflow:
    //   parsing fails (fail is set) and +/-infinity is stored

    TEST (T,  inf, _RWSTD_STRSTR ( _RWSTD_DBL_MAX), -1, 0, Eof | Fail);
    std_log(LOG_FILENAME_LINE,"Test end");
#ifndef	SAME_AS_UNIX_COMMENT 
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, -inf, _RWSTD_STRSTR (-_RWSTD_DBL_MAX), -1, 0, Eof | Fail);
    std_log(LOG_FILENAME_LINE,"Test end");
#endif

#  ifdef _RWSTD_LDBL_MAX

    std::ios_base::iostate state = Fail;

    // eofbit set unless LDBL_MAX ends in an 'L' or 'l'
    // (it may not when it's the same as double)
    if (0 == std::strpbrk (_RWSTD_STRSTR ( _RWSTD_LDBL_MAX), "Ll"))
        state |= Eof;

    std_log(LOG_FILENAME_LINE,"Test start");

    TEST (T,  inf, _RWSTD_STRSTR ( _RWSTD_LDBL_MAX), -1, 0, state);
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, -inf, _RWSTD_STRSTR (-_RWSTD_LDBL_MAX), -1, 0, state);
    std_log(LOG_FILENAME_LINE,"Test end");


#  endif   // _RWSTD_LDBL_MAX


#else   // if defined (NO_GET_FLT)

    _RWSTD_UNUSED (t);

#endif   // NO_GET_FLT

}

/**************************************************************************/

template <class charT>
void test_flt (charT, const char *cname)
{
#ifndef NO_GET_FLT

    const char* const tname = "float";

    rw_info (0, 0, 0, "std::num_get<%s>::get (..., %s&)",
             cname, tname);

    Punct<charT>::thousands_sep_ = ';';

#  if    !defined (_RWSTD_NO_STRTOF) || !defined (_RWSTD_NO_STRTOF_IN_LIBC) \
      || !defined (_RWSTD_NO_STRTOD) || !defined (_RWSTD_NO_STRTOD_IN_LIBC)

    const bool test_locale = true;

#  else

    const bool test_locale = false;

#  endif   // _RWSTD_NO_STRTO{D,F}(_IN_LIBC)

    test_floating_point (charT (), float (), test_locale);

    const char *str = _RWSTD_STRSTR (FLT_MAX);

    // some platforms, e.g., HP-UX, #define FLT_MAX (float)3.40282347e+38F
    // guard against such definitions
    if (*str >= '0' && *str <= '9') {
        // FLT_MAX is something like "3.40282347e+38F" (note the 'F'
        // at the end) so parsing stops before the end is reached,
        // i.e., Eof is not set
#ifndef	FLT_MAX_NOT_SAME
    std_log(LOG_FILENAME_LINE,"Test start");
        TEST (T,  FLT_MAX, _RWSTD_STRSTR ( FLT_MAX), -1, 0, Good);
        std_log(LOG_FILENAME_LINE,"Test end");
#endif        
        std_log(LOG_FILENAME_LINE,"Test start");
            TEST (T, -FLT_MAX, _RWSTD_STRSTR (-FLT_MAX), -1, 0, Good);
        std_log(LOG_FILENAME_LINE,"Test end");
    }
    else {
        std_log(LOG_FILENAME_LINE,"Test start");
                TEST (T,  FLT_MAX, _RWSTD_STRSTR ( _RWSTD_FLT_MAX), -1, 0, Good);
        std_log(LOG_FILENAME_LINE,"Test end");
        std_log(LOG_FILENAME_LINE,"Test start");
        
        TEST (T, -FLT_MAX, _RWSTD_STRSTR (-_RWSTD_FLT_MAX), -1, 0, Good);
        std_log(LOG_FILENAME_LINE,"Test end");


    }

    if (rw_opt_no_uflow) {
        rw_note (0, 0, 0, "underflow test disabled");
    }
    else {
        test_flt_uflow (charT (), cname);
    }

#else   // if defined (NO_GET_FLT)

    _RWSTD_UNUSED (t);

#endif   // NO_GET_FLT

}

/**************************************************************************/


template <class charT>
void test_dbl_uflow (charT, const char *cname)
{
#ifndef NO_GET_DBL

    rw_info (0, 0, 0, "std::num_get<%s>::get (..., double&) on underflow",
             cname);

    // exercise bahvior on underflow
    double val = DBL_MIN;
    pmax = &val;

    // on underflow, get() follows C99 requirements on strtod()
    // i.e., it stores a value in the range [0, +/-DBL_MIN]
    // the function does NOT set failbit on underflow (regardless
    // of whether strtod() sets errno)
    //     If the result underflows (7.12.1), the functions return
    //     a value whose magnitude is no greater than the smallest
    //     normalized positive number in the return type; whether
    //     errno acquires the value ERANGE is implementation-defined.
#ifndef	SAME_AS_UNIX_COMMENT 
        std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, (val = DBL_MIN, 0.0),   "1.111111e-9999", 14, 0, Eof);
            std_log(LOG_FILENAME_LINE,"Test end");
            std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, (val = 0.0, -DBL_MIN), "-1.111111e-9999", 15, 0, Eof);
            std_log(LOG_FILENAME_LINE,"Test end");
#endif            
            std_log(LOG_FILENAME_LINE,"Test start");
    if (1.23456789e-309L < DBL_MIN) {
        std_log(LOG_FILENAME_LINE,"Test start");

        TEST (T, (val = DBL_MIN, 0.0),   "1.23456789e-309", 15, 0, Eof);
            std_log(LOG_FILENAME_LINE,"Test end");
            std_log(LOG_FILENAME_LINE,"Test start");
        TEST (T, (val = 0.0, -DBL_MIN), "-1.23456789e-309", 16, 0, Eof);
            std_log(LOG_FILENAME_LINE,"Test end");

    }

#if 0 //build break-floating constant out of range
    if (1.234567e-409L < DBL_MIN) {

            std_log(LOG_FILENAME_LINE,"Test start");
        TEST (T, (val = DBL_MIN, 0.0),   "1.23456789e-409", 15, 0, Eof);
            std_log(LOG_FILENAME_LINE,"Test end");
            std_log(LOG_FILENAME_LINE,"Test start");
        TEST (T, (val = 0.0, -DBL_MIN), "-1.23456789e-409", 16, 0, Eof);
            std_log(LOG_FILENAME_LINE,"Test end");

    }

    if (1.234567e-999L < DBL_MIN) {

            std_log(LOG_FILENAME_LINE,"Test start");
        TEST (T, (val = DBL_MIN, 0.0),   "1.23456789e-999", 15, 0, Eof);
            std_log(LOG_FILENAME_LINE,"Test end");
            std_log(LOG_FILENAME_LINE,"Test start");
        TEST (T, (val = 0.0, -DBL_MIN), "-1.23456789e-999", 16, 0, Eof);
            std_log(LOG_FILENAME_LINE,"Test end");

    }
#endif
    pmax = 0;   // reset before next test


#  ifdef _RWSTD_LDBL_MAX
    rw_info (0, 0, 0, "get (..., double&) on overflow");

    const double inf = std::numeric_limits<double>::infinity ();

    if (LDBL_MAX_EXP > DBL_MAX_EXP) {
        // exercise bahvior on overflow

            std_log(LOG_FILENAME_LINE,"Test start");
        TEST (T,  inf, _RWSTD_STRSTR ( _RWSTD_LDBL_MAX), -1, 0, Fail);
            std_log(LOG_FILENAME_LINE,"Test end");
            std_log(LOG_FILENAME_LINE,"Test start");
        TEST (T, -inf, _RWSTD_STRSTR (-_RWSTD_LDBL_MAX), -1, 0, Fail);
            std_log(LOG_FILENAME_LINE,"Test end");

    }

            std_log(LOG_FILENAME_LINE,"Test start");

    // long double may have the same range and precision as double
    TEST (T,  inf,  "9.999999e+9999", 14, 0, Eof | Fail);
            std_log(LOG_FILENAME_LINE,"Test end");
            std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, -inf, "-9.999999e+9999", 15, 0, Eof | Fail);
            std_log(LOG_FILENAME_LINE,"Test end");


#  endif   // _RWSTD_LDBL_MAX

#else   // if defined (NO_GET_DBL)

    _RWSTD_UNUSED (t);

#endif   // NO_GET_DBL

}

/**************************************************************************/

template <class charT>
void test_dbl (charT, const char *cname)
{
#ifndef NO_GET_DBL

    // 22.2.3.1, p2 -- floating point grammar:
    //
    // fltval ::=   [sign] units [decimal-point [digits]] [e [sign] digits]
    //            | [sign] decimal-point digits [e [sign] digits]
    // sign   ::= '+' | '-'
    // units  ::= digits [thousands-sep units]
    // digits ::= digit [digits]
    // digit  ::= '0' | '1' | '2' | '3' | '4' | '5' | '6' | '7' | '8' | '9'
    // e      ::= 'e' | 'E'

    const char* const tname = "double";

    rw_info (0, 0, 0, "std::num_get<%s>::get (..., %s&)",
             cname, tname);

#  if !defined (_RWSTD_NO_STRTOD) || !defined (_RWSTD_NO_STRTOD_IN_LIBC)

    const bool test_locale = true;

#  else

    const bool test_locale = false;

#  endif   // _RWSTD_NO_STRTOD(_IN_LIBC)

    test_floating_point (charT (), double (), test_locale);

    if (rw_opt_no_uflow) {
        rw_note (0, 0, 0, "underflow test disabled");
    }
    else {
        test_dbl_uflow (charT (), cname);
    }

#else   // if defined (NO_GET_DBL)

    _RWSTD_UNUSED (t);

#endif   // NO_GET_DBL

}

/**************************************************************************/


template <class charT>
void test_ldbl_uflow (charT, const char *cname)
{
#ifndef NO_GET_LDBL

    rw_info (0, 0, 0,
             "std::num_get<%s>::get (..., long double&) on underflow", cname);

    // exercise bahvior on underflow
    long double val = LDBL_MIN;
    pmax = &val;

    // on underflow, get() follows C99 requirements on strtold()
    // i.e., it stores a value in the range [0, +/-LDBL__MIN]
    // the function does NOT set failbit on underflow (regardless
    // of whether strtold() sets errno)
    //     If the result underflows (7.12.1), the functions return
    //     a value whose magnitude is no greater than the smallest
    //     normalized positive number in the return type; whether
    //     errno acquires the value ERANGE is implementation-defined.

            std_log(LOG_FILENAME_LINE,"Test start");

    TEST (T, (val = LDBL_MIN, 0.0L),   "1.987654321e-99999", 18, 0, Eof);
            std_log(LOG_FILENAME_LINE,"Test end");
            std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, (val = 0.0L, -LDBL_MIN), "-1.987654321e-99999", 19, 0, Eof);
            std_log(LOG_FILENAME_LINE,"Test end");


    const char fmt[] = "%Lg";

    long double ld = 0.0;
    #ifdef __SYMBIAN32__
     if (1 == sscanf ("3.456789e-4932", fmt, &ld) && ld < LDBL_MIN) {

            std_log(LOG_FILENAME_LINE,"Test start");
        TEST (T, (val = LDBL_MIN, 0.0L),   "3.456789e-4932", 14, 0, Eof);
            std_log(LOG_FILENAME_LINE,"Test end");
            std_log(LOG_FILENAME_LINE,"Test start");
        TEST (T, (val = 0.0L, -LDBL_MIN), "-3.456789e-4932", 15, 0, Eof);
            std_log(LOG_FILENAME_LINE,"Test end");

    }

    if (1 == sscanf ("3.456789e-9999", fmt, &ld) && ld < LDBL_MIN) {

            std_log(LOG_FILENAME_LINE,"Test start");
        TEST (T, (val = LDBL_MIN, 0.0L),   "3.456789e-9999", 14, 0, Eof);
            std_log(LOG_FILENAME_LINE,"Test end");
            std_log(LOG_FILENAME_LINE,"Test start");
        TEST (T, (val = 0.0L, -LDBL_MIN), "-3.456789e-9999", 15, 0, Eof);
            std_log(LOG_FILENAME_LINE,"Test end");

    }
    #else
    if (1 == std::sscanf ("3.456789e-4932", fmt, &ld) && ld < LDBL_MIN) {

            std_log(LOG_FILENAME_LINE,"Test start");
        TEST (T, (val = LDBL_MIN, 0.0L),   "3.456789e-4932", 14, 0, Eof);
            std_log(LOG_FILENAME_LINE,"Test end");
            std_log(LOG_FILENAME_LINE,"Test start");
        TEST (T, (val = 0.0L, -LDBL_MIN), "-3.456789e-4932", 15, 0, Eof);
            std_log(LOG_FILENAME_LINE,"Test end");

    }

    if (1 == std::sscanf ("3.456789e-9999", fmt, &ld) && ld < LDBL_MIN) {

            std_log(LOG_FILENAME_LINE,"Test start");
        TEST (T, (val = LDBL_MIN, 0.0L),   "3.456789e-9999", 14, 0, Eof);
            std_log(LOG_FILENAME_LINE,"Test end");
            std_log(LOG_FILENAME_LINE,"Test start");
        TEST (T, (val = 0.0L, -LDBL_MIN), "-3.456789e-9999", 15, 0, Eof);
            std_log(LOG_FILENAME_LINE,"Test end");

    }
   #endif //__SYMBIAN32__
    pmax = 0;   // reset before next test


#  ifdef _RWSTD_LDBL_MAX

    rw_info (0, 0, 0, "get (..., long double&) on overflow");

    const double inf = std::numeric_limits<long double>::infinity ();

            std_log(LOG_FILENAME_LINE,"Test start");

    TEST (T,  inf,  "9.999999e+9876543210", 20, 0, Eof | Fail);
            std_log(LOG_FILENAME_LINE,"Test end");
            std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, -inf, "-9.999999e+9876543210", 21, 0, Eof | Fail);
            std_log(LOG_FILENAME_LINE,"Test end");


#  endif   // _RWSTD_LDBL_MAX

#else   // if defined (NO_GET_LDBL)

    _RWSTD_UNUSED (t);

#endif   // NO_GET_LDBL

}

/**************************************************************************/

template <class charT>
void test_ldbl (charT, const char *cname)
{
#ifndef NO_GET_LDBL

#  ifndef _RWSTD_NO_LONG_DOUBLE

    const char* const tname = "long double";

    rw_info (0, 0, 0, "std::num_get<%s>::get (..., %s&)",
             cname, tname);

#    if !defined (_RWSTD_NO_STRTOLD) || !defined (_RWSTD_NO_STRTOLD_IN_LIBC)

    const bool test_locale = true;

#    else

    const bool test_locale = false;

#    endif   // _RWSTD_NO_STRTOLD(_IN_LIBC)

    test_floating_point (charT (), (long double)0, test_locale);

    long double long_val;

    char long_str [_RWSTD_LDBL_MAX_10_EXP + 2];
    
    #ifdef __SYMBIAN32__
     memset (long_str, '0', sizeof long_str);
    #else 
    std::memset (long_str, '0', sizeof long_str);
    #endif
    long_str [sizeof long_str - 1] = '\0';

    // parse a string of LDBL_MAX_10_EXP + 1 zeros

            std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, 0.0L, long_str, sizeof long_str - 1, 0, Eof);
            std_log(LOG_FILENAME_LINE,"Test end");


    const char *start;

#    if _RWSTD_LDBL_MAX_10_EXP >= 38

    // parse "10...<repeats 37 times>" (i.e., 1 followed by 38 zeros)
    start = &(long_str [sizeof long_str - 40] = '1');

            std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, 1.0e+38L, start, 39, 0, Eof);
            std_log(LOG_FILENAME_LINE,"Test end");
            std_log(LOG_FILENAME_LINE,"Test start");

    // parse the same as above but preceded by a bunch of zeros
    TEST (T, 1.0e+38L, long_str, sizeof long_str - 1, 0, Eof);
                std_log(LOG_FILENAME_LINE,"Test end");


    long_str [sizeof long_str - 40] = '0';

#    endif   // _RWSTD_LDBL_MAX_10_EXP > 129


#    if _RWSTD_LDBL_MAX_10_EXP >= 128

    // parse "10...<repeats 127 times>" (i.e., 1 followed by 128 zeros)
    start = &(long_str [sizeof long_str - 130] = '1');
            std_log(LOG_FILENAME_LINE,"Test start");
            TEST (T, 1.0e+128L, start, 129, 0, Eof);
            std_log(LOG_FILENAME_LINE,"Test end");
            std_log(LOG_FILENAME_LINE,"Test start");

    // parse the same as above but preceded by a bunch of zeros
    TEST (T, 1.0e+128L, long_str, sizeof long_str - 1, 0, Eof);
            std_log(LOG_FILENAME_LINE,"Test end");
            
    long_str [sizeof long_str - 130] = '0';

#    endif   // _RWSTD_LDBL_MAX_10_EXP > 129

#    if _RWSTD_LDBL_MAX_10_EXP >= 308

    // parse "10...<repeats 307 times>" (i.e., 1 followed by 308 zeros)
    start = &(long_str [sizeof long_str - 310] = '1');
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, 1.0e+308L, start, 309, 0, Eof);
std_log(LOG_FILENAME_LINE,"Test end");
std_log(LOG_FILENAME_LINE,"Test start");
    // parse the same as above but preceded by a bunch of zeros
    TEST (T, 1.0e+308L, long_str, sizeof long_str - 1, 0, Eof);
std_log(LOG_FILENAME_LINE,"Test end");

    long_str [sizeof long_str - 310] = '0';

#    endif   // _RWSTD_LDBL_MAX_10_EXP > 308

#    undef CAT
#    undef CONCAT

#    define CAT(a, b)      CONCAT (a, b)
#    define CONCAT(a, b)   a ## b

    // parse "10..." (i.e., 1 followed by LDBL_MAX_10_EXP zeros)
    long_str [0] = '1';
    long_val     = CAT (CAT (1.0e+, _RWSTD_LDBL_MAX_10_EXP), L);
std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, long_val, long_str, sizeof long_str - 1, 0, Eof);
std_log(LOG_FILENAME_LINE,"Test end");
#  endif   // _RWSTD_NO_LONG_DOUBLE

    if (rw_opt_no_uflow) {
        rw_note (0, 0, 0, "underflow test disabled");
    }
    else {
        test_ldbl_uflow (charT (), cname);
    }

#else   // if defined (NO_GET_LDBL)

    _RWSTD_UNUSED (t);

#endif   // NO_GET_LDBL

}

/**************************************************************************/

template <class charT>
void run_tests (charT, const char *cname)
{
#undef TEST
#define TEST(T, tname)                                          \
    if (rw_enabled (tname))                                     \
        test_ ## T (charT (), cname);                           \
    else                                                        \
        rw_note (0, __FILE__, __LINE__, "%s test disabled", tname)

#ifndef _RWSTD_NO_BOOL
    std_log(LOG_FILENAME_LINE,"Test start");
    
    TEST (bool, "bool");
std_log(LOG_FILENAME_LINE,"Test end");

#endif   // _RWSTD_NO_BOOL

   //#ifndef __SYMBIAN32__  // Bug -> 481
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (shrt, "short");
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (int, "int");
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
  //#endif 
    TEST (long, "long");
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (ulong, "unsigned long");
    std_log(LOG_FILENAME_LINE,"Test end");

#if 0//FIX //no long long
#ifndef _RWSTD_NO_LONG_LONG

    std_log(LOG_FILENAME_LINE,"Test start");

    TEST (llong, "long long");
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (ullong, "unsigned long long");
    std_log(LOG_FILENAME_LINE,"Test end");

#endif //#if 0
#endif   // _RWSTD_NO_LONG_LONG

    std_log(LOG_FILENAME_LINE,"Test start");

    TEST (pvoid, "void*");
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");

    TEST (flt, "float");
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
    TEST (dbl, "double");
    std_log(LOG_FILENAME_LINE,"Test end");
    std_log(LOG_FILENAME_LINE,"Test start");
#if 0//FIX //no long double    
    TEST (ldbl, "long double");
    std_log(LOG_FILENAME_LINE,"Test end");

#endif //#if 0
}

/**************************************************************************/

static int
run_test (int, char*[])
{
    if (rw_enabled ("char"))
        run_tests (char (), "char");
    else
        rw_note (0, __FILE__, __LINE__, "char test disabled");

#ifndef _RWSTD_NO_WCHAR_T

    if (rw_enabled ("wchar_t"))
        run_tests (wchar_t (), "wchar_t");
    else
        rw_note (0, __FILE__, __LINE__, "wchar_t test disabled");

#else   // if defined (_RWSTD_NO_WCHAR_T)

    rw_note (0, __FILE__, __LINE__,
             "wchar_t test disabled: _RWSTD_NO_WCHAR_T #defined");

#endif   // _RWSTD_NO_WCHAR_T

    return 0;
}

/**************************************************************************/
//#endif // #if 0
int main (int argc, char *argv[])
{
 #ifndef __SYMBIAN32__
    return rw_test (argc, argv, __FILE__,"lib.locale.num.get",0 , run_test,"|-no-errno# ""|-no-grouping# ""|-no-widen# ""|-no-underflow#",&rw_opt_no_errno,&rw_opt_no_grouping,&rw_opt_no_widen,&rw_opt_no_uflow);
 #else
 rw_test (argc, argv, __FILE__,"lib.locale.num.get",0 , run_test,"|-no-errno# ""|-no-grouping# ""|-no-widen# ""|-no-underflow#",&rw_opt_no_errno,&rw_opt_no_grouping,&rw_opt_no_widen,&rw_opt_no_uflow);
      if(failures)
      {
            assert_failed = true;
       std_log(LOG_FILENAME_LINE,"Result: Failed");
      }
     else
      {

       std_log(LOG_FILENAME_LINE,"Result: Passed");
      }
  
   std_log(LOG_FILENAME_LINE,"[End Test Case]");  
   
   testResultXml("22_locale_num_get");
    close_log_file();
 
 return 0;
 #endif //__SYMBIAN32__
}

