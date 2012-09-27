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
#include "m_algobase.h"
#include "m_utility.h"
#include "m_type_traits.h"
#include "m_assert.h"

namespace mstl {


template <typename T> inline vector<T>::reverse_iterator::reverse_iterator(iterator i) :m_i(i) {}


template <typename T>
inline
typename vector<T>::iterator
vector<T>::reverse_iterator::base() const
{
	return m_i - 1;
}


template <typename T>
inline
typename vector<T>::reverse_iterator&
vector<T>::reverse_iterator::operator=(iterator i)
{
	m_i = i + 1;
	return *this;
}


template <typename T>
inline
bool
vector<T>::reverse_iterator::operator==(reverse_iterator const& iter) const
{
	return m_i == iter.m_i;
}


template <typename T>
inline
bool
vector<T>::reverse_iterator::operator!=(reverse_iterator const& iter) const
{
	return m_i != iter.m_i;
}


template <typename T>
inline
bool
vector<T>::reverse_iterator::operator<=(reverse_iterator const& iter) const
{
	return iter.m_i <= m_i;
}


template <typename T>
inline
bool
vector<T>::reverse_iterator::operator<(reverse_iterator const& iter) const
{
	return iter.m_i < m_i;
}


template <typename T>
inline
typename vector<T>::reference
vector<T>::reverse_iterator::operator*() const
{
	iterator prev = m_i;
	return *--prev;
}


template <typename T>
inline
typename vector<T>::pointer
vector<T>::reverse_iterator::operator->() const
{
	return &(operator*());
}


template <typename T>
inline
typename vector<T>::reverse_iterator&
vector<T>::reverse_iterator::operator++()
{
	--m_i;
	return *this;
}


template <typename T>
inline
typename vector<T>::reverse_iterator&
vector<T>::reverse_iterator::operator--()
{
	++m_i;
	return *this;
}


template <typename T>
inline
typename vector<T>::reverse_iterator
vector<T>::reverse_iterator::operator++(int)
{
	reverse_iterator prev = *this;
	--m_i;
	return prev;
}


template <typename T>
inline
typename vector<T>::reverse_iterator
vector<T>::reverse_iterator::operator--(int)
{
	reverse_iterator prev = *this;
	++m_i;
	return prev;
}


template <typename T>
inline
typename vector<T>::reverse_iterator&
vector<T>::reverse_iterator::operator+=(size_t n)
{
	m_i -= n;
	return *this;
}


template <typename T>
inline
typename vector<T>::reverse_iterator&
vector<T>::reverse_iterator::operator-=(size_t n)
{
	m_i += n;
	return *this;
}


template <typename T>
inline
typename vector<T>::reverse_iterator
vector<T>::reverse_iterator::operator+(size_t n) const
{
	return reverse_iterator(m_i - n);
}


template <typename T>
inline
typename vector<T>::reverse_iterator
vector<T>::reverse_iterator::operator-(size_t n) const
{
	return reverse_iterator(m_i + n);
}


template <typename T>
inline
typename vector<T>::reference
vector<T>::reverse_iterator::operator[](difference_type n) const
{
	return *(*this + n);
}


template <typename T>
inline
typename vector<T>::difference_type
vector<T>::reverse_iterator::operator-(reverse_iterator const& i) const
{
	return i.m_i - m_i;
}


template <typename T>
inline
vector<T>::const_reverse_iterator::const_reverse_iterator(iterator i)
	:m_i(i)
{
}


template <typename T>
inline
vector<T>::const_reverse_iterator::const_reverse_iterator(const_iterator i)
	:m_i(i)
{
}


template <typename T>
inline
typename vector<T>::const_iterator
vector<T>::const_reverse_iterator::base() const
{
	return m_i - 1;
}


template <typename T>
inline
typename vector<T>::const_reverse_iterator&
vector<T>::const_reverse_iterator::operator=(iterator i)
{
	m_i = i + 1;
	return *this;
}


template <typename T>
inline
typename vector<T>::const_reverse_iterator&
vector<T>::const_reverse_iterator::operator=(const_iterator i)
{
	m_i = i + 1;
	return *this;
}


template <typename T>
inline
bool
vector<T>::const_reverse_iterator::operator==(const_reverse_iterator const& iter) const
{
	return m_i == iter.m_i;
}


template <typename T>
inline
bool
vector<T>::const_reverse_iterator::operator!=(const_reverse_iterator const& iter) const
{
	return m_i != iter.m_i;
}


template <typename T>
inline
bool
vector<T>::const_reverse_iterator::operator<=(const_reverse_iterator const& iter) const
{
	return iter.m_i <= m_i;
}


template <typename T>
inline
bool
vector<T>::const_reverse_iterator::operator<(const_reverse_iterator const& iter) const
{
	return iter.m_i < m_i;
}


template <typename T>
inline
typename vector<T>::const_reference
vector<T>::const_reverse_iterator::operator*() const
{
	const_iterator prev  = m_i;
	return *--prev;
}


template <typename T>
inline
typename vector<T>::const_pointer
vector<T>::const_reverse_iterator::operator->() const
{
	return &(operator*());
}


template <typename T>
inline
typename vector<T>::const_reverse_iterator&
vector<T>::const_reverse_iterator::operator++()
{
	--m_i;
	return *this;
}


template <typename T>
inline
typename vector<T>::const_reverse_iterator&
vector<T>::const_reverse_iterator::operator--()
{
	++m_i;
	return *this;
}


template <typename T>
inline
typename vector<T>::const_reverse_iterator
vector<T>::const_reverse_iterator::operator++(int)
{
	const_reverse_iterator prev = *this;
	--m_i;
	return prev;
}


template <typename T>
inline
typename vector<T>::const_reverse_iterator
vector<T>::const_reverse_iterator::operator--(int)
{
	const_reverse_iterator prev = *this;
	++m_i;
	return prev;
}


template <typename T>
inline
typename vector<T>::const_reverse_iterator&
vector<T>::const_reverse_iterator::operator+=(size_t n)
{
	m_i -= n;
	return *this;
}


template <typename T>
inline
typename vector<T>::const_reverse_iterator&
vector<T>::const_reverse_iterator::operator-=(size_t n)
{
	m_i += n;
	return *this;
}


template <typename T>
inline
typename vector<T>::const_reverse_iterator
vector<T>::const_reverse_iterator::operator+(size_t n) const
{
	return const_reverse_iterator(m_i - n);
}


template <typename T>
inline
typename vector<T>::const_reverse_iterator
vector<T>::const_reverse_iterator::operator-(size_t n) const
{
	return const_reverse_iterator(m_i + n);
}


template <typename T>
inline
typename vector<T>::reference
vector<T>::const_reverse_iterator::operator[](difference_type n) const
{
	return *(*this + n);
}


template <typename T>
inline
typename vector<T>::difference_type
vector<T>::const_reverse_iterator::operator-(const_reverse_iterator const& i) const
{
	return i.m_i - m_i;
}


template <typename T> inline void swap(vector<T>& lhs, vector<T>& rhs) { lhs.swap(rhs); }

template <typename T> inline bool vector<T>::operator!=(vector const& v) const { return !(*this == v); }

template <typename T> inline bool vector<T>::empty() const { return this->m_start == this->m_finish; }

template <typename T>
inline typename vector<T>::iterator vector<T>::begin()					{ return this->m_start; }

template <typename T>
inline typename vector<T>::const_iterator vector<T>::begin() const	{ return this->m_start; }

template <typename T>
inline typename vector<T>::iterator vector<T>::end()						{ return this->m_finish; }

template <typename T>
inline typename vector<T>::const_iterator vector<T>::end() const		{ return this->m_finish; }


template <typename T>
inline
typename vector<T>::reverse_iterator
vector<T>::rbegin()
{
	return reverse_iterator(end());
}


template <typename T>
inline
typename vector<T>::reverse_iterator
vector<T>::rend()
{
	return reverse_iterator(begin());
}


template <typename T>
inline
typename vector<T>::const_reverse_iterator
vector<T>::rbegin() const
{
	return const_reverse_iterator(end());
}


template <typename T>
inline
typename vector<T>::const_reverse_iterator
vector<T>::rend() const
{
	return const_reverse_iterator(begin());
}


template <typename T>
inline
typename vector<T>::size_type
vector<T>::size() const
{
	return this->m_finish - this->m_start;
}


template <typename T>
inline
typename vector<T>::size_type
vector<T>::capacity() const
{
	return this->m_end_of_storage - this->m_start;
}


template <typename T>
inline
typename vector<T>::reference
vector<T>::operator[](size_type n)
{
	//M_REQUIRE(n < size());
	return this->m_start[n];
}


template <typename T>
inline
typename vector<T>::const_reference
vector<T>::operator[](size_type n) const
{
	//M_REQUIRE(n < size());
	return this->m_start[n];
}


template <typename T>
inline
typename vector<T>::reference
vector<T>::at(size_type n)
{
	//M_REQUIRE(n < size());
	return this->m_start[n];
}


template <typename T>
inline
typename vector<T>::const_reference
vector<T>::at(size_type n) const
{
	//M_REQUIRE(n < size());
	return this->m_start[n];
}


template <typename T>
inline
typename vector<T>::reference
vector<T>::front()
{
	//M_REQUIRE(!empty());
	return *this->m_start;
}


template <typename T>
inline
typename vector<T>::const_reference
vector<T>::front() const
{
	//M_REQUIRE(!empty());
	return *this->m_start;
}


template <typename T>
inline
typename vector<T>::reference
vector<T>::back()
{
	//M_REQUIRE(!empty());
	return *(this->m_finish - 1);
}


template <typename T>
inline
typename vector<T>::const_reference
vector<T>::back() const
{
	//M_REQUIRE(!empty());
	return *(this->m_finish - 1);
}


template <typename T>
inline
vector<T>::vector()
{
}


template <typename T>
inline
vector<T>::vector(size_type n)
	:memblock<T>(n)
{
	this->m_finish = mstl::uninitialized_fill_n(this->m_finish, n, T());
}


template <typename T>
inline
vector<T>::vector(size_type n, const_reference v)
	:memblock<T>(n)
{
	this->m_finish = mstl::uninitialized_fill_n(this->m_finish, n, v);
}


template <typename T>
inline
vector<T>::vector(vector const& v)
	:memblock<T>(v.size())
{
	this->m_finish = mstl::uninitialized_copy(v.begin(), v.end(), this->m_start);
}


template <typename T>
inline
vector<T>::vector(vector const& v, size_type n)
	:memblock<T>(n)
{
	//M_REQUIRE(n <= v.size());
	this->m_finish = mstl::uninitialized_copy(v.begin(), v.begin() + n, this->m_start);
}


template <typename T>
inline
vector<T>::~vector() throw()
{
	//M_ASSERT((begin() == 0) == (end() == 0));
	mstl::bits::destroy(begin(), end());
}


template <typename T>
inline
vector<T>&
vector<T>::operator=(vector const& v)
{
	if (this != &v)
	{
		clear();
		reserve(v.size());
		this->m_finish = mstl::uninitialized_copy(v.begin(), v.end(), this->m_start);
	}

	return *this;
}


template <typename T>
inline
bool
vector<T>::equal(vector_type const& v, size_type n) const
{
	//M_REQUIRE(n <= v.size());
	//M_REQUIRE(n <= size());

	if (mstl::is_pod<T>::value)
		return ::memcmp(this->m_start, v.m_start, n*sizeof(T)) == 0;

	for (size_type i = 0; i < n; ++i)
	{
		if (this->m_start[i] != v.m_start[i])
			return false;
	}

	return true;
}


template <typename T>
inline
bool
vector<T>::equal(vector_type const& v) const
{
	return size() == v.size() && equal(v, size());
}


template <typename T>
inline
void
vector<T>::push_back(const_reference v)
{
	reserve(size() + 1);
	mstl::bits::construct(this->m_finish++, v);
}


template <typename T>
inline
void
vector<T>::push_back()
{
	push_back(T());
}


template <typename T>
inline
void
vector<T>::push_front(const_reference v)
{
	insert(begin(), v);
}


template <typename T>
inline
void
vector<T>::push_front()
{
	push_front(T());
}


template <typename T>
inline
void
vector<T>::pop_back()
{
	//M_REQUIRE(!empty());
	mstl::bits::destroy(--this->m_finish);
}


template <typename T>
inline
void
vector<T>::pop_front()
{
	//M_REQUIRE(!empty());
	erase(begin());
}


template <typename T>
inline
void
vector<T>::reserve(size_type n)
{
	if (n <= capacity())
		return;

	memblock<T> block(memblock<T>::compute_capacity(capacity(), n, 4));
	block.m_finish = mstl::uninitialized_move(this->m_start, this->m_finish, block.m_start);
	block.swap(*this);
}


template <typename T>
inline
void
vector<T>::reserve_exact(size_type n)
{
	if (n <= capacity())
		return;

	memblock<T> block(n);
	block.m_finish = mstl::uninitialized_move(this->m_start, this->m_finish, block.m_start);
	block.swap(*this);
}


template <typename T>
inline
void
vector<T>::resize(size_type n, const_reference v)
{
	int k = int(size()) - int(n);

	if (k < 0)
	{
		reserve(n);
		this->m_finish = mstl::uninitialized_fill_n(this->m_finish, -k, v);
	}
	else if (k > 0)
	{
		mstl::bits::destroy(this->m_finish - k, this->m_finish);
		this->m_finish -= k;
	}
}


template <typename T>
inline
void
vector<T>::resize(size_type n)
{
	resize(n, T());
}


template <typename T>
void
vector<T>::release()
{
	mstl::bits::destroy(begin(), end());
	delete this->m_start;
	this->m_start = this->m_finish = this->m_end_of_storage = 0;
}


template <typename T>
inline
void
vector<T>::clear()
{
	mstl::bits::destroy(this->m_start, this->m_finish);
	this->m_finish = this->m_start;
}


template <typename T>
inline
void
vector<T>::swap(vector& v)
{
	memblock<T>::swap(v);
}


template <typename T>
typename vector<T>::iterator
vector<T>::insert(iterator position, const_reference value)
{
	//M_REQUIRE(position <= end());

	size_type n = position - begin();

	if (position == end() && this->m_finish != this->m_end_of_storage)
		mstl::bits::construct(this->m_finish++, value);
	else
		insert_aux(position, value);

	return begin() + n;
}


template <typename T>
inline
void
vector<T>::insert(iterator position, size_type n, const_reference value)
{
	//M_REQUIRE(position <= end());
	fill_insert(position, n, value);
}


template <typename T>
inline
typename vector<T>::iterator
vector<T>::erase(iterator position)
{
	//M_REQUIRE(position < end());

	if (is_movable<T>::value)
	{
		mstl::bits::destroy(position);
		::memmove(position, position + 1, mstl::distance(position + 1, end())*sizeof(T));
		--this->m_finish;
	}
	else
	{
		mstl::copy(position + 1, end(), position);
		mstl::bits::destroy(--this->m_finish);
	}

	return position;
}


template <typename T>
inline
typename vector<T>::iterator
vector<T>::erase(reverse_iterator position)
{
	//M_REQUIRE(position < rend());
	return erase(position.base());
}


template <typename T>
inline
typename vector<T>::iterator
vector<T>::erase(iterator first, iterator last)
{
	//M_REQUIRE(last <= end());
	//M_REQUIRE(first <= last);

	iterator i;

	if (is_movable<T>::value)
	{
		size_t distance = mstl::distance(last, end());
		::memmove(first, last, distance*sizeof(T));
		i = first + distance;
	}
	else
	{
		i = mstl::copy(last, end(), first);
	}

	mstl::bits::destroy(i, end());
	this->m_finish = this->m_finish - (last - first);

	return first;
}


template <typename T>
inline
typename vector<T>::iterator
vector<T>::erase(reverse_iterator first, reverse_iterator last)
{
	//M_REQUIRE(last <= rend());
	//M_REQUIRE(first <= last);

	return erase(last.base(), first.base());
}


template <typename T>
inline
bool
vector<T>::operator==(vector const& v) const
{
	size_t n = size();

	if (n != v.size())
		return false;

	for (size_t i = 0; i < n; ++i)
	{
		if (!(this->m_start[i] == v.m_start[i]))
			return false;
	}

	return true;
}


template <typename T>
template <typename Iterator>
inline
void
vector<T>::insert(iterator position, Iterator first, Iterator last)
{
	//M_REQUIRE(position <= end());

	size_type n = mstl::distance(first, last);

	if (n == 0)
		return;

	if (size_type(this->m_end_of_storage - this->m_finish) >= n)
	{
		mstl::uninitialized_move(position, position + n, this->m_finish);
		bits::destroy(position, position + n);
		this->m_finish = mstl::uninitialized_copy(first, last, position);
	}
	else
	{
		memblock<T> block(memblock<T>::compute_capacity(capacity(), size() + n, 4));
		block.swap(*this);

		this->m_finish = mstl::uninitialized_move(block.m_start, position, this->m_start);
		this->m_finish = mstl::uninitialized_copy(first, last, this->m_finish);
		this->m_finish = mstl::uninitialized_move(position, block.m_finish, this->m_finish);
	}
}


template <typename T>
inline
void
vector<T>::fill_insert(iterator position, size_type n, const_reference value)
{
	if (n == 0)
		return;

	if (size_type(this->m_end_of_storage - this->m_finish) >= n)
	{
		size_type elems_after = this->m_finish - position;

		if (elems_after > n)
		{
			if (is_movable<T>::value)
			{
				::memmove(position + n, position, mstl::distance(position, end())*sizeof(T));
				this->m_finish += n;
				mstl::fill_n(position, n, value);
			}
			else
			{
				iterator old_finish = this->m_finish;
				mstl::uninitialized_copy(this->m_finish - n, this->m_finish, this->m_finish);
				this->m_finish += n;
				mstl::copy_backward(position, old_finish - n, old_finish);
				mstl::fill(position, position + n, value);
			}
		}
		else
		{
			iterator old_finish = this->m_finish;
			mstl::uninitialized_fill_n(this->m_finish, n - elems_after, value);
			this->m_finish += n - elems_after;
			mstl::uninitialized_copy(position, old_finish, this->m_finish);
			this->m_finish += elems_after;
			mstl::fill(position, old_finish, value);
		}
	}
	else
	{
		memblock<T> block(memblock<T>::compute_capacity(capacity(), size() + n, 4));
		block.swap(*this);

		this->m_finish = mstl::uninitialized_move(block.m_start, position, this->m_start);
		this->m_finish = mstl::uninitialized_fill_n(this->m_finish, n, value);
		this->m_finish = mstl::uninitialized_move(position, block.m_finish, this->m_finish);
	}
}


template <typename T>
void
vector<T>::insert_aux(iterator position, const_reference value)
{
	if (this->m_finish != this->m_end_of_storage)
	{
		if (is_movable<T>::value)
		{
			::memmove(position + 1, position, mstl::distance(position, end())*sizeof(T));
			mstl::bits::construct(position, value);
		}
		else
		{
			mstl::bits::construct(this->m_finish, *(this->m_finish - 1));
			if (position < this->m_finish)
				mstl::copy_backward(position, this->m_finish - 1, this->m_finish);
			*position = value;
		}

		++this->m_finish;
	}
	else
	{
		memblock<T> block(memblock<T>::compute_capacity(capacity(), size() + 1, 4));
		block.swap(*this);

		this->m_finish = mstl::uninitialized_move(block.m_start, position, this->m_start);
		mstl::bits::construct(this->m_finish++, value);
		this->m_finish = mstl::uninitialized_move(position, block.m_finish, this->m_finish);
	}
}


template <typename T>
template <typename Iter>
inline
vector<T>::vector(Iter* first, Iter* last)
{
	//M_REQUIRE(first <= last);
	//M_REQUIRE(last || !first);

	size_type n = distance(first, last);

	reserve(n);

	for ( ; first < last; ++first)
		mstl::bits::construct(this->m_finish++, T(*first));
}


template <typename T>
void
vector<T>::fill(value_type const& value)
{
	if (mstl::is_pod<value_type>::value)
		mstl::uninitialized_fill_n(this->m_start, size(), value);
	else
		mstl::fill_n(this->m_start, size(), value);
}


#if HAVE_0X_MOVE_CONSTRCUTOR_AND_ASSIGMENT_OPERATOR

template <typename T>
inline vector<T>::vector(vector&& v) : memblock<T>(mstl::move(*this)) {}


template <typename T>
inline
vector<T>&
vector<T>::operator=(vector&& v)
{
	static_cast<memblock<T>&>(*this) = mstl::move(*this);
	return *this;
}

#endif

} // namespace mstl

// vi:set ts=3 sw=3:
