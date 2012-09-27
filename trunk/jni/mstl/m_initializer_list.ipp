// ======================================================================
// Author : $Author$
// Version: $Revision$
// Date   : $Date$
// Url    : $URL$
// ======================================================================

// ======================================================================
// Copyright: (C) 2011-2012 Gregor Cramer
// ======================================================================

// ======================================================================
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
// ======================================================================

namespace mstl {

inline constexpr initializer_list::initializer_list() :m_arr(0), m_size(0) {}

inline constexpr initializer_list::size_type size()									{ return m_size; }
inline constexpr initializer_list::const_iterator initializer_list::begin()	{ return m_arr; }
inline constexpr initializer_list::const_iterator initializer_list::end()		{ return m_arr + m_size; }


inline
constexpr
initializer_list::initializer_list(const_iterator arr, size_type size)
	:m_arr(arr)
	,m_size(size)
{
}


/// Return an iterator pointing to the first element of the initilizer_list.
template<class T>
inline
constexpr T const* begin(initializer_list<T> list) { return list.begin(); }


/// Return an iterator pointing to one past the last element of the initilizer_list.
template<class T>
inline
constexpr T const* end(initializer_list<T> list) { return list.end(); }

} // namespace mstl

// vi:set ts=3 sw=3:
