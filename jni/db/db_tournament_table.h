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
// Copyright: (C)2011-2012 Gregor Cramer
// ======================================================================

// ======================================================================
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
// ======================================================================

#ifndef _db_tournament_table_included
#define _db_tournament_table_included

#include "db_date.h"
#include "db_common.h"

#include "m_map.h"
#include "m_chunk_allocator.h"
#include "m_utility.h"

namespace db {

class Database;
class NamebaseEvent;
class NamebasePlayer;
class Filter;

class TournamentTable : public mstl::noncopyable
{
public:

	enum Mode
	{
		Auto,
		Crosstable,
		Scheveningen,
		Swiss,
		Match,
		Knockout,
		RankingList,
	};

	enum Order
	{
		Score,
		Alphabetical,
		Rating,
		Federation,
	};

	enum KnockoutOrder
	{
		Triangle,
		Pyramid,
	};

	enum ScoringSystem
	{
		Traditional,
		Bilbao,			// 3 points for a win, 1 for a draw, 0 for lose
	};

	enum Tiebreak
	{
		Buchholz,					// swiss
		MedianBuchholz,			// swiss
		ModifiedMedianBuchholz,	// swiss
		SonnebornBerger,			// all-play-all, swiss
		Progressive,				// swiss
		KoyaSystem,					// all-play-all
		GamesWon,					// all-play-all
		GamesWonWithBlack,		// all-play-all
		RefinedBuchholz,			// swiss
		ParticularResult,			// (result of the individual game between the tied players)
		TraditionalScoring,		// (useful for Bilbao scoring system)
		None,

		LastBuchholz = SonnebornBerger,
		LastTiebreak = TraditionalScoring,
	};

	typedef Tiebreak TiebreakRules[6];

	TournamentTable(Database const& db, NamebaseEvent const& event, Filter const& gameFilter);
	~TournamentTable() throw();

	Mode mode() const;
	Mode bestMode() const;
	unsigned averageElo() const;
	unsigned fideCategory() const;
	unsigned countPlayers() const;
	int getPlayerId(unsigned ranking, color::ID& side) const;
	NamebasePlayer const* getPlayer(unsigned ranking) const;

	static unsigned fideCategory(unsigned elo);
	static unsigned computeEloPerformance(unsigned opponentAverage, unsigned percentage);
	static unsigned computeDWZPerformance(unsigned opponentAverage, unsigned percentage);
	static int ratingChange(int elo, int oppAvg, int percentage, int numGames);

	// private, although must be public due to technical reasons:

	class Player;
	friend class Player;

	struct Clash
	{
		Player*		player;
		Clash*		opponent;
		result::ID	result;
		unsigned		progress;
		unsigned		gameIndex;
		color::ID	color;
		unsigned		round;
		unsigned		subround;
		Date			date;
		bool			deleted;

		termination::Reason termination;
	};

private:

	typedef mstl::map<unsigned,Player*> PlayerMap;
	typedef mstl::vector<unsigned> Map;
	typedef mstl::chunk_allocator<Clash> Allocator;

	void buildList(Database const& db, Filter const& gameFilter);
	void eliminateDuplicates();
	void computeScores();
	void computePerformance();
	void guessBestMode();
	void computeTiebreaks();
	void sort(ScoringSystem scoringSystem, TiebreakRules const& tiebreakRules, Order order, Mode mode);

	NamebaseEvent const& m_event;

	Date			m_startDate;
	Date			m_endDate;
	Mode			m_bestMode;
	Mode			m_mode;
	Order			m_order;
	PlayerMap	m_playerMap;
	unsigned		m_avgElo;
	unsigned		m_numGames;
	unsigned		m_numRounds;
	unsigned		m_lastRound;
	unsigned		m_parity;
	unsigned		m_maxRound;
	unsigned		m_maxSubround;
	bool			m_missingRoundInfo;
	Map			m_orderMap;
	Allocator	m_allocator;
};

} // namespace db

#include "db_tournament_table.ipp"

#endif // _db_tournament_table_included

// vi:set ts=3 sw=3:
