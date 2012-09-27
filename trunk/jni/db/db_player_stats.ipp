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
// Copyright: (C) 2012 Gregor Cramer
// ======================================================================

// ======================================================================
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
// ======================================================================

#include "m_assert.h"

namespace db {

inline Date const& PlayerStats::firstDate() const	{ return m_firstDate; }
inline Date const& PlayerStats::lastDate() const	{ return m_lastDate; }

inline uint16_t PlayerStats::minRating(rating::Type type) const { return m_minRating[type]; }
inline uint16_t PlayerStats::maxRating(rating::Type type) const { return m_maxRating[type]; }

inline unsigned PlayerStats::countGames() const { return m_countGames[0] + m_countGames[1]; }
inline unsigned PlayerStats::countGames(color::ID color) const { return m_countGames[color]; }

inline unsigned PlayerStats::countEcoLines(color::ID color) const	{ return m_ecoMap[color].size(); }


inline
unsigned
PlayerStats::score(color::ID color, result::ID result) const
{
	return m_score[color][result];
}


inline
unsigned
PlayerStats::score(result::ID result) const
{
	return score(color::White, result) + score(color::Black, result::opponent(result));
}


inline
Eco const&
PlayerStats::ecoLine(color::ID color, unsigned i) const
{
	//M_REQUIRE(i < countEcoLines(color));
	return m_ecoMap[color].container()[i].first;
}


inline
unsigned
PlayerStats::ecoCount(color::ID color, unsigned i) const
{
	//M_REQUIRE(i < countEcoLines(color));
	return m_ecoMap[color].container()[i].second;
}

} // namespace db

// vi:set ts=3 sw=3:
