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

#include "m_assert.h"
#include "m_utility.h"

#include <string.h>

namespace db {
namespace material {
namespace si3 {

inline Signature::Signature() :u32(0) {}
inline Signature::Signature(uint32_t sig) :u32(sig) {}

} // namespace si3

inline
uint32_t
signature(Signature white, Signature black)
{
	return (uint32_t(black.value) << 16) | uint64_t(white.value);
}

inline
void
split(uint32_t signature, Signature& white, Signature& black)
{
	white.value = signature;
	black.value = signature >> 16;
}

inline
unsigned
count(Signature sig)
{
	return count(sig.part[0]) + count(sig.part[1]);
}


} // namespace material

namespace color {

inline ID opposite(ID color) { return ID(!color); }

inline bool isWhite(ID color) { return color == White; }
inline bool isBlack(ID color) { return color == Black; }

inline
ID
fromSide(char const* side)
{
	//M_REQUIRE(side);
	return *side == 'w' ? White : Black;
}

inline
char const*
printColor(ID color)
{
	return color == White ? "white" : "black";
}

} // namespace color

namespace sq {

inline Fyle fyle(Square square) { return Fyle(square & 7); }
inline Rank rank(Square square) { return Rank(square >> 3); }

inline ID make(Square fyle, Square rank) { return ID((rank << 3) | fyle); }

inline char printFyle(Square square)	{ return 'a' + fyle(square); }
inline char printFYLE(Square square)	{ return 'A' + fyle(square); }
inline char printRank(Square square)	{ return '1' + rank(square); }
inline char printRank(Rank rank)			{ return '1' + rank; }

inline ID rankToSquare(Rank rank) { return ID(rank << 3); }

inline db::color::ID color(ID s) { return db::color::ID(!((rank(s) + fyle(s)) & 1)); }

inline int fyleDistance(ID a, ID b) { return mstl::abs(int(fyle(a)) - int(fyle(b))); }
inline int rankDistance(ID a, ID b) { return mstl::abs(int(rank(a)) - int(rank(b))); }

inline int distance(ID a, ID b) { return mstl::max(fyleDistance(a, b), rankDistance(a, b)); }

inline ID flipFyle(ID s)	{ return make(Fyle(Rank8 - fyle(s)), rank(s)); }
inline ID flipRank(ID s)	{ return make(fyle(s), Rank(Rank8 - rank(s))); }

inline Rank flipRank(Rank rank)	{ return Rank(Rank8 - rank); }

inline Rank homeRank(db::color::ID color) { return color == db::color::White ? Rank1 : Rank8; }
inline Rank pawnRank(db::color::ID color) { return color == db::color::White ? Rank2 : Rank7; }

inline bool isAdjacent(ID a, ID b) { return distance(a, b) > 1; }

inline bool
isValid(char const* s)
{
	return 'a' <= s[0] && s[0] <= 'h' && '1' <= s[1] && s[1] <= '8';
}

inline
ID
make(char const* s)
{
	//M_REQUIRE(isValid(s));
	return sq::make(s[0] - 'a', s[1] - '1');
}

} //namespace sq

namespace piece {

inline db::color::ID color(ID piece)	{ return db::color::ID(piece >> 3); }
inline Type type(ID piece)					{ return Type(piece & 7); }

inline bool isWhite(ID piece) { return piece && !color(piece); }
inline bool isBlack(ID piece) { return color(piece); }

inline ID piece(Type type, db::color::ID color)		{ return ID(type | (color << 3)); }

inline bool canPromoteTo(Type type)
{
	return piece::Queen <= type && type <= piece::Knight;
}

inline
char
print(db::piece::ID piece)
{
	//M_ASSERT(piece <= 14);
	return " KQRBNP  kqrbnp"[piece];
};

inline
char
print(Type type)
{
	//M_ASSERT(type <= 6);
	return " KQRBNP"[type];
};

inline
char
printNumeric(Type type)
{
	static_assert(King == 1 && Queen == 2, "change numeric conversion");
	return '0' + unsigned(type) - 1;
}

} // namespace piece

namespace castling {

inline Index kingSideIndex(color::ID color)	{ return Index(WhiteKS | (color << 1)); }
inline Index queenSideIndex(color::ID color)	{ return Index(WhiteQS | (color << 1)); }

inline Rights kingSide(db::color::ID color)	{ return Rights(WhiteKingside << (color << 1)); }
inline Rights queenSide(db::color::ID color)	{ return Rights(WhiteQueenside << (color << 1)); }

inline
Rights
bothSides(db::color::ID color)
{
	return Rights((WhiteKingside | WhiteQueenside) << (color << 1));
}

} // namespace castling

namespace result {

inline
ID
opponent(ID result)
{
	switch (int(result))
	{
		case White: return Black;
		case Black: return White;
	}

	return result;
}


inline
ID
fromColor(color::ID color)
{
	return isWhite(color) ? White : Black;
}


inline
unsigned
value(ID result)
{
	static_assert(Unknown == 0, "reimplementation required");
	static_assert(White == 1, "reimplementation required");
	static_assert(Black == 2, "reimplementation required");
	static_assert(Draw == 3, "reimplementation required");
	static_assert(Lost == 4, "reimplementation required");

	static unsigned const Value[5] = { 0, 2, 0, 1, 0 };

	//M_ASSERT(size_t(result) < U_NUMBER_OF(Value));
	return Value[result];
}

} // namespace result

namespace pawns {

inline
bool
Side::test(uint8_t r, uint8_t f)
{
	static_assert(sizeof(rank) == 2, "reimplementation required");
	return ((1 << r) & ((1 << sq::Rank2) | (1 << sq::Rank3))) && rank[r - 1] & (1 << f);
}

inline
bool
Side::testRank2(uint8_t fyle)
{
	return rank[sq::Rank2 - 1] & (1 << fyle);
}

inline
void
Side::add(uint8_t s)
{
	static_assert(sizeof(rank) == 2, "reimplementation required");

	sq::Rank r = sq::rank(s);

	if ((1 << r) & ((1 << sq::Rank2) | (1 << sq::Rank3)))
		rank[r - 1] |= 1 << sq::fyle(s);
}

inline
void
Side::remove(uint8_t s)
{
	static_assert(sizeof(rank) == 2, "reimplementation required");

	sq::Rank r = sq::rank(s);

	if ((1 << r) & ((1 << sq::Rank2) | (1 << sq::Rank3)))
		rank[r - 1] &= ~(1 << sq::fyle(s));
}

inline
void
Side::move(uint8_t from, uint8_t to)
{
	remove(from);
	add(to);
}

} // namespace pawn

namespace tb {

inline bool isError(int score) { return score & 0xc0000000; }
inline bool isScore(int score) { return !isError(score); }

} // namespace tb

namespace rating {

inline
unsigned
convertUscfToElo(unsigned uscf)
{
	// source: http://www.glicko.net/ratings.rating.system.pdf

	if (uscf <= 720)
		return 0;

	if (uscf < 1970)
		return unsigned(1.6*(uscf - 720) + 0.5);

	return unsigned((uscf + 350)/1.16 + 0.5);
}


inline
unsigned
convertEloToUscf(unsigned elo)
{
	// source: http://www.glicko.net/ratings.rating.system.pdf

	if (elo == 0)
		return 0;

	if (elo < 2000)
		return 720 + unsigned(0.625*elo + 0.5);

	return unsigned(1.16*elo + 0.5) - 350;
}


inline
unsigned
convertEloToEcf(unsigned elo)
{
	if (elo <= 1250)
		return 0;

	if (elo <= 2325)
		return unsigned((elo - 1250)/5.0 + 0.5);

	return unsigned((elo - 600)/8.0 + 0.5);
}


inline
unsigned
convertEcfToElo(unsigned ecf)
{
	if (ecf == 0)
		return 0;

	if (ecf < 216)
		return ecf*5 + 1250;

	return ecf*8 + 600;
}

} // namespace rating

namespace tag {

inline bool isMandatory(ID tag) { return Event <= tag && tag <= Result; }

} // namespace tag

namespace nag {

inline bool isPrefix(ID nag)	{ return WithTheIdea <= nag && nag <= EditorsRemark; }
inline bool isInfix(ID nag)	{ return GoodMove <= nag && nag <= QuestionableMove; }
inline bool isSuffix(ID nag)	{ return nag && !isPrefix(nag) && !isInfix(nag); }

} // namespace nag

namespace sex {

inline char toChar(ID sex) { return sex == Male ? 'm' : (sex == Female ? 'f' : ' '); }

inline ID fromChar(char sex)
{
	return sex == 'm' ? Male : (sex == 'f' || sex == 'w' ? Female : Unspecified);
}

inline ID fromString(char const* sex) { return fromChar(*sex); }

} // namespace sex

namespace title {

inline bool contains(unsigned titles, title::ID title) { return titles & (1 << (title - 1)); }

inline
bool
containsFemaleTtile(unsigned titles)
{
	return titles & (Mask_WGM | Mask_WIM | Mask_WFM | Mask_WCM);
}

} // namespace title

namespace country {

inline unsigned count() { return LAST + 1; }


inline
bool
isGermanSpeakingCountry(Code code)
{
	return	code == Germany
			|| code == Austria
			|| code == Switzerland
			|| code == East_Germany
			|| code == West_Germany;
}

} // namespace country

namespace save {

inline bool isOk(State state) { return state == Ok || state == TooManyRoundNames; }

} // namespace save

namespace format {

inline bool isScidFormat(Type type) { return type & (Scid3 | Scid4); }

} // namespace format
} // namespace db

// vi:set ts=3 sw=3:
