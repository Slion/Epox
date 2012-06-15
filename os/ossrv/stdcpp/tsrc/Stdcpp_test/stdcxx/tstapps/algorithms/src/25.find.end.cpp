

/***************************************************************************
 *
 * find_end.cpp - test exercising 25.1.3 [lib.alg.find.end]
 *
 * $Id: 25.find.end.cpp 354156 2005-12-05 20:19:45Z sebor $
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

#include <algorithm>    // for find_end
#include <functional>   // for equal_to
#include <cstddef>      // for size_t

#include <alg_test.h>   
#include <driver.h>     // for rw_test
#include"std_log_result.h"
#define LOG_FILENAME_LINE __FILE__, __LINE__
int failures=0;

/**************************************************************************/

_RWSTD_NAMESPACE (std) { 

#ifndef _RWSTD_NO_EXPLICIT_INSTANTIATION

template
FwdIter<eq_comp<base<> > > 
find_end (FwdIter<eq_comp<base<> > >, FwdIter<eq_comp<base<> > >, 
          FwdIter<eq_comp<base<> > >, FwdIter<eq_comp<base<> > >);

template
FwdIter<eq_comp<base<> > > 
find_end (FwdIter<eq_comp<base<> > >, FwdIter<eq_comp<base<> > >, 
          FwdIter<eq_comp<base<> > >, FwdIter<eq_comp<base<> > >, 
          binary_predicate<eq_comp<base<> > >);

#endif // _RWSTD_NO_EXPLICIT_INSTANTIATION

}   // namespace std

/**************************************************************************/

// used to initialize an array of objects of type T
static const char *tinit_begin;

int tinit ()
{
    typedef unsigned char UChar;

    return UChar (*tinit_begin++);
}


// exercises std::find_end()
template <class ForwardIterator1, class ForwardIterator2, class T>
void do_test (int               line,     // line number of test case
              const char       *src,      // source sequence
              const char       *fseq,     // sequence to be found
              std::size_t       resoff,   // offset of result
              ForwardIterator1 &dummy_iter1,
              ForwardIterator2 &dummy_iter2,
              const T*,
              const char       *predicate)
{
    static const char* const it1name = type_name (dummy_iter1, (T*)0);
    static const char* const it2name = type_name (dummy_iter2, (T*)0);
  #ifdef __SYMBIAN32__
     const std::size_t nsrc  = strlen (src);
     const std::size_t nfseq = strlen (fseq);
  #else
    const std::size_t nsrc  = std::strlen (src);
    const std::size_t nfseq = std::strlen (fseq);
  #endif
    if (std::size_t (-1) == resoff)
        resoff = nsrc;

    // have the T default ctor initialize objects from `src'
    tinit_begin = src;
  #ifndef __SYMBIAN32__
   T::gen_     = tinit;
  #else    
      gen_     = tinit;
  #endif
    T* const tsrc = new T [nsrc];

    T* const src_begin = tsrc;
    T* const src_end   = tsrc + nsrc;

    // have the T default ctor initialize sequence to be found
    tinit_begin = fseq;

    T* const tfseq = new T [nfseq];

    T* const fseq_begin = tfseq;
    T* const fseq_end   = tfseq + nfseq;

    // create iterators
    const ForwardIterator1 first1 =
        make_iter (src_begin, src_begin, src_end, dummy_iter1);

    const ForwardIterator1 last1 =
        make_iter (src_end, src_begin, src_end, dummy_iter1);

    const ForwardIterator2 first2 =
        make_iter (fseq_begin, fseq_begin, fseq_end, dummy_iter2);

    const ForwardIterator2 last2 =
        make_iter (fseq_end, fseq_begin, fseq_end, dummy_iter2);

    // compute the number of invocations of the predicate
    #ifndef __SYMBIAN32__
    std::size_t last_n_op_assign = T::n_total_op_assign_;
    #else
    std::size_t last_n_op_assign = n_total_op_assign_;
    #endif
    const ForwardIterator1 res = predicate ? 
          std::find_end (first1, last1, first2, last2, std::equal_to<T>())
        : std::find_end (first1, last1, first2, last2);
    
    // silence a bogus EDG eccp remark #550-D:
    // variable "res" was set but never used
    _RWSTD_UNUSED (res);

    bool success = res.cur_ == first1.cur_ + resoff;
    if(!success)
     {
        failures++;
        std_log(LOG_FILENAME_LINE,"Reason: Failing");
      }
    rw_assert (success, 0, line, 
               "line %d: find_end<%s, %s>(it = \"%s\", \"%s\")"
               " == (it + %zu), got (it + %td)",
               __LINE__, it1name, it2name, src, fseq,
               resoff, res.cur_ - first1.cur_);

    if (!success) {
        delete[] tsrc;
        delete[] tfseq;
        return;
    }

    if (res.cur_ != last1.cur_) {
        std::size_t j = 0;
        for ( ; j < nfseq; ++j) {
            success = (res.cur_ + j)->val_ == fseq[j];
            if (!success)
                break;
        }

     if(!success)
     {
        failures++;
        std_log(LOG_FILENAME_LINE,"Reason: Failing ");
      }

        rw_assert (success, 0, line, 
                   "line %d: find_end<%s, %s>(it = \"%s\", \"%s\")"
                   " expected value %c got %c at %zu position",
                   __LINE__, it1name, it2name, src, fseq,
                   fseq[j], (res.cur_ + j)->val_, j);

        if (!success) {
            delete[] tsrc;
            delete[] tfseq;
            return;
        }
    }

    success =
//        T::n_total_op_assign_ - last_n_op_assign <= nfseq * (nsrc - nfseq + 1);
       n_total_op_assign_ - last_n_op_assign <= nfseq * (nsrc - nfseq + 1);
  /*  rw_assert (success, 0, line, 
               "line %d: find_end<%s, %s>(it = \"%s\", \"%s\")"
               " %s invoked %zu times, expected no more than %td",
               __LINE__, it1name, it2name, src, fseq,
               predicate ? predicate : "comparison operator", 
               T::n_total_op_eq_, nfseq * (nsrc - nfseq + 1));
*/ if(!success)
     {
        failures++;
        std_log(LOG_FILENAME_LINE,"Reason: Failing");
      }

   rw_assert (success, 0, line, 
               "line %d: find_end<%s, %s>(it = \"%s\", \"%s\")"
               " %s invoked %zu times, expected no more than %td",
               __LINE__, it1name, it2name, src, fseq,
               predicate ? predicate : "comparison operator", 
               n_total_op_eq_, nfseq * (nsrc - nfseq + 1));

    delete[] tsrc;
    delete[] tfseq;
}

/**************************************************************************/

template <class ForwardIterator1, class ForwardIterator2, class T>
void run_tests (const ForwardIterator1& dummy_iter1, 
                const ForwardIterator2& dummy_iter2,
                const T*, const char* predicate)
{
    static const char* const it1name = type_name (dummy_iter1, (T*)0);
    static const char* const it2name = type_name (dummy_iter2, (T*)0);

    rw_info (0, 0, 0, "%s std::find_end (%1$s, %1$s, %s, %2$s%{?}, %s%{;})", 
             it1name, it2name, 0 != predicate, predicate);
    
#define TEST(src, fnd, off_res) \
    do_test (__LINE__, src, fnd, std::size_t (off_res), \
             dummy_iter1, dummy_iter2, (X*)0, predicate)

    //    +------------------ subject sequence
    //    |                +--- sequence to be found
    //    |                |   
    //    |                |      +-- offset of returned iterator,
    //    |                |      |   -1 denotes the end of sequence
    //    v                v      v
    TEST ("abcdefghijk",   "ab",  0);
     if(failures)
       {
        std_log(LOG_FILENAME_LINE,"Reason: Failing");
      }
      
    TEST ("abcdefghijk",   "bc",  1);
    
     if(failures)
       {
        std_log(LOG_FILENAME_LINE,"Reason: Failing");
      }

    TEST ("abcdefghijk",   "cd",  2);
    
    if(failures)
       {
        std_log(LOG_FILENAME_LINE,"Reason: Failing");
      }

    TEST ("abcdefghijk",   "de",  3);
    
    if(failures)
       {
        std_log(LOG_FILENAME_LINE,"Reason: Failing");
      }

    TEST ("abcdefghijk",   "ef",  4);
    if(failures)
       {
        std_log(LOG_FILENAME_LINE,"Reason: Failing");
      }

    TEST ("abcdefghijk",   "fg",  5);
    if(failures)
       {
        std_log(LOG_FILENAME_LINE,"Reason: Failing");
      }

    TEST ("abcdefghijk",   "gh",  6);
    if(failures)
       {
        std_log(LOG_FILENAME_LINE,"Reason: Failing");
      }

    TEST ("abcdefghijk",   "hi",  7);
      if(failures)
       {
        std_log(LOG_FILENAME_LINE,"Reason: Failing");
      }

    TEST ("abcdefghijk",   "ij",  8);
    if(failures)
       {
        std_log(LOG_FILENAME_LINE,"Reason: Failing");
      }

    TEST ("abcdefghijk",   "jk",  9);
    if(failures)
       {
        std_log(LOG_FILENAME_LINE,"Reason: Failing");
      }

    TEST ("ababcdefghij",  "ab",  2);
    if(failures)
       {
        std_log(LOG_FILENAME_LINE,"Reason: Failing");
      }

    TEST ("abcbcdefghij",  "bc",  3);
    if(failures)
       {
        std_log(LOG_FILENAME_LINE,"Reason: Failing");
      }

    TEST ("abcdcdefghij",  "cd",  4);
    if(failures)
       {
        std_log(LOG_FILENAME_LINE,"Reason: Failing");
      }

    TEST ("abcdedefghij",  "de",  5);
    if(failures)
       {
        std_log(LOG_FILENAME_LINE,"Reason: Failing");
      }

    TEST ("abcdefefghij",  "ef",  6);
    if(failures)
       {
        std_log(LOG_FILENAME_LINE,"Reason: Failing");
      }

    TEST ("abcdefgfghij",  "fg",  7);
    if(failures)
       {
        std_log(LOG_FILENAME_LINE,"Reason: Failing");
      }

    TEST ("abcdefghghij",  "gh",  8);
    if(failures)
       {
        std_log(LOG_FILENAME_LINE,"Reason: Failing");
      }

    TEST ("abcdefghihij",  "hi",  9);
    if(failures)
       {
        std_log(LOG_FILENAME_LINE,"Reason: Failing");
      }

    TEST ("bcdefghijijk",  "ij",  9);
    if(failures)
       {
        std_log(LOG_FILENAME_LINE,"Reason: Failing");
      }

    TEST ("bcdefghijkjk",  "jk", 10);
    if(failures)
       {
        std_log(LOG_FILENAME_LINE,"Reason: Failing");
      }
    TEST ("",               "a", -1);
    if(failures)
       {
        std_log(LOG_FILENAME_LINE,"Reason: Failing");
      }

    TEST ("a",             "ab", -1);
    if(failures)
       {
        std_log(LOG_FILENAME_LINE,"Reason: Failing");
      }

    TEST ("ab",           "abc", -1);
    if(failures)
       {
        std_log(LOG_FILENAME_LINE,"Reason: Failing");
      }

    TEST ("abc",         "abcd", -1);
    if(failures)
       {
        std_log(LOG_FILENAME_LINE,"Reason: Failing");
      }

    TEST ("abcd",       "abcde", -1);
    if(failures)
       {
        std_log(LOG_FILENAME_LINE,"Reason: Failing");
      }

    TEST ("abcde",     "abcdef", -1);
    if(failures)
       {
        std_log(LOG_FILENAME_LINE,"Reason: Failing");
      }

    TEST ("abcdef",   "abcdefg", -1);
    if(failures)
       {
        std_log(LOG_FILENAME_LINE,"Reason: Failing");
      }

    TEST ("abcdefg", "abcdefgh", -1);
    if(failures)
       {
        std_log(LOG_FILENAME_LINE,"Reason: Failing");
      }


    TEST ("abcdefghijk",   "ba", -1);
    if(failures)
       {
        std_log(LOG_FILENAME_LINE,"Reason: Failing");
      }

    TEST ("abcdefghijk",   "cb", -1);
    if(failures)
       {
        std_log(LOG_FILENAME_LINE,"Reason: Failing");
      }

    TEST ("abcdefghijk",   "dc", -1);
    if(failures)
       {
        std_log(LOG_FILENAME_LINE,"Reason: Failing");
      }

    TEST ("abcdefghijk",   "ed", -1);
    if(failures)
       {
        std_log(LOG_FILENAME_LINE,"Reason: Failing");
      }

    TEST ("abcdefghijk",   "fe", -1);
    if(failures)
       {
        std_log(LOG_FILENAME_LINE,"Reason: Failing");
      }

    TEST ("abcdefghijk",   "gf", -1);
    if(failures)
       {
        std_log(LOG_FILENAME_LINE,"Reason: Failing");
      }

    TEST ("abcdefghijk",   "hg", -1);
    if(failures)
       {
        std_log(LOG_FILENAME_LINE,"Reason: Failing");
      }

    TEST ("abcdefghijk",   "ih", -1);

    if(failures)
       {
        std_log(LOG_FILENAME_LINE,"Reason: Failing");
      }
    TEST ("abcdefghijk",   "ji", -1);
    if(failures)
       {
        std_log(LOG_FILENAME_LINE,"Reason: Failing");
      }

}

/**************************************************************************/

/* extern */ int rw_opt_no_fwd_iter;     // --no-ForwardIterator
/* extern */ int rw_opt_no_bidir_iter;   // --no-BidirectionalIterator
/* extern */ int rw_opt_no_rnd_iter;     // --no-RandomAccessIterator
/* extern */ int rw_opt_no_predicate;    // --no-Predicate


template <class ForwardIterator, class T>
void run_test_find_end (const ForwardIterator& dummy, T*, 
                        const char* predicate)
{
    if (rw_opt_no_fwd_iter) {
        rw_note (0, __FILE__, __LINE__, "ForwardIterator test disabled");
    }
    else {
        run_tests (dummy, FwdIter<T>(), (T*)0, predicate);
    }

    if (rw_opt_no_bidir_iter) {
        rw_note (0, __FILE__, __LINE__, "BidirectionalIterator test disabled");
    }
    else {
        run_tests (dummy, BidirIter<T>(), (T*)0, predicate);
    }

    if (rw_opt_no_rnd_iter) {
        rw_note (0, __FILE__, __LINE__, "RandomAccessIterator test disabled");
    }
    else {
        run_tests (dummy, RandomAccessIter<T>(), (T*)0, predicate);
    }
}

/**************************************************************************/

static void
test_find_end (const char* predicate)
{
    rw_info (0, 0, 0, 
             "template <class %s, class %s> "
             "%1$s std::find_end (%1$s, %1$s, %2$s, %2$s%{?}, %s%{;})",
             "ForwardIterator1", "ForwardIterator2", 
             0 != predicate, predicate);

    if (rw_opt_no_fwd_iter) {
        rw_note (0, __FILE__, __LINE__, "ForwardIterator test disabled");
    }
    else {       
        run_test_find_end (FwdIter<X>(), (X*)0, predicate);
    }

    if (rw_opt_no_bidir_iter) {
        rw_note (0, __FILE__, __LINE__, "BidirectionalIterator test disabled");
    }
    else {
        run_test_find_end (BidirIter<X>(), (X*)0, predicate);
    }

    if (rw_opt_no_rnd_iter) {
        rw_note (0, __FILE__, __LINE__, "RandomAccessIterator test disabled");
    }
    else {
        run_test_find_end (RandomAccessIter<X>(), (X*)0, predicate);
    }
}

/**************************************************************************/

static int
run_test (int, char*[])
{
    test_find_end (0);

    if (rw_opt_no_predicate) {
        rw_note (0, __FILE__, __LINE__, "Predicate test disabled");
    }
    else {
        test_find_end ("std::equal_to<X>");
    }
                
    return 0;
}

/**************************************************************************/

int main (int argc, char *argv[])
{
   std_log(LOG_FILENAME_LINE,"[Test Case for find_end]");
   
   #ifndef __SYMBIAN32__ 
    return rw_test (argc, argv, __FILE__,
                    "lib.alg.find.end",
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
                    "lib.alg.find.end",
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
   
   testResultXml("25_find_end");
    close_log_file();
 #endif
}
