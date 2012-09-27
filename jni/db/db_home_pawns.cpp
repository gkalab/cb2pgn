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

#include "db_home_pawns.h"

#include "db_move.h"

#include "m_assert.h"
#include "m_bit_functions.h"
#include "m_bitfield.h"
#include "m_string.h"
#include "m_utility.h"
#include "m_stdio.h"

#include <string.h>

using namespace db;


uint16_t HomePawnMask[2][64];


inline static uint8_t flipRank(db::Square s) { return sq::flipRank(sq::ID(s)); }


static void __attribute__((constructor)) initialize() { HomePawns::initialize(); }


HomePawns::HomePawns()
	:m_signature(Start)
	,m_shift(0)
{
	m_data.value = 0;
}


HomePawns::HomePawns(unsigned count, hp::Pawns data)
	:m_data(data)
	,m_signature(Start)
	,m_shift(0)
{
	//M_REQUIRE(count <= 16);

	unsigned endShift = mstl::mul4(count);

	for ( ; m_shift < endShift; m_shift += 4)
		m_signature &= ~(uint16_t(1) << ((m_data.value >> m_shift) & 15));
}


void
HomePawns::clear()
{
	m_signature = Start;
	m_shift = 0;
	m_data.value = 0;
}


void
HomePawns::update(uint16_t mask)
{
	uint16_t signature = m_signature & ~mask;

	if (signature == m_signature)
		return;

	//M_ASSERT(m_shift < 64);

	m_data.value |= uint64_t(mstl::bf::msb_index(m_signature & ~signature)) << m_shift;
	m_signature = signature;
	m_shift += 4;
}


void
HomePawns::move(Move const& move)
{
	if (move.pieceMoved() == piece::Pawn)
		update(::HomePawnMask[move.color()][move.from()]);

	if (move.captured() == piece::Pawn)
		update(::HomePawnMask[color::opposite(move.color())][move.to()]);
}


void
HomePawns::transpose()
{
	uint64_t data = 0;
	m_signature = Start;

	for (unsigned shift = 0; shift < m_shift; shift += 4)
	{
		uint16_t change = mstl::bf::reverse(uint8_t(m_data.value >> shift) & 15);

		m_signature &= ~(uint16_t(1) << change);
		data |= uint64_t(change) << shift;
	}
}


bool
HomePawns::checkIfReachable(uint16_t currentSig, hp::Pawns targetData, unsigned count)
{
	//M_ASSERT(currentSig);
	//M_ASSERT(count);

	uint16_t targetSig	= Start;
	unsigned maxShift		= mstl::mul4(count);

	for (unsigned shift = 0; shift < maxShift; shift += 4)
	{
		targetSig &= ~(uint16_t(1) << (unsigned(targetData.value >> shift) & 15));

		if (targetSig == currentSig)
			return true;

		// If the target signature contains a home pawn not in the current signature,
		// it could never match since pawns cannot reappear on their home rank.
		if ((targetSig & currentSig) != currentSig)
			return false;
	}

	return false;
}


mstl::string&
HomePawns::print(mstl::string& s) const
{
	static char const* Map = "HGFEDCBAhgfedcba";

	for (unsigned shift = 0; shift < m_shift; shift += 4)
	{
		s += Map[unsigned(m_data.value >> shift) & 15];

		if (shift < m_shift - 4)
			s += '-';
	}

	return s;
}


void
HomePawns::debug() const
{
	mstl::string s;
	::printf("%2d: %s\n", used(), print(s).c_str());
}


void
HomePawns::initialize()
{
	::memset(::HomePawnMask, 0, sizeof(::HomePawnMask));

	::HomePawnMask[color::White][sq::a2] = a2;
	::HomePawnMask[color::White][sq::b2] = b2;
	::HomePawnMask[color::White][sq::c2] = c2;
	::HomePawnMask[color::White][sq::d2] = d2;
	::HomePawnMask[color::White][sq::e2] = e2;
	::HomePawnMask[color::White][sq::f2] = f2;
	::HomePawnMask[color::White][sq::g2] = g2;
	::HomePawnMask[color::White][sq::h2] = h2;

	::HomePawnMask[color::Black][sq::a7] = a7;
	::HomePawnMask[color::Black][sq::b7] = b7;
	::HomePawnMask[color::Black][sq::c7] = c7;
	::HomePawnMask[color::Black][sq::d7] = d7;
	::HomePawnMask[color::Black][sq::e7] = e7;
	::HomePawnMask[color::Black][sq::f7] = f7;
	::HomePawnMask[color::Black][sq::g7] = g7;
	::HomePawnMask[color::Black][sq::h7] = h7;
}

// vi:set ts=3 sw=3:
