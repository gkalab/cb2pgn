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

#include "db_tournament_table.h"
#include "db_database.h"
#include "db_game_info.h"
#include "db_namebase_entry.h"
#include "db_tree_info.h"
#include "db_filter.h"

#include "m_vector.h"
#include "m_utility.h"
#include "m_ref_counted_ptr.h"
#include "m_ref_counter.h"
#include "m_assert.h"
#include "m_bitset.h"
#include "m_limits.h"
#include "m_algorithm.h"

#include <stdlib.h>
#include <string.h>

//#define DEBUG

#ifdef DEBUG
# define TRACE(x) x
#else
# define TRACE(x)
#endif

using namespace db;

enum { MaxRounds = 255 };


static unsigned const InvalidScore = mstl::numeric_limits<unsigned>::max();

static TournamentTable::Tiebreak m_tiebreakRule = TournamentTable::None;


static unsigned const EloDiff[51] =
{
//   0    1    2    3    4    5    6    7    8    9
	  0,   7,  14,  21,  29,  36,  43,  50,  57,  65,	//  50 - 59
	 72,  80,  87,  95, 102, 110, 117, 125, 133, 141,	//  60 - 69
	149, 158, 166, 175, 184, 193, 202, 211, 220, 230,	//  70 - 79
	366, 383, 401, 422, 444, 470, 501, 538, 589, 677,	//  90 - 99
	999,																// 100
};


static unsigned
opponentElo(unsigned elo, unsigned oppElo)
{
	enum { Margin = 350 };

	if (elo > 0)
	{
		if (elo > oppElo + Margin)
			return elo - Margin;
		if (oppElo > elo + Margin)
			return elo + Margin;
	}

	return oppElo;
}


struct TournamentTable::Player
{
	enum { Win, Draw, Loss };

	typedef TournamentTable::Clash Clash;
	typedef mstl::vector<Clash*> ClashList;
	typedef Clash* Lookup[::MaxRounds + 1];

	Player(NamebasePlayer const* entry, unsigned index);

	unsigned oppAvgRating() const;
	unsigned percentage() const;

	int resultScore(TournamentTable::Clash const* clash) const;

	NamebasePlayer const* entry;

	unsigned			ranking;
	unsigned			group;
	unsigned			elo;
	unsigned			performance;
	int				ratingChange;
	unsigned			score[2];
	unsigned			winDrawLoss[3];
	unsigned			medianScore;
	unsigned			lastProgress;
	unsigned			maxRound;
	unsigned			tiebreak[LastTiebreak + 1];
	unsigned			refinedBuchholz[LastBuchholz + 1];
	unsigned			oppEloCount;
	unsigned			oppEloTotal;
	unsigned			oppEloScore;
	ScoringSystem	scoringSystem;
	ClashList		clashList;
	Lookup			lookup;
};


#ifdef DEBUG

#include <stdio.h>

static void
debugClash(TournamentTable::Player::Clash const* clash)
{
	printf("Name:       %s\n", clash->player->entry->name().c_str());
	printf("Opponent:   %s\n", clash->opponent->player->entry->name().c_str());
	printf("Result:     %s\n", result::toString(clash->result).c_str());
	printf("Progress:   %u\n", clash->progress);
	printf("Color:      %s\n", clash->color == color::White ? "white" : "black");
	printf("Game index: %u\n", clash->gameIndex);
	printf("Date:       %s\n", clash->date.asString().c_str());
	printf("Round:      %u\n", clash->round);
	printf("Sub-round:  %u\n", clash->subround);
	printf("Deleted:    %s\n", clash->deleted ? "yes" : "no");
	printf("--------------------------------------------\n");
}


static void
debugPlayer(TournamentTable::Player const* player)
{
	printf("Name:           %s\n", player->entry->name().c_str());
	printf("Ranking:        %u\n", player->ranking);
	printf("Group:          %u\n", player->group);
	printf("Trad. Score:    %u\n", player->score[TournamentTable::Traditional]);
	printf("Bilbao Score:   %u\n", player->score[TournamentTable::Bilbao]);
	printf("Median Score:   %u\n", player->medianScore);
	printf("Max.Round:      %u\n", player->maxRound);
	printf("Game Count:     %u\n", unsigned(player->clashList.size()));
	printf("Elo:            %u\n", player->elo);
	printf("Performance:    %u\n", player->performance);
	printf("Rating Change:  %d\n", player->ratingChange);
	printf("Tiebreak:      ");
	for (unsigned i = 0; i < unsigned(TournamentTable::LastTiebreak); ++i)
		printf(" %u", player->tiebreak[i]);
	printf("\n");
	printf("Refined:       ");
	for (unsigned i = 0; i < unsigned(TournamentTable::LastBuchholz); ++i)
		printf(" %u", player->refinedBuchholz[i]);
	printf("\n");
	printf("Win/Draw/Loss:  +%u =%u -%u\n",
				player->winDrawLoss[TournamentTable::Player::Win],
				player->winDrawLoss[TournamentTable::Player::Draw],
				player->winDrawLoss[TournamentTable::Player::Loss]);
	printf("Opp. Elo Count: %u\n", player->oppEloCount);
	printf("Opp. Elo Total: %u\n", player->oppEloTotal);
	printf("Opp. Elo Score: %u\n", player->oppEloScore);
	printf("============================================\n");
}

#endif


static bool
match(TournamentTable::Clash const* lhs, TournamentTable::Clash const* rhs)
{
	return	lhs->opponent->player == rhs->opponent->player
			&& lhs->round == rhs->round
			&& lhs->subround == rhs->subround;
}


static void
removeOpponentsClash(TournamentTable::Clash const* clash)
{
	TournamentTable::Clash const* opponent = clash->opponent;
	TournamentTable::Player::ClashList& clashList = opponent->player->clashList;

	for (TournamentTable::Player::ClashList::iterator i = clashList.begin(); i != clashList.end(); ++i)
	{
		if (*i== opponent)
		{
			clashList.erase(i);
			return;
		}
	}
}



TournamentTable::Player::Player(NamebasePlayer const* entry, unsigned ranking)
	:entry(entry)
	,ranking(ranking)
	,group(0)
	,elo(0)
	,performance(0)
	,ratingChange(0)
	,medianScore(0)
	,lastProgress(0)
	,maxRound(0)
	,oppEloCount(0)
	,oppEloTotal(0)
	,oppEloScore(0)
{
	::memset(score, 0, sizeof(score));
	::memset(winDrawLoss, 0, sizeof(winDrawLoss));
	::memset(lookup, 0, sizeof(lookup));
	::memset(tiebreak, 0, sizeof(tiebreak));
	::memset(refinedBuchholz, 0, sizeof(refinedBuchholz));
}


int
TournamentTable::Player::resultScore(TournamentTable::Clash const* clash) const
{
	//M_ASSERT(clash);

	switch (scoringSystem)
	{
		case TournamentTable::Traditional:
			return result::value(clash->result);	// return white=2, draw=1, black=0

		case TournamentTable::Bilbao:
			switch (int(clash->result))
			{
				case result::White:	return 3;
				case result::Black:	return 0;
				case result::Draw:	return 2;
				case result::Lost:	return 0;
			}
			break;
	}

	return -1; // never reached
}


unsigned
TournamentTable::Player::oppAvgRating() const
{
	return oppEloCount ? oppEloTotal/oppEloCount : 0;
}


unsigned
TournamentTable::Player::percentage() const
{
	return oppEloCount ? (oppEloScore*50 + mstl::div2(oppEloCount))/oppEloCount : 0;
}


static int
cmpClashOpponent(void const* lhs, void const* rhs)
{
	TournamentTable::Clash const* cl = *reinterpret_cast<TournamentTable::Clash* const*>(lhs);
	TournamentTable::Clash const* cr = *reinterpret_cast<TournamentTable::Clash* const*>(rhs);

	return int(cl->opponent->player->ranking) - int(cr->opponent->player->ranking);
}


static int
cmpClashRound(void const* lhs, void const* rhs)
{
	TournamentTable::Clash const* cl = *reinterpret_cast<TournamentTable::Clash* const*>(lhs);
	TournamentTable::Clash const* cr = *reinterpret_cast<TournamentTable::Clash* const*>(rhs);

	if (cl->round != cr->round)
		return int(cl->round) - int(cr->round);

	if (cl->subround != cr->subround)
		return int(cl->subround) - int(cr->subround);

	if (cl->date != cr->date)
		return Date::compare(cl->date, cr->date);

	return int(cl->gameIndex) - int(cr->gameIndex);
}


static int
cmpName(void const* lhs, void const* rhs)
{
	return ::strcmp(	(*static_cast<TournamentTable::Player* const*>(lhs))->entry->name(),
							(*static_cast<TournamentTable::Player* const*>(rhs))->entry->name());
}


static int
cmpRating(void const* lhs, void const* rhs)
{
	return	int((*static_cast<TournamentTable::Player* const*>(rhs))->elo)
			 - int((*static_cast<TournamentTable::Player* const*>(lhs))->elo);
}


static int
cmpFederation(void const* lhs, void const* rhs)
{
	return country::compare(
				(*static_cast<TournamentTable::Player* const*>(lhs))->entry->findFederation(),
				(*static_cast<TournamentTable::Player* const*>(rhs))->entry->findFederation());
}


static int
cmpScore(void const* lhs, void const* rhs)
{
	TournamentTable::Player const* pl = *static_cast<TournamentTable::Player* const*>(lhs);
	TournamentTable::Player const* pr = *static_cast<TournamentTable::Player* const*>(rhs);

	unsigned lscore = pl->score[pl->scoringSystem];
	unsigned rscore = pr->score[pr->scoringSystem];

	if (lscore != rscore)
		return int(rscore) - int(lscore);

	return int(pl->clashList.size()) - int(pr->clashList.size());
}


static int
cmpProgress(void const* lhs, void const* rhs)
{
	TournamentTable::Player const* pl = *static_cast<TournamentTable::Player* const*>(lhs);
	TournamentTable::Player const* pr = *static_cast<TournamentTable::Player* const*>(rhs);

	if (pl->maxRound != pr->maxRound)
		return int(pr->maxRound) - int(pl->maxRound);

	TournamentTable::Clash const* cl = pl->lookup[pl->maxRound];
	TournamentTable::Clash const* cr = pr->lookup[pr->maxRound];

	if (cl && cr && cr->progress != cl->progress)
		return int(cr->progress) - int(cl->progress);

	return cmpScore(lhs, rhs);
}


static int
cmpTiebreak(void const* lhs, void const* rhs)
{
	TournamentTable::Player const* pl = *static_cast<TournamentTable::Player* const*>(lhs);
	TournamentTable::Player const* pr = *static_cast<TournamentTable::Player* const*>(rhs);

	//M_ASSERT(size_t(m_tiebreakRule) < U_NUMBER_OF(pl->tiebreak));
	return int(pr->tiebreak[m_tiebreakRule]) - int(pl->tiebreak[m_tiebreakRule]);
}


static int
cmpRefinement(void const* lhs, void const* rhs)
{
	TournamentTable::Player const* pl = *static_cast<TournamentTable::Player* const*>(lhs);
	TournamentTable::Player const* pr = *static_cast<TournamentTable::Player* const*>(rhs);

	//M_ASSERT(size_t(m_tiebreakRule) < U_NUMBER_OF(pl->refinedBuchholz));
	return int(pr->refinedBuchholz[m_tiebreakRule]) - int(pl->refinedBuchholz[m_tiebreakRule]);
}


static int
cmpGroup(void const* lhs, void const* rhs)
{
	TournamentTable::Player const* pl = *static_cast<TournamentTable::Player* const*>(lhs);
	TournamentTable::Player const* pr = *static_cast<TournamentTable::Player* const*>(rhs);

	return int(pl->group) - int(pr->group);
}


TournamentTable::~TournamentTable() throw()
{
	for (PlayerMap::iterator i = m_playerMap.begin(); i != m_playerMap.end(); ++i)
		delete i->second;
}


TournamentTable::TournamentTable(Database const& db,
											NamebaseEvent const& event,
											Filter const& gameFilter)
	:m_event(event)
	,m_startDate(event.date())
	,m_endDate(event.date())
	,m_bestMode(RankingList)
	,m_mode(RankingList)
	,m_order(Score)
	,m_avgElo(0)
	,m_numGames(gameFilter.count())
	,m_numRounds(0)
	,m_lastRound(1)
	,m_parity(0)
	,m_maxRound(0)
	,m_maxSubround(0)
	,m_missingRoundInfo(false)
	,m_allocator(32768)
{
	m_playerMap.reserve(2*m_numGames);

	buildList(db, gameFilter);
	eliminateDuplicates();
	computeScores();
	computePerformance();
	guessBestMode();
	computeTiebreaks();
}


void
TournamentTable::buildList(Database const& db, Filter const& gameFilter)
{
	for (int index = gameFilter.next(); index != Filter::Invalid; index = gameFilter.next(index))
	{
		GameInfo const&			info			= db.gameInfo(index);
		NamebasePlayer const*	whitePlayer = info.playerEntry(color::White);
		NamebasePlayer const*	blackPlayer = info.playerEntry(color::Black);

		Player* wPlayer = m_playerMap[whitePlayer->id()];
		Player* bPlayer = m_playerMap[blackPlayer->id()];

		if (wPlayer == 0)
		{
			wPlayer = new Player(whitePlayer, m_playerMap.size() - 1);
			m_playerMap.replace(PlayerMap::value_type(whitePlayer->id(), wPlayer));
		}
		if (bPlayer == 0)
		{
			bPlayer = new Player(blackPlayer, m_playerMap.size() - 1);
			m_playerMap.replace(PlayerMap::value_type(blackPlayer->id(), bPlayer));
		}

		m_maxRound = mstl::max(info.round(), m_maxRound);
		m_maxSubround = mstl::max(info.subround(), m_maxSubround);

		if (info.round() == 0)
			m_missingRoundInfo = true;

		wPlayer->elo = mstl::max(uint16_t(wPlayer->elo), info.findElo(color::White));
		bPlayer->elo = mstl::max(uint16_t(bPlayer->elo), info.findElo(color::Black));

		Clash* wClash = m_allocator.alloc();
		Clash* bClash = m_allocator.alloc();

		wPlayer->clashList.push_back(wClash);
		bPlayer->clashList.push_back(bClash);

		wClash->player = wPlayer;
		wClash->opponent = bClash;
		wClash->result = info.result();
		wClash->progress = 0;
		wClash->gameIndex = index;
		wClash->date = info.date();
		wClash->color = color::White;
		wClash->round = info.round();
		wClash->subround = info.subround();
		wClash->deleted = info.isDeleted();
		wClash->termination = info.terminationReason();

		bClash->player = bPlayer;
		bClash->opponent = wClash;
		bClash->result = result::opponent(info.result());
		bClash->progress = 0;
		bClash->date = info.date();
		bClash->gameIndex = index;
		bClash->color = color::Black;
		bClash->round = info.round();
		bClash->subround = info.subround();
		bClash->deleted = info.isDeleted();
		bClash->termination = info.terminationReason();

		if (!m_startDate || (info.date() < m_startDate && m_startDate.year() - info.dateYear() <= 1))
			m_startDate = info.date();
		if (!m_endDate || (info.date() > m_endDate && m_endDate.year() - info.dateYear() <= 1))
			m_endDate = info.date();
	}
}


void
TournamentTable::eliminateDuplicates()
{
	for (unsigned i = 0; i < m_playerMap.size(); ++i)
		m_playerMap.container()[i].second->ranking = i;

	for (PlayerMap::iterator i = m_playerMap.begin(); i != m_playerMap.end(); ++i)
	{
		Player* player = i->second;

		::qsort(player->clashList.begin(), player->clashList.size(), sizeof(Clash*), ::cmpClashRound);
		::qsort(player->clashList.begin(), player->clashList.size(), sizeof(Clash*), ::cmpClashOpponent);

		Player::ClashList& clashList = player->clashList;
		Player::ClashList::reverse_iterator prev = clashList.rbegin();

		for (Player::ClashList::reverse_iterator i = prev + 1; i != clashList.rend(); ++i)
		{
			if (match(*i, *prev))
			{
				if ((*i)->deleted)
				{
					removeOpponentsClash(*i);
					clashList.erase(i.base());
				}
				else
				{
					if ((*prev)->deleted)
					{
						removeOpponentsClash(*prev);
						clashList.erase(prev.base());
					}

					prev = i;
				}
			}
			else
			{
				prev = i;
			}
		}
	}
}


void
TournamentTable::computeScores()
{
	for (PlayerMap::iterator i = m_playerMap.begin(); i != m_playerMap.end(); ++i)
	{
		Player* player = i->second;
		Player::ClashList const& clashList = player->clashList;

		for (unsigned k = 0; k < clashList.size(); ++k)
		{
			Clash* clash = clashList[k];
			Player* opponent = clash->opponent->player;

			player->maxRound = mstl::max(player->maxRound, clash->round);

			if (clash->color == color::White)
			{
				switch (int(clash->result))
				{
					case result::White:
					case result::Black:
					case result::Draw:
						if (opponent->elo > 0)
						{
							++player->oppEloCount;
							player->oppEloTotal += ::opponentElo(player->elo, opponent->elo);
						}
						if (player->elo > 0)
						{
							++opponent->oppEloCount;
							opponent->oppEloTotal += ::opponentElo(opponent->elo, player->elo);
						}
						break;
				}

				unsigned whiteMedianScore = 0;
				unsigned blackMedianScore = 0;

				switch (int(clash->result))
				{
					case result::White:
						++player->winDrawLoss[Player::Win];
						player->score[Traditional] += 2;
						player->score[Bilbao] += 6;
						whiteMedianScore = 2;
						if (opponent->elo > 0)
							player->oppEloScore += 2;
						break;

					case result::Black:
						++player->winDrawLoss[Player::Loss];
						opponent->score[Traditional] += 2;
						opponent->score[Bilbao] += 6;
						blackMedianScore = 2;
						if (player->elo > 0)
							opponent->oppEloScore += 2;
						break;

					case result::Draw:
						++player->winDrawLoss[Player::Draw];
						player->score[Traditional] += 1;
						player->score[Bilbao] += 2;
						opponent->score[Traditional] += 1;
						opponent->score[Bilbao] += 2;
						whiteMedianScore = 1;
						blackMedianScore = 1;
						if (opponent->elo > 0)
							player->oppEloScore += 1;
						if (player->elo > 0)
							opponent->oppEloScore += 1;
						break;
				}

				if (clash->termination == termination::Unplayed)
				{
					if (whiteMedianScore == 2)
						whiteMedianScore = 1;
					if (blackMedianScore == 2)
						blackMedianScore = 1;
				}

				player->medianScore += whiteMedianScore;
				opponent->medianScore += blackMedianScore;

			}
		}
	}
}


void
TournamentTable::computePerformance()
{
	unsigned gamesPerRound[MaxRounds + 1];
	unsigned eloCount = 0;

	::memset(gamesPerRound, 0, sizeof(gamesPerRound));

	for (PlayerMap::iterator i = m_playerMap.begin(); i != m_playerMap.end(); ++i)
	{
		Player* player = i->second;
		Player::ClashList const& clashList = player->clashList;
		Player const* previous = 0;
		unsigned parity = 0;
		unsigned rounds = 0;

		for (unsigned k = 0; k < clashList.size(); ++k)
		{
			Clash* clash = clashList[k];
			Player* opponent = clash->opponent->player;

			if (opponent == previous)
			{
				++parity;
			}
			else
			{
				m_parity = mstl::max(m_parity, parity);
				previous = opponent;
				parity = 1;
				++rounds;

				if (clash->color == color::White)
					++gamesPerRound[clash->round];
			}

			if (clash->subround <= 1 || player->lookup[clash->round] == 0)
				player->lookup[clash->round] = clash;

			if (Clash* root = player->lookup[clash->round])
				root->progress += result::value(clash->result);

			TRACE(debugClash(clashList[k]));
		}

		m_parity = mstl::max(m_parity, parity);
		m_numRounds  = mstl::max(m_numRounds, rounds);

		unsigned oppAvgRating	= player->oppAvgRating();
		unsigned percentage		= player->percentage();
		unsigned performance		= computeEloPerformance(oppAvgRating, percentage);

		if (0 < performance && performance < 5000)
		{
			if (player->elo)
			{
				player->performance = performance;
				player->ratingChange = ratingChange(player->elo,
																oppAvgRating,
																percentage,
																player->oppEloCount);
			}
		}

		if (player->elo > 0)
		{
			++eloCount;
			m_avgElo += player->elo;
		}
	}

	m_numRounds = mstl::min(unsigned(::MaxRounds), m_numRounds);
	m_avgElo = eloCount ? ((m_avgElo*10) + 5)/(eloCount*10) : 0;
	m_lastRound = m_maxRound + 1;

	for (unsigned i = 1; i <= m_maxRound; ++i)
	{
		unsigned ngames = gamesPerRound[i];
		unsigned round;

		if (ngames == 0)
			round = 1u << (i - 1);
		else if (mstl::is_pow_2(ngames))
			round = mstl::bf::msb_index(ngames) + 1;
		else
			round = mstl::bf::msb_index(ngames) + 2;

		m_lastRound = mstl::min(m_lastRound, round);
	}
}


void
TournamentTable::guessBestMode()
{
	mstl::bitset group1(m_playerMap.size());
	mstl::bitset group2(m_playerMap.size());

	unsigned minOppCount	= mstl::numeric_limits<unsigned>::max();
	unsigned maxOppCount = 0;

	for (PlayerMap::iterator i = m_playerMap.begin(); i != m_playerMap.end(); ++i)
	{
		Player const*					player		= i->second;
		Player::ClashList const&	clashList	= player->clashList;
		Player const*					opponent		= 0;

		unsigned oppCount = 0;

		bool isGroup1 = i == m_playerMap.begin();
		bool isGroup2 = false;

		for (unsigned k = 0; k < clashList.size(); ++k)
		{
			if (clashList[k]->opponent->player != opponent)
			{
				++oppCount;
				opponent = clashList[k]->opponent->player;

				if (group1.test(opponent->ranking))
					isGroup2 = true;
				else if (group2.test(opponent->ranking))
					isGroup1 = true;
			}
		}

		if (isGroup1 ^ isGroup2)
		{
			for (unsigned k = 0; k < clashList.size(); ++k)
				(isGroup1 ? group2 : group1).set(clashList[k]->opponent->player->ranking);

			(isGroup1 ? group1 : group2).set(player->ranking);
		}

		minOppCount = mstl::min(minOppCount, oppCount);
		maxOppCount = mstl::max(maxOppCount, oppCount);
	}

	bool isDisjoint = group1.disjunctive(group2) && group1.count() + group2.count() == m_playerMap.size();

	if (isDisjoint)
	{
		for (PlayerMap::iterator i = m_playerMap.begin(); i != m_playerMap.end(); ++i)
		{
			if (group2.test(i->second->ranking))
				i->second->group = 1;
		}
	}

	unsigned nplayers = m_playerMap.size();

	switch (m_event.type())
	{
		case event::Unknown:
		case event::Game:
		case event::Team:
			if (maxOppCount == 1)
				m_bestMode = Match;
			else if (nplayers < 2)
				m_bestMode = RankingList;
			else if (isDisjoint && group1.count() == group2.count())
				m_bestMode = Scheveningen;
			else if (minOppCount == 1 && maxOppCount == mstl::log2_ceil(nplayers))
				m_bestMode = Knockout;
			else if (nplayers <= 12)
				m_bestMode = Crosstable;
			else if (nplayers > 30)
				m_bestMode = Swiss;
			else if (m_numGames/nplayers < 5)
				m_bestMode = Swiss;
			else if (m_numGames < ((m_playerMap.size()*(nplayers - 1))/4))
				m_bestMode = Swiss;
			else if (nplayers <= 30)
				m_bestMode = Crosstable;
			else
				m_bestMode = RankingList;
			break;

		case event::Swiss:		m_bestMode = Swiss; break;
		case event::Match:		m_bestMode = Match; break;
		case event::Tournament:	m_bestMode = Crosstable; break;
		case event::Knockout:	m_bestMode = Knockout; break;
		case event::Simultan:	m_bestMode = RankingList; break;
		case event::Schev:		m_bestMode = Scheveningen; break;
	}

	if (m_missingRoundInfo && (m_bestMode == Knockout || m_bestMode == Swiss))
		m_bestMode = RankingList;

	m_mode = m_bestMode;
}


void
TournamentTable::computeTiebreaks()
{
	mstl::bitset used(m_playerMap.size());
	mstl::bitset particular(m_playerMap.size());

	unsigned nrounds = m_maxRound*m_maxSubround;

	for (PlayerMap::iterator i = m_playerMap.begin(); i != m_playerMap.end(); ++i)
	{
		Player* player = i->second;
		Player::ClashList const& clashList = player->clashList;

		unsigned lowestScore				= ::InvalidScore;
		unsigned secondLowestScore		= ::InvalidScore;
		unsigned highestScore			= 0;
		unsigned secondHighestScore	= 0;
		unsigned progressiveScore		= 0;

		used.reset();

		player->tiebreak[TraditionalScoring] = player->score[Traditional];

		for (unsigned k = 0; k < clashList.size(); ++k)
		{
			Clash const*	clash		= clashList[k];
			Player*			opponent	= clash->opponent->player;

			unsigned oppScore = opponent->score[Traditional];

			if (!used.test_and_set(opponent->ranking))
			{
				player->tiebreak[Buchholz] += oppScore;
				player->tiebreak[MedianBuchholz] += opponent->medianScore;
				player->tiebreak[ModifiedMedianBuchholz] += opponent->medianScore;

				secondLowestScore = mstl::min(secondLowestScore, lowestScore);
				secondHighestScore = mstl::max(secondHighestScore, highestScore);
				lowestScore = mstl::min(lowestScore, oppScore);
				highestScore = mstl::max(highestScore, oppScore);
			}

			switch (int(clash->result))
			{
				case result::White:
					progressiveScore += 2;
					player->tiebreak[SonnebornBerger] += mstl::mul2(oppScore);
					player->tiebreak[GamesWon] += 1;
					if (oppScore >= nrounds)
						player->tiebreak[KoyaSystem] += 2;
					if (clash->color == color::Black)
						player->tiebreak[GamesWonWithBlack] += 1;
					if (oppScore == player->score[Traditional])
					{
						player->tiebreak[ParticularResult] += 2;
						particular.set(player->ranking);
					}
					break;

				case result::Draw:
					progressiveScore += 1;
					player->tiebreak[SonnebornBerger] += oppScore;
					if (oppScore >= nrounds)
						player->tiebreak[KoyaSystem] += 1;
					if (oppScore == player->score[Traditional])
					{
						player->tiebreak[ParticularResult] += 1;
						particular.set(player->ranking);
					}
					break;
			}

			player->tiebreak[Progressive] += progressiveScore;
		}

		if (!particular.test(player->ranking))
			player->tiebreak[ParticularResult] = ::InvalidScore;

		if (lowestScore != ::InvalidScore)
		{
			player->tiebreak[MedianBuchholz] -= lowestScore + highestScore;

			if (nrounds >= 9 && secondLowestScore != ::InvalidScore)
				player->tiebreak[MedianBuchholz] -= secondLowestScore + secondHighestScore;

			if (player->score[Traditional] > nrounds)
			{
				player->tiebreak[ModifiedMedianBuchholz] -= lowestScore;

				if (nrounds >= 9 && secondLowestScore != ::InvalidScore)
					player->tiebreak[ModifiedMedianBuchholz] -= secondLowestScore;
			}
			else if (player->score[Traditional] < nrounds)
			{
				player->tiebreak[ModifiedMedianBuchholz] -= highestScore;

				if (nrounds >= 9)
					player->tiebreak[ModifiedMedianBuchholz] -= secondHighestScore;
			}
		}
	}

	for (PlayerMap::iterator i = m_playerMap.begin(); i != m_playerMap.end(); ++i)
	{
		Player* player = i->second;
		Player::ClashList const& clashList = player->clashList;

		mstl::bitset used(m_playerMap.size());

		for (unsigned k = 0; k < clashList.size(); ++k)
		{
			Player const* opponent = clashList[k]->opponent->player;

			if (!used.test_and_set(opponent->ranking))
			{
				player->refinedBuchholz[Buchholz] += opponent->tiebreak[Buchholz];
				player->refinedBuchholz[MedianBuchholz] += opponent->tiebreak[MedianBuchholz];
				player->refinedBuchholz[ModifiedMedianBuchholz] +=opponent->tiebreak[ModifiedMedianBuchholz];
			}
		}
	}
}


void
TournamentTable::sort(	ScoringSystem scoringSystem,
								TiebreakRules const& tiebreakRules,
								Order order,
								Mode mode)
{
	unsigned size = m_playerMap.size();
	Player* playerList[size];

	for (unsigned i = 0; i < size; ++i)
	{
		Player* player = m_playerMap.container()[i].second;

		playerList[i] = player;
		player->ranking = i;
		player->scoringSystem = scoringSystem;
	}

	::qsort(playerList, size, sizeof(Player*), ::cmpName);

	switch (m_order = order)
	{
		case Alphabetical:
			// already done
			break;

		case Rating:
			::qsort(playerList, size, sizeof(Player*), ::cmpRating);
			break;

		case Federation:
			::qsort(playerList, size, sizeof(Player*), ::cmpFederation);
			break;

		case Score:
			for (int i = U_NUMBER_OF(tiebreakRules) - 1; i >= 0; --i)
			{
				if (tiebreakRules[i] == RefinedBuchholz)
				{
					int k = i - 1;

					while (k >= 0 && tiebreakRules[k] >= LastBuchholz)
						--k;

					if (k >= 0)
					{
						::m_tiebreakRule = tiebreakRules[k];
						::qsort(playerList, size, sizeof(Player*), ::cmpRefinement);
					}
				}
				else if (tiebreakRules[i] != None)
				{
					::m_tiebreakRule = tiebreakRules[i];
					::qsort(playerList, size, sizeof(Player*), ::cmpTiebreak);
				}
			}

			::qsort(playerList, size, sizeof(Player*), mode == Knockout ? ::cmpProgress : ::cmpScore);
			break;
	}

	switch (int(mode))
	{
		case Match:
			{
				mstl::bitset used(size);
				Player* players[size];
				unsigned index = 0;

				for (unsigned i = 0; i < size; ++i)
				{
					Player* player = playerList[i];

					if (!used.test_and_set(player->ranking))
					{
						Player::ClashList const& clashList = player->clashList;

						players[index++] = player;

						if (!clashList.empty())
						{
							Player* opponent = clashList[0]->opponent->player;

							if (!used.test_and_set(opponent->ranking))
								players[index++] = opponent;
						}
					}
				}

				::memcpy(playerList, players, size*sizeof(Player*));
			}
			break;

		case Scheveningen:
			if (size && playerList[0]->group == 1)
			{
				for (unsigned i = 0; i < size; ++i)
					playerList[i]->group = 1 - playerList[i]->group;
			}

			::qsort(playerList, size, sizeof(Player*), ::cmpGroup);
			break;
	}

	m_orderMap.resize(size);

	for (unsigned i = 0; i < size; ++i)
		m_orderMap[i] = playerList[i]->ranking;

	for (unsigned i = 0; i < size; ++i)
	{
		playerList[i]->ranking = i;
		TRACE(debugPlayer(playerList[i]));
	}
}




unsigned
TournamentTable::computeEloPerformance(unsigned opponentAverage, unsigned percentage)
{
	//M_REQUIRE(percentage <= 100);

	unsigned performance = opponentAverage;

	if (percentage < 50)
		performance -= ::EloDiff[50 - percentage];
	else
		performance += ::EloDiff[percentage - 50];

	return performance;
}


unsigned
TournamentTable::computeDWZPerformance(unsigned opponentAverage, unsigned percentage)
{
	if (percentage == 0)
		return opponentAverage - 677;

	if (percentage == 100)
		return opponentAverage + 677;

	return computeEloPerformance(opponentAverage, percentage);
}


int
TournamentTable::ratingChange(int elo, int oppAvg, int percentage, int numGames)
{
	//M_REQUIRE(percentage <= 100);

	int diff = mstl::abs(elo - oppAvg);
	unsigned const* i = mstl::upper_bound(::EloDiff, ::EloDiff + U_NUMBER_OF(::EloDiff), unsigned(diff));
	int expected = i - ::EloDiff;

	if (elo > oppAvg)
		expected += 50;
	else
		expected = 50 - expected;

	int cutoff = (percentage > expected) ? 5 : -5;

	return ((percentage - expected)*numGames + cutoff)/10;
}


NamebasePlayer const*
TournamentTable::getPlayer(unsigned ranking) const
{
	//M_REQUIRE(ranking < countPlayers());

	for (unsigned i = 0; i < m_playerMap.size(); ++i)
	{
		Player const* player = m_playerMap.container()[i].second;

		if (player->ranking == ranking)
			return player->entry;
	}

	return 0; // should not be reached
}


int
TournamentTable::getPlayerId(unsigned ranking, color::ID& side) const
{
	//M_REQUIRE(ranking < countPlayers());

	for (unsigned i = 0; i < m_playerMap.size(); ++i)
	{
		Player const* player = m_playerMap.container()[i].second;

		if (player->ranking == ranking)
		{
			for (unsigned k = 0; k < player->clashList.size(); ++k)
			{
				if (player->clashList[k]->player == player)
				{
					side = player->clashList[k]->color;
					return player->clashList[k]->gameIndex;
				}
			}
		}
	}

	return -1;
}

// vi:set ts=3 sw=3:
