

/***************************************************************************
 *
 * 25.copy.cpp - test exercising 25.2.1 [lib.alg.copy]
 *
 * $Id: 25.copy.cpp 351761 2005-12-02 15:58:25Z sebor $
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

#include <algorithm>    // for copy
#include <cstddef>      // for size_t

#include <alg_test.h>
#include <driver.h>     // for rw_test()...
#ifdef __SYMBIAN32__
#include"std_log_result.h"
#define LOG_FILENAME_LINE __FILE__, __LINE__
#endif
int failures=0;
/**************************************************************************/

_RWSTD_NAMESPACE (std) { 

#ifndef _RWSTD_NO_EXPLICIT_INSTANTIATION

template
OutputIter<assign<base<> > >
copy (InputIter<const_cvt<base<>, assign<base<> > > >, 
      InputIter<const_cvt<base<>, assign<base<> > > >, 
      OutputIter<assign<base<> > >);

template
BidirIter<assign<base<> > >
copy_backward (BidirIter<const_cvt<base<>, assign<base<> > > >, 
               BidirIter<const_cvt<base<>, assign<base<> > > >,
               BidirIter<assign <base<> > >);

#endif // _RWSTD_NO_EXPLICIT_INSTANTIATION

}   // namespace std


/**************************************************************************/

// exercises std::copy()
template <class InputIterator, class OutputIterator, class T>
void test_copy (std::size_t N,
                const InputIterator &input_iter,
                const OutputIterator &output_iter,
                const T* )
{
    static const char* const it1name = type_name (input_iter, (T*) 0);
    static const char* const it2name = type_name (output_iter, (T*) 0);

    rw_info (0, 0, 0, "std::copy (%s, %1$s, %s)", it1name, it2name);

    // generate sequential values for each default constructed T
    //T::gen_ = gen_seq;
    gen_ = gen_seq;

    // create separate buffers for src and dst to ensure 25.2.1, p3
    T* const src = new T [N];
    T* const dst = new T [N];

    for (std::size_t i = 0; i != N; ++i) {

        T* const src_end = src + i + 1;
        T* const dst_end = dst + i + 1;

        // exercise 25.2.1 - std::copy()
        //std::size_t last_n_op_assign = T::n_total_op_assign_;
         std::size_t last_n_op_assign = n_total_op_assign_;
        const InputIterator begin =
            make_iter (src, src, src_end, input_iter);

        const InputIterator end =
            make_iter (src_end, src_end, src_end, input_iter);

        const OutputIterator dest =
            make_iter (dst, dst, dst_end, output_iter);

        const OutputIterator result =
            std::copy (begin, end, dest);

        T* const resptr = result.cur_;

        // verify 25.2.1, p2
        bool success = resptr == dst_end;
        if(!success)
         {
           failures++;
        std_log(LOG_FILENAME_LINE,"Reason: Returned destination is incorrect");  
        rw_assert (success, 0, __LINE__, 
                   "%zu. copy(%p, %p, %p) == %p, got %p",
                   i, src, src_end, dst, dst_end, resptr);
          }         

        if (!success)
            break;

        // check that algjrithm works correctly
        std::size_t j = 0;
        for (; j != i + 1; ++j) {
            success = src[j].val_ == dst[j].val_;
            if (!success)
                break;
        }

        rw_assert (success, 0, __LINE__, 
                   "%zu. copy(%p, %p, %p) mismatch at "
                   "offset %zu: expected %d, got %d: (%p, %p, %zu)",
                   i, src, src_end, dst,
                   j, src[j].val_, dst[j].val_,
                   dst, dst_end, j);

        if (!success)
            break;

        // verify 25.2.1, p4
      //  success = T::n_total_op_assign_ - last_n_op_assign == i + 1;
        success = n_total_op_assign_ - last_n_op_assign == i + 1;
      /*  rw_assert (success, 0, __LINE__, 
                   "%zu. copy(%p, %p, %p) complexity: ",
                   "%zu != %zu", i + 1, src, src_end, dst,
                    T::n_total_op_assign_ - last_n_op_assign, i + 1);
*/
        rw_assert (success, 0, __LINE__, 
                   "%zu. copy(%p, %p, %p) complexity: ",
                   "%zu != %zu", i + 1, src, src_end, dst,
                    n_total_op_assign_ - last_n_op_assign, i + 1);

       
       
        if (!success)
            break;
    }

    delete[] src;
    delete[] dst;
}

/**************************************************************************/

// exercises std::copy_backward()
template <class BidirIterator1, class BidirIterator2, class T>
void test_copy_backward (std::size_t N,
                         const BidirIterator1 &bidir_iter1,
                         const BidirIterator2 &bidir_iter2,
                         const T*)
{
    static const char* const it1name = type_name (bidir_iter1, (T*) 0);
    static const char* const it2name = type_name (bidir_iter2, (T*) 0);

    rw_info (0, 0, 0, "std::copy_backward (%s, %1$s, %s)", it1name, it2name);

    // generate sequential values for each default constructed T
   // T::gen_ = gen_seq;
    gen_ = gen_seq;

    // create separate buffers for src and dst to ensure 25.2.1, p3
    T* const src = new T [N];
    T* const dst = new T [N];

    for (std::size_t i = 0; i != N; ++i) {

        // default-construct a new T at the end of each sequence
        new (src + i) T ();
        new (dst + i) T ();

        // exercise 25.2.1 - std::copy_backward()
       // std::size_t last_n_op_assign = T::n_total_op_assign_;
       std::size_t last_n_op_assign = n_total_op_assign_;

        T* const src_end = src + i + 1;
        T* const dst_end = dst + i + 1;

        const BidirIterator1 begin =
            make_iter (src, src, src_end, bidir_iter1);

        const BidirIterator1 end =
            make_iter (src_end, src, src_end, bidir_iter1);

        const BidirIterator2 dest =
            make_iter (dst_end, dst, dst_end, bidir_iter2);

        const BidirIterator2 result =
            std::copy_backward (begin, end, dest);

        const T* const resptr = result.cur_;

        // verify 25.2.1, p7
        bool success = resptr == dst;
    
        if(!success)
          {
     std_log(LOG_FILENAME_LINE,"Reason: Failing as returned iterator is not equal to destination");
     std_log(LOG_FILENAME_LINE,"Reason :Failing for i=%d",i);
           }
        rw_assert (success, 0, __LINE__, 
                   "%zu. copy_backward(%p, %p, %p) == %p, got %p",
                   i, src, src_end, dst_end, dst, resptr);

        if (!success)
            break;

        // verify 25.2.1, p5
        std::size_t j = 0;
        for (; j != i + 1; ++j) {
            success = src[j].val_ == dst[j].val_;
        if(!success)
           {
         std_log(LOG_FILENAME_LINE,"Reason: Failing as returned as source and destination values are not equal ");
         std_log(LOG_FILENAME_LINE,"Reason :Failing for j=%d",j);
           }
            
            if (!success)
                  break;
        }
 
       rw_assert (success, 0, __LINE__, 
                   "%zu. copy_backward(%p, %p, %p) mismatch at "
                   "offset %zu: expected %d, got %d: (%p, %p, %zu)",
                   i, src, src_end, dst_end,
                   j, src[j].val_, dst[j].val_,
                   dst, dst_end, j);

        if (!success)
            break;

        // verify 25.2.1, p8
        //success = T::n_total_op_assign_ - last_n_op_assign == i + 1;
        success = n_total_op_assign_ - last_n_op_assign == i + 1;
     /*   rw_assert (success, 0, __LINE__, 
                   "%zu. copy_backward(%p, %p, %p) complexity: "
                   "%zu != %zu", i + 1, src, src_end, dst_end,
                   T::n_total_op_assign_ - last_n_op_assign, i + 1);
*/
        if(!success)
           {
         std_log(LOG_FILENAME_LINE,"Reason: Failing ");
            }
      rw_assert (success, 0, __LINE__, 
                   "%zu. copy_backward(%p, %p, %p) complexity: "
                   "%zu != %zu", i + 1, src, src_end, dst_end,
                   n_total_op_assign_ - last_n_op_assign, i + 1);

         if (!success)
            break;
    }

    delete[] src;
    delete[] dst;
}

/**************************************************************************/

/* extern */ int rw_opt_nloops = 32;     // --nloops
/* extern */ int rw_opt_no_input_iter;   // --no-InputIterator
/* extern */ int rw_opt_no_output_iter;  // --no-OutputIterator
/* extern */ int rw_opt_no_fwd_iter;     // --no-ForwardIterator
/* extern */ int rw_opt_no_bidir_iter;   // --no-BidirectionalIterator
/* extern */ int rw_opt_no_rnd_iter;     // --no-RandomAccessIterator

template <class InputIterator, class T>
void test_copy (const std::size_t N, const InputIterator &dummy, T*)
{
    if (rw_opt_no_output_iter) {
        rw_note (0, __FILE__, __LINE__, "OutputIterator test disabled");
       std_log(LOG_FILENAME_LINE,"OutputIterator test disabled");  
    }
    else {
        test_copy (N, dummy, OutputIter<X>((X*)0, (X*)0, (X*)0), (X*)0);
    }

    if (rw_opt_no_fwd_iter) {
        rw_note (0, __FILE__, __LINE__, "ForwardIterator test disabled");
     std_log(LOG_FILENAME_LINE,"ForwardIterator test disabled");  
    }
    else {
        test_copy (N, dummy, FwdIter<X>(), (X*)0);
    }
    
    if (rw_opt_no_bidir_iter) {
        rw_note (0, __FILE__, __LINE__, 
            "BidirectionalIterator test disabled");
    std_log(LOG_FILENAME_LINE,"BidirectionalIterator test disabled");          
    }
    else {
        test_copy (N, dummy, BidirIter<X>(), (X*)0);
    }

    if (rw_opt_no_rnd_iter) {
        rw_note (0, __FILE__, __LINE__, 
            "RandomAccessIterator test disabled");
    std_log(LOG_FILENAME_LINE,"RandomAccessIterator test disabled");                   
    }
    else {
        test_copy (N, dummy, RandomAccessIter<X>(), (X*)0);
    }
}

/**************************************************************************/

static void
test_copy (const std::size_t N)
{
    rw_info (0, 0, 0,
             "template <class %s, class %s> "
             "%2$s std::copy (%1$s, %1$s, %2$s)",
             "InputIterator", "OutputIterator");

    if (rw_opt_no_input_iter) {
        rw_note (0, __FILE__, __LINE__, "InputIterator test disabled");
   std_log(LOG_FILENAME_LINE,"InputIterator test disabled");                   
    }
    else {
        test_copy (N, InputIter<X>((X*)0, (X*)0, (X*)0), (X*)0);

    }

    if (rw_opt_no_fwd_iter) {
         rw_note (0, __FILE__, __LINE__, "ForwardIterator test disabled");
   std_log(LOG_FILENAME_LINE,"ForwardIterator test disabled");                  
    }
    else {
        test_copy (N, FwdIter<X>(), (X*)0);
    }

    if (rw_opt_no_bidir_iter) {
        rw_note (0, __FILE__, __LINE__, 
            "BidirectionalIterator test disabled");
   std_log(LOG_FILENAME_LINE,"BidirectionalIterator test disabled");                            
    }
    else {
        test_copy (N, BidirIter<X>(), (X*)0);
    }

    if (rw_opt_no_rnd_iter) {
        rw_note (0, __FILE__, __LINE__, 
            "RandomAccessIterator test disabled");
    std_log(LOG_FILENAME_LINE,"RandomAccessIterator test disabled");
    }
    else {
        test_copy (N, RandomAccessIter<X>(), (X*)0);
    }
}

/**************************************************************************/

template <class InputIterator, class T>
void test_copy_backward (const std::size_t N, const InputIterator &dummy, T*)
{
    if (rw_opt_no_bidir_iter) {
        rw_note (0, __FILE__, __LINE__, 
            "BidirectionalIterator test disabled");
  std_log(LOG_FILENAME_LINE,"BidirectionalIterator test disabled");          
            
    }
    else {
        test_copy_backward (N, dummy, BidirIter<X>(), (X*)0);
    }

    if (rw_opt_no_rnd_iter) {
        rw_note (0, __FILE__, __LINE__, 
            "RandomAccessIterator test disabled");
    std_log(LOG_FILENAME_LINE,"RandomAccessIterator test disabled");
    }
    else {
        test_copy_backward (N, dummy, RandomAccessIter<X>(), (X*)0);
    }
}

/**************************************************************************/

static void
test_copy_backward (const std::size_t N)
{
     rw_info (0, 0, 0,
              "template <class %s, class %s> %2$s "
              "std::copy_backward (%1$s, %1$s, %2$s)",
              "BidirectionalIterator1", "BidirectionalIterator2");

    if (rw_opt_no_bidir_iter) {
        rw_note (0, __FILE__, __LINE__, 
            "BidirectionalIterator test disabled");
    std_log(LOG_FILENAME_LINE,"BidirectionalIterator test disabled");
    }
    else {
        test_copy_backward (N, BidirIter<X>(), (X*)0);
    }

    if (rw_opt_no_rnd_iter) {
        rw_note (0, __FILE__, __LINE__, 
            "RandomAccessIterator test disabled");
    std_log(LOG_FILENAME_LINE,"RandomAccessIterator test disabled");        
    }
    else {
        test_copy_backward (N, RandomAccessIter<X>(), (X*)0);
    }
}

/**************************************************************************/

static int
run_test (int, char*[])
{
        // check that the number of loops is non-negative
    rw_fatal (-1 < rw_opt_nloops, 0, 0,
              "number of loops must be non-negative, got %d",
              rw_opt_nloops);

    const std::size_t N = std::size_t (rw_opt_nloops);

    test_copy (N);

    test_copy_backward (N);

    return 0;
}

/**************************************************************************/

int main (int argc, char *argv[])
{
   
  std_log(LOG_FILENAME_LINE,"[Test Case for Copy ]");
  
 #ifndef __SYMBIAN32__
    return rw_test (argc, argv, __FILE__,
                    "lib.alg.copy",
                    0 /* no comment */, run_test,
                    "|-nloops#"
                    "|-no-InputIterator#"
                    "|-no-OutputIterator#"
                    "|-no-ForwardIterator#"
                    "|-no-BidirectionalIterator#"
                    "|-no-RandomAccessIterator#",
                    &rw_opt_nloops,
                    &rw_opt_no_input_iter,
                    &rw_opt_no_output_iter,
                    &rw_opt_no_fwd_iter,
                    &rw_opt_no_bidir_iter,
                    &rw_opt_no_rnd_iter);
  #else                
     rw_test (argc, argv, __FILE__,
                    "lib.alg.copy",
                    0 /* no comment */, run_test,
                    "|-nloops#"
                    "|-no-InputIterator#"
                    "|-no-OutputIterator#"
                    "|-no-ForwardIterator#"
                    "|-no-BidirectionalIterator#"
                    "|-no-RandomAccessIterator#",
                    &rw_opt_nloops,
                    &rw_opt_no_input_iter,
                    &rw_opt_no_output_iter,
                    &rw_opt_no_fwd_iter,
                    &rw_opt_no_bidir_iter,
                    &rw_opt_no_rnd_iter);
  
  if(failures)
   {
               assert_failed = true;
       std_log(LOG_FILENAME_LINE,"Result: Failed");   
      }
  else
    {

       std_log(LOG_FILENAME_LINE,"Result: Passed");
      }

  std_log(LOG_FILENAME_LINE,"[End Test Case ]");
  testResultXml("25_copy");
    close_log_file(); 
 return failures;
#endif 

}

