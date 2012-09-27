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

#include "m_assert.h"

namespace mstl {

template <class T>
inline
unsigned
ref_counted_traits<T>::use_count(T const* obj)
{
	return obj ? obj->use_count() : 0;
}


template <class T>
inline
bool
ref_counted_traits<T>::expired(T const* obj)
{
	return use_count(obj) == 0;
}


template <class T>
inline
bool
ref_counted_traits<T>::unique(T const* obj)
{
	return use_count(obj) == 1;
}


template <class T>
inline
void
ref_counted_traits<T>::ref(T* obj)
{
	// M_REQUIRE(obj);

	obj->ref();
}


template <class T>
inline
bool
ref_counted_traits<T>::release(T* obj)
{
	// M_REQUIRE(!expired(obj));

	return obj->release();
}

} // namespace mstl

// vi:set ts=3 sw=3:
