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

#include "db_signature.h"
#include "db_pawn_progress.h"
#include "db_home_pawns.h"

#include "m_hash.h"
#include "m_algorithm.h"
#include "m_bit_functions.h"
#include "m_string.h"
#include "m_utility.h"
#include "m_stdio.h"

#define USE_HASH

using namespace db;
using namespace db::color;


enum { BitLength	= 5 };
enum { BitShift	= 8 - BitLength };
enum { BitMask		= (1 << BitLength) - 1 };


#ifdef USE_HASH

typedef mstl::hash<uint32_t,bool> Hash;

#if __GNUC_PREREQ(4,7)
__attribute__((init_priority(65534)))
#endif
Hash pawnStructureHash;

static void
#if __GNUC_PREREQ(4,7)
__attribute__((constructor(65535)))
#else
__attribute__((constructor))
#endif
initialize() { Signature::initialize(); }

#endif


inline
static uint32_t
hashL(pawns::Side side)
{
	return uint32_t(side.rank[0] & BitMask) << BitLength | uint32_t(side.rank[1] & BitMask);
}


inline
static uint32_t
hashL(pawns::Side lhs, pawns::Side rhs)
{
	return hashL(lhs) << 2*BitLength | hashL(rhs);
}


inline
static uint32_t
hashR(pawns::Side pawns)
{
	// IMPORTANT NOTE:
	// (pos.rank[0] >> BitShift) << BitLength | pos.rank[1] >> BitShift
	// does not work!

	return	uint32_t(mstl::bf::reverse(pawns.rank[0]) & BitMask) << BitLength
			 | uint32_t(mstl::bf::reverse(pawns.rank[1]) & BitMask);
}


inline
static uint32_t
hashR(pawns::Side lhs, pawns::Side rhs)
{
	return hashR(lhs) << 2*BitLength | hashR(rhs);
}


inline
static bool
isReachablePawnStructure(uint32_t hash)
{
	// our hash does not include pawn progressions to empty ranks (no pawns on 2nd and 3rd rank)
	if ((hash & ((1 << 2*BitLength) - 1)) == 0)
		return true;

#ifdef USE_HASH

	return ::pawnStructureHash.has_key(hash);

#else

	return mstl::binary_search(PawnProgressTable,
										PawnProgressTable + U_NUMBER_OF(PawnProgressTable),
										hash) != PawnProgressTable + U_NUMBER_OF(PawnProgressTable);

#endif
}


bool
Signature::isReachablePawnStructure(pawns::Side lhs, pawns::Side rhs)
{
	if (rhs.rankValue == 0)
		return true;

	return ::isReachablePawnStructure(hashL(lhs, rhs)) && ::isReachablePawnStructure(hashR(lhs, rhs));
}


template <typename T>
inline static bool isGreaterOrEq(T start, T target) { return !(target & ~start); }


bool
Signature::isReachableFinalMaterial(Signature const& target) const
{
	// M_REQUIRE(!hasPromotion() || target.hasPromotion());
	// M_REQUIRE(!hasUnderPromotion() || target.hasUnderPromotion());

	if (target.hasPromotion())
	{
		if (target.hasUnderPromotion())
		{
			// we can only check pawn counts
			return	::isGreaterOrEq(m_material.part[White].pawn, target.m_material.part[White].pawn)
					&& ::isGreaterOrEq(m_material.part[Black].pawn, target.m_material.part[Black].pawn);
		}

		// we cannot check queen counts
		return
			::isGreaterOrEq(m_material.part[White].butNotQueen, target.m_material.part[White].butNotQueen)
		&& ::isGreaterOrEq(m_material.part[Black].butNotQueen, target.m_material.part[Black].butNotQueen);
	}

	// cannot reach target position if target has more material than we have
	return ::isGreaterOrEq(m_material.value, target.m_material.value);
}


bool
Signature::isReachableFinalPosition(Signature const& target, uint16_t currentHpSig) const
{
	// cannot reach target position if we have more promotions than target
	if (hasPromotion() && !target.hasPromotion())
		return false;

	// cannot reach target position if we have more under-promotions than target
	if (hasUnderPromotion() && !target.hasUnderPromotion())
		return false;

	if (!isReachableFinalMaterial(target))
		return false;

	if (!HomePawns::isReachable(currentHpSig, target.m_homePawns, target.hpCount()))
		return false;

	// check whether pawn structure is reachable
	return	isReachablePawnStructure(m_progress.side[White], target.m_progress.side[White])
			&& isReachablePawnStructure(m_progress.side[Black], target.m_progress.side[Black]);
}


void
Signature::transpose()
{
	m_castling = castling::transpose(m_castling);

	m_progress.side[White].rank[0] = mstl::bf::reverse(m_progress.side[White].rank[0]);
	m_progress.side[White].rank[1] = mstl::bf::reverse(m_progress.side[White].rank[1]);

	m_progress.side[Black].rank[0] = mstl::bf::reverse(m_progress.side[Black].rank[0]);
	m_progress.side[Black].rank[1] = mstl::bf::reverse(m_progress.side[Black].rank[1]);

	HomePawns hp(hpCount(), m_homePawns);
	hp.transpose();
	m_homePawns = hp.data();
}


void
Signature::debug(unsigned spaces) const
{
	HomePawns homePawns(hpCount(), m_homePawns);

	mstl::string material;
	mstl::string castling;
	mstl::string progress[2];
	mstl::string s;

	pawns::print(m_progress, White, progress[White]);
	pawns::print(m_progress, Black, progress[Black]);
	material::print(m_material, material);
	castling::print(castling::Rights(m_castling), castling);
	homePawns.print(s);

	::printf("%*cMaterial:          %s\n", spaces, ' ', material.c_str());
	::printf("%*cPromotions:        %u\n", spaces, ' ', unsigned(m_promotions));
	::printf("%*cUnder-promotions:  %u\n", spaces, ' ', unsigned(m_underPromotions));
	::printf("%*cCastling:          %s\n", spaces, ' ', castling.c_str());
	::printf("%*cPawn progress (W): %s\n", spaces, ' ', progress[color::White].c_str());
	::printf("%*cPawn progress (B): %s\n", spaces, ' ', progress[color::Black].c_str());
	::printf("%*cHome pawn count:   %u\n", spaces, ' ', hpCount());
	::printf("%*cHome pawns:        %s\n", spaces, ' ', s.c_str());

	::fflush(stdout);
}


void
Signature::initialize()
{
#ifdef USE_HASH
	if (!pawnStructureHash.empty())
		return;

	// 500.0/Load
	// --------------------------------
	// collisions: 8.326
	// max. bucket length: 2
	// average bucket length: 1.28
	// storage size: 2.549.596 (~ 2.5 MB)

	// 250.0/Load
	// --------------------------------
	// collisions: 15.001
	// max. bucket length: 4
	// average bucket length: 1.66
	// storage size: 1.501.020 (~ 1.5 MB)

	::pawnStructureHash.rebuild(unsigned((U_NUMBER_OF(PawnProgressTable))*(250.0/Hash::Load)));

	for (unsigned i = 0; i < U_NUMBER_OF(PawnProgressTable); ++i)
		::pawnStructureHash.insert_unique(PawnProgressTable[i], true);

//	::fprintf(stderr, "collisions: %u\n", pawnStructureHash.count_collisions());
//	::fprintf(stderr, "max. bucket length: %u\n", pawnStructureHash.max_bucket_length());
//	::fprintf(stderr, "average bucket length: %0.2f\n", pawnStructureHash.average_bucket_length());
//	::fprintf(stderr, "storage size: %u\n", pawnStructureHash.storage_size());

#endif
}

// vi:set ts=3 sw=3:
