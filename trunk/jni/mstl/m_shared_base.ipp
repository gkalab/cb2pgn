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

#include "m_assert.h"
#include "m_cast.h"
#include "m_utility.h"

namespace mstl {
namespace detail {
namespace sbd_ {

struct counter
{
	counter()
		:m_shared(1)
#ifdef HAVE_WEAK_PTR
		,m_weak(1)
#endif
{
}

	size_t m_shared;
#ifdef HAVE_WEAK_PTR
	size_t m_weak;
#endif
};


template <class T> struct is_shared_counter	{ static bool const Value = false; };
#ifdef HAVE_WEAK_PTR
template <class T> struct is_weak_counter		{ static bool const Value = false; };
#endif

} // namespace sbd_


#ifdef HAVE_WEAK_PTR
struct weak_counter
{
	static void ref(sbd_::counter* count)
	{
		M_REQUIRE(count);
		++count->m_weak;
	}

	static bool release(sbd_::counter* count)
	{
		M_REQUIRE(count);
		M_ASSERT(count->m_weak > 0);

		--count->m_weak;
		return false;
	}
};
#endif


struct shared_counter
{
	static void ref(sbd_::counter* count)
	{
		M_REQUIRE(count);
#ifdef HAVE_WEAK_PTR
		M_ASSERT(count->m_shared != 0 || count->m_weak == 0);
#endif

		++count->m_shared;
#ifdef HAVE_WEAK_PTR
		++count->m_weak;
#endif
	}

	static bool release(sbd_::counter* count)
	{
		M_REQUIRE(count);
		M_ASSERT(count->m_shared > 0);
#ifdef HAVE_WEAK_PTR
		M_ASSERT(count->m_shared <= count->m_weak);

		--count->m_weak;
#endif
		return --count->m_shared == 0;
	}
};


namespace sbd_ {

template <> struct is_shared_counter<shared_counter>	{ static bool const Value = true; };
#ifdef HAVE_WEAK_PTR
template <> struct is_weak_counter<weak_counter>		{ static bool const Value = true; };
#endif

} // namespace sbd_


template <class T, class Counter, class Deleter>
inline
T*
shared_base<T,Counter,Deleter>::get()
{
	return m_p;
}


template <class T, class Counter, class Deleter>
inline
T const*
shared_base<T,Counter,Deleter>::get() const
{
	return m_p;
}


template <class T, class Counter, class Deleter>
inline
size_t
shared_base<T,Counter,Deleter>::use_count() const
{
	return m_count ? m_count->m_shared : 0;
}


template <class T, class Counter, class Deleter>
inline
bool
shared_base<T,Counter,Deleter>::unique() const
{
	return use_count() == 1;
}


template <class T, class Counter, class Deleter>
inline
bool
shared_base<T,Counter,Deleter>::expired() const
{
	return use_count() == 0;
}


template <class T, class Counter, class Deleter>
inline
bool
shared_base<T,Counter,Deleter>::class_invariant() const
{
#ifdef HAVE_WEAK_PTR
	return m_count == 0 ?	m_p == 0
							 :	m_p != 0 && m_count->m_shared > 0 && m_count->m_weak >= m_count->m_shared;
#else
	return m_count == 0 ? m_p == 0 : m_p != 0 && m_count->m_shared > 0;
#endif
}


template <class T, class Counter, class Deleter>
inline
shared_base<T,Counter,Deleter>::shared_base()
	:m_p(0)
	,m_count(0)
{
	M_ASSERT(class_invariant());
}


template <class T, class Counter, class Deleter>
inline
shared_base<T,Counter,Deleter>::shared_base(T* p)
	:m_p(p)
	,m_count(m_p ? new sbd_::counter() : 0)
{
#ifdef HAVE_WEAK_PTR
	static_assert(sbd_::is_shared_counter<Counter>::Value, "type not allowed for shared pointer");
#endif
	M_ASSERT(class_invariant());
}


#ifdef HAVE_WEAK_PTR
template <class T, class Counter, class Deleter>
inline
shared_base<T,Counter,Deleter>::shared_base(shared_base<T,weak_counter,Deleter> const& sb)
	:m_p(sb.m_p)
	,m_count(sb.m_count)
{
	if (m_count)
		Counter::ref(m_count);

	M_ASSERT(class_invariant());
}
#endif


template <class T, class Counter, class Deleter>
shared_base<T,Counter,Deleter>::shared_base(shared_base<T,shared_counter,Deleter> const& sb)
	:m_p(0)
	,m_count(0)
{
	if (!sb.expired())
	{
		m_p = sb.m_p;
		m_count = sb.m_count;
		Counter::ref(m_count);
	}

	M_ASSERT(class_invariant());
}


template <class T, class Counter, class Deleter>
template <class U>
inline
shared_base<T,Counter,Deleter>::shared_base(shared_base<U,shared_counter,Deleter> const& sb)
	:m_p(safe_cast_ptr<T>(sb.m_p))
	,m_count(sb.m_count)
{
#ifdef HAVE_WEAK_PTR
	static_assert(sbd_::is_shared_counter<Counter>::Value, "type not allowed for shared pointer");
#endif

	if (m_count)
		Counter::ref(m_count);

	M_ASSERT(class_invariant());
}


template <class T, class Counter, class Deleter>
inline
shared_base<T,Counter,Deleter>::~shared_base()
{
	reset();
}


template <class T, class Counter, class Deleter>
shared_base<T,Counter,Deleter>&
shared_base<T,Counter,Deleter>::operator=(shared_base<T,weak_counter,Deleter> const& sb)
{
#ifdef HAVE_WEAK_PTR
	static_assert(sbd_::is_weak_counter<Counter>::Value, "type not allowed for weak pointer");
#endif

	if (m_p != sb.m_p)
	{
		if (sb.expired())
			reset();
		else
			reset(sb.m_p, sb.m_count);
	}

	return *this;
}


template <class T, class Counter, class Deleter>
inline
shared_base<T,Counter,Deleter>&
shared_base<T,Counter,Deleter>::operator=(shared_base<T,shared_counter,Deleter> const& sb)
{
	if (m_p != sb.m_p)
		reset(sb.m_p, sb.m_count);

	return *this;
}


template <class T, class Counter, class Deleter>
template <class U>
inline
shared_base<T,Counter,Deleter>&
shared_base<T,Counter,Deleter>::operator=(shared_base<U,shared_counter,Deleter> const& sb)
{
#ifdef HAVE_WEAK_PTR
	G_STATIC_CHECK(sbd_::is_shared_counter<Counter>::Value, Not_Allowed_For_Weak_Pointer);
#endif

	if (m_p != sb.m_p)
		reset(safe_cast_ptr<T>(sb.m_p), sb.m_count);

	return *this;
}


template <class T, class Counter, class Deleter>
void
shared_base<T,Counter,Deleter>::reset()
{
	if (m_count)
	{
		if (Counter::release(m_count))
			delete m_p;

#ifdef HAVE_WEAK_PTR
		if (m_count->m_weak == 0)
#else
		if (m_count->m_shared == 0)
#endif
			delete m_count;

		m_count = 0;
		m_p = 0;
	}

	M_ASSERT(class_invariant());
}


template <class T, class Counter, class Deleter>
void
shared_base<T,Counter,Deleter>::reset(T* p)
{
#ifdef HAVE_WEAK_PTR
	static_assert(sbd_::is_shared_counter<Counter>::Value, "type not allowed for shared pointer");
#endif
	M_REQUIRE(p == 0 || m_p != p);

	if (m_count)
	{
		if (Counter::release(m_count))
			delete m_p;

#ifdef HAVE_WEAK_PTR
		if (m_count->m_weak == 0)
#else
		if (m_count->m_shared == 0)
#endif
		{
			if ((m_p = p))
			{
				m_count->m_shared = 1;	// reuse expired counter
#ifdef HAVE_WEAK_PTR
				m_count->m_weak = 1;
#endif
			}
			else
			{
				delete m_count;
				m_count = 0;
			}
		}
		else
		{
			m_count = (m_p = p) ? new sbd_::counter() : 0;
		}
	}
	else if (p != 0)
	{
		m_p = p;
		m_count = new sbd_::counter();
	}

	M_ASSERT(class_invariant());
}


template <class T, class Counter, class Deleter>
void
shared_base<T,Counter,Deleter>::reset(T* p, sbd_::counter* count)
{
	M_REQUIRE(m_p != p);
	M_REQUIRE(m_count != count);

	reset();
	m_p = p;

	if ((m_count = count))
		Counter::ref(m_count);

	M_ASSERT(class_invariant());
}


template <class T, class Counter, class Deleter>
T*
shared_base<T,Counter,Deleter>::release()
{
	T* p = m_p;

	if (m_count)
	{
		Counter::release(m_count);

#ifdef HAVE_WEAK_PTR
		if (m_count->m_weak == 0)
#else
		if (m_count->m_shared == 0)
#endif
			delete m_count;

		m_count = 0;
		m_p = 0;
	}

	M_ASSERT(class_invariant());

	return p;
}


template <class T, class Counter, class Deleter>
inline
void
shared_base<T,Counter,Deleter>::swap(shared_base& sb)
{
	mstl::swap(m_p, sb.m_p);
	mstl::swap(m_count, sb.m_count);
}


#if HAVE_0X_MOVE_CONSTRCUTOR_AND_ASSIGMENT_OPERATOR

template <class T, class Counter, class Deleter>
inline
shared_base<T,Counter,Deleter>::shared_base(shared_base&& sb)
	:m_p(sb.m_p)
	,m_count(sb.m_count)
{
	sb.m_p = 0;
	sb.m_count = 0;
}


template <class T, class Counter, class Deleter>
inline
shared_base<T,Counter,Deleter>&
shared_base<T,Counter,Deleter>::operator=(shared_base&& sb)
{
	swap(sb);
	return *this;
}

#endif

} // namespace detail


template <class T>
inline
void
delete_ptr::dispose(T* p)
{
	delete p;
}
} // namespace mstl

// vi:set ts=3 sw=3:
