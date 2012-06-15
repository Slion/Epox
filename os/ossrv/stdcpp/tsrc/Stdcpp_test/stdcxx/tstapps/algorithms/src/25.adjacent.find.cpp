

/***************************************************************************
 *
 * adjacent_find.cpp - test exercising 25.1.5 [lib.alg.adjacent.find]
 *
 * $Id: 25.adjacent.find.cpp 351771 2005-12-02 16:55:51Z sebor $
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

#include <algorithm>   // for adjacent_find
#include <functional>  // for equal_to
#include <cstddef>     // for size_t

#include <alg_test.h>
#include <driver.h>
#include"std_log_result.h"
#define LOG_FILENAME_LINE __FILE__, __LINE__

/**************************************************************************/

// used to initialize an array of objects of type X
static const char *xinit_begin;
 int failures=0;
int xinit ()
{
    typedef unsigned char UChar;

    static const char *cur = 0;

    if (!cur || !*cur)
        cur = xinit_begin;

    return UChar (*cur++);
}


// exercises std::adjacent_find()
template <class ForwardIterator, class T>
void do_test (int         line,     // line number of test case
              const char *src,      // source sequence
              std::size_t resoff,   // offset of result
              ForwardIterator    dummy_iter,
              const T*,
              const char* predicate)
{
    static const char* const itname = type_name (dummy_iter, (T*)0);

   #ifdef __SYMBIAN32__
     const size_t nsrc = strlen (src);
   #else  
    const std::size_t nsrc = std::strlen (src);
   #endif //__SYMBIAN32__
     
    if (std::size_t (-1) == resoff)
        resoff = nsrc;

    // have the X default ctor initialize objects from `src'
    xinit_begin = src;
    #ifndef __SYMBIAN32__
     X::gen_     = xinit;
    #else
     gen_     = xinit;
    #endif 

    X* const xsrc = new X [nsrc];

    const ForwardIterator first =
        make_iter (xsrc, xsrc, xsrc + nsrc, dummy_iter);

    const ForwardIterator last =
        make_iter (xsrc + nsrc, xsrc, xsrc + nsrc, dummy_iter);

    // compute the number of invocations of the predicate
    #ifndef __SYMBIAN32__
     std::size_t n_total_pred = X::n_total_op_eq_;
    #else
    std::size_t n_total_pred = n_total_op_eq_;
    #endif 
    const ForwardIterator res =
        predicate ? std::adjacent_find (first, last, std::equal_to<X>())
                  : std::adjacent_find (first, last);

    // silence a bogus EDG eccp remark #550-D:
    // variable "res" was set but never used
    _RWSTD_UNUSED (res);
    
    #ifndef __SYMBIAN32__
    n_total_pred =X::n_total_op_eq_ - n_total_pred;
    #else
    n_total_pred = n_total_op_eq_ - n_total_pred;
    #endif
    // verify that the returned iterator is set as expected
    int success = res.cur_ == first.cur_ + resoff;
    
    if(!success)
      {
        failures++;
       std_log(LOG_FILENAME_LINE,"Reason: Failing as expected iterator is not returned"); 
      }
       
   
    rw_assert (success, 0, line, 
               "line %d: adjacent_find<%s>(it = \"%s\", ...)"
               " == (it + %zu), got (it + %td)",
               __LINE__, itname, src,
               resoff, res.cur_ - first.cur_);

    // verify the number of applications of the predicate (lwg issue 240):
    // Complexity: For a nonempty range, exactly
    //             min((i - first) + 1, (last - first) - 1)
    // applications of the corresponding predicate, where i is
    // adjacent_find's return value.

    // compute the expected number of invocations of the predicate
    std::size_t n_expect_pred = 0;

    if (nsrc) {
        // test iterators are guaranteed to be in range
        _RWSTD_ASSERT (first.cur_ <= res.cur_ && res.cur_ <= last.cur_);

        n_expect_pred = std::size_t (res.cur_ - first.cur_) + 1;
        const std::size_t tmp = std::size_t (last.cur_ - first.cur_) - 1;
        if (tmp < n_expect_pred)
            n_expect_pred = tmp;
    }

    success = std::size_t (n_expect_pred) == n_total_pred;
    
    if(!success)
     {
       failures ++;
       std_log(LOG_FILENAME_LINE,"Reason: Failing as iteratoris not in range");
      }
    rw_assert (success, 0, line, 
               "line %d: adjacent_find<%s>(\"%s\", ...) "
               "invoked %s %zu times, expected %td",
               __LINE__, itname, src,
               predicate ? predicate : "operator==()", 
               n_total_pred, n_expect_pred);
}

/**************************************************************************/

template <class ForwardIterator, class T>
void run_tests (ForwardIterator dummy_iter, const T*, const char* predicate)
{
    static const char* const itname = type_name (dummy_iter, (T*)0);

    rw_info (0, 0, 0, "std::adjacent_find (%s, %1$s%{?}, %s%{;})", 
             itname, 0 != predicate, predicate);
    
#define TEST(src, off) \
    do_test (__LINE__, src, std::size_t (off), dummy_iter, (X*)0, predicate)

    //    +------------------ subject sequence
    //    |               +-- offset of returned iterator,
    //    |               |   -1 denotes the end of sequence
    //    v               v
    TEST ("",            -1);
    if(failures)
      {
       std_log(LOG_FILENAME_LINE,"Failing");
      }
    TEST ("a",           -1);
    if(failures)
      {
       std_log(LOG_FILENAME_LINE,"Failing");
      }
    TEST ("ab",          -1);
    if(failures)
      {
       std_log(LOG_FILENAME_LINE,"Failing");
      }
    TEST ("abc",         -1);
    if(failures)
      {
       std_log(LOG_FILENAME_LINE,"Failing");
      }

    TEST ("abcd",        -1);
    if(failures)
      {
       std_log(LOG_FILENAME_LINE,"Failing");
      }

    TEST ("abcde",       -1);
    if(failures)
      {
       std_log(LOG_FILENAME_LINE,"Failing");
      }

    TEST ("abcdef",      -1);
    if(failures)
      {
       std_log(LOG_FILENAME_LINE,"Failing");
      }

    TEST ("abcdefg",     -1);
    if(failures)
      {
       std_log(LOG_FILENAME_LINE,"Failing");
      }

    TEST ("abcdefgh",    -1);
    if(failures)
      {
       std_log(LOG_FILENAME_LINE,"Failing");
      }

    TEST ("abcdefghi",   -1);
    if(failures)
      {
       std_log(LOG_FILENAME_LINE,"Failing");
      }

    TEST ("abcdefghij",  -1);
    if(failures)
      {
       std_log(LOG_FILENAME_LINE,"Failing");
      }

    TEST ("abcdefghijk", -1);
    if(failures)
      {
       std_log(LOG_FILENAME_LINE,"Failing");
      }

    TEST ("aabcdefghij",  0);
    if(failures)
      {
       std_log(LOG_FILENAME_LINE,"Failing");
      }

    TEST ("abbcdefghij",  1);
    if(failures)
      {
       std_log(LOG_FILENAME_LINE,"Failing");
      }

    TEST ("abccdefghij",  2);
    if(failures)
      {
       std_log(LOG_FILENAME_LINE,"Failing");
      }

    TEST ("abcddefghij",  3);
    if(failures)
      {
       std_log(LOG_FILENAME_LINE,"Failing");
      }

    TEST ("abcdeefghij",  4);
    if(failures)
      {
       std_log(LOG_FILENAME_LINE,"Failing");
      }

    TEST ("abcdeffghij",  5);
    if(failures)
      {
       std_log(LOG_FILENAME_LINE,"Failing");
      }

    TEST ("abcdefgghij",  6);
    if(failures)
      {
       std_log(LOG_FILENAME_LINE,"Failing");
      }

    TEST ("abcdefghhij",  7);
    if(failures)
      {
       std_log(LOG_FILENAME_LINE,"Failing");
      }

    TEST ("abcdefghiij",  8);
    if(failures)
      {
       std_log(LOG_FILENAME_LINE,"Failing");
      }

    TEST ("abcdefghijj",  9);

    if(failures)
      {
       std_log(LOG_FILENAME_LINE,"Failing");
      }

    TEST ("aaabcdefghi",  0);
    if(failures)
      {
       std_log(LOG_FILENAME_LINE,"Failing");
      }

    TEST ("abbbcdefghi",  1);
    if(failures)
      {
       std_log(LOG_FILENAME_LINE,"Failing");
      }
    TEST ("abcccdefghi",  2);
    if(failures)
      {
       std_log(LOG_FILENAME_LINE,"Failing");
      }

    TEST ("abcdddefghi",  3);
    if(failures)
      {
       std_log(LOG_FILENAME_LINE,"Failing");
      }

    TEST ("abcdeeefghi",  4);
    if(failures)
      {
       std_log(LOG_FILENAME_LINE,"Failing");
      }
    TEST ("abcdefffghi",  5);
    if(failures)
      {
       std_log(LOG_FILENAME_LINE,"Failing");
      }

    TEST ("abcdefggghi",  6);
    if(failures)
      {
       std_log(LOG_FILENAME_LINE,"Failing");
      }

    TEST ("abcdefghhhi",  7);
    if(failures)
      {
       std_log(LOG_FILENAME_LINE,"Failing");
      }

    TEST ("abcdefghiii",  8);
    if(failures)
      {
       std_log(LOG_FILENAME_LINE,"Failing");
      }

    TEST ("abcdefghijjj", 9);
    if(failures)
      {
       std_log(LOG_FILENAME_LINE,"Failing");
      }


}

/**************************************************************************/

/* extern */ int rw_opt_no_fwd_iter;     // --no-ForwardIterator
/* extern */ int rw_opt_no_bidir_iter;   // --no-BidirectionalIterator
/* extern */ int rw_opt_no_rnd_iter;     // --no-RandomAccessIterator
/* extern */ int rw_opt_no_predicate;    // --no-Predicate

static 
void test_adjacent_find (const char* predicate)
{
    rw_info (0, 0, 0, 
             "template <class %s%{?}, class %s%{;}> "
             "%1$s std::adjacent_find (%1$s, %1$s%{?}, %2$s%{;})",
             "ForwardIterator",
             0 != predicate, "BinaryPredicate", 0 != predicate);

    if (rw_opt_no_fwd_iter) {
        rw_note (0, __FILE__, __LINE__, "ForwardIterator test disabled");
    }
    else {
        run_tests (FwdIter<X>(), (X*)0, predicate);
    }

    if (rw_opt_no_bidir_iter) {
        rw_note (0, __FILE__, __LINE__, "BidirectionalIterator test disabled");
    }
    else {
        run_tests (BidirIter<X>(), (X*)0, predicate);
    }

    if (rw_opt_no_rnd_iter) {
        rw_note (0, __FILE__, __LINE__, "RandomAccessIterator test disabled");
    }
    else {
        run_tests (RandomAccessIter<X>(), (X*)0, predicate);
    }
}

/**************************************************************************/

static int
run_test (int, char*[])
{
    test_adjacent_find (0);

    if (rw_opt_no_predicate) {
        rw_note (0, __FILE__, __LINE__, "Predicate test disabled");
    }
    else {
        test_adjacent_find ("std::equal_to<X>");
    }
                
    return 0;
}

/**************************************************************************/

int main (int argc, char *argv[])
{
	  std_log(LOG_FILENAME_LINE,"[Test Case for adjacent_find ]");
	     
 #ifndef __SYMBIAN32__	      
   return rw_test (argc, argv, __FILE__,
                  	  "lib.alg.adjacent.find",
                    0 /* no comment */, run_test,
                    "|-no-ForwardIterator#"
                    "|-no-BidirectionalIterator#"
                    "|-no-RandomAccessIterator#"
                    "|-no-Predicate#",
                    &rw_opt_no_fwd_iter,
                    &rw_opt_no_bidir_iter,
                    &rw_opt_no_rnd_iter,
                    &rw_opt_no_predicate);
  #else
    
     rw_test (argc, argv, __FILE__,
                  	  "lib.alg.adjacent.find",
                    0 /* no comment */, run_test,
                    "|-no-ForwardIterator#"
                    "|-no-BidirectionalIterator#"
                    "|-no-RandomAccessIterator#"
                    "|-no-Predicate#",
                    &rw_opt_no_fwd_iter,
                    &rw_opt_no_bidir_iter,
                    &rw_opt_no_rnd_iter,
                    &rw_opt_no_predicate);
    
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
   
   testResultXml("25_adjacent_find");
    close_log_file();
  return failures;
  #endif
}
