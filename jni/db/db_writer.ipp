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
// Copyright: (C) 2009-2012 Gregor Cramer
// ======================================================================

// ======================================================================
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
// ======================================================================

#include "m_assert.h"

namespace db {

inline unsigned Writer::level() const		{ return m_level; }
inline unsigned Writer::flags() const		{ return m_flags; }
inline bool Writer::needSpace() const		{ return m_needSpace; }
inline bool Writer::insideComment() const	{ return m_nullLevel; }

inline bool Writer::test(unsigned flags) const { return m_flags & flags; }

inline void Writer::addFlag(unsigned flag)		{ m_flags |= flag; }
inline void Writer::removeFlag(unsigned flag)	{ m_flags &= ~flag; }

} // namespace db

// vi:set ts=3 sw=3:
