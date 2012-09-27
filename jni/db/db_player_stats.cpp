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

#include "db_player_stats.h"

#include "u_base.h"

#include "m_assert.h"

#include <string.h>
#include <stdlib.h>

using namespace db;


static int
cmpEco(void const* lhs, void const* rhs)
{
	return	int(static_cast<PlayerStats::MapType const*>(rhs)->second)
			 - int(static_cast<PlayerStats::MapType const*>(lhs)->second);
}


PlayerStats::PlayerStats()
{
	::memset(m_countGames, 0, sizeof(m_countGames));
	::memset(m_minRating, 0, sizeof(m_minRating));
	::memset(m_maxRating, 0, sizeof(m_maxRating));
	::memset(m_score, 0, sizeof(m_score));
}


void
PlayerStats::addDate(Date const& date)
{
	if (date)
	{
		if (m_firstDate)
			m_firstDate = mstl::min(m_firstDate, date);
		else
			m_firstDate = date;

		if (m_lastDate)
			m_lastDate = mstl::max(m_lastDate, date);
		else
			m_lastDate = date;
	}
}


void
PlayerStats::addRating(rating::Type type, uint16_t value)
{
	if (type != rating::Last && value > 0)
	{
		if (m_minRating[type] == 0)
			m_minRating[type] = value;
		else
			m_minRating[type] = mstl::min(m_minRating[type], value);

		m_maxRating[type] = mstl::max(m_maxRating[type], value);
	}
}


void
PlayerStats::addScore(color::ID color, result::ID result)
{
	// M_ASSERT(size_t(result) < U_NUMBER_OF(m_score[0]));

	++m_countGames[color];
	m_score[color][result]++;
}


void
PlayerStats::addEco(color::ID color, Eco const& eco)
{
	++m_ecoMap[color][eco];
}


void
PlayerStats::finish()
{
	for (unsigned i = 0; i < 2; ++i)
	{
		::qsort(	const_cast<MapType*>(m_ecoMap[i].container().begin()),
					m_ecoMap[i].size(),
					sizeof(MapType),
					::cmpEco);
	}
}


double
PlayerStats::percentage() const
{
	unsigned countGames = m_countGames[color::White] + m_countGames[color::Black];

	if (countGames == 0)
		return 0.0;

	double score	= (m_score[color::White][result::White] + m_score[color::Black][result::Black])*2.0
						+ m_score[color::White][result::Draw]
						+ m_score[color::Black][result::Draw]
						+ m_score[color::White][result::Unknown]
						+ m_score[color::Black][result::Unknown];

	return score*50.0/countGames;
}


double
PlayerStats::percentage(color::ID color) const
{
	if (m_countGames[color] == 0)
		return 0.0;

	double score	= m_score[color][result::fromColor(color)]*2.0
						+ m_score[color][result::Draw]
						+ m_score[color][result::Unknown];

	return score*50.0/m_countGames[color];
}

// vi:set ts=3 sw=3:
