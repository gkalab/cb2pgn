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

#include "m_cast.h"
#include "m_assert.h"

namespace mstl {

template <class T>
inline
scoped_ptr<T>::scoped_ptr(T* p)
	:m_p(p)
{
}


template <class T>
template <class U>
inline
scoped_ptr<T>::scoped_ptr(auto_ptr<U>& ap)
{
	U* p = ap.release();
	m_p = safe_cast_ptr<T>(p);
}


template <class T>
inline
scoped_ptr<T>::~scoped_ptr()
{
	delete m_p;
}


template <class T>
inline
bool
scoped_ptr<T>::operator!() const
{
	return m_p == 0;
}


template <class T>
inline
T*
scoped_ptr<T>::get()
{
	return m_p;
}


template <class T>
inline
T const*
scoped_ptr<T>::get() const
{
	return m_p;
}


template <class T>
inline
T&
scoped_ptr<T>::operator*()
{
	M_REQUIRE(get());
	return *m_p;
}


template <class T>
inline
T const&
scoped_ptr<T>::operator*() const
{
	M_REQUIRE(get());
	return *m_p;
}


template <class T>
inline
T*
scoped_ptr<T>::operator->()
{
	M_REQUIRE(get());
	return m_p;
}


template <class T>
inline
T const*
scoped_ptr<T>::operator->() const
{
	M_REQUIRE(get());
	return m_p;
}


template <class T>
inline
T*
scoped_ptr<T>::release()
{
	T* p = m_p;
	m_p = 0;
	return p;
}


template <class T>
inline
void
scoped_ptr<T>::reset(T* p)
{
	if (m_p != p)
	{
		delete m_p;
		m_p = p;
	}
}


template <class T>
template <class U>
inline
void
scoped_ptr<T>::reset(auto_ptr<U>& ap)
{
	G_ASSERT(m_p != ap.get());
	delete m_p;
	U* p = ap.release();
	m_p = safe_cast_ptr<T>(p);
}


template <class T>
inline
void
scoped_ptr<T>::swap(scoped_ptr& sp)
{
	mstl::swap(m_p, sp.m_p);
}


template <class T>
scoped_ptr<T>::operator void const* () const
{
	return m_p;
}

} // namespace mstl

// vi:set ts=3 sw=3:
