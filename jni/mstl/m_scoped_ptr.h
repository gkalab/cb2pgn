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

#ifndef _mstl_scoped_ptr_included
#define _mstl_scoped_ptr_included

#include "m_utility.h"

namespace mstl {

template <class> class auto_ptr;

template <class T>
class scoped_ptr : private mstl::noncopyable
{
public:

	// types
	typedef T element_type;
	typedef T* pointer;
	typedef T const* const_pointer;
	typedef T& reference;
	typedef T const& const_reference;

	// structors
	explicit scoped_ptr(T* p = 0);
	template <class U> scoped_ptr(auto_ptr<U>& ap);
	~scoped_ptr();

	// queries
	bool operator!() const;

	// accessors
	T& operator*();
	T const& operator*() const;

	T* operator->();
	T const* operator->() const;

	T* get();
	T const* get() const;

	// for comparison; e.g. 'if (p)'
	operator void const* () const;

	// modifiers
	T* release();
	void reset(T* p = 0);
	template <class U> void reset(auto_ptr<U>& ap);
	void swap(scoped_ptr& sp);

private:

	// attributes
	T* m_p;
};

} // namespace mstl

#include "m_scoped_ptr.ipp"

#endif // _mstl_scoped_ptr_included

// vi:set ts=3 sw=3:
