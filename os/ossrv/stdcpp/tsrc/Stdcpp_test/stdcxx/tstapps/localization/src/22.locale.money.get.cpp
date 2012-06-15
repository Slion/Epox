/***************************************************************************
 *
 * locale.money.put.cpp - tests exercising the std::money_get facet
 *
 * $Id: 22.locale.money.get.cpp 279135 2005-09-06 21:50:57Z sebor $
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

#include <ios>
#include <locale>

#include <cstdio>    // for sscanf()
#include <cstdlib>   // for mbstowcs(), wcstombs()
#include <cstring>   // for strcat(), strlen()

#include <cmdopt.h>
#include <driver.h>
#include <valcmp.h>

#ifdef __SYMBIAN32__
#include<assert.h>
#include<sstream>
#include<iterator>
#include<istream>
#include"std_log_result.h"
#define LOG_FILENAME_LINE __FILE__, __LINE__
int failures=0;
#endif 
using namespace std;

#ifndef _RWSTD_NO_LONG_DOUBLE
#  define LongDouble long double
#else
#  define LongDouble   double
#  define rw_ldblcmp   rw_dblcmp
#endif   // _RWSTD_NO_LONG_DOUBLE

#define	SAME_AS_UNIX_COMMENT	
/**************************************************************************/

// set to non-zero value when the basic_string overloads
// of moneY-put::put() shouldn't be exercised
static int no_basic_string;

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

int foo ()
{
    static int i;

    return ++i;
}

/**************************************************************************/

template <class charT, bool Intl>
struct Punct: std::moneypunct<charT, Intl>
{
    typedef std::moneypunct<charT, Intl> Base;
    typedef typename Base::char_type     char_type;
    typedef typename Base::string_type   string_type;

    static char_type                 decimal_point_;
    static char_type                 thousands_sep_;
    static const char               *grouping_;
    static const char_type          *curr_symbol_;
    static const char_type          *positive_sign_;
    static const char_type          *negative_sign_;
    static std::money_base::pattern  format_ [2];
    static int                       frac_digits_;

    static int n_thousands_sep_;   // number of calls to do_thousands_sep()

    Punct (std::size_t ref): Base (ref) { }

    char_type do_decimal_point () const {
        return decimal_point_;
    }

    int do_frac_digits () const {
        return frac_digits_;
    }

    virtual string_type do_curr_symbol () const {
        foo ();
        return curr_symbol_ ? curr_symbol_ : string_type ();
    }

    virtual std::string do_grouping () const {
        return grouping_ ? std::string (grouping_) : std::string ();
    }

    virtual char_type do_thousands_sep () const {
        ++n_thousands_sep_;
        return thousands_sep_;
    }

    virtual string_type do_positive_sign () const {
        return positive_sign_ ? positive_sign_ : string_type ();
    }

    virtual string_type do_negative_sign () const {
        return negative_sign_ ? negative_sign_ : string_type ();
    }

    virtual std::money_base::pattern do_pos_format () const {
        return format_ [0];
    }

    virtual std::money_base::pattern do_neg_format () const {
        return format_ [1];
    }
};

template <class charT, bool Intl>
const char* Punct<charT, Intl>::grouping_ = "";

template <class charT, bool Intl>
typename Punct<charT, Intl>::char_type
Punct<charT, Intl>::decimal_point_ = '.';

template <class charT, bool Intl>
typename Punct<charT, Intl>::char_type
Punct<charT, Intl>::thousands_sep_ = ',';

template <class charT, bool Intl>
const typename Punct<charT, Intl>::char_type*
Punct<charT, Intl>::curr_symbol_;

template <class charT, bool Intl>
const typename Punct<charT, Intl>::char_type*
Punct<charT, Intl>::positive_sign_;

template <class charT, bool Intl>
const typename Punct<charT, Intl>::char_type*
Punct<charT, Intl>::negative_sign_;

template <class charT, bool Intl>
std::money_base::pattern Punct<charT, Intl>::format_[2] = {
    { { // positive
        std::money_base::symbol, std::money_base::sign,
        std::money_base::none,   std::money_base::value
    } },
    { { // negative
        std::money_base::symbol, std::money_base::sign,
        std::money_base::none,   std::money_base::value
    } }
};

template <class charT, bool Intl>
int Punct<charT, Intl>::frac_digits_;


template <class charT, bool Intl>
int Punct<charT, Intl>::n_thousands_sep_;


/**************************************************************************/
template <class charT>
struct Ios: std::basic_ios<charT>
{
    Ios () { this->init (0); }
};
template <class charT>
struct MoneyGet: std::money_get<charT, const charT*> { };

/**************************************************************************/

const char* narrow (char *dst, const char *src)
{
    if (src == dst || !src || !dst)
        return src;

  #ifdef __SYMBIAN32__
    memcpy (dst, src, strlen (src) + 1);
  #else  
    std::memcpy (dst, src, std::strlen (src) + 1);
  #endif  
    return dst;
}


const char* widen (char *dst, const char *src)
{
    if (src == dst || !src || !dst)
         return src;
  #ifdef __SYMBIAN32__
    memcpy (dst, src, strlen (src) + 1);
   #else  
    std::memcpy (dst, src, std::strlen (src) + 1);
#endif 
    return dst;
}


const char* narrow (char *dst, const wchar_t *src)
{
    static char buf [4096];

    if (!src)
        return 0;

    if (!dst)
        dst = buf;
  #ifdef __SYMBIAN32__
    size_t len = wcslen (src);
  #else  
    std::size_t len = std::wcslen (src);
  #endif
    _RWSTD_ASSERT (len < sizeof buf);
  
  #ifdef __SYMBIAN32__
   len = wcstombs (dst, src, sizeof buf / sizeof *buf);
  #else 
    len = std::wcstombs (dst, src, sizeof buf / sizeof *buf);
  #endif
    if (std::size_t (-1) == len)
        *dst = 0;

    return dst;
}


const wchar_t* widen (wchar_t *dst, const char *src)
{
    static wchar_t buf [4096];

    if (!src)
        return 0;

    if (!dst)
         dst = buf;
   
   #ifdef __SYMBIAN32__
    size_t len = strlen (src);
   #else    
    std::size_t len = std::strlen (src);
   #endif 

    _RWSTD_ASSERT (len < sizeof buf /sizeof *buf);

    #ifdef __SYMBIAN32__
      len =mbstowcs (dst, src, sizeof buf / sizeof *buf); 
    #else  
    len = std::mbstowcs (dst, src, sizeof buf / sizeof *buf);
    #endif
    if (std::size_t (-1) == len)
        *dst = 0;

    return dst;
}

//#ifndef __SYMBIAN32__
template <class charT>
void do_test (charT       which,   // which overload to exercise
              const char *cname,   // the name of the charT type
              const char *tname,   // the name of the floating point type
              int         lineno,  // line number
              LongDouble  val,     // the expected extracted value
              const char *str,     // the sequence to extract from
              int         consumed = -1,   // expected number extractions
              int         flags = 0,       // initial ios flags
              int         err_expect = -1, // expected final state
              int         frac_digits = 0, // fractional digits
              const char *fmat = 0,        // money_base::pattern
              const char *cursym = 0,      // currency symbol
              const char *grouping = "",   // grouping string
              bool intl = false)           // international?
/*#else
template <class charT>
 void do_test (charT       which,   // which overload to exercise
              const char *cname,   // the name of the charT type
              const char *tname,   // the name of the floating point type
              int         lineno,  // line number
              basic_string<charT>&  val,     // the expected extracted value
              const char *str,     // the sequence to extract from
              int         consumed = -1,   // expected number extractions
              int         flags = 0,       // initial ios flags
              int         err_expect = -1, // expected final state
              int         frac_digits = 0, // fractional digits
              const char *fmat = 0,        // money_base::pattern
              const char *cursym = 0,      // currency symbol
              const char *grouping = "",   // grouping string
              bool intl = false)           // international?
#endif              */
 {
    if (!rw_enabled (lineno)) {
        rw_note (0, __FILE__, __LINE__, "test on line %d disabled", lineno);
        return;
    }

    if (!fmat) {
        // if fmat isn't set, use the default pattern
        static const std::money_base::pattern pat = { {
            std::money_base::symbol, std::money_base::sign,
            std::money_base::none,   std::money_base::value
        } };

        fmat = pat.field;
    }

    // take care to initialize Punct static data before installing
    // the facet in case locale or the base facet calls the overridden
    // virtuals early to cache the results
    Punct<charT, false>::format_ [1].field [0] = fmat [0];
    Punct<charT, false>::format_ [1].field [1] = fmat [1];
    Punct<charT, false>::format_ [1].field [2] = fmat [2];
    Punct<charT, false>::format_ [1].field [3] = fmat [3];

    // zero out positive format (not used by money_get)
    Punct<charT, false>::format_ [0] = std::money_base::pattern ();

    charT wcursym_buf [40];
    Punct<charT, false>::curr_symbol_ = widen (wcursym_buf, cursym);
    Punct<charT, false>::grouping_    = grouping;
    Punct<charT, false>::frac_digits_ = frac_digits;

    Ios<charT> io;
    MoneyGet<charT> mg;

    // create distinct punctuation facets for each iteration to make sure
    // any data cached in between successive calls to the facet's public
    // member functions are flushed
    if (intl) {
        const std::moneypunct<charT, true> *pf = new Punct<charT, true>(0);
        io.imbue (std::locale (io.getloc (), pf));
    }
    else {
        const std::moneypunct<charT, false> *pf = new Punct<charT, false>(0);
        io.imbue (std::locale (io.getloc (), pf));
    }

    io.flags (std::ios_base::fmtflags (flags));

    // widen input string if necessary
   
   //#ifndef __SYMBIAN32__
    charT wstr_buf [4096];
   // const charT *next = widen (wstr_buf, str);
   // const charT *last;
   const charT *next = widen (wstr_buf, str);
   charT *last;
   std::ios_base::iostate err = std::ios_base::goodbit;
   typedef basic_string<charT> stringbuf;
   #ifndef __SYMBIAN32__
    LongDouble x;
   #else 
   stringbuf x;
   #endif
   if (0 == which) {   // exercise get (..., long double)

   
        //#ifndef __SYMBIAN32__
        last =(charT*)(next + (std::char_traits<charT>::length (next)));
       // long double x = 0;
        
        last =(charT*) mg.get (next, last, intl, io, err, x);
        /*#else
        string x; 
            last(mg.get (next, last, intl, io, err, x));
        #endif  */
    
  
        if (-1 == err_expect)
            err_expect = err;

        const int success =
            !(-1 != consumed && last - next != consumed || err != err_expect);

        rw_assert (success, __FILE__, lineno,
                   "money_get<%s>::get (%s, ..., %d, ..., %s&), "
                   "ate %d, expected %d, frac_digits = %d, "
                   "flags = %{If}, grouping = %#s, pattern = %{LM}, "
                   "state = %{Is}, expected = %{Is}",
                   cname, next, intl, tname,
                   last - next, consumed, frac_digits,
                   flags, grouping, fmat,
                   err, err_expect);

      #ifndef __SYMBIAN32__
        rw_assert (2 > rw_ldblcmp (x, val), __FILE__, lineno,
                   "money_get<%s>::get (%s, ..., %d, ..., %s&), "
                   "got %Lg, expected %Lg, frac_digits = %d, "
                   "flags = %{If}s, grouping = %#s, pattern = %{LM}, "
                   "state = %{Is}, expected %{Is}",
                   cname, next, intl, tname,
                   x, val, frac_digits,
                   flags, grouping, fmat,
                   err, err_expect);
      #else
        if(!(2>rw_strcmp(x,val)))
        { 
         failures++;
         std_log(LOG_FILENAME_LINE,"Reason : Failing");
         }
      #endif    
          
                     
    }
    else {   // exercise get (..., string_type)

        static const charT initial[] = { '*', '*', '*', '\0' };
        typename std::money_get<charT, const charT*>::string_type bs (initial);

        last =(charT*)(next + std::char_traits<charT>::length (next));

        last =(charT*)mg.get (next, last, intl, io, err, bs);

        int success =
            !(-1 != consumed && last - next != consumed || err != err_expect);

        #ifndef __SYMBIAN32__
        rw_assert (success, __FILE__, lineno,
                   "money_get<%s>::get (%s, ..., %d, ..., "
                   "basic_string<%s>&), ate %d, expected %d, "
                   "frac_digits = %d, flags = %{If}, grouping = %#s"
                   ", format = %{LM}, state = %{Is}, expected = %{Is}",
                   cname, next, intl,
                   cname, last - next, consumed,
                   frac_digits, flags, grouping,
                   fmat, err, err_expect);
        #else
          if(!success)   
          {
            failures++;
            std_log(LOG_FILENAME_LINE,"Reason:Failing");
            }
         #endif   
           
        if (success) {

            char narrow_buf [4096];
            narrow (narrow_buf, bs.c_str ());

            LongDouble x = 0;

            // prevent gcc warning: ANSI C does not support
            // the `L' length modifier
            const char fmt[] = "%" _RWSTD_LDBL_PRINTF_PREFIX "g";
           
            #ifdef __SYMBIAN32__
             int n = sscanf (narrow_buf, fmt, &x);           
            #else
            int n = std::sscanf (narrow_buf, fmt, &x);
            #endif

            success = 
                !(   err_expect & std::ios::failbit && !*grouping
                  && (1 == n || bs != initial)
                  || !(err_expect & std::ios::failbit)
                  && 1 < rw_ldblcmp (x, val));
            
            #ifndef __SYMBIAN32__
            rw_assert (success, __FILE__, lineno,
                       "money_get<%s>::get (%s, ..., %d, ..., "
                       "basic_string<%s>&), got %s, expected %Lg, "
                       "frac_digits = %d, flags = %{If}, grouping = %#s,"
                       " pattern = %{LM}, iostate = %{Is}, expected %{Is}",
                       cname, next, intl,
                       cname, bs.c_str (), val,
                       frac_digits, flags, grouping,
                       fmat, err, err_expect);
             #else
              if(!success)
              {
                failures++;
                std_log(LOG_FILENAME_LINE,"Reason: Failing");
                }
              #endif  
        }
    }
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

#define badbit      std::ios_base::badbit
#define eofbit      std::ios_base::eofbit
#define failbit     std::ios_base::failbit
#define goodbit     std::ios_base::goodbit


template <class charT>
void test_memfun (charT opt, const char *cname, const char *tname)
{
#define T      opt, cname, tname, __LINE__
#define TEST   do_test

    rw_info (0, 0, 0,
             "money_get<%s, istreambuf_iterator<%s> >::get "
             "(..., %s&) const", cname, cname, tname);

    // 22.2.6.3, p3:
    // The format of the numeric monetary value is a decimal number:
    //     value ::= units [ decimalpoint [ digits ]] | decimalpoint digits
    // If frac_digits() returns a positive value, or
    //     value ::= units
    // otherwise. The symbol decimalpoint indicates the character returned
    // by decimal_point(). The other symbols are defined as follows:
    //     units ::= digits [ thousandssep units ] digits ::= adigit [ digits ]

    Punct<charT, false>::positive_sign_ = 0;
    Punct<charT, false>::negative_sign_ = 0;
    Punct<charT, false>::thousands_sep_ = '\0';

    // parsing zero, no curr_symbol or sign, default pattern, 0 frac_digits
   std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, 0.0, "0",   1, 0, eofbit);
   std_log(LOG_FILENAME_LINE,"Test end") ;
   std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, 0.0, "0 ",  1, 0, goodbit);
   std_log(LOG_FILENAME_LINE,"Test end") ;
   std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, 0.0, "00",  2, 0, eofbit);
   std_log(LOG_FILENAME_LINE,"Test end") ;
   std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, 0.0, "000", 3, 0, eofbit);
   std_log(LOG_FILENAME_LINE,"Test end") ;
   std_log(LOG_FILENAME_LINE,"Test start");

    // optional whitespace is consumed where none or space appears
    TEST (T, 0.0, " 0",   2, 0, eofbit);
   std_log(LOG_FILENAME_LINE,"Test end") ;
   std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, 0.0, "  0 ", 3, 0, goodbit);
   std_log(LOG_FILENAME_LINE,"Test end") ;
   std_log(LOG_FILENAME_LINE,"Test start");

    // verify that decimal point isn't extracted (frac_digits == 0)
    TEST (T, 0.0, "0.",    1, 0, goodbit);
   std_log(LOG_FILENAME_LINE,"Test end") ;
   std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, 0.0, "0.0",   1, 0, goodbit);
   std_log(LOG_FILENAME_LINE,"Test end") ;
   std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, 0.0, "0.00",  1, 0, goodbit);
   std_log(LOG_FILENAME_LINE,"Test end") ;
   std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, 0.0, "00.0",  2, 0, goodbit);
   std_log(LOG_FILENAME_LINE,"Test end") ;
   std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, 0.0, "000.0", 3, 0, goodbit);
   std_log(LOG_FILENAME_LINE,"Test end") ;
   std_log(LOG_FILENAME_LINE,"Test start");

    // parsing zero, non-zero frac_digits
    TEST (T, 0.0, "0.",    2, 0, eofbit,  1);
   std_log(LOG_FILENAME_LINE,"Test end") ;
   std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, 0.0, "0. ",   2, 0, goodbit, 2);
   std_log(LOG_FILENAME_LINE,"Test end") ;
   std_log(LOG_FILENAME_LINE,"Test start");

#ifndef	SAME_AS_UNIX_COMMENT   
    TEST (T, 0.0, ".0",    2, 0, eofbit,  3); //fails in unix also
#endif
   std_log(LOG_FILENAME_LINE,"Test end") ;
   std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, 0.0, "0.0",   3, 0, eofbit,  4);
   std_log(LOG_FILENAME_LINE,"Test end") ;
   std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, 0.0, "0.00",  4, 0, eofbit,  5);
   std_log(LOG_FILENAME_LINE,"Test end") ;
   std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, 0.0, "00.0",  4, 0, eofbit,  6);
   std_log(LOG_FILENAME_LINE,"Test end") ;
   std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, 0.0, "00.00", 5, 0, eofbit,  7);
   std_log(LOG_FILENAME_LINE,"Test end") ;
   std_log(LOG_FILENAME_LINE,"Test start");

    // parsing zero, bad format
    TEST (T, 0.0, "$0",   0, 0, failbit);
   std_log(LOG_FILENAME_LINE,"Test end") ;
   std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, 0.0, "+0",   0, 0, failbit);
   std_log(LOG_FILENAME_LINE,"Test end") ;
   std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, 0.0, "-0",   0, 0, failbit);
   std_log(LOG_FILENAME_LINE,"Test end") ;
   std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, 0.0, ".0",   0, 0, failbit);
   std_log(LOG_FILENAME_LINE,"Test end") ;
   std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, 0.0, ".1",   0, 0, failbit);
   std_log(LOG_FILENAME_LINE,"Test end") ;
   std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, 0.0, ".",    0, 0, failbit);
   std_log(LOG_FILENAME_LINE,"Test end") ;
#ifndef	SAME_AS_UNIX_COMMENT  
   std_log(LOG_FILENAME_LINE,"Test start");   
    TEST (T, 0.0, "",     0, 0, eofbit | failbit);    
   std_log(LOG_FILENAME_LINE,"Test end") ;
#endif   
   std_log(LOG_FILENAME_LINE,"Test start");

    // parsing non-zero, no curr_symbol or sign, default pattern, 0 frac_digits
    TEST (T,   1.0, "1",    1, 0, eofbit);
   std_log(LOG_FILENAME_LINE,"Test end") ;
   std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T,   2.0, "2",    1, 0, eofbit);
   std_log(LOG_FILENAME_LINE,"Test end") ;
   std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T,   3.0, "3",    1, 0, eofbit);
   std_log(LOG_FILENAME_LINE,"Test end") ;
   std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T,   4.0, "4",    1, 0, eofbit);
   std_log(LOG_FILENAME_LINE,"Test end") ;
   std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T,   5.0, "5.1",  1, 0, goodbit);
   std_log(LOG_FILENAME_LINE,"Test end") ;
   std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T,   6.0, "6.23", 1, 0, goodbit);
   std_log(LOG_FILENAME_LINE,"Test end") ;
   std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T,  73.0, "73.4", 2, 0, goodbit);
   std_log(LOG_FILENAME_LINE,"Test end") ;
   std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, 845.0, "845.", 3, 0, goodbit);
   std_log(LOG_FILENAME_LINE,"Test end") ;
   std_log(LOG_FILENAME_LINE,"Test start");

    // parsing non-zero, frac_digits == 1
    TEST (T, 100.0, "10",   2, 0, eofbit, 1);
   std_log(LOG_FILENAME_LINE,"Test end") ;
   std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T,  90.0, "9",    1, 0, eofbit, 1);
   std_log(LOG_FILENAME_LINE,"Test end") ;
   std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T,  80.0, "8.",   2, 0, eofbit, 1);
   std_log(LOG_FILENAME_LINE,"Test end") ;
#ifndef	SAME_AS_UNIX_COMMENT     
   std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T,   7.0, "0.7",  3, 0, eofbit, 1);
   std_log(LOG_FILENAME_LINE,"Test end") ;
   std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T,   6.0, ".6",   2, 0, eofbit, 1);
   std_log(LOG_FILENAME_LINE,"Test end") ;
   std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T,   0.0, ".05",  3, 0, eofbit, 1);
   std_log(LOG_FILENAME_LINE,"Test end") ;
   std_log(LOG_FILENAME_LINE,"Test start");

    // parsing non-zero, frac_digits > 1
    TEST (T,         4.0, ".04",       3, 0, eofbit, 2);
   std_log(LOG_FILENAME_LINE,"Test end") ;
   std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T,        32.0, ".32",       3, 0, eofbit, 2);
#endif    
   std_log(LOG_FILENAME_LINE,"Test end") ;
   std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T,       210.0, "2.10",      4, 0, eofbit, 2);
   std_log(LOG_FILENAME_LINE,"Test end") ;
   std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T,      1098.0, "10.98",     5, 0, eofbit, 2);
   std_log(LOG_FILENAME_LINE,"Test end") ;
   std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T,       987.0, "09.876",    6, 0, eofbit, 2);
   std_log(LOG_FILENAME_LINE,"Test end") ;
   std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T,      9876.0, "98.765",    6, 0, eofbit, 2);
   std_log(LOG_FILENAME_LINE,"Test end") ;
   std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T,     87654.0, "876.5432",  8, 0, eofbit, 2);
   std_log(LOG_FILENAME_LINE,"Test end") ;
   std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T,    765432.0, "765.4321",  8, 0, eofbit, 3);
   std_log(LOG_FILENAME_LINE,"Test end") ;
   std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T,   6543210.0, "654.32109", 9, 0, eofbit, 4);
   std_log(LOG_FILENAME_LINE,"Test end") ;
   std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T,  54321098.0, "5432.1098", 9, 0, eofbit, 4);
   std_log(LOG_FILENAME_LINE,"Test end") ;
   std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, 432109870.0, "43210.987", 9, 0, eofbit, 4);
   std_log(LOG_FILENAME_LINE,"Test end") ;
   std_log(LOG_FILENAME_LINE,"Test start");

    Punct<charT, false>::thousands_sep_ = ',';

    // parsing with thousands_sep and/or grouping
    TEST (T,      0.0, "0",        1, 0, eofbit,  0, 0, "", "\1");
   std_log(LOG_FILENAME_LINE,"Test end") ;
   std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T,      0.0, "0,0",      3, 0, eofbit,  0, 0, "", "\1");
   std_log(LOG_FILENAME_LINE,"Test end") ;
   std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T,      0.0, "0,",       1, 0, goodbit, 0, 0, "", "\0");
   std_log(LOG_FILENAME_LINE,"Test end") ;
   std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T,      0.0, "0,0,0",    5, 0, eofbit,  0, 0, "", "\1");
   std_log(LOG_FILENAME_LINE,"Test end") ;
   std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T,      1.0, "0,0,1",    5, 0, eofbit,  0, 0, "", "\1");
   std_log(LOG_FILENAME_LINE,"Test end") ;
   std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T,      2.0, "0,0,2.",   5, 0, goodbit, 0, 0, "", "\1");
   std_log(LOG_FILENAME_LINE,"Test end") ;
   std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T,     30.0, "0,0,3.",   6, 0, eofbit,  1, 0, "", "\1");
   std_log(LOG_FILENAME_LINE,"Test end") ;
   std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T,      4.0, "0,0.4",    5, 0, eofbit,  1, 0, "", "\1");
   std_log(LOG_FILENAME_LINE,"Test end") ;
   std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T,   5670.0, "5,67.0",   6, 0, eofbit,  1, 0, "", "\2");
   std_log(LOG_FILENAME_LINE,"Test end") ;
   std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T,   6789.0, "678.9",    5, 0, eofbit,  1, 0, "", "\3");
   std_log(LOG_FILENAME_LINE,"Test end") ;
   std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, 789012.0, "7,890.12", 8, 0, eofbit,  2, 0, "", "\3");
   std_log(LOG_FILENAME_LINE,"Test end") ;
   std_log(LOG_FILENAME_LINE,"Test start");

    // 22.2.6.1.2, p1: thousands_sep-arators are optional
    TEST (T, 890123.0, "890.123", 7, 0, eofbit, 3, 0, "", "\1");
   std_log(LOG_FILENAME_LINE,"Test end") ;
   std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, 901234.0, "901.234", 7, 0, eofbit, 3, 0, "", "\2");
   std_log(LOG_FILENAME_LINE,"Test end") ;
   std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, 123450.0, "0123.45", 7, 0, eofbit, 3, 0, "", "\3");
   std_log(LOG_FILENAME_LINE,"Test end") ;
   std_log(LOG_FILENAME_LINE,"Test start");

    // invalid use of thousands_sep with empty grouping
    // the first thousands_sep terminates parsing in grouping is empty
    TEST (T,  0.0, ",",     0, 0, failbit,          0, 0, "", "\0");
   std_log(LOG_FILENAME_LINE,"Test end") ;
   std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T,  0.0, ",0",    0, 0, failbit,          0, 0, "", "\0");
   std_log(LOG_FILENAME_LINE,"Test end") ;
   std_log(LOG_FILENAME_LINE,"Test start");

    // invalid grouping
#ifndef	SAME_AS_UNIX_COMMENT       
    TEST (T,  1.0, ",1",    2, 0, eofbit | failbit, 0, 0, "", "\1");
   std_log(LOG_FILENAME_LINE,"Test end") ;
#endif   
   std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T,  2.0, "2,",    2, 0, eofbit | failbit, 0, 0, "", "\1");
   std_log(LOG_FILENAME_LINE,"Test end") ;
#ifndef	SAME_AS_UNIX_COMMENT   
   std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T,  3.0, ",3,",   3, 0, eofbit | failbit, 0, 0, "", "\1");
   std_log(LOG_FILENAME_LINE,"Test end") ;
   std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T,  4.0, ",,4",   3, 0, eofbit | failbit, 0, 0, "", "\1");
   std_log(LOG_FILENAME_LINE,"Test end") ;
   std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T,  5.0, "5,,",   3, 0, eofbit | failbit, 0, 0, "", "\1");
   std_log(LOG_FILENAME_LINE,"Test end") ;
   std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T,  6.0, ",6,,",  4, 0, eofbit | failbit, 0, 0, "", "\2");
   std_log(LOG_FILENAME_LINE,"Test end") ;
   std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T,  7.0, ",,7,",  4, 0, eofbit | failbit, 0, 0, "", "\2");
   std_log(LOG_FILENAME_LINE,"Test end") ;
   std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T,  8.0, "0,,8",  4, 0, eofbit | failbit, 0, 0, "", "\2");
   std_log(LOG_FILENAME_LINE,"Test end") ;
#endif   
   std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T,  9.0, "0,0,9", 5, 0, eofbit | failbit, 0, 0, "", "\2");
   std_log(LOG_FILENAME_LINE,"Test end") ;
   std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, 10.0, "0,10,", 5, 0, eofbit | failbit, 0, 0, "", "\2");
   std_log(LOG_FILENAME_LINE,"Test end") ;
   std_log(LOG_FILENAME_LINE,"Test start");


    // default pattern: { symbol sign none value }, e.g., "$-1"

    // parsing with a required curr_symbol, no sign, and default pattern
    TEST (T,   0.0, "$0",      2, showbase, eofbit,  0, 0, "$");
   std_log(LOG_FILENAME_LINE,"Test end") ;
   std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T,   1.0, "$1",      2, showbase, eofbit,  0, 0, "$");
   std_log(LOG_FILENAME_LINE,"Test end") ;
   std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T,   2.0, "$ 2",     3, showbase, eofbit,  0, 0, "$");
   std_log(LOG_FILENAME_LINE,"Test end") ;
   std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T,   3.0, "$  3",    4, showbase, eofbit,  0, 0, "$");
   std_log(LOG_FILENAME_LINE,"Test end") ;
   std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, 456.0, "$456",    4, showbase, eofbit,  0, 0, "$");
   std_log(LOG_FILENAME_LINE,"Test end") ;
   std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T,  56.0, "$56$",    3, showbase, goodbit, 0, 0, "$");
   std_log(LOG_FILENAME_LINE,"Test end") ;
   std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T,   6.0, "$6$7",    2, showbase, goodbit, 0, 0, "$");
   std_log(LOG_FILENAME_LINE,"Test end") ;
   std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T,   7.0, "L7",      2, showbase, eofbit,  0, 0, "L");
   std_log(LOG_FILENAME_LINE,"Test end") ;
   std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T,   8.0, "DM  8",   5, showbase, eofbit,  0, 0, "DM");
   std_log(LOG_FILENAME_LINE,"Test end") ;
   std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T,   9.0, "USD9",    4, showbase, eofbit,  0, 0, "USD");
   std_log(LOG_FILENAME_LINE,"Test end") ;
   std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T,  10.0, "YEN 10",  6, showbase, eofbit,  0, 0, "YEN");
   std_log(LOG_FILENAME_LINE,"Test end") ;
   std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T,  11.0, "$CAN11$", 6, showbase, goodbit, 0, 0, "$CAN");
   std_log(LOG_FILENAME_LINE,"Test end") ;
   std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, 121.0, "kr12.1",  6, showbase, eofbit,  1, 0, "kr");
   std_log(LOG_FILENAME_LINE,"Test end") ;
   std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, 132.0, "FT13.21", 7, showbase, eofbit,  1, 0, "FT");
   std_log(LOG_FILENAME_LINE,"Test end") ;
   std_log(LOG_FILENAME_LINE,"Test start");

    // invalid input with a required (showbase set) curr_symbol
#ifndef	SAME_AS_UNIX_COMMENT       
    TEST (T, 0.0, "FLU",    3, showbase, eofbit | failbit, 0, 0, "FLUX");
   std_log(LOG_FILENAME_LINE,"Test end") ;
#endif   
   std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, 0.0, "Fr. +1", 4, showbase, failbit, 0, 0, "Fr.");
   std_log(LOG_FILENAME_LINE,"Test end") ;
#ifndef	SAME_AS_UNIX_COMMENT      
   std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, 0.0, "Pts1",   3, showbase, failbit, 0, 0, "Pts.");
   std_log(LOG_FILENAME_LINE,"Test end") ;
   std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, 0.0, " Kn1",   0, showbase, failbit, 0, 0, "Kn");
   std_log(LOG_FILENAME_LINE,"Test end") ;
   std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, 0.0, "L.1",    2, showbase, failbit, 0, 0, "L. ");
   std_log(LOG_FILENAME_LINE,"Test end") ;
   std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, 0.0, "1",      0, showbase, failbit, 0, 0, "TL");
   std_log(LOG_FILENAME_LINE,"Test end") ;
#endif   
   std_log(LOG_FILENAME_LINE,"Test start");

    // parsing with an optional (showbase clear) curr_symbol
    TEST (T, 12.0, "12",     2, 0, eofbit, 0, 0, "mk");
   std_log(LOG_FILENAME_LINE,"Test end") ;
   std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, 13.0, "din13",  5, 0, eofbit, 0, 0, "din");
   std_log(LOG_FILENAME_LINE,"Test end") ;
   std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, 14.0, "Esc 14", 6, 0, eofbit, 0, 0, "Esc");
   std_log(LOG_FILENAME_LINE,"Test end") ;
   std_log(LOG_FILENAME_LINE,"Test start");

    // parsing with a simple non-empty sign

    static const charT plus[]  = { '+', '\0' };
    static const charT minus[] = { '-', '\0' };

    Punct<charT, false>::positive_sign_ = plus;
    Punct<charT, false>::negative_sign_ = minus;

    TEST (T,  15.0, "+15",     3, 0, eofbit,  0, 0, "");
   std_log(LOG_FILENAME_LINE,"Test end") ;
   std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T,  16.0, "+ 16",    4, 0, eofbit,  0, 0, "");
   std_log(LOG_FILENAME_LINE,"Test end") ;
   std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T,  17.0, "+  17.0", 5, 0, goodbit, 0, 0, "");
   std_log(LOG_FILENAME_LINE,"Test end") ;
   std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T,  18.0, "+  18+",  5, 0, goodbit, 0, 0, "");
   std_log(LOG_FILENAME_LINE,"Test end") ;
#if 0 // if both neg and pos signs present and no sign present at format specific place it sets fail bit   
   std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T,  19.0, "19+",     2, 0, goodbit, 0, 0, "");
   std_log(LOG_FILENAME_LINE,"Test end") ;
#endif   
   std_log(LOG_FILENAME_LINE,"Test start");

    TEST (T, -20.0, "-20",     3, 0, eofbit,  0, 0, "");
   std_log(LOG_FILENAME_LINE,"Test end") ;
   std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, -21.0, "- 21",    4, 0, eofbit,  0, 0, "");
   std_log(LOG_FILENAME_LINE,"Test end") ;
   std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, -22.0, "-  22.1", 5, 0, goodbit, 0, 0, "");
   std_log(LOG_FILENAME_LINE,"Test end") ;
   std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, -23.0, "- 23-",   4, 0, goodbit, 0, 0, "");
   std_log(LOG_FILENAME_LINE,"Test end") ;
#if 0 // if both neg and pos signs present and no sign present at format specific place it sets fail bit
   std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T,  24.0, "24+",     2, 0, goodbit, 0, 0, "");
   std_log(LOG_FILENAME_LINE,"Test end") ;
#endif   
   std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, -25.0, "-25+",    3, 0, goodbit, 0, 0, "");
   std_log(LOG_FILENAME_LINE,"Test end") ;
   std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T,  26.0, "+26-",    3, 0, goodbit, 0, 0, "");
   std_log(LOG_FILENAME_LINE,"Test end") ;
   std_log(LOG_FILENAME_LINE,"Test start");

    // invalid input
    TEST (T, 0.0, "++1.23",    1, 0, failbit, 0, 0, "");
   std_log(LOG_FILENAME_LINE,"Test end") ;
   std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, 0.0, "--2.34",    1, 0, failbit, 0, 0, "");
   std_log(LOG_FILENAME_LINE,"Test end") ;
   std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, 0.0, "+-3.45",    1, 0, failbit, 0, 0, "");
   std_log(LOG_FILENAME_LINE,"Test end") ;
   std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, 0.0, "-+4.56",    1, 0, failbit, 0, 0, "");
   std_log(LOG_FILENAME_LINE,"Test end") ;
   std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, 0.0, "- +5.67",   2, 0, failbit, 0, 0, "");
   std_log(LOG_FILENAME_LINE,"Test end") ;
#ifndef	SAME_AS_UNIX_COMMENT   
   std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, 0.0, "+",         1, 0, eofbit | failbit, 0, 0, "");
   std_log(LOG_FILENAME_LINE,"Test end") ;
   std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, 0.0, "-",         1, 0, eofbit | failbit, 0, 0, "");
   std_log(LOG_FILENAME_LINE,"Test end") ;
   std_log(LOG_FILENAME_LINE,"Test start");

    // parsing with a multi-char non-empty sign

    static const charT plus_plus[]   = { '+', '+', '\0' };
    static const charT minus_minus[] = { '-', '-', '\0' };

    Punct<charT, false>::positive_sign_ = plus_plus;
    Punct<charT, false>::negative_sign_ = minus_minus;

    TEST (T,  27.0, "+27+",    4, 0, eofbit, 0, 0, "");
   std_log(LOG_FILENAME_LINE,"Test end") ;
   std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T,  28.0, "+  28+",  6, 0, eofbit, 0, 0, "");
   std_log(LOG_FILENAME_LINE,"Test end") ;
   std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, -29.0, "-2.99-",  6, 0, eofbit, 1, 0, "");
   std_log(LOG_FILENAME_LINE,"Test end") ;
   std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, -30.0, "- 3.09-", 7, 0, eofbit, 1, 0, "");
   std_log(LOG_FILENAME_LINE,"Test end") ;
   std_log(LOG_FILENAME_LINE,"Test start");

    // with a simple currency symbol
    TEST (T,  31.0, "$+31+",    5, 0, eofbit, 0, 0, "$");
   std_log(LOG_FILENAME_LINE,"Test end") ;
   std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T,  32.0, "$+  32+",  7, 0, eofbit, 0, 0, "$");
   std_log(LOG_FILENAME_LINE,"Test end") ;
   std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, -33.0, "$-3.30-",  7, 0, eofbit, 1, 0, "$");
   std_log(LOG_FILENAME_LINE,"Test end") ;
   std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, -34.0, "$- 3.41-", 8, 0, eofbit, 1, 0, "$");
   std_log(LOG_FILENAME_LINE,"Test end") ;
   std_log(LOG_FILENAME_LINE,"Test start");

    // with a multi-char currency symbol
    TEST (T,  35.0, "USD+35+",     7, 0, eofbit, 0, 0, "USD");
   std_log(LOG_FILENAME_LINE,"Test end") ;
   std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T,  36.0, "USD+  36+",   9, 0, eofbit, 0, 0, "USD");
   std_log(LOG_FILENAME_LINE,"Test end") ;
   std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, -37.0, "USD-3.70-",   9, 0, eofbit, 1, 0, "USD");
   std_log(LOG_FILENAME_LINE,"Test end") ;
   std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, -38.0, "USD- 3.81-", 10, 0, eofbit, 1, 0, "USD");
   std_log(LOG_FILENAME_LINE,"Test end") ;
   std_log(LOG_FILENAME_LINE,"Test start");

    // invalid input
    TEST (T, 0.0, "+31 +",     3, 0, failbit, 0, 0, "");
   std_log(LOG_FILENAME_LINE,"Test end") ;
   std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, 0.0, "+  32 +",   5, 0, failbit, 0, 0, "");
   std_log(LOG_FILENAME_LINE,"Test end") ;
   std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, 0.0, "-33.9 -",   3, 0, failbit, 0, 0, "");
   std_log(LOG_FILENAME_LINE,"Test end") ;
   std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, 0.0, "- 3.49 -",  6, 0, failbit, 1, 0, "");
   std_log(LOG_FILENAME_LINE,"Test end") ;
   std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, 0.0, "$+35 +",    4, 0, failbit, 0, 0, "$");
   std_log(LOG_FILENAME_LINE,"Test end") ;
   std_log(LOG_FILENAME_LINE,"Test start");
    // the space below is read since the optional sign is considered missing
    TEST (T, 0.0, "$ +36+",    2, 0, failbit, 0, 0, "$");
   std_log(LOG_FILENAME_LINE,"Test end") ;
   std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, 0.0, "$+  37 +",  6, 0, failbit, 0, 0, "$");
   std_log(LOG_FILENAME_LINE,"Test end") ;
   std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, 0.0, "$-3.80 -",  3, 0, failbit, 0, 0, "$");
   std_log(LOG_FILENAME_LINE,"Test end") ;
   std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, 0.0, "$- 3.91 -", 7, 0, failbit, 1, 0, "$");
   std_log(LOG_FILENAME_LINE,"Test end") ;
   std_log(LOG_FILENAME_LINE,"Test start");

    static const charT plus_minus[] = { '+', '-', '\0' };
    Punct<charT, false>::positive_sign_ = plus_plus;
    Punct<charT, false>::negative_sign_ = plus_minus;

    // 22.2.6.1.2, p3: if the first character of positive
    // and negative sign is the same, the result is positive
    TEST (T, 40.0,   "+40+", 4, 0,        eofbit,  0, 0, "");
   std_log(LOG_FILENAME_LINE,"Test end") ;
   std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, 41.0,   "+41-", 4, 0,        eofbit,  0, 0, "");
   std_log(LOG_FILENAME_LINE,"Test end") ;
   std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, 42.0,  "+42+ ", 4, 0,        goodbit, 0, 0, "");
   std_log(LOG_FILENAME_LINE,"Test end") ;
   std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, 43.0,  "+43- ", 4, 0,        goodbit, 0, 0, "");
   std_log(LOG_FILENAME_LINE,"Test end") ;
   std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, 44.0, "+44 $+", 6, showbase, eofbit,  0, "\3\4\0\2", "$");
   std_log(LOG_FILENAME_LINE,"Test end") ;
   std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, 45.0, "+45 $-", 6, showbase, eofbit,  0, "\3\4\0\2", "$");
   std_log(LOG_FILENAME_LINE,"Test end") ;
   std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, 46.0, "+46$+ ", 5, showbase, goodbit, 0, "\3\4\2\0", "$");
   std_log(LOG_FILENAME_LINE,"Test end") ;
   std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, 47.0, "+47$- ", 5, showbase, goodbit, 0, "\3\4\2\0", "$");
   std_log(LOG_FILENAME_LINE,"Test end") ;
#endif   
   std_log(LOG_FILENAME_LINE,"Test start");

    // none = 0, space = 1, symbol = 2, sign = 3, value = 4

    // test various patterns
    Punct<charT, false>::positive_sign_ = plus;
    Punct<charT, false>::negative_sign_ = minus;

    TEST (T,  -100.0, "$-1",   3, showbase, eofbit,  2, "\2\3\4\0", "$");
   std_log(LOG_FILENAME_LINE,"Test end") ;
#ifndef	SAME_AS_UNIX_COMMENT      
   std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T,  -200.0, "$-2 ",  3, showbase, goodbit, 2, "\2\3\4\0", "$");
   std_log(LOG_FILENAME_LINE,"Test end") ;
#else  
	static const charT plus_plus[]   = { '+', '+', '\0' };
#endif
   std_log(LOG_FILENAME_LINE,"Test start");
    // where none appears in the pattern, whitespace is optional
    TEST (T,  -300.0, "-3$",   3, showbase, eofbit,  2, "\3\4\0\2", "$");
   std_log(LOG_FILENAME_LINE,"Test end") ;
   std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T,  -400.0, "-4 $",  4, showbase, eofbit,  2, "\3\4\0\2", "$");
   std_log(LOG_FILENAME_LINE,"Test end") ;
   std_log(LOG_FILENAME_LINE,"Test start");
    // where space appears in the pattern, a whitespace character is required
    TEST (T,    0.0, "-5$",    2, showbase, failbit, 2, "\3\4\1\2", "$");
   std_log(LOG_FILENAME_LINE,"Test end") ;
   std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T,  -600.0, "-6 $",  4, showbase, eofbit,  2, "\3\4\1\2", "$");
   std_log(LOG_FILENAME_LINE,"Test end") ;
   std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T,  -700.0, "-7\t$", 4, showbase, eofbit,  2, "\3\4\1\2", "$");
   std_log(LOG_FILENAME_LINE,"Test end") ;
   std_log(LOG_FILENAME_LINE,"Test start");

    TEST (T,     0.0, "-8$",   1, showbase, failbit, 2, "\3\1\4\2", "$");
   std_log(LOG_FILENAME_LINE,"Test end") ;
   std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T,  -900.0, "-\n9$", 4, showbase, eofbit,  2, "\3\1\4\2", "$");
   std_log(LOG_FILENAME_LINE,"Test end") ;
   std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, -1000.0, "- 10$", 5, showbase, eofbit,  2, "\3\1\4\2", "$");
   std_log(LOG_FILENAME_LINE,"Test end") ;
   std_log(LOG_FILENAME_LINE,"Test start");
    // white space is required even at the beginning of input
    // (if the optional sign or symbol is missing)
    TEST (T,     0.0, "11$",   0, showbase, failbit, 2, "\3\1\4\2", "$");
   std_log(LOG_FILENAME_LINE,"Test end") ;
   std_log(LOG_FILENAME_LINE,"Test start");

    TEST (T, -1200.0, "12$-",  4, showbase, eofbit,  2, "\4\0\2\3", "$");
   std_log(LOG_FILENAME_LINE,"Test end") ;
   std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, -1300.0, "13 $-", 5, showbase, eofbit,  2, "\4\0\2\3", "$");
   std_log(LOG_FILENAME_LINE,"Test end") ;
   std_log(LOG_FILENAME_LINE,"Test start");
#ifndef	SAME_AS_UNIX_COMMENT        
    TEST (T,     0.0, "14$-",  2, showbase, failbit, 2, "\4\1\2\3", "$");
   std_log(LOG_FILENAME_LINE,"Test end") ;
   std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, -1500.0, "15 $-", 5, showbase, eofbit,  2, "\4\1\2\3", "$");
   std_log(LOG_FILENAME_LINE,"Test end") ;
#endif      
   std_log(LOG_FILENAME_LINE,"Test start");

    const charT parens[] = { '(', ')', '\0' };
    Punct<charT, false>::negative_sign_ = parens;
#ifndef	SAME_AS_UNIX_COMMENT        
    // { sign, space, value, symbol }, with symbol required
    TEST (T, -90.0, "( 9$)",    5, showbase, eofbit, 1, "\3\1\4\2", "$");
   std_log(LOG_FILENAME_LINE,"Test end") ;
   std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, -80.0, "( 8SIT)",  7, showbase, eofbit, 1, "\3\1\4\2", "SIT");
   std_log(LOG_FILENAME_LINE,"Test end") ;
   std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, -81.0, "( 8.1Fr)", 8, showbase, eofbit, 1, "\3\1\4\2", "Fr");
   std_log(LOG_FILENAME_LINE,"Test end") ;
   std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T,  70.0, " 7Ptas",   6, showbase, eofbit, 1, "\3\1\4\2", "Ptas");
   std_log(LOG_FILENAME_LINE,"Test end") ;
   std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T,  61.0, " 6.1Lek",  7, showbase, eofbit, 1, "\3\1\4\2", "Lek");
   std_log(LOG_FILENAME_LINE,"Test end") ;
   std_log(LOG_FILENAME_LINE,"Test start");

    // { sign, value, none, symbol }
    TEST (T,  101.0, "101    ", 3, 0,        goodbit, 0, "\3\4\0\2", "");
   std_log(LOG_FILENAME_LINE,"Test end") ;
   std_log(LOG_FILENAME_LINE,"Test start");
    // trailing space not extracted since currency symbol is optional
    TEST (T,  102.0, "102 ", 3, 0, goodbit, 0, "\3\4\0\2", " ");
   std_log(LOG_FILENAME_LINE,"Test end") ;
#endif      
   std_log(LOG_FILENAME_LINE,"Test start");
    // all spaces extracted since currency symbol (last) is mandatory
    // verify that the facet doesn't extract too many optional spaces
    // leaving none for the final required currency symbol
#ifndef	SAME_AS_UNIX_COMMENT        
    TEST (T,  103.0, "103 ", 4, showbase, eofbit, 0, "\3\4\0\2", " ");
   std_log(LOG_FILENAME_LINE,"Test end") ;
   std_log(LOG_FILENAME_LINE,"Test start");

    // 22.2.6.1.2, p3: [Example: If showbase is off, then for
    // a negative sign value of "()" and a currency symbol of "L",
    // in "(100 L)" the "L" is consumed; but if negative sign is "�",
    // the "L" in "�100 L" is not consumed. ]
    TEST (T, -104.0, "(104 L)", 7, 0,        eofbit,  0, "\3\4\0\2", "L");
   std_log(LOG_FILENAME_LINE,"Test end") ;
   std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, -105.0, "(105 L)", 7, showbase, eofbit,  0, "\3\4\0\2", "L");
   std_log(LOG_FILENAME_LINE,"Test end") ;
   std_log(LOG_FILENAME_LINE,"Test start");

    Punct<charT, false>::positive_sign_ = plus;
    Punct<charT, false>::negative_sign_ = minus;

    // { sign, value, none, symbol }
    // trailing optional curr_symbol or whitespace preceding it (regardless
    // of whether the latter is required or optional) is not consumed)
    TEST (T, -101.0, "-101 $", 4, 0,        goodbit, 0, "\3\4\0\2", "$");
   std_log(LOG_FILENAME_LINE,"Test end") ;
#else
	    Punct<charT, false>::positive_sign_ = plus;
    	Punct<charT, false>::negative_sign_ = minus;

#endif  
   std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, -102.0, "-102 $", 6, showbase, eofbit, 0, "\3\4\0\2", "$");
   std_log(LOG_FILENAME_LINE,"Test end") ;
#ifndef	SAME_AS_UNIX_COMMENT           
   std_log(LOG_FILENAME_LINE,"Test start");

    TEST (T, -103.0, "-103 $", 4, 0,        goodbit, 0, "\3\4\1\2", "$");
   std_log(LOG_FILENAME_LINE,"Test end") ;
#endif   
   std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, -104.0, "-104 $", 6, showbase, eofbit, 0, "\3\4\1\2", "$");
   std_log(LOG_FILENAME_LINE,"Test end") ;
#ifndef	SAME_AS_UNIX_COMMENT              
   std_log(LOG_FILENAME_LINE,"Test start");

    // { sign, value, symbol, none }
    TEST (T, -105.0, "-105 $", 4, 0,        goodbit, 0, "\3\4\2\0", "$");
   std_log(LOG_FILENAME_LINE,"Test end") ;
   std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, -106.0, "-106$ ", 5, showbase, goodbit, 0, "\3\4\2\0", "$");
   std_log(LOG_FILENAME_LINE,"Test end") ;
   std_log(LOG_FILENAME_LINE,"Test start");

    // { value, none, sign, symbol }
    // trailing whitespace may need to be extracted if characters may
    // be needed to complete the pattern (e.g., sign or symbol)
    TEST (T,  107.0, "107 $",  4, 0,        goodbit, 0, "\4\0\3\2", "$");
   std_log(LOG_FILENAME_LINE,"Test end") ;
   std_log(LOG_FILENAME_LINE,"Test start");
    // trailing optional curr_symbol is not consumed
    TEST (T, -108.0, "108-$",  4, 0,        goodbit, 0, "\4\0\3\2", "$");
   std_log(LOG_FILENAME_LINE,"Test end") ;
   std_log(LOG_FILENAME_LINE,"Test start");
    // trailing required curr_symbol is consumed
    TEST (T, -109.0, "109-$",  5, showbase, eofbit,  0, "\4\0\3\2", "$");
   std_log(LOG_FILENAME_LINE,"Test end") ;
   std_log(LOG_FILENAME_LINE,"Test start");

    const charT minus_space[] = { '-', ' ', '\0' };
    Punct<charT, false>::negative_sign_ = minus_space;

    // { sign, value, none, symbol }
    // negative_sign is "- ", (note the single trailing space)
    // verify that the extraction suceeds, i.e., that the money_base::none
    // specifier that's last in the pattern doesn't confuse the facet into
    // extracting all the optional whitespace, leaving none to complete
    // the negative_sign
    TEST (T, -109.1, "-109  ", 6, 0, eofbit, 0, "\3\4\0\2", "");
   std_log(LOG_FILENAME_LINE,"Test end") ;
#else
   	const charT minus_space[] = { '-', ' ', '\0' };
    Punct<charT, false>::negative_sign_ = minus_space;
#endif        
   std_log(LOG_FILENAME_LINE,"Test start");

    // verify that optional space after value and before currency
    // symbol is treated correctly
    Punct<charT, false>::positive_sign_ = plus_plus;
    Punct<charT, false>::negative_sign_ = 0;

    // { sign, value, none, symbol }
    TEST (T,  1090.0, "1090 $",   6, showbase, eofbit,  0, "\3\4\0\2", "$");
   std_log(LOG_FILENAME_LINE,"Test end") ;
   std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T,  1091.0, "1091 $",   6, showbase, eofbit,  0, "\3\4\0\2", "$");
   std_log(LOG_FILENAME_LINE,"Test end") ;
 #ifndef	SAME_AS_UNIX_COMMENT   
   std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, +1092.0, "+1092 $+", 8, showbase, eofbit,  0, "\3\4\0\2", "$");
   std_log(LOG_FILENAME_LINE,"Test end") ;
#endif   
   std_log(LOG_FILENAME_LINE,"Test start");

    Punct<charT, false>::positive_sign_ = 0;
    Punct<charT, false>::negative_sign_ = parens;

    TEST (T,  1093.0, "1093 $",   6, showbase, eofbit,  0, "\3\4\0\2", "$");
   std_log(LOG_FILENAME_LINE,"Test end") ;
   std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T,  1094.0, "1094 $",   6, showbase, eofbit,  0, "\3\4\0\2", "$");
   std_log(LOG_FILENAME_LINE,"Test end") ;
#ifndef	SAME_AS_UNIX_COMMENT     
   std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, -1095.0, "(1095 $)", 8, showbase, eofbit,  0, "\3\4\0\2", "$");
   std_log(LOG_FILENAME_LINE,"Test end") ;
   std_log(LOG_FILENAME_LINE,"Test start");

    // verify a single-char sign with a multichar one (see bug #428)
    Punct<charT, false>::positive_sign_ = plus;
    Punct<charT, false>::negative_sign_ = parens;

    // { sign, value, none, symbol }
    TEST (T,  1096.0, "1096 $",   6, showbase, eofbit,  0, "\3\4\0\2", "$");
   std_log(LOG_FILENAME_LINE,"Test end") ;
   std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T,  1097.0, "1097 $",   6, showbase, eofbit,  0, "\3\4\0\2", "$");
   std_log(LOG_FILENAME_LINE,"Test end") ;
   std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, +1098.0, "+1098 $",  7, showbase, eofbit,  0, "\3\4\0\2", "$");
   std_log(LOG_FILENAME_LINE,"Test end") ;
   std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, -1099.0, "(1099 $)", 8, showbase, eofbit,  0, "\3\4\0\2", "$");
   std_log(LOG_FILENAME_LINE,"Test end") ;
   std_log(LOG_FILENAME_LINE,"Test start");

    Punct<charT, false>::positive_sign_ = 0;
    Punct<charT, false>::negative_sign_ = 0;

    // { value, none, symbol, sign }
    // trailing whitespace is not extracted (even if required by the format)
    // if it can be determined that no other characters can possibly complete
    // the format (e.g., if both signs and symbol are empty or if signs are
    // empty and symbol is optional)
    TEST (T,  110.0, "110 $",  3, 0,        goodbit, 0, "\4\0\3\2", "$");
   std_log(LOG_FILENAME_LINE,"Test end") ;
   std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T,  111.0, "111 $",  3, 0,        goodbit, 0, "\4\1\3\2", "$");
   std_log(LOG_FILENAME_LINE,"Test end") ;
#else
	
    Punct<charT, false>::positive_sign_ = 0;
    Punct<charT, false>::negative_sign_ = 0; 
#endif    
   std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T,  112.0, "112$",   4, showbase, eofbit,  0, "\4\0\3\2", "$");
   std_log(LOG_FILENAME_LINE,"Test end") ;
   std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T,  113.0, "113$",   4, showbase, eofbit,  0, "\4\0\3\2", "$");
   std_log(LOG_FILENAME_LINE,"Test end") ;
   std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T,  114.0, "114 $",  5, showbase, eofbit,  0, "\4\1\3\2", "$");
   std_log(LOG_FILENAME_LINE,"Test end") ;
   std_log(LOG_FILENAME_LINE,"Test start");
#if 0 // Unix taking by default the format "\2\3\0\4", not reading from derived class
    TEST (T,  115.0, "115 $",  3, 0,        goodbit, 0, "\4\0\2\3", "$");
   std_log(LOG_FILENAME_LINE,"Test end") ;
   std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T,  116.0, "116 $",  3, 0,        goodbit, 0, "\4\1\2\3", "$");
   std_log(LOG_FILENAME_LINE,"Test end") ;
#endif   
   std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T,  117.0, "117$",   4, showbase, eofbit,  0, "\4\0\2\3", "$");
   std_log(LOG_FILENAME_LINE,"Test end") ;
   std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T,  118.0, "118$",   4, showbase, eofbit,  0, "\4\0\2\3", "$");
   std_log(LOG_FILENAME_LINE,"Test end") ;
   std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T,  119.0, "119 $",  5, showbase, eofbit,  0, "\4\1\2\3", "$");
   std_log(LOG_FILENAME_LINE,"Test end") ;
   std_log(LOG_FILENAME_LINE,"Test start");


    Punct<charT, false>::positive_sign_ = plus;
    Punct<charT, false>::negative_sign_ = minus;
    Punct<charT, false>::thousands_sep_ = ';';
#ifndef	SAME_AS_UNIX_COMMENT    
    // test grouping, default pattern { symbol sign none value }
    TEST (T, 0.0, "$0",     2, 0, eofbit, 0, 0, "$", "\1");
   std_log(LOG_FILENAME_LINE,"Test end") ;
   std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, 0.0, "$0;0",   4, 0, eofbit, 0, 0, "$", "\1");
   std_log(LOG_FILENAME_LINE,"Test end") ;
   std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, 0.0, "$0;0;0", 6, 0, eofbit, 0, 0, "$", "\1");
   std_log(LOG_FILENAME_LINE,"Test end") ;
#endif   
   std_log(LOG_FILENAME_LINE,"Test start");

    TEST (T, 0.0, "$+0",     3, 0, eofbit, 0, 0, "$", "\1");
   std_log(LOG_FILENAME_LINE,"Test end") ;
   std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, 0.0, "$+0;0",   5, 0, eofbit, 0, 0, "$", "\1");
   std_log(LOG_FILENAME_LINE,"Test end") ;
   std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, 0.0, "$+0;0;0", 7, 0, eofbit, 0, 0, "$", "\1");
   std_log(LOG_FILENAME_LINE,"Test end") ;
   std_log(LOG_FILENAME_LINE,"Test start");

    TEST (T, 0.0, "$-0.0",        5, 0, eofbit, 1, 0, "$", "\1");
   std_log(LOG_FILENAME_LINE,"Test end") ;
   std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, 0.0, "$-0;0.00",     8, 0, eofbit, 2, 0, "$", "\1");
   std_log(LOG_FILENAME_LINE,"Test end") ;
   std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, 0.0, "$-0;0;0.000", 11, 0, eofbit, 3, 0, "$", "\1");
   std_log(LOG_FILENAME_LINE,"Test end") ;
   std_log(LOG_FILENAME_LINE,"Test start");

    TEST (T,     -10.0, "$-1.",           4, 0, eofbit, 1, 0, "$", "\1");
   std_log(LOG_FILENAME_LINE,"Test end") ;
   std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T,   -2000.0, "$-2;0.0000",    10, 0, eofbit, 2, 0, "$", "\1");
   std_log(LOG_FILENAME_LINE,"Test end") ;
   std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, -300000.0, "$-3;0;0.00000", 13, 0, eofbit, 3, 0, "$", "\1");
   std_log(LOG_FILENAME_LINE,"Test end") ;
   std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, -300001.0, "$-3;0;0.00123", 13, 0, eofbit, 3, 0, "$", "\1");
   std_log(LOG_FILENAME_LINE,"Test end") ;
#ifndef	SAME_AS_UNIX_COMMENT       
   std_log(LOG_FILENAME_LINE,"Test start");

    // invalid grouping: store value and set failbit
    TEST (T, 0.0, "$0;",   3, 0, eofbit | failbit, 0, 0, "$", "\1");
   std_log(LOG_FILENAME_LINE,"Test end") ;
   std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, 0.0, "$;0;0", 5, 0, eofbit | failbit, 0, 0, "$", "\1");
   std_log(LOG_FILENAME_LINE,"Test end") ;
   std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, 0.0, "$0;;0", 5, 0, eofbit | failbit, 0, 0, "$", "\1");
   std_log(LOG_FILENAME_LINE,"Test end") ;
   std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, 0.0, "$;.0",  4, 0, eofbit | failbit, 1, 0, "$", "\1");
   std_log(LOG_FILENAME_LINE,"Test end") ;
   std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, 0.0, "$;;.0", 5, 0, eofbit | failbit, 1, 0, "$", "\1");
   std_log(LOG_FILENAME_LINE,"Test end") ;
   std_log(LOG_FILENAME_LINE,"Test start");
    // decimal_point alone is not a valid number
    TEST (T, 0.0, "$.;1",  2, 0, failbit,          1, 0, "$", "\1");
   std_log(LOG_FILENAME_LINE,"Test end") ;
   std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, 0.0, "$;.;2", 3, 0, failbit,          1, 0, "$", "\1");
   std_log(LOG_FILENAME_LINE,"Test end") ;
#endif   
   std_log(LOG_FILENAME_LINE,"Test start");

    TEST (T, 0.0, "$+0;",   4, 0, eofbit | failbit, 0, 0, "$", "\1");
   std_log(LOG_FILENAME_LINE,"Test end") ;
#ifndef	SAME_AS_UNIX_COMMENT          
   std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, 0.0, "$+;0;0", 6, 0, eofbit | failbit, 0, 0, "$", "\1");
   std_log(LOG_FILENAME_LINE,"Test end") ;
   std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, 0.0, "$+0;;0", 6, 0, eofbit | failbit, 0, 0, "$", "\1");
   std_log(LOG_FILENAME_LINE,"Test end") ;
   std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, 0.0, "$+;.0;", 5, 0, failbit,          1, 0, "$", "\1");
   std_log(LOG_FILENAME_LINE,"Test end") ;
 #endif  
   std_log(LOG_FILENAME_LINE,"Test start");

    TEST (T,     -40.0, "$-4;. ",           5, 0, failbit, 1, 0, "$", "\1");
   std_log(LOG_FILENAME_LINE,"Test end") ;
#ifndef	SAME_AS_UNIX_COMMENT             
   std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T,   -5000.0, "$-;5;0.0000 ",    11, 0, failbit, 2, 0, "$", "\1");
   std_log(LOG_FILENAME_LINE,"Test end") ;
   std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, -600000.0, "$-6;0;;0.00000 ", 14, 0, failbit, 3, 0, "$", "\1");
   std_log(LOG_FILENAME_LINE,"Test end") ;
   std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, -700004.0, "$-7;;0;0.00456 ", 14, 0, failbit, 3, 0, "$", "\1");
   std_log(LOG_FILENAME_LINE,"Test end") ;
   

#ifndef _RWSTD_NO_LONG_DOUBLE
#  define LDBL(x)   x ## L
#else
#  define LDBL(x)   x
#endif
   std_log(LOG_FILENAME_LINE,"Test start");
    TEST (T, LDBL (1234567890.0),
          "$1;2;3;4;5;6;7;8;9;0", 20, 0, eofbit, 0, 0, "$", "\1");
   std_log(LOG_FILENAME_LINE,"Test end") ;
   std_log(LOG_FILENAME_LINE,"Test start");

    TEST (T, LDBL (1234567890.0),
          "$0;1;2;3;4;5;6;7;8;9;0", 22, 0, eofbit, 0, 0, "$", "\1");
   std_log(LOG_FILENAME_LINE,"Test end") ;
   std_log(LOG_FILENAME_LINE,"Test start");

    TEST (T, LDBL (1234567890.0),
          "$0;0;1;2;3;4;5;6;7;8;9;0", 24, 0, eofbit, 0, 0, "$", "\1");
   std_log(LOG_FILENAME_LINE,"Test end") ;
#else
#ifndef _RWSTD_NO_LONG_DOUBLE
#  define LDBL(x)   x ## L
#else
#  define LDBL(x)   x
#endif   
#endif   
   std_log(LOG_FILENAME_LINE,"Test start");

    TEST (T, LDBL (1234567890.0),
          "$+1;2;3;4;5;6;7;8;9;0", 21, 0, eofbit, 0, 0, "$", "\1");
   std_log(LOG_FILENAME_LINE,"Test end") ;
   std_log(LOG_FILENAME_LINE,"Test start");

    TEST (T, LDBL (1234567890.0),
          "$+0;1;2;3;4;5;6;7;8;9;0", 23, 0, eofbit, 0, 0, "$", "\1");
   std_log(LOG_FILENAME_LINE,"Test end") ;
   std_log(LOG_FILENAME_LINE,"Test start");

    TEST (T, LDBL (1234567890.0),
          "$+0;0;1;2;3;4;5;6;7;8;9;0", 25, 0, eofbit, 0, 0, "$", "\1");
   std_log(LOG_FILENAME_LINE,"Test end") ;
   std_log(LOG_FILENAME_LINE,"Test start");

    TEST (T, LDBL (1234567890.0),
          "$+0;1;2;3;4;5;6;7;8;9;0", 23, 0, eofbit, 0, 0, "$", "\1");
   std_log(LOG_FILENAME_LINE,"Test end") ;
   std_log(LOG_FILENAME_LINE,"Test start");

    TEST (T, LDBL (-1234567890.0),
          "$-1;2;3;4;5;6;7;8;9;0", 21, 0, eofbit, 0, 0, "$", "\1");
   std_log(LOG_FILENAME_LINE,"Test end") ;
   std_log(LOG_FILENAME_LINE,"Test start");

    TEST (T, LDBL (-1234567890.0),
          "$-0;1;2;3;4;5;6;7;8;9;0", 23, 0, eofbit, 0, 0, "$", "\1");
   std_log(LOG_FILENAME_LINE,"Test end") ;
   std_log(LOG_FILENAME_LINE,"Test start");

    TEST (T, LDBL (-1234567890.0),
          "$-0;0;1;2;3;4;5;6;7;8;9;0", 25, 0, eofbit, 0, 0, "$", "\1");
   std_log(LOG_FILENAME_LINE,"Test end") ;
#ifndef	SAME_AS_UNIX_COMMENT     
   std_log(LOG_FILENAME_LINE,"Test start");


    TEST (T, LDBL (1234567890.0),
          "$12;34;56;78;90", 15, 0, eofbit, 0, 0, "$", "\2");
   std_log(LOG_FILENAME_LINE,"Test end") ;
#endif   
   std_log(LOG_FILENAME_LINE,"Test start");

    TEST (T, LDBL (1234567890.0),
          "$+12;34;56;78;90", 16, 0, eofbit, 0, 0, "$", "\2");
   std_log(LOG_FILENAME_LINE,"Test end") ;
   std_log(LOG_FILENAME_LINE,"Test start");

    TEST (T, LDBL (-1234567890.0),
          "$-12;34;56;78;90", 16, 0, eofbit, 0, 0, "$", "\2");
   std_log(LOG_FILENAME_LINE,"Test end") ;


#undef T

}

/**************************************************************************/

template <class charT>
void
test_long_double (charT, const char *cname)
{
    if (rw_enabled ("long double"))
        test_memfun (charT (0 /* long double argument */),
                     cname, "long double");
    else
        rw_note (0, __FILE__, __LINE__, "long double test disabled");
}

/**************************************************************************/

template <class charT>
void
test_string (charT, const char *cname)
{
    if (rw_enabled ("basic_string"))
        test_memfun (charT (1 /* basic_string argument */),
                     cname, "basic_string");
    else
        rw_note (0, __FILE__, __LINE__,
                 "basic_string<%s> test disabled", cname);
}

/**************************************************************************/

static int
run_test (int, char*[])
{
    // check the prescribed values of the required constants
    rw_fatal ('\0' == std::money_base::none, 0, 0,
              "'\\0' == money_base::none, got %d", std::money_base::none);

    rw_fatal ('\1' == std::money_base::space, 0, 0,
              "'\\1' == money_base::space, got %d", std::money_base::space);

    rw_fatal ('\2' == std::money_base::symbol, 0, 0,
              "'\\2' == money_base::symbol, got %d", std::money_base::symbol);

    rw_fatal ('\3' == std::money_base::sign, 0, 0,
              "'\\3' == money_base::sign, got %d", std::money_base::sign);

    rw_fatal ('\4' == std::money_base::value, 0, 0,
              "'\\4' == money_base::value, got %d", std::money_base::value);
               
    if (rw_enabled ("char")) {
        test_long_double (char (), "char");
        test_string (char (), "char");
    }
    else
        rw_note (0, __FILE__, __LINE__, "char test disabled");

#ifndef _RWSTD_NO_WCHAR_T

    if (rw_enabled ("wchar_t")) {
        test_long_double (wchar_t (), "wchar_t");
        test_string (wchar_t (), "wchar_t");
    }
    else
        rw_note (0, __FILE__, __LINE__, "wchar_t test disabled");

#endif   // _RWSTD_NO_WCHAR_T

    return 0;
}

/**************************************************************************/

int main (int argc, char* argv [])
{
   #ifndef __SYMBIAN32__
    return rw_test (argc, argv, __FILE__,
                    "lib.locale.money.get",
                    0 /* no comment */, run_test,
                    "|-no-basic_string#",
                    &no_basic_string);
   #else
     rw_test (argc, argv, __FILE__,
                    "lib.locale.money.get",
                    0 /* no comment */, run_test,
                    "|-no-basic_string#",
                    &no_basic_string);               
                    
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
   
   testResultXml("22_locale_money_get");
    close_log_file();
 return 0;     
   #endif    
}
