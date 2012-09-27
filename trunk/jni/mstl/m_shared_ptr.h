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

#ifndef _mstl_shared_ptr_included
#define _mstl_shared_ptr_included

#include "m_shared_base.h"

#include <stddef.h>

namespace mstl {

template <class> class auto_ptr;
#ifdef HAVE_WEAK_PTR
template <class> class weak_ptr;
#endif

template <class T>
class shared_ptr
{
public:

	// types
	typedef T element_type;
	typedef T* pointer;
	typedef T const* const_pointer;
	typedef T& reference;
	typedef T const& const_reference;

	// structors
	explicit shared_ptr(T* p = 0);
	shared_ptr(shared_ptr const& sp);
	template <class U> shared_ptr(shared_ptr<U> const& sp);
	template <class U> shared_ptr(auto_ptr<U>& ap);

#if HAVE_0X_MOVE_CONSTRCUTOR_AND_ASSIGMENT_OPERATOR
	shared_ptr(shared_ptr&& p);
	shared_ptr& operator=(shared_ptr&& p);
#endif

	// assignment
	shared_ptr& operator=(shared_ptr const& sp);
	template <class U> shared_ptr<T>& operator=(shared_ptr<U> const& sp);
	template <class U> shared_ptr<T>& operator=(auto_ptr<U>& ap);

	// queries
	bool operator!() const;
	bool unique() const;
	bool expired() const;

	// accessors
	size_t use_count() const;

	T* operator->();
	T const* operator->() const;

	T& operator*();
	T const& operator*() const;

	T* get();
	T const* get() const;

	// for comparison; e.g. 'if (p)' or 'if (p == this)'
	operator void const* () const;

	// modifiers
	T* release();
	void reset(T* p = 0);
	void swap(shared_ptr& sp);

private:

	// types
	typedef detail::shared_base<T,detail::shared_counter,delete_ptr> shared_base;

	// nested classes
	template <class U>
	struct by_ref
	{
		by_ref(shared_base const& b);

		shared_base m_base;
	};

public:

	// structors
	shared_ptr(by_ref<T> ref);

	// assignment
	shared_ptr& operator=(by_ref<T> ref);

	// accessors
	template <class U> operator by_ref<U>();

private:

#ifdef HAVE_WEAK_PTR
	// structors
	shared_ptr(weak_ptr<T> const& wp);
#endif

	// attributes
	shared_base m_base;

	// friends
	template <class U> friend class shared_ptr;
#ifdef HAVE_WEAK_PTR
	template <class U> friend class weak_ptr;
#endif
};

template <class T> void swap(shared_ptr<T>& lhs, shared_ptr<T>& rhs);

} // namespace mstl

#include "m_shared_ptr.ipp"

#endif // _mstl_shared_ptr_included

// vi:set ts=3 sw=3:
