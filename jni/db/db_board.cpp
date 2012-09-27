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

// ======================================================================
// The implementation is loosely based on chessx/src/database/bitboard.cpp
//   (C) 2003 Sune Fischer
//   (C) 2005-2006 Marius Roets <roets.marius@gmail.com>
//   (C) 2005-2009 Michal Rudolf <mrudolf@kdewebdev.org>
// ======================================================================

#include "db_board.h"
#include "db_board_base.h"
#include "db_rand64.h"

#include "m_assert.h"
#include "m_bitfield.h"
#include "m_bit_functions.h"
#include "m_utility.h"
#include "m_stdio.h"

#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <assert.h>

using namespace db;
using namespace db::sq;
using namespace db::color;
using namespace db::castling;
using namespace db::board;

namespace bf = mstl::bf;

static uint64_t const DarkSquares	= A1 | C1 | E1 | G1
												| B2 | D2 | F2 | H2
												| A3 | C3 | E3 | G3
												| B4 | D4 | F4 | H4
												| A5 | C5 | E5 | G5
												| B6 | D6 | F6 | H6
												| A7 | C7 | E7 | G7
												| B8 | D8 | F8 | H8;

static uint64_t const LiteSquares	= B1 | D1 | F1 | H1
												| A2 | C2 | E2 | G2
												| B3 | D3 | F3 | H3
												| A4 | C4 | E4 | G4
												| B5 | D5 | F5 | G5
												| A6 | C6 | E6 | G6
												| B7 | D7 | F7 | H7
												| A8 | C8 | E8 | G8;

Board Board::m_standardBoard;
Board Board::m_shuffleChessBoard;
Board Board::m_emptyBoard;

inline static int mul8(int x)				{ return x << 3; }
inline static int mul16(int x)			{ return x << 4; }

inline static int fyle(int s) 			{ return s & 7; }
inline static int rank(int s) 			{ return s >> 3; }

inline static bool isFyle(char c)		{ return c >= 'a' && c <= 'h'; }
inline static bool isRank(char c)		{ return c >= '1' && c <= '8'; }

inline static char toFyle(char fyle)	{ return fyle - 'a'; }
inline static char toFYLE(char fyle)	{ return fyle - 'A'; }
inline static char toRank(char rank)	{ return rank - '1'; }

inline static unsigned flipRank(unsigned s)		{ return flipRank(sq::ID(s)); }

inline static Byte kingSideIndex(Byte color)		{ return castling::kingSideIndex(color::ID(color)); }
inline static Byte queenSideIndex(Byte color)	{ return castling::queenSideIndex(color::ID(color)); }


static void __attribute__((constructor)) initialize() { Board::initialize(); }


template <typename T>
inline static
T
flipFyle(T s)
{
	if (s == Null)
		return Null;

	return flipFyle(sq::ID(s));
}


inline
static piece::ID
toPiece(Byte type, Byte color)
{
	return piece::piece(piece::Type(type), color::ID(color));
}


inline
static uint64_t
epSquareHashKey(Square s)
{
	return rand64::EnPassant[s - (rank(s) == Rank3 ? a3 : a6)];
}


static char const*
skipPromotion(char const* s)
{
	char const* t = s;

	// skip promotion as in bc8Q, bxc8=Q, bxc8=(Q), bxc8(Q), bxc8/Q, or bxc8/(Q)

	if (*t == '=' || *t == '/')
		++s;

	char match = '\0';

	if (*t == '(')
	{
		match = ')';
		++t;
	}

	switch (*t)
	{
		case 'Q': case 'q':
		case 'R': case 'r':
		case 'B': case 'b':
		case 'N': case 'n':
			++t;
			break;

		default:
			return s;
	}

	if (match)
	{
		if (*t != match)
			return s;

		++t;
	}

	return t;
}


uint64_t Board::knightAttacks(Square square) const	{ return KnightAttacks[square]; }
uint64_t Board::kingAttacks(Square square) const	{ return KingAttacks[square]; }


uint64_t
Board::rankAttacks(Square square, uint64_t occupied) const
{
	return RankAttacks[square][(occupied >> ((square & ~7) | 1)) & 63];
}


uint64_t
Board::fyleAttacks(Square square, uint64_t occupied) const
{
	return FyleAttacks[square][(occupied >> (((square & 7) << 3) | 1)) & 63];
}


uint64_t
Board::rankAttacks(Square square) const
{
	return rankAttacks(square, m_occupied);
}


uint64_t
Board::fyleAttacks(Square square) const
{
	return fyleAttacks(square, m_occupiedL90);
}


uint64_t
Board::diagA1H8Attacks(Square square) const
{
	return R45Attacks[square][(m_occupiedR45 >> ShiftR45[square]) & 63];
}


uint64_t
Board::diagH1A8Attacks(Square square) const
{
	return L45Attacks[square][(m_occupiedL45 >> ShiftL45[square]) & 63];
}


uint64_t
Board::bishopAttacks(Square square) const
{
	return diagA1H8Attacks(square) | diagH1A8Attacks(square);
}


uint64_t
Board::rookAttacks(Square square) const
{
	return rankAttacks(square) | fyleAttacks(square);
}


uint64_t
Board::queenAttacks(Square square) const
{
	return rookAttacks(square) | bishopAttacks(square);
}


uint64_t
Board::attacks(unsigned color, Square square) const
{
	uint64_t attackers =   (PawnAttacks[color ^ 1][square] & m_pawns)
								| (knightAttacks(square) & m_knights)
								| (bishopAttacks(square) & (m_bishops | m_queens))
								| (rookAttacks(square) & (m_rooks | m_queens))
								| (kingAttacks(square) & m_kings);

	return attackers & m_occupiedBy[color];
}


bool
Board::isAttackedBy(unsigned color, uint64_t squares) const
{
	while (squares)
	{
		if (isAttackedBy(color, Square(lsbClear(squares))))
			return true;
	}

	return false;
}


void
Board::pawnProgressMove(unsigned color, unsigned from, unsigned to)
{
	if (color == White)
		m_progress.side[White].move(from, to);
	else
		m_progress.side[Black].move(::flipRank(from), ::flipRank(to));
}


void
Board::pawnProgressRemove(unsigned color, unsigned at)
{
	if (color == White)
		m_progress.side[White].remove(at);
	else
		m_progress.side[Black].remove(::flipRank(at));
}


void
Board::pawnProgressAdd(unsigned color, unsigned at)
{
	if (color == White)
		m_progress.side[White].add(at);
	else
		m_progress.side[Black].add(::flipRank(at));
}


unsigned
Board::checkState(Move const& move) const
{
	Board peek(*this);
	peek.doMove(move);
	return peek.checkState();
}


bool
Board::isDoubleCheck() const
{
	return count(attacks(m_stm ^ 1, m_ksq[m_stm])) > 1;
}


bool
Board::isSamePosition(Board const& target) const
{
	return ::memcmp(&target, this, sizeof(board::Position)) == 0;
}


bool
Board::isEqualPosition(Board const& target) const
{
	return m_hash == target.m_hash && ::memcmp(&target, this, sizeof(board::ExactPosition)) == 0;
}


void
Board::hashPawn(Square s, piece::ID piece)
{
	////M_ASSERT(piece == piece::WhitePawn || piece == piece::BlackPawn);

	uint64_t value = rand64::Squares[piece][s];

	m_hash ^= value;
	m_pawnHash ^= value;
}


void
Board::hashPawn(Square s, Square t, piece::ID piece)
{
	//M_ASSERT(piece == piece::WhitePawn || piece == piece::BlackPawn);

	uint64_t const*	values	= rand64::Squares[piece];
	uint64_t				value		= values[s] ^ values[t];

	m_hash ^= value;
	m_pawnHash ^= value;
}


void
Board::hashPiece(Square s, piece::ID piece)
{
	//M_ASSERT(piece != piece::WhitePawn && piece != piece::BlackPawn);
	m_hash ^= rand64::Squares[piece][s];
}


void
Board::hashPiece(Square s, Square t, piece::ID piece)
{
	//M_ASSERT(piece != piece::WhitePawn && piece != piece::BlackPawn);

	uint64_t const* values = rand64::Squares[piece];

	m_hash ^= values[s];
	m_hash ^= values[t];
}


void
Board::hashCastling(Index right)
{
	//M_ASSERT(m_castleRookAtStart[right] != Null);
	//M_ASSERT(rank(m_castleRookAtStart[right]) == Rank1 || rank(m_castleRookAtStart[right]) == Rank8);

	m_hash ^= rand64::Castling[right];

// we don't want to hash the castling rook square
//	m_hash ^= rand64::CastlingRook[m_castleRookAtStart[right]];
}


void Board::hashCastlingKingside(color::ID color)	{ hashCastling(kingSideIndex(color)); }
void Board::hashCastlingQueenside(color::ID color)	{ hashCastling(queenSideIndex(color)); }
void Board::hashToMove()									{ m_hash ^= rand64::ToMove; }


void
Board::hashCastling(color::ID color)
{
	if (canCastleShort(color))
		hashCastlingKingside(color);
	if (canCastleLong(color))
		hashCastlingQueenside(color);
}


void
Board::hashEnPassant()
{
	//M_ASSERT(m_epSquare != Null);
	m_hash ^= ::epSquareHashKey(m_epSquare);
}


bool
Board::isIntoCheck(Move const& move) const
{
	//////M_ASSERT(move);

	if (move.isNull())
		return false;

	Board peek(*this);
	peek.doMove(move);
	return peek.givesCheck();
}


bool
Board::givesMate() const
{
	if (!givesCheck())
		return false;

	MoveList moves;
	generateMoves(moves);

	for (unsigned i = 0; i < moves.size(); ++i)
	{
		if (!isIntoCheck(moves[i]))
			return false;
	}

	return true;
}


unsigned
Board::checkState() const
{
	unsigned state = NoCheck;

	switch (count(attacks(m_stm ^ 1, m_ksq[m_stm])))
	{
		case 0:  break;
		case 1:  state |= Check; break;
		default: state |= Check | DoubleCheck; break;
	}

	MoveList moves;
	generateMoves(moves);

	for (unsigned i = 0; i < moves.size(); ++i)
	{
		if (!isIntoCheck(moves[i]))
			return state;
	}

	return (state & Check) ? state | CheckMate : state | StaleMate;
}


uint64_t
Board::hashNoEP() const
{
	return m_epSquare == Null ? m_hash : m_hash ^ ::epSquareHashKey(m_epSquare);
}


void
Board::setEnPassantSquare(color::ID color, Square sq)
{
// we don't want this check here
//	////M_ASSERT(sq::rank(sq) == (color::isWhite(color) ? sq::Rank6 : sq::Rank3));

	if (PawnAttacks[color ^ 1][sq] & m_occupiedBy[color] & m_pawns)
	{
		m_epSquare = sq;
		hashEnPassant();
	}
	else
	{
		m_epSquare = Null;
	}

	m_epSquareFen = sq;
}


void
Board::setEnPassantFyle(color::ID color, Fyle fyle)
{
	setEnPassantSquare(color, sq::make(fyle, EpRank[color]));
}


void
Board::removeIllegalFrom(Move move, uint64_t& b) const
{
	typedef mstl::bitfield<uint64_t> BitField;

	BitField squares(b);

	for (unsigned sq = squares.find_first(); sq != BitField::npos; sq = squares.find_next(sq))
	{
		move.setFrom(sq);

		if (isIntoCheck(move))
			b &= ~setBit(sq);
	}
}


void
Board::removeIllegalTo(Move move, uint64_t& b) const
{
	typedef mstl::bitfield<uint64_t> BitField;

	BitField squares(b);

	for (unsigned sq = squares.find_first(); sq != BitField::npos; sq = squares.find_next(sq))
	{
		move.setTo(sq);

		if (isIntoCheck(move))
			b &= ~setBit(sq);
	}
}


Move&
Board::prepareForPrint(Move& move) const
{
	if (!move.isPrintable())
	{
		if (!move.isNull())
		{
			Board peek(*this);
			peek.doMove(move);

			unsigned state = NoCheck;

			if (peek.isInCheck())
			{
				move.setCheck();
				state |= Check;

				if (peek.checkState() & CheckMate)
				{
					move.setMate();
					state |= CheckMate;
				}
			}

			if (!move.isCastling())
			{
				int from	= move.from();
				int to	= move.to();

				if (m_piece[from] != piece::Pawn)
				{
					// we may need disambiguation
					uint64_t others = 0;

					switch (m_piece[from])
					{
						case piece::Knight:	others = m_knights & knightAttacks(to); break;
						case piece::Bishop:	others = m_bishops & bishopAttacks(to); break;
						case piece::Rook:		others = m_rooks & rookAttacks(to); break;
						case piece::Queen:	others = m_queens & queenAttacks(to); break;
						case piece::King:		others = m_kings & kingAttacks(to); break;
					}

					others ^= setBit(from);
					others &= m_occupiedBy[m_stm];

					// Do not disambiguate with moves that put oneself in check.
					if (others)
					{
						if (move.isLegal())
							removeIllegalFrom(move, others);

						if (state & (Check | CheckMate))
						{
							uint64_t movers = 0;

							switch (m_piece[from])
							{
								case piece::Knight:	movers = m_knights; break;
								case piece::Rook:		movers = m_rooks; break;
								case piece::Queen:	movers = m_queens; break;
								case piece::King:		movers = m_kings; break;

								case piece::Bishop:
									movers = m_bishops;
									if (color::isWhite(sq::color(sq::ID(from))))
										movers &= ::LiteSquares;
									else
										movers &= ::DarkSquares;
									break;
							}

							if (count(movers & m_occupiedBy[m_stm]) == 1)
							{
								// this is confusing if more than moving piece exists
								if (state & CheckMate)
									filterCheckMateMovesTo(move, others);
								else
									filterCheckMovesTo(move, others);
							}
						}

						if (others)
						{
							if (others & RankMask[::rank(from)])
								move.setNeedsFyle();

							if (others & FyleMask[::fyle(from)])
								move.setNeedsRank();
							else
								move.setNeedsFyle();
						}
					}

					// we may need disambiguation of destination square
					if (move.captured() != piece::None)
					{
						// case 1: more than one piece of same type which can capture
						// case 2: this piece can capture more pieces of this type
						uint64_t others[2] = { 0, 0 }; // otherwise gcc will complain

						switch (m_piece[from])
						{
							case piece::Knight:
								others[0] = m_knights & knightAttacks(to);
								others[1] = knightAttacks(from);
								break;

							case piece::Bishop:
								others[0] = m_bishops & bishopAttacks(to);
								others[1] = bishopAttacks(from);
								break;

							case piece::Rook:
								others[0] = m_rooks & rookAttacks(to);
								others[1] = rookAttacks(from);
								break;

							case piece::Queen:
								others[0] = m_queens & queenAttacks(to);
								others[1] = queenAttacks(from);
								break;

							case piece::King:
								others[0] = m_kings & kingAttacks(to);
								others[1] = kingAttacks(from);
								break;
						}

						others[0] ^= setBit(from);
						others[0] &= m_occupiedBy[m_stm];

						others[1] ^= setBit(to);
						others[1] &= m_occupiedBy[m_stm ^ 1];

						switch (m_piece[to])
						{
							case piece::Pawn:		others[1] &= m_pawns; break;
							case piece::Knight:	others[1] &= m_knights; break;
							case piece::Bishop:	others[1] &= m_bishops; break;
							case piece::Rook:		others[1] &= m_rooks; break;
							case piece::Queen:	others[1] &= m_queens; break;
						}

						if (move.isLegal())
						{
							if (others[0])
								removeIllegalFrom(move, others[0]);

							if (others[1])
								removeIllegalTo(move, others[1]);
						}

						// this may be confusing if more than one of captured piece exists
						if (state & (Check | CheckMate))
							filterCheckMovesFrom(move, others[1]);

						if (others[0] | others[1])
							move.setNeedsDestinationSquare();
					}
				}
				else if (m_piece[to] != piece::Empty || move.isEnPassant())
				{
					// we may need disambiguation of pawn captures
					if (pawnCapturesTo(to) ^ setBit(from))
						move.setNeedsFyle();
				}
			}
		}

		move.setPrintable();
	}

	return move;
}


bool
Board::isMovable(Square from, move::Constraint flag) const
{
	if (m_occupiedBy[m_stm] & setBit(from))
	{
		uint64_t squares = 0;

		switch (m_piece[from])
		{
			case piece::Pawn:		squares = pawnMovesFrom(from); break;
			case piece::Knight:	squares = knightAttacks(from); break;
			case piece::Bishop:	squares = bishopAttacks(from); break;
			case piece::Rook:		squares = rookAttacks(from); break;
			case piece::Queen:	squares = queenAttacks(from); break;
			case piece::King:		squares = kingAttacks(m_ksq[m_stm]); break;
		}

		squares &= ~m_occupiedBy[m_stm];

		while (squares)
		{
			if (prepareMove(from, lsbClear(squares), flag))
				return true;
		}
	}

	return false;
}


void
Board::setMoveNumber(unsigned number)
{
	// allow move number 0, many FEN's do use this
	m_plyNumber = mstl::mul2(mstl::max(1u, number) - 1) + (m_stm == Black);
}


bool
Board::setAt(Square s, piece::ID p)
{
	piece::Type pt = piece::type(p);

	if (pt == piece::None)
		return true;

	uint64_t bit = setBit(s);

	if (m_occupied & bit)
		removeAt(s);

	color::ID color = piece::color(p);

	switch (pt)
	{
		case piece::Pawn:
			if (rank(s) == Rank1 || rank(s) == Rank8)
				return false;
			hashPawn(s, p);
			m_pawns |= bit;
			++m_matCount[color].pawn;
			m_material.part[color].pawn |= m_material.part[color].pawn + 1;
			pawnProgressAdd(color, s);
			break;

		case piece::Knight:
			hashPiece(s, p);
			m_knights |= bit;
			++m_matCount[color].knight;
			m_material.part[color].knight |= m_material.part[color].knight + 1;
			break;

		case piece::Bishop:
			hashPiece(s, p);
			m_bishops |= bit;
			++m_matCount[color].bishop;
			m_material.part[color].bishop |= m_material.part[color].bishop + 1;
			break;

		case piece::Rook:
			hashPiece(s, p);
			m_rooks |= bit;
			++m_matCount[color].rook;
			m_material.part[color].rook |= m_material.part[color].rook + 1;
			break;

		case piece::Queen:
			hashPiece(s, p);
			m_queens |= bit;
			++m_matCount[color].queen;
			m_material.part[color].queen |= m_material.part[color].queen + 1;
			break;

		case piece::King:
			hashPiece(s, p);
			if (m_kings & m_occupiedBy[color])
				removeAt(m_ksq[color]);
			m_kings |= bit;
			m_ksq[color] = s;
			break;

		case piece::None:
			break; // error
	}

	m_piece[s] = pt;
	m_occupied ^= bit;
	m_occupiedBy[color] ^= bit;
	m_occupiedL90 ^= MaskL90[s];
	m_occupiedL45 ^= MaskL45[s];
	m_occupiedR45 ^= MaskR45[s];

	return true;
}


void
Board::removeAt(Square s)
{
	//////M_ASSERT(piece(s) != piece::Pawn || (rank(s) != Rank1 && rank(s) != Rank8));

	uint64_t bit = setBit(s);

	if (!(m_occupied & bit))
		return;

	color::ID color = m_occupiedBy[White] & bit ? White : Black;

	switch (m_piece[s])
	{
		case piece::Pawn:
			hashPiece(s, ::toPiece(piece::Pawn, color));
			m_pawns ^= bit;
			m_material.part[color].pawn = (1 << --m_matCount[color].pawn) - 1;
			pawnProgressRemove(color, s);
			break;

		case piece::Knight:
			hashPiece(s, ::toPiece(piece::Knight, color));
			m_knights ^= bit;
			m_material.part[color].knight = (1 << --m_matCount[color].knight) - 1;
			break;

		case piece::Bishop:
			hashPiece(s, ::toPiece(piece::Bishop, color));
			m_bishops ^= bit;
			m_material.part[color].bishop = (1 << --m_matCount[color].bishop) - 1;
			break;

		case piece::Rook:
			hashPiece(s, ::toPiece(piece::Rook, color));
			m_rooks ^= bit;
			m_material.part[color].rook = (1 << --m_matCount[color].rook) - 1;
			{
				Byte castling = m_destroyCastle[s];
				if (castling != 0xff)
				{
					if (m_castle & ~castling)
						hashCastling(Index(lsb(uint8_t(~castling))));
					m_castle &= castling;
				}
			}
			break;

		case piece::Queen:
			hashPiece(s, ::toPiece(piece::Queen, color));
			m_queens ^= bit;
			m_material.part[color].queen = (1 << --m_matCount[color].queen) - 1;
			break;

		case piece::King:
			hashPiece(s, ::toPiece(piece::King, color));
			m_kings ^= bit;
			m_ksq[color] = Null;
			hashCastling(color);
			destroyCastle(color);
			break;

		case piece::None:
			break; // error
	}

	m_piece[s] = piece::Empty;
	m_occupied ^= bit;
	m_occupiedBy[color] ^= bit;
	m_occupiedL90 ^= MaskL90[s];
	m_occupiedL45 ^= MaskL45[s];
	m_occupiedR45 ^= MaskR45[s];
}


void
Board::transpose()
{
	Board board(m_emptyBoard);

	for (unsigned i = 0; i < 64; ++i)
	{
		piece::ID	piece		= pieceAt(i);
		unsigned		square	= flipFyle(sq::ID(i));

		if (piece != piece::Empty)
			board.setAt(square, piece);

		board.m_destroyCastle[square] = m_destroyCastle[i];
	}

	board.m_stm = m_stm;
	board.m_epSquare = ::flipFyle(m_epSquare);
	board.m_epSquareFen = ::flipFyle(m_epSquareFen);
	board.m_halfMoveClock = m_halfMoveClock;
	board.m_plyNumber = m_plyNumber;
	board.m_castle = castling::transpose(m_castle);

	board.m_castleRookCurrent[WhiteKS] = ::flipFyle(m_castleRookCurrent[WhiteQS]);
	board.m_castleRookCurrent[WhiteQS] = ::flipFyle(m_castleRookCurrent[WhiteKS]);
	board.m_castleRookCurrent[BlackKS] = ::flipFyle(m_castleRookCurrent[BlackQS]);
	board.m_castleRookCurrent[BlackQS] = ::flipFyle(m_castleRookCurrent[BlackKS]);

	board.m_castleRookAtStart[WhiteKS] = ::flipFyle(m_castleRookAtStart[WhiteQS]);
	board.m_castleRookAtStart[WhiteQS] = ::flipFyle(m_castleRookAtStart[WhiteKS]);
	board.m_castleRookAtStart[BlackKS] = ::flipFyle(m_castleRookAtStart[BlackQS]);
	board.m_castleRookAtStart[BlackQS] = ::flipFyle(m_castleRookAtStart[BlackKS]);

	board.m_unambiguous[WhiteKS] = m_unambiguous[WhiteQS];
	board.m_unambiguous[WhiteQS] = m_unambiguous[WhiteKS];
	board.m_unambiguous[BlackKS] = m_unambiguous[BlackQS];
	board.m_unambiguous[BlackQS] = m_unambiguous[BlackKS];

	static_cast<Signature&>(board) = static_cast<Signature const&>(*this);
	static_cast<Signature&>(board).transpose();

	*this = board;

	if (blackToMove())
		hashToMove();
	if (m_epSquare != Null)
		hashEnPassant();
	hashCastling(White);
	hashCastling(Black);
}


bool
Board::shortCastlingWhiteIsLegal() const
{
	//M_ASSERT(m_castleRookCurrent[WhiteKS] != Null);

	uint64_t king = setBit(m_ksq[White]);
	uint64_t rook = setBit(m_castleRookCurrent[WhiteKS]);

	if (m_occupied & RankMask1 & (rook - 1) & ~(king - 1) & ~king)	// (king+1)...(rook-1)
		return false;

	if (m_ksq[White] >= g1)
		return !isAttackedBy(Black, (G1 | H1) & (king | (king - 1)));	// G1...king

	if (m_occupied & (A1 | B1 | C1 | D1 | E1 | F1 | G1) & ~(rook | (rook - 1)) & ~king)	// rook...G1
		return false;

	return !isAttackedBy(Black, (A1 | B1 | C1 | D1 | E1 | F1 | G1) & ~(king - 1));	// king...G1
}


bool
Board::shortCastlingBlackIsLegal() const
{
	//M_ASSERT(m_castleRookCurrent[BlackKS] != Null);

	uint64_t king = setBit(m_ksq[Black]);
	uint64_t rook = setBit(m_castleRookCurrent[BlackKS]);

	if (m_occupied & RankMask8 & (rook - 1) & ~(king - 1) & ~king)	// (king+1)...(rook-1)
		return false;

	if (m_ksq[Black] >= g8)
		return !isAttackedBy(White, (G8 | H8) & (king | (king - 1)));	// G8...king

	if (m_occupied & (A8 | B8 | C8 | D8 | E8 | F8 | G8) & ~(rook | (rook - 1)) & ~king)	// rook...G8
		return false;

	return !isAttackedBy(White, (A8 | B8 | C8 | D8 | E8 | F8 | G8) & ~(king - 1));	// king...G8
}


bool
Board::longCastlingWhiteIsLegal() const
{
	//M_ASSERT(m_castleRookCurrent[WhiteQS] != Null);

	uint64_t king = setBit(m_ksq[White]);
	uint64_t rook = setBit(m_castleRookCurrent[WhiteQS]);

	if (m_occupied & RankMask1 & (king - 1) & ~(rook - 1) & ~rook)	// (rook+1)...(king-1)
		return false;

	if (m_ksq[White] <= c1)
	{
		if (m_occupied & (A1 | B1 | C1 | D1) & ~(king - 1) & ~king)	// (king+1)...D1
			return false;

		return !isAttackedBy(Black, (A1 | B1 | C1) & ~(king - 1));	// king...C1
	}

	return !isAttackedBy(Black, (C1 | D1 | E1 | F1 | G1 | H1) & (king | (king - 1)));	// C8...king
}


bool
Board::longCastlingBlackIsLegal() const
{
	//M_ASSERT(m_castleRookCurrent[BlackQS] != Null);

	uint64_t king = setBit(m_ksq[Black]);
	uint64_t rook = setBit(m_castleRookCurrent[BlackQS]);

	if (m_occupied & RankMask8 & (king - 1) & ~(rook - 1) & ~rook)	// (rook+1)...(king-1)
		return false;

	if (m_ksq[Black] <= c8)
	{
		if (m_occupied & (A8 | B8 | C8 | D8) & ~(king - 1) & ~king)	// (king+1)...D8
			return false;

		return !isAttackedBy(White, (A8 | B8 | C8) & ~(king - 1));	// king...C8
	}

	return !isAttackedBy(White, (C8 | D8 | E8 | F8 | G8 | H8) & (king | (king - 1)));	// C8...king
}


bool
Board::shortCastlingWhiteIsPossible() const
{
	//M_ASSERT(m_castleRookCurrent[WhiteKS] != Null);

	uint64_t king = setBit(m_ksq[White]);
	uint64_t rook = setBit(m_castleRookCurrent[WhiteKS]);

	if (m_occupied & (A1 | B1 | C1 | D1 | E1 | F1 | G1) & ~(rook | (rook - 1)) & ~king)	// rook...G1
		return false;

	return !(m_occupied & RankMask1 & (rook - 1) & ~(king - 1) & ~king);	// (king+1)...(rook-1)
}


bool
Board::shortCastlingBlackIsPossible() const
{
	//M_ASSERT(m_castleRookCurrent[BlackKS] != Null);

	uint64_t king = setBit(m_ksq[Black]);
	uint64_t rook = setBit(m_castleRookCurrent[BlackKS]);

	if (m_occupied & (A8 | B8 | C8 | D8 | E8 | F8 | G8) & ~(rook | (rook - 1)) & ~king)	// rook...G8
		return false;

	return !(m_occupied & RankMask8 & (rook - 1) & ~(king - 1) & ~king);	// (king+1)...(rook-1)
}


bool
Board::longCastlingWhiteIsPossible() const
{
	//M_ASSERT(m_castleRookCurrent[WhiteQS] != Null);

	uint64_t king = setBit(m_ksq[White]);
	uint64_t rook = setBit(m_castleRookCurrent[WhiteQS]);

	if (m_occupied & (A1 | B1 | C1 | D1) & ~(king - 1) & ~king)	// (king+1)...D1
		return false;

	return !(m_occupied & RankMask1 & (king - 1) & ~(rook - 1) & ~rook);	// (rook+1)...(king-1)
}


bool
Board::longCastlingBlackIsPossible() const
{
	//M_ASSERT(m_castleRookCurrent[BlackQS] != Null);

	uint64_t king = setBit(m_ksq[Black]);
	uint64_t rook = setBit(m_castleRookCurrent[BlackQS]);

	if (m_occupied & (A8 | B8 | C8 | D8) & ~(king - 1) & ~king)	// (king+1)...D8
		return false;

	return !(m_occupied & RankMask8 & (king - 1) & ~(rook - 1) & ~rook);	// (rook+1)...(king-1)
}


Board::SetupStatus
Board::validate(variant::Type variant, Handicap handicap, move::Constraint flag) const
{
	// Pawns on 1st or 8th (although it cannot happen)
	if (pawns(White) & RankMask8 || pawns(Black) & RankMask1)
		return PawnsOn18;

	// Exactly one king per side
	if (m_ksq[White] == Null)	return NoWhiteKing;
	if (m_ksq[Black] == Null)	return NoBlackKing;
	if (count(m_kings) > 2)		return TooManyKings;	// cannot happen

	// No more than 8 pawns per side
	if (count(pawns(White)) > 8)	return TooManyWhitePawns;
	if (count(pawns(Black)) > 8)	return TooManyBlackPawns;

	// Maximum 16 pieces per side
	if (count(pieces(White)) > 16)	return TooManyWhite;
	if (count(pieces(Black)) > 16)	return TooManyBlack;

	// Too many queens, rooks, bishops, or knights?
	if (	count(queens (White) | pawns(White)) > 9
		|| count(rooks  (White) | pawns(White)) > 10
		|| count(bishops(White) | pawns(White)) > 10
		|| count(knights(White) | pawns(White)) > 10)
	{
		return TooManyWhitePieces;
	}
	if (	count(queens(Black)  | pawns(Black)) > 9
		|| count(rooks (Black)  | pawns(Black)) > 10
		|| count(bishops(Black) | pawns(Black)) > 10
		|| count(knights(Black) | pawns(Black)) > 10)
	{
		return TooManyBlackPieces;
	}

	if (flag == move::DontAllowIllegalMove)
	{
		// Bad checks
		if (givesCheck())
		{
			if (isInCheck())
				return BothInCheck;

			return OppositeCheck;
		}
	}

	// Detect multi pawn checks.
	uint64_t attackers = attacks(m_stm ^ 1, m_ksq[m_stm]);

	if (count(attackers & m_pawns) >= 2)
		return MultiPawnCheck;

	// Detect triple checks.
	if (count(attackers) >= 3)
		return TripleCheck;

	// Detect unreasonable ep square
	if (	m_epSquareFen != Null
		&& (	(m_stm == White && (m_epSquareFen < a6 || m_epSquareFen > h6))
			|| (m_stm == Black && (m_epSquareFen < a3 || m_epSquareFen > h3))
			|| m_occupied & setBit(m_epSquareFen)
			|| m_occupied & PawnF1[m_stm][m_epSquareFen]))
//			|| !enPassantMoveExists(m_stm)
//			|| !(PawnF1[m_stm ^ 1][m_epSquareFen] & m_pawns & m_occupiedBy[m_stm ^ 1]))
	{
		return InvalidEnPassant;
	}

	// Can't castle if rook field is occupied by another piece
	// (in standard chess we allow castling with missing rook for historical reasons (handicap games))
	if (	(m_queens  & m_standardBoard.m_queens ) != m_queens
		|| (m_rooks   & m_standardBoard.m_rooks  ) != m_rooks
		|| (m_bishops & m_standardBoard.m_bishops) != m_bishops
		|| (m_knights & m_standardBoard.m_knights) != m_knights)
	{
		if (	(m_castle & WhiteQueenside)
			&& !(rooks(White) & RankMask1 & (king(White) - 1))
			&& m_piece[a1] != piece::None)
		{
			return BadCastlingRights;
		}
		if (	(m_castle & WhiteKingside)
			&& !(rooks(White) & RankMask1 & ~((king(White) - 1) << 1))
			&& m_piece[h1] != piece::None)
		{
			return BadCastlingRights;
		}
		if (	(m_castle & BlackQueenside)
			&& !(rooks(Black) & RankMask8 & (king(Black) - 1))
			&& m_piece[a8] != piece::None)
		{
			return BadCastlingRights;
		}
		if (	(m_castle & BlackKingside)
			&& !(rooks(Black) & RankMask8 & ~((king(Black) - 1) << 1))
			&& m_piece[h8] != piece::None)
		{
			return BadCastlingRights;
		}
	}

	// Can't castle if king has moved
	if (canCastle(White) && ::rank(m_ksq[White]) != Rank1)
		return BadCastlingRights;
	if (canCastle(Black) && ::rank(m_ksq[Black]) != Rank8)
		return BadCastlingRights;

	if (notDerivableFromChess960())
		return BadCastlingRights;

	// Detect unreasonable rook squares (for castling)
	// (in standard chess we allow castling with missing rook for historical reasons (handicap games))
	{
		if (	((m_castle & WhiteQueenside) && rank(m_castleRookCurrent[WhiteQS]) != Rank1)
			|| ((m_castle & WhiteKingside ) && rank(m_castleRookCurrent[WhiteKS]) != Rank1)
			|| ((m_castle & BlackQueenside) && rank(m_castleRookCurrent[BlackQS]) != Rank8)
			|| ((m_castle & BlackKingside ) && rank(m_castleRookCurrent[BlackKS]) != Rank8))
		{
			return InvalidCastlingRights;
		}

		if (	(m_castle & WhiteKingside)
			&& (m_castle & BlackKingside)
			&& fyle(m_castleRookCurrent[WhiteKS]) != fyle(m_castleRookCurrent[BlackKS]))
		{
			return InvalidCastlingRights;
		}

		if (	(m_castle & WhiteQueenside)
			&& (m_castle & BlackQueenside)
			&& fyle(m_castleRookCurrent[WhiteQS]) != fyle(m_castleRookCurrent[BlackQS]))
		{
			return InvalidCastlingRights;
		}

		uint64_t whiteKS = m_castleRookCurrent[WhiteKS] == Null ? 0 : setBit(m_castleRookCurrent[WhiteKS]);
		uint64_t whiteQS = m_castleRookCurrent[WhiteQS] == Null ? 0 : setBit(m_castleRookCurrent[WhiteQS]);
		uint64_t blackKS = m_castleRookCurrent[BlackKS] == Null ? 0 : setBit(m_castleRookCurrent[BlackKS]);
		uint64_t blackQS = m_castleRookCurrent[BlackQS] == Null ? 0 : setBit(m_castleRookCurrent[BlackQS]);

		uint64_t whiteRooks = rooks(White);
		uint64_t blackRooks = rooks(Black);

		if (handicap == DontAllowHandicap || notDerivableFromStandardChess())
		{
			// Cannot castle without rook
			if (	((m_castle & WhiteKingside ) && !(whiteKS & whiteRooks))
				|| ((m_castle & BlackKingside ) && !(blackKS & blackRooks))
				|| ((m_castle & WhiteQueenside) && !(whiteQS & whiteRooks))
				|| ((m_castle & BlackQueenside) && !(blackQS & blackRooks)))
			{
				return BadCastlingRights;
			}
		}
		else
		{
			if (mstl::bf::count_bits(whiteRooks) == 2 && mstl::bf::count_bits(pawns(White)) == 8)
			{
				if (	((m_castle & WhiteKingside ) && !(whiteKS & whiteRooks))
					|| ((m_castle & WhiteQueenside) && !(whiteQS & whiteRooks)))
				{
					return BadCastlingRights;	// cannot be a handicap game
				}
			}

			if (mstl::bf::count_bits(blackRooks) == 2 && mstl::bf::count_bits(pawns(Black)) == 8)
			{
				if (	((m_castle & BlackKingside ) && !(blackKS & blackRooks))
					|| ((m_castle & BlackQueenside) && !(blackQS & blackRooks)))
				{
					return BadCastlingRights;	// cannot be a handicap game
				}
			}
		}

//		if (isStartPosition() && !isChess960Position() && m_castle)
//		{
//			// we do not allow start positions with castle rights except
//			// the start position is a Chess 960 position.
//			return InvalidStartPosition;
//		}

		if (variant != variant::Standard)
		{
			uint64_t mask = setBit(m_ksq[White]) - 1;

			if (	rank(m_ksq[White]) == Rank1
				&& (	(	(m_castle & WhiteKingside)
						&& !m_unambiguous[WhiteKS]
						&& count(whiteRooks & ~mask & RankMask1) > 1)
					|| (	(m_castle & WhiteQueenside)
						&& !m_unambiguous[WhiteQS]
						&& count(whiteRooks & mask & RankMask1) > 1)))
			{
				return AmbiguousCastlingFyles;
			}

			mask = setBit(m_ksq[Black]) - 1;

			if (	rank(m_ksq[Black]) == Rank8
				&& (	(	(m_castle & BlackKingside)
						&& !m_unambiguous[BlackKS]
						&& count(blackRooks & ~mask & RankMask8) > 1)
					|| (	(m_castle & BlackQueenside)
						&& !m_unambiguous[BlackQS]
						&& count(blackRooks & mask & RankMask8) > 1)))
			{
				return AmbiguousCastlingFyles;
			}
		}
	}

	return Valid;
}


bool
Board::isValidFen(char const* fen, variant::Type variant, Handicap handicap, move::Constraint flag)
{
	Board board;
	return board.setup(fen) && board.validate(variant::Unknown, handicap, flag) == Valid;
}


void
Board::setCastleShort(color::ID color, unsigned square)
{
	//M_ASSERT(square != Null);
	//M_ASSERT(sq::rank(square) == HomeRank[color]);

	Byte rights	= kingSide(color);
	Byte index	= ::kingSideIndex(color);

	m_castleRookCurrent[index] = m_castleRookAtStart[index] = square;
	m_destroyCastle[square] = ~rights;

	if (!(m_castle & rights))
	{
		m_castle |= rights;
		hashCastlingKingside(color);
	}
}


void
Board::setCastleLong(color::ID color, unsigned square)
{
	//M_ASSERT(square != Null);
	//M_ASSERT(sq::rank(square) == HomeRank[color]);

	Byte rights	= queenSide(color);
	Byte index	= ::queenSideIndex(color);

	m_castleRookCurrent[index] = m_castleRookAtStart[index] = square;
	m_destroyCastle[square] = ~rights;

	if (!(m_castle & rights))
	{
		m_castle |= rights;
		hashCastlingQueenside(color);
	}
}


void
Board::tryCastleShort(color::ID color)
{
	Byte rights = kingSide(color);

	if ((m_castling & rights) == 0)
	{
		Square sq = m_castleRookAtStart[::kingSideIndex(color)];

		if (sq != Null)
		{
			m_destroyCastle[sq] = ~rights;
			m_castleRookCurrent[::kingSideIndex(color)] = sq;

			if (!(m_castle & rights))
			{
				m_castle |= rights;
				hashCastlingKingside(color);
			}
		}
	}
}


void
Board::tryCastleLong(color::ID color)
{
	Byte rights = queenSide(color);

	if ((m_castling & rights) == 0)
	{
		Square sq = m_castleRookAtStart[::queenSideIndex(color)];

		if (sq != Null)
		{
			m_destroyCastle[sq] = ~rights;
			m_castleRookCurrent[::queenSideIndex(color)] = sq;

			if (!(m_castle & rights))
			{
				m_castle |= rights;
				hashCastlingQueenside(color);
			}
		}
	}
}


Square
Board::shortCastlingRook(color::ID color) const
{
	if (::rank(m_ksq[color]) != HomeRank[color])
		return Null;

	uint64_t	rooks = this->rooks(color) & HomeRankMask[color];
	Square	square;

	if (rooks)
	{
		square = msb(rooks);

		if (square > m_ksq[color])
			return square;
	}

	if (::fyle(m_ksq[color]) != FyleE)
		return Null;

	// NOTE: in handicap games the rook is probably missing (only allowed in standard chess)
	square = color::isWhite(color) ? h1 : h8;
	return m_occupied & setBit(square) ? Null : square;
}


Square
Board::longCastlingRook(color::ID color) const
{
	if (::rank(m_ksq[color]) != HomeRank[color])
		return Null;

	uint64_t	rooks = this->rooks(color) & HomeRankMask[color];
	Square	square;

	if (rooks)
	{
		square = lsb(rooks);

		if (square < m_ksq[color])
			return square;
	}

	if (::fyle(m_ksq[color]) != FyleE)
		return Null;

	// NOTE: in handicap games the rook is probably missing (only allowed in standard chess)
	square = color::isWhite(color) ? a1 : a8;
	return m_occupied & setBit(square) ? Null : square;
}


void
Board::setCastleShort(color::ID color)
{
	unsigned square = shortCastlingRook(color);

	if (square != Null)
		setCastleShort(color, square);
}


void
Board::setCastleLong(color::ID color)
{
	unsigned square = longCastlingRook(color);

	if (square != Null)
		setCastleLong(color, square);
}


void
Board::removeCastlingRights(castling::Index index)
{
	if (m_castle & (1 << index))
	{
		hashCastling(index);
		m_destroyCastle[m_castleRookCurrent[index]] = 0xff;
		m_castleRookCurrent[index] = Null;
		m_castleRookAtStart[index] = Null;
		m_unambiguous[index] = false;
	}
}


void
Board::removeCastlingRights(color::ID color)
{
	removeCastlingRights(kingSideIndex(color));
	removeCastlingRights(queenSideIndex(color));
}


void
Board::removeCastlingRights()
{
	removeCastlingRights(WhiteKS);
	removeCastlingRights(WhiteQS);
	removeCastlingRights(BlackKS);
	removeCastlingRights(BlackQS);
}


void
Board::removeCastlingRights(Square rook)
{
	//////M_ASSERT(piece(rook) == piece::Rook);

	Byte castling = m_destroyCastle[rook];

	if (m_castle & ~castling)
		removeCastlingRights(Index(lsb(uint8_t(~castling))));
}


void
Board::setCastlingFyle(color::ID color, Fyle fyle)
{
	// IMPORTANT NOTE: This function is not updating the hash code.

	Square	sq	= sq::make(fyle, HomeRank[color]);
	Byte		i	= sq < m_ksq[color] ? queenSideIndex(color) : kingSideIndex(color);

	m_castleRookCurrent[i] = m_castleRookAtStart[i] = sq;
	m_destroyCastle[sq] = ~(1 << i);
	m_unambiguous[i] = true;
}


void
Board::setCastlingRights(castling::Rights rights)
{
	// IMPORTANT NOTE:
	// This function is for validation only.
	// The board is now in an inconsistent state.

	m_castle |= rights;
}


void
Board::fixBadCastlingRights()
{
	// NOTE: usable only for standard chess positions.
	// NOTE: mainly used to fix bad FEN's from Scid.

	if (m_ksq[White] != e1)
	{
		hashCastling(White);
		m_castle &= ~WhiteBothSides;
		m_destroyCastle[WhiteKS] = 0xff;
		m_destroyCastle[WhiteQS] = 0xff;
		m_castleRookCurrent[WhiteKS] = m_castleRookAtStart[WhiteKS] = Null;
		m_castleRookCurrent[WhiteQS] = m_castleRookAtStart[WhiteQS] = Null;
	}
	else
	{
		uint64_t whiteRooks = rooks(White);

		if (!(whiteRooks & H1))
		{
			if (m_castle & WhiteKingside)
			{
				unsigned nrooks	= count(whiteRooks);
				unsigned npawns	= count(pawns(White));
				unsigned npieces	= count(pieces(White));

				if ((nrooks == 2 && npawns == 8) || npieces + npawns >= 16)
				{
					// cannot be a handicap game
					m_castleRookAtStart[WhiteKS] = shortCastlingRook(White);
					hashCastlingKingside(White);
					m_castle &= ~WhiteKingside;
					m_destroyCastle[WhiteKS] = 0xff;
				}
			}

			m_castleRookCurrent[WhiteKS] = m_castleRookAtStart[WhiteKS] = Null;
		}

		if (!(whiteRooks & A1))
		{
			if (m_castle & WhiteQueenside)
			{
				unsigned nrooks	= count(whiteRooks);
				unsigned npawns	= count(pawns(White));
				unsigned npieces	= count(pieces(White));

				if ((nrooks == 2 && npawns == 8) || npieces + npawns >= 16)
				{
					// cannot be a handicap game
					m_castleRookAtStart[WhiteQS] = longCastlingRook(White);
					hashCastlingQueenside(White);
					m_castle &= ~WhiteQueenside;
					m_destroyCastle[WhiteQS] = 0xff;
				}
			}

			m_castleRookCurrent[WhiteQS] = m_castleRookAtStart[WhiteQS] = Null;
		}
	}

	if (m_ksq[Black] != e8)
	{
		hashCastling(Black);
		m_castle &= ~BlackBothSides;
		m_destroyCastle[BlackKS] = 0xff;
		m_destroyCastle[BlackQS] = 0xff;
		m_castleRookCurrent[BlackKS] = m_castleRookAtStart[BlackKS] = Null;
		m_castleRookCurrent[BlackQS] = m_castleRookAtStart[BlackQS] = Null;
	}
	else
	{
		uint64_t blackRooks = rooks(Black);

		if (!(blackRooks & H8))
		{
			if (m_castle & BlackKingside)
			{
				unsigned nrooks	= count(blackRooks);
				unsigned npawns	= count(pawns(Black));
				unsigned npieces	= count(pieces(Black));

				if ((nrooks == 2 && npawns == 8) || npieces + npawns >= 16)
				{
					// cannot be a handicap game
					m_castleRookAtStart[BlackKS] = shortCastlingRook(Black);
					hashCastlingKingside(Black);
					m_castle &= ~BlackKingside;
					m_destroyCastle[BlackKS] = 0xff;
				}
			}

			m_castleRookCurrent[BlackKS] = m_castleRookAtStart[BlackKS] = Null;
		}

		if (!(blackRooks & A8))
		{
			if (m_castle & BlackQueenside)
			{
				unsigned nrooks	= count(blackRooks);
				unsigned npawns	= count(pawns(Black));
				unsigned npieces	= count(pieces(Black));

				if ((nrooks == 2 && npawns == 8) || npieces + npawns >= 16)
				{
					// cannot be a handicap game
					m_castleRookAtStart[BlackQS] = longCastlingRook(Black);
					hashCastlingQueenside(Black);
					m_castle &= ~BlackQueenside;
					m_destroyCastle[BlackQS] = 0xff;
				}
			}

			m_castleRookCurrent[BlackQS] = m_castleRookAtStart[BlackQS] = Null;
		}
	}

	::memset(m_unambiguous, true, sizeof(m_unambiguous));
}


bool
Board::setup(char const* fen)
{
	// Piece position
	unsigned	s = 56;

	clear();

	for ( ; *fen && *fen != ' '; ++fen)
	{
		if (*fen == '/')
		{
			s -= 16;
		}
		else if (::isdigit(*fen))
		{
			s += *fen - '0';
		}
		else if (s > 63)
		{
			return false;
		}
		else
		{
			m_occupiedL90 |= MaskL90[s];
			m_occupiedL45 |= MaskL45[s];
			m_occupiedR45 |= MaskR45[s];

			switch (*fen)
			{
				case 'p':
					if ((1 << ::rank(s)) & ((1 << Rank1) | (1 << Rank8)))
						return false;
					hashPawn(s, piece::BlackPawn);
					m_piece[s] = piece::Pawn;
					m_pawns |= setBit(s);
					m_occupiedBy[Black] |= setBit(s);
					++m_matCount[Black].pawn;
					m_material.part[Black].pawn |= m_material.part[Black].pawn + 1;
					m_progress.side[Black].add(::flipRank(s));
					++s;
					break;

				case 'n':
					hashPiece(s, piece::BlackKnight);
					m_piece[s] = piece::Knight;
					m_knights |= setBit(s);
					m_occupiedBy[Black] |= setBit(s);
					++m_matCount[Black].knight;
					m_material.part[Black].knight |= m_material.part[Black].knight + 1;
					++s;
					break;

				case 'b':
					hashPiece(s, piece::BlackBishop);
					m_piece[s] = piece::Bishop;
					m_bishops |= setBit(s);
					m_occupiedBy[Black] |= setBit(s);
					++m_matCount[Black].bishop;
					m_material.part[Black].bishop |= m_material.part[Black].bishop + 1;
					++s;
					break;

				case 'r':
					hashPiece(s, piece::BlackRook);
					m_piece[s] = piece::Rook;
					m_rooks |= setBit(s);
					m_occupiedBy[Black] |= setBit(s);
					++m_matCount[Black].rook;
					m_material.part[Black].rook |= m_material.part[Black].rook + 1;
					++s;
					break;

				case 'q':
					hashPiece(s, piece::BlackQueen);
					m_piece[s] = piece::Queen;
					m_queens |= setBit(s);
					m_occupiedBy[Black] |= setBit(s);
					++m_matCount[Black].queen;
					m_material.part[Black].queen |= m_material.part[Black].queen + 1;
					++s;
					break;

				case 'k':
					if (m_ksq[Black] != Null)
						return false;
					hashPiece(s, piece::BlackKing);
					m_piece[s] = piece::King;
					m_kings |= setBit(s);
					m_occupiedBy[Black] |= setBit(s);
					m_ksq[Black] = s;
					++s;
					break;

				case 'P':
					if ((1 << ::rank(s)) & ((1 << Rank1) | (1 << Rank8)))
						return false;
					hashPawn(s, piece::WhitePawn);
					m_piece[s] = piece::Pawn;
					m_pawns |= setBit(s);
					m_occupiedBy[White] |= setBit(s);
					++m_matCount[White].pawn;
					m_material.part[White].pawn |= m_material.part[White].pawn + 1;
					m_progress.side[White].add(s);
					++s;
					break;

				case 'N':
					hashPiece(s, piece::WhiteKnight);
					m_piece[s] = piece::Knight;
					m_knights |= setBit(s);
					m_occupiedBy[White] |= setBit(s);
					++m_matCount[White].knight;
					m_material.part[White].knight |= m_material.part[White].knight + 1;
					++s;
					break;

				case 'B':
					hashPiece(s, piece::WhiteBishop);
					m_piece[s] = piece::Bishop;
					m_bishops |= setBit(s);
					m_occupiedBy[White] |= setBit(s);
					++m_matCount[White].bishop;
					m_material.part[White].bishop |= m_material.part[White].bishop + 1;
					++s;
					break;

				case 'R':
					hashPiece(s, piece::WhiteRook);
					m_piece[s] = piece::Rook;
					m_rooks |= setBit(s);
					m_occupiedBy[White] |= setBit(s);
					++m_matCount[White].rook;
					m_material.part[White].rook |= m_material.part[White].rook + 1;
					++s;
					break;

				case 'Q':
					hashPiece(s, piece::WhiteQueen);
					m_piece[s] = piece::Queen;
					m_queens |= setBit(s);
					m_occupiedBy[White] |= setBit(s);
					++m_matCount[White].queen;
					m_material.part[White].queen |= m_material.part[White].queen + 1;
					++s;
					break;

				case 'K':
					if (m_ksq[White] != Null)
						return false;
					hashPiece(s, piece::WhiteKing);
					m_piece[s] = piece::King;
					m_kings |= setBit(s);
					m_occupiedBy[White] |= setBit(s);
					m_ksq[White] = s;
					++s;
					break;

				default:
					return false;
			}
		}
	}

	if (s != 8)
		return false;

	// Set remainder of board data appropriately
	m_occupied = m_occupiedBy[White] | m_occupiedBy[Black];

	while (*fen == ' ')
		++fen;

	if (!*fen)
		return true;

	// Side to move
	switch (*fen++)
	{
		case 'w':	m_stm = White; break;
		case 'b':	m_stm = Black; hashToMove(); break;
		default:		return false;
	}

	while (*fen == ' ')
		++fen;

	if (!*fen)
		return true;

	// Castling Rights
	if (*fen == '-')
	{
		++fen;
	}
	else
	{
		for ( ; *fen != ' '; ++fen)
		{
			switch (*fen)
			{
				case 'A' ... 'H':
					{
						Byte fyle	= ::toFYLE(*fen);
						Byte square	= sq::make(fyle, Rank1);

						if (fyle < ::fyle(m_ksq[White]))
						{
							setCastleLong(White, square);
							m_unambiguous[WhiteQS] = true;
						}
						else
						{
							setCastleShort(White, square);
							m_unambiguous[WhiteKS] = true;
						}
					}
					break;

				case 'a' ... 'h':
					{
						Byte fyle	= ::toFyle(*fen);
						Byte square	= sq::make(fyle, Rank8);

						if (fyle < ::fyle(m_ksq[Black]))
						{
							setCastleLong(Black, square);
							m_unambiguous[BlackQS] = true;
						}
						else
						{
							setCastleShort(Black, square);
							m_unambiguous[BlackKS] = true;
						}
					}
					break;

				case 'K': setCastleShort(White); break;
				case 'k': setCastleShort(Black); break;
				case 'Q': setCastleLong(White);  break;
				case 'q': setCastleLong(Black);  break;

				default: return false;
			}
		}
	}

	while (*fen == ' ')
		++fen;

	if (!*fen)
		return true;

	// En Passant Square
	m_epSquareFen = m_epSquare = Null;
	char c = ::tolower(*fen++);

	if (c != '-')
	{
		if (!::isFyle(c) || !::isRank(*fen))
			return false;

		setEnPassantSquare(sq::make(::toFyle(c), ::toRank(*fen++)));
	}

	while (*fen == ' ')
		++fen;

	if (!*fen)
		return true;

	// Half move clock
	if (!::isdigit(*fen))
		return false;
	m_halfMoveClock = ::strtoul(fen, const_cast<char**>(&fen), 10);

	while (*fen == ' ')
		++fen;

	if (!*fen)
		return true;

	// Move number
	if (!::isdigit(*fen))
		return false;
	unsigned moveNo = ::strtoul(fen, nullptr, 10);
	if (moveNo & (~unsigned(0) << 12))
		moveNo = 0;	// silently fix broken move numbers (Scid's sg3/sg4 may contain broken FEN's)
	setMoveNumber(moveNo);

	// IMPORTANT NOTE:
	// The FEN has one weakness:
	// it does not provide information for detecting 3-fold repetition.

	return true;
}


void
Board::setup(ExactPosition const& position)
{
	// IMPORTANT NOTE: The information in 'position' is not sufficient
	// to build a consistent board. The resulting board should not be
	// used for playing or validation.

	clear();

	for (unsigned color = 0; color < 2; ++color)
	{
		for (unsigned sq = 0; sq < 64; ++sq)
		{
			uint64_t mask = setBit(sq);

			if (position.m_occupiedBy[color] & mask)
			{
				piece::Type piece;

				if (position.m_pawns & mask)
				{
					piece = piece::Pawn;
				}
				else if (position.m_knights & mask)
				{
					piece = piece::Knight;
				}
				else if (position.m_bishops & mask)
				{
					piece = piece::Bishop;
				}
				else if (position.m_rooks & mask)
				{
					piece = piece::Rook;
				}
				else if (position.m_queens & mask)
				{
					piece = piece::Queen;
				}
				else
				{
					piece = piece::King;
					m_ksq[color] = sq;
				}

				setAt(sq, piece::piece(piece, color::ID(color)));
			}
		}
	}

	m_castleRookAtStart[castling::WhiteQS] = a1;
	m_castleRookAtStart[castling::WhiteKS] = h1;
	m_castleRookAtStart[castling::BlackQS] = a8;
	m_castleRookAtStart[castling::BlackKS] = h8;

	::memcpy(m_castleRookCurrent, position.m_castleRookCurrent, sizeof(m_castleRookCurrent));

	for (unsigned i = 0; i < 4; ++i)
	{
		Square sq = m_castleRookCurrent[i];

		//M_ASSERT((sq == Null) == ((position.m_castle & (1 << i)) == 0));

		if (sq == sq::Null)
			hashCastling(Index(i));
		else
			m_castleRookAtStart[i] = sq;
	}

	::memset(m_unambiguous, true, sizeof(m_unambiguous));

	if ((m_stm = position.m_stm) == Black)
		hashToMove();

	if ((m_epSquare = position.m_epSquare) != Null)
		hashEnPassant();

	m_castle = position.m_castle;
	m_epSquareFen = position.m_epSquare;
}


bool
Board::notDerivableFromStandardChess() const
{
	return	((m_castle & WhiteKingside ) && (m_ksq[White] != e1 || m_castleRookCurrent[WhiteKS] != h1))
			|| ((m_castle & BlackKingside ) && (m_ksq[Black] != e8 || m_castleRookCurrent[BlackKS] != h8))
			|| ((m_castle & WhiteQueenside) && (m_ksq[White] != e1 || m_castleRookCurrent[WhiteQS] != a1))
			|| ((m_castle & BlackQueenside) && (m_ksq[Black] != e8 || m_castleRookCurrent[BlackQS] != a8));
}


bool
Board::notDerivableFromChess960() const
{
	return	((m_castle & WhiteKingside ) && m_castleRookCurrent[WhiteKS] < m_ksq[White])
			|| ((m_castle & BlackKingside ) && m_castleRookCurrent[BlackKS] < m_ksq[Black])
			|| ((m_castle & WhiteQueenside) && m_castleRookCurrent[WhiteQS] > m_ksq[White])
			|| ((m_castle & BlackQueenside) && m_castleRookCurrent[BlackQS] > m_ksq[Black]);
}


bool
Board::checkShuffleChessPosition() const
{
	return		isStartPosition()
				// check whether all white and black pieces are in opposition
				&& knights(White) == (knights(Black) >> a8)
				&& bishops(White) == (bishops(Black) >> a8)
				&& rooks(White)   == (rooks(Black)   >> a8)
				&& queens(White)  == (queens(Black)  >> a8)
				// check whether the bishops have opposite colors
				&& hasBishopOnLite(White) && hasBishopOnDark(White);
}


bool
Board::isStartPosition() const
{
	if (m_epSquareFen != Null || m_stm == Black)
		return false;

	return	// check material: KQRRBBNN
					m_matCount[White].value == m_shuffleChessBoard.m_matCount[White].value
				&& m_matCount[Black].value == m_shuffleChessBoard.m_matCount[Black].value
				// all white/black pawns are on 2nd/7th rank?
				&& pawns(White) == RankMask2
				&& pawns(Black) == RankMask7
				// all white/black pieces are on 1st/8th rank?
				&& (pieces(White) & ~m_pawns) == RankMask1
				&& (pieces(Black) & ~m_pawns) == RankMask8;
}


bool
Board::isChess960Position() const
{
	return	m_castle == AllRights
			&& checkShuffleChessPosition()
			// check whether king is between the rooks
			&& lsb(rooks(White)) < m_ksq[White] && m_ksq[White] < msb(rooks(White));
}


bool
Board::isShuffleChessPosition() const
{
	return m_castle == NoRights ? checkShuffleChessPosition() : isChess960Position();
}


unsigned
Board::computeIdn() const
{
#define __ -1
	static int8_t const BishopTable[8][8] =
	{
		{ __,  0, __,  1, __,  2, __,  3 },
		{ __, __,  4, __,  8, __, 12, __ },
		{ __, __, __,  5, __,  6, __,  7 },
		{ __, __, __, __,  9, __, 13, __ },
		{ __, __, __, __, __, 10, __, 11 },
		{ __, __, __, __, __, __, 14, __ },
		{ __, __, __, __, __, __, __, 15 },
		{ __, __, __, __, __, __, __, __ },
	};
#undef __
#define _  -1
	static int8_t const N5NTable[5][5] =
	{
		{ _, 0, 1, 2, 3 },
		{ _, _, 4, 5, 6 },
		{ _, _, _, 7, 8 },
		{ _, _, _, _, 9 },
		{ _, _, _, _, _ },
	};
#undef _

	// firstly handle the most common case
	if (isStandardPosition())
		return chess960::StandardIdn;

	if (!isShuffleChessPosition())
		return 0;

	uint64_t bishops	= this->bishops(White);
	uint64_t knights	= this->knights(White);
	uint64_t queen		= this->queens(White);

	// 1. compute the bishops code
	int bCode = BishopTable[::lsb(bishops)][::msb(bishops)];

	//M_ASSERT(0 <= bCode && bCode < 16);

	// 2. compute queen's position
	int qPos	= lsb(queen) - count(bishops & (queen - 1));

	//M_ASSERT(0 <= qPos && qPos <= 5);

	// 3. compute knights position
	int k1Sq		= lsb(knights);
	int k2Sq		= msb(knights);
	int k1Pos	= k1Sq - count((bishops | queen) & (setBit(k1Sq) - 1));
	int k2Pos	= k2Sq - count((bishops | queen) & (setBit(k2Sq) - 1));

	//M_ASSERT(0 <= k1Pos && k1Pos <= 3);
	//M_ASSERT(0 <= k2Pos && k2Pos <= 4);

	int n5nCode = N5NTable[k1Pos][k2Pos];

	//M_ASSERT(0 <= n5nCode && n5nCode <= 9);

	int idn = bCode + ::mul16(qPos) + 96*n5nCode;

	//M_ASSERT(0 <= idn && idn < 960);

	if (idn == 0)
		idn = 960;

	// 4. shift range depending on castling rights and rook positions
	uint64_t rooks = this->rooks(White);

	if (lsb(rooks) > m_ksq[White])
		idn += 2*960;
	else if (msb(rooks) < m_ksq[White])
		idn += 960;
	else if (m_castle == NoRights)
		idn += 3*960;

	return idn;
}


void
Board::setup(unsigned idn)
{
	//////M_ASSERT(idn > 0);
	//////M_ASSERT(idn <= 4*960);

	// firstly handle the most common case
	if (idn == chess960::StandardIdn)
		return setStandardPosition();

	bool frcCastling;

	if (idn <= 960)
	{
		frcCastling = true;
	}
	else
	{
		frcCastling = false;

		if (idn > 3*960)
			idn -= 3*960;
	}

	*this = m_shuffleChessBoard;	// setup pawns, signature, and other stuff

	char placement[8];
	::memcpy(placement, chess960::position(((idn - 1) % 960) + 1), 8);

	if (idn > 2*960)
	{
		char* r = ::strchr(placement, 'R');
		char* k = ::strchr(r + 1, 'K');

		mstl::swap(*r, *k);
	}
	else if (idn > 960)
	{
		char* k = ::strchr(placement, 'K');
		char* r = ::strchr(k + 1, 'R');

		mstl::swap(*r, *k);
	}

	for (unsigned i = 0; i < 8; ++i)
	{
		Square wSq = a1 + i;
		Square bSq = a8 + i;

		uint64_t whiteMask = setBit(wSq);
		uint64_t blackMask = setBit(bSq);

		m_occupiedBy[White] ^= whiteMask;
		m_occupiedBy[Black] ^= blackMask;
		m_occupiedL90 ^= MaskL90[wSq] | MaskL90[bSq];
		m_occupiedL45 ^= MaskL45[wSq] | MaskL45[bSq];
		m_occupiedR45 ^= MaskR45[wSq] | MaskR45[bSq];

		switch (placement[i])
		{
			case 'K':
				m_ksq[White] = wSq;
				m_ksq[Black] = bSq;
				m_kings |= whiteMask | blackMask;
				m_piece[wSq] = m_piece[bSq] = piece::King;
				hashPiece(wSq, piece::WhiteKing);
				hashPiece(bSq, piece::BlackKing);
				break;

			case 'Q':
				m_queens |= whiteMask | blackMask;
				m_piece[wSq] = m_piece[bSq] = piece::Queen;
				hashPiece(wSq, piece::WhiteQueen);
				hashPiece(bSq, piece::BlackQueen);
				break;

			case 'R':
				m_rooks |= whiteMask | blackMask;
				m_piece[wSq] = m_piece[bSq] = piece::Rook;
				hashPiece(wSq, piece::WhiteRook);
				hashPiece(bSq, piece::BlackRook);
				break;

			case 'B':
				m_bishops |= whiteMask | blackMask;
				m_piece[wSq] = m_piece[bSq] = piece::Bishop;
				hashPiece(wSq, piece::WhiteBishop);
				hashPiece(bSq, piece::BlackBishop);
				break;

			case 'N':
				m_knights |= whiteMask | blackMask;
				m_piece[wSq] = m_piece[bSq] = piece::Knight;
				hashPiece(wSq, piece::WhiteKnight);
				hashPiece(bSq, piece::BlackKnight);
				break;
		}
	}

	// set remainder of board data appropriately
	m_occupied = m_occupiedBy[White] | m_occupiedBy[Black];

	if (frcCastling)
	{
		setCastleShort(White);
		setCastleShort(Black);
		setCastleLong(White);
		setCastleLong(Black);
	}

	// simple validation
	//M_ASSERT(frcCastling ? isChess960Position() : isShuffleChessPosition());
	//M_ASSERT(computeIdn() == (idn <= 960 && !frcCastling) ? idn + 3*960 : idn);
}


bool
Board::checkIfLegalMove(Move& move) const
{
	if (move.isLegal())
		return true;

	Board board(*this);
	board.doMove(move);

	if (!board.isLegal())
		return false;

	move.setLegalMove();
	return true;
}


void
Board::filterLegalMoves(MoveList& result) const
{
	unsigned k = 0;

	for (unsigned i = 0; i < result.size(); ++i)
	{
		Move& move = result[i];

		if (move.isLegal())
		{
			result[k++] = move;
		}
		else
		{
			prepareUndo(move);
			const_cast<Board&>(*this).doMove(move);

			if (isLegal())
			{
				move.setLegalMove();
				result[k++] = move;
			}

			const_cast<Board&>(*this).undoMove(move);
		}
	}

	result.cut(k);
}


void
Board::filterCheckMovesTo(Move move, uint64_t& movers) const
{
	typedef mstl::bitfield<uint64_t> BitField;

	Board peek(*this);
	BitField squares(movers);

	prepareUndo(move);

	for (unsigned sq = squares.find_first(); sq != BitField::npos; sq = squares.find_next(sq))
	{
		move.setTo(sq);
		peek.doMove(move);

		if (!peek.givesCheck())
			movers &= ~setBit(sq);

		peek.undoMove(move);
	}
}


void
Board::filterCheckMateMovesTo(Move move, uint64_t& movers) const
{
	typedef mstl::bitfield<uint64_t> BitField;

	Board peek(*this);
	BitField squares(movers);

	prepareUndo(move);

	for (unsigned sq = squares.find_first(); sq != BitField::npos; sq = squares.find_next(sq))
	{
		move.setTo(sq);
		peek.doMove(move);

		if (!peek.givesMate())
			movers &= ~setBit(sq);

		peek.undoMove(move);
	}
}


void
Board::filterCheckMovesFrom(Move move, uint64_t& movers) const
{
	typedef mstl::bitfield<uint64_t> BitField;

	Board peek(*this);
	BitField squares(movers);

	prepareUndo(move);

	for (unsigned sq = squares.find_first(); sq != BitField::npos; sq = squares.find_next(sq))
	{
		move.setFrom(sq);
		peek.doMove(move);

		if (!peek.givesCheck())
			movers &= ~setBit(sq);

		peek.undoMove(move);
	}
}


void
Board::filterCheckMateMovesFrom(Move move, uint64_t& movers) const
{
	typedef mstl::bitfield<uint64_t> BitField;

	Board peek(*this);
	BitField squares(movers);

	prepareUndo(move);

	for (unsigned sq = squares.find_first(); sq != BitField::npos; sq = squares.find_next(sq))
	{
		move.setFrom(sq);
		peek.doMove(move);

		if (!peek.givesMate())
			movers &= ~setBit(sq);

		peek.undoMove(move);
	}
}


void
Board::genCastleShort(MoveList& result, color::ID side) const
{
	//M_ASSERT(m_castleRookCurrent[::kingSideIndex(side)] != Null);

	Move m(Move::genCastling(m_ksq[side], m_castleRookCurrent[::kingSideIndex(side)]));
	m.setLegalMove();
	result.append(m);
}


void
Board::genCastleLong(MoveList& result, color::ID side) const
{
	//M_ASSERT(m_castleRookCurrent[::queenSideIndex(side)] != Null);

	Move m(Move::genCastling(m_ksq[side], m_castleRookCurrent[::queenSideIndex(side)]));
	m.setLegalMove();
	result.append(m);
}


void
Board::generateCastlingMoves(MoveList& result) const
{
	if (m_stm == White)
	{
		if ((m_castle & WhiteKingside) && shortCastlingWhiteIsLegal())
			genCastleShort(result, White);
		if ((m_castle & WhiteQueenside) && longCastlingWhiteIsLegal())
			genCastleLong(result, White);
	}
	else
	{
		if ((m_castle & BlackKingside) && shortCastlingBlackIsLegal())
			genCastleShort(result, Black);
		if ((m_castle & BlackQueenside) && longCastlingBlackIsLegal())
			genCastleLong(result, Black);
	}
}


void
Board::generateMoves(MoveList& result) const
{
	result.clear();

	if (m_stm == White)
	{
		// castle moves
		if ((m_castle & WhiteKingside) && shortCastlingWhiteIsLegal())
			genCastleShort(result, White);
		if ((m_castle & WhiteQueenside) && longCastlingWhiteIsLegal())
			genCastleLong(result, White);

		// pawn en passant moves
		uint64_t movers = m_pawns & m_occupiedBy[White];

		if (m_epSquare != Null)
		{
			uint64_t moves = PawnAttacks[Black][m_epSquare] & movers;

			while (moves)
				result.append(Move::genEnPassant(lsbClear(moves), m_epSquare));
		}

		// pawn captures
		uint64_t moves = ::shiftUpRight(movers) & m_occupiedBy[Black];

		while (moves)
		{
			uint32_t to = lsbClear(moves);
			uint32_t captured = m_piece[to];

			if (::rank(to) == Rank8)
			{
				result.append(Move::genCapturePromote(to - 9, to, piece::Queen, captured));
				result.append(Move::genCapturePromote(to - 9, to, piece::Knight, captured));
				result.append(Move::genCapturePromote(to - 9, to, piece::Rook, captured));
				result.append(Move::genCapturePromote(to - 9, to, piece::Bishop, captured));
			}
			else
			{
				result.append(Move::genPawnCapture(to - 9, to, captured));
			}
		}

		moves = ::shiftUpLeft(movers) & m_occupiedBy[Black];

		while (moves)
		{
			uint32_t to = lsbClear(moves);
			uint32_t captured = m_piece[to];

			if (::rank(to) == Rank8)
			{
				result.append(Move::genCapturePromote(to - 7, to, piece::Queen, captured));
				result.append(Move::genCapturePromote(to - 7, to, piece::Knight, captured));
				result.append(Move::genCapturePromote(to - 7, to, piece::Rook, captured));
				result.append(Move::genCapturePromote(to - 7, to, piece::Bishop, captured));
			}
			else
			{
				result.append(Move::genPawnCapture(to - 7, to, captured));
			}
		}

		// pawns 1 forward
		moves = ::shiftUp(movers) & ~m_occupied;
		movers = moves;

		while (moves)
		{
			unsigned to = lsbClear(moves);

			if (::rank(to) == Rank8)
			{
				result.append(Move::genPromote(to - 8, to, piece::Queen));
				result.append(Move::genPromote(to - 8, to, piece::Knight));
				result.append(Move::genPromote(to - 8, to, piece::Rook));
				result.append(Move::genPromote(to - 8, to, piece::Bishop));
			}
			else
			{
				result.append(Move::genOneForward(to - 8, to));
			}
		}

		// pawns 2 forward
		moves = ::shiftUp(movers) & RankMask4 & ~m_occupied;

		while (moves)
		{
			unsigned to = lsbClear(moves);
			result.append(Move::genTwoForward(to - 16, to));
		}
	}
	else
	{
		// castle moves
		if ((m_castle & BlackKingside) && shortCastlingBlackIsLegal())
			genCastleShort(result, Black);
		if ((m_castle & BlackQueenside) && longCastlingBlackIsLegal())
			genCastleLong(result, Black);

		// pawn en passant moves
		uint64_t movers = m_pawns & m_occupiedBy[Black];

		if (m_epSquare != Null)
		{
			uint64_t moves = PawnAttacks[White][m_epSquare] & movers;

			while (moves)
				result.append(Move::genEnPassant(lsbClear(moves), m_epSquare));
		}

		// pawn captures
		uint64_t moves = ::shiftDownLeft(movers) & m_occupiedBy[White];

		while (moves)
		{
			unsigned to = lsbClear(moves);

			if (::rank(to) == Rank1)
			{
				result.append(Move::genCapturePromote(to + 9, to, piece::Queen,  m_piece[to]));
				result.append(Move::genCapturePromote(to + 9, to, piece::Knight, m_piece[to]));
				result.append(Move::genCapturePromote(to + 9, to, piece::Rook,   m_piece[to]));
				result.append(Move::genCapturePromote(to + 9, to, piece::Bishop, m_piece[to]));
			}
			else
			{
				result.append(Move::genPawnCapture(to + 9, to, m_piece[to]));
			}
		}

		moves = ::shiftDownRight(movers) & m_occupiedBy[White];

		while (moves)
		{
			unsigned to = lsbClear(moves);

			if (::rank(to) == Rank1)
			{
				result.append(Move::genCapturePromote(to + 7, to, piece::Queen,  m_piece[to]));
				result.append(Move::genCapturePromote(to + 7, to, piece::Knight, m_piece[to]));
				result.append(Move::genCapturePromote(to + 7, to, piece::Rook,   m_piece[to]));
				result.append(Move::genCapturePromote(to + 7, to, piece::Bishop, m_piece[to]));
			}
			else
			{
				result.append(Move::genPawnCapture(to + 7, to, m_piece[to]));
			}
		}

		// pawns 1 forward
		moves = ::shiftDown(movers) & ~m_occupied;
		movers = moves;

		while (moves)
		{
			unsigned to = lsbClear(moves);

			if (::rank(to) != 0)
			{
				result.append(Move::genOneForward(to + 8, to));
			}
			else
			{
				result.append(Move::genPromote(to + 8, to, piece::Queen));
				result.append(Move::genPromote(to + 8, to, piece::Knight));
				result.append(Move::genPromote(to + 8, to, piece::Rook));
				result.append(Move::genPromote(to + 8, to, piece::Bishop));
			}
		}

		// pawns 2 forward
		moves = ::shiftDown(movers) & RankMask5 & ~m_occupied;

		while (moves)
		{
			unsigned to = lsbClear(moves);
			result.append(Move::genTwoForward(to + 16, to));
		}
	}

	uint64_t occupied = m_occupiedBy[m_stm];
	uint64_t movers;

	// knight moves
	movers = m_knights & occupied;

	while (movers)
	{
		unsigned from = lsbClear(movers);
		uint64_t moves = knightAttacks(from) & ~occupied;

		while (moves)
		{
			unsigned to = lsbClear(moves);
			result.append(Move::genKnightMove(from, to, m_piece[to]));
		}
	}

	// bishop moves
	movers = m_bishops & occupied;

	while (movers)
	{
		unsigned from = lsbClear(movers);
		uint64_t moves = bishopAttacks(from) & ~occupied;

		while (moves)
		{
			unsigned to = lsbClear(moves);
			result.append(Move::genBishopMove(from, to, m_piece[to]));
		}
	}

	// rook moves
	movers = m_rooks & occupied;

	while (movers)
	{
		unsigned from = lsbClear(movers);
		uint64_t moves = rookAttacks(from) & ~occupied;

		while (moves)
		{
			unsigned to = lsbClear(moves);
			result.append(Move::genRookMove(from, to, m_piece[to]));
		}
	}

	// queen moves
	movers = m_queens & occupied;

	while (movers)
	{
		unsigned from = lsbClear(movers);
		uint64_t moves = queenAttacks(from) & ~occupied;

		while (moves)
		{
			unsigned to = lsbClear(moves);
			result.append(Move::genQueenMove(from, to, m_piece[to]));
		}
	}

	// king moves
	uint64_t moves = kingAttacks(m_ksq[m_stm]) & ~occupied;

	while (moves)
	{
		uint8_t to = lsbClear(moves);

		if (!isAttackedBy(m_stm ^ 1, to))
			result.append(Move::genKingMove(m_ksq[m_stm], to, m_piece[to]));
	}
}


void
Board::generatePawnCapturingMoves(MoveList& result) const
{
	if (m_stm == White)
	{
		// en passant moves
		uint64_t movers = m_pawns & m_occupiedBy[White];

		if (m_epSquare != Null)
		{
			uint64_t moves = PawnAttacks[Black][m_epSquare] & movers;

			while (moves)
				result.append(Move::genEnPassant(lsbClear(moves), m_epSquare));
		}

		// captures
		uint64_t moves = ::shiftUpRight(movers) & m_occupiedBy[Black];

		while (moves)
		{
			uint32_t to = lsbClear(moves);
			uint32_t captured = m_piece[to];

			if (::rank(to) == Rank8)
			{
				result.append(Move::genCapturePromote(to - 9, to, piece::Queen,  captured));
				result.append(Move::genCapturePromote(to - 9, to, piece::Knight, captured));
				result.append(Move::genCapturePromote(to - 9, to, piece::Rook,   captured));
				result.append(Move::genCapturePromote(to - 9, to, piece::Bishop, captured));
			}
			else
			{
				result.append(Move::genPawnCapture(to - 9, to, captured));
			}
		}

		moves = ::shiftUpLeft(movers) & m_occupiedBy[Black];

		while (moves)
		{
			uint32_t to = lsbClear(moves);
			uint32_t captured = m_piece[to];

			if (::rank(to) == Rank8)
			{
				result.append(Move::genPawnCapture(to - 7, to, captured));
			}
			else
			{
				result.append(Move::genCapturePromote(to - 7, to, piece::Queen,  captured));
				result.append(Move::genCapturePromote(to - 7, to, piece::Knight, captured));
				result.append(Move::genCapturePromote(to - 7, to, piece::Rook,   captured));
				result.append(Move::genCapturePromote(to - 7, to, piece::Bishop, captured));
			}
		}
	}
	else
	{
		// en passant moves
		uint64_t movers = m_pawns & m_occupiedBy[Black];

		if (m_epSquare != Null)
		{
			uint64_t moves = PawnAttacks[White][m_epSquare] & movers;

			while (moves)
				result.append(Move::genEnPassant(lsbClear(moves), m_epSquare));
		}

		// captures
		uint64_t moves = ::shiftDownLeft(movers) & m_occupiedBy[White];

		while (moves)
		{
			uint32_t to = lsbClear(moves);
			uint32_t captured = m_piece[to];

			if (::rank(to) == Rank1)
			{
				result.append(Move::genCapturePromote(to + 9, to, piece::Queen,  captured));
				result.append(Move::genCapturePromote(to + 9, to, piece::Knight, captured));
				result.append(Move::genCapturePromote(to + 9, to, piece::Rook,   captured));
				result.append(Move::genCapturePromote(to + 9, to, piece::Bishop, captured));
			}
			else
			{
				result.append(Move::genPawnCapture(to + 9, to, captured));
			}
		}

		moves = ::shiftDownRight(movers) & m_occupiedBy[White];

		while (moves)
		{
			uint32_t to = lsbClear(moves);
			uint32_t captured = m_piece[to];

			if (::rank(to) == Rank1)
			{
				result.append(Move::genCapturePromote(to + 7, to, piece::Queen,  captured));
				result.append(Move::genCapturePromote(to + 7, to, piece::Knight, captured));
				result.append(Move::genCapturePromote(to + 7, to, piece::Rook,   captured));
				result.append(Move::genCapturePromote(to + 7, to, piece::Bishop, captured));
			}
			else
			{
				result.append(Move::genPawnCapture(to + 7, to, captured));
			}
		}
	}
}


void
Board::generateCapturingMoves(MoveList& result) const
{
	result.clear();

	generatePawnCapturingMoves(result);

	uint64_t occupied	= m_occupiedBy[m_stm];
	uint64_t capture	= m_occupiedBy[m_stm ^ 1];
	uint64_t movers;

	// knight moves
	movers = m_knights & occupied;

	while (movers)
	{
		unsigned from = lsbClear(movers);
		uint64_t moves = knightAttacks(from) & capture;

		while (moves)
		{
			unsigned to = lsbClear(moves);
			result.append(Move::genKnightMove(from, to, m_piece[to]));
		}
	}

	// bishop moves
	movers = m_bishops & occupied;

	while (movers)
	{
		unsigned from = lsbClear(movers);
		uint64_t moves = bishopAttacks(from) & capture;

		while (moves)
		{
			unsigned to = lsbClear(moves);
			result.append(Move::genBishopMove(from, to, m_piece[to]));
		}
	}

	// rook moves
	movers = m_rooks & occupied;

	while (movers)
	{
		unsigned from = lsbClear(movers);
		uint64_t moves = rookAttacks(from) & capture;

		while (moves)
		{
			unsigned to = lsbClear(moves);
			result.append(Move::genRookMove(from, to, m_piece[to]));
		}
	}

	// queen moves
	movers = m_queens & occupied;

	while (movers)
	{
		unsigned from = lsbClear(movers);
		uint64_t moves = queenAttacks(from) & capture;

		while (moves)
		{
			unsigned to = lsbClear(moves);
			result.append(Move::genQueenMove(from, to, m_piece[to]));
		}
	}

	// king moves
	uint64_t moves = kingAttacks(m_ksq[m_stm]) & capture;

	while (moves)
	{
		uint8_t to = lsbClear(moves);

		if (!isAttackedBy(m_stm ^ 1, to))
		{
			//M_ASSERT(m_piece[to] != piece::None);
			result.append(Move::genKingMove(m_ksq[m_stm], to, m_piece[to]));
		}
	}
}


Move*
Board::findMatchingMove(MoveList& list, unsigned state) const
{
	for (Move* m = list.begin(); m != list.end(); ++m)
	{
		if ((checkState(*m) & state) != 0)
			return m;
	}

	return 0;
}


char const*
Board::parseMove(char const* algebraic, Move& move, move::Constraint flag) const
{
	//////M_ASSERT(algebraic);

	char const*	s = algebraic;
	unsigned		type;

	switch (*s)
	{
		case '-':	// "--"		null move used in ChessBase
			if (*++s != '-')
				return 0;
			if (s[1] == '-' && s[2] == '-')	// "----" null move used in LAN
				s += 2;
			move = makeNullMove();
			return ++s;

		case '@':
			if (s[1] != '@' || s[2] != '@' || s[3] != '@')
				return 0;
			return s + 4;	// "@@@@" null move used in WinBoard protocol

		case 'n':
			if (s[1] != 'u' || s[2] != 'l' || s[3] != 'l')
				return 0;
			return s + 4;	// "null" null move used in WinBoard protocol

		case 'p':
			if (s[1] != 'a' || s[2] != 's' || s[3] != 's')
				return 0;
			return s + 4;	// "pass" null move used in WinBoard protocol

		case '0':	// "0000"	null move used in UCI protocol
			if (s[1] != '0' || s[2] != '0' || s[3] != '0')
				return 0;
			move = makeNullMove();
			return s + 4;

		case 'O':	// Castling
			if (s[1] == '-' && s[2] == 'O')
			{
				unsigned index;

				if (s[3] == '-' && s[4] == 'O')
				{
					index = ::queenSideIndex(m_stm);
					s += 5;
				}
				else
				{
					index = ::kingSideIndex(m_stm);
					s += 3;
				}

				unsigned rook = m_castleRookCurrent[index];

				if (rook != Null)
				{
					move = prepareMove(m_ksq[m_stm], rook, flag);
					return s;
				}
			}

			return 0;

		// Piece

		case 'Q':	type = piece::Queen;  ++s; break;
		case 'R':	type = piece::Rook;   ++s; break;
		case 'B':	type = piece::Bishop; ++s; break;
		case 'N':	type = piece::Knight; ++s; break;
		case 'K':	type = piece::King;   ++s; break;
		case 'P':	type = piece::Pawn;   ++s; break;
		default:		type = piece::Pawn;   break;
	}

	int fromSquare	= -1;
	int toSquare	= -1;
	int fromFyle	= -1;
	int fromRank	= -1;

	// Check for disambiguation
	if (::isFyle(*s))
	{
		fromFyle = ::toFyle(*s++);

		if (::isRank(*s))
		{
			fromSquare = ::mul8(::toRank(*s++)) + fromFyle;
			fromFyle = -1;
		}
	}
	else if (::isRank(*s))
	{
		fromRank = ::toRank(*s++);
	}

	// Capture indicator; dash in the case of a LAN move; colon is the german capture indicator
	if ((*s == 'x' || *s == '-' || *s == ':') && ::isFyle(s[1]))
		++s;

	// Destination square
	if (::isFyle(*s))
	{
		int toFyle = ::toFyle(*s++);

		if (::isRank(*s))
		{
			toSquare = ::mul8(::toRank(*s++)) + toFyle;
		}
		else
		{
			if (type != piece::Pawn)
				return 0;

			if (fromSquare < 0)	// handle pawn captures like 'ed'
			{
				//M_ASSERT(fromFyle != -1);

				char const* t = ::skipPromotion(s);

				MoveList moveList;
				MoveList validList;
				generatePawnCapturingMoves(moveList);
				Move* m = 0;

				for (Move* m = moveList.begin(); m != moveList.end(); ++m)
				{
					if (	::fyle(m->from()) == fromFyle
						&& ::fyle(m->to()) == toFyle
						&& (flag != move::DontAllowIllegalMove || checkIfLegalMove(*m)))
					{
						validList.push(*m);
					}
				}

				if (*t == '#')
					m = findMatchingMove(validList, CheckMate);

				if (m == 0 && (t[0] == '+' && t[1] == '+'))
					m = findMatchingMove(validList, DoubleCheck);

				if (m == 0 && (*t == '#' || *t == '+'))
					m = findMatchingMove(validList, Check | DoubleCheck | CheckMate);

				if (m == 0)
				{
					if (validList.isEmpty())
						return 0;

					m = validList.begin();
				}

				fromSquare = m->from();
				toSquare = m->to();
			}
			else						// handle pawn captures like 'e4d'
			{
				int toSq1 = -1, toSq2 = -1;

				if (::fyle(fromSquare) > FyleA)
				{
					toSq1 = fromSquare + (m_stm == White ? +7 : -9);

					if (toSq1 >= 0)
					{
						Move move = prepareMove(fromSquare, toSq1, flag);

						if (!move)
							toSq1 = -1;
					}
				}

				if (::fyle(fromSquare) < FyleH)
				{
					toSq2 = fromSquare + (m_stm == White ? +9 : -7);

					if (toSq2 >= 0)
					{
						Move move = prepareMove(fromSquare, toSq2, flag);

						if (!move)
							toSq2 = -1;
					}
				}

				if ((toSquare = toSq1 == -1 ? toSq2 : toSq1) == -1)
					return 0;
			}
		}
	}
	else
	{
		toSquare = fromSquare;
		fromSquare = -1;
	}

	if (toSquare < 0)
		return 0;

	if (type == piece::Pawn)
	{
		if (fromSquare < 0)
		{
			int base = (m_stm == White ? -8 : 8);

			if (fromFyle < 0)
			{
				fromSquare = toSquare + base;

				if (!(m_occupiedBy[m_stm] & setBit(fromSquare)))
					fromSquare += base;
			}
			else if (fromFyle <= int(::fyle(toSquare)))
			{
				fromSquare = toSquare + base - 1;
			}
			else
			{
				fromSquare = toSquare + base + 1;
			}
		}

		move = prepareMove(fromSquare, toSquare, flag);

		if (move.isPromotion())
		{
			// Promotion as in bc8Q, bxc8=Q, bxc8=(Q), bxc8(Q), bxc8/Q, or bxc8/(Q)
			if (*s == '=' || *s == '/')
				++s;

			if (*s == '(')
			{
				if (s[1] && s[2] == ')')
				{
					++s;
				}
				else
				{
					move.setPromoted(piece::Queen);
					if (s[-1] == '=')
						--s;
					return s;
				}
			}

			switch (*s++)
			{
				case 'Q': case 'q': move.setPromoted(piece::Queen);  break;
				case 'R': case 'r': move.setPromoted(piece::Rook);   break;
				case 'B': case 'b': move.setPromoted(piece::Bishop); break;
				case 'N': case 'n': move.setPromoted(piece::Knight); break;

				case 'K': case 'k':
#if 0
					if (s[-2] == '=' && !::isalnum(s[0]))
					{
						move.setPromoted(piece::Knight); break;	// catching a mistake
					}
					else
#endif
					{
						if (s[-2] == '(')
							--s;
						return s[-2] == '=' ? s - 2 : s - 1;
					}
					break;

				default:
					move.setPromoted(piece::Queen);
					if (s[-2] == '(')
						--s;
					return s[-2] == '=' ? s - 2 : s - 1;
			}
		}

		if (s[-2] == '(')
			++s;

		return s;
	}

	if (fromSquare < 0)
	{
		uint64_t match;

		switch (type)
		{
			case piece::Queen:	match = queenAttacks(toSquare) & m_queens; break;
			case piece::Rook:		match = rookAttacks(toSquare) & m_rooks; break;
			case piece::Bishop:	match = bishopAttacks(toSquare) & m_bishops; break;
			case piece::Knight:	match = knightAttacks(toSquare) & m_knights; break;
			case piece::King:		match = kingAttacks(toSquare) & m_kings; break;
			default:					return 0;
		}

		match &= m_occupiedBy[m_stm];

		if (fromRank >= 0)
			match &= RankMask[fromRank];
		else if (fromFyle >= 0)
			match &= FyleMask[fromFyle];

		// If not yet fully disambiguated, all but one move must be illegal.
		// Cycle through them, and pick the first legal move.

		// Only mating moves will be regarded.
		if (*s == '#')
		{
			uint64_t m = match;

			while (m)
			{
				fromSquare = lsbClear(m);

				//M_ASSERT(type == m_piece[fromSquare]);

				move = prepareMove(fromSquare, toSquare, flag);

				if (move.isLegal() && (checkState(move) & CheckMate))
					return s;
			}
		}

		// Only double checking moves will be regarded.
		if (s[0] == '+' && s[1] == '+')
		{
			uint64_t m = match;

			while (m)
			{
				fromSquare = lsbClear(m);

				//M_ASSERT(type == m_piece[fromSquare]);

				move = prepareMove(fromSquare, toSquare, flag);

				if (move.isLegal() && (checkState(move) & DoubleCheck))
					return s;
			}
		}

		// Only checking moves will be regarded.
		if (*s == '#' || *s == '+')
		{
			uint64_t m = match;

			while (m)
			{
				fromSquare = lsbClear(m);

				//M_ASSERT(type == m_piece[fromSquare]);

				move = prepareMove(fromSquare, toSquare, flag);

				if (move.isLegal() && (checkState(move) & (Check | DoubleCheck | CheckMate)))
					return s;
			}
		}

		// All moves will be regarded.
		while (match)
		{
			fromSquare = lsbClear(match);

			//M_ASSERT(type == m_piece[fromSquare]);

			move = prepareMove(fromSquare, toSquare, flag);

			if (move.isLegal())
				return s;
		}

		// probably a castling move is desired
		if (type == piece::King)
		{
			if (canCastleShort(color::ID(m_stm)) && toSquare == m_castleRookCurrent[::kingSideIndex(m_stm)])
			{
				move = prepareMove(m_ksq[m_stm], toSquare, flag);
				return s;
			}

			if (canCastleLong(color::ID(m_stm)) && toSquare == m_castleRookCurrent[::queenSideIndex(m_stm)])
			{
				move = prepareMove(m_ksq[m_stm], toSquare, flag);
				return s;
			}

			switch (toSquare)
			{
				case c1:
					if (whiteToMove() && canCastleLong(White))
					{
						move = prepareMove(m_ksq[White], m_castleRookCurrent[::queenSideIndex(White)], flag);
						return s;
					}
					break;

				case g1:
					if (whiteToMove() && canCastleShort(White))
					{
						move = prepareMove(m_ksq[White], m_castleRookCurrent[::kingSideIndex(White)], flag);
						return s;
					}
					break;

				case c8:
					if (blackToMove() && canCastleLong(Black))
					{
						move = prepareMove(m_ksq[Black], m_castleRookCurrent[::queenSideIndex(Black)], flag);
						return s;
					}
					break;

				case g8:
					if (blackToMove() && canCastleShort(Black))
					{
						move = prepareMove(m_ksq[Black], m_castleRookCurrent[::kingSideIndex(Black)], flag);
						return s;
					}
					break;
			}
		}

		return s;
	}

	if (type == m_piece[fromSquare])
		move = prepareMove(fromSquare, toSquare, flag);

	return s;
}


void
Board::doMove(Move const& m)
{
	//////M_ASSERT(!m.isEmpty());

	m_epSquareFen = Null;

	if (m_epSquare != Null)
	{
		hashEnPassant();
		m_epSquare = Null;
	}

	if (__builtin_expect(!m.isNull(), 1))
	{
		unsigned from		= m.from();
		unsigned to			= m.to();
		unsigned sntm		= m_stm ^ 1; // side not to move
		uint64_t fromMask	= setBit(from);
		uint64_t toMask	= setBit(to);
		uint64_t bothMask	= fromMask ^ toMask;

		switch (m.action())
		{
			case Move::One_Forward:
				m_halfMoveClock = 0;
				m_pawns ^= bothMask;
				m_piece[to] = piece::Pawn;
				pawnProgressMove(m_stm, from, to);
				hashPawn(from, to, ::toPiece(piece::Pawn, m_stm));
				break;

			case Move::Two_Forward:
				m_halfMoveClock = 0;
				m_pawns ^= bothMask;
				m_piece[to] = piece::Pawn;
				pawnProgressMove(m_stm, from, to);
				hashPawn(from, to, ::toPiece(piece::Pawn, m_stm));
				setEnPassantFyle(color::ID(sntm), sq::fyle(to));
				break;

			case piece::Knight:
				++m_halfMoveClock;
				m_knights ^= bothMask;
				m_piece[to] = piece::Knight;
				hashPiece(from, to, ::toPiece(piece::Knight, m_stm));
				break;

			case piece::Bishop:
				++m_halfMoveClock;
				m_bishops ^= bothMask;
				m_piece[to] = piece::Bishop;
				hashPiece(from, to, ::toPiece(piece::Bishop, m_stm));
				break;

			case piece::Rook:
				++m_halfMoveClock;
				m_rooks ^= bothMask;
				m_piece[to] = piece::Rook;
				hashPiece(from, to, ::toPiece(piece::Rook, m_stm));
				{
					Byte castling = m_destroyCastle[from];
					if (m_castle & ~castling)
					{
						Index index = Index(lsb(uint8_t(~castling)));
						hashCastling(index);
						m_castle &= castling;
						m_castleRookCurrent[index] = Null;
					}
				}
				break;

			case piece::Queen:
				++m_halfMoveClock;
				m_queens ^= bothMask;
				m_piece[to] = piece::Queen;
				hashPiece(from, to, ::toPiece(piece::Queen, m_stm));
				break;

			case piece::King:
				++m_halfMoveClock;
				m_kings ^= bothMask;
				m_ksq[m_stm] = to;
				m_piece[to] = piece::King;
				hashPiece(from, to, ::toPiece(piece::King, m_stm));
				if (canCastle(color::ID(m_stm)))
				{
					hashCastling(color::ID(m_stm));
					destroyCastle(color::ID(m_stm));
					m_castleRookCurrent[::kingSideIndex(m_stm)] = Null;
					m_castleRookCurrent[::queenSideIndex(m_stm)] = Null;
				}
				break;

			case Move::Castle:
				{
					//M_ASSERT(from == m_ksq[m_stm]);

					++m_halfMoveClock;

					unsigned rank		= ::rank(to);
					unsigned rookFrom	= to;
					unsigned rookTo;

					hashCastling(color::ID(m_stm));
					destroyCastle(color::ID(m_stm));
					m_castleRookCurrent[::kingSideIndex(m_stm)] = Null;
					m_castleRookCurrent[::queenSideIndex(m_stm)] = Null;

					if (from < to)
					{
						addCastling(kingSide(color::ID(m_stm)));
						rookTo = sq::make(FyleF, rank);
						to = sq::make(FyleG, rank);
					}
					else
					{
						addCastling(queenSide(color::ID(m_stm)));
						rookTo = sq::make(FyleD, rank);
						to = sq::make(FyleC, rank);
					}

					// in Chess 960 the king may stand still
					if (to != m_ksq[m_stm])
					{
						bothMask = fromMask ^ setBit(to);
						m_ksq[m_stm] = to;
						m_kings ^= bothMask;
						m_piece[from] = piece::Empty;
						hashPiece(from, to, ::toPiece(piece::King, m_stm));

						m_occupiedBy[m_stm] ^= bothMask;
						m_occupiedL90 ^= MaskL90[from] ^ MaskL90[to];
						m_occupiedL45 ^= MaskL45[from] ^ MaskL45[to];
						m_occupiedR45 ^= MaskR45[from] ^ MaskR45[to];
					}

					// in handicap games the rook field is possibly empty
					if (m_piece[rookFrom] != piece::Empty)
					{
						//M_ASSERT(m_piece[rookFrom] == piece::Rook);

						uint64_t rookMask = setBit(rookFrom) ^ setBit(rookTo);

						m_piece[rookFrom] = piece::Empty;
						m_piece[rookTo] = piece::Rook;
						m_rooks ^= rookMask;
						m_occupiedBy[m_stm] ^= rookMask;
						m_occupiedL90 ^= MaskL90[rookFrom] ^ MaskL90[rookTo];
						m_occupiedL45 ^= MaskL45[rookFrom] ^ MaskL45[rookTo];
						m_occupiedR45 ^= MaskR45[rookFrom] ^ MaskR45[rookTo];
						hashPiece(rookFrom, rookTo, ::toPiece(piece::Rook, m_stm));
					}

					m_piece[to] = piece::King;
					m_occupied = m_occupiedBy[White] | m_occupiedBy[Black];

					hashToMove();
					swapToMove();
					++m_plyNumber;
				}
				return;

			case Move::Promote:
				static_assert(sizeof(m_progress.side[0].rank) < 7, "reimplement pawn progress");
				m_halfMoveClock = 0;
				m_pawns ^= fromMask;
				++m_promotions;
				m_material.part[m_stm].pawn = (1 << --m_matCount[m_stm].pawn) - 1;
				hashPawn(from, ::toPiece(piece::Pawn, m_stm));

				switch (Byte(m.promoted()))
				{
					case piece::Knight:
						m_knights ^= toMask;
						m_piece[to] = piece::Knight;
						++m_underPromotions;
						m_material.part[m_stm].knight = (1 << ++m_matCount[m_stm].knight) - 1;
						hashPiece(to, ::toPiece(piece::Knight, m_stm));
						break;

					case piece::Bishop:
						m_bishops ^= toMask;
						m_piece[to] = piece::Bishop;
						++m_underPromotions;
						m_material.part[m_stm].bishop = (1 << ++m_matCount[m_stm].bishop) - 1;
						hashPiece(to, ::toPiece(piece::Bishop, m_stm));
						break;

					case piece::Rook:
						m_rooks ^= toMask;
						m_piece[to] = piece::Rook;
						++m_underPromotions;
						m_material.part[m_stm].rook = (1 << ++m_matCount[m_stm].rook) - 1;
						hashPiece(to, ::toPiece(piece::Rook, m_stm));
						break;

					case piece::Queen:
						m_queens ^= toMask;
						m_piece[to] = piece::Queen;
						m_material.part[m_stm].queen = (1 << ++m_matCount[m_stm].queen) - 1;
						hashPiece(to, ::toPiece(piece::Queen, m_stm));
						break;
				}
				break;
		}

		switch (m.removal())
		{
			case piece::None:
				// extra cleanup needed for non-captures
				m_occupiedL90 ^= MaskL90[to];
				m_occupiedL45 ^= MaskL45[to];
				m_occupiedR45 ^= MaskR45[to];
				break;

			case piece::Pawn:
				m_halfMoveClock = 0;
				m_pawns ^= toMask;
				m_occupiedBy[sntm] ^= toMask;
				pawnProgressRemove(sntm, to);
				m_material.part[sntm].pawn = (1 << --m_matCount[sntm].pawn) - 1;
				hashPawn(to, ::toPiece(piece::Pawn, sntm));
				break;

			case piece::Knight:
				m_halfMoveClock = 0;
				m_knights ^= toMask;
				m_occupiedBy[sntm] ^= toMask;
				m_material.part[sntm].knight = (1 << --m_matCount[sntm].knight) - 1;
				hashPiece(to, ::toPiece(piece::Knight, sntm));
				break;

			case piece::Bishop:
				m_halfMoveClock = 0;
				m_bishops ^= toMask;
				m_occupiedBy[sntm] ^= toMask;
				m_material.part[sntm].bishop = (1 << --m_matCount[sntm].bishop) - 1;
				hashPiece(to, ::toPiece(piece::Bishop, sntm));
				break;

			case piece::Rook:
				m_halfMoveClock = 0;
				m_rooks ^= toMask;
				m_occupiedBy[sntm] ^= toMask;
				{
					Byte castling = m_destroyCastle[to];
					if (m_castle & ~castling)
					{
						Index index = Index(lsb(uint8_t(~castling)));
						hashCastling(index);
						m_castle &= castling;
						m_castleRookCurrent[index] = Null;
					}
				}
				m_material.part[sntm].rook = (1 << --m_matCount[sntm].rook) - 1;
				hashPiece(to, ::toPiece(piece::Rook, sntm));
				break;

			case piece::Queen:
				m_halfMoveClock = 0;
				m_queens ^= toMask;
				m_occupiedBy[sntm] ^= toMask;
				m_material.part[sntm].queen = (1 << --m_matCount[sntm].queen) - 1;
				hashPiece(to, ::toPiece(piece::Queen, sntm));
				break;

			case Move::En_Passant:
				m_halfMoveClock = 0;
				// annoying move, the capture is not on the 'to' square
				unsigned epsq = PrevRank[m_stm][to];
				m_piece[epsq] = piece::Empty;
				m_pawns ^= setBit(epsq);
				m_occupiedBy[sntm] ^= setBit(epsq);
				m_occupiedL90 ^= MaskL90[to] ^ MaskL90[epsq];
				m_occupiedL45 ^= MaskL45[to] ^ MaskL45[epsq];
				m_occupiedR45 ^= MaskR45[to] ^ MaskR45[epsq];
				m_material.part[sntm].pawn = (1 << --m_matCount[sntm].pawn) - 1;
				pawnProgressRemove(sntm, epsq);
				hashPawn(epsq, ::toPiece(piece::Pawn, sntm));
				break;
		}
		// ...no we did not forget the king!

		m_piece[from] = piece::Empty;
		m_occupiedBy[m_stm] ^= bothMask;
		m_occupiedL90 ^= MaskL90[from];
		m_occupiedL45 ^= MaskL45[from];
		m_occupiedR45 ^= MaskR45[from];
		m_occupied = m_occupiedBy[White] | m_occupiedBy[Black];
	}

	hashToMove();
	swapToMove();
	++m_plyNumber;
}


void
Board::undoMove(Move const& m)
{
	//////M_ASSERT(!m.isEmpty());
	//////M_ASSERT(m.preparedForUndo());

	if (__builtin_expect(!m.isNull(), 1))
	{
		unsigned from		= m.from();
		unsigned to			= m.to();
		unsigned sntm		= m_stm ^ 1;		// side not to move
		uint64_t fromMask	= setBit(from);
		uint64_t toMask	= setBit(to);
		uint64_t bothMask	= fromMask ^ toMask;

		switch (m.action())
		{
			case Move::One_Forward:
				m_pawns ^= bothMask;
				m_piece[from] = piece::Pawn;
				pawnProgressMove(sntm, to, from);
				hashPawn(from, to, ::toPiece(piece::Pawn, sntm));
				break;

			case Move::Two_Forward:
				m_pawns ^= bothMask;
				m_piece[from] = piece::Pawn;
				pawnProgressMove(sntm, to, from);
				hashPawn(from, to, ::toPiece(piece::Pawn, sntm));
				if (m_epSquare != Null)
					hashEnPassant();
				break;

			case piece::Knight:
				m_knights ^= bothMask;
				m_piece[from] = piece::Knight;
				hashPiece(from, to, ::toPiece(piece::Knight, sntm));
				break;

			case piece::Bishop:
				m_bishops ^= bothMask;
				m_piece[from] = piece::Bishop;
				hashPiece(from, to, ::toPiece(piece::Bishop, sntm));
				break;

			case piece::Rook:
				m_rooks ^= bothMask;
				m_piece[from] = piece::Rook;
				hashPiece(from, to, ::toPiece(piece::Rook, sntm));
				break;

			case piece::Queen:
				m_queens ^= bothMask;
				m_piece[from] = piece::Queen;
				hashPiece(from, to, ::toPiece(piece::Queen, sntm));
				break;

			case piece::King:
				m_kings ^= bothMask;
				m_ksq[sntm] = from;
				m_piece[from] = piece::King;
				hashPiece(from, to, ::toPiece(piece::King, sntm));
				break;

			case Move::Castle:
				{
					unsigned rank		= ::rank(to);
					unsigned rookFrom	= to;
					unsigned rookTo;

					if (from < to)
					{
						removeCastling(kingSide(color::ID(sntm)));
						rookTo = sq::make(FyleF, rank);
						to = sq::make(FyleG, rank);
					}
					else
					{
						removeCastling(queenSide(color::ID(sntm)));
						rookTo = sq::make(FyleD, rank);
						to = sq::make(FyleC, rank);
					}

					// we have to take into account that the castling was potentially illegal
					uint8_t prevCastlingRights = m.prevCastlingRights();

					if (prevCastlingRights & kingSide(color::ID(sntm)))
					{
						unsigned index = ::kingSideIndex(sntm);
						m_castleRookCurrent[index] = m_castleRookAtStart[index];
					}
					if (prevCastlingRights & queenSide(color::ID(sntm)))
					{
						unsigned index = ::queenSideIndex(sntm);
						m_castleRookCurrent[index] = m_castleRookAtStart[index];
					}

					// in Chess 960 the king may stand still
					if (from != m_ksq[sntm])
					{
						bothMask = fromMask ^ setBit(to);
						m_kings ^= bothMask;
						m_piece[to] = piece::Empty;
						m_ksq[sntm] = from;
						hashPiece(from, to, ::toPiece(piece::King, sntm));

						m_occupiedBy[sntm] ^= bothMask;
						m_occupiedL90 ^= MaskL90[from] ^ MaskL90[to];
						m_occupiedL45 ^= MaskL45[from] ^ MaskL45[to];
						m_occupiedR45 ^= MaskR45[from] ^ MaskR45[to];
					}

					// in handicap games the rook field is possibly empty
					if (m_piece[rookTo] != piece::Empty)
					{
						uint64_t rookMask = setBit(rookFrom) ^ setBit(rookTo);

						m_piece[rookTo] = piece::Empty;
						m_piece[rookFrom] = piece::Rook;
						m_rooks ^= rookMask;
						m_occupiedBy[sntm] ^= rookMask;
						m_occupiedL90 ^= MaskL90[rookFrom] ^ MaskL90[rookTo];
						m_occupiedL45 ^= MaskL45[rookFrom] ^ MaskL45[rookTo];
						m_occupiedR45 ^= MaskR45[rookFrom] ^ MaskR45[rookTo];
						hashPiece(rookFrom, rookTo, ::toPiece(piece::Rook, sntm));
					}

					m_piece[from] = piece::King;
					m_occupied = m_occupiedBy[White] | m_occupiedBy[Black];

					uint8_t castling = prevCastlingRights ^ m_castle;

					while (castling)
						hashCastling(Index(lsbClear(castling)));

					m_castle = prevCastlingRights;
					m_halfMoveClock = m.prevHalfMoves();
					m_epSquareFen = m.prevEpSquare();

					if (m.prevEpSquareExists())
					{
						m_epSquare = m_epSquareFen;
						hashEnPassant();
					}
					else
					{
						m_epSquare = Null;
					}

					hashToMove();
					swapToMove();
					--m_plyNumber;
				}
				return;

			case Move::Promote:
				static_assert(sizeof(m_progress.side[0].rank) < 7, "reimplement pawn progress");
				m_pawns ^= fromMask;
				m_piece[from] = piece::Pawn;
				--m_promotions;
				m_material.part[sntm].pawn = (1 << ++m_matCount[sntm].pawn) - 1;
				hashPawn(from, ::toPiece(piece::Pawn, sntm));

				switch (m.promoted())
				{
					case piece::Knight:
						m_knights ^= toMask;
						--m_underPromotions;
						m_material.part[sntm].knight = (1 << --m_matCount[sntm].knight) - 1;
						hashPiece(to, ::toPiece(piece::Knight, sntm));
						break;

					case piece::Bishop:
						m_bishops ^= toMask;
						--m_underPromotions;
						m_material.part[sntm].bishop = (1 << --m_matCount[sntm].bishop) - 1;
						hashPiece(to, ::toPiece(piece::Bishop, sntm));
						break;

					case piece::Rook:
						m_rooks ^= toMask;
						--m_underPromotions;
						m_material.part[sntm].rook = (1 << --m_matCount[sntm].rook) - 1;
						hashPiece(to, ::toPiece(piece::Rook, sntm));
						break;

					case piece::Queen:
						m_queens ^= toMask;
						m_material.part[sntm].queen = (1 << --m_matCount[sntm].queen) - 1;
						hashPiece(to, ::toPiece(piece::Queen, sntm));
						break;

					default:
						break;
				}
				break;
		}

		// Reverse captures
		unsigned replace = m.capturedType();

		switch (m.removal())
		{
			case piece::Empty:
				// extra cleanup needed for non-captures
				m_occupiedL90 ^= MaskL90[to];
				m_occupiedL45 ^= MaskL45[to];
				m_occupiedR45 ^= MaskR45[to];
				break;

			case piece::Pawn:
				m_pawns ^= toMask;
				m_occupiedBy[m_stm] ^= toMask;
				pawnProgressAdd(m_stm, to);
				m_material.part[m_stm].pawn = (1 << ++m_matCount[m_stm].pawn) - 1;
				hashPawn(to, ::toPiece(piece::Pawn, m_stm));
				break;

			case piece::Knight:
				m_knights ^= toMask;
				m_occupiedBy[m_stm] ^= toMask;
				m_material.part[m_stm].knight = (1 << ++m_matCount[m_stm].knight) - 1;
				hashPiece(to, ::toPiece(piece::Knight, m_stm));
				break;

			case piece::Bishop:
				m_bishops ^= toMask;
				m_occupiedBy[m_stm] ^= toMask;
				m_material.part[m_stm].bishop = (1 << ++m_matCount[m_stm].bishop) - 1;
				hashPiece(to, ::toPiece(piece::Bishop, m_stm));
				break;

			case piece::Rook:
				m_rooks ^= toMask;
				m_occupiedBy[m_stm] ^= toMask;
				m_material.part[m_stm].rook = (1 << ++m_matCount[m_stm].rook) - 1;
				hashPiece(to, ::toPiece(piece::Rook, m_stm));
				break;

			case piece::Queen:
				m_queens ^= toMask;
				m_occupiedBy[m_stm] ^= toMask;
				m_material.part[m_stm].queen = (1 << ++m_matCount[m_stm].queen) - 1;
				hashPiece(to, ::toPiece(piece::Queen, m_stm));
				break;

			case Move::En_Passant:
				replace = piece::Empty;
				// annoying move, the capture is not on the 'to' square
				unsigned epsq = PrevRank[sntm][to];
				m_piece[epsq] = piece::Pawn;
				m_pawns ^= setBit(epsq);
				m_occupiedBy[m_stm] ^= setBit(epsq);
				m_occupiedL90 ^= MaskL90[to] ^ MaskL90[epsq];
				m_occupiedL45 ^= MaskL45[to] ^ MaskL45[epsq];
				m_occupiedR45 ^= MaskR45[to] ^ MaskR45[epsq];
				m_material.part[m_stm].pawn = (1 << ++m_matCount[m_stm].pawn) - 1;
				pawnProgressAdd(m_stm, epsq);
				hashPawn(epsq, ::toPiece(piece::Pawn, m_stm));
				break;
		}
		// ...no we did not forget the king!

		m_piece[to] = replace;
		m_occupiedBy[sntm] ^= bothMask;
		m_occupiedL90 ^= MaskL90[from];
		m_occupiedL45 ^= MaskL45[from];
		m_occupiedR45 ^= MaskR45[from];
		m_occupied = m_occupiedBy[White] | m_occupiedBy[Black];

		uint8_t castling = m.prevCastlingRights() ^ m_castle;

		while (castling)
		{
			Index index = Index(lsbClear(castling));
			m_castleRookCurrent[index] = m_castleRookAtStart[index];
			hashCastling(index);
		}

		m_castle = m.prevCastlingRights();
	}

	m_halfMoveClock = m.prevHalfMoves();
	m_epSquareFen = m.prevEpSquare();

	if (m.prevEpSquareExists())
	{
		m_epSquare = m_epSquareFen;
		hashEnPassant();
	}
	else
	{
		m_epSquare = Null;
	}

	hashToMove();
	swapToMove();
	--m_plyNumber;
}


uint64_t
Board::pawnMovesFrom(Square s) const
{
	uint64_t targets = PawnF1[m_stm][s] & ~m_occupied;

	if (targets)
		targets |= PawnF2[m_stm][s] & ~m_occupied;

	uint64_t t = m_occupiedBy[m_stm ^ 1];

	if (m_epSquare != Null)
		t |= setBit(m_epSquare);

	return targets | (t & PawnAttacks[m_stm][s]);
}


uint64_t
Board::pawnCapturesTo(Square s) const
{
	uint64_t attackers	= 0;
	uint64_t destination	= setBit(s);

	if (m_stm == White)
	{
		// en passant moves
		if (m_epSquare != Null)
			attackers |= PawnAttacks[Black][m_epSquare];

		// captures
		attackers |= ::shiftDownRight(destination);
		attackers |= ::shiftDownLeft(destination);
		attackers &= m_occupiedBy[White];
	}
	else
	{
		// en passant moves
		if (m_epSquare != Null)
			attackers |= PawnAttacks[White][m_epSquare];

		// captures
		attackers |= ::shiftUpLeft(destination);
		attackers |= ::shiftUpRight(destination);
		attackers &= m_occupiedBy[Black];
	}

	return attackers & m_pawns;
}


bool
Board::checkMove(Move const& move, move::Constraint flag) const
{
	if (move.isEmpty())
		return false;
	if (move.isNull())
		return true;
	if (move.color() != m_stm)
		return false;

	Square from = move.from();

	if (from == Null)
		return false;
	if (piece(from) != move.pieceMoved())
		return false;

	uint64_t src = setBit(from);

	if (!(m_occupiedBy[m_stm] & src))
		return false;

	Square to = move.to();

	if (to == Null)
		return false;

	uint64_t dst = setBit(to);

	if (m_occupiedBy[m_stm] & dst)
	{
		if (move.action() != Move::Castle)
			return false;

		if (move.isShortCastling())
		{
			if (m_castleRookCurrent[::kingSideIndex(m_stm)] == Null)
				return flag == move::AllowIllegalMove;

			if (flag == move::AllowIllegalMove)
				return shortCastlingIsPossible();

			return canCastleShort(sideToMove()) && shortCastlingIsLegal();
		}

		if (m_castleRookCurrent[::queenSideIndex(m_stm)] == Null)
			return flag == move::AllowIllegalMove;

		if (flag == move::AllowIllegalMove)
			return longCastlingIsPossible();

		return canCastleLong(sideToMove()) && longCastlingIsLegal();
	}

	if ((move.capturedType() != piece::Empty) == !(m_occupiedBy[m_stm ^ 1] & dst))
		return to == m_epSquare && (pawns(color::ID(m_stm)) & src);

	switch (move.action())
	{
		case Move::Promote:
		case Move::One_Forward:
		case Move::Two_Forward:
			if (!(pawnMovesFrom(from) & dst))
				return false;
			break;

		case piece::King:
			if (!(kingAttacks(to) & src))
				return false;
			break;

		case piece::Queen:
			if (!(queenAttacks(to) & src))
				return false;
			break;

		case piece::Rook:
			if (!(rookAttacks(to) & src))
				return false;
			break;

		case piece::Bishop:
			if (!(bishopAttacks(to) & src))
				return false;
			break;

		case piece::Knight:
			if (!(knightAttacks(to) & src))
				return false;
			break;

		case Move::Castle:
			if (to == m_castleRookCurrent[::kingSideIndex(m_stm)])
			{
				if (flag == move::AllowIllegalMove)
					return shortCastlingIsPossible();

				return canCastleShort(sideToMove()) && shortCastlingIsLegal();
			}

			if (to == m_castleRookCurrent[::queenSideIndex(m_stm)])
			{
				if (flag == move::AllowIllegalMove)
					return longCastlingIsPossible();

				return canCastleLong(sideToMove()) && longCastlingIsLegal();
			}

			return false;
	}

	return true;
}


bool
Board::isValidMove(Move const& move, move::Constraint flag) const
{
	if (!checkMove(move, flag))
		return false;

	if (move.isNull())
	{
		unsigned state = checkState();
		return (state & (CheckMate | StaleMate)) == 0;
	}

	if (flag == move::AllowIllegalMove)
		return true;

	return !isIntoCheck(move);
}


Move
Board::prepareMove(Square from, Square to, move::Constraint flag) const
{
	//M_ASSERT(from != Null);
	//M_ASSERT(to != Null);

	uint64_t src = setBit(from);

	if (!(m_occupiedBy[m_stm] & src))
		return Move::empty();

	uint64_t dst = setBit(to);

	if (m_occupiedBy[m_stm] & dst)
	{
		// In Chess 960 a king move could both be the castling king move
		// or just a normal king move. This is why castling moves are
		// generated in the form king "takes" his own rook.
		// Example: e1h1 for the white short castle move in the standard
		// chess start position.
		// We have to catch this special case.

		if (m_ksq[m_stm] == from)
		{
			if (from < to)
			{
				if (m_castleRookCurrent[::kingSideIndex(m_stm)] == to && canCastleShort(color::ID(m_stm)))
				{
					if (shortCastlingIsLegal())
						return setMoveColor(setLegalMove(Move::genCastling(m_ksq[m_stm], to)));

					if (flag == move::AllowIllegalMove && shortCastlingIsPossible())
						return setMoveColor(Move::genCastling(m_ksq[m_stm], to));
				}
			}
			else
			{
				if (m_castleRookCurrent[::queenSideIndex(m_stm)] == to && canCastleLong(color::ID(m_stm)))
				{
					if (longCastlingIsLegal())
						return setMoveColor(setLegalMove(Move::genCastling(m_ksq[m_stm], to)));

					if (flag == move::AllowIllegalMove && longCastlingIsPossible())
						return setMoveColor(Move::genCastling(m_ksq[m_stm], to));
				}
			}
		}

		return Move::empty();
	}

	Byte piece		= m_piece[from];
	Byte captured	= m_piece[to];
	Move move;

	switch (piece)
	{
		case piece::Pawn:
			if (!(pawnMovesFrom(from) & dst))
				return Move::empty();

			if (to == m_epSquare)
			{
				move = Move::genEnPassant(from, to);
			}
			else if (dst & PawnF2[m_stm][from])
			{
				move = Move::genTwoForward(from, to);
			}
			else if (captured == piece::Empty)
			{
				if (dst & HomeRankMask[m_stm ^ 1])
					move = Move::genPromote(from, to, piece::Queen);
				else
					move = Move::genOneForward(from, to);
			}
			else
			{
				if (dst & HomeRankMask[m_stm ^ 1])
					move = Move::genCapturePromote(from, to, piece::Queen, captured);
				else
					move = Move::genPawnCapture(from, to, captured);
			}
			break;

		case piece::King:
			if (!(kingAttacks(to) & src))
			{
				if ((move = prepareCastle(from, to, flag)))
					move.setColor(m_stm);
				return move;
			}
			move = Move::genKingMove(from, to, captured);
			break;

		case piece::Queen:
			if (queenAttacks(to) & src)
				move = Move::genQueenMove(from, to, captured);
			break;

		case piece::Rook:
			if (rookAttacks(to) & src)
				move = Move::genRookMove(from, to, captured);
			break;

		case piece::Bishop:
			if (bishopAttacks(to) & src)
				move = Move::genBishopMove(from, to, captured);
			break;

		case piece::Knight:
			if (knightAttacks(to) & src)
				move = Move::genKnightMove(from, to, captured);
			break;
	}

	if (move)
	{
		move.setColor(m_stm);

		if (!isIntoCheck(move))
			move.setLegalMove();
		else if (flag == move::DontAllowIllegalMove)
			return Move::empty();
	}

	return move;
}


Move
Board::prepareCastle(Square from, Square to, move::Constraint flag) const
{
	if (!canCastle(sideToMove()))
		return Move::empty();

	if (whiteToMove())
	{
		if (from == e1)
		{
			switch (to)
			{
				case g1:
					if (m_castleRookCurrent[WhiteKS] == h1 && (m_castle & WhiteKingside))
					{
						if (shortCastlingWhiteIsLegal())
							return setLegalMove(Move::genCastling(m_ksq[White], m_castleRookCurrent[WhiteKS]));

						if (flag == move::AllowIllegalMove && shortCastlingWhiteIsPossible())
							return Move::genCastling(m_ksq[White], m_castleRookCurrent[WhiteKS]);
					}
					break;

				case c1:
					if (m_castleRookCurrent[WhiteQS] == a1 && (m_castle & WhiteQueenside))
					{
						if (longCastlingWhiteIsLegal())
							return setLegalMove(Move::genCastling(m_ksq[White], m_castleRookCurrent[WhiteQS]));

						if (flag == move::AllowIllegalMove && longCastlingWhiteIsPossible())
							return Move::genCastling(m_ksq[White], m_castleRookCurrent[WhiteQS]);
					}
					break;
			}
		}

		if (to == m_castleRookCurrent[WhiteKS])
		{
			if (m_castle & WhiteKingside)
			{
				if (shortCastlingWhiteIsLegal())
					return setLegalMove(Move::genCastling(m_ksq[White], m_castleRookCurrent[WhiteKS]));

				if (flag == move::AllowIllegalMove && shortCastlingWhiteIsPossible())
					return Move::genCastling(m_ksq[White], m_castleRookCurrent[WhiteKS]);
			}
		}
		else if (to == m_castleRookCurrent[WhiteQS])
		{
			if (m_castle & WhiteQueenside)
			{
				if (longCastlingWhiteIsLegal())
					return setLegalMove(Move::genCastling(m_ksq[White], m_castleRookCurrent[WhiteQS]));

				if (flag == move::AllowIllegalMove && longCastlingWhiteIsPossible())
					return Move::genCastling(m_ksq[White], m_castleRookCurrent[WhiteQS]);
			}
		}
	}
	else
	{
		if (from == e8)
		{
			switch (to)
			{
				case g8:
					if (m_castleRookCurrent[BlackKS] == h8 && (m_castle & BlackKingside))
					{
						if (shortCastlingBlackIsLegal())
							return setLegalMove(Move::genCastling(m_ksq[Black], m_castleRookCurrent[BlackKS]));

						if (flag == move::AllowIllegalMove && shortCastlingBlackIsPossible())
							return Move::genCastling(m_ksq[Black], m_castleRookCurrent[BlackKS]);
					}
					break;

				case c8:
					if (m_castleRookCurrent[BlackQS] == a8 && (m_castle & BlackQueenside))
					{
						if (longCastlingBlackIsLegal())
							return setLegalMove(Move::genCastling(m_ksq[Black], m_castleRookCurrent[BlackQS]));

						if (flag == move::AllowIllegalMove && longCastlingBlackIsPossible())
							return Move::genCastling(m_ksq[Black], m_castleRookCurrent[BlackQS]);
					}
					break;
			}
		}

		if (to == m_castleRookCurrent[BlackKS])
		{
			if (m_castle & BlackKingside)
			{
				if (shortCastlingBlackIsLegal())
					return setLegalMove(Move::genCastling(m_ksq[Black], m_castleRookCurrent[BlackKS]));

				if (flag == move::AllowIllegalMove && shortCastlingBlackIsPossible())
					return Move::genCastling(m_ksq[Black], m_castleRookCurrent[BlackKS]);
			}
		}
		else if (to == m_castleRookCurrent[BlackQS])
		{
			if (m_castle & BlackQueenside)
			{
				if (longCastlingBlackIsLegal())
					return setLegalMove(Move::genCastling(m_ksq[Black], m_castleRookCurrent[BlackQS]));

				if (flag == move::AllowIllegalMove && longCastlingBlackIsPossible())
					return Move::genCastling(m_ksq[Black], m_castleRookCurrent[BlackQS]);
			}
		}
	}

	return Move::empty();
}


Move
Board::makeMove(Square from, Square to, piece::Type promoted) const
{
	// NOTE: we assume a valid move (but illegal moves are allowed)

	unsigned piece		= m_piece[from];
	unsigned captured	= m_piece[to];

	switch (piece)
	{
		case piece::Pawn:
			if (mstl::abs(to - from) == 16)
				return setMoveColor(Move::genTwoForward(from, to));

			if (to == m_epSquare)
				return setMoveColor(Move::genEnPassant(from, to));

			if (::rank(to) == HomeRank[m_stm ^ 1])
			{
				if (captured == piece::Empty)
					return setMoveColor(Move::genPromote(from, to, promoted));

				return setMoveColor(Move::genCapturePromote(from, to, promoted, captured));
			}

			if (captured == piece::Empty)
				return setMoveColor(Move::genOneForward(from, to));

			return setMoveColor(Move::genPawnCapture(from, to, captured));

		case piece::King:
			// the following takes illegal castlings into account:
			if (m_occupiedBy[m_stm] & setBit(to))
				return setMoveColor(Move::genCastling(from, to));

			// the following takes into account that the rook is probably missing
			// (handicap game; only allowed in standard chess)
			switch (int(from) - int(to))
			{
				case -2:
					return setMoveColor(
						Move::genCastling(from, m_castleRookCurrent[::queenSideIndex(m_stm)]));

				case +2:
					return setMoveColor(Move::genCastling(from, m_castleRookCurrent[::kingSideIndex(m_stm)]));
			}

			return setMoveColor(Move::genKingMove(from, to, captured));

		case piece::Queen:
			return setMoveColor(Move::genQueenMove(from, to, captured));

		case piece::Rook:
			return setMoveColor(Move::genRookMove(from, to, captured));

		case piece::Bishop:
			return setMoveColor(Move::genBishopMove(from, to, captured));

		case piece::Knight:
			return setMoveColor(Move::genKnightMove(from, to, captured));
	}

	return Move::null();
}


piece::ID
Board::pieceAt(Square s) const
{
	uint64_t mask = setBit(s);

	if (!(m_occupied & mask))
		return piece::Empty;

	return ::toPiece(m_piece[s], m_occupiedBy[White] & mask ? White : Black);
}


bool
Board::needCastlingFyles() const
{
	uint64_t whiteRooks = rooks(White) & RankMask1;
	uint64_t blackRooks = rooks(Black) & RankMask8;

	return	(	whiteRooks
				&& (m_castle & WhiteKingside)
				&& count(whiteRooks & ~(setBit(m_ksq[White]) - 1)) > 1)
			|| (	blackRooks
				&& (m_castle & BlackKingside)
				&& count(blackRooks & ~(setBit(m_ksq[Black]) - 1)) > 1)
			|| (	whiteRooks
				&& (m_castle & WhiteQueenside)
				&& count(whiteRooks &  (setBit(m_ksq[White]) - 1)) > 1)
			|| (	blackRooks
				&& (m_castle & BlackQueenside)
				&& count(blackRooks &  (setBit(m_ksq[Black]) - 1)) > 1);
}


mstl::string
Board::asString() const
{
	mstl::string result;

	for (unsigned i = 0; i < 64; ++i)
	{
		piece::ID piece = pieceAt(i);
		result += piece == piece::Empty ? '.' : piece::print(piece);
	}

	result += whiteToMove() ? 'w' : 'b';

	return result;
}


mstl::string&
Board::toFen(mstl::string& result, Format format) const
{
	result.reserve(result.size() + 90);

	// piece placement
	for (int row = 7, empty = 0; row >= 0; --row)
	{
		for (unsigned col = 0; col < 8; ++col)
		{
			piece::ID piece = pieceAt(::mul8(row) + col);

			if (piece == piece::Empty)
			{
				++empty;
			}
			else
			{
				if (empty)
				{
					result += char(empty + '0');
					empty = 0;
				}

				result += piece::print(piece);
			}
		}

		if (empty)
		{
			result += char(empty + '0');
			empty = 0;
		}

		if (row > 0)
			result += '/';
	}

	// side to move
	result += whiteToMove() ? " w " : " b ";

	// castling rights
	if (castlingRights() == NoRights)
	{
		result += "- ";
	}
	else
	{
		if (castlingRights() & WhiteBothSides)
		{
			uint64_t rooks = this->rooks(White) & RankMask1;

			if (castlingRights() & WhiteKingside)
			{
				int sq = m_castleRookAtStart[WhiteKS];

				//M_ASSERT(sq != sq::Null);

				if (format == Shredder)
				{
					result += printFYLE(sq);
				}
				else
				{
					int rt = msb(rooks);

					if (rt > sq || mstl::is_between(lsb(rooks), int(m_ksq[White]), rt - 1))
						result += printFYLE(sq);
					else
						result += 'K';
				}
			}

			if (castlingRights() & WhiteQueenside)
			{
				int sq = m_castleRookAtStart[WhiteQS];

				//M_ASSERT(sq != sq::Null);

				if (format == Shredder)
				{
					result += printFYLE(sq);
				}
				else
				{
					int lt = lsb(rooks);

					if (lt < sq || mstl::is_between(msb(rooks), lt + 1, int(m_ksq[White])))
						result += printFYLE(sq);
					else
						result += 'Q';
				}
			}
		}

		if (castlingRights() & BlackBothSides)
		{
			uint64_t rooks = this->rooks(Black) & RankMask8;

			if (castlingRights() & BlackKingside)
			{
				int sq = m_castleRookAtStart[BlackKS];

				//M_ASSERT(sq != sq::Null);

				if (format == Shredder)
				{
					result += printFyle(sq);
				}
				else
				{
					int rt = msb(rooks);

					if (rt > sq || mstl::is_between(lsb(rooks), int(m_ksq[Black]), rt - 1))
						result += printFyle(sq);
					else
						result += 'k';
				}
			}

			if (castlingRights() & BlackQueenside)
			{
				int sq = m_castleRookAtStart[BlackQS];

				//M_ASSERT(sq != sq::Null);

				if (format == Shredder)
				{
					result += printFyle(sq);
				}
				else
				{
					int lt = lsb(rooks);

					if (lt < sq || mstl::is_between(msb(rooks), lt + 1, int(m_ksq[Black])))
						result += printFyle(sq);
					else
						result += 'q';
				}
		}
		}

		result += ' ';
	}

	// en passant square
	if (m_epSquareFen == Null)
	{
		result += '-';
	}
	else
	{
		result += printFyle(sq::ID(m_epSquareFen));
		result += printRank(sq::ID(m_epSquareFen));
	}
	result += ' ';

	// half move clock
	result.format("%u", unsigned(halfMoveClock()));

	// move number
	result.format(" %u", unsigned(moveNumber()));

	return result;
}


mstl::string
Board::toFen(Format format) const
{
	mstl::string fen;
	return toFen(fen, format);
}


bool
Board::doMoves(char const* text)
{
	while (::isspace(*text)) ++text;
	while (*text && !::isalpha(*text)) ++text;

	while (*text)
	{
		Move move = parseMove(text);

		if (!move.isLegal())
			return false;

		doMove(move);

		while (*text && !::isspace(*text)) ++text;
		while (*text && !::isalpha(*text)) ++text;
	}

	return true;
}


bool
Board::hasBishopOnDark(color::ID side) const
{
	return bishops(side) & ::DarkSquares;
}


bool
Board::hasBishopOnLite(color::ID side) const
{
	return bishops(side) & ::LiteSquares;
}


void
Board::dump() const
{
	::printf("\n");

	for (unsigned r = 8; r > 0; --r)
	{
		for (unsigned f = 0; f < 8; ++f)
		{
			Square s = (r - 1)*8 + f;
			::printf("%c", pieceAt(s) == piece::Empty ? '-' : piece::print(pieceAt(s)));
		}

		::printf("\n");
	}

	::printf("\n");
	::fflush(stdout);
}


void
Board::initialize()
{
	// Empty board
	::memset(&m_emptyBoard, 0, sizeof(m_emptyBoard));
	::memset(m_emptyBoard.m_destroyCastle, 0xff, sizeof(m_emptyBoard.m_destroyCastle));
	::memset(m_emptyBoard.m_castleRookCurrent, Null, 4);
	::memset(m_emptyBoard.m_castleRookAtStart, Null, 4);
	m_emptyBoard.m_epSquare = Null;
	m_emptyBoard.m_epSquareFen = Null;
	m_emptyBoard.m_ksq[0] = Null;
	m_emptyBoard.m_ksq[1] = Null;

	// Standard board
	::memset(&m_standardBoard, 0, sizeof(m_standardBoard));
	m_standardBoard.setup("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
	::memset(m_standardBoard.m_unambiguous, true, 4);

	// Shuffle Chess board
	::memset(&m_shuffleChessBoard, 0, sizeof(m_shuffleChessBoard));
	m_shuffleChessBoard.setup("8/pppppppp/8/8/8/8/PPPPPPPP/8 w - - 0 1");
	::memset(m_shuffleChessBoard.m_unambiguous, true, 4);
	m_shuffleChessBoard.m_matCount[White].value = m_standardBoard.m_matCount[White].value;
	m_shuffleChessBoard.m_matCount[Black].value = m_standardBoard.m_matCount[Black].value;
	m_shuffleChessBoard.m_material.value = m_standardBoard.m_material.value;
}

// vi:set ts=3 sw=3:
