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

#ifndef _mstl_construct_included
#define _mstl_construct_included

namespace mstl {
namespace bits {

template<typename T, typename U> void construct(T* p, U const& value);
template<typename T> void construct(T* p);
template<typename T> void destroy(T* pointer);
template<typename T> void destroy(T* first, T* last);

} // namespace bits
} // namespace mstl

#include "m_construct.ipp"

#endif // _mstl_construct_included

// vi:set ts=3 sw=3:
