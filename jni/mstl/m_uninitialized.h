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

#ifndef _mstl_uninitialized_included
#define _mstl_uninitialized_included

#include <stddef.h>

namespace mstl {

template<typename T, typename U> U* uninitialized_copy(T const* first, T const* last, U* result);
template<typename T, typename U> U* uninitialized_move(T const* first, T const* last, U* result);
template<typename T> T* uninitialized_fill_n(T* first, size_t n, T const& value);

} // namespace mstl

#include "m_uninitialized.ipp"

#endif // _mstl_uninitialized_included

// vi:set ts=3 sw=3:
