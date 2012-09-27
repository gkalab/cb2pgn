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

namespace db {

inline bool MoveInfoTable::isEmpty() const	{ return m_table.empty(); }
inline unsigned MoveInfoTable::size() const	{ return m_table.size(); }

inline MoveInfoSet const& MoveInfoTable::operator[](unsigned n) const	{ return m_table[n]; }
inline MoveInfoSet& MoveInfoTable::operator[](unsigned n)					{ return m_table[n]; }
inline MoveInfoSet& MoveInfoTable::back()											{ return m_table.back(); }

inline void MoveInfoTable::resize(unsigned n)	{ m_table.resize(n); }
inline void MoveInfoTable::reserve(unsigned n)	{ m_table.reserve(n); }
inline void MoveInfoTable::clear()					{ m_table.clear(); }


inline bool
MoveInfoTable::isEmpty(unsigned n) const
{
	return n < m_table.size() || m_table[n].isEmpty();
}

} // namespace db

// vi:set ts=3 sw=3:
