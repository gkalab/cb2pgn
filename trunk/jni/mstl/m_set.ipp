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

#include "m_algorithm.h"
#include "m_utility.h"
#include "m_assert.h"

namespace mstl {

template <typename T> inline void swap(set<T>& lhs, set<T>& rhs) { lhs.swap(rhs); }

template <typename T> inline set<T>::set() {}
template <typename T> inline set<T>::set(size_type n) : m_v(n) {}
template <typename T> inline set<T>::set(set const& v) : m_v(v.m_v) {}

template <typename T> inline bool set<T>::empty() const			{ return m_v.empty(); }
template <typename T> inline void set<T>::clear()					{ m_v.clear(); }
template <typename T> inline void set<T>::reserve(size_type n)	{ m_v.reserve(n); }
template <typename T> inline void set<T>::swap(set& v)			{ m_v.swap(v.m_v); }

template <typename T> inline typename set<T>::iterator set<T>::end()					{ return m_v.end(); }
template <typename T> inline typename set<T>::const_iterator set<T>::end() const	{ return m_v.end(); }

template <typename T> inline bool set<T>::contains(const_reference v) const { return find(v) != end(); }


template <typename T>
inline
typename set<T>::container_type const&
set<T>::container() const
{
	return m_v;
}


template <typename V>
inline
set<V>&
set<V>::operator=(set const& v)
{
	m_v = v.m_v;
	return *this;
}


template <typename V>
inline
bool
set<V>::operator==(set const& v) const
{
	return m_v == v.m_v;
}


template <typename V>
inline
bool
set<V>::operator!=(set const& v) const
{
	return m_v != v.m_v;
}


template <typename T>
inline
typename set<T>::iterator
set<T>::begin()
{
	return m_v.begin();
}


template <typename T>
inline
typename set<T>::const_iterator
set<T>::begin() const
{
	return m_v.begin();
}


template <typename T>
inline
set<T>::set(const_iterator first, const_iterator last)
{
	insert(first, last);
}


template <typename T>
inline
typename set<T>::size_type
set<T>::size() const
{
	return m_v.size();
}


template <typename T>
inline
typename set<T>::const_iterator
set<T>::find(const_reference v) const
{
	return mstl::binary_search(begin(), end(), v);
}


template <typename T>
inline
typename set<T>::iterator
set<T>::find(const_reference v)
{
	return const_cast<iterator>(const_cast<set const&>(*this).find(v));
}


template <typename T>
inline
typename set<T>::iterator
set<T>::insert(const_reference v)
{
	iterator i = mstl::lower_bound(begin(), end(), v);

	if (i == end() || v < *i)
		i = m_v.insert(i, v);
	else
		*i = v;

	return i;
}


template <typename T>
inline
bool
set<T>::insert_unique(const_reference v)
{
	iterator i = mstl::lower_bound(begin(), end(), v);

	if (i != end() && !(v < *i))
		return false;

	m_v.insert(i, v);
	return true;
}


template <typename T>
inline
void
set<T>::insert(const_iterator first, const_iterator last)
{
	M_REQUIRE(first <= last);

	reserve(size() + distance(first, last));

	for (; first < last; ++first)
		push_back(*first);
}


template <typename T>
inline void
set<T>::push_back(const_reference v)
{
	insert(v);
}


template <typename T>
inline
typename set<T>::iterator
set<T>::erase(iterator i)
{
	return m_v.erase(i);
}


template <typename T>
inline
typename set<T>::iterator
set<T>::erase(iterator first, iterator last)
{
	return m_v.erase(first, last);
}


template <typename T>
inline
void
set<T>::erase(const_reference v)
{
	iterator i = find(v);

	if (i != end())
		erase(i);
}


template <typename T>
inline
void
set<T>::assign(const_iterator first, const_iterator last)
{
	clear();
	insert(first, last);
}


#if HAVE_0X_MOVE_CONSTRCUTOR_AND_ASSIGMENT_OPERATOR

template <typename T>
inline set<T>::set(set&& v) : m_v(mstl::move(v.m_v)) {}


template <typename T>
inline
set<T>&
set<T>::operator=(set&& v)
{
	m_v = mstl::move(v.m_v);
	return *this;
}

#endif

} // namespace mstl

// vi:set ts=3 sw=3:
