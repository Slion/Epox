

/***************************************************************************
 *
 * 25.mismatch.cpp - test exercising 25.1.7 [lib.mismatch]
 *
 * $Id: 25.mismatch.cpp 349017 2005-11-25 20:23:11Z sebor $
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

#include <algorithm>    // for mismatch
#include <functional>   // for equal_to
#include <cstddef>      // for size_t

#include <alg_test.h>
#include <driver.h>     // for rw_test()...
#include"std_log_result.h"
#define LOG_FILENAME_LINE __FILE__, __LINE__
int failures=0;

/**************************************************************************/

// exercises std::mismatch()
template <class InputIterator1, class InputIterator2, class T>
void test_mismatch (std::size_t           N,
                    const InputIterator1 &it1_dummy,
                    const InputIterator2 &it2_dummy,
                    T*,
                    const char           *predicate)
{
    static const char* const it1name = type_name (it1_dummy, (T*)0);
    static const char* const it2name = type_name (it2_dummy, (T*)0);

    rw_info (0, 0, 0, "std::mismatch (%s, %1$s, %s%{?}, %s%{;})",
             it1name, it2name, 0 != predicate, predicate);

    // generate sequential values for each default constructed X
    // for both lists
//    X::gen_ = gen_seq_2lists;
  gen_ = gen_seq_2lists;

    // use ::operator new() to prevent default initialization
    X *buf1 = _RWSTD_STATIC_CAST (X*, ::operator new (N * sizeof (X)));
    X *buf2 = _RWSTD_STATIC_CAST (X*, ::operator new (N * sizeof (X)));

    const std::size_t mid_inx = N / 2;

    for (std::size_t i = 0; i != N; ++i) {

        // default-construct a new X at the end of the array
        new (buf1 + i) X ();

        // build a nearly identical list only missing N/2 (the N/2th
        // element) from it's list
        if (i != mid_inx)
            new (buf2 + i) X ();
    }

    new (buf2 + mid_inx) X ();

    for (std::size_t i = 0; i != N; ++i) {

        // exercise 25.1.7 - std::mismatch<>()
       // std::size_t last_n_op_eq  = X::n_total_op_eq_;
        std::size_t last_n_op_eq  = n_total_op_eq_;

        X* const buf1_end = buf1 + i + 1;
        X* const buf2_end = buf2 + i + 1;

        const InputIterator1 first1 =
            make_iter (buf1, buf1, buf1_end, it1_dummy);

        const InputIterator1 last1 =
            make_iter (buf1_end, buf1_end, buf1_end, it1_dummy);

        const InputIterator2 first2 =
            make_iter (buf2, buf2, buf2_end, it2_dummy);

        const std::equal_to<X> pred = std::equal_to<X>();

        const std::pair<InputIterator1, InputIterator2> result =
            predicate ? std::mismatch (first1, last1, first2, pred)
                      : std::mismatch (first1, last1, first2);

        // verify 25.1.7, p2
        X* const expected_buf1_end = 
            (i < mid_inx) ? buf1_end : buf1 + mid_inx;

        X* const expected_buf2_end = 
            (i < mid_inx) ? buf2_end : buf2 + mid_inx;

        X* const res_buf1_end = 
            buf1 + (result.first.cur_ - first1.cur_);

        X* const res_buf2_end = 
            buf2 + (result.second.cur_ - first2.cur_);

        // check that the returned iterators are valid
        bool success = 
            buf1 <= res_buf1_end && res_buf1_end <= buf1_end &&
            buf2 <= res_buf2_end && res_buf2_end <= buf2_end;
    
       if(!success)
        {
           failures++;
           std_log(LOG_FILENAME_LINE,"Reason: Failing");
         }  
     

        rw_assert (success, 0, __LINE__, 
                   "%zu. mismatch (%s, %2$s, %s%{?}, %s%{;}): "
                   "result { %d, %d } out of range: expected "
                   "{ [0, %d], [0, %d] }",
                   i, it1name, it2name, 0 != predicate, predicate,
                   res_buf1_end - buf1, res_buf2_end - buf2,
                   buf1_end - buf1, buf2_end - buf2);

        // iterators are valid check that the algorithm works correctly
        success = 
               res_buf1_end->val_ == expected_buf1_end->val_
            && res_buf2_end->val_ == expected_buf2_end->val_;
       if(!success)
        {
           failures++;
           std_log(LOG_FILENAME_LINE,"Reason: Failing");
         }
      
        rw_assert (success, 0, __LINE__, 
                   "%zu. mismatch (%s, %2$s, %s%{?}, %s%{;}): "
                   "correctness: got: %d "
                   "expected: %d, got: %d expected: %d",
                   i, it1name, it2name, 0 != predicate, predicate,
                   res_buf1_end->val_, expected_buf1_end->val_,
                   res_buf2_end->val_, expected_buf2_end->val_);

        // verify 25.1.7, p3
    /*    success = X::n_total_op_eq_ - last_n_op_eq <= (N + 1);

        rw_assert (success, 0, __LINE__, 
                   "%zu. mismatch (%s, %2$s, %s%{?}, %s%{;}): "
                   "complexity: %zu <= %zu",
                   i, it1name, it2name, 0 != predicate, predicate,
                   X::n_total_op_eq_, i + 1);
           */
           success = n_total_op_eq_ - last_n_op_eq <= (N + 1);

			if(!success)
			        {
			           failures++;
			           std_log(LOG_FILENAME_LINE,"Reason: Failing");
			         }

        rw_assert (success, 0, __LINE__, 
                   "%zu. mismatch (%s, %2$s, %s%{?}, %s%{;}): "
                   "complexity: %zu <= %zu",
                   i, it1name, it2name, 0 != predicate, predicate,
                   n_total_op_eq_, i + 1);    
           
        if (!success)
            break;
    }
    
    ::operator delete (buf1);
    ::operator delete (buf2);
}


/**************************************************************************/

/* extern */ int rw_opt_nloops = 32;     // --nloops
/* extern */ int rw_opt_no_input_iter;   // --no-InputIterator
/* extern */ int rw_opt_no_fwd_iter;     // --no-ForwardIterator
/* extern */ int rw_opt_no_bidir_iter;   // --no-BidirectionalIterator
/* extern */ int rw_opt_no_rnd_iter;     // --no-RandomAccessIterator
/* extern */ int rw_opt_no_predicate;    // --no-Predicate


template <class InputIterator, class T>
void test_mismatch (const InputIterator &dummy, T*, const char *predicate)
{
    // check that the number of loops is non-negative
    rw_fatal (-1 < rw_opt_nloops, 0, 0,
              "number of loops must be non-negative, got %d",
              rw_opt_nloops);

    const std::size_t N = std::size_t (rw_opt_nloops);

    if (rw_opt_no_input_iter) {
        rw_note (0, __FILE__, __LINE__, "InputIterator test disabled");
    }
    else {
        test_mismatch (N, dummy, InputIter<X>(0, 0, 0), (X*)0, predicate);
    }

    if (rw_opt_no_fwd_iter) {
        rw_note (0, __FILE__, __LINE__, "ForwardIterator test disabled");
    }
    else {
        test_mismatch (N, dummy, FwdIter<X>(), (X*)0, predicate);
    }

    if (rw_opt_no_bidir_iter) {
        rw_note (0, __FILE__, __LINE__, 
            "BidirectionalIterator test disabled");
    }
    else {
        test_mismatch (N, dummy, BidirIter<X>(), (X*)0, predicate);
    }

    if (rw_opt_no_rnd_iter) {
        rw_note (0, __FILE__, __LINE__, 
            "RandomAccessIterator test disabled");
    }
    else {
        test_mismatch (N, dummy, RandomAccessIter<X>(), (X*)0, predicate);
    }
}


/**************************************************************************/

static void
test_mismatch (const char *predicate)
{
    rw_info (0, 0, 0,
             "template <class %s, class %s> "
             "std::pair<%1$s, %2$s> "
             "std::mismatch (%1$s, %1$s, %2$s%{?}, %s%{;})",
             "InputIterator1", "InputIterator2",
             0 != predicate, predicate);

    if (rw_opt_no_input_iter) {
        rw_note (0, __FILE__, __LINE__, "InputIterator test disabled");
    }
    else {
        test_mismatch (InputIter<X>(0, 0, 0), (X*)0, predicate);
    }

    if (rw_opt_no_fwd_iter) {
        rw_note (0, __FILE__, __LINE__, "ForwardIterator test disabled");
    }
    else {
        test_mismatch (ConstFwdIter<X>(), (X*)0, predicate);
        test_mismatch (FwdIter<X>(), (X*)0, predicate);
    }

    if (rw_opt_no_bidir_iter) {
        rw_note (0, __FILE__, __LINE__, 
            "BidirectionalIterator test disabled");
    }
    else {
        test_mismatch (ConstBidirIter<X>(), (X*)0, predicate);
        test_mismatch (BidirIter<X>(), (X*)0, predicate);
    }

    if (rw_opt_no_rnd_iter) {
        rw_note (0, __FILE__, __LINE__, 
            "RandomAccessIterator test disabled");
    }
    else {
        test_mismatch (ConstRandomAccessIter<X>(), (X*)0, predicate);
        test_mismatch (RandomAccessIter<X>(), (X*)0, predicate);
    }
}

/**************************************************************************/

static int
run_test (int, char*[])
{
    test_mismatch (0);

    if (rw_opt_no_predicate) {
        rw_note (0, __FILE__, __LINE__, "Predicate test disabled");
    }
    else {
        test_mismatch ("std::equal_to<X>");
    }

    return 0;
}

/**************************************************************************/

int main (int argc, char *argv[])
{
   std_log(LOG_FILENAME_LINE,"[Test Case for mismatch]");
    
    #ifndef __SYMBIAN32__
    return rw_test (argc, argv, __FILE__,
                    "lib.alg.mismatch",
                    0 /* no comment */, run_test,
                    "|-nloops#"
                    "|-no-InputIterator#"
                    "|-no-ForwardIterator#"
                    "|-no-BidirectionalIterator#"
                    "|-no-RandomAccessIterator#"
                    "|-no-Predicate",
                    &rw_opt_nloops,
                    &rw_opt_no_input_iter,
                    &rw_opt_no_fwd_iter,
                    &rw_opt_no_bidir_iter,
                    &rw_opt_no_rnd_iter,
                    &rw_opt_no_predicate);
   #else
     rw_test (argc, argv, __FILE__,
                    "lib.alg.mismatch",
                    0 /* no comment */, run_test,
                    "|-nloops#"
                    "|-no-InputIterator#"
                    "|-no-ForwardIterator#"
                    "|-no-BidirectionalIterator#"
                    "|-no-RandomAccessIterator#"
                    "|-no-Predicate",
                    &rw_opt_nloops,
                    &rw_opt_no_input_iter,
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
   
   testResultXml("25_mismatch");
    close_log_file();
 #endif

}
