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
#include "m_utility.h"
#include "m_cast.h"

namespace mstl {

template <class T>
template <class U>
ref_counted_ptr<T>::by_ref<U>::by_ref(U* p)
	:m_p(safe_cast_ptr<T>(p))
{
}


template <class T>
inline
ref_counted_ptr<T>::ref_counted_ptr(T* p)
	:m_p(p)
{
	if (m_p)
		mstl::ref_counted_traits<T>::ref(m_p);
}


template <class T>
inline
ref_counted_ptr<T>::ref_counted_ptr(ref_counted_ptr<T> const& sp)
	:m_p(sp.m_p)
{
	if (m_p)
		mstl::ref_counted_traits<T>::ref(m_p);
}


template <class T>
template <class U>
inline
ref_counted_ptr<T>::ref_counted_ptr(ref_counted_ptr<U> const& sp)
	:m_p(safe_cast_ptr<T>(sp.m_p))
{
	if (m_p)
		mstl::ref_counted_traits<T>::ref(m_p);
}


template <class T>
inline
ref_counted_ptr<T>::ref_counted_ptr(by_ref<T> ref)
	:m_p(ref.m_p)
{
	if (m_p)
		mstl::ref_counted_traits<T>::ref(m_p);
}


template <class T>
inline
ref_counted_ptr<T>::~ref_counted_ptr()
{
	reset();
}


template <class T>
inline
ref_counted_ptr<T>&
ref_counted_ptr<T>::operator=(ref_counted_ptr<T> const& sp)
{
	reset(sp.m_p);
	return *this;
}


template <class T>
template <class U>
inline
ref_counted_ptr<T>&
ref_counted_ptr<T>::operator=(ref_counted_ptr<U> const& sp)
{
	reset(safe_cast_ptr<T>(sp.m_p));
	return *this;
}


template <class T>
ref_counted_ptr<T>&
ref_counted_ptr<T>::operator=(by_ref<T> ref)
{
	if (m_p != ref.m_p)
	{
		if (m_p && mstl::ref_counted_traits<T>::release(m_p))
			delete m_p;

		if ((m_p = ref.m_p))
			mstl::ref_counted_traits<T>::ref(m_p);
	}

	return *this;
}


template <class T>
inline
T*
ref_counted_ptr<T>::release()
{
	T* p = m_p;
	m_p = 0;
	return p;
}


template <class T>
void
ref_counted_ptr<T>::reset(T* p)
{
	if (p != m_p)
	{
		if (m_p && mstl::ref_counted_traits<T>::release(m_p))
			delete m_p;

		if ((m_p = p))
			mstl::ref_counted_traits<T>::ref(m_p);
	}
}


template <class T>
inline
bool
ref_counted_ptr<T>::operator!() const
{
	return m_p == 0;
}


template <class T>
inline
bool
ref_counted_ptr<T>::expired() const
{
	return mstl::ref_counted_traits<T>::expired(m_p);
}


template <class T>
inline
bool
ref_counted_ptr<T>::unique() const
{
	return mstl::ref_counted_traits<T>::unique(m_p);
}


template <class T>
inline
T*
ref_counted_ptr<T>::get()
{
	return m_p;
}


template <class T>
inline
T const*
ref_counted_ptr<T>::get() const
{
	return m_p;
}


template <class T>
inline
T*
ref_counted_ptr<T>::operator->()
{
	// M_REQUIRE(!expired());

	return m_p;
}


template <class T>
inline
T const*
ref_counted_ptr<T>::operator->() const
{
	// M_REQUIRE(!expired());

	return m_p;
}


template <class T>
inline
T&
ref_counted_ptr<T>::operator*()
{
	// M_REQUIRE(!expired());

	return *m_p;
}


template <class T>
inline
T const&
ref_counted_ptr<T>::operator*() const
{
	// M_REQUIRE(!expired());

	return *m_p;
}


template <class T>
template <class U>
inline
#if (__GNUC__ < 3)
ref_counted_ptr<T>::operator ref_counted_ptr<T>::by_ref<U>()
#else
ref_counted_ptr<T>::operator by_ref<U>()
#endif
{
	return by_ref<U>(m_p);
}


template <class T>
inline
ref_counted_ptr<T>::operator void const* () const
{
	return m_p;
}


template <class T>
void
ref_counted_ptr<T>::swap(ref_counted_ptr& p)
{
	mstl::swap(m_p, p.m_p);
}


template <class T>
inline
void
swap(ref_counted_ptr<T>& lhs, ref_counted_ptr<T>& rhs)
{
	lhs.swap(rhs);
}


#if HAVE_0X_MOVE_CONSTRCUTOR_AND_ASSIGMENT_OPERATOR

template <class T>
inline
ref_counted_ptr<T>::ref_counted_ptr(ref_counted_ptr&& p)
	:m_p(p.m_p)
{
	p.m_p = 0;
}


template <class T>
inline
ref_counted_ptr<T>&
ref_counted_ptr<T>::operator=(ref_counted_ptr&& p)
{
	mstl::swap(m_p, p.m_p);
	return *this;
}

#endif

} // namespace mstl

// vi:set ts=3 sw=3:
