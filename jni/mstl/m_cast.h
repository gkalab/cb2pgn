// ======================================================================
// Author : $Author$
// Version: $Revision$
// Date   : $Date$
// Url    : $URL$
// ======================================================================

// ======================================================================
// Copyright: (C) 2010-2012 Gregor Cramer
// ======================================================================

// ======================================================================
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
// ======================================================================

#ifndef _mstl_cast_included
#define _mstl_cast_included

namespace mstl {

template <class T, class U> T& safe_cast_ref(U& p);
template <class T, class U> T* safe_cast_ptr(U* p);

} // namespace mstl

#include "m_cast.ipp"

#endif // _mstl_cast_included

// vi:set ts=3 sw=3:
