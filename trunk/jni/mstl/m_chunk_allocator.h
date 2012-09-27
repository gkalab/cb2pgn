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

#ifndef _mstl_chunk_allocator_included
#define _mstl_chunk_allocator_included

#include "m_stack.h"
#include "m_types.h"

namespace mstl {

template <typename T, bool Zero = false>
class chunk_allocator
{
public:

	static unsigned const NotFound = unsigned(-1);

	typedef T value_type;

	chunk_allocator(size_t chunk_size = 0);
	~chunk_allocator();

#if HAVE_OX_EXPLICITLY_DEFAULTED_AND_DELETED_SPECIAL_MEMBER_FUNCTIONS
	chunk_allocator(chunk_allocator const&) = delete;
	chunk_allocator& operator=(chunk_allocator const&) = delete;
#endif

	bool canRelease() const;
	bool canShrink(size_t size) const;
	bool empty() const;

	unsigned lookup(T const* p) const;

	size_t chunk_size() const;
	size_t elems_per_chunk() const;

	void clear();

	T* alloc();
	T* alloc(size_t length);

	void shrink(size_t allocatedLength, size_t newLength);
	void release();

private:

	struct chunk
	{
		T* base;
		T* curr;
	};

	typedef stack<chunk> chunk_list;

#if !HAVE_OX_EXPLICITLY_DEFAULTED_AND_DELETED_SPECIAL_MEMBER_FUNCTIONS
	chunk_allocator(chunk_allocator const&);
	chunk_allocator& operator=(chunk_allocator const&);
#endif

	chunk* new_chunk();

	size_t		m_chunk_size;
	size_t		m_num_elems;
	chunk_list	m_chunk_list;
};

} // namespace mstl

#include "m_chunk_allocator.ipp"

#endif // _mstl_chunk_allocator_included

// vi:set ts=3 sw=3:
