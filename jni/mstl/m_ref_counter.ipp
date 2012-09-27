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

inline
ref_counter::ref_counter()
	:m_count(0)
{
}


inline
ref_counter::ref_counter(ref_counter const& counter)
	:mstl::noncopyable()
	,m_count(counter.m_count)
{
}


inline
unsigned
ref_counter::use_count() const
{
	return m_count;
}


inline
bool
ref_counter::expired() const
{
	return use_count() == 0;
}


inline
bool
ref_counter::unique() const
{
	return use_count() == 1;
}


inline
void
ref_counter::ref()
{
	++m_count;
};


inline
bool
ref_counter::release()
{
	// M_REQUIRE(!expired());

	return --m_count == 0;
}

} // namespace mstl

// vi:set ts=3 sw=3:
