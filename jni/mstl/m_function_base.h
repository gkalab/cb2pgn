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

#ifndef _mstl_function_base_included
#define _mstl_function_base_included

namespace mstl {
namespace fn_ {

template <typename> class function;


struct function_base
{
	// structors
	function_base(void* p);
	function_base(void (*f)());
	function_base(void (function_base::*m)(), void* p);

	// queries
	bool equal(function_base const& f) const;
	bool empty() const;

	// attributes
	union
	{
		void (*f_)();
		void*	p_;
	};
	void (function_base::*m_)();
};


template <typename R>
class function<R ()> : protected function_base
{
public:

	// constants
	static unsigned const Arity = 0;

	// types
	typedef R result_type;

	// nested classes
	template <class Obj>
	struct meth_t
	{
		typedef R (Obj::*method_ptr)();
		typedef R (Obj::*const_method_ptr)() const;
	};

	// structors
	function();
	function(R (*func)());
	template <class T> function(R (T::*func)(), T* obj);
	template <class T> function(R (T::*func)() const, T const* obj);
	template <class Functor> function(Functor* func);
	template <class Functor> function(Functor const* func);

	// invocation
	R operator()();
	R operator()() const;

private:

	// helpers
	struct invoke_func { static R invoke(function_base* f); };
	struct invoke_null { static R invoke(function_base* f); };

	template <class Functor> struct invoke_obj
	{
		static R invoke(function_base* f);
		static R invoke(function_base const* f);
	};
	template <class Obj> struct invoke_mem
	{
		static R invoke(function_base* f);
		static R invoke(function_base const* f);
	};

	// attributes
	union
	{
		R (*invoker_)(function_base*);
		R (*const_invoker_)(function_base const*);
	};
};


template <typename R, typename T1>
class function<R (T1)> : protected function_base
{
public:

	// constants
	static unsigned const Arity = 1;

	// types
	typedef R	result_type;
	typedef T1	arg1_type;

	// nested classes
	template <class Obj>
	struct meth_t
	{
		typedef R (Obj::*method_ptr)(T1);
		typedef R (Obj::*const_method_ptr)(T1);
	};

	// structors
	function();
	function(R (*func)(T1));
	template <class T> function(R (T::*func)(T1), T* obj);
	template <class T> function(R (T::*func)(T1) const, T const* obj);
	template <class Functor> function(Functor* func);
	template <class Functor> function(Functor const* func);

	// invocation
	R operator()(T1 t1);
	R operator()(T1 t1) const;

private:

	// helpers
	struct invoke_func { static R invoke(function_base* f, T1 t1); };
	struct invoke_null { static R invoke(function_base* f, T1 t1); };

	template <class Functor> struct invoke_obj
	{
		static R invoke(function_base* f, T1 t1);
		static R invoke(function_base const* f, T1 t1);
	};
	template <class Obj> struct invoke_mem
	{
		static R invoke(function_base* f, T1 t1);
		static R invoke(function_base const* f, T1 t1);
	};

	// attributes
	union
	{
		R (*invoker_)(function_base*, T1);
		R (*const_invoker_)(function_base const*, T1);
	};
};


template <typename R, typename T1, typename T2>
class function<R (T1, T2)> : protected function_base
{
public:

	// constants
	static unsigned const Arity = 2;

	// types
	typedef R	result_type;
	typedef T1	arg1_type;
	typedef T2	arg2_type;

	// nested classes
	template <class Obj>
	struct meth_t
	{
		typedef R (Obj::*method_ptr)(T1, T2);
		typedef R (Obj::*const_method_ptr)(T1, T2);
	};

	// structors
	function();
	function(R (*func)(T1, T2));
	template <class T> function(R (T::*func)(T1, T2), T* obj);
	template <class T> function(R (T::*func)(T1, T2) const, T const* obj);
	template <class Functor> function(Functor* func);
	template <class Functor> function(Functor const* func);

	// invocation
	R operator()(T1 t1, T2 t2);
	R operator()(T1 t1, T2 t2) const;

private:

	// helpers
	struct invoke_func { static R invoke(function_base* f, T1 t1, T2 t2); };
	struct invoke_null { static R invoke(function_base* f, T1 t1, T2 t2); };

	template <class Functor> struct invoke_obj
	{
		static R invoke(function_base* f, T1 t1, T2 t2);
		static R invoke(function_base const* f, T1 t1, T2 t2);
	};
	template <class Obj> struct invoke_mem
	{
		static R invoke(function_base* f, T1 t1, T2 t2);
		static R invoke(function_base const* f, T1 t1, T2 t2);
	};

	// attributes
	union
	{
		R (*invoker_)(function_base*, T1, T2);
		R (*const_invoker_)(function_base const*, T1, T2);
	};
};


template <typename R, typename T1, typename T2, typename T3>
class function<R (T1, T2, T3)> : protected function_base
{
public:

	// constants
	static unsigned const Arity = 3;

	// types
	typedef R	result_type;
	typedef T1	arg1_type;
	typedef T2	arg2_type;
	typedef T3	arg3_type;

	// nested classes
	template <class Obj>
	struct meth_t
	{
		typedef R (Obj::*method_ptr)(T1, T2, T3);
		typedef R (Obj::*const_method_ptr)(T1, T2, T3);
	};

	// structors
	function();
	function(R (*func)(T1, T2, T3));
	template <class T> function(R (T::*func)(T1, T2, T3), T* obj);
	template <class T> function(R (T::*func)(T1, T2, T3) const, T const* obj);
	template <class Functor> function(Functor* func);
	template <class Functor> function(Functor const* func);

	// invocation
	R operator()(T1 t1, T2 t2, T3 t3);
	R operator()(T1 t1, T2 t2, T3 t3) const;

private:

	// helpers
	struct invoke_func { static R invoke(function_base* f, T1 t1, T2 t2, T3 t3); };
	struct invoke_null { static R invoke(function_base* f, T1 t1, T2 t2, T3 t3); };

	template <class Functor> struct invoke_obj
	{
		static R invoke(function_base* f, T1 t1, T2 t2, T3 t3);
		static R invoke(function_base const* f, T1 t1, T2 t2, T3 t3);
	};
	template <class Obj> struct invoke_mem
	{
		static R invoke(function_base* f, T1 t1, T2 t2, T3 t3);
		static R invoke(function_base const* f, T1 t1, T2 t2, T3 t3);
	};

	// attributes
	union
	{
		R (*invoker_)(function_base*, T1, T2, T3);
		R (*const_invoker_)(function_base const*, T1, T2, T3);
	};
};


template <typename R, typename T1, typename T2, typename T3, typename T4>
class function<R (T1, T2, T3, T4)> : protected function_base
{
public:

	// constants
	static unsigned const Arity = 4;

	// types
	typedef R	result_type;
	typedef T1	arg1_type;
	typedef T2	arg2_type;
	typedef T3	arg3_type;
	typedef T4	arg4_type;

	// nested classes
	template <class Obj>
	struct meth_t
	{
		typedef R (Obj::*method_ptr)(T1, T2, T3, T4);
		typedef R (Obj::*const_method_ptr)(T1, T2, T3, T4);
	};

	// structors
	function();
	function(R (*func)(T1, T2, T3, T4));
	template <class T> function(R (T::*func)(T1, T2, T3, T4), T* obj);
	template <class T> function(R (T::*func)(T1, T2, T3, T4) const, T const* obj);
	template <class Functor> function(Functor* func);
	template <class Functor> function(Functor const* func);

	// invocation
	R operator()(T1 t1, T2 t2, T3 t3, T4 t4);
	R operator()(T1 t1, T2 t2, T3 t3, T4 t4) const;

private:

	// helpers
	struct invoke_func { static R invoke(function_base* f, T1 t1, T2 t2, T3 t3, T4 t4); };
	struct invoke_null { static R invoke(function_base* f, T1 t1, T2 t2, T3 t3, T4 t4); };

	template <class Functor> struct invoke_obj
	{
		static R invoke(function_base* f, T1 t1, T2 t2, T3 t3, T4 t4);
		static R invoke(function_base const* f, T1 t1, T2 t2, T3 t3, T4 t4);
	};
	template <class Obj> struct invoke_mem
	{
		static R invoke(function_base* f, T1 t1, T2 t2, T3 t3, T4 t4);
		static R invoke(function_base const* f, T1 t1, T2 t2, T3 t3, T4 t4);
	};

	// attributes
	union
	{
		R (*invoker_)(function_base*, T1, T2, T3, T4);
		R (*const_invoker_)(function_base const*, T1, T2, T3, T4);
	};
};


template <typename R, typename T1, typename T2, typename T3, typename T4, typename T5>
class function<R (T1, T2, T3, T4, T5)> : protected function_base
{
public:

	// constants
	static unsigned const Arity = 5;

	// types
	typedef R	result_type;
	typedef T1	arg1_type;
	typedef T2	arg2_type;
	typedef T3	arg3_type;
	typedef T4	arg4_type;
	typedef T5	arg5_type;

	// nested classes
	template <class Obj>
	struct meth_t
	{
		typedef R (Obj::*method_ptr)(T1, T2, T3, T4, T5);
		typedef R (Obj::*const_method_ptr)(T1, T2, T3, T4, T5);
	};

	// structors
	function();
	function(R (*func)(T1, T2, T3, T4, T5));
	template <class T> function(R (T::*func)(T1, T2, T3, T4, T5), T* obj);
	template <class T> function(R (T::*func)(T1, T2, T3, T4, T5) const, T const* obj);
	template <class Functor> function(Functor* func);
	template <class Functor> function(Functor const* func);

	// invocation
	R operator()(T1 t1, T2 t2, T3 t3, T4 t4, T5 t5);
	R operator()(T1 t1, T2 t2, T3 t3, T4 t4, T5 t5) const;

private:

	// helpers
	struct invoke_func { static R invoke(function_base* f, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5); };
	struct invoke_null { static R invoke(function_base* f, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5); };

	template <class Functor> struct invoke_obj
	{
		static R invoke(function_base* f, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5);
		static R invoke(function_base const* f, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5);
	};
	template <class Obj> struct invoke_mem
	{
		static R invoke(function_base* f, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5);
		static R invoke(function_base const* f, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5);
	};

	// attributes
	union
	{
		R (*invoker_)(function_base*, T1, T2, T3, T4, T5);
		R (*const_invoker_)(function_base const*, T1, T2, T3, T4, T5);
	};
};

} // namespace fn_
} // namespace mstl

#include "m_function_base.ipp"

#endif // _mstl_function_base_included

// vi:set ts=3 sw=3:
