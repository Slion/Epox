// -*- C++ -*-
/***************************************************************************
 *
 * 2.smartptr.shared.cpp - test exercising class template shared_ptr
 *
 * $Id: 2.smartptr.shared.cpp 354223 2005-12-05 23:29:59Z sebor $
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
#include "tr1/_smartptr.h"
#include <driver.h>
#include"std_log_result.h"
#define LOG_FILENAME_LINE __FILE__, __LINE__
int failures=0;
/**************************************************************************/

int next_id;

template <class T>
struct Base
{
    static int n_dtors;

    int id;

    Base () { }

    void init () {
        id = ++next_id;
    }

    // Base dtor is not virtual in order to detect
    // deletions through a pointer to the wrong base
    /* NOT virtual */
    ~Base () {
        ++n_dtors;
    }
};


template <class T>
int Base<T>::n_dtors;


template <class T>
struct Base_0: virtual Base<T>
{
    static int n_dtors;

    T dummy;   // make body non-empty

    Base_0 (): Base<T>() { this->init (); }

    virtual ~Base_0 () {
        ++n_dtors;
    }
};

  

  
template <class T>
int Base_0<T>::n_dtors;

template <class T>
struct Base_1: virtual Base<T>
{
    static int n_dtors;

    T dummy;   // make body non-empty

    Base_1 (): Base<T>() { this->init (); }

    virtual ~Base_1 () {
        ++n_dtors;
    }
};
template <class T>
int Base_1<T>::n_dtors;

template <class T>
struct Derived: Base_0<T>, Base_1<T>
{
    static int n_dtors;

    T dummy;   // make body non-empty

    Derived (): Base_0<T>(), Base_1<T>() { }

    virtual ~Derived () {
        ++n_dtors;
    }
};
template <class T>
int Derived<T>::n_dtors;

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
    rw_info (0, "tr.util.smartptr.shared.cons", 0,
             "shared_ptr constructors");

    {   // default ctor
        std::tr1::shared_ptr<char> ptr;
       #ifndef __SYMBIAN32__ 
        rw_assert (0 == ptr.get (), 0, __LINE__, "");
        rw_assert (0 == ptr.use_count (), 0, __LINE__, "");
       #else 
        if(0 != ptr.get ())
          {
            failures++;
           std_log(LOG_FILENAME_LINE,"Reason:ptr.get() is failing");  
           }
         if(0 != ptr.use_count ())
          {
            failures++;
           std_log(LOG_FILENAME_LINE,"Reason:ptr.use_count () is failing");  
           }  
        #endif   
           
           
    }

    {   // default ctor
        std::tr1::shared_ptr<const short> ptr;
        #ifndef __SYMBIAN32__
        rw_assert (0 == ptr.get (), 0, __LINE__, "");
        rw_assert (0 == ptr.use_count (), 0, __LINE__, "");
        #else   
         if(0 != ptr.get ())
          {
            failures++;
           std_log(LOG_FILENAME_LINE,"Reason:ptr.get() is failing");  
           }
         if(0 != ptr.use_count ())
          {
            failures++;
           std_log(LOG_FILENAME_LINE,"Reason:ptr.use_count () is failing");  
           }  
         #endif
    }

    {   // default ctor
        std::tr1::shared_ptr<volatile int> ptr;
        #ifndef __SYMBIAN32__
        rw_assert (0 == ptr.get (), 0, __LINE__, "");
        rw_assert (0 == ptr.use_count (), 0, __LINE__, "");
        #else 
         if(0 != ptr.get ())
          {
            failures++;
           std_log(LOG_FILENAME_LINE,"Reason:ptr.get() is failing");  
           }
  
        if(0 != ptr.use_count ())
          {
            failures++;
           std_log(LOG_FILENAME_LINE,"Reason:ptr.use_count () is failing");  
           }  
          #endif
    }

    {   // default ctor
        std::tr1::shared_ptr<const volatile long> ptr;
        #ifndef __SYMBIAN32__
        rw_assert (0 == ptr.get (), 0, __LINE__, "");
        rw_assert (0 == ptr.use_count (), 0, __LINE__, "");
        #else 
         if(0 != ptr.get ())
          {
            failures++;
           std_log(LOG_FILENAME_LINE,"Reason:ptr.get() is failing");  
           }
       
         if(0 != ptr.use_count ())
          {
            failures++;
           std_log(LOG_FILENAME_LINE,"Reason:ptr.use_count () is failing");  
           }  
        #endif
    }

    {   // template <class U> shared_ptr(U*)
        const char* const p = new char ('A');
        std::tr1::shared_ptr<const char> ptr (p);
      #ifndef __SYMBIAN32__
      rw_assert (p == ptr.get (), 0, __LINE__, "");
      rw_assert (1 == ptr.use_count (), 0, __LINE__, "");
      #else
           if(p != ptr.get ())
          {
            failures++;
           std_log(LOG_FILENAME_LINE,"Reason:ptr.get() is failing");  
           }
        
        if(1 != ptr.use_count ())
          {
            failures++;
           std_log(LOG_FILENAME_LINE,"Reason:ptr.use_count () is failing");  
           }  
       #endif
    }

    {   // template <class U> shared_ptr(U*)
        const char* const p = new char ('A');
        std::tr1::shared_ptr<const void> ptr (p);
        #ifndef __SYMBIAN32__
        rw_assert (p == ptr.get (), 0, __LINE__, "");
        rw_assert (1 == ptr.use_count (), 0, __LINE__, ""); 
        #else
        if(p != ptr.get ())
          {
            failures++;
           std_log(LOG_FILENAME_LINE,"Reason:ptr.get() is failing");  
           }
             
        if(1 != ptr.use_count ())
          {
            failures++;
           std_log(LOG_FILENAME_LINE,"Reason:ptr.use_count () is failing");  
           }  
        #endif
    }

    {   // template <class U> shared_ptr(U*)
        short* const p = new short (__LINE__);
        std::tr1::shared_ptr<volatile void> ptr (p);
        #ifndef __SYMBIAN32__
        rw_assert (p == ptr.get (), 0, __LINE__, "");
        rw_assert (1 == ptr.use_count (), 0, __LINE__, "");
        #else
        if(p != ptr.get ())
          {
            failures++;
           std_log(LOG_FILENAME_LINE,"Reason:ptr.get() is failing");  
           }
       
        if(1 != ptr.use_count ())
          {
            failures++;
           std_log(LOG_FILENAME_LINE,"Reason:ptr.use_count () is failing");  
           }  
         #endif
    }

    {   // template <class U> shared_ptr(U*)
        double* const p = new double ();
        std::tr1::shared_ptr<const volatile void> ptr (p);
        #ifndef __SYMBIAN32__
        rw_assert (p == ptr.get (), 0, __LINE__, "");
        rw_assert (1 == ptr.use_count (), 0, __LINE__, "");
        #else
        if(p != ptr.get ())
          {
            failures++;
           std_log(LOG_FILENAME_LINE,"Reason:ptr.get() is failing");  
           }

        
        if(1 != ptr.use_count ())
          {
            failures++;
           std_log(LOG_FILENAME_LINE,"Reason:ptr.use_count () is failing");  
           }  
         #endif
    }

#if ILL_FORMED == -1 || ILL_FORMED == __LINE__

    {   // template <class U> shared_ptr(U*)
        char* const p = new char ('A');
        std::tr1::shared_ptr<int> ptr (p);
    }

#endif   // ILL_FORMED

    {   // template <class U> shared_ptr(U*)
        Derived<int>* const pd0 = new Derived<int>;
        Derived<int>* const pd1 = new Derived<int>;
        Derived<int>* const pd2 = new Derived<int>;
        Derived<int>* const pd3 = new Derived<int>;

        std::tr1::shared_ptr<const Derived<int> > ptr_d (pd0);
        std::tr1::shared_ptr<const Base_0<int> >  ptr_0 (pd1);
        std::tr1::shared_ptr<const Base_1<int> >  ptr_1 (pd2);
        std::tr1::shared_ptr<const Base<int> >    ptr_b (pd3);

        #ifndef __SYMBIAN32__
        rw_assert (pd0 == ptr_d.get (), 0, __LINE__,
                   "shared_ptr<Derived>(Derived* = %#p).get() == %#p, got %#p",
                   pd0, pd0, ptr_d.get ());
        rw_assert (pd1 == ptr_0.get (), 0, __LINE__,
                   "shared_ptr<Base_0>(Derived* = %#p).get() == %#p, got %#p",
                   pd1, (Base_0<int>*)pd1, ptr_0.get ());
        rw_assert (pd2 == ptr_1.get (), 0, __LINE__,
                   "shared_ptr<Base_1>(Derived* = %#p).get() == %#p, got %#p",
                   pd2, (Base_1<int>*)pd2, ptr_1.get ());
        rw_assert (pd3 == ptr_b.get (), 0, __LINE__,
                   "shared_ptr<Baee>(Derived* = %#p).get() == %#p, got %#p",
                   pd3, (Base<int>*)pd3, ptr_b.get ());
        rw_assert (1 == ptr_d.use_count (), 0, __LINE__, "");
        rw_assert (1 == ptr_0.use_count (), 0, __LINE__, "");
        rw_assert (1 == ptr_1.use_count (), 0, __LINE__, "");
        rw_assert (1 == ptr_b.use_count (), 0, __LINE__, "");
        #else
        if(pd0 != ptr_d.get ())
          {
            failures++;
           std_log(LOG_FILENAME_LINE,"Reason:ptr_d.get () is failing");  
           }
     
         if(pd1 != ptr_0.get ())
          {
            failures++;
           std_log(LOG_FILENAME_LINE,"Reason:ptr_0.get () is failing");  
           }
                   
        if(pd2 != ptr_1.get ())
          {
            failures++;
           std_log(LOG_FILENAME_LINE,"Reason:ptr_1.get () is failing");  
           }

        if(pd3 != ptr_b.get ())
          {
            failures++;
           std_log(LOG_FILENAME_LINE,"Reason:ptr_b.get() is failing");  
           }

        if(1 != ptr_d.use_count ())
          {
            failures++;
           std_log(LOG_FILENAME_LINE,"Reason:ptr_d.use_count () is failing");  
           }
        
        if(1 != ptr_0.use_count ())
          {
            failures++;
           std_log(LOG_FILENAME_LINE,"Reason:ptr_0.use_count () is failing");  
           }
        
        
        if(1 != ptr_1.use_count ())
          {
            failures++;
           std_log(LOG_FILENAME_LINE,"Reason:ptr_1.use_count () is failing");  
           }
        
        
        if(1 != ptr_b.use_count ())
          {
            failures++;
           std_log(LOG_FILENAME_LINE,"Reason:ptr_b.use_count () is failing");  
           }
         #endif
     
    }

    {   // template <class U, class D> shared_ptr(U*, D)
        int* const p = new int (__LINE__);
        Deleter<int> d;
        std::tr1::shared_ptr<int> ptr (p, d);
        #ifndef __SYMBIAN32__
        rw_assert (p == ptr.get (), 0, __LINE__, "");
        rw_assert (1 == ptr.use_count (), 0, __LINE__, "");
        #else
        if(p != ptr.get())
          {
            failures++;
           std_log(LOG_FILENAME_LINE,"Reason:ptr.get() is failing");  
           }
       
        if(1 != ptr.use_count ())
          {
            failures++;
           std_log(LOG_FILENAME_LINE,"Reason:ptr.use_count () is failing");  
           }
        #endif
    }
}

/**************************************************************************/                                                                                                                                                                                    

static void
test_copy_ctor ()
{
    rw_info (0, "tr.util.smartptr.shared.copy", 0,
             "shared_ptr copy constructors");

    {
        // shared_ptr (const shared_ptr&)
        std::tr1::shared_ptr<void> ptr;
        std::tr1::shared_ptr<void> cpy (ptr);
        
        #ifndef __SYMBIAN32__
        rw_assert (0 == ptr.get (), 0, __LINE__, "");
        rw_assert (0 == cpy.get (), 0, __LINE__, "");
        rw_assert (0 == ptr.use_count (), 0, __LINE__, "");
        rw_assert (0 == cpy.use_count (), 0, __LINE__, "");
        #else
        if(0 != ptr.get ())
          {
            failures++;
           std_log(LOG_FILENAME_LINE,"Reason:ptr.get() is failing");  
           }     
         if(0 != cpy.get ())
          {
            failures++;
           std_log(LOG_FILENAME_LINE,"Reason:cpy.get() is failing");  
           }     
               
        if(0 != ptr.use_count ())
          {
            failures++;
           std_log(LOG_FILENAME_LINE,"Reason:cpy.get() is failing");  
           }     
        
        if(0 != cpy.use_count ())
          {
            failures++;
           std_log(LOG_FILENAME_LINE,"Reason:cpy.get() is failing");  
           }     
         #endif
    }

    {
        // shared_ptr (const shared_ptr&)
        std::tr1::shared_ptr<double> ptr;
        std::tr1::shared_ptr<double> cpy (ptr);
     #ifndef __SYMBIAN32__
        rw_assert (ptr.get () == cpy.get (), 0, __LINE__, "");
        rw_assert (ptr.use_count () == cpy.use_count (), 0, __LINE__, "");
     #else  
       if(ptr.get () != cpy.get ())
          {
            failures++;
           std_log(LOG_FILENAME_LINE,"Reason:failing");  
           }     
      if(ptr.use_count () != cpy.use_count ())
          {
            failures++;
           std_log(LOG_FILENAME_LINE,"Reason:failing");  
           }     
      #endif    
    }

    {
        // shared_ptr (const shared_ptr&)
        int* const p = new int (__LINE__);
        std::tr1::shared_ptr<int> ptr (p);
        std::tr1::shared_ptr<int> cpy (ptr);
        #ifndef __SYMBIAN32__
        rw_assert (ptr.get () == cpy.get (), 0, __LINE__, "");
        rw_assert (ptr.use_count () == cpy.use_count (), 0, __LINE__, "");
        #else
        if(ptr.get () != cpy.get ())
          {
            failures++;
           std_log(LOG_FILENAME_LINE,"Reason:failing");  
           }     
                
        if(ptr.use_count () != cpy.use_count ())
          {
            failures++;
           std_log(LOG_FILENAME_LINE,"Reason:failing");  
           }     
        #endif
    }

    {
        // shared_ptr (const shared_ptr&)
        int* const p = new int (__LINE__);
        std::tr1::shared_ptr<int> ptr (p);
        std::tr1::shared_ptr<void> cpy (ptr);
        #ifndef __SYMBIAN32__
        rw_assert (ptr.get () == cpy.get (), 0, __LINE__, "");
        rw_assert (ptr.use_count () == cpy.use_count (), 0, __LINE__, "");
        #else
        if(ptr.get () != cpy.get ())
          {
            failures++;
           std_log(LOG_FILENAME_LINE,"Reason:failing");  
           }     
       
        if(ptr.use_count () != cpy.use_count ())
          {
            failures++;
           std_log(LOG_FILENAME_LINE,"Reason:failing");  
           }     
        #endif  
    }

#if ILL_FORMED == -1 || ILL_FORMED == __LINE__

    {
        // shared_ptr (const shared_ptr&)
        int* const p = new int (__LINE__);
        std::tr1::shared_ptr<void> ptr (p);
        std::tr1::shared_ptr<int> cpy (ptr);
        #ifndef __SYMBIAN32__
        rw_assert (ptr.get () == cpy.get (), 0, __LINE__, "");
        rw_assert (ptr.use_count () == cpy.use_count (), 0, __LINE__, "");
        #else 
         if(ptr.get () != cpy.get ())
          {
            failures++;
           std_log(LOG_FILENAME_LINE,"Reason:failing");  
           }     
     
        if(ptr.use_count () != cpy.use_count ())
          {
            failures++;
           std_log(LOG_FILENAME_LINE,"Reason:failing");  
           }     
         #endif
    }

#endif   // ILL_FORMED

    {   // template <class U> shared_ptr(const shared_ptr<U>&)
        Derived<int>* const pd = new Derived<int>;

        std::tr1::shared_ptr<volatile Derived<int> > ptr_d (pd);
        std::tr1::shared_ptr<volatile Base_0<int> >  ptr_0 (ptr_d);
        std::tr1::shared_ptr<volatile Base_1<int> >  ptr_1 (ptr_d);
        std::tr1::shared_ptr<volatile Base<int> >    ptr_b (ptr_d);
    #ifndef __SYMBIAN32__
        rw_assert (pd == ptr_d.get (), 0, __LINE__,
                   "shared_ptr<Derived>(Derived* = %#p).get() == %#p, got %#p",
                   pd, pd, ptr_d.get ());
      rw_assert (ptr_d.get () == ptr_0.get (), 0, __LINE__,
                   "shared_ptr<Base_0>(const shared_ptr<Derived>(%#p))"
                   ".get() == %#p, got %#p",
                   ptr_d.get (),
                   (volatile Base_0<int>*)ptr_d.get (), ptr_0.get ());
        rw_assert (ptr_d.get () == ptr_1.get (), 0, __LINE__,
                   "shared_ptr<Base_1>(const shared_ptr<Derived>(%#p))"
                   ".get() == %#p, got %#p",
                   ptr_d.get (),
                   (volatile Base_1<int>*)ptr_d.get (), ptr_1.get ());
        rw_assert (ptr_d.get () == ptr_b.get (), 0, __LINE__,
                   "shared_ptr<Base>(const shared_ptr<Derived>(%#p))"
                   ".get() == %#p, got %#p",
                   ptr_d.get (),
                   (volatile Base<int>*)ptr_d.get (), ptr_b.get ());
        rw_assert (4 == ptr_d.use_count (), 0, __LINE__, "");
        rw_assert (ptr_d.use_count () == ptr_0.use_count (), 0, __LINE__, "");
        rw_assert (ptr_d.use_count () == ptr_1.use_count (), 0, __LINE__, "");
        rw_assert (ptr_d.use_count () == ptr_b.use_count (), 0, __LINE__, "");
        #else
        if(pd != ptr_d.get ())
          {
            failures++;
           std_log(LOG_FILENAME_LINE,"Reason:failing");  
           }     


       if(ptr_d.get () != ptr_0.get ())
          {
            failures++;
           std_log(LOG_FILENAME_LINE,"Reason:failing");  
           }     

                   
  
      if(ptr_d.get () != ptr_1.get ())
          {
            failures++;
           std_log(LOG_FILENAME_LINE,"Reason:failing");  
           }     

       if(ptr_d.get () != ptr_b.get ())
          {
            failures++;
           std_log(LOG_FILENAME_LINE,"Reason:failing");  
           }     

       if(4 != ptr_d.use_count ())
          {
            failures++;
           std_log(LOG_FILENAME_LINE,"Reason:failing");  
           }     
        
        
       if(ptr_d.use_count () != ptr_0.use_count ())
          {
            failures++;
           std_log(LOG_FILENAME_LINE,"Reason:failing");  
           }     

        
       if(ptr_d.use_count () != ptr_1.use_count ())
          {
            failures++;
           std_log(LOG_FILENAME_LINE,"Reason:failing");  
           }     

        
       if(ptr_d.use_count () != ptr_b.use_count ())
          {
            failures++;
           std_log(LOG_FILENAME_LINE,"Reason:failing");  
           }     
      #endif
   
}
}
/**************************************************************************/

static void
test_dtor ()
{
    rw_info (0, "tr.util.smartptr.shared.dest", 0,
             "shared_ptr destructor");

    {
        // ~shared_ptr()
        const int base_dtors = Base<int>::n_dtors;
        {
            std::tr1::shared_ptr<Base<int> > ptr;
        }
       #ifndef __SYMBIAN32__
        rw_assert (base_dtors == Base<int>::n_dtors, 0, __LINE__, "");
       #else
        if(base_dtors != Base<int>::n_dtors)
          {
            failures++;
           std_log(LOG_FILENAME_LINE,"Reason:failing");  
           }     
        #endif
    }

    {
        // ~shared_ptr()
        const int base_dtors = Base<int>::n_dtors;
        {
            std::tr1::shared_ptr<Base<int> > ptr (new Base<int>);
        }
        #ifndef __SYMBIAN32__
        rw_assert (base_dtors + 1 == Base<int>::n_dtors, 0, __LINE__, "");
        #else
        if(base_dtors + 1 != Base<int>::n_dtors)
          {
            failures++;
           std_log(LOG_FILENAME_LINE,"Reason:failing");  
           }     
        #endif  
    }

    {
        // ~shared_ptr()
        const int base_dtors    = Base<int>::n_dtors;
        const int derived_dtors = Derived<int>::n_dtors;
        {
            std::tr1::shared_ptr<Derived<int> > ptr (new Derived<int>);
        }
        #ifndef __SYMBIAN32__
        rw_assert (base_dtors + 1 == Base<int>::n_dtors, 0, __LINE__, "");
        rw_assert (derived_dtors + 1 == Derived<int>::n_dtors, 0, __LINE__, "");
        #else
        if(base_dtors + 1 != Base<int>::n_dtors)
          {
            failures++;
           std_log(LOG_FILENAME_LINE,"Reason:failing");  
           }     
               
        if(derived_dtors + 1 != Derived<int>::n_dtors)
          {
            failures++;
           std_log(LOG_FILENAME_LINE,"Reason:failing");  
           }     
        #endif
    }

    {
        // ~shared_ptr()
        const int base_dtors    = Base<int>::n_dtors;
        const int derived_dtors = Derived<int>::n_dtors;
        {
            std::tr1::shared_ptr<Base<int> > ptr (new Derived<int>);
        }
      #ifndef __SYMBIAN32__  
        rw_assert (base_dtors + 1 == Base<int>::n_dtors, 0, __LINE__, "");
        rw_assert (derived_dtors + 1 == Derived<int>::n_dtors, 0, __LINE__, "");
       #else 
        if(base_dtors + 1 != Base<int>::n_dtors)
          {
            failures++;
           std_log(LOG_FILENAME_LINE,"Reason:failing");  
           }     
        
        if(derived_dtors + 1 != Derived<int>::n_dtors)
          {
            failures++;
           std_log(LOG_FILENAME_LINE,"Reason:failing");  
           }     
         #endif
    }

    {
        // ~shared_ptr()
        const int base_dtors    = Base<int>::n_dtors;
        const int derived_dtors = Derived<int>::n_dtors;
        {
            std::tr1::shared_ptr<void> ptr (new Derived<int>);
        }
        #ifndef __SYMBIAN32__
        rw_assert (base_dtors + 1 == Base<int>::n_dtors, 0, __LINE__, "");
        rw_assert (derived_dtors + 1 == Derived<int>::n_dtors, 0, __LINE__, "");
        #else
        if(base_dtors + 1 != Base<int>::n_dtors)
          {
            failures++;
           std_log(LOG_FILENAME_LINE,"Reason:failing");  
           }     
        
       if(derived_dtors + 1 != Derived<int>::n_dtors)
          {
            failures++;
           std_log(LOG_FILENAME_LINE,"Reason:failing");  
           }     
       #endif  
    }

    {
        // ~shared_ptr()
        Deleter<int> d;
        const int funcalls = Deleter<int>::n_funcalls;
        {
            std::tr1::shared_ptr<int> ptr ((int*)0, d);
            {
                std::tr1::shared_ptr<int> cpy (ptr);
                _RWSTD_UNUSED (cpy);
            }
      #ifndef __SYMBIAN32__
            rw_assert (funcalls == Deleter<int>::n_funcalls, 0, __LINE__, "");
       #else  
         if(funcalls != Deleter<int>::n_funcalls)
          {
            failures++;
           std_log(LOG_FILENAME_LINE,"Reason:failing");  
           }     
        #endif
        }
        
        #ifndef __SYMBIAN32__
        rw_assert (funcalls + 1 == Deleter<int>::n_funcalls, 0, __LINE__, "");
        #else
        if(funcalls +1 != Deleter<int>::n_funcalls)
          {
            failures++;
           std_log(LOG_FILENAME_LINE,"Reason:failing");  
           }     
        #endif 
    }

    {
        // ~shared_ptr()
        Deleter<int> d;
        const int funcalls = Deleter<int>::n_funcalls;
        {
            std::tr1::shared_ptr<int> ptr ((int*)0, d);
            _RWSTD_UNUSED (ptr);
        }
        #ifndef __SYMBIAN32__
        rw_assert (funcalls + 1 == Deleter<int>::n_funcalls, 0, __LINE__, "");
        #else
        if(funcalls +1 != Deleter<int>::n_funcalls)
           {
            failures++;
           std_log(LOG_FILENAME_LINE,"Reason:failing");  
           }     
        #endif
    }

    {
        // ~shared_ptr()
        int* const p = new int (__LINE__);
        Deleter<int> d;
        const int funcalls = Deleter<int>::n_funcalls;
        {
            std::tr1::shared_ptr<int> ptr (p, d);
        }
        #ifndef __SYMBIAN32__
        rw_assert (funcalls + 1 == Deleter<int>::n_funcalls, 0, __LINE__, "");
        #else
        if(funcalls +1 != Deleter<int>::n_funcalls)
          {
            failures++;
           std_log(LOG_FILENAME_LINE,"Reason:failing");  
           }     
       #endif
    }

    {   // ~shared_ptr

        // verify that the destructor of the referenced object
        // including all its subobjects get invoked
        int d_dtors  = Derived<int>::n_dtors;
        int b1_dtors = Base_1<int>::n_dtors;
        int b0_dtors = Base_0<int>::n_dtors;
        int b_dtors  = Base<int>::n_dtors;

        {
            std::tr1::shared_ptr<Derived<int> >(new Derived<int>);
        }
      #ifndef __SYMBAIN32__
       rw_assert (1 == Derived<int>::n_dtors - d_dtors, 0, __LINE__,
                   "shared_ptr<Derived>(Derived*) called ~Derived()");
        rw_assert (1 == Base_1<int>::n_dtors - b1_dtors, 0, __LINE__,
                     "shared_ptr<Derived>(Derived*) called ~Base_1()");
        rw_assert (1 == Base_0<int>::n_dtors - b0_dtors, 0, __LINE__,
                   "shared_ptr<Derived>(Derived*) called ~Base_0()");
        rw_assert (1 == Base<int>::n_dtors - b_dtors, 0, __LINE__,
                   "shared_ptr<Derived>(Derived*) called ~Base()");
        rw_assert (1 == Base<int>::n_dtors - b_dtors, 0, __LINE__,
                   "shared_ptr<Derived>(Derived*) called ~Base()");
     #else
       if(1 != Derived<int>::n_dtors - d_dtors)
          {
            failures++;
           std_log(LOG_FILENAME_LINE,"Reason:failing");  
           }     
           
      
      if(1 != Base_1<int>::n_dtors - b1_dtors)
      {
        failures++;
        std_log(LOG_FILENAME_LINE,"Reason: failing") ;
        }          
      if(1 != Base_0<int>::n_dtors - b0_dtors)
      {
         failures++;
          std_log(LOG_FILENAME_LINE,"Reason: failing") ;
          }
       if(1 != Base<int>::n_dtors - b_dtors)
       {
         failures++;
          std_log(LOG_FILENAME_LINE,"Reason: failing") ;
          }
       if(1 != Base<int>::n_dtors - b_dtors)
       {
         failures++;
          std_log(LOG_FILENAME_LINE,"Reason: failing") ;
          }

      #endif
       d_dtors  = Derived<int>::n_dtors;
        b1_dtors = Base_1<int>::n_dtors;
       b0_dtors = Base_0<int>::n_dtors;
        b_dtors  = Base<int>::n_dtors;

        {
            std::tr1::shared_ptr<Base_1<int> >(new Derived<int>);
        }
    
    #ifndef __SYMBIAN32__
        rw_assert (1 == Derived<int>::n_dtors - d_dtors, 0, __LINE__,
                   "shared_ptr<Base_1>(Derived*) called ~Derived()");
        rw_assert (1 == Base_1<int>::n_dtors - b1_dtors, 0, __LINE__,
                   "shared_ptr<Base_1>(Derived*) called ~Base_1()");

        rw_assert (1 == Base_0<int>::n_dtors - b0_dtors, 0, __LINE__,
                   "shared_ptr<Base_1>(Derived*) called ~Base_0()");
        rw_assert (1 == Base<int>::n_dtors - b_dtors, 0, __LINE__,
                   "shared_ptr<Base_1>(Derived*) called ~Base()");
     #else
       if(1 != Derived<int>::n_dtors - d_dtors)
       {
         failures++;
         std_log(LOG_FILENAME_LINE,"Reason: failing");
         }
         
      if(1 != Base_1<int>::n_dtors - b1_dtors)
       {
         failures++;
         std_log(LOG_FILENAME_LINE,"Reason: failing");
         }
 
      if(1 != Base_0<int>::n_dtors - b0_dtors)
       {
         failures++;
         std_log(LOG_FILENAME_LINE,"Reason: failing");
         }
 
      if(1 != Base<int>::n_dtors - b_dtors)
      {
        failures++;
        std_log(LOG_FILENAME_LINE,"Reason:failing");
        }
     #endif   
        d_dtors  = Derived<int>::n_dtors;
        b1_dtors = Base_1<int>::n_dtors;
        b0_dtors = Base_0<int>::n_dtors;
        b_dtors  = Base<int>::n_dtors;

        {
            std::tr1::shared_ptr<Base_0<int> >(new Derived<int>);
        }

    #ifndef __SYMBIAN32__
       rw_assert (1 == Derived<int>::n_dtors - d_dtors, 0, __LINE__,
                   "shared_ptr<Base_0>(Derived*) called ~Derived()");
       rw_assert (1 == Base_1<int>::n_dtors - b1_dtors, 0, __LINE__,
                    "shared_ptr<Base_0>(Derived*) called ~Base_1()");
       rw_assert (1 == Base_0<int>::n_dtors - b0_dtors, 0, __LINE__,
                   "shared_ptr<Base_0>(Derived*) called ~Base_0()");
       rw_assert (1 == Base<int>::n_dtors - b_dtors, 0, __LINE__,
                   "shared_ptr<Base_0>(Derived*) called ~Base()");
     #else
         if(1 != Derived<int>::n_dtors - d_dtors)
          {
            failures++;
           std_log(LOG_FILENAME_LINE,"Reason:failing");  
           }     
                 
       if(1 != Base_1<int>::n_dtors - b1_dtors)
          {
            failures++;
           std_log(LOG_FILENAME_LINE,"Reason:failing");  
           }     
 
       if(1 != Base_0<int>::n_dtors - b0_dtors)
          {
            failures++;
           std_log(LOG_FILENAME_LINE,"Reason:failing");  
           }     
 
      if(1 != Base<int>::n_dtors - b_dtors)
          {
            failures++;
           std_log(LOG_FILENAME_LINE,"Reason:failing");  
           }     
        #endif
        d_dtors  = Derived<int>::n_dtors;
        b1_dtors = Base_1<int>::n_dtors;
        b0_dtors = Base_0<int>::n_dtors;
        b_dtors  = Base<int>::n_dtors;

        {
            std::tr1::shared_ptr<Base<int> >(new Derived<int>);
        }
          
        #ifndef __SYMBIAN32__  
        rw_assert (1 == Derived<int>::n_dtors - d_dtors, 0, __LINE__,
                   "shared_ptr<Base>(Derived*) called ~Derived()");
        rw_assert (1 == Base_1<int>::n_dtors - b1_dtors, 0, __LINE__,
                   "shared_ptr<Base>(Derived*) called ~Base_1()");
        rw_assert (1 == Base_0<int>::n_dtors - b0_dtors, 0, __LINE__,
                   "shared_ptr<Base>(Derived*) called ~Base_0()");
        rw_assert (1 == Base<int>::n_dtors - b_dtors, 0, __LINE__,
                   "shared_ptr<Base>(Derived*) called ~Base()");
        #else
       if(1 != Derived<int>::n_dtors - d_dtors)
          {
            failures++;
           std_log(LOG_FILENAME_LINE,"Reason:failing");  
           }     

        if(1 != Base_1<int>::n_dtors - b1_dtors)
        {
         failures++;
         std_log(LOG_FILENAME_LINE,"Reason:failing");
         }           
        if(1 != Base_0<int>::n_dtors - b0_dtors)
        {
         failures++;
         std_log(LOG_FILENAME_LINE,"Reason:failing");
         }           

        if(1 != Base<int>::n_dtors - b_dtors)
        {
         failures++;
         std_log(LOG_FILENAME_LINE,"Reason:failing");
         }           
        #endif 
        d_dtors  = Derived<int>::n_dtors;
        b1_dtors = Base_1<int>::n_dtors;
        b0_dtors = Base_0<int>::n_dtors;
        b_dtors  = Base<int>::n_dtors;

        {
            std::tr1::shared_ptr<void>(new Derived<int>);
        }
      #ifndef __SYMBIAN32__   
        rw_assert (1 == Derived<int>::n_dtors - d_dtors, 0, __LINE__,
                   "shared_ptr<void>(Derived*) called ~Derived()");
        rw_assert (1 == Base_1<int>::n_dtors - b1_dtors, 0, __LINE__,
                   "shared_ptr<void>(Derived*) called ~Base_1()");
        rw_assert (1 == Base_0<int>::n_dtors - b0_dtors, 0, __LINE__,
                   "shared_ptr<void>(Derived*) called ~Base_0()");
        rw_assert (1 == Base<int>::n_dtors - b_dtors, 0, __LINE__,
                   "shared_ptr<void>(Derived*) called ~Base()");
      #else  
        if(1 != Derived<int>::n_dtors - d_dtors)
        {
         failures++;
         std_log(LOG_FILENAME_LINE,"Reason:failing");
         }           

        if(1 != Base_1<int>::n_dtors - b1_dtors)
        {
         failures++;
         std_log(LOG_FILENAME_LINE,"Reason:failing");
         }           

        if(1 != Base_0<int>::n_dtors - b0_dtors)
        {
         failures++;
         std_log(LOG_FILENAME_LINE,"Reason:failing");
         }           

        if(1 != Base<int>::n_dtors - b_dtors)
        {
         failures++;
         std_log(LOG_FILENAME_LINE,"Reason:failing");
         }
       #endif             

    }
}

/**************************************************************************/

static void
test_assign ()
{
    rw_info (0, "tr.util.smartptr.shared.assign", 0,
             "shared_ptr assignment operators");

    {   // operator=(const shared_ptr&)
        std::tr1::shared_ptr<void> ptr0;
        std::tr1::shared_ptr<void> ptr1;

        ptr1 = ptr0;
       
       #ifndef __SYMBIAN32__
        rw_assert (ptr1.get () == ptr0.get (), 0, __LINE__, "");
        rw_assert (ptr1.use_count () == ptr0.use_count (), 0, __LINE__, "");
       #else 
        if(ptr1.get () != ptr0.get ())
        {
         failures++;
         std_log(LOG_FILENAME_LINE,"Reason: failing");
         }
        if(ptr1.use_count () != ptr0.use_count ())
        {
         failures++;
         std_log(LOG_FILENAME_LINE,"Reason: failing");
         }
       #endif
    }

    {   // operator=(const shared_ptr&)
        int* const p = new int (__LINE__);
        std::tr1::shared_ptr<int> ptr0 (p);
        std::tr1::shared_ptr<int> ptr1;

        ptr1 = ptr0;
        
        #ifndef __SYMBIAN32__
        rw_assert (ptr1.get () == ptr0.get (), 0, __LINE__, "");
        rw_assert (ptr1.use_count () == ptr0.use_count (), 0, __LINE__, "");
        #else
         if(ptr1.get () != ptr0.get ())
        {
         failures++;
         std_log(LOG_FILENAME_LINE,"Reason: failing");
         }
             
        if(ptr1.use_count () != ptr0.use_count ())
        {
         failures++;
         std_log(LOG_FILENAME_LINE,"Reason: failing");
         }
        #endif
    }

    {   // template <class U> operator=(const shared_ptr<U>&)
        int* const p = new int (__LINE__);
        std::tr1::shared_ptr<int>  ptr0 (p);
        std::tr1::shared_ptr<void> ptr1;

        ptr1 = ptr0;
       
        #ifdef __SYMBIAN32__
        rw_assert (ptr1.get () == ptr0.get (), 0, __LINE__, "");
        rw_assert (ptr1.use_count () == ptr0.use_count (), 0, __LINE__, "");
        #else
        if(ptr1.get () != ptr0.get ())
        {
         failures++;
         std_log(LOG_FILENAME_LINE,"Reason: failing");
         }
       
        if(ptr1.use_count () != ptr0.use_count ())
        {
         failures++;
         std_log(LOG_FILENAME_LINE,"Reason: failing");
         }
        #endif  
    }

    {   // template <class U> operator=(const shared_ptr<U>&)
        Derived<int>* const p = new Derived<int>;

        std::tr1::shared_ptr<Derived<int> > ptr0 (p);
        std::tr1::shared_ptr<Base<int> >    ptr1;

        ptr1 = ptr0;
        
        #ifndef __SYMBIAN32__
        rw_assert (ptr1.get ()->id == ptr0.get ()->id, 0, __LINE__,
                   "%d == %d, got %d", ptr1.get ()->id, ptr1.get ()->id,
                   ptr0.get ()->id);
        rw_assert (ptr1.use_count () == ptr0.use_count (), 0, __LINE__, "");
        if(ptr1.get ()->id != ptr0.get ()->id)
        {
         failures++;
         std_log(LOG_FILENAME_LINE,"Reason: failing");
         }
        if(ptr1.use_count () != ptr0.use_count ())
        {
         failures++;
         std_log(LOG_FILENAME_LINE,"Reason: failing");
         }
         #endif
    }

    {   // template <class U> shared_ptr(U*)
        Derived<int>* const pd = new Derived<int>;

        const std::tr1::shared_ptr<Derived<int> > ptr_d (pd);

        std::tr1::shared_ptr<Base_0<int> > ptr_0;
        std::tr1::shared_ptr<Base_1<int> > ptr_1;
        std::tr1::shared_ptr<Base<int> >   ptr_b;
        std::tr1::shared_ptr<void>         ptr_v;

        ptr_0 = ptr_d;
        ptr_1 = ptr_d;
        ptr_b = ptr_d;
        ptr_v = ptr_d;
        
        #ifndef __SYMBIAN32__
        rw_assert (ptr_d.get () == ptr_0.get (), 0, __LINE__,
                   "(shared_ptr<Base_0>() = shared_ptr<Derived>(Derived* "
                   "= %#p)).get() == %#p, got %#p",
                   ptr_d.get (), (Base_0<int>*)ptr_d.get (), ptr_0.get ());
        rw_assert (ptr_d.get () == ptr_1.get (), 0, __LINE__,
                   "(shared_ptr<Base_1>() = shared_ptr<Derived>(Derived* "
                   "= %#p)).get() == %#p, got %#p",
                   ptr_d.get (), (Base_1<int>*)ptr_d.get (), ptr_1.get ());
        rw_assert (ptr_d.get () == ptr_b.get (), 0, __LINE__,
                   "(shared_ptr<Base>() = shared_ptr<Derived>(Derived* = %#p))"
                   ".get() == %#p, got %#p",
                   ptr_d.get (), (Base<int>*)ptr_d.get (), ptr_b.get ());
        rw_assert (ptr_d.get () == ptr_v.get (), 0, __LINE__,
                   "(shared_ptr<void>() = shared_ptr<Derived>(Derived* = %#p))"
                   ".get() == %#p, got %#p",
                   ptr_d.get (), (void*)ptr_d.get (), ptr_v.get ());
        rw_assert (5                  == ptr_d.use_count (), 0, __LINE__, "");     
        rw_assert (ptr_0.use_count () == ptr_d.use_count (), 0, __LINE__, "");
        rw_assert (ptr_1.use_count () == ptr_d.use_count (), 0, __LINE__, "");
        rw_assert (ptr_b.use_count () == ptr_d.use_count (), 0, __LINE__, "");
        rw_assert (ptr_v.use_count () == ptr_d.use_count (), 0, __LINE__, "");
        #else
        if(ptr_d.get () != ptr_0.get ())
        {
         failures++;
         std_log(LOG_FILENAME_LINE,"Reason: failing");
         }

         if(ptr_d.get () != ptr_1.get ())
        {
         failures++;
         std_log(LOG_FILENAME_LINE,"Reason: failing");
         }

        if(ptr_d.get () != ptr_b.get ())
        {
         failures++;
         std_log(LOG_FILENAME_LINE,"Reason: failing");
         }

        if(ptr_d.get () != ptr_v.get ())
        {
         failures++;
         std_log(LOG_FILENAME_LINE,"Reason: failing");
         }


        if(5 != ptr_d.use_count ())
        {
         failures++;
         std_log(LOG_FILENAME_LINE,"Reason: failing");
         }

        
        if(ptr_0.use_count () != ptr_d.use_count ())
        {
         failures++;
         std_log(LOG_FILENAME_LINE,"Reason: failing");
         }

        
        if(ptr_1.use_count () != ptr_d.use_count ())
        {
         failures++;
         std_log(LOG_FILENAME_LINE,"Reason: failing");
         }

        
        if(ptr_b.use_count () != ptr_d.use_count ())
        {
         failures++;
         std_log(LOG_FILENAME_LINE,"Reason: failing");
         }

        
        if(ptr_v.use_count () != ptr_d.use_count ())
        {
         failures++;
         std_log(LOG_FILENAME_LINE,"Reason: failing");
         }
        #endif
    }
}

/**************************************************************************/

static void
test_modifiers ()
{
    rw_info (0, "tr.util.smartptr.shared.mod", 0,
             "shared_ptr modifiers");

    rw_warn (0, 0, 0,
             "shared_ptr modifiers not exercised");
}

/**************************************************************************/

static void
test_observers ()
{
    rw_info (0, "tr.util.smartptr.shared.obs", 0,
             "shared_ptr observers");

    {   // operator*()
        std::tr1::shared_ptr<void> ptr;
        #ifndef __SYMBIAN32__
        rw_assert (0 == ptr.get (), 0, __LINE__, "");
        rw_assert (0 == ptr.use_count (), 0, __LINE__, "");
        #else
        if(0 != ptr.get ())
        {
         failures++;
         std_log(LOG_FILENAME_LINE,"Reason: failing");
         }
       
        if(0 != ptr.use_count ())
        {
         failures++;
         std_log(LOG_FILENAME_LINE,"Reason: failing");
         }
        #endif
    }

    rw_warn (0, 0, 0,
             "shared_ptr observers not exercised");
}

/**************************************************************************/

static void
test_comparison ()
{
    rw_info (0, "tr.util.smartptr.shared.cmp", 0,
             "shared_ptr comparison");

    {   // operator==
        std::tr1::shared_ptr<void> ptr;
        #ifndef __SYMBIAN32__
        rw_assert (ptr == ptr, 0, __LINE__, "");
        rw_assert (!(ptr != ptr), 0, __LINE__, "");
        #else
        if(ptr != ptr)
        {
         failures++;
         std_log(LOG_FILENAME_LINE,"Reason: failing");
         }
       
        if((ptr != ptr))
        {
         failures++;
         std_log(LOG_FILENAME_LINE,"Reason: failing");
         }
         #endif
    }

    {   // operator==
        std::tr1::shared_ptr<void> ptr0;
        std::tr1::shared_ptr<int>  ptr1;
        #ifndef __SYMBIAN32__
        rw_assert (ptr0 == ptr1, 0, __LINE__, "");
        rw_assert (ptr1 == ptr0, 0, __LINE__, "");
        rw_assert (!(ptr0 != ptr1), 0, __LINE__, "");
        rw_assert (!(ptr1 != ptr0), 0, __LINE__, "");
        #else
        if(ptr0 != ptr1)
        {
         failures++;
         std_log(LOG_FILENAME_LINE,"Reason: failing");
         }

        
        if(ptr1 != ptr0)
        {
         failures++;
         std_log(LOG_FILENAME_LINE,"Reason: failing");
         }

        
        if((ptr0 != ptr1))
        {
         failures++;
         std_log(LOG_FILENAME_LINE,"Reason: failing");
         }

        
        if((ptr1 != ptr0))
        {
         failures++;
         std_log(LOG_FILENAME_LINE,"Reason: failing");
         }
        #endif
    }

#if ILL_FORMED == -1 || ILL_FORMED == __LINE__

    {
        std::tr1::shared_ptr<char> ptr0;
        std::tr1::shared_ptr<int>  ptr1;
       #ifndef __SYMBIAN32__
        rw_assert (ptr0 == ptr1, 0, __LINE__, "");
        rw_assert (ptr1 == ptr0, 0, __LINE__, "");
        rw_assert (!(ptr0 != ptr1), 0, __LINE__, "");
        rw_assert (!(ptr1 != ptr0), 0, __LINE__, "");
        #else
        if(ptr0 != ptr1)
        {
         failures++;
         std_log(LOG_FILENAME_LINE,"Reason: failing");
         }

        
        if(ptr1 != ptr0)
        {
         failures++;
         std_log(LOG_FILENAME_LINE,"Reason: failing");
         }

        
        if((ptr0 != ptr1))
        {
         failures++;
         std_log(LOG_FILENAME_LINE,"Reason: failing");
         }

        
        if((ptr1 != ptr0))
        {
         failures++;
         std_log(LOG_FILENAME_LINE,"Reason: failing");
         }
       #endif
    } 

#endif   // ILL_FORMED

    {   // operator==
        int* const p = new int (__LINE__);
        std::tr1::shared_ptr<int> ptr0 (p);
        std::tr1::shared_ptr<int> ptr1 (ptr0);
        #ifndef __SYMBIAN32__
        rw_assert (ptr0 == ptr1, 0, __LINE__, "");
        rw_assert (ptr1 == ptr0, 0, __LINE__, "");
        rw_assert (!(ptr0 != ptr1), 0, __LINE__, "");
        rw_assert (!(ptr1 != ptr0), 0, __LINE__, "");
        #else
        if(ptr0 != ptr1)
        {
         failures++;
         std_log(LOG_FILENAME_LINE,"Reason: failing");
         }

        
        if(ptr1 != ptr0)
        {
         failures++;
         std_log(LOG_FILENAME_LINE,"Reason: failing");
         }

        
        if((ptr0 != ptr1))
        {
         failures++;
         std_log(LOG_FILENAME_LINE,"Reason: failing");
         }

        
        if((ptr1 != ptr0))
        {
         failures++;
         std_log(LOG_FILENAME_LINE,"Reason: failing");
         }
        #endif
    }

    {   // operator==
        int* const p0 = new int (__LINE__);
        int* const p1 = new int (__LINE__);
        std::tr1::shared_ptr<int> ptr0 (p0);
        std::tr1::shared_ptr<int> ptr1 (p1);
       
       #ifndef __SYMBIAN32__
        rw_assert (!(ptr0 == ptr1), 0, __LINE__, "");
        rw_assert (!(ptr1 == ptr0), 0, __LINE__, "");
        rw_assert (ptr0 != ptr1, 0, __LINE__, "");
        rw_assert (ptr1 != ptr0, 0, __LINE__, "");
        #else
        if((ptr0 == ptr1))
         {
         failures++;
         std_log(LOG_FILENAME_LINE,"Reason: failing");
         }

        
        if((ptr1 == ptr0))
        {
         failures++;
         std_log(LOG_FILENAME_LINE,"Reason: failing");
         }

        
        if(!(ptr0 != ptr1))
        {
         failures++;
         std_log(LOG_FILENAME_LINE,"Reason: failing");
         }

        
        if(!(ptr1 != ptr0))
        {
         failures++;
         std_log(LOG_FILENAME_LINE,"Reason: failing");
         }
       #endif
    }

    {   // operator<
        Derived<int>* const p = new Derived<int>;

        std::tr1::shared_ptr<Derived<int> > ptr2 (p);
        std::tr1::shared_ptr<Base_1<int> >  ptr1 (ptr2);
        std::tr1::shared_ptr<Base_0<int> >  ptr0 (ptr2);
        std::tr1::shared_ptr<Base<int> >    ptr_b1 (ptr1);
        std::tr1::shared_ptr<Base<int> >    ptr_b0 (ptr0);

        const bool eq_1_2 = (ptr1.get () == ptr2.get ()) == (ptr1 == ptr2);
        const bool eq_0_2 = (ptr0.get () == ptr2.get ()) == (ptr0 == ptr2);

        // rw_assert (eq_0_1, 0, __LINE__, "");
        #ifndef __SYMBIAN32__
        rw_assert (eq_1_2, 0, __LINE__, "");
        rw_assert (eq_0_2, 0, __LINE__, "");
        #else
        if(!eq_1_2)
        {
         failures++;
         std_log(LOG_FILENAME_LINE,"Reason: failing");
         }
      
        if(!eq_0_2)
        {
         failures++;
         std_log(LOG_FILENAME_LINE,"Reason: failing");
         }
        #endif
    }

    {   // operator<
        std::tr1::shared_ptr<void> ptr;
        #ifndef __SYMBIAN32__
        rw_assert (!(ptr < ptr), 0, __LINE__, "");
        #else
        if((ptr < ptr))
        {
         failures++;
         std_log(LOG_FILENAME_LINE,"Reason: failing");
         }
        #endif
   }

    {   // operator<
        std::tr1::shared_ptr<void> ptr;
        std::tr1::shared_ptr<void> cpy (ptr);
        #ifndef __SYMBIAN32__
        rw_assert (!(ptr < cpy) && !(cpy < ptr), 0, __LINE__, "");
        #else
        if(!(!(ptr < cpy) && !(cpy < ptr)))
        {
         failures++;
         std_log(LOG_FILENAME_LINE,"Reason: failing");
         }
        #endif
    }

    {   // operator<
        int* const p = new int (__LINE__);
        std::tr1::shared_ptr<void> ptr (p);
        std::tr1::shared_ptr<void> cpy (ptr);
        #ifndef __SYMBIAN32__
        rw_assert (!(ptr < cpy) && !(cpy < ptr), 0, __LINE__, "");
        #else
        if(!(!(ptr < cpy) && !(cpy < ptr)))
        {
         failures++;
         std_log(LOG_FILENAME_LINE,"Reason: failing");
         }
        #endif
    }

    {   // operator<
        Derived<int>* const p = new Derived<int>;

        std::tr1::shared_ptr<Derived<int> > ptr2 (p);
        std::tr1::shared_ptr<Base_1<int> >  ptr1 (ptr2);
        std::tr1::shared_ptr<Base_0<int> >  ptr0 (ptr2);
        std::tr1::shared_ptr<void>          ptr  (ptr0);
       #ifndef __SYMBIAN32__
        rw_assert (!(ptr  < ptr0) && !(ptr0 < ptr),  0, __LINE__, "");
        rw_assert (!(ptr  < ptr1) && !(ptr1 < ptr),  0, __LINE__, "");
        rw_assert (!(ptr  < ptr2) && !(ptr2 < ptr ), 0, __LINE__, "");
        rw_assert (!(ptr0 < ptr1) && !(ptr0 < ptr1), 0, __LINE__, "");
        rw_assert (!(ptr0 < ptr2) && !(ptr0 < ptr2), 0, __LINE__, "");
        rw_assert (!(ptr1 < ptr2) && !(ptr1 < ptr2), 0, __LINE__, "");
        #else
         if(!(!(ptr  < ptr0) && !(ptr0 < ptr)))
        {
         failures++;
         std_log(LOG_FILENAME_LINE,"Reason: failing");
         }

        
        if(!(!(ptr  < ptr1) && !(ptr1 < ptr)))
        {
         failures++;
         std_log(LOG_FILENAME_LINE,"Reason: failing");
         }

        
        if(!(!(ptr  < ptr2) && !(ptr2 < ptr )))
        {
         failures++;
         std_log(LOG_FILENAME_LINE,"Reason: failing");
         }

        
        if(!(!(ptr0 < ptr1) && !(ptr0 < ptr1)))
        {
         failures++;
         std_log(LOG_FILENAME_LINE,"Reason: failing");
         }

        
        if(!(!(ptr0 < ptr2) && !(ptr0 < ptr2)))
        {
         failures++;
         std_log(LOG_FILENAME_LINE,"Reason: failing");
         }

        
        if(!(!(ptr1 < ptr2) && !(ptr1 < ptr2)))
        {
         failures++;
         std_log(LOG_FILENAME_LINE,"Reason: failing");
         }
      #endif
    }

    {   // operator<
        short* const p0 = new short (__LINE__);
        float* const p1 = new float (__LINE__);

        std::tr1::shared_ptr<short> ptr0 (p0);
        std::tr1::shared_ptr<float> ptr1 (p1);
        #ifndef __SYMBIAN32__
        rw_assert (ptr0 < ptr1 || ptr1 < ptr0, 0, __LINE__, "");
        #else
        if(!(ptr0 < ptr1 || ptr1 < ptr0))
        {
         failures++;
         std_log(LOG_FILENAME_LINE,"Reason: failing");
         }
        #endif
    }
}

/**************************************************************************/

static void
test_io ()
{
    rw_info (0, "tr.util.smartptr.shared.io", 0,
             "shared_ptr I/O");

    rw_warn (0, 0, 0,
             "shared_ptr I/O not exercised");
}

/**************************************************************************/

static void
test_specialized ()
{
    rw_info (0, "tr.util.smartptr.shared.spec", 0,
             "shared_ptr specialized algorithms");

    rw_warn (0, 0, 0,
             "shared_ptr specialized algorithms not exercised");
}

/**************************************************************************/

static void
test_casts ()
{
    rw_info (0, "tr.util.smartptr.shared.cast", 0,
             "shared_ptr casts");

    {
        // dynamic_pointer_cast
        
    }

    rw_warn (0, 0, 0,
             "shared_ptr casts not exercised");
}

/**************************************************************************/

static void
test_deleter ()
{
    rw_info (0, "tr.util.smartptr.shared.getdeleter", 0,
             "shared_ptr get_deleter");

    rw_warn (0, 0, 0,
             "shared_ptr get_deleter not exercised");
}

/**************************************************************************/

static int no_ctor; 
static int no_copy_ctor; 
static int no_dtor; 
static int no_assign; 
static int no_modifiers; 
static int no_observers; 
static int no_comparison; 
static int no_io; 
static int no_specialized; 
static int no_casts; 
static int no_deleter;


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
    TEST (io);
    TEST (specialized);
    TEST (casts);
    TEST (deleter);

    return 0;
}

/**************************************************************************/
int main (int argc, char *argv[])
{
  std_log(LOG_FILENAME_LINE,"[Test Case for smartptr_shared]");

   #ifndef __SYMBIAN32__
    return rw_test (argc, argv, __FILE__,
                    "tr.util.smartptr.shared",
                     0, run_test,
                    "|-no-ctor# "
                    "|-no-copy_ctor# "
                    "|-no-dtor# "
                    "|-no-assign# "
                    "|-no-modifiers# "
                    "|-no-observers# "
                    "|-no-comparison# "
                    "|-no-io# "
                    "|-no-specialized# "
                    "|-no-casts# "
                    "|-no-deleter# ",
                    &no_ctor, 
                    &no_copy_ctor, 
                    &no_dtor, 
                    &no_assign, 
                    &no_modifiers, 
                    &no_observers, 
                    &no_comparison, 
                    &no_io, 
                    &no_specialized, 
                    &no_casts, 
                    &no_deleter);
 #else
  rw_test (argc, argv, __FILE__,"tr.util.smartptr.shared",0 , run_test,
                    "|-no-ctor# "
                    "|-no-copy_ctor# "
                    "|-no-dtor# "
                    "|-no-assign# "
                    "|-no-modifiers# "
                    "|-no-observers# "
                    "|-no-comparison# "
                    "|-no-io# "
                    "|-no-specialized# "
                    "|-no-casts# "
                    "|-no-deleter# ",
                    &no_ctor, 
                    &no_copy_ctor, 
                    &no_dtor, 
                    &no_assign, 
                    &no_modifiers, 
                    &no_observers, 
                    &no_comparison, 
                    &no_io, 
                    &no_specialized, 
                    &no_casts, 
                    &no_deleter);
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
   
   testResultXml("2_smartptr_shared");
    close_log_file();
  return failures;
 #endif
// return 0;
}

