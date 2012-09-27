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

#ifndef _db_tablebase_included
#define _db_tablebase_included

namespace db {

class Board;
class Probe;
class Move;

class Tablebase
{
public:

	Tablebase(Probe const* probe = 0);

	Probe const* setProbe(Probe const* probe);
	void useOnlineQuery(bool flag = true);

	int bestMove(Board const& board, Move& result) const;

private:

	static int getOnlineQuery(Board const& board, Move& result);

	Probe const*	m_probe;
	bool				m_useOnlineQuery;
};

} // namespace db

#endif // _db_tablebase_included

// vi:set ts=3 sw=3:
