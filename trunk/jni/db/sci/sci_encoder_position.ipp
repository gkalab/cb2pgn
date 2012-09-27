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

namespace db {
namespace sci {
namespace encoder {

inline Byte Position::Lookup::operator[](unsigned i) const	{ return numbers[i]; }
inline Byte& Position::Lookup::operator[](unsigned i)			{ return numbers[i]; }

inline Position::Lookup& Position::lookup() { return m_stack.top(); }

inline void Position::preparePush()	{ m_stack.reserve(m_stack.size() + 1); }
inline void Position::push()			{ m_stack.dup(); }
inline void Position::pop()			{ m_stack.pop(); }

inline void Position::doMove(Move const& move) { doMove(lookup(), move); }

inline Byte Position::operator[](int n) const { return m_stack.top()[n]; }


inline
Position::Lookup&
Position::previous()
{
	// M_ASSERT(m_stack.size() > 1);
	return *(m_stack.end() - 2);
}

} // namespace encoder
} // namespace sci
} // namespace db

// vi:set ts=3 sw=3:
