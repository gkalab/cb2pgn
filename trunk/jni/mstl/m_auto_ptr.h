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

#ifndef _mstl_auto_ptr_included
#define _mstl_auto_ptr_included

namespace mstl {

template <class T>
class auto_ptr
{
public:

	// types
	typedef T						value_type;
	typedef value_type*			pointer;
	typedef value_type const*	const_pointer;
	typedef value_type&			reference;
	typedef value_type const&	const_reference;

	// structors
	explicit auto_ptr(pointer p = 0);
	auto_ptr(auto_ptr& ap);
	~auto_ptr();

#if HAVE_0X_MOVE_CONSTRCUTOR_AND_ASSIGMENT_OPERATOR
	auto_ptr(auto_ptr&& p);
	auto_ptr& operator=(auto_ptr&& p);
#endif

	// assignment
	auto_ptr& operator=(auto_ptr& ap);

	// queries
	bool operator!() const;

	// accessors
	reference operator*();
	const_reference operator*() const;

	pointer operator->();
	const_pointer operator->() const;

	pointer get();
	const_pointer get() const;

	// for comparison; e.g. 'if (p)'
	operator void const* () const;

	// modifiers
	pointer release();
	void reset(pointer p = 0);
	void swap(auto_ptr& ap);

private:

	// nested classes
	struct by_ref
	{
		by_ref(pointer p);
		pointer m_p;
	};

public:

	// structors
	auto_ptr(by_ref ref);

	// assignment
	auto_ptr& operator=(by_ref ref);

	// accessors
	operator by_ref();

private:

	// attributes
	pointer m_p;
};

template <typename T> void swap(auto_ptr<T>& lhs, auto_ptr<T>& rhs);

} // namespace mstl

#include "m_auto_ptr.ipp"

#endif // _mstl_auto_ptr_included

// vi:set ts=3 sw=3:
