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
#include "m_algobase.h"
#include "m_construct.h"

#include <string.h>

namespace mstl {
namespace bits {

template <size_t N> struct uninitialized;

template <>
struct uninitialized<0>
{
	template <typename T, typename U>
	inline static U* copy(T const* first, T const* last, U* result)
	{
		T* curr = result;

		for ( ; first != last; ++first, ++curr)
			construct(curr, *first);

		return curr;
	}

	template<typename T>
	inline static T* fill_n(T* first, size_t n, T const& value)
	{
		while (n--)
			construct(first++, value);

		return first;
	}
};

template <size_t NBytes>
struct uninitialized_pod
{
	template <typename T, typename U>
	inline static U* copy(T const* first, T const* last, U* result)
	{
		::memmove(result, first, NBytes*(last - first));
		return result + (last - first);
	}

	template<typename T>
	inline static T* fill_n(T* first, size_t n, T const& value)
	{
		return ::mstl::fill_n(first, n, value);
	}
};

template <>
struct uninitialized_pod<1>
{
	template <typename T, typename U>
	inline static U* copy(T const* first, T const* last, U* result)
	{
		::memmove(result, first, last - first);
		return result + (last - first);
	}

	template<typename T>
	inline static T* fill_n(T* first, size_t n, T const value)
	{
		::memset(first, value, n);
		return first + n;
	}
};

template <>
struct uninitialized<1>
{
	template <typename T, typename U>
	inline static U* copy(T const* first, T const* last, U* result)
	{
		return uninitialized_pod<sizeof(T)>::copy(first, last, result);
	}

	template<typename T>
	inline static T* fill_n(T* first, size_t n, T const& value)
	{
		return uninitialized_pod<sizeof(T)>::fill_n(first, n, value);
	}
};

} // namespace bits


template<typename T, typename U>
inline
U*
uninitialized_copy(T const* first, T const* last, U* result)
{
	return bits::uninitialized<is_pod<T>::value>::copy(first, last, result);
}


template<typename T, typename U>
inline
U*
uninitialized_move(T const* first, T const* last, U* result)
{
	return bits::uninitialized<is_movable<T>::value>::copy(first, last, result);
}


template<typename T>
inline
T*
uninitialized_fill_n(T* first, size_t n, T const& value)
{
	return bits::uninitialized<is_pod<T>::value>::fill_n(first, n, value);
}

} // namespace mstl

// vi:set ts=3 sw=3:
