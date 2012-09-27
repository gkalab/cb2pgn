// ======================================================================
// Author : $Author$
// Version: $Revision$
// Date   : $Date$
// Url    : $URL$
// ======================================================================

// ======================================================================
//    _/|            __
//   // o\         /    )           ,        /    /
//   || ._)    ----\---------__----------__-/----/__-
//   //__\          \      /   '  /    /   /    /   )
//   )___(     _(____/____(___ __/____(___/____(___/_
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

#include "m_assert.h"

namespace db {

inline bool EngineList::isEmpty() const				{ return m_map.empty(); }

inline unsigned EngineList::count() const				{ return m_map.size(); }

inline void EngineList::swap(EngineList& engines)	{ m_map.swap(engines.m_map); }
inline void EngineList::reserve(unsigned size)		{ m_map.reserve(size); }
inline void EngineList::clear()							{ m_map.clear(); }


inline
mstl::string const&
EngineList::operator[](unsigned n) const
{
	//M_REQUIRE(n < count());
	return engine(n + 1);
}

} // namespace db

// vi:set ts=3 sw=3:
