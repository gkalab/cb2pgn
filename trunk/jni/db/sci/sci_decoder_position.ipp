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

namespace db {
namespace sci {
namespace decoder {

inline void Position::push()								{ m_stack.dup(); }
inline void Position::pop()								{ m_stack.pop(); }

inline Board const& Position::board() const			{ return m_stack.top().board; }
inline Board& Position::board()							{ return m_stack.top().board; }

inline bool Position::whiteToMove() const				{ return board().whiteToMove(); }
inline bool Position::blackToMove() const				{ return board().blackToMove(); }

inline piece::Type Position::piece(Square s) const	{ return board().piece(s); }

inline Square Position::operator[](int n) const		{ return m_stack.top().squares[n]; }


inline
Move
Position::makeShortCastlingMove(Square from)
{
	Board const& board = m_stack.top().board;
	castling::Index idx = castling::kingSideIndex(board.sideToMove());
	//M_ASSERT(board.castlingRookSquare(idx) != sq::Null);
	return Move::genCastling(from, board.castlingRookSquare(idx));
}


inline
Move
Position::makeLongCastlingMove(Square from)
{
	Board const& board = m_stack.top().board;
	castling::Index idx = castling::queenSideIndex(board.sideToMove());
	//M_ASSERT(board.castlingRookSquare(idx) != sq::Null);
	return Move::genCastling(from, board.castlingRookSquare(idx));
}


inline
Move
Position::makeKingMove(Square from, Square to) const
{
	return Move::genKingMove(from, to, piece(to));
}


inline
Move
Position::makeQueenMove(Square from, Square to) const
{
	return Move::genQueenMove(from, to, piece(to));
}


inline
Move
Position::makeRookMove(Square from, Square to) const
{
	return Move::genRookMove(from, to, piece(to));
}


inline
Move
Position::makeBishopMove(Square from, Square to) const
{
	return Move::genBishopMove(from, to, piece(to));
}


inline
Move
Position::makeKnightMove(Square from, Square to) const
{
	return Move::genKnightMove(from, to, piece(to));
}

} // namespace decoder
} // namespace sci
} // namespace db

// vi:set ts=3 sw=3:
