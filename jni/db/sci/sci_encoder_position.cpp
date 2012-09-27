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
// Copyright: (C) 2008-2012 Gregor Cramer
// ======================================================================

// ======================================================================
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
// ======================================================================

#include "sci_encoder_position.h"

#include "db_board.h"
#include "db_exception.h"

#include <string.h>

using namespace db;
using namespace db::sci::encoder;


//__attribute__((noreturn))
inline static void
throwInvalidBoardPosition()
{
//	IO_RAISE(Game, Corrupted, "invalid board position");
}


inline static unsigned
flipSquare(unsigned s)
{
	return sq::make(sq::fyle(s), sq::Rank8 - sq::rank(s));
}


Position::Position()
{
	m_stack.reserve(20);
	m_stack.push();
}


void
Position::doMove(Lookup& lookup, Move const& move)
{
	Square to = move.to();

	if (move.isCastling())
	{
		sq::Rank	rank		= sq::rank(to);
		Byte		pieceNum	= lookup[move.from()];

		if (move.isShortCastling())
		{
			lookup[sq::make(sq::FyleF, rank)] = m_rookNumbers[castling::kingSideIndex(move.color())];
			lookup[sq::make(sq::FyleG, rank)] = pieceNum;
		}
		else
		{
			lookup[sq::make(sq::FyleD, rank)] = m_rookNumbers[castling::queenSideIndex(move.color())];
			lookup[sq::make(sq::FyleC, rank)] = pieceNum;
		}
	}
	else if (__builtin_expect(!move.isNull(), 0))
	{
		lookup[to] = lookup[move.from()];
	}
}


void
Position::setup(Board const& board)
{
	Square shortCastlingRook[2];
	Square longCastlingRook[2];

	Byte whitePieceNum = 0;
	Byte blackPieceNum = 0;

	while (m_stack.size() > 1)
		m_stack.pop();

	Lookup& lookup = m_stack.top();
	Byte squares[2][16];

	::memset(lookup.numbers, 255, sizeof(Lookup::Numbers));
	::memset(m_rookNumbers, 255, sizeof(m_rookNumbers));
	::memset(squares, 0, sizeof(squares));

	shortCastlingRook[color::White] = board.shortCastlingRook(color::White);
	shortCastlingRook[color::Black] = board.shortCastlingRook(color::Black);
	longCastlingRook [color::White] = board.longCastlingRook (color::White);
	longCastlingRook [color::Black] = board.longCastlingRook (color::Black);

	for (unsigned i = 0; i < 64; ++i)
	{
		Square square = ::flipSquare(i);

		switch (board.pieceAt(square))
		{
			case piece::Empty:
				break;

			case piece::WhiteKing:
				if (whitePieceNum)
				{
					if (m_rookNumbers[castling::WhiteQS] == 0)
						m_rookNumbers[castling::WhiteQS] = whitePieceNum;
					else if (m_rookNumbers[castling::WhiteKS] == 0)
						m_rookNumbers[castling::WhiteKS] = whitePieceNum;
					lookup[squares[color::White][0]] = whitePieceNum;
				}
				whitePieceNum++;
				lookup[square] = 0;
				break;

			case piece::BlackKing:
				if (blackPieceNum)
				{
					if (m_rookNumbers[castling::BlackQS] == 0)
						m_rookNumbers[castling::BlackQS] = blackPieceNum;
					else if (m_rookNumbers[castling::BlackKS] == 0)
						m_rookNumbers[castling::BlackKS] = blackPieceNum;
					lookup[squares[color::Black][0]] = blackPieceNum;
				}
				blackPieceNum++;
				lookup[square] = 0;
				break;

			case piece::WhiteRook:
				if (square == shortCastlingRook[color::White])
					m_rookNumbers[castling::WhiteKS] = whitePieceNum;
				else if (square == longCastlingRook[color::White])
					m_rookNumbers[castling::WhiteQS] = whitePieceNum;
				// fallthru

			case piece::WhiteQueen:
			case piece::WhiteBishop:
			case piece::WhiteKnight:
			case piece::WhitePawn:
				if (__builtin_expect(whitePieceNum == 16, 0))
					::throwInvalidBoardPosition();
				squares[color::White][whitePieceNum] = square;
				lookup[square] = whitePieceNum++;
				break;

			case piece::BlackRook:
				if (square == shortCastlingRook[color::Black])
					m_rookNumbers[castling::BlackKS] = blackPieceNum;
				else if (square == longCastlingRook[color::Black])
					m_rookNumbers[castling::BlackQS] = blackPieceNum;
				// fallthru

			case piece::BlackQueen:
			case piece::BlackBishop:
			case piece::BlackKnight:
			case piece::BlackPawn:
				if (__builtin_expect(blackPieceNum == 16, 0))
					::throwInvalidBoardPosition();
				squares[color::Black][blackPieceNum] = square;
				lookup[square] = blackPieceNum++;
				break;
		}
	}
}


void
Position::setup()
{
#define _ 255
	static Lookup::Numbers const StandardPieceNumbers =
	{
		 8,  9, 10, 11,  0, 13, 14, 15,
		12,  1,  2,  3,  4,  5,  6,  7,
		 _,  _,  _,  _,  _,  _,  _,  _,
		 _,  _,  _,  _,  _,  _,  _,  _,
		 _,  _,  _,  _,  _,  _,  _,  _,
		 _,  _,  _,  _,  _,  _,  _,  _,
		 8,  9, 10, 11, 12, 13, 14, 15,
		 4,  1,  2,  3,  0,  5,  6,  7,
	};
#undef _

	while (m_stack.size() > 1)
		m_stack.pop();

	::memcpy(m_stack.top().numbers, StandardPieceNumbers, sizeof(StandardPieceNumbers));

	m_rookNumbers[castling::WhiteQS] =  8;
	m_rookNumbers[castling::WhiteKS] = 15;
	m_rookNumbers[castling::BlackQS] =  4;
	m_rookNumbers[castling::BlackKS] =  7;
}

// vi:set ts=3 sw=3:
