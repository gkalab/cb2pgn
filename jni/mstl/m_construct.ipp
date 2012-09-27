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

#include "m_type_traits.h"

#include <new>

namespace mstl {
namespace bits {
namespace detail {

template <size_t N> struct destroy;

template <>
struct destroy<0>
{
	template <typename T>
	inline static void each(T* first, T* last)
	{
		for ( ; first != last; ++first)
			::mstl::bits::destroy(first);
	}
};

template <>
struct destroy<1>
{
	template <typename T> inline static void each(T*, T*) {}
};

} // namespace detail


template<typename T, typename U>
inline void construct(T* p, U const& value) { new(static_cast<void*>(p)) T(value); }

template<typename T> inline void construct(T* p) { new(static_cast<void*>(p)) T(); }

template<typename T>
inline void destroy(T* pointer) { pointer->~T(); }

template<typename T>
inline
void
destroy(T* first, T* last)
{
	detail::destroy<has_trivial_destructor<T>::value>::each(first, last);
}

} // namespace bits
} // namespace mstl

// vi:set ts=3 sw=3:
