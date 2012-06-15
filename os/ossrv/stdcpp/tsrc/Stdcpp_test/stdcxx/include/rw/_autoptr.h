// -*- C++ -*-
/***************************************************************************
 *
 * _autoptr.h - definition of the class template auto_ptr
 *
 * This is an internal header file used to implement the C++ Standard
 * Library. It should never be #included directly by a program.
 *
 * $Id: _autoptr.h 225375 2005-07-26 19:04:10Z sebor $
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

#ifndef _RWSTD_AUTOPTR_H_INCLUDED
#define _RWSTD_AUTOPTR_H_INCLUDED

#include <rw/_defs.h>


_RWSTD_NAMESPACE (std) { 


// 20.4.5 - Template class auto_ptr

template <class _TypeT>
class auto_ptr;


// 20.4.5, p2 (defined outside of auto_ptr<> according to the proposed
// resolution of lwg issue 127)
template <class _TypeT>
class auto_ptr_ref 
{
#ifndef _RWSTD_NO_MEMBER_TEMPLATES

    template <class _TypeU>
    friend class auto_ptr;

    // implicit conversion from auto_ptr<U> available only
    // if U* is implicitly convertible to T* (via inheritance)
    template <class _TypeU>
    auto_ptr_ref (auto_ptr<_TypeU>& __rhs, const _TypeU* = 0) _THROWS (())
        : _C_ptr (_RWSTD_REINTERPRET_CAST (auto_ptr<_TypeT>&,__rhs)) { }
#else   // if defined (_RWSTD_NO_MEMBER_TEMPLATES)

public:

    // useless, auto_ptr_ref necessary only for auto_ptr conversions
    auto_ptr_ref (auto_ptr<_TypeT>& __rhs, const _TypeT* = 0) _THROWS (())
        : _C_ptr (__rhs) { }

#endif   // _RWSTD_NO_MEMBER_TEMPLATES

    // reference to the owning auto_ptr object
    auto_ptr<_TypeT>& _C_ptr;
};


template<class _TypeT>
class auto_ptr
{
public:
    typedef _TypeT element_type;

    // 20.4.5.1, p1
    _EXPLICIT auto_ptr (element_type* __p = 0) _THROWS (())
     : _C_ptr (__p) { }

    // 20.4.5.1, p2
    auto_ptr (auto_ptr& __rhs) _THROWS (())
     : _C_ptr (__rhs.release ()) { }

    // 20.4.5.1, p7
    auto_ptr& operator= (auto_ptr& __rhs) _THROWS (()) { 
        reset (__rhs.release ());
        return *this;
    }

    // 20.4.5.1, p13
    ~auto_ptr () _THROWS (()) {
        delete _C_ptr;
    }

#ifndef _RWSTD_NO_MEMBER_TEMPLATES

    // 20.4.5.1, p4
    template <class _TypeU>
    auto_ptr (auto_ptr<_TypeU>& __rhs) _THROWS (())
    : _C_ptr (__rhs.release ()) { }

    // 20.4.5.1, p10
    template <class _TypeU>
    auto_ptr& operator= (auto_ptr<_TypeU>& __rhs)  _THROWS (()) { 
        reset (__rhs.release ());
        return *this;
    }

    // 20.4.5.3, p3
    template <class _TypeU>
    operator auto_ptr_ref<_TypeU>() _THROWS (()) {
        // convertible only if T* is implicitly convertible to U*
        return auto_ptr_ref<_TypeU>(*this, _C_ptr);
    }

    // 20.4.5.3, p4
    template <class _TypeU> 
    operator auto_ptr<_TypeU>() _THROWS (()) {
        return auto_ptr<_TypeU>(release ());
    }

#else   // if defined (_RWSTD_NO_MEMBER_TEMPLATES)

    // 20.4.5.3, p3 (limited to T == U)
    operator auto_ptr_ref<_TypeT>() _THROWS (()) {
        return auto_ptr_ref<_TypeT>(*this, _C_ptr);
    }

#endif   // _RWSTD_NO_MEMBER_TEMPLATES

    // 20.4.5.2, p1
    element_type & operator* () const _THROWS (()) {
        _RWSTD_ASSERT (0 != get ());
        return *get (); 
    }

    // 20.4.5.2, p3
    _RWSTD_OPERATOR_ARROW (element_type* operator-> () const _THROWS (()))

    // 20.4.5.2, p4
    element_type* get () const _THROWS (()) {
        return _C_ptr;
    }

    // 20.4.5.2, p5
    element_type* release () _THROWS (()) { 
        element_type* __tmp = _C_ptr;
        _C_ptr = 0;
        return __tmp; 
    }

    // 20.4.5.2, p7
    void reset (element_type* __p = 0) _THROWS (()) { 
        if (_C_ptr != __p) {
            delete _C_ptr;
            _C_ptr = __p;
        }
    }

    // 20.4.5.3, p1
    auto_ptr (auto_ptr_ref<element_type> __r) _THROWS (())
    : _C_ptr (__r._C_ptr.release ()) { }

    // 20.4.5.3, p? - follows lwg issue 127
    auto_ptr& operator= (auto_ptr_ref<element_type> __rhs) _THROWS (()) {
        reset (__rhs._C_ptr.release ());
        return *this;
    }

private:
    element_type* _C_ptr;
};


}   // namespace std


#endif   // _RWSTD_AUTOPTR_H_INCLUDED
