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

#include "db_tablebase.h"
#include "db_board.h"
#include "db_probe.h"
#include "db_move.h"

#include "u_http.h"

#include "m_string.h"
#include "m_utility.h"

#include <ctype.h>
#include <stdlib.h>

using namespace db;


static int
getNextInt(char const*& p)
{
	while (!isdigit(*p))
	{
		if (!*p)
			return -1;

		++p;
	}

	return strtol(p, const_cast<char**>(&p), 10);
}


Tablebase::Tablebase(Probe const* probe)
	:m_probe(probe)
	,m_useOnlineQuery(false)
{
}


Probe const*
Tablebase::setProbe(Probe const* probe)
{
	mstl::swap(m_probe, probe);
	return probe;
}


void
Tablebase::useOnlineQuery(bool flag)
{
	m_useOnlineQuery = flag;
}


int
Tablebase::bestMove(Board const& board, Move& result) const
{
	int numWhitePieces = material::count(board.signature().material(color::White)) + 1;
	int numBlackPieces = material::count(board.signature().material(color::Black)) + 1;

	if (	numWhitePieces + numBlackPieces > 6
		|| numWhitePieces + numBlackPieces <= 2
		|| mstl::max(numWhitePieces, numBlackPieces) > 4)
	{
		return tb::Not_Found;
	}

	if (m_probe)
	{
		int score = m_probe->findBest(board, result);

		switch (score)
		{
			case tb::Any_Move:
			case tb::No_Legal_Move:
				return score;
		}
	}

	return m_useOnlineQuery ? getOnlineQuery(board, result) : int(tb::Not_Found);
}


int
Tablebase::getOnlineQuery(Board const& board, Move& result)
{
	// Fetch result from www.shredderchess.com/online/playshredder/fetch.php?action=egtb
	// (an alternative is http://k4it.de/egtb/fetch.php?action=egtb).

	// IMPORTANT NOTE: Both do not allow online access from other chess applications.

	// other sites:
	// http://www.lokasoft.nl/tbweb.aspx		!! allows access from other chess applications
	// http://chessok.com/?page_id=361

	// Probably this is the most interesting one:
	// http://chess.jaet.org/endings/

	static bool onlineQueryAllowed = ::getenv("SCIDB_ONLINE_QUERY_ALLOWED");

	if (!onlineQueryAllowed)
		return tb::Not_Found;

	static mstl::string const NextColor("NEXTCOLOR");

	util::Http		http("www.shredderchess.com");
	mstl::string	url("/online/playshredder/fetch.php?action=egtb");
	mstl::string	answer;

	url += "&hook=";
	url += char(board.whiteToMove() ? 'w' : 'b');
	url += "&fen=";
	board.toFen(url);

	if (http.get(url, answer) <= 0)
		return tb::Not_Found;

	mstl::string::size_type n = answer.find(NextColor);

	if (n == mstl::string::npos)
		return tb::Broken;

	char const* p = answer.c_str() + n + 9;	// position after 'NEXTCOLOR'

	while (!isupper(*p))
		++p;

	int score;

	switch (*p)
	{
		case 'E':	// "Error"
			{
				unsigned state = board.checkState();

				if (state & Board::CheckMate)
					return tb::Is_Check_Mate;
				if (state & Board::StaleMate)
					return tb::Is_Stale_Mate;

				return tb::Illegal_Position;
			}
			// not reached

		case 'N':	// "Not found"
			return tb::Not_Found;

		case 'L':	// "Loss in"
			score = -::getNextInt(p);
			break;

		case 'D':	// "Draw"
			score = 0;
			break;

		case 'W':	// "Win in"
			score = ::getNextInt(p);
			break;

		default:
			return tb::Broken;
	}

	int from	= ::getNextInt(p);
	int to	= ::getNextInt(p);

	if (score < 0 || from < 0 || to < 0)
		return tb::Broken;

	result = board.prepareMove(from, to);

	if (!result.isLegal())
		return tb::Broken;

	if (result.promotedPiece())
	{
		switch (getNextInt(p))
		{
			case 8:	result.setPromotionPiece(piece::Queen); break;
			case 9:	result.setPromotionPiece(piece::Rook); break;
			case 10:	result.setPromotionPiece(piece::Bishop); break;
			case 11:	result.setPromotionPiece(piece::Knight); break;
			default:	return tb::Broken;
		}
	}

	return score;
}

// vi:set ts=3 sw=3:
