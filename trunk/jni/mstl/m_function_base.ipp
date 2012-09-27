// ======================================================================
// Author : $Author$
// Version: $Revision$
// Date   : $Date$
// Url    : $URL$
// ======================================================================

// ======================================================================
// Copyright: (C) 2009-2012 Gregor Cramer
// ======================================================================

// ======================================================================
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
// ======================================================================

#include "m_types.h"

namespace mstl {
namespace fn_ {

inline
function_base::function_base(void* p)
	:p_(p)
	,m_(0)
{
	static_assert(sizeof(p_) == sizeof(f_), "implementation problem");
}


inline
function_base::function_base(void (*f)())
	:f_(f)
	,m_(0)
{
	static_assert(sizeof(p_) == sizeof(f_), "implementation problem");
}


inline
function_base::function_base(void (function_base::*m)(), void* p)
	:p_(p)
	,m_(m)
{
	static_assert(sizeof(p_) == sizeof(f_), "implementation problem");
}


inline
bool
function_base::empty() const
{
	return p_ == 0;
}


inline
bool
function_base::equal(function_base const& f) const
{
	return p_ == f.p_ && m_ == f.m_;
}


template <typename R>
inline
function<R ()>::function(R (*func)())
	:function_base(reinterpret_cast<void (*)()>(func))
	,invoker_(invoke_func::invoke)
{
}


template <typename R>
inline
function<R ()>::function()
	:function_base(static_cast<void*>(0))
	,invoker_(invoke_null::invoke)
{
}


template <typename R>
template <class Functor>
inline
function<R ()>::function(Functor* func)
	:function_base(func)
	,invoker_(invoke_obj<Functor>::invoke)
{
}


template <typename R>
template <class T>
inline
function<R ()>::function(R (T::*func)(), T* obj)
	:function_base(reinterpret_cast<void (function_base::*)()>(func), obj)
	,invoker_(invoke_mem<T>::invoke)
{
}


template <typename R>
template <class T>
inline
function<R ()>::function(R (T::*func)() const, T const* obj)
	:function_base(reinterpret_cast<void (function_base::*)()>(func), const_cast<T*>(obj))
	,const_invoker_(invoke_mem<T>::invoke)
{
}

template <typename R>
inline
R
function<R ()>::operator()()
{
	return invoker_(this);
}


template <typename R>
inline
R
function<R ()>::operator()() const
{
	return const_invoker_(this);
}


template <typename R>
template <class Obj>
inline
R
function<R ()>::invoke_mem<Obj>::invoke(function_base* f)
{
	return (static_cast<Obj*>(f->p_)->*reinterpret_cast<R (Obj::*)()>(f->m_))();
}


template <typename R>
template <class Obj>
inline
R
function<R ()>::invoke_mem<Obj>::invoke(function_base const* f)
{
	return (static_cast<Obj const*>(f->p_)->*reinterpret_cast<R (Obj::*)() const>(f->m_))();
}


template <typename R>
template <class Functor>
inline
R
function<R ()>::invoke_obj<Functor>::invoke(function_base* f)
{
	return (*static_cast<Functor*>(f->p_))();
}


template <typename R>
template <class Functor>
inline
R
function<R ()>::invoke_obj<Functor>::invoke(function_base const* f)
{
	return (*static_cast<Functor const*>(f->p_))();
}


template <typename R>
inline
R
function<R ()>::invoke_func::invoke(function_base* f)
{
	return (*reinterpret_cast<R (*)()>(f->f_))();
}


template <typename R>
inline
R
function<R ()>::invoke_null::invoke(function_base*)
{
	return R();
}


template <typename R, typename T1>
inline
function<R (T1)>::function(R (*func)(T1))
	:function_base(reinterpret_cast<void (*)()>(func))
	,invoker_(invoke_func::invoke)
{
}


template <typename R, typename T1>
inline
function<R (T1)>::function()
	:function_base(static_cast<void*>(0))
	,invoker_(invoke_null::invoke)
{
}


template <typename R, typename T1>
template <class Functor>
inline
function<R (T1)>::function(Functor* func)
	:function_base(func)
	,invoker_(invoke_obj<Functor>::invoke)
{
}


template <typename R, typename T1>
template <class Functor>
inline
function<R (T1)>::function(Functor const* func)
	:function_base(func)
	,const_invoker_(invoke_obj<Functor>::invoke)
{
}


template <typename R, typename T1>
template <class T>
inline
function<R (T1)>::function(R (T::*func)(T1), T* obj)
	:function_base(reinterpret_cast<void (function_base::*)()>(func), obj)
	,invoker_(invoke_mem<T>::invoke)
{
}


template <typename R, typename T1>
template <class T>
inline
function<R (T1)>::function(R (T::*func)(T1) const, T const* obj)
	:function_base(reinterpret_cast<void (function_base::*)()>(func), const_cast<T*>(obj))
	,const_invoker_(invoke_mem<T>::invoke)
{
}


template <typename R, typename T1>
inline
R
function<R (T1)>::operator()(T1 t1)
{
	return invoker_(this, t1);
}


template <typename R, typename T1>
inline
R
function<R (T1)>::operator()(T1 t1) const
{
	return const_invoker_(this, t1);
}


template <typename R, typename T1>
template <class Obj>
inline
R
function<R (T1)>::invoke_mem<Obj>::invoke(function_base* f, T1 t1)
{
	return (static_cast<Obj*>(f->p_)->*reinterpret_cast<R (Obj::*)(T1)>(f->m_))(t1);
}


template <typename R, typename T1>
template <class Obj>
inline
R
function<R (T1)>::invoke_mem<Obj>::invoke(function_base const* f, T1 t1)
{
	return (static_cast<Obj const*>(f->p_)->*reinterpret_cast<R (Obj::*)(T1) const>(f->m_))(t1);
}


template <typename R, typename T1>
template <class Functor>
inline
R
function<R (T1)>::invoke_obj<Functor>::invoke(function_base* f, T1 t1)
{
	return (*static_cast<Functor*>(f->p_))(t1);
}


template <typename R, typename T1>
template <class Functor>
inline
R
function<R (T1)>::invoke_obj<Functor>::invoke(function_base const* f, T1 t1)
{
	return (*static_cast<Functor const*>(f->p_))(t1);
}


template <typename R, typename T1>
inline
R
function<R (T1)>::invoke_func::invoke(function_base* f, T1 t1)
{
	return (*reinterpret_cast<R (*)(T1)>(f->f_))(t1);
}


template <typename R, typename T1>
inline
R
function<R (T1)>::invoke_null::invoke(function_base*, T1)
{
	return R();
}


template <typename R, typename T1, typename T2>
inline
function<R (T1,T2)>::function(R (*func)(T1, T2))
	:function_base(reinterpret_cast<void (*)()>(func))
	,invoker_(invoke_func::invoke)
{
}


template <typename R, typename T1, typename T2>
inline
function<R (T1,T2)>::function()
	:function_base(static_cast<void*>(0))
	,invoker_(invoke_null::invoke)
{
}


template <typename R, typename T1, typename T2>
template <class Functor>
inline
function<R (T1,T2)>::function(Functor* func)
	:function_base(func)
	,invoker_(invoke_obj<Functor>::invoke)
{
}


template <typename R, typename T1, typename T2>
template <class Functor>
inline
function<R (T1,T2)>::function(Functor const* func)
	:function_base(func)
	,const_invoker_(invoke_obj<Functor>::invoke)
{
}


template <typename R, typename T1, typename T2>
template <class T>
inline
function<R (T1,T2)>::function(R (T::*func)(T1, T2), T* obj)
	:function_base(reinterpret_cast<void (function_base::*)()>(func), obj)
	,invoker_(invoke_mem<T>::invoke)
{
}


template <typename R, typename T1, typename T2>
template <class T>
inline
function<R (T1,T2)>::function(R (T::*func)(T1, T2) const, T const* obj)
	:function_base(reinterpret_cast<void (function_base::*)()>(func), const_cast<T*>(obj))
	,const_invoker_(invoke_mem<T>::invoke)
{
}


template <typename R, typename T1, typename T2>
inline
R
function<R (T1,T2)>::operator()(T1 t1, T2 t2)
{
	return invoker_(this, t1, t2);
}


template <typename R, typename T1, typename T2>
inline
R
function<R (T1,T2)>::operator()(T1 t1, T2 t2) const
{
	return const_invoker_(this, t1, t2);
}


template <typename R, typename T1, typename T2>
template <class Obj>
inline
R
function<R (T1,T2)>::invoke_mem<Obj>::invoke(function_base* f, T1 t1, T2 t2)
{
	return (static_cast<Obj*>(f->p_)->*reinterpret_cast<R (Obj::*)(T1, T2)>(f->m_))(t1, t2);
}


template <typename R, typename T1, typename T2>
template <class Obj>
inline
R
function<R (T1,T2)>::invoke_mem<Obj>::invoke(function_base const* f, T1 t1, T2 t2)
{
	return (static_cast<Obj const*>(f->p_)->*reinterpret_cast<R (Obj::*)(T1, T2) const>(f->m_))(t1, t2);
}


template <typename R, typename T1, typename T2>
template <class Functor>
inline
R
function<R (T1,T2)>::invoke_obj<Functor>::invoke(function_base* f, T1 t1, T2 t2)
{
	return (*static_cast<Functor*>(f->p_))(t1, t2);
}


template <typename R, typename T1, typename T2>
template <class Functor>
inline
R
function<R (T1,T2)>::invoke_obj<Functor>::invoke(function_base const* f, T1 t1, T2 t2)
{
	return (*static_cast<Functor const*>(f->p_))(t1, t2);
}


template <typename R, typename T1, typename T2>
inline
R
function<R (T1,T2)>::invoke_func::invoke(function_base* f, T1 t1, T2 t2)
{
	return (*reinterpret_cast<R (*)(T1, T2)>(f->f_))(t1, t2);
}


template <typename R, typename T1, typename T2>
inline
R
function<R (T1,T2)>::invoke_null::invoke(function_base*, T1, T2)
{
	return R();
}


template <typename R, typename T1, typename T2, typename T3>
inline
function<R (T1,T2,T3)>::function(R (*func)(T1, T2, T3))
	:function_base(reinterpret_cast<void (*)()>(func))
	,invoker_(invoke_func::invoke)
{
}


template <typename R, typename T1, typename T2, typename T3>
inline
function<R (T1,T2,T3)>::function()
	:function_base(static_cast<void*>(0))
	,invoker_(invoke_null::invoke)
{
}


template <typename R, typename T1, typename T2, typename T3>
template <class Functor>
inline
function<R (T1,T2,T3)>::function(Functor* func)
	:function_base(func)
	,invoker_(invoke_obj<Functor>::invoke)
{
}


template <typename R, typename T1, typename T2, typename T3>
template <class Functor>
inline
function<R (T1,T2,T3)>::function(Functor const* func)
	:function_base(func)
	,const_invoker_(invoke_obj<Functor>::invoke)
{
}


template <typename R, typename T1, typename T2, typename T3>
template <class T>
inline
function<R (T1,T2,T3)>::function(R (T::*func)(T1, T2, T3) const, T const* obj)
	:function_base(reinterpret_cast<void (function_base::*)()>(func), const_cast<T*>(obj))
	,invoker_(invoke_mem<T>::invoke)
{
}


template <typename R, typename T1, typename T2, typename T3>
template <class T>
inline
function<R (T1,T2,T3)>::function(R (T::*func)(T1, T2, T3), T* obj)
	:function_base(reinterpret_cast<void (function_base::*)()>(func), obj)
	,const_invoker_(invoke_mem<T>::invoke)
{
}


template <typename R, typename T1, typename T2, typename T3>
inline
R
function<R (T1,T2,T3)>::operator()(T1 t1, T2 t2, T3 t3)
{
	return invoker_(this, t1, t2, t3);
}


template <typename R, typename T1, typename T2, typename T3>
inline
R
function<R (T1,T2,T3)>::operator()(T1 t1, T2 t2, T3 t3) const
{
	return const_invoker_(this, t1, t2, t3);
}


template <typename R, typename T1, typename T2, typename T3>
template <class Obj>
inline
R
function<R (T1,T2,T3)>::invoke_mem<Obj>::invoke(function_base* f, T1 t1, T2 t2, T3 t3)
{
	return (static_cast<Obj*>(f->p_)->*
				reinterpret_cast<R (Obj::*)(T1, T2, T3)>(f->m_))(t1, t2, t3);
}


template <typename R, typename T1, typename T2, typename T3>
template <class Obj>
inline
R
function<R (T1,T2,T3)>::invoke_mem<Obj>::invoke(function_base const* f, T1 t1, T2 t2, T3 t3)
{
	return (static_cast<Obj const*>(f->p_)->*
				reinterpret_cast<R (Obj::*)(T1, T2, T3) const>(f->m_))(t1, t2, t3);
}


template <typename R, typename T1, typename T2, typename T3>
template <class Functor>
inline
R
function<R (T1,T2,T3)>::invoke_obj<Functor>::invoke(function_base* f, T1 t1, T2 t2, T3 t3)
{
	return (*static_cast<Functor*>(f->p_))(t1, t2, t3);
}


template <typename R, typename T1, typename T2, typename T3>
template <class Functor>
inline
R
function<R (T1,T2,T3)>::invoke_obj<Functor>::invoke(function_base const* f, T1 t1, T2 t2, T3 t3)
{
	return (*static_cast<Functor const*>(f->p_))(t1, t2, t3);
}


template <typename R, typename T1, typename T2, typename T3>
inline
R
function<R (T1,T2,T3)>::invoke_func::invoke(function_base* f, T1 t1, T2 t2, T3 t3)
{
	return (*reinterpret_cast<R (*)(T1, T2, T3)>(f->p_))(t1, t2, t3);
}


template <typename R, typename T1, typename T2, typename T3>
inline
R
function<R (T1,T2,T3)>::invoke_null::invoke(function_base*, T1, T2, T3)
{
	return R();
}


template <typename R, typename T1, typename T2, typename T3, typename T4>
inline
function<R (T1,T2,T3,T4)>::function(R (*func)(T1, T2, T3, T4))
	:function_base(reinterpret_cast<void (*)()>(func))
	,invoker_(invoke_func::invoke)
{
}


template <typename R, typename T1, typename T2, typename T3, typename T4>
inline
function<R (T1,T2,T3,T4)>::function()
	:function_base(static_cast<void*>(0))
	,invoker_(invoke_null::invoke)
{
}


template <typename R, typename T1, typename T2, typename T3, typename T4>
template <class Functor>
inline
function<R (T1,T2,T3,T4)>::function(Functor* func)
	:function_base(func)
	,invoker_(invoke_obj<Functor>::invoke)
{
}


template <typename R, typename T1, typename T2, typename T3, typename T4>
template <class T>
inline
function<R (T1,T2,T3,T4)>::function(R (T::*func)(T1, T2, T3, T4), T* obj)
	:function_base(reinterpret_cast<void (function_base::*)()>(func), obj)
	,invoker_(invoke_mem<T>::invoke)
{
}


template <typename R, typename T1, typename T2, typename T3, typename T4>
template <class T>
inline
function<R (T1,T2,T3,T4)>::function(R (T::*func)(T1, T2, T3, T4) const, T const* obj)
	:function_base(reinterpret_cast<void (function_base::*)()>(func), const_cast<T*>(obj))
	,const_invoker_(invoke_mem<T>::invoke)
{
}


template <typename R, typename T1, typename T2, typename T3, typename T4>
inline
R
function<R (T1,T2,T3,T4)>::operator()(T1 t1, T2 t2, T3 t3, T4 t4)
{
	return invoker_(this, t1, t2, t3, t4);
}


template <typename R, typename T1, typename T2, typename T3, typename T4>
inline
R
function<R (T1,T2,T3,T4)>::operator()(T1 t1, T2 t2, T3 t3, T4 t4) const
{
	return const_invoker_(this, t1, t2, t3, t4);
}


template <typename R, typename T1, typename T2, typename T3, typename T4>
template <class Obj>
inline
R
function<R (T1,T2,T3,T4)>::invoke_mem<Obj>::invoke(function_base* f, T1 t1, T2 t2, T3 t3, T4 t4)
{
	return (static_cast<Obj*>(f->p_)->*
				reinterpret_cast<R (Obj::*)(T1, T2, T3, T4)>(f->m_))(t1, t2, t3, t4);
}


template <typename R, typename T1, typename T2, typename T3, typename T4>
template <class Obj>
inline
R
function<R (T1,T2,T3,T4)>::invoke_mem<Obj>::invoke(function_base const* f, T1 t1, T2 t2, T3 t3, T4 t4)
{
	return (static_cast<Obj const*>(f->p_)->*
				reinterpret_cast<R (Obj::*)(T1, T2, T3, T4) const>(f->m_))(t1, t2, t3, t4);
}


template <typename R, typename T1, typename T2, typename T3, typename T4>
template <class Functor>
inline
R
function<R (T1,T2,T3,T4)>::invoke_obj<Functor>::invoke(function_base* f, T1 t1, T2 t2, T3 t3, T4 t4)
{
	return (*static_cast<Functor*>(f->p_))(t1, t2, t3, t4);
}


template <typename R, typename T1, typename T2, typename T3, typename T4>
template <class Functor>
inline
R
function<R (T1,T2,T3,T4)>::invoke_obj<Functor>::invoke(	function_base const* f,
																			T1 t1, T2 t2, T3 t3, T4 t4)
{
	return (*static_cast<Functor const*>(f->p_))(t1, t2, t3, t4);
}


template <typename R, typename T1, typename T2, typename T3, typename T4>
inline
R
function<R (T1,T2,T3,T4)>::invoke_func::invoke(function_base* f, T1 t1, T2 t2, T3 t3, T4 t4)
{
	return (*reinterpret_cast<R (*)(T1, T2, T3, T4)>(f->p_))(t1, t2, t3, t4);
}


template <typename R, typename T1, typename T2, typename T3, typename T4>
inline
R
function<R (T1,T2,T3,T4)>::invoke_null::invoke(function_base*, T1, T2, T3, T4)
{
	return R();
}


template <typename R, typename T1, typename T2, typename T3, typename T4, typename T5>
inline
function<R (T1,T2,T3,T4,T5)>::function()
	:function_base(static_cast<void*>(0))
	,invoker_(invoke_null::invoke)
{
}


template <typename R, typename T1, typename T2, typename T3, typename T4, typename T5>
template <class Functor>
inline
function<R (T1,T2,T3,T4,T5)>::function(Functor const* func)
	:function_base(func)
	,const_invoker_(invoke_obj<Functor>::invoke)
{
}


template <typename R, typename T1, typename T2, typename T3, typename T4, typename T5>
inline
function<R (T1,T2,T3,T4,T5)>::function(R (*func)(T1, T2, T3, T4, T5))
	:function_base(reinterpret_cast<void (*)()>(func))
	,invoker_(invoke_func::invoke)
{
}


template <typename R, typename T1, typename T2, typename T3, typename T4, typename T5>
template <class T>
inline
function<R (T1,T2,T3,T4,T5)>::function(R (T::*func)(T1, T2, T3, T4, T5), T* obj)
	:function_base(reinterpret_cast<void (function_base::*)()>(func), obj)
	,invoker_(invoke_mem<T>::invoke)
{
}


template <typename R, typename T1, typename T2, typename T3, typename T4, typename T5>
template <class T>
inline
function<R (T1,T2,T3,T4,T5)>::function(R (T::*func)(T1, T2, T3, T4, T5) const, T const* obj)
	:function_base(reinterpret_cast<void (function_base::*)()>(func), const_cast<T*>(obj))
	,const_invoker_(invoke_mem<T>::invoke)
{
}


template <typename R, typename T1, typename T2, typename T3, typename T4, typename T5>
inline
R
function<R (T1,T2,T3,T4,T5)>::operator()(T1 t1, T2 t2, T3 t3, T4 t4, T5 t5)
{
	return invoker_(this, t1, t2, t3, t4, t5);
}


template <typename R, typename T1, typename T2, typename T3, typename T4, typename T5>
inline
R
function<R (T1,T2,T3,T4,T5)>::operator()(T1 t1, T2 t2, T3 t3, T4 t4, T5 t5) const
{
	return const_invoker_(this, t1, t2, t3, t4, t5);
}


template <typename R, typename T1, typename T2, typename T3, typename T4, typename T5>
template <class Obj>
inline
R
function<R (T1,T2,T3,T4,T5)>::invoke_mem<Obj>::invoke(function_base* f,
																		T1 t1, T2 t2, T3 t3, T4 t4, T5 t5)
{
	return (static_cast<Obj*>(f->p_)->*
				reinterpret_cast<R (Obj::*)(T1, T2, T3, T4, T5)>(f->m_))(t1, t2, t3, t4, t5);
}


template <typename R, typename T1, typename T2, typename T3, typename T4, typename T5>
template <class Obj>
inline
R
function<R (T1,T2,T3,T4,T5)>::invoke_mem<Obj>::invoke(	function_base const* f,
																			T1 t1, T2 t2, T3 t3, T4 t4, T5 t5)
{
	return (static_cast<Obj const*>(f->p_)->*
				reinterpret_cast<R (Obj::*)(T1, T2, T3, T4, T5) const>(f->m_))(t1, t2, t3, t4, t5);
}


template <typename R, typename T1, typename T2, typename T3, typename T4, typename T5>
template <class Functor>
inline
R
function<R (T1,T2,T3,T4,T5)>::invoke_obj<Functor>::invoke(	function_base* f,
																				T1 t1, T2 t2, T3 t3, T4 t4, T5 t5)
{
	return (*static_cast<Functor*>(f->p_))(t1, t2, t3, t4, t5);
}


template <typename R, typename T1, typename T2, typename T3, typename T4, typename T5>
template <class Functor>
inline
R
function<R (T1,T2,T3,T4,T5)>::invoke_obj<Functor>::invoke(	function_base const* f,
																				T1 t1, T2 t2, T3 t3, T4 t4, T5 t5)
{
	return (*static_cast<Functor const*>(f->p_))(t1, t2, t3, t4, t5);
}


template <typename R, typename T1, typename T2, typename T3, typename T4, typename T5>
inline
R
function<R (T1,T2,T3,T4,T5)>::invoke_func::invoke(function_base* f, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5)
{
	return (*reinterpret_cast<R (*)(T1, T2, T3, T4, T5)>(f->p_))(t1, t2, t3, t4, t5);
}


template <typename R, typename T1, typename T2, typename T3, typename T4, typename T5>
inline
R
function<R (T1,T2,T3,T4,T5)>::invoke_null::invoke(function_base*, T1, T2, T3, T4, T5)
{
	return R();
}


template <typename R, typename T1, typename T2, typename T3, typename T4, typename T5>
template <class Functor>
inline
function<R (T1,T2,T3,T4,T5)>::function(Functor* func)
	:function_base(func)
	,invoker_(invoke_obj<Functor>::invoke)
{
}

} // namespace fn_
} // namespace mstl

// vi:set ts=3 sw=3:
