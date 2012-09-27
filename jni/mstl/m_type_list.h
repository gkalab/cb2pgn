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

#ifndef _mstl_type_list_included
#define _mstl_type_list_included

namespace mstl {

struct null_type {};

template <typename T, typename U>
struct type_list
{
	typedef T head;
	typedef U tail;
};

namespace tl {

template <typename TList> struct length;
template <typename TList, int Index> struct type_at;
template <typename TList, int Index, typename DefaultType> struct type_at_non_strict;
template <typename TList, typename T> struct index_of;
template <typename TList, typename T> struct append;
template <typename TList, typename T> struct erase;
template <typename TList, typename T> struct erase_all;
template <typename TList> struct no_duplicates;
template <typename TList, typename T, typename U> struct replace;
template <typename TList, typename T, typename U> struct replace_all;
template <typename TList> struct reverse;
template <typename TList> struct size_of;
#if 0
template <typename TList, typename T> struct most_derived;
template <typename TList> struct derived_to_front;
#endif

} // namespace tl
} // namespace mstl

#include "m_type_list.ipp"

#endif // _mstl_type_list_included

// vi:set ts=3 sw=3:
