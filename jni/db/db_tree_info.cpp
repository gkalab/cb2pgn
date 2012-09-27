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
// Copyright: (C) 2010-2012 Gregor Cramer
// ======================================================================

// ======================================================================
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
// ======================================================================

#include "db_tree_info.h"
#include "db_game_info.h"
#include "db_namebase_entry.h"
#include "db_tournament_table.h"

#include "m_utility.h"

#include <string.h>

using namespace db;


static NamebasePlayer g_player;


void
TreeInfo::Pair::operator+=(Pair const& pair)
{
	count += pair.count;
	sum += pair.sum;
}


TreeInfo::TreeInfo()
	:m_frequency(0)
	,m_lastYear(0)
	,m_bestRating(0)
	,m_bestPlayer(&g_player)
	,m_mostFrequentPlayer(&g_player)
{
	::memset(m_scoreCount, 0, sizeof(m_scoreCount));
}


TreeInfo::TreeInfo(Eco eco, Move const& move)
	:m_move(move)
	,m_eco(eco)
	,m_frequency(0)
	,m_lastYear(0)
	,m_bestRating(0)
	,m_bestPlayer(&g_player)
	,m_mostFrequentPlayer(&g_player)
{
	::memset(m_scoreCount, 0, sizeof(m_scoreCount));
}


void
TreeInfo::add(GameInfo const& info, color::ID sideToMove, rating::Type ratingType)
{
	// M_REQUIRE(ratingType != rating::Last);

	//M_ASSERT(unsigned(info.result()) < U_NUMBER_OF(m_scoreCount));

	NamebasePlayer const* player = info.playerEntry(sideToMove);

	++m_frequency;
	++m_scoreCount[info.result()];

	uint16_t rating = info.playerRating(sideToMove, ratingType);

	if (rating)
		m_averageRating.add(rating);

	if (rating > m_bestRating)
	{
		m_bestRating = rating;
		m_bestPlayer = player;
	}

	if (uint16_t oppRating = info.playerRating(color::opposite(sideToMove), ratingType))
		m_performance.add(oppRating);

	if (	m_mostFrequentPlayer->frequency() < player->frequency()
		|| (	m_mostFrequentPlayer->frequency() == player->frequency()
			&& m_mostFrequentPlayer->findRating(ratingType) < player->findRating(ratingType)))
	{
		m_mostFrequentPlayer = player;
	}

	if (uint16_t y = info.date().year())
	{
		m_averageYear.add(y);
		m_lastYear = mstl::max(m_lastYear, y);
	}
}


void
TreeInfo::add(TreeInfo const& info, rating::Type ratingType)
{
	// M_REQUIRE(ratingType != rating::Last);

	m_frequency							+= info.m_frequency;
	m_scoreCount[result::Unknown]	+= info.m_scoreCount[result::Unknown];
	m_scoreCount[result::White]	+= info.m_scoreCount[result::White];
//	m_scoreCount[result::Black]	+= info.m_scoreCount[result::Black];
	m_scoreCount[result::Draw]		+= info.m_scoreCount[result::Draw];
	m_scoreCount[result::Lost]		+= info.m_scoreCount[result::Lost];
	m_averageRating					+= info.m_averageRating;
	m_performance						+= info.m_performance;
	m_averageYear						+= info.m_averageYear;

	m_lastYear = mstl::max(m_lastYear, info.m_lastYear);

	if (info.m_bestRating > m_bestRating)
	{
		m_bestPlayer = info.m_bestPlayer;
		m_bestRating = info.m_bestRating;
	}

	if (	m_mostFrequentPlayer->frequency() < info.m_mostFrequentPlayer->frequency()
		|| (	m_mostFrequentPlayer->frequency() == info.m_mostFrequentPlayer->frequency()
			&& m_mostFrequentPlayer->findRating(ratingType)
					 < info.m_mostFrequentPlayer->findRating(ratingType)))
	{
		m_mostFrequentPlayer = info.m_mostFrequentPlayer;
	}
}


bool
TreeInfo::isLessThan(TreeInfo const& info, rating::Type ratingType, attribute::tree::ID column) const
{
	// M_REQUIRE(ratingType != rating::Last);

	switch (column)
	{
		case attribute::tree::Move:				return m_move < info.m_move;
		case attribute::tree::Eco:					return m_eco < info.m_eco;
		case attribute::tree::Frequency:			return m_frequency > info.m_frequency;
		case attribute::tree::AverageRating:	return averageRating() > info.averageRating();
		case attribute::tree::BestRating:		return m_bestRating > info.m_bestRating;
		case attribute::tree::AverageYear:		return m_averageYear && m_averageYear > info.m_averageYear;
		case attribute::tree::LastYear:			return m_lastYear && m_lastYear > info.m_lastYear;
		case attribute::tree::LastColumn:		return false;

		case attribute::tree::Performance:
			return performance(ratingType) > info.performance(ratingType);

		case attribute::tree::Score:
			return score() > info.score();

		case attribute::tree::Draws:
			return draws() > info.draws();

		case attribute::tree::BestPlayer:
			return m_bestPlayer->elo() && m_bestPlayer->elo() > info.m_bestPlayer->elo();

		case attribute::tree::MostFrequentPlayer:
			return m_mostFrequentPlayer->frequency() > info.m_mostFrequentPlayer->frequency();
	}

	return false;	// satsifies the compiler
}


int
TreeInfo::performance(rating::Type ratingType) const
{
	// M_REQUIRE(ratingType != rating::Last);

	if (m_performance.count < 10)
		return -1;

	unsigned average = unsigned(m_performance.average() + 0.5);

	if (average == 0)
		return -1;

	unsigned score = (this->score() + 5)/10;

	if (color::isBlack(m_move.color()))
		score = 100 - score;

	switch (ratingType)
	{
		case rating::ICCF:
		case rating::Elo:
			return TournamentTable::computeEloPerformance(average, score);

		case rating::USCF:
			return TournamentTable::computeEloPerformance(rating::convertUscfToElo(average), score);

		case rating::DWZ:
			return TournamentTable::computeDWZPerformance(average, score);

		// probably use 'int(average) + (int(score) - 50)*x'
		// we have to choose 'x' appropriately (e.g. 850 for ELO system)

		case rating::ECF:		return -1;	// don't know how to calculate
		case rating::IPS:		return -1;	// don't know how to calculate
		case rating::Rapid:	return -1;	// don't know how to calculate
		case rating::Rating:	return -1;	// don't know how to calculate

		case rating::Last:	return -1;//M_ASSERT(!"unexpected");
	}

	return score;	// satisfies the compiler
}

// vi:set ts=3 sw=3:
