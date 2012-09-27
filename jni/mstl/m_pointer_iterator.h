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

#ifndef _m_pointer_iterator_included
#define _m_pointer_iterator_included

#include "m_types.h"

namespace mstl {

template <typename> class pointer_const_iterator;

template <typename T>
class pointer_iterator
{
public:

	typedef ptrdiff_t difference_type;

	pointer_iterator(T** elems = 0);

	bool operator==(pointer_iterator const& it) const;
	bool operator!=(pointer_iterator const& it) const;

	pointer_iterator& operator++();
	pointer_iterator  const operator++(int);
	pointer_iterator& operator--();
	pointer_iterator  const operator--(int);
	pointer_iterator& operator+=(difference_type n);
	pointer_iterator  operator+ (difference_type n) const;
	pointer_iterator& operator-=(difference_type n);
	pointer_iterator  operator- (difference_type n) const;

	difference_type operator-(pointer_iterator const& i) const;

	T& operator[](size_t n) const;

	T& operator*() const;
	T* operator->() const;

	operator T* () const;

	void swap(pointer_iterator i);
	T**& ref();	// Use with care!

private:

	T** m_elems;

	friend class pointer_const_iterator<T>;
};


template <typename T>
class pointer_const_iterator
{
public:

	typedef ptrdiff_t difference_type;

	pointer_const_iterator(T*const* elems = 0);
	pointer_const_iterator(pointer_iterator<T> const& it);

	bool operator==(pointer_const_iterator const& it) const;
	bool operator!=(pointer_const_iterator const& it) const;

	pointer_const_iterator& operator++();
	pointer_const_iterator  operator++(int);
	pointer_const_iterator& operator--();
	pointer_const_iterator  operator--(int);
	pointer_const_iterator& operator+=(difference_type n);
	pointer_const_iterator  operator+ (difference_type n) const;
	pointer_const_iterator& operator-=(difference_type n);
	pointer_const_iterator  operator- (difference_type n) const;

	difference_type operator-(pointer_const_iterator const& i) const;

	T const& operator[](size_t n) const;

	T const& operator*() const;
	T const* operator->() const;

	operator T const* () const;

	T* const* ref() const;	// Use with care!

private:

	T* const* m_elems;
};

} // namespace mstl

#include "m_pointer_iterator.ipp"

#endif // _m_list_iterator_included

// vi:set ts=3 sw=3:
