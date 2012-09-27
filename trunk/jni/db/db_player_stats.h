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

#ifndef _db_player_stats_included
#define _db_player_stats_included

#include "db_date.h"
#include "db_eco.h"
#include "db_common.h"

#include "m_map.h"

namespace db {

class PlayerStats
{
public:

	PlayerStats();

	unsigned countGames() const;
	unsigned countGames(color::ID color) const;
	Date const& firstDate() const;
	Date const& lastDate() const;
	uint16_t minRating(rating::Type type) const;
	uint16_t maxRating(rating::Type type) const;
	unsigned score(color::ID color, result::ID result) const;
	unsigned score(result::ID result) const;
	double percentage(color::ID color) const;
	double percentage() const;
	unsigned countEcoLines(color::ID color) const;
	Eco const& ecoLine(color::ID color, unsigned i) const;
	unsigned ecoCount(color::ID color, unsigned i) const;

	void addDate(Date const& date);
	void addRating(rating::Type type, uint16_t value);
	void addScore(color::ID color, result::ID result);
	void addEco(color::ID color, Eco const& eco);

	void finish();

private:

	typedef mstl::map<Eco,unsigned> EcoMap;

	Date		m_firstDate;
	Date		m_lastDate;
	unsigned	m_countGames[2];
	uint16_t	m_score[2][5];
	uint16_t	m_minRating[rating::Last];
	uint16_t	m_maxRating[rating::Last];
	EcoMap	m_ecoMap[2];

public:

	typedef EcoMap::value_type MapType;
};

};

#include "db_player_stats.ipp"

#endif // _db_player_stats_included

// vi:set ts=3 sw=3:
