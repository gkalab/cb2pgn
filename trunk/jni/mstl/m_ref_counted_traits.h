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

#ifndef _mstl_ref_counted_traits_included
#define _mstl_ref_counted_traits_included

namespace mstl {

template <class T>
struct ref_counted_traits
{
	// queries
	static bool expired(T const* obj);
	static bool unique(T const* obj);

	// accessors
	static unsigned use_count(T const* obj);

	// modifiers
	static void ref(T* obj);
	static bool release(T* obj);
};

} // namespace mstl

#include "m_ref_counted_traits.ipp"

#endif // _mstl_ref_counted_traits_included

// vi:set ts=3 sw=3:
