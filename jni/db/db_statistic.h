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

#ifndef _db_statistic_included
#define _db_statistic_included

#include "u_base.h"

namespace db {

class GameInfo;

class Statistic
{
public:

	enum Mode { Reset, Continue };

	Statistic();

	void clear();
	void add(GameInfo const& info);
	void compute(GameInfo* const* first, GameInfo* const* last, Mode mode);

	unsigned deleted;
	uint16_t minYear;
	uint16_t maxYear;
	uint16_t avgYear;
	uint16_t minElo;
	uint16_t maxElo;
	uint16_t avgElo;
	unsigned result[5];

private:

	void count(GameInfo const& info);

	double	m_sumYear;
	double	m_sumElo;
	unsigned	m_dateCount;
	unsigned	m_eloCount;
};

} // namespace db

#endif // _db_statistic_included

// vi:set ts=3 sw=3:
