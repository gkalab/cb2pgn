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

#ifndef _db_home_pawns_included
#define _db_home_pawns_included

#include "db_common.h"

namespace mstl { class string; }

namespace db {

class Move;

class HomePawns
{
public:

	HomePawns();
	HomePawns(unsigned count, hp::Pawns data);

	unsigned used() const;
	uint16_t signature() const;
	hp::Pawns data() const;

	void move(Move const& move);
	void clear();
	void transpose();

	mstl::string& print(mstl::string& s) const;
	void debug() const;

	static bool isReachable(uint16_t currentSig, hp::Pawns targetData, unsigned count);
	static void initialize();

private:

	// coincides with Scid
	enum
	{
		a2 = 0x8000,
		b2 = 0x4000,
		c2 = 0x2000,
		d2 = 0x1000,
		e2 = 0x0800,
		f2 = 0x0400,
		g2 = 0x0200,
		h2 = 0x0100,

		a7 = 0x0080,
		b7 = 0x0040,
		c7 = 0x0020,
		d7 = 0x0010,
		e7 = 0x0008,
		f7 = 0x0004,
		g7 = 0x0002,
		h7 = 0x0001,
	};

	static uint16_t const Start =	a2 | b2 | c2 | d2 | e2 | f2 | g2 | h2
										 | a7 | b7 | c7 | d7 | e7 | f7 | g7 | h7;

	void update(uint16_t  mask);

	static bool checkIfReachable(uint16_t currentSig, hp::Pawns targetData, unsigned count);

	unsigned		m_used;
	hp::Pawns	m_data;
	uint16_t		m_signature;	// remember last signature
	unsigned		m_shift;
};

} // namespace db

namespace mstl {

template <typename T> struct is_pod;
template <> struct is_pod<db::HomePawns> { enum { value = 1 }; };

} // namespace mstl

#include "db_home_pawns.ipp"

#endif // _db_home_pawns_included

// vi:set ts=3 sw=3:
