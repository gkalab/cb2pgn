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

#include <stdlib.h>

namespace mstl {

template <typename T>
inline memblock<T>::memblock() : m_start(0), m_finish(0), m_end_of_storage(0) {}

template <typename T>
inline memblock<T>::memblock(T* finish) : m_start(0), m_finish(finish), m_end_of_storage(0) {}


template <typename T>
inline
memblock<T>::memblock(size_t n)
{
	m_start = static_cast<T*>(operator new(n*sizeof(T)));
	m_finish = m_start;
	m_end_of_storage = m_start + n;
}


template <typename T>
inline
memblock<T>::~memblock() throw()
{
	operator delete(m_start);
}


template <typename T>
inline
void
memblock<T>::swap(memblock& block)
{
	mstl::swap(m_start, block.m_start);
	mstl::swap(m_finish, block.m_finish);
	mstl::swap(m_end_of_storage, block.m_end_of_storage);
}


template <typename T>
inline
size_t
memblock<T>::compute_capacity(size_t old_capacity, size_t wanted_size, size_t min_capacity)
{
	static size_t const max_size = ((size_t(-1))/sizeof(T))/4;

	//if (wanted_size > max_size)
	//	M_THROW(exception("maximal size exceeded"));

	static size_t const page_size				= 4096;	// must be 2^i * sub_page_size
	static size_t const sub_page_size		= 128;	// should be >> malloc_header_size
	static size_t const malloc_header_size	= 4*sizeof(void*);
//	static size_t const page_capacity		= (page_size - malloc_header_size)/sizeof(T);

	if (old_capacity >= mstl::mul2(wanted_size))
		return mstl::max(old_capacity, min_capacity);

	if (wanted_size > old_capacity && wanted_size < mstl::mul2(old_capacity))
		wanted_size = mstl::mul2(old_capacity);

	size_t size = wanted_size*sizeof(T);
	size_t adj_size = size + malloc_header_size;

	if (adj_size > page_size)
		wanted_size += (page_size - adj_size%page_size)/sizeof(T);
	else if (size > sub_page_size)
		wanted_size += (sub_page_size - adj_size%sub_page_size)/sizeof(T);

	return mstl::max(min_capacity, wanted_size);
}

#if HAVE_0X_MOVE_CONSTRCUTOR_AND_ASSIGMENT_OPERATOR

template <typename T>
inline
memblock<T>::memblock(memblock&& mb)
	:m_start(mb.m_start)
	,m_finish(mb.m_finish)
	,m_end_of_storage(mb.m_end_of_storage)
{
	mb.m_start = 0;
}


template <typename T>
inline
memblock<T>&
memblock<T>::operator=(memblock&& mb)
{
	mstl::swap(m_start, mb.m_start);
	m_finish = mb.m_finish;
	m_end_of_storage = mb.m_end_of_storage;

	return *this;
}

#endif

} // namespace mstl

// vi:set ts=3 sw=3:
