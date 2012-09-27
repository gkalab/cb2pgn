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
#include "m_bit_functions.h"
#include "m_construct.h"
#include "m_assert.h"

#include <string.h>
#include <stdlib.h>

namespace mstl {

template <typename T, bool Zero>
chunk_allocator<T,Zero>::chunk_allocator(size_t chunk_size)
{
	static size_t const page_size				= 4096;					// must be power of 2
	static size_t const malloc_header_size	= 4*sizeof(void*);	// should be always sufficient

	m_chunk_size = mstl::max(chunk_size, page_size);

	if (is_not_pow_2(chunk_size))
	{
		// we round up to next power of 2.
		chunk_size = 1 << (bf::msb_index(chunk_size) + 1);
	}

	m_chunk_size -= malloc_header_size;	// take overhead of memory allocation into account
	m_num_elems = m_chunk_size/sizeof(T);
	m_chunk_size = m_num_elems*sizeof(T);

	new_chunk();
}


template <typename T, bool Zero>
chunk_allocator<T,Zero>::~chunk_allocator()
{
	static_assert(is_pod<T>::value || !Zero, "value type not POD");

	for (typename chunk_list::iterator i = m_chunk_list.begin(); i != m_chunk_list.end(); ++i)
	{
		bits::destroy(i->base, i->curr);
		::free(i->base);
	}
}


template <typename T, bool Zero>
bool
chunk_allocator<T,Zero>::canRelease() const
{
	// M_ASSERT(!m_chunk_list.empty());
	return m_chunk_list.top().curr > m_chunk_list.top().base;
}


template <typename T, bool Zero>
bool
chunk_allocator<T,Zero>::canShrink(size_t size) const
{
	// M_ASSERT(!m_chunk_list.empty());
	return m_chunk_list.top().curr >= m_chunk_list.top().base + size;
}



template <typename T, bool Zero>
bool
chunk_allocator<T,Zero>::empty() const
{
	return m_chunk_list.size() == 1 && m_chunk_list.top().base == m_chunk_list.top().curr;
}


template <typename T, bool Zero>
inline
size_t
chunk_allocator<T,Zero>::chunk_size() const
{
	return m_chunk_size;
}


template <typename T, bool Zero>
inline
size_t
chunk_allocator<T,Zero>::elems_per_chunk() const
{
	return m_num_elems;
}


template <typename T, bool Zero>
inline
void
chunk_allocator<T,Zero>::clear()
{
	for (typename chunk_list::iterator i = m_chunk_list.begin(); i != m_chunk_list.end(); ++i)
	{
		bits::destroy(i->base, i->curr);
		::free(i->base);
	}

	m_chunk_list.clear();
	new_chunk();
}


template <typename T, bool Zero>
typename chunk_allocator<T,Zero>::chunk*
chunk_allocator<T,Zero>::new_chunk()
{
	m_chunk_list.push();
	chunk& c = m_chunk_list.top();
	c.curr = c.base = static_cast<T*>(::malloc(m_chunk_size));

	if (Zero)
		::memset(c.base, 0, m_chunk_size);

	return &c;
}


template <typename T, bool Zero>
inline
T*
chunk_allocator<T,Zero>::alloc()
{
	// M_ASSERT(!m_chunk_list.empty());

	chunk* c = &m_chunk_list.top();

	if (c->base + m_num_elems == c->curr)
		c = new_chunk();

	T* obj = c->curr++;
	bits::construct(obj);
	return obj;
}


template <typename T, bool Zero>
T*
chunk_allocator<T,Zero>::alloc(size_t length)
{
	// otherwise uninitialized_fill_n(p, length, T()) is required
	static_assert(is_pod<T>::value, "value type not POD");
	// M_REQUIRE(length <= chunk_size());
	// M_ASSERT(!m_chunk_list.empty());

	chunk*	c = &m_chunk_list.top();
	T*			p = c->curr;

	if ((c->curr += length) >= c->base + m_num_elems)
	{
		c = new_chunk();
		p = c->curr;
		c->curr += length;
	}

	return p;
}


template <typename T, bool Zero>
void
chunk_allocator<T,Zero>::release()
{
	// M_REQUIRE(canRelease());

	chunk* c = &m_chunk_list.top();
	bits::destroy(--c->curr);
}


template <typename T, bool Zero>
void
chunk_allocator<T,Zero>::shrink(size_t allocatedLength, size_t newLength)
{
	static_assert(is_pod<T>::value, "value type not POD");
	// M_REQUIRE(newLength <= allocatedLength);
	// M_ASSERT(!m_chunk_list.empty());

	if (canShrink(allocatedLength - newLength))
	{
		chunk* c = &m_chunk_list.top();
		c->curr -= allocatedLength - newLength;
	}
}


template <typename T, bool Zero>
unsigned
chunk_allocator<T,Zero>::lookup(T const* p) const
{
	for (unsigned i = 0; i < m_chunk_list.size(); ++i)
	{
		chunk const& chunk = m_chunk_list[i];

		if (chunk.base <= p && p < chunk.curr)
			return i*m_num_elems + (p - chunk.base);
	}

	return NotFound;
}

} // namespace mstl

// vi:set ts=3 sw=3:
