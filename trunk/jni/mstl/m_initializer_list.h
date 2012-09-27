// ======================================================================
// Author : $Author$
// Version: $Revision$
// Date   : $Date$
// Url    : $URL$
// ======================================================================

// ======================================================================
// Copyright: (C)2011-2012 Gregor Cramer
// ======================================================================

// ======================================================================
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
// ======================================================================

#ifndef _mstl_initializer_list_included
#define _mstl_initializer_list_included

#if defined(__GNUC_MINOR__) && ((__GNUC__ << 16) + __GNUC_MINOR__ >= (4 << 16) + 4)

namespace mstl {

template<class E>
class initializer_list
{
public:

	typedef E				value_type;
	typedef const E&		reference;
	typedef const E&		const_reference;
	typedef bits::size_t	size_type;
	typedef const E*		iterator;
	typedef const E*		const_iterator;

	constexpr initializer_list();

	constexpr size_type size();

	constexpr const_iterator begin();
	constexpr const_iterator end();

private:

	iterator		m_arr;
	size_type	m_size;

	constexpr initializer_list(const_iterator arr, size_type size);
};

template<class T> constexpr const T* begin(initializer_list<T> list);
template<class T> constexpr const T* end(initializer_list<T> list);

} // namespace mstl

# include "m_initializer_list.ipp"

#else

# error "requires g++ version >= 4.4"

#endif

// vi:set ts=3 sw=3:
