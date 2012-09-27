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

#include "m_utility.h"
#include "m_assert.h"

namespace mstl {

template <class T>
template <class U>
shared_ptr<T>::by_ref<U>::by_ref(shared_ptr<T>::shared_base const& base)
	:m_base(base)
{
}


template <class T>
inline
shared_ptr<T>::shared_ptr(T* p)
	:m_base(p)
{
}


template <class T>
inline
shared_ptr<T>::shared_ptr(shared_ptr<T> const& sp)
	:m_base(sp.m_base)
{
}


template <class T>
template <class U>
inline
shared_ptr<T>::shared_ptr(shared_ptr<U> const& sp)
	:m_base(sp.m_base)
{
}


#ifdef HAVE_WEAK_PTR
template <class T>
inline
shared_ptr<T>::shared_ptr(weak_ptr<T> const& wp)
	:m_base(wp.m_base)
{
}
#endif


template <class T>
inline
shared_ptr<T>::shared_ptr(by_ref<T> ref)
	:m_base(ref.m_base)
{
}


template <class T>
template <class U>
inline
shared_ptr<T>::shared_ptr(auto_ptr<U>& ap)
	:m_base(ap.release())
{
}


template <class T>
inline
shared_ptr<T>&
shared_ptr<T>::operator=(shared_ptr<T> const& sp)
{
	m_base = sp.m_base;
	return *this;
}


template <class T>
template <class U>
inline
shared_ptr<T>&
shared_ptr<T>::operator=(shared_ptr<U> const& sp)
{
	m_base = sp.m_base;
	return *this;
}


template <class T>
inline
shared_ptr<T>&
shared_ptr<T>::operator=(by_ref<T> ref)
{
	m_base = ref.m_base;
	return *this;
}


template <class T>
inline
void
shared_ptr<T>::reset(T* p)
{
	m_base.reset(p);
}


template <class T>
template <class U>
inline
shared_ptr<T>&
shared_ptr<T>::operator=(auto_ptr<U>& ap)
{
	reset(ap.release());
	return *this;
}


template <class T>
inline
size_t
shared_ptr<T>::use_count() const
{
	return m_base.use_count();
}


template <class T>
inline
bool
shared_ptr<T>::expired() const
{
	return m_base.expired();
}


template <class T>
inline
bool
shared_ptr<T>::unique() const
{
	return m_base.unique();
}


template <class T>
inline
bool
shared_ptr<T>::operator!() const
{
	return m_base.get() == 0;
}


template <class T>
inline
T*
shared_ptr<T>::get()
{
	return m_base.get();
}


template <class T>
inline
T const*
shared_ptr<T>::get() const
{
	return m_base.get();
}


template <class T>
inline
T*
shared_ptr<T>::operator->()
{
	M_REQUIRE(!expired());
	return m_base.get();
}


template <class T>
inline
T const*
shared_ptr<T>::operator->() const
{
	M_REQUIRE(!expired());
	return m_base.get();
}


template <class T>
inline
T&
shared_ptr<T>::operator*()
{
	M_REQUIRE(!expired());
	return *m_base.get();
}


template <class T>
inline
T const&
shared_ptr<T>::operator*() const
{
	M_REQUIRE(!expired());
	return *m_base.get();
}


template <class T>
inline
T*
shared_ptr<T>::release()
{
	return m_base.release();
}


template <class T>
template <class U>
inline
shared_ptr<T>::operator by_ref<U>()
{
	return by_ref<U>(m_base);
}


template <class T>
inline
shared_ptr<T>::operator void const* () const
{
	return m_base.get();
}


template <class T>
inline
void
shared_ptr<T>::swap(shared_ptr& sp)
{
	m_base.swap(sp.m_base);
}


template <class T>
inline
void
swap(shared_ptr<T>& lhs, shared_ptr<T>& rhs)
{
	lhs.swap(rhs);
}

#if HAVE_0X_MOVE_CONSTRCUTOR_AND_ASSIGMENT_OPERATOR

template <class T>
inline
shared_ptr<T>::shared_ptr(shared_ptr&& p)
	:m_base(mstl::move(p.m_base))
{
}


template <class T>
inline
shared_ptr<T>&
shared_ptr<T>::operator=(shared_ptr&& p)
{
	swap(m_base, p.m_base);
	return *this;
}

#endif

} // namespace mstl

// vi:set ts=3 sw=3:
