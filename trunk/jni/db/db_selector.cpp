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

#include "db_selector.h"
#include "db_database.h"
#include "db_date.h"
#include "db_player.h"
#include "db_filter.h"

#include "m_string.h"
#include "m_bit_functions.h"
#include "m_utility.h"

#include <string.h>
#include <stdlib.h>

#ifdef MT_SAFE
# error "This implementation is not mt-safe"
#endif

using namespace db;
using namespace db::color;

static Database const* database = 0;
static int (*compareFunc)(void const*, void const*) = 0;


template <typename T>
inline static int compare(T lhs, T rhs) { return lhs - rhs; }
inline static int compare(Date const& lhs, Date const& rhs) { return Date::compare(lhs, rhs); }

inline static int
compare(mstl::string const& lhs, mstl::string const& rhs)
{
	return lhs==rhs; //sys::utf8::casecmp(lhs, rhs);
}

static int
reverseCompare(void const* lhs, void const* rhs)
{
	return -compareFunc(lhs, rhs);
}


namespace game {

static int
compRound(unsigned const* lhs, unsigned const* rhs)
{
	GameInfo const& l = database->gameInfo(*lhs);
	GameInfo const& r = database->gameInfo(*rhs);

	if (l.round() == r.round())
		return compare(l.subround(), r.subround());

	return compare(l.round(), r.round());
}


static int
compAcv(unsigned const* lhs, unsigned const* rhs)
{
	GameInfo const& l = database->gameInfo(*lhs);
	GameInfo const& r = database->gameInfo(*rhs);

	return compare(l.countAnnotations() + l.countComments() + l.countVariations(),
						r.countAnnotations() + r.countComments() + r.countVariations());
}


static int
compRatingWhite(unsigned const* lhs, unsigned const* rhs, rating::Type type)
{
	GameInfo const& l = database->gameInfo(*lhs);
	GameInfo const& r = database->gameInfo(*rhs);

	return compare(l.findRating(White, type), r.findRating(White, type));
}


static int
compRatingBlack(unsigned const* lhs, unsigned const* rhs, rating::Type type)
{
	GameInfo const& l = database->gameInfo(*lhs);
	GameInfo const& r = database->gameInfo(*rhs);

	return compare(l.findRating(Black, type), r.findRating(Black, type));
}


static int
compWhiteAny(unsigned const* lhs, unsigned const* rhs)
{
	GameInfo const& l = database->gameInfo(*lhs);
	GameInfo const& r = database->gameInfo(*rhs);

	uint16_t eloL = l.rating(White);
	uint16_t eloR = r.rating(White);

	if (eloL == 0) eloL = l.findElo(White);
	if (eloR == 0) eloR = r.findElo(White);

	return compare(eloL, eloR);
}


static int
compBlackAny(unsigned const* lhs, unsigned const* rhs)
{
	GameInfo const& l = database->gameInfo(*lhs);
	GameInfo const& r = database->gameInfo(*rhs);

	uint16_t eloL = l.rating(Black);
	uint16_t eloR = r.rating(Black);

	if (eloL == 0) eloL = l.findElo(Black);
	if (eloR == 0) eloR = r.findElo(Black);

	return compare(eloL, eloR);
}


static int
compAverageElo(unsigned const* lhs, unsigned const* rhs)
{
	GameInfo const& l = database->gameInfo(*lhs);
	GameInfo const& r = database->gameInfo(*rhs);

	int lw = l.findElo(White);
	int rw = r.findElo(White);
	int lb = l.findElo(Black);
	int rb = r.findElo(Black);
	int av = lw + lb - rw - rb;

	return -(av ? av : compare(mstl::max(lw, lb), mstl::max(rw, rb)));
}


static int
compRatingAverage(unsigned const* lhs, unsigned const* rhs, rating::Type type)
{
	GameInfo const& l = database->gameInfo(*lhs);
	GameInfo const& r = database->gameInfo(*rhs);

	int lw = l.findRating(White, type);
	int rw = r.findRating(White, type);
	int lb = l.findRating(Black, type);
	int rb = r.findRating(Black, type);
	int av = lw + lb - rw - rb;

	return -(av ? av : compare(mstl::max(lw, lb), mstl::max(rw, rb)));
}


static int
compAverageAny(unsigned const* lhs, unsigned const* rhs)
{
	GameInfo const& l = database->gameInfo(*lhs);
	GameInfo const& r = database->gameInfo(*rhs);

	int lw = l.rating(White);
	int rw = r.rating(White);
	int lb = l.rating(Black);
	int rb = r.rating(Black);

	if (lw == 0) lw = l.findElo(White);
	if (rw == 0) rw = r.findElo(White);
	if (lb == 0) lb = l.findElo(Black);
	if (rb == 0) rb = r.findElo(Black);

	int av = lw + lb - rw - rb;

	return -(av ? av : compare(mstl::max(lw, lb), mstl::max(rw, rb)));
}


#define DEF_COMPARE_RATING(Color,Type) \
	static int comp##Color##Type(unsigned const* lhs, unsigned const* rhs) \
	{ \
		return compRating##Color(lhs, rhs, rating::Type); \
	}
#define DEF_COMPARE(Type) \
	DEF_COMPARE_RATING(White,Type) \
	DEF_COMPARE_RATING(Black,Type) \
	\
	static int compAverage##Type(unsigned const* lhs, unsigned const* rhs) \
	{ \
		return compRatingAverage(lhs, rhs, rating::Type); \
	}

DEF_COMPARE(DWZ)
DEF_COMPARE(ECF)
DEF_COMPARE(ICCF)
DEF_COMPARE(IPS)
DEF_COMPARE(Rapid)
DEF_COMPARE(Rating)
DEF_COMPARE(USCF)

#undef DEF_COMPARE_RATING
#undef DEF_COMPARE


static int
compMaterial(unsigned const* lhs, unsigned const* rhs)
{
	material::Signature ml = database->gameInfo(*lhs).signature().material();
	material::Signature mr = database->gameInfo(*rhs).signature().material();

	unsigned l, r;

	// white ---------------------------------

	l = mstl::bf::count_bits(ml.part[0].queen);
	r = mstl::bf::count_bits(mr.part[0].queen);

	if (l < r)	return -1;
	if (l > r)	return  1;

	l = mstl::bf::count_bits(ml.part[0].rook);
	r = mstl::bf::count_bits(mr.part[0].rook);

	if (l < r)	return -1;
	if (l > r)	return  1;

	l = mstl::bf::count_bits(ml.part[0].minor);
	r = mstl::bf::count_bits(mr.part[0].minor);

	if (l < r)	return -1;
	if (l > r)	return  1;

	l = mstl::bf::count_bits(ml.part[0].pawn);
	r = mstl::bf::count_bits(mr.part[0].pawn);

	if (l < r)	return -1;
	if (l > r)	return  1;

	// black ---------------------------------

	l = mstl::bf::count_bits(ml.part[1].queen);
	r = mstl::bf::count_bits(mr.part[1].queen);

	if (l < r)	return -1;
	if (l > r)	return  1;

	l = mstl::bf::count_bits(ml.part[1].rook);
	r = mstl::bf::count_bits(mr.part[1].rook);

	if (l < r)	return -1;
	if (l > r)	return  1;

	l = mstl::bf::count_bits(ml.part[1].minor);
	r = mstl::bf::count_bits(mr.part[1].minor);

	if (l < r)	return -1;
	if (l > r)	return  1;

	l = mstl::bf::count_bits(ml.part[1].pawn);
	r = mstl::bf::count_bits(mr.part[1].pawn);

	if (l < r)	return -1;
	if (l > r)	return  1;

	// white ---------------------------------

	l = mstl::bf::count_bits(ml.part[0].bishop);
	r = mstl::bf::count_bits(mr.part[0].bishop);

	if (l < r)	return -1;
	if (l > r)	return  1;

	// black ---------------------------------

	l = mstl::bf::count_bits(ml.part[1].bishop);
	r = mstl::bf::count_bits(mr.part[1].bishop);

	if (l < r)	return -1;
	if (l > r)	return  1;

	return 0;
}


static int
compUserEco(unsigned const* lhs, unsigned const* rhs)
{
	GameInfo const& il = database->gameInfo(*lhs);
	GameInfo const& ir = database->gameInfo(*rhs);

	switch (il.idn())
	{
		case chess960::StandardIdn:
			switch (ir.idn())
			{
				case 0:								return -1;
				case chess960::StandardIdn:	return compare(il.userEco(), ir.userEco());
				default:								return int(chess960::StandardIdn) - int(ir.idn());
			}
			// not reached

		case 0:	return -int(ir.idn());
		default:	return ir.idn() ? int(il.idn()) - int(ir.idn()) : -1;
	}

	return 0; // not reached
}


static int
compOverview(unsigned const* lhs, unsigned const* rhs)
{
	GameInfo const& il = database->gameInfo(*lhs);
	GameInfo const& ir = database->gameInfo(*rhs);

	int rc = int(il.idn()) - int(ir.idn());

	if (rc)
		return rc;

	if (il.idn() != chess960::StandardIdn)
		return 0;

	return int(il.ecoKey()) - int(ir.ecoKey());
}


static int
compFlags(unsigned const* lhs, unsigned const* rhs)
{
	uint32_t fl = database->gameInfo(*lhs).flags();
	uint32_t fr = database->gameInfo(*rhs).flags();

	int nl = mstl::bf::count_bits(fl);
	int nr = mstl::bf::count_bits(fr);

	//M_ASSERT(nl >= 0);
	//M_ASSERT(nr >= 0);

	if (nl != nr)
		return nl - nr;

	return int(mstl::bf::lsb_index(fl)) - int(mstl::bf::lsb_index(fr));
}


static int
compWhiteCountry(unsigned const* lhs, unsigned const* rhs)
{
	return country::compare(database->gameInfo(*lhs).findFederation(White),
									database->gameInfo(*rhs).findFederation(White));
}


static int
compBlackCountry(unsigned const* lhs, unsigned const* rhs)
{
	return country::compare(database->gameInfo(*lhs).findFederation(Black),
									database->gameInfo(*rhs).findFederation(Black));
}


#define DEF_COMPARE(Func,Accessor) \
	static int \
	comp##Func(unsigned const* lhs, unsigned const* rhs) \
	{ \
		return compare(database->gameInfo(*lhs).Accessor, database->gameInfo(*rhs).Accessor); \
	}

DEF_COMPARE(WhitePlayer, playerName(White));
DEF_COMPARE(BlackPlayer, playerName(Black));
DEF_COMPARE(WhiteFideID, fideID(White));
DEF_COMPARE(BlackFideID, fideID(Black));
DEF_COMPARE(Event, event());
DEF_COMPARE(Site, site());
DEF_COMPARE(Date, date());
DEF_COMPARE(Result, result());
DEF_COMPARE(Annotator, annotator());
DEF_COMPARE(EventCountry, eventCountry());
DEF_COMPARE(EventType, eventType());
DEF_COMPARE(EventDate, eventDate());
DEF_COMPARE(Length, plyCount());
DEF_COMPARE(WhiteElo, findElo(White));
DEF_COMPARE(BlackElo, findElo(Black));
DEF_COMPARE(WhiteTitle, findTitle(White));
DEF_COMPARE(BlackTitle, findTitle(Black));
DEF_COMPARE(WhiteSex, findSex(White));
DEF_COMPARE(BlackSex, findSex(Black));
DEF_COMPARE(WhiteType, findPlayerType(White));
DEF_COMPARE(BlackType, findPlayerType(Black));
DEF_COMPARE(WhiteRatingType, findRatingType(White));
DEF_COMPARE(BlackRatingType, findRatingType(Black));
DEF_COMPARE(Eco, eco());
DEF_COMPARE(Idn, idn());
DEF_COMPARE(Position, position());
DEF_COMPARE(Termination, terminationReason());
DEF_COMPARE(Mode, eventMode());
DEF_COMPARE(TimeMode, timeMode());
DEF_COMPARE(Deleted, isDeleted());
DEF_COMPARE(Changed, isChanged());
DEF_COMPARE(StandardPosition, hasStandardPosition());
DEF_COMPARE(Chess960Position, hasChess960Position());
DEF_COMPARE(Promotion, hasPromotion());
DEF_COMPARE(UnderPromotion, hasUnderPromotion());
DEF_COMPARE(EngFlag, containsEnglishLanguage());
DEF_COMPARE(OthFlag, containsOtherLanguage());

} // namespace game

#undef DEF_COMPARE

namespace player {

static int
compRatingAny(unsigned const* lhs, unsigned const* rhs)
{
	NamebasePlayer const& l = database->player(*lhs);
	NamebasePlayer const& r = database->player(*rhs);

	uint16_t eloL = l.playerHighestRating();
	uint16_t eloR = r.playerHighestRating();

	if (eloL == 0) eloL = l.playerHighestElo();
	if (eloR == 0) eloR = r.playerHighestElo();

	return compare(eloL, eloR);
}


static int
compRatingLatestAny(unsigned const* lhs, unsigned const* rhs)
{
	NamebasePlayer const& l = database->player(*lhs);
	NamebasePlayer const& r = database->player(*rhs);

	uint16_t eloL = l.playerLatestRating();
	uint16_t eloR = r.playerLatestRating();

	if (eloL == 0) eloL = l.playerLatestElo();
	if (eloR == 0) eloR = r.playerLatestElo();

	return compare(eloL, eloR);
}


static int
compCountry(unsigned const* lhs, unsigned const* rhs)
{
	return country::compare(database->player(*lhs).findFederation(),
									database->player(*rhs).findFederation());
}


static int
compSex(unsigned const* lhs, unsigned const* rhs)
{
	NamebasePlayer const& l = database->player(*lhs);
	NamebasePlayer const& r = database->player(*rhs);

	sex::ID sexL = l.findSex();
	sex::ID sexR = r.findSex();

	if (sexL != sexR)
		return int(sexL) - int(sexR);

	return int(l.findType()) - int(r.findType());
}


#define DEF_COMPARE(Type) \
	static int compRating##Type(unsigned const* lhs, unsigned const* rhs) \
	{ \
		return compare(database->player(*lhs).playerHighestRating(rating::Type),  \
							database->player(*rhs).playerHighestRating(rating::Type)); \
	} \
	\
	static int compRatingLatest##Type(unsigned const* lhs, unsigned const* rhs) \
	{ \
		return compare(database->player(*lhs).playerLatestRating(rating::Type),  \
							database->player(*rhs).playerLatestRating(rating::Type)); \
	}

DEF_COMPARE(DWZ)
DEF_COMPARE(ECF)
DEF_COMPARE(ICCF)
DEF_COMPARE(IPS)
DEF_COMPARE(Rapid)
DEF_COMPARE(Rating)
DEF_COMPARE(USCF)

#undef DEF_COMPARE

#define DEF_COMPARE(Func,Accessor) \
	static int \
	comp##Func(unsigned const* lhs, unsigned const* rhs) \
	{ \
		return compare(database->player(*lhs).Accessor, database->player(*rhs).Accessor); \
	}

//DEF_COMPARE(Sex, findSex());
DEF_COMPARE(Name, name());
DEF_COMPARE(FideID, fideID());
DEF_COMPARE(Elo, playerHighestElo());
DEF_COMPARE(EloLatest, playerLatestElo());
DEF_COMPARE(RatingType, playerRatingType());
DEF_COMPARE(Title, findTitle());
DEF_COMPARE(Type, findType());
DEF_COMPARE(PlayerInfo, havePlayerInfo());
DEF_COMPARE(Frequency, frequency());

} // namespace player

namespace event {

#undef DEF_COMPARE

#define DEF_COMPARE(Func,Accessor) \
	static int \
	comp##Func(unsigned const* lhs, unsigned const* rhs) \
	{ \
		return compare(database->event(*lhs).Accessor, database->event(*rhs).Accessor); \
	}

DEF_COMPARE(Country, site()->country());
DEF_COMPARE(Site, site()->name());
DEF_COMPARE(Title, name());
DEF_COMPARE(Type, type());
DEF_COMPARE(Date, date());
DEF_COMPARE(Mode, eventMode());
DEF_COMPARE(TimeMode, timeMode());
DEF_COMPARE(Frequency, frequency());

} // namespace event

namespace site {

#undef DEF_COMPARE

#define DEF_COMPARE(Func,Accessor) \
	static int \
	comp##Func(unsigned const* lhs, unsigned const* rhs) \
	{ \
		return compare(database->site(*lhs).Accessor, database->site(*rhs).Accessor); \
	}

DEF_COMPARE(Country, country());
DEF_COMPARE(Site, name());
DEF_COMPARE(Frequency, frequency());

} // namespace site

namespace annotator {

#undef DEF_COMPARE

#define DEF_COMPARE(Func,Accessor) \
	static int \
	comp##Func(unsigned const* lhs, unsigned const* rhs) \
	{ \
		return compare(database->annotator(*lhs).Accessor, database->annotator(*rhs).Accessor); \
	}

DEF_COMPARE(Name, name());
DEF_COMPARE(Frequency, frequency());

} // namespace annotator


void
Selector::finish(Database const& db, unsigned numEntries, order::ID order, Compar compFunc)
{
	//M_ASSERT(m_map.size() <= numEntries);
	//M_ASSERT(compFunc);

	if (numEntries != m_map.size())
	{
		unsigned k = m_map.size();

		m_map.resize(numEntries);

		for (unsigned i = k; i < numEntries; ++i)
			m_map[i] = i;
	}

	::database = &db;

	if (order == order::Descending)
	{
		::compareFunc = compFunc;
		compFunc = ::reverseCompare;
	}

	::qsort(m_map.begin(), m_map.size(), sizeof(Map::value_type), compFunc);
}


void
Selector::sort(Database const& db, attribute::game::ID attr, order::ID order, rating::Type ratingType)
{
	typedef int(*InfoCompar)(unsigned const*, unsigned const*);
	typedef int(*Compar)(void const*, void const*);

	InfoCompar func = 0;

	switch (attr)
	{
		case attribute::game::Number:						break;
		case attribute::game::WhitePlayer:				func = game::compWhitePlayer; break;
		case attribute::game::BlackPlayer:				func = game::compBlackPlayer; break;
		case attribute::game::WhiteFideID:				func = game::compWhiteFideID; break;
		case attribute::game::BlackFideID:				func = game::compBlackFideID; break;
		case attribute::game::Event:						func = game::compEvent; break;
		case attribute::game::Site:						func = game::compSite; break;
		case attribute::game::Date:						func = game::compDate; break;
		case attribute::game::Result:						func = game::compResult; break;
		case attribute::game::Round:						func = game::compRound; break;
		case attribute::game::Annotator:					func = game::compAnnotator; break;
		case attribute::game::WhiteElo:					func = game::compWhiteElo; break;
		case attribute::game::BlackElo:					func = game::compBlackElo; break;
		case attribute::game::AverageElo:				func = game::compAverageElo; break;
		case attribute::game::WhiteRatingType:			func = game::compWhiteRatingType; break;
		case attribute::game::BlackRatingType:			func = game::compBlackRatingType; break;
		case attribute::game::WhiteCountry:				func = game::compWhiteCountry; break;
		case attribute::game::BlackCountry:				func = game::compBlackCountry; break;
		case attribute::game::WhiteTitle:				func = game::compWhiteTitle; break;
		case attribute::game::BlackTitle:				func = game::compBlackTitle; break;
		case attribute::game::WhiteType:					func = game::compWhiteType; break;
		case attribute::game::BlackType:					func = game::compBlackType; break;
		case attribute::game::WhiteSex:					func = game::compWhiteSex; break;
		case attribute::game::BlackSex:					func = game::compBlackSex; break;
		case attribute::game::EventType:					func = game::compEventType; break;
		case attribute::game::EventCountry:				func = game::compEventCountry; break;
		case attribute::game::EventDate:					func = game::compEventDate; break;
		case attribute::game::Length:						func = game::compLength; break;
		case attribute::game::Flags:						func = game::compFlags; break;
		case attribute::game::Material:					func = game::compMaterial; break;
		case attribute::game::Idn:							func = game::compIdn; break;
		case attribute::game::Position:					func = game::compPosition; break;
		case attribute::game::Acv:							func = game::compAcv; break;
		case attribute::game::CommentEngFlag:			func = game::compEngFlag; break;
		case attribute::game::CommentOthFlag:			func = game::compOthFlag; break;
		case attribute::game::Termination:				func = game::compTermination; break;
		case attribute::game::Mode:						func = game::compMode; break;
		case attribute::game::TimeMode:					func = game::compTimeMode; break;
		case attribute::game::Deleted:					func = game::compDeleted; break;
		case attribute::game::Changed:					func = game::compChanged; break;
		case attribute::game::StandardPosition:		func = game::compStandardPosition; break;
		case attribute::game::Chess960Position:		func = game::compChess960Position; break;
		case attribute::game::Promotion:					func = game::compPromotion; break;
		case attribute::game::UnderPromotion:			func = game::compUnderPromotion; break;
		case attribute::game::Overview:					func = game::compOverview; break;

		case attribute::game::Eco:
			if (db.format() == format::Scidb)
				func = game::compUserEco;
			else
				func = game::compEco;
			break;

		case attribute::game::WhiteRating:
			switch (ratingType)
			{
				case rating::DWZ:		func = game::compWhiteDWZ; break;
				case rating::ECF:		func = game::compWhiteECF; break;
				case rating::Elo:		func = game::compWhiteElo; break;
				case rating::ICCF:	func = game::compWhiteICCF; break;
				case rating::IPS:		func = game::compWhiteIPS; break;
				case rating::Rapid:	func = game::compWhiteRapid; break;
				case rating::Rating:	func = game::compWhiteRating; break;
				case rating::USCF:	func = game::compWhiteUSCF; break;
				case rating::Any:		func = game::compWhiteAny; break;
			}
			break;

		case attribute::game::BlackRating:
			switch (ratingType)
			{
				case rating::DWZ:		func = game::compBlackDWZ; break;
				case rating::ECF:		func = game::compBlackECF; break;
				case rating::Elo:		func = game::compBlackElo; break;
				case rating::ICCF:	func = game::compBlackICCF; break;
				case rating::IPS:		func = game::compBlackIPS; break;
				case rating::Rapid:	func = game::compBlackRapid; break;
				case rating::Rating:	func = game::compBlackRating; break;
				case rating::USCF:	func = game::compBlackUSCF; break;
				case rating::Any:		func = game::compBlackAny; break;
			}
			break;

		case attribute::game::AverageRating:
			switch (ratingType)
			{
				case rating::DWZ:		func = game::compAverageDWZ; break;
				case rating::ECF:		func = game::compAverageECF; break;
				case rating::Elo:		func = game::compAverageElo; break;
				case rating::ICCF:	func = game::compAverageICCF; break;
				case rating::IPS:		func = game::compAverageIPS; break;
				case rating::Rapid:	func = game::compAverageRapid; break;
				case rating::Rating:	func = game::compAverageRating; break;
				case rating::USCF:	func = game::compAverageUSCF; break;
				case rating::Any	:	func = game::compAverageAny; break;
			}
			break;

		case attribute::game::WhiteRating1:
		case attribute::game::BlackRating1:
		case attribute::game::WhiteRating2:
		case attribute::game::BlackRating2:
		case attribute::game::Opening:
		case attribute::game::Variation:
		case attribute::game::SubVariation:
		case attribute::game::InternalEco:
			return;
	}

	if (func)
		finish(db, db.countGames(), order, reinterpret_cast<Compar>(func));
	else
		m_map.release();
}


void
Selector::sort(Database const& db, attribute::player::ID attr, order::ID order, rating::Type ratingType)
{
	typedef int(*InfoCompar)(unsigned const*, unsigned const*);
	typedef int(*Compar)(void const*, void const*);

	InfoCompar func = 0;

	switch (attr)
	{
		case attribute::player::Name:				func = player::compName; break;
		case attribute::player::FideID:			func = player::compFideID; break;
		case attribute::player::Sex:				func = player::compSex; break;
		case attribute::player::EloHighest:		func = player::compElo; break;
		case attribute::player::EloLatest:		func = player::compEloLatest; break;
		case attribute::player::RatingType:		func = player::compRatingType; break;
		case attribute::player::Country:			func = player::compCountry; break;
		case attribute::player::Title:			func = player::compTitle; break;
		case attribute::player::Type:				func = player::compType; break;
		case attribute::player::PlayerInfo:		func = player::compPlayerInfo; break;
		case attribute::player::Frequency:		func = player::compFrequency; break;

		case attribute::player::RatingHighest:
			switch (ratingType)
			{
				case rating::DWZ:		func = player::compRatingDWZ; break;
				case rating::ECF:		func = player::compRatingECF; break;
				case rating::Elo:		func = player::compElo; break;
				case rating::ICCF:	func = player::compRatingICCF; break;
				case rating::IPS:		func = player::compRatingIPS; break;
				case rating::Rapid:	func = player::compRatingRapid; break;
				case rating::Rating:	func = player::compRatingRating; break;
				case rating::USCF:	func = player::compRatingUSCF; break;
				case rating::Any:		func = player::compRatingAny; break;
			}
			break;

		case attribute::player::RatingLatest:
			switch (ratingType)
			{
				case rating::DWZ:		func = player::compRatingLatestDWZ; break;
				case rating::ECF:		func = player::compRatingLatestECF; break;
				case rating::Elo:		func = player::compEloLatest; break;
				case rating::ICCF:	func = player::compRatingLatestICCF; break;
				case rating::IPS:		func = player::compRatingLatestIPS; break;
				case rating::Rapid:	func = player::compRatingLatestRapid; break;
				case rating::Rating:	func = player::compRatingLatestRating; break;
				case rating::USCF:	func = player::compRatingLatestUSCF; break;
				case rating::Any:		func = player::compRatingLatestAny; break;
			}
			break;

		case attribute::player::Rating1:
		case attribute::player::Rating2:
		case attribute::player::DateOfBirth:
		case attribute::player::DateOfDeath:
		case attribute::player::DsbID:
		case attribute::player::EcfID:
		case attribute::player::IccfID:
		case attribute::player::ViafID:
		case attribute::player::PndID:
		case attribute::player::ChessgComLink:
		case attribute::player::WikiLink:
		case attribute::player::Aliases:
			return;
	}

	finish(db, db.countPlayers(), order, reinterpret_cast<Compar>(func));
}


void
Selector::sort(Database const& db, attribute::event::ID attr, order::ID order)
{
	typedef int(*InfoCompar)(unsigned const*, unsigned const*);
	typedef int(*Compar)(void const*, void const*);

	InfoCompar func = 0;

	switch (attr)
	{
		case attribute::event::Country:		func = ::event::compCountry; break;
		case attribute::event::Site:			func = ::event::compSite; break;
		case attribute::event::Title:			func = ::event::compTitle; break;
		case attribute::event::Type:			func = ::event::compType; break;
		case attribute::event::Date:			func = ::event::compDate; break;
		case attribute::event::Mode:			func = ::event::compMode; break;
		case attribute::event::TimeMode:		func = ::event::compTimeMode; break;
		case attribute::event::Frequency:	func = ::event::compFrequency; break;
		case attribute::event::LastColumn:	return;
	}

	finish(db, db.countEvents(), order, reinterpret_cast<Compar>(func));
}


void
Selector::sort(Database const& db, attribute::site::ID attr, order::ID order)
{
	typedef int(*InfoCompar)(unsigned const*, unsigned const*);
	typedef int(*Compar)(void const*, void const*);

	InfoCompar func = 0;

	switch (attr)
	{
		case attribute::site::Country:		func = ::site::compCountry; break;
		case attribute::site::Site:			func = ::site::compSite; break;
		case attribute::site::Frequency:		func = ::site::compFrequency; break;
		case attribute::site::LastColumn:	return;
	}

	finish(db, db.countSites(), order, reinterpret_cast<Compar>(func));
}


void
Selector::sort(Database const& db, attribute::annotator::ID attr, order::ID order)
{
	typedef int(*InfoCompar)(unsigned const*, unsigned const*);
	typedef int(*Compar)(void const*, void const*);

	InfoCompar func = 0;

	switch (attr)
	{
		case attribute::annotator::Name:			func = annotator::compName; break;
		case attribute::annotator::Frequency:	func = annotator::compFrequency; break;
		case attribute::annotator::LastColumn:	return;
	}

	//M_ASSERT(func);

	finish(db, db.countAnnotators(), order, reinterpret_cast<Compar>(func));
}


void
Selector::reverse(Database const& db)
{
	if (m_map.size() == 0)
	{
		unsigned n = db.countGames();

		m_map.resize(n);

		for (unsigned i = 0; i < n; ++i)
			m_map[i] = n - i - 1;
	}
	else
	{
		unsigned n = m_map.size();
		unsigned middle = n/2;

		for (unsigned i = 0; i < middle; ++i)
			mstl::swap(m_map[i], m_map[n - i - 1]);
	}
}


unsigned
Selector::find(unsigned number) const
{
	unsigned n = m_list.size();

	if (number >= m_list.size() || m_list[number] != number)
	{
		for (unsigned i = 0; i < n; ++i)
		{
			if (m_list[i] == number)
				return i;
		}
	}

	return number;
}


int
Selector::findPlayer(Database const& db, mstl::string const& name) const
{
	unsigned n = m_map.size();

	if (n == 0)
	{
		n = db.countPlayers();

		for (unsigned i = 0; i < n; ++i)
		{
			if (db.player(i).name() == name)
				return i;
		}
	}
	else
	{
		for (unsigned i = 0; i < n; ++i)
		{
			if (db.player(m_map[i]).name() == name)
				return i;
		}
	}

	return -1;
}


int
Selector::findEvent(Database const& db, mstl::string const& name) const
{
	unsigned n = m_map.size();

	if (n == 0)
	{
		n = db.countEvents();

		for (unsigned i = 0; i < n; ++i)
		{
			if (db.event(i).name() == name)
				return i;
		}
	}
	else
	{
		for (unsigned i = 0; i < n; ++i)
		{
			if (db.event(m_map[i]).name() == name)
				return i;
		}
	}

	return -1;
}


int
Selector::findSite(Database const& db, mstl::string const& name) const
{
	unsigned n = m_map.size();

	if (n == 0)
	{
		n = db.countSites();

		for (unsigned i = 0; i < n; ++i)
		{
			if (db.site(i).name() == name)
				return i;
		}
	}
	else
	{
		for (unsigned i = 0; i < n; ++i)
		{
			if (db.site(m_map[i]).name() == name)
				return i;
		}
	}

	return -1;
}


int
Selector::findAnnotator(Database const& db, mstl::string const& name) const
{
	unsigned n = m_map.size();

	if (n == 0)
	{
		n = db.countAnnotators();

		for (unsigned i = 0; i < n; ++i)
		{
			if (db.annotator(i).name() == name)
				return i;
		}
	}
	else
	{
		for (unsigned i = 0; i < n; ++i)
		{
			if (db.annotator(m_map[i]).name() == name)
				return i;
		}
	}

	return -1;
}


int
Selector::searchPlayer(Database const& db, mstl::string const& name) const
{
	unsigned n = m_map.size();

	if (n == 0)
	{
		n = db.countPlayers();

		for (unsigned i = 0; i < n; ++i)
		{
			if (strncmp(db.player(i).name(), name, name.size()))
				return i;
		}
	}
	else
	{
		for (unsigned i = 0; i < n; ++i)
		{
			if (strncmp(db.player(m_map[i]).name(), name, name.size()))
				return i;
		}
	}

	return -1;
}


int
Selector::searchEvent(Database const& db, mstl::string const& name) const
{
	unsigned n = m_map.size();

	if (n == 0)
	{
		n = db.countEvents();

		for (unsigned i = 0; i < n; ++i)
		{
			if (strncmp(db.event(i).name(), name, name.size()))
				return i;
		}
	}
	else
	{
		for (unsigned i = 0; i < n; ++i)
		{
			if (strncmp(db.event(m_map[i]).name(), name, name.size()))
				return i;
		}
	}

	return -1;
}


int
Selector::searchSite(Database const& db, mstl::string const& name) const
{
	unsigned n = m_map.size();

	if (n == 0)
	{
		n = db.countSites();

		for (unsigned i = 0; i < n; ++i)
		{
			if (strncmp(db.site(i).name(), name, name.size()))
				return i;
		}
	}
	else
	{
		for (unsigned i = 0; i < n; ++i)
		{
			if (strncmp(db.site(m_map[i]).name(), name, name.size()))
				return i;
		}
	}

	return -1;
}

int
Selector::searchAnnotator(Database const& db, mstl::string const& name) const
{
	unsigned n = m_map.size();

	if (n == 0)
	{
		n = db.countAnnotators();

		for (unsigned i = 0; i < n; ++i)
		{
			if (strncmp(db.annotator(i).name(), name, name.size()))
				return i;
		}
	}
	else
	{
		for (unsigned i = 0; i < n; ++i)
		{
			if (strncmp(db.annotator(m_map[i]).name(), name, name.size()))
				return i;
		}
	}
	for (unsigned i = 0; i < n; ++i)
	{
		if (strncmp(db.annotator(m_map[i]).name(), name, name.size()))
			return i;
	}

	return -1;
}


void
Selector::update(Filter const& filter)
{
	if (!m_map.empty())
	{
		if (filter.size() >= m_map.size())
		{
			m_list.resize(filter.count());

			Map::iterator list = m_list.begin();

			for (unsigned i = 0, n = m_map.size(); i < n; ++i)
			{
				unsigned k = m_map[i];

				if (filter.contains(k))
					*list++ = k;
			}

			//M_ASSERT(list == m_list.end());
		}
		else
		{
			m_map.clear();
			m_list.release();
		}
	}
	else if (filter.isComplete())
	{
		m_list.release();
	}
	else
	{
		m_list.resize(filter.count());

		if (!m_list.empty())
		{
			Map::iterator list = m_list.begin();

			for (unsigned i = 0, n = filter.size(); i < n; ++i)
			{
				if (filter.contains(i))
					*list++ = i;
			}

			//M_ASSERT(list == m_list.end());
		}
	}
}


void
Selector::update()
{
	if (!m_map.empty())
	{
		m_list.resize(m_map.size());

		Map::iterator list = m_list.begin();

		for (unsigned i = 0, n = m_map.size(); i < n; ++i)
			*list++ = m_map[i];
	}
	else
	{
		m_list.release();
	}
}


void
Selector::update(unsigned newSize)
{
	if (newSize < m_map.size())
	{
		m_map.clear();
		m_list.release();
	}
}


void
Selector::swap(Selector& selector)
{
	m_map.swap(selector.m_map);
	m_list.swap(selector.m_list);
}

// vi:set ts=3 sw=3:
