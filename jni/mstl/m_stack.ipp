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

#include "m_uninitialized.h"
#include "m_utility.h"
#include "m_assert.h"

namespace mstl {

template <typename T> inline void swap(stack<T>& lhs, stack<T>& rhs) { lhs.swap(rhs); }

template <typename T>
inline typename stack<T>::iterator stack<T>::begin()					{ return this->m_start; }

template <typename T>
inline typename stack<T>::const_iterator stack<T>::begin() const	{ return this->m_start; }

template <typename T>
inline typename stack<T>::iterator stack<T>::end()						{ return this->m_finish + 1; }

template <typename T>
inline typename stack<T>::const_iterator stack<T>::end() const		{ return this->m_finish + 1; }


template <typename T> inline stack<T>::stack() : memblock<T>(static_cast<T*>(0) - 1) {}


template <typename T>
inline
stack<T>::stack(size_type n)
	:memblock<T>(n)
{
	this->m_finish = ::mstl::uninitialized_fill_n(this->m_start, n, T()) - 1;
}


template <typename T>
inline
stack<T>::stack(size_type n, const_reference v)
	:memblock<T>(n)
{
	this->m_finish = ::mstl::uninitialized_fill_n(this->m_start, n, v) - 1;
}


template <typename T>
inline
stack<T>::stack(stack const& v)
	:memblock<T>(v.size())
{
	this->m_finish = ::mstl::uninitialized_copy(v.begin(), v.end(), this->m_start) - 1;
}


template <typename T>
inline
stack<T>::~stack() throw()
{
	//M_ASSERT((begin() == 0) == (end() == 0));
	::mstl::bits::destroy(begin(), end());
}


template <typename T>
inline
stack<T>&
stack<T>::operator=(stack const& v)
{
	if (this != &v)
	{
		clear();
		reserve(v.size());
		this->m_finish = ::mstl::uninitialized_copy(v.begin(), v.end(), this->m_start) - 1;
	}

	return *this;
}


template <typename T>
inline
typename stack<T>::size_type
stack<T>::size() const
{
	return size_type(this->m_finish - this->m_start) + 1;
}


template <typename T>
inline
bool
stack<T>::empty() const
{
	return size() == 0;
}


template <typename T>
inline
typename stack<T>::size_type
stack<T>::capacity() const
{
	return this->m_end_of_storage - this->m_start;
}


template <typename T>
inline
typename stack<T>::reference
stack<T>::operator[](size_type n)
{
	//M_REQUIRE(n < size());
	return this->m_start[n];
}


template <typename T>
inline
typename stack<T>::const_reference
stack<T>::operator[](size_type n) const
{
	//M_REQUIRE(n < size());
	return this->m_start[n];
}


template <typename T>
inline
typename stack<T>::reference
stack<T>::bottom()
{
	//M_REQUIRE(!empty());
	return *this->m_start;
}


template <typename T>
inline
typename stack<T>::const_reference
stack<T>::bottom() const
{
	//M_REQUIRE(!empty());
	return *this->m_start;
}


template <typename T>
inline
typename stack<T>::reference
stack<T>::top()
{
	//M_REQUIRE(!empty());
	return *this->m_finish;
}


template <typename T>
inline
typename stack<T>::const_reference
stack<T>::top() const
{
	//M_REQUIRE(!empty());
	return *this->m_finish;
}


template <typename T>
inline
void
stack<T>::push(const_reference value)
{
	reserve(size() + 1);
	::mstl::bits::construct(++this->m_finish, value);
}


template <typename T>
inline
void
stack<T>::dup()
{
	//M_REQUIRE(!empty());

	reserve(size() + 1);
	::mstl::bits::construct(this->m_finish + 1, *this->m_finish);
	++this->m_finish;
}


template <typename T>
inline
void
stack<T>::push()
{
	push(T());
}


template <typename T>
inline
void
stack<T>::pop()
{
	//M_REQUIRE(!empty());
	::mstl::bits::destroy(this->m_finish--);
}


template <typename T>
inline
void
stack<T>::reserve(size_type n)
{
	if (n <= capacity())
		return;

	memblock<T> block(memblock<T>::compute_capacity(capacity(), n, 4));
	block.m_finish = ::mstl::uninitialized_move(this->m_start, this->m_finish + 1, block.m_start) - 1;
	block.swap(*this);
}


template <typename T>
inline
void
stack<T>::clear()
{
	::mstl::bits::destroy(this->m_start, this->m_finish + 1);
	this->m_finish = this->m_start - 1;
}


template <typename T>
inline
void
stack<T>::swap(stack& v)
{
	static_cast<memblock<T>&>(*this).swap(static_cast<memblock<T>&>(v));
}


#if HAVE_0X_MOVE_CONSTRCUTOR_AND_ASSIGMENT_OPERATOR

template <typename T>
inline stack<T>::stack(stack&& v) : memblock<T>(mstl::move(*this)) {}


template <typename T>
inline
stack<T>&
stack<T>::operator=(stack&& v)
{
	static_cast<memblock<T>&>(*this) = mstl::move(*this);
	return *this;
}

#endif

} // namespace mstl

// vi:set ts=3 sw=3:
