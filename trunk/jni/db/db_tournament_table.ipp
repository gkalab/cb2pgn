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

inline TournamentTable::Mode TournamentTable::mode() const		{ return m_mode; }
inline TournamentTable::Mode TournamentTable::bestMode() const	{ return m_bestMode; }
inline unsigned TournamentTable::averageElo() const				{ return m_avgElo; }
inline unsigned TournamentTable::countPlayers() const				{ return m_playerMap.size(); }


inline
unsigned
TournamentTable::fideCategory(unsigned elo)
{
	return elo <= 2251 ? 0 : 1 + ((elo - 2251)/25);
}


inline
unsigned
TournamentTable::fideCategory() const
{
	return fideCategory(m_avgElo);
}

} // namespace db

// vi:set ts=3 sw=3:
