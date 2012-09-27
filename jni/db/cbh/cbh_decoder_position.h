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

#ifndef _cbh_decoder_position_included
#define _cbh_decoder_position_included

#include "db_board.h"
#include "db_move.h"

#include "m_stack.h"

namespace util { class BitStream; }

namespace db {
namespace cbh {
namespace decoder {

class Position
{
public:

	Position();

	unsigned variationLevel() const;

	void setup();
	void setup(util::BitStream& strm);

	void push();
	void pop();

	Board const& board() const;

	Move doNullMove();
	Move doCastling(Byte offs);
	Move doKingMove(Byte offs);
	Move doQueenMove(Byte number, Byte offs);
	Move doRookMove(Byte number, Byte offs);
	Move doBishopMove(Byte number, Byte offs);
	Move doKnightMove(Byte number, Byte offs);
	Move doPawnOneForward(Byte number);
	Move doPawnTwoForward(Byte number);
	Move doCaptureRight(Byte number);
	Move doCaptureLeft(Byte number);
	Move doMove(Byte from, Byte to, Byte promoted);

private:

	typedef Byte Count[64];
	typedef Byte Pieces[10][15];

	struct Lookup
	{
		Board		board;
		Count		pieceCount;
		Pieces	pieces;
	};

	typedef mstl::stack<Lookup> Stack;

	void reset();
	void handleCapture(Pieces& pieces, Count& pieceCount, Byte to, Byte piece);
	void doMove(piece::Type pieceType,
					Byte number,
					Byte offs,
					Byte& from,
					Byte& to,
					bool dontWrap,
					Byte* captured = 0);
	Move doCapture(Byte number, Byte offs);
	Move doMove(Move move);

	Stack	m_stack;
	Byte	m_rookNumbers[4];
};

} // namespace decoder
} // namespace cbh
} // namespace db

namespace mstl {

template <typename> struct is_pod;

template <>
struct is_pod<db::cbh::decoder::Position::Lookup> { enum { value = is_pod<db::Board>::value }; };

} // namespace mstl

#include "cbh_decoder_position.ipp"

#endif // _cbh_decoder_position_included

// vi:set ts=3 sw=3:
