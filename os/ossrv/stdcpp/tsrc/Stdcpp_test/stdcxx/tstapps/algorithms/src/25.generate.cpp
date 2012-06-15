

/***************************************************************************
 *
 * 25.generate.cpp - test exercising 25.2.6 [lib.alg.generate]
 *
 * $Id: 25.generate.cpp 354939 2005-12-08 02:28:44Z sebor $
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

#include <algorithm>    // for generate
#include <cstddef>      // for size_t

#include <alg_test.h>
#include <driver.h>     // for rw_test()
#include"std_log_result.h"
#define LOG_FILENAME_LINE __FILE__, __LINE__
int failures=0;

/**************************************************************************/

template <class T>
struct Generator
{
    // return a const reference to avoid requiring
    // that T be copy-constructible
    const T& operator() () const {
        static const union {
            void*         align_;
            unsigned char buf_ [sizeof (T)];
        } u = { 0 };
        return *(const T*)(const void*)&u;
    }
};


_RWSTD_SPECIALIZED_CLASS
struct Generator<X>
{
    // dummy arguments provided to prevent Generator
    // from being DefaultConstructible
    Generator (int, int /* dummy */) {
        // use the generator of sequential values
     //   X::gen_ = gen_seq;
        gen_ = gen_seq;
    }

    X operator() () /* non-const */ {
        // return a default-constructed X intialized
        // to the next sequential value
        return X();
    }
};


// Size template argument to generate_n()
struct Size
{
    // dummy argument provided to prevent Size from being constructible
    // by conversion from size_t
    Size (int val, int /* dummy */ )
        : val_ (val) { /* empty */ }

    // Size must be convertible to an integral type
    operator int () const { return val_; }
    int operator++()
    {
    return ++val_;
    }
    int operator--()
    {
    return --val_;
    }
private:

    int val_;
};

/**************************************************************************/
/*
_RWSTD_NAMESPACE (std) { 

// disable explicit instantiation for compilers (like MSVC)
// that can't handle it
#ifndef _RWSTD_NO_EXPLICIT_INSTANTIATION

template 
void generate (FwdIter<assign<base<> > >, FwdIter<assign<base<> > >,
               Generator<assign<base<> > >);

template
void generate_n (OutputIter<assign<base<> > >, std::size_t,
                 Generator<assign<base<> > >);
#endif // _RWSTD_NO_EXPLICIT_INSTANTIATION

}   // namespace std
*/
/**************************************************************************/

// exercises std::generate() 
template <class ForwardIterator, class T>
void test_generate (std::size_t N,
                    const ForwardIterator& gen_iter,
                    const T* )
{
    static const char* const itname  = type_name (gen_iter, (T*) 0);
    static const char* const genname = "Generator";

    rw_info (0, 0, 0, 
             "void std::generate (%s, %1$s, %s)", itname, genname);

    // generate sequential values for each default constructed T
    #ifndef __SYMBIAN32__
    T::gen_ = gen_seq;
    #else
    gen_ = gen_seq;
    #endif
    
    T *buf = new X [N];

    for (std::size_t i = 0; i < N; ++i) {

        // exercise 25.2.6, std::generate<> ()
    #ifndef __SYMBIAN32__
        std::size_t last_n_op_assign = T::n_total_op_assign_;
    #else
         std::size_t last_n_op_assign = n_total_op_assign_;
    #endif     
        T* const buf_end = buf + i + 1;

        const ForwardIterator begin =
            make_iter (buf, buf, buf_end, gen_iter);

        const ForwardIterator end =
            make_iter (buf_end, buf_end, buf_end, gen_iter);

        const Generator<T> gen (0, 0);

        // store the value of the next element
        const int last_val = Generator<T>(0, 0)().val_;

        std::generate (begin, end, gen);

        bool success = true;

        // verify 25.2.6, p2
        std::size_t j = 0;
        for ( ; j != i; ++j) {
            success = (begin.cur_ + j)->val_ == int (last_val + j + 1);
            if (!success)
                break;
        }
   
        if(!success)
         {
           failures++;
           std_log(LOG_FILENAME_LINE,"Reason: Failing");
          }

        rw_assert (success, 0, __LINE__, 
                   "%zu. generate (): buf[%zu]: %d != %d",
                   i + 1, j, last_val + j + 1, (begin.cur_ + j)->val_ );

        if (!success)
            break;

        // verify 25.2.6, p3
#ifndef __SYMBIAN32__
        success = T::n_total_op_assign_ - last_n_op_assign == i + 1;
        rw_assert (success, 0, __LINE__, 
                   "%zu. generate (): complexity: %zu != %zu",
                   i + 1, T::n_total_op_assign_ - last_n_op_assign, i + 1);
#else
         success = n_total_op_assign_ - last_n_op_assign == i + 1;
        rw_assert (success, 0, __LINE__, 
                   "%zu. generate (): complexity: %zu != %zu",
                   i + 1, n_total_op_assign_ - last_n_op_assign, i + 1);
       if(!success)
         {
           failures++;
           std_log(LOG_FILENAME_LINE,"Reason: Failing");
          }
#endif 
      
         
        if (!success)
            break;
    }
  delete[] buf;

}

/**************************************************************************/

// exercises std::generate_n() 
template <class ForwardIterator, class Size, class T>
void test_generate_n (std::size_t            N,
                      const ForwardIterator &gen_iter,
                      const Size*,
                      const T*)
{
    static const char* const itname  = type_name (gen_iter, (T*) 0);
    static const char* const szname  = "Size";
    static const char* const genname = "Generator";

    rw_info (0, 0, 0, 
             "void std::generate_n (%s, %s, %s)", itname, szname, genname);

    // generate sequential values for each default constructed T
    #ifndef __SYMBIAN32__
    T::gen_ = gen_seq;
    #else
    gen_ = gen_seq;
    #endif

    T *buf = new X [N];

    for (std::size_t i = 0; i <= N; ++i) {

        #ifndef __SYMBIAN32__
        std::size_t last_n_op_assign = T::n_total_op_assign_;
        #else
        std::size_t last_n_op_assign = n_total_op_assign_;
        #endif

        T* const buf_end = buf + i + 1;

        const ForwardIterator begin =
            make_iter (buf, buf, buf_end, gen_iter);

        const Size sz (i, 0);
        const Generator<T> gen (0, 0);

        // store the value of the next element
        const int last_val = Generator<T>(0, 0)().val_;

        std::generate_n (begin, sz, gen);

        bool success = true;

        // verify 25.2.6, p2
        std::size_t j = 0;
        for ( ; j != i; ++j) {
            success = (begin.cur_ + j)->val_ == int (last_val + j + 1);
            if (!success)
                break;
        } 
         if(!success)
         {
           failures++;
           std_log(LOG_FILENAME_LINE,"Reason: Failing");
          }

        rw_assert (success, 0, __LINE__, 
                   "%zu. generate_n (): buf[%zu]: %d != %d",
                   i + 1, j, last_val + j + 1, (begin.cur_ + j)->val_ );

        if (!success)
            break;

        // verify 25.2.6, p3
        #ifndef __SYMBIAN32__
        success = T::n_total_op_assign_ - last_n_op_assign == i;
        rw_assert (success, 0, __LINE__, 
                   "%zu. generate_n (): complexity: %zu != %zu",
                   i + 1, T::n_total_op_assign_ - last_n_op_assign, i);
        #else
               success = n_total_op_assign_ - last_n_op_assign == i;
        rw_assert (success, 0, __LINE__, 
                   "%zu. generate_n (): complexity: %zu != %zu",
                   i + 1, n_total_op_assign_ - last_n_op_assign, i);
         
          if(!success)
         {
           failures++;
           std_log(LOG_FILENAME_LINE,"Reason: Failing");
          }
       #endif
        if (!success)
            break;
    }

//   #ifndef __SYNBIAN32__
  // ::operator  delete(buf);
   //#else
     delete[] buf;
   //#endif
}

/**************************************************************************/

/* extern */ int rw_opt_nloops = 32;     // --nloops
/* extern */ int rw_opt_no_output_iter;  // --no-OutputIterator
/* extern */ int rw_opt_no_fwd_iter;     // --no-ForwardIterator
/* extern */ int rw_opt_no_bidir_iter;   // --no-BidirectionalIterator
/* extern */ int rw_opt_no_rnd_iter;     // --no-RandomAccessIterator

static void
test_generate (const std::size_t N)
{
    rw_info (0, 0, 0,
             "template <class %s, class %s> "
             "void std::generate (%1$s, %1$s, %2$s&)",
             "ForwardIterator", "Generator");

    if (rw_opt_no_fwd_iter) {
        rw_note (0, __FILE__, __LINE__, "ForwardIterator test disabled");
    }
    else {
        test_generate (N, FwdIter<X>(), (X*)0);
    }

    if (rw_opt_no_bidir_iter) {
        rw_note (0, __FILE__, __LINE__, "BidirectionalIterator test disabled");
    }
    else {
        test_generate (N, BidirIter<X>(), (X*)0);
    }

    if (rw_opt_no_rnd_iter) {
        rw_note (0, __FILE__, __LINE__,  "RandomAccessIterator test disabled");
    }
    else {
        test_generate (N, RandomAccessIter<X>(), (X*)0);
    }
}

/**************************************************************************/

static void
test_generate_n (const std::size_t N)
{
    rw_info (0, 0, 0,
             "template <class %s, class %s, class %s> "
             "void std::generate_n (%1$s, %2$s, const %3$s&)",
             "OutputIterator", "Size", "Generator");

    if (rw_opt_no_output_iter) {
        rw_note (0, __FILE__, __LINE__, "OutputIterator test disabled");
    }
    else {
        test_generate_n (N, OutputIter<X>(0, 0, 0), (Size*)0, (X*)0);
    }

    if (rw_opt_no_fwd_iter) {
        rw_note (0, __FILE__, __LINE__, "ForwardIterator test disabled");
    }
    else {
        test_generate_n (N, FwdIter<X>(), (Size*)0, (X*)0);
    }

    if (rw_opt_no_bidir_iter) {
        rw_note (0, __FILE__, __LINE__, "BidirectionalIterator test disabled");
    }
    else {
        test_generate_n (N, BidirIter<X>(), (Size*)0, (X*)0);
    }

    if (rw_opt_no_rnd_iter) {
        rw_note (0, __FILE__, __LINE__, "RandomAccessIterator test disabled");
    }
    else {
        test_generate_n (N, RandomAccessIter<X>(), (Size*)0, (X*)0);
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

    test_generate (N);

    test_generate_n (N);

    return 0;
}

/**************************************************************************/

int main (int argc, char *argv[])
{
  std_log(LOG_FILENAME_LINE,"[Test Case for generate]");
  #ifndef __SYMBIAN32__
    return rw_test (argc, argv, __FILE__,
                    "lib.alg.generate",
                    0 /* no comment */, run_test,
                    "|-nloops# "
                    "|-no-OutputIterator# "
                    "|-no-ForwardIterator# "
                    "|-no-BidirectionalIterator# "
                    "|-no-RandomAccessIterator#",
                    &rw_opt_nloops,
                    &rw_opt_no_output_iter,
                    &rw_opt_no_fwd_iter,
                    &rw_opt_no_bidir_iter,
                    &rw_opt_no_rnd_iter);
  #else 
    rw_test (argc, argv, __FILE__,
                    "lib.alg.generate",
                    0 /* no comment */, run_test,
                    "|-nloops# "
                    "|-no-OutputIterator# "
                    "|-no-ForwardIterator# "
                    "|-no-BidirectionalIterator# "
                    "|-no-RandomAccessIterator#",
                    &rw_opt_nloops,
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
  
   std_log(LOG_FILENAME_LINE,"[End Test Case]");  
   
   testResultXml("25_generate");
    close_log_file();
  return failures;
  #endif
   
}
