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

// ChessBase format description:
// http://talkchess.com/forum/viewtopic.php?t=29468&highlight=cbh
// http://talkchess.com/forum/viewtopic.php?topic_view=threads&p=287896&t=29468&sid=a535ba2e9a17395e2582bdddf57c2425

#include "cbh_decoder_position.h"

#include "db_exception.h"

#include "u_bit_stream.h"

#include "m_utility.h"

#include <string.h>

using namespace util;
using namespace db;
using namespace db::cbh::decoder;
using namespace db::cbh;
using namespace db::sq;


inline
static Byte
mapSquare(Byte sq)
{
	return sq::make(sq >> 3, sq & 7);
}


Position::Position()
{
	m_stack.reserve(5);
	m_stack.push();
}


void
Position::reset()
{
	//M_ASSERT(!m_stack.empty());

	while (m_stack.size() > 1)
		m_stack.pop();
}


Move
Position::doMove(Move move)
{
	Board& board = m_stack.top().board;

	move.setColor(board.sideToMove());

	if (board.isValidMove(move, move::AllowIllegalMove))
		move.setLegalMove();
	else if (!board.checkMove(move, move::AllowIllegalMove))
		return Move::empty();

	board.prepareUndo(move);
	board.doMove(move);

	return move;
}


void
Position::doMove(	piece::Type pieceType,
						Byte number,
						Byte offs,
						Byte& from,
						Byte& to,
						bool dontWrap,
						Byte* captured)
{
	//M_ASSERT(pieceType != piece::None);
	//M_ASSERT(number < 10);
	//M_ASSERT(number <= 2 || pieceType == piece::Pawn);
	//M_ASSERT(number == 0 || pieceType != piece::King);

	Lookup&	lookup		= m_stack.top();
	Pieces&	pieces		= lookup.pieces;
	Count&	pieceCount	= lookup.pieceCount;
	Board&	board			= lookup.board;
	Byte&		square		= pieces[number][piece::piece(pieceType, board.sideToMove())];

	if (dontWrap && sq::fyle(square) + (offs & 0x7) > sq::FyleH)
		offs -= 8;

	from = square;
	to = (from + offs) & 63;

	if (captured)
	{
		Byte capturedPiece = board.pieceAt(to);
		handleCapture(pieces, pieceCount, to, capturedPiece);
		*captured = piece::type(piece::ID(capturedPiece));
	}

	square = to;
	pieceCount[to] = number;
}


Move
Position::doMove(Byte from, Byte to, Byte promoted)
{
	from = ::mapSquare(from);
	to = ::mapSquare(to);

	Move move = m_stack.top().board.makeMove(from, to);

	// TODO:
	// ChessBase 10 supports chess 960. I don't know how castling moves are handled.
	// Possibly K captures R?

	if (move.isPromotion())
	{
		if (promoted == piece::None)
			return Move::empty();

		Lookup&	lookup			= m_stack.top();
		Pieces&	pieces			= lookup.pieces;
		Count&	pieceCount		= lookup.pieceCount;
		Board&	board				= lookup.board;
		Byte		capturedPiece	= board.pieceAt(to);
		unsigned	number			= 0;

		if (promoted)
			move.setPromotionPiece(piece::Type(promoted));
		promoted = piece::piece(piece::Type(promoted), board.sideToMove());

		while (pieces[number][promoted] != sq::Null)
		{
			if (++number == 10)
				return Move::empty();
		}

		handleCapture(pieces, pieceCount, to, capturedPiece);
		pieces[number][promoted] = to;
		pieceCount[to] = number;
	}

	return doMove(move);
}


Move
Position::doCastling(Byte offs)
{
	Byte from, to;

	doMove(piece::King, 0, offs, from, to, false);

	Lookup&		lookup				= m_stack.top();
	Count&		pieceCount			= lookup.pieceCount;
	Pieces&		pieces				= lookup.pieces;
	Board&		board					= lookup.board;
	color::ID	sideToMove			= board.sideToMove();
	bool			isShortCastling	= from < to;	// XXX only works with KxR notation

	Byte rank		= sq::rank(to);
	// TODO: does not work with chess 960
	Byte rookFrom	= sq::make(isShortCastling ? sq::FyleH : sq::FyleA, rank);
	Byte rookTo		= sq::make(isShortCastling ? sq::FyleF : sq::FyleD, rank);
	Byte rookNum	= pieceCount[rookFrom];

	pieces[rookNum][piece::piece(piece::Rook, sideToMove)] = rookTo;
	pieceCount[rookTo] = rookNum;

	return doMove(Move::genCastling(from, rookFrom));
}


Move
Position::doKingMove(Byte offs)
{
	//M_ASSERT(mstl::abs(char(offs)) != 2);	// we do not handle castling here

	Byte from, to, captured;
	doMove(piece::King, 0, offs, from, to, true, &captured);
	return doMove(Move::genKingMove(from, to, captured));
}


Move
Position::doQueenMove(Byte number, Byte offs)
{
	Byte from, to, captured;
	doMove(piece::Queen, number, offs, from, to, true, &captured);
	return doMove(Move::genQueenMove(from, to, captured));
}


Move
Position::doRookMove(Byte number, Byte offs)
{
	Byte from, to, captured;
	doMove(piece::Rook, number, offs, from, to, true, &captured);
	return doMove(Move::genRookMove(from, to, captured));
}


Move
Position::doBishopMove(Byte number, Byte offs)
{
	Byte from, to, captured;
	doMove(piece::Bishop, number, offs, from, to, true, &captured);
	return doMove(Move::genBishopMove(from, to, captured));
}


Move
Position::doKnightMove(Byte number, Byte offs)
{
	Byte from, to, captured;
	doMove(piece::Knight, number, offs, from, to, false, &captured);
	return doMove(Move::genKnightMove(from, to, captured));
}


Move
Position::doPawnOneForward(Byte number)
{
	Byte from, to;
	doMove(piece::Pawn, number, m_stack.top().board.whiteToMove() ? +8 : -8, from, to, false);
	return doMove(Move::genOneForward(from, to));
}


Move
Position::doPawnTwoForward(Byte number)
{
	Byte from, to;
	doMove(piece::Pawn, number, m_stack.top().board.whiteToMove() ? +16 : -16, from, to, false);
	return doMove(Move::genTwoForward(from, to));
}


Move
Position::doCapture(Byte number, Byte offs)
{
	Byte from, to, captured;

	doMove(piece::Pawn, number, offs, from, to, false, &captured);

	if (captured != piece::None)
		return doMove(Move::genPawnCapture(from, to, captured));

	Move move = Move::genEnPassant(from, to);

	Lookup&	lookup	= m_stack.top();
	Square	epSquare	= move.enPassantSquare();

	handleCapture(lookup.pieces, lookup.pieceCount, epSquare, lookup.board.pieceAt(epSquare));

	return doMove(move);
}


void
Position::handleCapture(Pieces& pieces, Count& pieceCount, Byte to, Byte piece)
{
	if (piece == piece::Empty)
		return;

	Byte pieceNum = pieceCount[to];

	pieces[pieceNum][piece] = Null;

	if (piece::type(piece::ID(piece)) == piece::Pawn)
		return;

	for (unsigned i = 0; i < 10; ++i)
	{
		Byte square = pieces[i][piece];

		if (square != Null)
		{
			Byte number = pieceCount[square];

			if (number > pieceNum)
			{
				pieces[number - 1][piece] = pieces[number][piece];
				pieces[number][piece] = Null;
				--pieceCount[square];
			}
		}
		else if (i != pieceNum)
		{
			return;
		}
	}
}


void
Position::setup(BitStream& strm)
{
	// M_REQUIRE(strm.bitsLeft() >= 224);

	reset();

	Lookup&	lookup		= m_stack.top();
	Board&	board			= lookup.board;
	Pieces&	pieces		= lookup.pieces;
	Count&	pieceCount	= lookup.pieceCount;

	board.clear();
	strm.skip(11);

	Byte toMove = strm.next(1);
	board.setToMove(color::ID(toMove));

	Byte epFyle	= strm.next(4);

	strm.skip(4);

	bool bshrt = strm.next(1);
	bool blong = strm.next(1);
	bool wshrt = strm.next(1);
	bool wlong = strm.next(1);

	board.setPlyNumber(mstl::mul2(mstl::max(1u, unsigned(strm.next(8))) - 1) + 1 + toMove);

	::memset(pieces, Null, sizeof(pieces));
	::memset(pieceCount, 0, sizeof(pieceCount));

	Byte countPieces[15] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

	for (unsigned i = 0; i < 64; ++i)
	{
		if (strm.next(1))
		{
			static Byte const PieceMap[16] =
			{
				piece::Empty,
				piece::WhiteKing,
				piece::WhiteQueen,
				piece::WhiteKnight,
				piece::WhiteBishop,
				piece::WhiteRook,
				piece::WhitePawn,
				piece::Empty,
				piece::Empty,
				piece::BlackKing,
				piece::BlackQueen,
				piece::BlackKnight,
				piece::BlackBishop,
				piece::BlackRook,
				piece::BlackPawn,
				piece::Empty,
			};

			Byte piece = PieceMap[strm.next(4)];

			// if (piece == piece::Empty)
				// IO_RAISE(Game, Corrupted, "illegal piece in start position");

			Byte& count = countPieces[piece];

			// if (count == 10)
				// IO_RAISE(Game, Corrupted, "too many pieces in start position");

			Square sq = ::mapSquare(i);

			if (!board.setAt(sq, piece::ID(piece))){}
				// IO_RAISE(Game, Corrupted, "invalid start position");

			pieces[count][piece] = sq;
			pieceCount[sq] = count++;
		}
	}

	if (bshrt) board.setCastleShort(color::Black);
	if (blong) board.setCastleLong (color::Black);
	if (wshrt) board.setCastleShort(color::White);
	if (wlong) board.setCastleLong (color::White);

	if (epFyle)
		board.setEnPassantFyle(sq::Fyle(sq::FyleA + (epFyle - 1)));

	// BUG: ChessBase 10 supports chess 960 very halfhearted. They do not
	// have a decoding for the castling rooks.

	if (strm.bitsLeft() == 0)
	{
		board.fixBadCastlingRights();

		if (board.validate(variant::Standard) != Board::Valid){}
			// IO_RAISE(Game, Corrupted, "illegal start position");
	}
	else
	{
		if (board.validate(variant::Chess960) != Board::Valid){}
			// IO_RAISE(Game, Corrupted, "unsupported start position");
	}
}


void
Position::setup()
{
#define __ Null
	static Pieces const StandardPosition =
	{
	//   e   wk  wq  wr  wb  wn  wp  -   -   bk  bq  br  bb  bn  bp
		{ __, e1, d1, a1, c1, b1, a2, __, __, e8, d8, a8, c8, b8, a7 },
		{ __, __, __, h1, f1, g1, b2, __, __, __, __, h8, f8, g8, b7 },
		{ __, __, __, __, __, __, c2, __, __, __, __, __, __, __, c7 },
		{ __, __, __, __, __, __, d2, __, __, __, __, __, __, __, d7 },
		{ __, __, __, __, __, __, e2, __, __, __, __, __, __, __, e7 },
		{ __, __, __, __, __, __, f2, __, __, __, __, __, __, __, f7 },
		{ __, __, __, __, __, __, g2, __, __, __, __, __, __, __, g7 },
		{ __, __, __, __, __, __, h2, __, __, __, __, __, __, __, h7 },
		{ __, __, __, __, __, __, __, __, __, __, __, __, __, __, __ },
		{ __, __, __, __, __, __, __, __, __, __, __, __, __, __, __ },
	};
#undef __
#define _ 0
	static Count const PieceCountSetup =
	{
		0, 0, 0, 0, 0, 1, 1, 1,	// a1 .. h1
	   0, 1, 2, 3, 4, 5, 6, 7,	// a2 .. h2
		_, _, _, _, _, _, _, _,	// a3 .. h3
		_, _, _, _, _, _, _, _,	// a4 .. h4
		_, _, _, _, _, _, _, _,	// a5 .. h5
		_, _, _, _, _, _, _, _,	// a6 .. h6
	   0, 1, 2, 3, 4, 5, 6, 7,	// a7 .. h7
		0, 0, 0, 0, 0, 1, 1, 1,	// a8 .. h8
	};
#undef _

	reset();

	Lookup& lookup = m_stack.top();

	lookup.board.setStandardPosition();
	::memcpy(lookup.pieces, StandardPosition, sizeof(StandardPosition));
	::memcpy(lookup.pieceCount, PieceCountSetup, sizeof(PieceCountSetup));
}

// vi:set ts=3 sw=3:
