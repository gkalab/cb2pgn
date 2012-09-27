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

#include "sci_decoder_position.h"

#include "db_exception.h"

#include "m_assert.h"

#include <string.h>
#include <ctype.h>

using namespace db;
using namespace db::sci::decoder;


static Byte const NoRook = 0xff;


//__attribute__((noreturn))
inline static void
throwInvalidFen()
{
	//IO_RAISE(Game, Corrupted, "error while decoding game data (invalid FEN)");
}


inline static unsigned
convSquare(unsigned s)
{
	return sq::make(sq::fyle(s), sq::Rank8 - sq::rank(s));
}


Position::Position()
{
	m_stack.reserve(10);
	m_stack.push();
}


void
Position::doMove(Move& move, unsigned pieceNum)
{
	Lookup& lookup = m_stack.top();

	if (move.isCastling())
	{
		Byte rank = sq::rank(move.to());

		if (move.isShortCastling())
		{
			lookup.squares[pieceNum] = sq::make(sq::FyleG, rank);

			unsigned rookNum = m_rookNumbers[castling::kingSideIndex(move.color())];

			if (rookNum != ::NoRook)	// we allow castlings without rook
				lookup.squares[rookNum] = sq::make(sq::FyleF, rank);
		}
		else
		{
			lookup.squares[pieceNum] = sq::make(sq::FyleC, rank);

			unsigned rookNum = m_rookNumbers[castling::queenSideIndex(move.color())];

			if (rookNum != ::NoRook)	// we allow castlings without rook
				lookup.squares[rookNum] = sq::make(sq::FyleD, rank);
		}
	}
	else if (__builtin_expect(!move.isNull(), 1))
	{
		lookup.squares[pieceNum] = move.to();
	}
}


void
Position::setup(char const* fen)
{
	while (m_stack.size() > 1)
		m_stack.pop();

	if (__builtin_expect(!board().setup(fen), 0))	// should never fail
		::throwInvalidFen();

	//M_ASSERT(board().validate(variant::Unknown) == Board::Valid);

	unsigned whitePieceNum = 0;
	unsigned blackPieceNum = 0x10;

	Squares& squares = m_stack.top().squares;

	::memset(squares, 0, sizeof(squares));
	::memset(m_rookNumbers, ::NoRook, sizeof(m_rookNumbers));

	Square shortCastlingRook[2];
	Square longCastlingRook[2];

	bool haveKing[2] = { false, false };

	shortCastlingRook[color::White] = ::convSquare(board().castlingRookSquare(castling::WhiteKS));
	shortCastlingRook[color::Black] = ::convSquare(board().castlingRookSquare(castling::BlackKS));
	longCastlingRook [color::White] = ::convSquare(board().castlingRookSquare(castling::WhiteQS));
	longCastlingRook [color::Black] = ::convSquare(board().castlingRookSquare(castling::BlackQS));

	for (unsigned i = 0; i < 64; ++fen)
	{
		if (::isdigit(*fen))
		{
			if (__builtin_expect(*fen == '9', 0))
				::throwInvalidFen();

			i += *fen - '0';
		}
		else switch (*fen)
		{
			case 'R':
				if (i == shortCastlingRook[color::White])
					m_rookNumbers[castling::WhiteKS] = whitePieceNum;
				else if (i == longCastlingRook[color::White])
					m_rookNumbers[castling::WhiteQS] = whitePieceNum;
				 // fallthru

			case 'Q': case 'B': case 'N':
				if (__builtin_expect(whitePieceNum == 0x10, 0))	// should never happen
					 ::throwInvalidFen();
				squares[whitePieceNum++] = ::convSquare(i++);
				break;

			case 'P':
				if (__builtin_expect(whitePieceNum == 0x10, 0))	// should never happen
					::throwInvalidFen();
				{
					Square sq = ::convSquare(i++);
					if (__builtin_expect((1 << sq::rank(sq)) & (1 << sq::Rank1 | 1 << sq::Rank8), 0))
						::throwInvalidFen();
					squares[whitePieceNum++] = sq;
				}
				break;

			case 'r':
				if (i == shortCastlingRook[color::Black])
					m_rookNumbers[castling::BlackKS] = blackPieceNum;
				else if (i == longCastlingRook[color::Black])
					m_rookNumbers[castling::BlackQS] = blackPieceNum;
				 // fallthru

			case 'q': case 'b': case 'n':
				if (__builtin_expect(blackPieceNum == 0x20, 0))	// should never happen
					::throwInvalidFen();
				squares[blackPieceNum++] = ::convSquare(i++);
				break;

			case 'p':
				if (__builtin_expect(blackPieceNum == 0x20, 0))	// should never happen
					::throwInvalidFen();
				{
					Square sq = ::convSquare(i++);
					if (__builtin_expect((1 << sq::rank(sq)) & (1 << sq::Rank1 | 1 << sq::Rank8), 0))
						::throwInvalidFen();
					squares[blackPieceNum++] = sq;
				}
				break;

			case 'K':
				if (__builtin_expect(haveKing[color::White], 0))
					::throwInvalidFen();
				if (m_rookNumbers[castling::WhiteQS] == 0)
					m_rookNumbers[castling::WhiteQS] = whitePieceNum;
				else if (m_rookNumbers[castling::WhiteKS] == 0)
					m_rookNumbers[castling::WhiteKS] = whitePieceNum;
				squares[whitePieceNum++] = squares[0];
				squares[0] = ::convSquare(i++);
				haveKing[color::White] = true;
				break;

			case 'k':
				if (__builtin_expect(haveKing[color::Black], 0))
					::throwInvalidFen();
				if (m_rookNumbers[castling::BlackQS] == 0x10)
					m_rookNumbers[castling::BlackQS] = blackPieceNum;
				else if (m_rookNumbers[castling::BlackKS] == 0x10)
					m_rookNumbers[castling::BlackKS] = blackPieceNum;
				squares[blackPieceNum++] = squares[0x10];
				squares[0x10] = ::convSquare(i++);
				haveKing[color::Black] = true;
				break;

			case '/':
				if (__builtin_expect(i & 7, 0))
					::throwInvalidFen();
				break;

			default:
				::throwInvalidFen();
		}
	}
}


void
Position::setup(Board const& board)
{
	//M_ASSERT(board.isShuffleChessPosition());

	static Square const Rank2[8] = { sq::a2, sq::b2, sq::c2, sq::d2, sq::e2, sq::f2, sq::g2, sq::h2 };
	static Square const Rank7[8] = { sq::a7, sq::b7, sq::c7, sq::d7, sq::e7, sq::f7, sq::g7, sq::h7 };

	Byte whitePieceNum = 8;
	Byte blackPieceNum = 16;

	Lookup&	lookup	= m_stack.top();
	Squares&	squares	= lookup.squares;

	::memset(squares, 0, sizeof(Squares));
	::memcpy(squares, Rank2, sizeof(Rank2));
	::memcpy(squares + 24, Rank7, sizeof(Rank7));
	::memset(m_rookNumbers, ::NoRook, sizeof(m_rookNumbers));

	for (unsigned square = sq::a1; square <= sq::h1; ++square)
	{
		switch (unsigned(board.piece(sq::ID(square))))
		{
			case piece::King:
				squares[whitePieceNum++] = squares[0];
				squares[0] = square;
				break;

			case piece::Rook:
				if (m_rookNumbers[castling::WhiteQS] == ::NoRook)
					m_rookNumbers[castling::WhiteQS] = whitePieceNum;
				else
					m_rookNumbers[castling::WhiteKS] = whitePieceNum;
				// fallthru

			case piece::Queen:
			case piece::Bishop:
			case piece::Knight:
				squares[whitePieceNum++] = square;
				break;
		}
	}

	for (unsigned square = sq::a8; square <= sq::h8; ++square)
	{
		switch (unsigned(board.piece(sq::ID(square))))
		{
			case piece::King:
				if (m_rookNumbers[castling::BlackQS] == 16)
					m_rookNumbers[castling::BlackQS] = blackPieceNum;
				squares[blackPieceNum++] = squares[16];
				squares[16] = square;
				break;

			case piece::Rook:
				if (m_rookNumbers[castling::BlackQS] == ::NoRook)
					m_rookNumbers[castling::BlackQS] = blackPieceNum;
				else
					m_rookNumbers[castling::BlackKS] = blackPieceNum;
				// fallthru

			case piece::Queen:
			case piece::Bishop:
			case piece::Knight:
				squares[blackPieceNum++] = square;
				break;
		}
	}
}


void
Position::setup(uint16_t idn)
{
	while (m_stack.size() > 1)
		m_stack.pop();

	if (idn == chess960::StandardIdn)
	{
		static Squares const StandardSquares =
		{
			sq::e1, sq::b2, sq::c2, sq::d2, sq::e2, sq::f2, sq::g2, sq::h2,
			sq::a1, sq::b1, sq::c1, sq::d1, sq::a2, sq::f1, sq::g1, sq::h1,
			sq::e8, sq::b8, sq::c8, sq::d8, sq::a8, sq::f8, sq::g8, sq::h8,
			sq::a7, sq::b7, sq::c7, sq::d7, sq::e7, sq::f7, sq::g7, sq::h7,
		};

		::memcpy(m_stack.top().squares, StandardSquares, sizeof(StandardSquares));

		m_rookNumbers[castling::WhiteQS] =  8;
		m_rookNumbers[castling::WhiteKS] = 15;
		m_rookNumbers[castling::BlackQS] = 20;
		m_rookNumbers[castling::BlackKS] = 23;

		board().setStandardPosition();
	}
	else
	{
		board().setup(idn);
		setup(board());
	}
}

// vi:set ts=3 sw=3:
