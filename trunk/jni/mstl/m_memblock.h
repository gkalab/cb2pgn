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

#ifndef _m_memblock_included
#define _m_memblock_included

#include "m_types.h"

namespace mstl {

template <typename T>
class memblock
{
public:

	memblock();
	explicit memblock(size_t n);
	explicit memblock(T* finish);
	~memblock() throw();	// NOTE: we don't want a virtual destructor

#if HAVE_OX_EXPLICITLY_DEFAULTED_AND_DELETED_SPECIAL_MEMBER_FUNCTIONS
	memblock(memblock const&) = delete;
	memblock& operator=(memblock const&) = delete;
#endif
#if HAVE_0X_MOVE_CONSTRCUTOR_AND_ASSIGMENT_OPERATOR
	memblock(memblock&& mb);
	memblock& operator=(memblock&& mb);
#endif

	void swap(memblock& block);

	static size_t compute_capacity(size_t old_capacity, size_t wanted_size, size_t min_capacity);

	T* m_start;
	T* m_finish;
	T* m_end_of_storage;

#if !HAVE_OX_EXPLICITLY_DEFAULTED_AND_DELETED_SPECIAL_MEMBER_FUNCTIONS
private:

	memblock(memblock const&);
	memblock& operator=(memblock const&);
#endif
};

template <typename T> struct is_movable;
template <typename U> struct is_movable< memblock<U> > { enum { value = 1 }; };

} // namespace mstl

#include "m_memblock.ipp"

#endif // _m_memblock_included

// vi:set ts=3 sw=3:
