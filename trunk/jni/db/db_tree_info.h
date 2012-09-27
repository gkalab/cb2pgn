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

#ifndef _db_tree_info_included
#define _db_tree_info_included

#include "db_move.h"
#include "db_eco.h"

#include "m_type_traits.h"

namespace db {

class GameInfo;
class NamebasePlayer;

class TreeInfo
{
public:

	TreeInfo();
	TreeInfo(Eco eco, Move const& move);

	struct Pair
	{
		Pair();

		operator bool() const;
		bool operator>(Pair const& p) const;

		void operator+=(Pair const& pair);

		double average() const;

		void add(unsigned value);

		unsigned	count;
		double	sum;
	};

	bool isLessThan(TreeInfo const& info, rating::Type ratingType, attribute::tree::ID column) const;

	Move const& move() const;
	Eco eco() const;
	unsigned frequency() const;
	unsigned score() const;
	unsigned draws() const;
	unsigned averageRating() const;
	int performance(rating::Type ratingType) const;
	uint16_t bestRating() const;
	uint16_t averageYear() const;
	uint16_t lastYear() const;
	NamebasePlayer const& bestPlayer() const;
	NamebasePlayer const& mostFrequentPlayer() const;

	Move& move();
	void setEco(Eco code);

	void add(GameInfo const& info, color::ID sideToMove, rating::Type ratingType);
	void add(TreeInfo const& info, rating::Type ratingType);

private:

	unsigned perMille(unsigned value) const;

	Move			m_move;				// move
	Eco			m_eco;				// ECO code for this move
	unsigned		m_frequency;		// frequency of this move
	unsigned		m_scoreCount[5];	// count result scores
	Pair			m_averageRating;	// average rating of side to move
	Pair			m_performance;		// opponent's rating
	Pair			m_averageYear;		// average year (if not zero)
	uint16_t		m_lastYear;			// year last played (if not zero)
	uint16_t		m_bestRating;		// best rating of side to move

	NamebasePlayer const* m_bestPlayer;
	NamebasePlayer const* m_mostFrequentPlayer;
};

} // namespace db

namespace mstl {

template <>
struct is_pod<db::TreeInfo>
{
	enum { value = is_pod<db::Move>::value & is_pod<db::Eco>::value };
};

template <>
struct is_movable<db::TreeInfo>
{
	enum { value = is_movable<db::Move>::value & is_movable<db::Eco>::value };
};

} // namespace mstl

#include "db_tree_info.ipp"

#endif // _db_tree_info_included

// vi:set ts=3 sw=3:
