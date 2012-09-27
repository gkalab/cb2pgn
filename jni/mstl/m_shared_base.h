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

#ifndef _gain_shared_base_included
#define _gain_shared_base_included

#include <stddef.h>

namespace mstl {
namespace detail {

namespace sbd_ { class counter; }

class weak_counter;
class shared_counter;


template <class T, class Counter, class Deleter>
class shared_base
{
public:

	// structors
	shared_base();
	shared_base(T* p);
	shared_base(shared_base<T,weak_counter,Deleter> const& sb);
	shared_base(shared_base<T,shared_counter,Deleter> const& sb);
	template <class U> shared_base(shared_base<U,shared_counter,Deleter> const& sb);
	~shared_base();

#if HAVE_0X_MOVE_CONSTRCUTOR_AND_ASSIGMENT_OPERATOR
	shared_base(shared_base&& sb);
	shared_base& operator=(shared_base&& sb);
#endif

	// assignment
	shared_base& operator=(shared_base<T,weak_counter,Deleter> const& sb);
	shared_base& operator=(shared_base<T,shared_counter,Deleter> const& sb);
	template <class U> shared_base& operator=(shared_base<U,shared_counter,Deleter> const& sb);

	// accessors
	size_t use_count() const;
	bool unique() const;
	bool expired() const;

	T* get();
	T const* get() const;

	// modifiers
	T* release();
	void reset();
	void reset(T* p);
	void swap(shared_base& sb);

private:

	// class invariance
	bool class_invariant() const;

	// modifiers
	void reset(T* p, sbd_::counter* count);

	// attributes
	T*					m_p;
	sbd_::counter*	m_count;

#if (__GNUC__ >= 3)
	// friends
	template <class, class, class> friend class shared_base;
#endif
};

} // namespace detail

struct delete_ptr
{
	template <class T> static void dispose(T* p);
};

} // namespace mstl

#include "m_shared_base.ipp"

#endif // _gain_shared_base_included

// vi:set ts=3 sw=3:
