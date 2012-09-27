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

#include "db_line.h"
#include "db_board.h"
#include "db_move.h"

#include "m_string.h"
#include "m_utility.h"
#include "m_stdio.h"
#include "m_assert.h"

using namespace db;


Line&
Line::transpose(Line& dst) const
{
	//M_REQUIRE(dst.length == length);

	for (unsigned i = 0; i < length; ++i)
	{
		Move m(moves[i]);
		m.transpose();
		const_cast<uint16_t*>(dst.moves)[i] = m.index();
	}

	return dst;
}


mstl::string&
Line::print(mstl::string& result, encoding::CharSet charSet) const
{
	Board board = Board::standardBoard();

	for (unsigned i = 0; i < length; ++i)
	{
		Move m = board.makeMove(moves[i]);

		if (i > 0)
			result += ' ';

		if ((i & 1) == 0)
			result.format("%u.", mstl::div2(i) + 1);

		board.prepareForPrint(m);
		m.printSan(result, charSet);
		board.doMove(m);
	}

	return result;
}


void
Line::dump() const
{
	mstl::string result;
	::printf("%s\n", print(result).c_str());
}

// vi:set ts=3 sw=3:
