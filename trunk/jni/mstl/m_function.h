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

#ifndef _mstl_function_included
#define _mstl_function_included

#include "m_function_base.h"

namespace mstl {

template <typename Signature>
class function : public fn_::function<Signature>
{
protected:

	typedef fn_::function<Signature> base;

public:

	// types
	typedef Signature signature;

	// structors
	function();
	function(Signature& func);
	template <class Obj> function(typename base::template meth_t<Obj>::method_ptr meth, Obj* obj);
	template <class Obj>
		function(typename base::template meth_t<Obj>::const_method_ptr meth, Obj const* obj);
	template <class Functor> function(Functor* functor);
	template <class Functor> function(Functor const* functor);

	// comparison
	bool operator==(function const& f) const;

	// for comparison; e.g. 'if (p)'
	operator bool () const;

	// queries
	bool empty() const;
};

} // namespace mstl

#include "m_function.ipp"

#endif // _mstl_function_included

///////////////////////////////////////////////////////////////
// Examples
///////////////////////////////////////////////////////////////
//#include <boost/function.hpp>
//#include <boost/functional.hpp>
//
//using namespace mstl;
//
//int foo(int i) { return i; }
//struct Baz { int operator()(int i) const { return i; } };
//
//int
//main()
//{
//	Baz baz;
//
//	{
//		int (*f)(int) = foo;
//		boost::function<int (int)> g(boost::bind1st(boost::mem_fun(&Baz::operator()), &baz));
//		int (Baz::*h)(int) const = &Baz::operator();
//		Baz i;
//
//		f(1);				//  5 time units
//		g(1);				// 15 time units
//		(baz.*h)(1);	//  6 time units
//		i(1);				//  1 time unit
//	}
//
//	{
//		function<int (int)> f(foo);
//		boost::function<int (int)> g(boost::bind1st(boost::mem_fun(&Baz::operator()), &baz));
//		function<int (int)> h(&Baz::operator(), static_cast<Baz const*>(&baz));
//		function<int (int)> i(&baz);
//
//		f(1);				// 12 time units
//		g(1);				// 15 time units
//		h(1);				// 13 time units
//		i(1);				//  8 time units
//	}
//
//	return 0;
//}

// vi:set ts=3 sw=3:
