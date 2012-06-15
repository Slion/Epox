

// -*- C++ -*-
/***************************************************************************
 *
 * 2.smartptr.weak.cpp - test exercising class template weak_ptr
 *
 * $Id: 2.smartptr.weak.cpp 354223 2005-12-05 23:29:59Z sebor $
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

#include <cassert>
#include <tr1/_smartptr.h>
#include<rw/_exception.h>
#include <driver.h>
#include"std_log_result.h"
#define LOG_FILENAME_LINE __FILE__, __LINE__
int failures=0;

/**************************************************************************/

template <class T>
struct Base
{
    static int n_dtors;

    virtual ~Base () {
        ++n_dtors;
    }
};

template <class T>
int Base<T>::n_dtors;


template <class T>
struct Derived: Base<T>
{
    static int n_dtors;

    virtual ~Derived () {
        ++n_dtors;
    }
};

template <class T>
int Derived<T>::n_dtors;


template <class T>
struct Derived2: Base<T>
{
    static int n_dtors;

    virtual ~Derived2 () {
        ++n_dtors;
    }
};

template <class T>
int Derived2<T>::n_dtors;


template <class T>
struct Deleter
{
    static int n_def_ctors;
    static int n_cpy_ctors;
    static int n_dtors;
    static int n_funcalls;

    Deleter () {
        ++n_def_ctors;
    }

    Deleter (const Deleter&) {
        ++n_cpy_ctors;
    }

    virtual ~Deleter () {
        ++n_dtors;
    }

    void operator()(T *p) {
        ++n_funcalls;
        delete p;
    }

private:
    void operator=(Deleter&);   // not defined
};

template <class T>
int Deleter<T>::n_def_ctors;

template <class T>
int Deleter<T>::n_cpy_ctors;

template <class T>
int Deleter<T>::n_dtors;

template <class T>
int Deleter<T>::n_funcalls;

/**************************************************************************/

static void
test_ctor ()
{
    rw_info (0, "tr.util.smartptr.weak.cons", 0,
             "weak_ptr constructors");

    {   // default ctor
        std::tr1::weak_ptr<char> weak;
        rw_assert (0 == weak.use_count (), 0, __LINE__, "");
        if(0!=weak.use_count ())
          {
            failures++;
            std_log(LOG_FILENAME_LINE,"Reason:Failing");
          }
    }

    {   // shared_ptr converting ctor
        std::tr1::shared_ptr<char> shared;
        std::tr1::weak_ptr<char>   weak (shared);
        rw_assert (0 == weak.use_count (), 0, __LINE__, "");
        if(0 != weak.use_count ())
          {
            failures++;
            std_log(LOG_FILENAME_LINE,"Reason:Failing");
          }

    }

    {   // shared_ptr converting ctor
        std::tr1::shared_ptr<char> shared;
        std::tr1::weak_ptr<void>   weak (shared);
        rw_assert (0 == weak.use_count (), 0, __LINE__, "");
        if(0!=weak.use_count ())
          {
            failures++;
            std_log(LOG_FILENAME_LINE,"Reason:Failing");
          }

    }

    {   // shared_ptr converting ctor
        int* const p = new int (__LINE__);
        std::tr1::shared_ptr<int> shared (p);
        std::tr1::weak_ptr<int>   weak (shared);
        rw_assert (shared.use_count () == weak.use_count (), 0, __LINE__, "");
        if(shared.use_count () != weak.use_count ())
          {
            failures++;
            std_log(LOG_FILENAME_LINE,"Reason:Failing");
          }

    }

    {   // shared_ptr converting ctor
        int* const p = new int (__LINE__);
        std::tr1::shared_ptr<int> shared0 (p);
        std::tr1::shared_ptr<int> shared1 (shared0);
        std::tr1::weak_ptr<int>   weak (shared1);
        rw_assert (shared1.use_count () == weak.use_count (), 0, __LINE__, "");
        
        if(shared1.use_count () != weak.use_count ())
        {
         failures++;
         std_log(LOG_FILENAME_LINE,"Reason: Failing");
         }
    }


    {   // shared_ptr converting ctor
        int* const p = new int (__LINE__);
        std::tr1::shared_ptr<int> shared (p);
        std::tr1::weak_ptr<void>  weak (shared);
        shared.reset ();

        rw_assert (0 == weak.use_count (), 0, __LINE__, "");
        if(0 != weak.use_count ())
        {
         failures++;
         std_log(LOG_FILENAME_LINE,"Reason: Failing");
         }

    }

    {   // shared_ptr converting ctor
        Derived<int>* const p = new Derived<int>;
        std::tr1::shared_ptr<Derived<int> > shared (p);
        std::tr1::weak_ptr<Base<int> >      weak (shared);
        shared.reset ();

        rw_assert (0 == weak.use_count (), 0, __LINE__, "");
        if(0 != weak.use_count ())
        {
         failures++;
         std_log(LOG_FILENAME_LINE,"Reason: Failing");
         }

    }
}

/**************************************************************************/

static void
test_copy_ctor ()
{
    rw_info (0, "tr.util.smartptr.weak.copy", 0,
             "weak_ptr copy constructors");

    {
        // weak_ptr (const weak_ptr&)
        std::tr1::weak_ptr<void> weak;
        std::tr1::weak_ptr<void> copy (weak);
        rw_assert (0 == weak.use_count (), 0, __LINE__, "");
        if(0 != weak.use_count ())
        {
         failures++;
         std_log(LOG_FILENAME_LINE,"Reason: Failing");
         }

        rw_assert (0 == copy.use_count (), 0, __LINE__, "");
        if(0 != copy.use_count ())
        {
         failures++;
         std_log(LOG_FILENAME_LINE,"Reason: Failing");
         }

    }

    {
        // weak_ptr (const weak_ptr&)
        int* const p = new int (__LINE__);
        std::tr1::shared_ptr<int> shared (p);
        std::tr1::weak_ptr<int>   weak (shared);
        std::tr1::weak_ptr<int>   copy (weak);

        rw_assert (weak.use_count () == shared.use_count (), 0, __LINE__, "");
        if(weak.use_count () != shared.use_count ())
        {
         failures++;
         std_log(LOG_FILENAME_LINE,"Reason: Failing");
         }

        rw_assert (copy.use_count () == shared.use_count (), 0, __LINE__, "");
        if(copy.use_count () != shared.use_count ())
        {
         failures++;
         std_log(LOG_FILENAME_LINE,"Reason: Failing");
         }

    }

    {
        // weak_ptr (const weak_ptr&)
        int* const p = new int (__LINE__);
        std::tr1::shared_ptr<int> shared0 (p);
        std::tr1::shared_ptr<int> shared1 (shared0);
        std::tr1::weak_ptr<int>   weak (shared1);
        std::tr1::weak_ptr<int>   copy (weak);

        rw_assert (weak.use_count () == shared1.use_count (), 0, __LINE__, "");
        if(weak.use_count () != shared1.use_count ())
        {
         failures++;
         std_log(LOG_FILENAME_LINE,"Reason: Failing");
         }

        rw_assert (copy.use_count () == shared1.use_count (), 0, __LINE__, "");
        if(copy.use_count () != shared1.use_count ())
        {
         failures++;
         std_log(LOG_FILENAME_LINE,"Reason: Failing");
         }

    }

    {
        // weak_ptr (const weak_ptr&)
        int* const p = new int (__LINE__);
        std::tr1::shared_ptr<int> shared0 (p);
        std::tr1::shared_ptr<int> shared1 (shared0);
        std::tr1::weak_ptr<int>   weak (shared1);
        std::tr1::weak_ptr<void>  copy (weak);

        rw_assert (weak.use_count () == shared1.use_count (), 0, __LINE__, "");
        if(weak.use_count () != shared1.use_count ())
        {
         failures++;
         std_log(LOG_FILENAME_LINE,"Reason: Failing");
         }

        rw_assert (copy.use_count () == shared1.use_count (), 0, __LINE__, "");
        if(copy.use_count () != shared1.use_count ())
        {
         failures++;
         std_log(LOG_FILENAME_LINE,"Reason: Failing");
         }

    }

    {
        // weak_ptr (const weak_ptr&)
        Derived<int>* const p = new Derived<int>;
        std::tr1::shared_ptr<Derived<int> > shared0 (p);
        std::tr1::shared_ptr<Derived<int> > shared1 (shared0);
        std::tr1::weak_ptr<Derived<int> >   weak (shared1);
        std::tr1::weak_ptr<Base<int> >      copy (weak);

        rw_assert (weak.use_count () == shared1.use_count (), 0, __LINE__, "");
        if(weak.use_count () != shared1.use_count ())
        {
         failures++;
         std_log(LOG_FILENAME_LINE,"Reason: Failing");
         }

        rw_assert (copy.use_count () == shared1.use_count (), 0, __LINE__, "");
        if(copy.use_count () != shared1.use_count ())
        {
         failures++;
         std_log(LOG_FILENAME_LINE,"Reason: Failing");
         }

        shared1.reset ();
        rw_assert (copy.use_count () == weak.use_count (), 0, __LINE__, "");
        if(copy.use_count () != weak.use_count ())
        {
         failures++;
         std_log(LOG_FILENAME_LINE,"Reason: Failing");
         }

        rw_assert (copy.use_count () == shared0.use_count (), 0, __LINE__, "");
        if(copy.use_count () != shared0.use_count ())
        {
         failures++;
         std_log(LOG_FILENAME_LINE,"Reason: Failing");
         }

        shared0.reset ();
        rw_assert (copy.use_count () == weak.use_count (), 0, __LINE__, "");
        if(copy.use_count () != weak.use_count ())
        {
         failures++;
         std_log(LOG_FILENAME_LINE,"Reason: Failing");
         }

        rw_assert (copy.use_count () == shared0.use_count (), 0, __LINE__, "");
        if(copy.use_count () != shared0.use_count ())
        {
         failures++;
         std_log(LOG_FILENAME_LINE,"Reason: Failing");
         }

    }
}

/**************************************************************************/

static void
test_dtor ()
{
    rw_info (0, "tr.util.smartptr.weak.dest", 0,
             "weak_ptr destructor");

    {
        // ~weak_ptr()
        const int base_dtors = Base<int>::n_dtors;
        {
            std::tr1::weak_ptr<Base<int> > weak;
        }
        rw_assert (base_dtors == Base<int>::n_dtors, 0, __LINE__, "");
        if(base_dtors != Base<int>::n_dtors)
        {
         failures++;
         std_log(LOG_FILENAME_LINE,"Reason: Failing");
         }

    }

    {
        // ~weak_ptr()
        const int base_dtors = Base<int>::n_dtors;
        Base<int>* const p = new Base<int>;
        std::tr1::shared_ptr<Base<int> > shared (p);
        {
            std::tr1::weak_ptr<Base<int> > weak (shared);
        }
        rw_assert (base_dtors == Base<int>::n_dtors, 0, __LINE__, "");
        if(base_dtors != Base<int>::n_dtors)
        {
         failures++;
         std_log(LOG_FILENAME_LINE,"Reason: Failing");
         }

    }

    {
        // ~weak_ptr()
        const int base_dtors    = Base<int>::n_dtors;
        const int derived_dtors = Derived<int>::n_dtors;

        Derived<int>* const p = new Derived<int>;
        std::tr1::shared_ptr<Derived<int> > shared (p);

        {
            std::tr1::weak_ptr<Base<int> > weak (shared);
        }
        rw_assert (base_dtors    == Base<int>::n_dtors, 0, __LINE__, "");
        if(base_dtors   != Base<int>::n_dtors)
        {
         failures++;
         std_log(LOG_FILENAME_LINE,"Reason: Failing");
         }

        rw_assert (derived_dtors == Derived<int>::n_dtors, 0, __LINE__, "");
        if(derived_dtors != Derived<int>::n_dtors)
        {
         failures++;
         std_log(LOG_FILENAME_LINE,"Reason: Failing");
         }

    }

    {
        // ~weak_ptr()
        const int base_dtors    = Base<int>::n_dtors;
        const int derived_dtors = Derived<int>::n_dtors;

        Derived<int>* const p = new Derived<int>;
        std::tr1::shared_ptr<Derived<int> > shared (p);

        {
            std::tr1::weak_ptr<Base<int> > weak (shared);
            std::tr1::weak_ptr<Base<int> > copy (shared);
        }
        rw_assert (base_dtors    == Base<int>::n_dtors, 0, __LINE__, "");
        if(base_dtors   != Base<int>::n_dtors)
        {
         failures++;
         std_log(LOG_FILENAME_LINE,"Reason: Failing");
         }

        rw_assert (derived_dtors == Derived<int>::n_dtors, 0, __LINE__, "");
        if(derived_dtors != Derived<int>::n_dtors)
        {
         failures++;
         std_log(LOG_FILENAME_LINE,"Reason: Failing");
         }

    }

    {
        // ~weak_ptr()
        const int base_dtors    = Base<int>::n_dtors;
        const int derived_dtors = Derived<int>::n_dtors;

        Derived<int>* const p = new Derived<int>;
        std::tr1::shared_ptr<Derived<int> > shared (p);

        {
            std::tr1::weak_ptr<Base<int> > weak (shared);
            shared.reset ();
        }
        rw_assert (base_dtors + 1    == Base<int>::n_dtors, 0, __LINE__, "");
        if(base_dtors + 1    != Base<int>::n_dtors)
        {
         failures++;
         std_log(LOG_FILENAME_LINE,"Reason: Failing");
         }

        rw_assert (derived_dtors + 1 == Derived<int>::n_dtors, 0, __LINE__, "");
        if(derived_dtors + 1 != Derived<int>::n_dtors)
        {
         failures++;
         std_log(LOG_FILENAME_LINE,"Reason: Failing");
         }

    }

    {
        // ~weak_ptr()
        const int base_dtors    = Base<int>::n_dtors;
        const int derived_dtors = Derived<int>::n_dtors;

        std::tr1::weak_ptr<Derived<int> > weak;

        {
            Derived<int>* const p = new Derived<int>;
            std::tr1::shared_ptr<Derived<int> > shared (p);

            weak = shared;
        }
        rw_assert (base_dtors + 1    == Base<int>::n_dtors, 0, __LINE__, "");
        if(base_dtors + 1   != Base<int>::n_dtors)
        {
         failures++;
         std_log(LOG_FILENAME_LINE,"Reason: Failing");
         }

        rw_assert (derived_dtors + 1 == Derived<int>::n_dtors, 0, __LINE__, "");
        if(derived_dtors + 1 != Derived<int>::n_dtors)
        {
         failures++;
         std_log(LOG_FILENAME_LINE,"Reason: Failing");
         }

    }
}

/**************************************************************************/

static void
test_assign ()
{
    rw_info (0, "tr.util.smartptr.weak.assign", 0,
             "weak_ptr assignment operators");

#if 0

    {   // operator=(const weak_ptr&)
        std::tr1::weak_ptr<void> ptr0;
        std::tr1::weak_ptr<void> ptr1;

        ptr1 = ptr0;

        rw_assert (ptr1.get () == ptr0.get (), 0, __LINE__, "");
        if(ptr1.get () != ptr0.get ())
        {
         failures++;
         std_log(LOG_FILENAME_LINE,"Reason: Failing");
         }

        rw_assert (ptr1.use_count () == ptr0.use_count (), 0, __LINE__, "");
        if(ptr1.use_count () != ptr0.use_count ())
        {
         failures++;
         std_log(LOG_FILENAME_LINE,"Reason: Failing");
         }

    }

    {   // operator=(const weak_ptr&)
        int* const p = new int (__LINE__);
        std::tr1::weak_ptr<int> ptr0 (p);
        std::tr1::weak_ptr<int> ptr1;

        ptr1 = ptr0;

        rw_assert (ptr1.get () == ptr0.get (), 0, __LINE__, "");
        if(ptr1.get () != ptr0.get ())
        {
         failures++;
         std_log(LOG_FILENAME_LINE,"Reason: Failing");
         }

        rw_assert (ptr1.use_count () == ptr0.use_count (), 0, __LINE__, "");
        if(ptr1.use_count () != ptr0.use_count ())
        {
         failures++;
         std_log(LOG_FILENAME_LINE,"Reason: Failing");
         }

    }

    {   // template <class U> operator=(const weak_ptr<U>&)
        int* const p = new int (__LINE__);
        std::tr1::weak_ptr<int> ptr0 (p);
        std::tr1::weak_ptr<void> ptr1;

        ptr1 = ptr0;

        rw_assert (ptr1.get () == ptr0.get (), 0, __LINE__, "");
        if(ptr1.get () != ptr0.get ())
        {
         failures++;
         std_log(LOG_FILENAME_LINE,"Reason: Failing");
         }

        rw_assert (ptr1.use_count () == ptr0.use_count (), 0, __LINE__, "");
        if(ptr1.use_count () != ptr0.use_count ())
        {
         failures++;
         std_log(LOG_FILENAME_LINE,"Reason: Failing");
         }

    }

    {   // template <class U> operator=(const weak_ptr<U>&)
        Derived<int>* const p = new Derived<int>;
        std::tr1::weak_ptr<Derived<int> > ptr0 (p);
        std::tr1::weak_ptr<Base<int> > ptr1;

        ptr1 = ptr0;

        rw_assert (ptr1.get () == ptr0.get (), 0, __LINE__, "");
        if(ptr1.get () != ptr0.get ())
        {
         failures++;
         std_log(LOG_FILENAME_LINE,"Reason: Failing");
         }

        rw_assert (ptr1.use_count () == ptr0.use_count (), 0, __LINE__, "");
        if(ptr1.use_count () != ptr0.use_count ())
        {
         failures++;
         std_log(LOG_FILENAME_LINE,"Reason: Failing");
         }

    }

    {   // template <class U> operator=(const weak_ptr<U>&)
        Derived<int>* const p = new Derived<int>;
        std::tr1::weak_ptr<Base<int> > ptr0 (p);
        std::tr1::weak_ptr<void>       ptr1;

        ptr1 = ptr0;

        rw_assert (ptr1.get () == ptr0.get (), 0, __LINE__, "");
        if(ptr1.get () != ptr0.get ())
        {
         failures++;
         std_log(LOG_FILENAME_LINE,"Reason: Failing");
         }

        rw_assert (ptr1.use_count () == ptr0.use_count (), 0, __LINE__, "");
        if(ptr1.use_count () != ptr0.use_count ())
        {
         failures++;
         std_log(LOG_FILENAME_LINE,"Reason: Failing");
         }

    }

#endif   // 0/1

}

/**************************************************************************/

static void
test_modifiers ()
{
    rw_info (0, "tr.util.smartptr.weak.mod", 0,
             "weak_ptr modifiers");

    rw_warn (0, 0, 0,
             "weak_ptr modifiers not exercised");
}

/**************************************************************************/

static void
test_observers ()
{
    rw_info (0, "tr.util.smartptr.weak.obs", 0,
             "weak_ptr observers");

    {   // use_count()
        std::tr1::weak_ptr<void> weak;
        rw_assert (0 == weak.use_count (), 0, __LINE__, "");
        if(0 != weak.use_count ())
        {
         failures++;
         std_log(LOG_FILENAME_LINE,"Reason: Failing");
         }

    }

    {   // expired()
        std::tr1::weak_ptr<void> weak;
        rw_assert (weak.expired (), 0, __LINE__, "");
        if(!weak.expired ())
        {
         failures++;
         std_log(LOG_FILENAME_LINE,"Reason: Failing");
         }

    }

    {   // expired()
        std::tr1::shared_ptr<void> shared;
        std::tr1::weak_ptr<void>   weak (shared);
        rw_assert (weak.expired (), 0, __LINE__, "");
        if(!weak.expired ())
        {
         failures++;
         std_log(LOG_FILENAME_LINE,"Reason: Failing");
         }

    }

    {   // expired()
        int* const p = new int (__LINE__);
        std::tr1::shared_ptr<void> shared (p);
        std::tr1::weak_ptr<void>   weak (shared);
        rw_assert (!weak.expired (), 0, __LINE__, "");
        if(weak.expired ())
        {
         failures++;
         std_log(LOG_FILENAME_LINE,"Reason: Failing");
         }

    }

    {   // expired()
        int* const p = new int (__LINE__);
        std::tr1::shared_ptr<void> shared (p);
        std::tr1::weak_ptr<void>   weak (shared);
        shared.reset ();
        rw_assert (weak.expired (), 0, __LINE__, "");
        if(!weak.expired ())
        {
         failures++;
         std_log(LOG_FILENAME_LINE,"Reason: Failing");
         }

    }

    {   // lock()
        std::tr1::weak_ptr<void>   weak;
        std::tr1::shared_ptr<void> shared (weak.lock ());
        
        rw_assert (0 == weak.use_count (), 0, __LINE__,
                   "weak_ptr<void>::use_count() == 1, got %ld",
                   weak.use_count ());
        if(0 != weak.use_count ())
        {
         failures++;
         std_log(LOG_FILENAME_LINE,"Reason: Failing");
         }

        rw_assert (0 == shared.use_count (), 0, __LINE__,
                   "shared_ptr<void>::use_count() == 1, got %ld",
                   shared.use_count ());
        if(0 != shared.use_count ())
        {
         failures++;
         std_log(LOG_FILENAME_LINE,"Reason: Failing");
         }

    }

    {   // lock()
        int* const p = new int (__LINE__);
        std::tr1::shared_ptr<void> shared0 (p);
        std::tr1::weak_ptr<void>   weak (shared0);
        std::tr1::shared_ptr<void> shared1 (weak.lock ());
        
        rw_assert (2 == weak.use_count (), 0, __LINE__,
                   "weak_ptr<void>::use_count() == 2, got %ld",
                   weak.use_count ());
        if(2 != weak.use_count ())
        {
         failures++;
         std_log(LOG_FILENAME_LINE,"Reason: Failing");
         }

        rw_assert (2 == shared1.use_count (), 0, __LINE__,
                   "shared_ptr<void>::use_count() == 2, got %ld",
                   shared1.use_count ());
        if(2 != shared1.use_count ())
        {
         failures++;
         std_log(LOG_FILENAME_LINE,"Reason: Failing");
         }

    }

    {   // lock()
        int* const p = new int (__LINE__);
        std::tr1::shared_ptr<void> shared0 (p);
        std::tr1::weak_ptr<void>   weak (shared0);
        std::tr1::shared_ptr<void> shared1 (weak.lock ());
        std::tr1::shared_ptr<void> shared2 (shared1);
        
        rw_assert (3 == weak.use_count (), 0, __LINE__,
                   "weak_ptr<void>::use_count() == 3, got %ld",
                   weak.use_count ());
        if(3 != weak.use_count ())
        {
         failures++;
         std_log(LOG_FILENAME_LINE,"Reason: Failing");
         }

        rw_assert (3 == shared0.use_count (), 0, __LINE__,
                   "shared_ptr<void>::use_count() == 3, got %ld",
                   shared1.use_count ());
        if(3 != shared0.use_count ())
        {
         failures++;
         std_log(LOG_FILENAME_LINE,"Reason: Failing");
         }


        shared0.reset ();

        rw_assert (2 == weak.use_count (), 0, __LINE__,
                   "weak_ptr<void>::use_count() == 2, got %ld",
                   weak.use_count ());

        if(2 != weak.use_count ())
        {
         failures++;
         std_log(LOG_FILENAME_LINE,"Reason: Failing");
         }

        shared1.reset ();

        rw_assert (1 == weak.use_count (), 0, __LINE__,
                   "weak_ptr<void>::use_count() == 1, got %ld",
                   weak.use_count ());

        if(1 != weak.use_count ())
        {
         failures++;
         std_log(LOG_FILENAME_LINE,"Reason: Failing");
         }

        shared2.reset ();


        rw_assert (0 == weak.use_count (), 0, __LINE__,
                   "weak_ptr<void>::use_count() == 0, got %ld",
                   weak.use_count ());
        if(0 != weak.use_count ())
        {
         failures++;
         std_log(LOG_FILENAME_LINE,"Reason: Failing");
         }

    }
}

/**************************************************************************/

static void
test_comparison ()
{
    rw_info (0, "tr.util.smartptr.weak.cmp", 0,
             "weak_ptr comparison");

    rw_warn (0, 0, 0,
             "weak_ptr comparison not exercised");
}

/**************************************************************************/

static void
test_specialized ()
{
    rw_info (0, "tr.util.smartptr.weak.spec", 0,
             "weak_ptr specialized algorithms");

    rw_warn (0, 0, 0,
             "weak_ptr specialized algorithms not exercised");
}

/**************************************************************************/

static int no_ctor; 
static int no_copy_ctor; 
static int no_dtor; 
static int no_assign; 
static int no_modifiers; 
static int no_observers; 
static int no_comparison; 
static int no_specialized; 


static int
run_test (int, char*[])
{
#define TEST(what)                                              \
    if (no_ ## what) {                                          \
        rw_note (0, 0, __LINE__, "%s test disabled", #what);    \
    }                                                           \
    else {                                                      \
        test_ ## what ();                                       \
    } typedef void unused_typedef

    TEST (ctor);
    TEST (copy_ctor);
    TEST (dtor);
    TEST (assign);
    TEST (modifiers);
    TEST (observers);
    TEST (comparison);
    TEST (specialized);

    return 0;
}

/**************************************************************************/

int main (int argc, char *argv[])
{
    std_log(LOG_FILENAME_LINE,"[Test Case for smartptr_shared]");

  #ifndef __SYMBIAN32__  
    return rw_test (argc, argv, __FILE__,
                    "tr.util.smartptr.weak",
                    0 /* no comment */, run_test,
                    "|-no-ctor# "
                    "|-no-copy_ctor# "
                    "|-no-dtor# "
                    "|-no-assign# "
                    "|-no-modifiers# "
                    "|-no-observers# "
                    "|-no-comparison# "
                    "|-no-specialized# ",
                    &no_ctor, 
                    &no_copy_ctor, 
                    &no_dtor, 
                    &no_assign, 
                    &no_modifiers, 
                    &no_observers, 
                    &no_comparison, 
                    &no_specialized);
 #else
    rw_test (argc, argv, __FILE__,
                    "tr.util.smartptr.weak",
                    0 /* no comment */, run_test,
                    "|-no-ctor# "
                    "|-no-copy_ctor# "
                    "|-no-dtor# "
                    "|-no-assign# "
                    "|-no-modifiers# "
                    "|-no-observers# "
                    "|-no-comparison# "
                    "|-no-specialized# ",
                    &no_ctor, 
                    &no_copy_ctor, 
                    &no_dtor, 
                    &no_assign, 
                    &no_modifiers, 
                    &no_observers, 
                    &no_comparison, 
                    &no_specialized);
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
   
   testResultXml("2_smartptr_weak");
    close_log_file();
  return failures;
#endif
}
