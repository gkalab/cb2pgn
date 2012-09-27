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

#include "db_guess.h"
#include "db_board.h"
#include "db_board_base.h"
#include "db_eco_table.h"

#include "m_utility.h"
#include "m_bit_functions.h"
#include "m_assert.h"
#include "m_stdio.h"

#include <string.h>

#ifdef USE_NULL_MOVE_SEARCH
# define SEARCH(moves, depth, alpha, beta, allowNull)		search(moves, depth, alpha, beta, allowNull)
# define ITERATE(moves, depth, alpha, beta, allowNull)	iterate(moves, depth, alpha, beta, allowNull)
# define INCR(counter)												++counter
# define DECR(counter)												--counter
#else
# define SEARCH(moves, depth, alpha, beta, allowNull)		search(moves, depth, alpha, beta)
# define ITERATE(moves, depth, alpha, beta, allowNull)	iterate(moves, depth, alpha, beta)
# define INCR(counter)
# define DECR(counter)
#endif

//#define USE_PV_SEARCH
//#define USE_ASPIRATION_WINDOW	// currently not working

// help lexers
#define LT_BRACE {
#define RT_BRACE }

//#define TRACE(expr) (LT_BRACE expr; RT_BRACE)

#ifndef TRACE
# define TRACE(expr)
#endif

using namespace db;
using namespace db::sq;
using namespace db::color;
using namespace db::castling;
using namespace db::board;


int const db::Guess::Piece[8] =
{
	0, KingValue, QueenValue, RookValue, BishopValue, KnightValue, PawnValue
};


namespace {

typedef db::Guess::Score Score;
typedef db::Guess::Transposition Transposition;
typedef int ScoreList[MoveList::Maximum_Moves];

} // namespace


// ------------------------------------------------------------------------
// class Transposition is adopted from toga/trans.cpp
// ------------------------------------------------------------------------
class db::Guess::Transposition
{
public:

	struct TransEntry
	{
		uint64_t key;
		uint16_t move;
		int16_t  score;
		uint16_t depth;
		uint16_t flags;

		TransEntry() : key(0), move(0), score(0), depth(0), flags(0) {}
	};

	enum { Lower = 1, Upper = 2, Exact = 3 };

	Transposition();

	bool lookup(uint64_t key, uint16_t& move, int& score, unsigned depth, int& alpha, int& beta) const;
	void store(uint64_t key, uint16_t move, unsigned depth, int score, int alpha, int beta);

private:

	enum { TableSize = 32768 };	// 0.5 MB

	TransEntry	m_table[TableSize];
	unsigned		m_size;
};


Transposition::Transposition() : m_size(0) {}


bool
Transposition::lookup(	uint64_t key,
								uint16_t& move,
								int& score,
								unsigned depth,
								int& alpha,
								int& beta) const
{
	TransEntry const* entry	= &m_table[key & (TableSize - 1)];

	if (entry->key == key)
	{
		move = entry->move;

		if (entry->depth < depth)
			return false;

		switch (entry->flags)
		{
			case Exact:
				score = entry->score;
				return true;

			case Upper:
				if (entry->score <= alpha)
				{
					score = entry->score;
					return true;
				}

				if (entry->score < beta)
					beta = entry->score;

				break;

			case Lower:
				if (entry->score >= beta)
				{
					score = entry->score;
					return true;
				}

				if (entry->score > alpha)
					alpha = entry->score;

				break;
		}

		return false;
	}

	return false;
}


void
Transposition::store(uint64_t key, uint16_t move, unsigned depth, int score, int alpha, int beta)
{
	//M_ASSERT(mstl::abs(score) < 32768);

   TransEntry* bestEntry	= 0;
	TransEntry* entry			= &m_table[key & (TableSize - 1)];

   int bestValue = Exact + 1;

	if (entry->key == key)
	{
		if (entry->depth <= depth)
		{
			entry->depth = depth;
			entry->flags = 0;
			entry->score = score;
			entry->move = move;
		}

		if (score > alpha)	entry->flags |= Lower;
		if (score < beta)		entry->flags |= Upper;
	}
	else
	{
	   if (entry->flags < bestValue)
		{
			bestEntry = entry;
			bestValue = entry->flags;
		}

		entry = bestEntry;

		//M_ASSERT(entry);
		//M_ASSERT(entry->key != key);

		entry->key = key;
		entry->depth = depth;
		entry->flags = 0;
		entry->score = score;
		entry->move = move;

		if (score > alpha)	entry->flags |= Lower;
		if (score < beta)		entry->flags |= Upper;
	}
}


Score&
Score::operator+=(int score)
{
	middleGame += score;
	endGame += score;
	return *this;
}


Score&
Score::operator-=(int score)
{
	middleGame -= score;
	endGame -= score;
	return *this;
}


Score&
Score::operator+=(Score const& score)
{
	middleGame += score.middleGame;
	endGame += score.endGame;
	return *this;
}


Score&
Score::operator-=(Score const& score)
{
	middleGame -= score.middleGame;
	endGame -= score.endGame;
	return *this;
}


int
Score::weightedScore(int totalPiecesWhite, int totalPiecesBlack) const
{
	double phase = mstl::min(62.0, double(totalPiecesWhite + totalPiecesBlack));
	return int(((middleGame*phase) + (endGame*(62.0 - phase)))/62.0 + 0.5);
}


db::Guess::Guess(Board const& board, unsigned idn)
	:Board(board)
	,m_idn(idn)
	,m_trans(new Transposition)
	,m_ply(0)
#ifdef USE_NULL_MOVE_SEARCH
	,m_pvCounter(0)
#endif
{
	//M_REQUIRE(idn <= 4*960);
}


db::Guess::~Guess() throw()
{
	delete m_trans;
}


unsigned db::Guess::minor(Material mat) { return mat.knight + mat.bishop; }
unsigned db::Guess::major(Material mat) { return mat.queen + mat.rook; }


unsigned
db::Guess::total(Material mat)
{
	return	piece::value::Queen*mat.queen
			 + piece::value::Rook*mat.rook
			 + piece::value::Bishop*mat.bishop
			 + piece::value::Knight*mat.knight;
}


Move
db::Guess::setColor(Move move)
{
	if (move)
	{
		//M_ASSERT(move.isLegal());
		move.setColor(sideToMove());
	}

	return move;
}


void
db::Guess::addKillerMove(Move const& move, int score)
{
	if (score < 0 || !move.isCaptureOrPromotion())
	{
		//M_ASSERT(m_ply < U_NUMBER_OF(m_killer)/2);

		uint32_t* killer = m_killer + mstl::mul2(m_ply);

		if (killer[0] != move.data())
		{
			killer[1] = killer[0];
			killer[0] = move.data();

			TRACE(::printf("%d: killer move(%s): %d\n", m_ply, move.asString().c_str(), score));
		}
	}
}


bool
db::Guess::isKillerMove(uint32_t move, unsigned index)
{
	//M_ASSERT(m_ply < U_NUMBER_OF(m_killer)/2);
	//M_ASSERT(index < 2);

	return (m_killer + mstl::mul2(m_ply))[index] == move;
}


void
db::Guess::generateMoves(Square square, MoveList& result) const
{
	//M_ASSERT(square != sq::Null);

	MoveList moves;
	Board::generateMoves(moves);

	uint64_t sqMask = ::setBit(square);

	if ((m_occupied & sqMask) && m_stm == (m_occupiedBy[White] & sqMask ? White : Black))
	{
		if (piece(square) == piece::Rook)
		{
			for (unsigned i = 0; i < moves.size(); ++i)
			{
				if ((moves[i].isCastling() ? moves[i].castlingRookFrom() : moves[i].from()) == square)
					result.append(moves[i]);
			}
		}
		else
		{
			for (unsigned i = 0; i < moves.size(); ++i)
			{
				if (moves[i].from() == square)
					result.append(moves[i]);
			}
		}
	}
	else
	{
		for (unsigned i = 0; i < moves.size(); ++i)
		{
			if (moves[i].to() == square)
				result.append(moves[i]);
		}
	}
}


Move
db::Guess::search(Square square, unsigned maxDepth)
{
	MoveList moves;
	generateMoves(square, moves);
	return setColor(search(moves, maxDepth));
}


Move
db::Guess::bestMove(Square square, MoveList const& exclude, unsigned maxDepth)
{
	EcoTable::Successors successors;
	EcoTable::specimen().getSuccessors(hash(), successors);

	MoveList moves;
	MoveList ecoMoves;

	generateMoves(square, moves);

	if (!exclude.isEmpty())
	{
		MoveList ml(moves);
		moves.clear();

		Move const* m = ml.begin();
		Move const* e = ml.end();

		for ( ; m != e; ++m)
		{
			if (exclude.find(m->index()) == -1)
				moves.append(*m);
		}
	}

	for (unsigned i = 0; i < successors.length; ++i)
	{
		Move m(successors.list[i].move);

		if (m.from() == square || m.to() == square)
		{
			int k = moves.find(successors.list[i].move);

			if (__builtin_expect(k >= 0, 1))
			{
				if (successors.list[i].weight > 0)
					return setColor(makeMove(successors.list[i].move));

				ecoMoves.append(moves[k]);
			}
			else if (	piece(m.from()) != piece::Pawn
						|| piece(m.to()) != piece::None
						|| sq::fyle(m.from()) == sq::fyle(m.to()))
			{
				// successor is not en-passant move: hash clash has occurred!
				return setColor(search(moves, maxDepth));
			}
		}
	}

	if (ecoMoves.isEmpty())
		return setColor(search(moves, maxDepth));

	return setColor(search(ecoMoves, maxDepth));
}


Move
db::Guess::bestMove(Square square, unsigned maxDepth)
{
	return bestMove(square, MoveList(), maxDepth);
}


Square
db::Guess::bestSquare(Square square, unsigned maxDepth)
{
	Move bestMove = this->bestMove(square, maxDepth);

	if (!bestMove)
		return sq::Null;

	return bestMove.from() == square ? bestMove.to() : bestMove.from();
}


Move
db::Guess::search(MoveList& moves, unsigned maxDepth)
{
	filterLegalMoves(moves);

	if (moves.isEmpty())
		return Move::empty();

	if (moves.size() == 1)
		return moves[0];

	if (isInCheck())
		++maxDepth;

	maxDepth = mstl::min(maxDepth, unsigned(MaxDepth));

	::memset(m_killer, 0, sizeof(m_killer));
	preEvaluate();

	ScoreList scores;

	for (unsigned i = 0; i < moves.size(); ++i)
	{
		Move& move = moves[i];

		prepareUndo(move);
		doMove(move);
		scores[i] = -quiesce(-Infinity, Infinity, move.isPromotion());
		TRACE(::printf("%d: quiesce(%s) = %d\n", m_ply, move.asString().c_str(), scores[i]));
		undoMove(move);
	}

	moves.sort(scores);

	int bestScore = scores[0];

	TRACE(::printf("1: search() = %d (best move: %s)\n", bestScore, moves[0].asString().c_str()));

	if (moves.isEmpty() || bestScore <= -Infinity)
		return Move::empty();

	if (bestScore >= Infinity || maxDepth == 1)
		return moves[0];

	int score;

#ifdef USE_ASPIRATION_WINDOW
	enum { AspirationWindow = 35 };

	int alpha = bestScore - AspirationWindow;
	int beta  = bestScore + AspirationWindow;

	score = ITERATE(moves, maxDepth, alpha, beta, true);
	TRACE(::printf("%d: iterator(%s) = %d\n", i, moves[0].asString().c_str(), score));

	if (score >= beta)
	{
		alpha = score - 1;
		beta = Infinity + 1;
		score = ITERATE(moves, maxDepth, alpha, beta, true);
	}
	else if (score <= alpha)
	{
		alpha = -Infinity - 1;
		beta = score + 1;
		score = ITERATE(moves, maxDepth, alpha, beta, true);
	}

	if (score < alpha || beta < score)
#else
		score = ITERATE(moves, maxDepth, -Infinity, Infinity, true);
#endif

	if (score >= Infinity)
		return moves[0];

	TRACE(::printf("best move: %s\n", moves[0].asString().c_str()));

	return moves[0];
}


int
db::Guess::SEARCH(MoveList& moves, unsigned depth, int alpha, int beta, bool allowNull)
{
	//M_ASSERT(depth >= 1);
	//M_ASSERT(alpha <= beta);

	uint16_t	bestMove	= 0;
	int		score;

	if (m_trans->lookup(m_hash, bestMove, score, depth, alpha, beta))
	{
		int bestMoveIndex = moves.find(bestMove);
		//M_ASSERT(bestMoveIndex >= 0);
		mstl::swap(moves[0], moves[bestMoveIndex]);
		TRACE(::printf("%d (%d): lookup(%s) = %d\n", depth, m_ply, moves[0].asString().c_str(), score));
		return score;
	}

	if (depth == 1)
		return iterate(moves, alpha, beta);

	ScoreList scores;
	unsigned	k = 0;

	for (unsigned i = 0; i < moves.size(); ++i)
	{
		Move& move = moves[i];

		if (!isIntoCheck(move))
		{
			if (move.index() == bestMove)
			{
				scores[k] = 10000;
			}
			else if (move.isCaptureOrPromotion())
			{
				int see = staticExchangeEvaluator(move);
				scores[k] = see >= 0 ? 1000 + see : see;
			}
			else if (isKillerMove(move.data(), 1))
			{
				scores[k] = 1001;
			}
			else if (isKillerMove(move.data(), 0))
			{
				scores[k] = 1000;
			}
			else if (move.isCastling())
			{
				scores[k] = 100;
			}
			else
			{
				scores[k] = 0;
			}

			prepareUndo(move);
			moves[k++] = move;
		}
	}

	moves.cut(k);
	moves.sort(scores);

	if (moves.isEmpty())
		return isInCheck() ? -Infinity  : 0;	// either checkmate or stalemate

	score = ITERATE(moves, depth, alpha, beta, allowNull);
	TRACE(::printf("%u (%d): iterator(%s) = %d\n", depth, m_ply, moves[0].asString().c_str(), score));
	m_trans->store(m_hash, moves[0].index(), m_ply, score, alpha, beta);

	return score;
}


int
db::Guess::iterate(MoveList& moves, int alpha, int beta)
{
	unsigned k = 0;

	for (unsigned i = 0; i < moves.size(); ++i)
	{
		Move move = moves[i];

		prepareUndo(move);
		doMove(move);

		if (isLegal())
		{
			int score = -quiesce(-beta, -alpha, move.isPromotion());
			TRACE(::printf("%d: quiesce(%s) = %d\n", m_ply, move.asString().c_str(), score));

			if (score >= beta)
			{
				addKillerMove(move, score);
				mstl::swap(moves[0], moves[i]);
				undoMove(move);
				return score;
			}

			if (score > alpha)
			{
				alpha = score;
				mstl::swap(moves[0], moves[i]);
			}

			moves[k++] = move;
		}

		undoMove(move);
	}

	if (moves.isEmpty())
		return isInCheck() ? -Infinity  : 0;	// either checkmate or stalemate

	return alpha;
}


int
db::Guess::ITERATE(MoveList& moves, unsigned depth, int alpha, int beta, bool allowNull)
{
	//M_ASSERT(!moves.isEmpty());
	//M_ASSERT(depth > 1);
	//M_ASSERT(alpha <= beta);

	int score = 0;	// satisfies the compiler

	unsigned baseExtension = (moves.size() == 1);

#ifdef USE_PV_SEARCH
	bool foundPV = false;
#endif

	for (unsigned i = 0; i < moves.size(); ++i)
	{
		Move move = moves[i];

		++m_ply;
		doMove(move);
		//M_ASSERT(isLegal());

		MoveList moveList;

		unsigned extension = baseExtension;

		if (	move.pieceMoved() == piece::Pawn
			&& !mstl::is_between(sq::rank(move.to()), sq::Rank3, sq::Rank6))
		{
			++extension;
		}

		// reduce extension if the search is deep
		if (m_ply >= mstl::mul2(depth))
			extension = mstl::div2(extension);

		// Limit extension to 1
		if (extension > 1)
			extension = 1;

#ifdef USE_PV_SEARCH
		if (foundPV)
		{
			Board::generateMoves(moveList);

			INCR(m_pvCounter);
			// Do a minimal window search first, to try and quickly
			// identify the common case of a move not being good
			// to improve alpha.
			score = -SEARCH(moveList, depth + extension - 1, -alpha - 1, -alpha, true);
			DECR(m_pvCounter);

			if (score > alpha && score < beta)
			{
				// This move is good enough to search with the proper
				// window; use the score it returned as the lower bound.
				score = -SEARCH(moveList, depth - 1, -beta, -score, true);
			}
		}
		else
#endif
		{
#ifdef USE_NULL_MOVE_SEARCH
			enum { NullDepth = 2 };

			//	null-move pruning allowed only if none of the
			//	following conditions are satsified:
			//		1. previous move was a null move
			//		2. inside principal variation
			//		3. side to move is in check
			//		4. side to move has less than 6 pieces

			if (	allowNull
				&& m_pvCounter == 0
				&& depth > NullDepth + 1
				&& m_totalPieces[m_stm] >= 6
				&& !isInCheck())
			{
				Move null(Move::null());

				prepareUndo(null);
				doMove(null);
				Board::generateMoves(moveList);
				score = -SEARCH(moveList, -beta, -beta + 1, depth + extension - NullDepth - 1, false);
				undoMove(null);

				if (score >= beta)
				{
					undoMove(move);
					--m_ply;
					addKillerMove(move, score);
					mstl::swap(moves[0], moves[i]);
					TRACE(::printf("%d (%d): move-back: %s\n", depth, m_ply + 1, move.asString().c_str()));
					return score;
				}
			}
#endif // USE_NULL_MOVE_SEARCH

			Board::generateMoves(moveList);
			score = -SEARCH(moveList, depth + extension - 1, -beta, -alpha, true);
			TRACE(::printf("%d (%d): search(%s) = %d\n", depth, m_ply, move.asString().c_str(), score));
		}

		undoMove(move);
		--m_ply;

		if (score >= beta)
		{
			addKillerMove(move, score);
			mstl::swap(moves[0], moves[i]);
			return score;
		}

		if (score > alpha)
		{
			alpha = score;
			mstl::swap(moves[0], moves[i]);
#ifdef USE_PV_SEARCH
			foundPV = true;
#endif
		}
	}

	return alpha;
}


int
db::Guess::quiesce(int alpha, int beta, bool isPromotion)
{
	if (!isPromotion)
		return quiesce(alpha, beta);

	MoveList moves;
	Board::generateMoves(moves);

	return SEARCH(moves, 1, alpha, beta, false);
}


int
db::Guess::quiesce(int alpha, int beta)
{
	int score = evaluate(alpha, beta);

	if (isBlack(sideToMove()))
		score = -score;

	if (score >= beta)
		return score;

	if (score + QueenValue + PawnValue < alpha)
		return alpha;

	if (score > alpha)
		alpha = score;

	MoveList		moves;
	ScoreList	scoreList;

	Board::generateCapturingMoves(moves);
	filterLegalMoves(moves);

	//M_ASSERT(moves.size() <= U_NUMBER_OF(scoreList));

	for (unsigned i = 0; i < moves.size(); ++i)
		scoreList[i] = staticExchangeEvaluator(moves[i]);

	int value = score;

	for (unsigned i = 0; i < moves.size(); ++i)
	{
		moves.sort(i, scoreList);

		Move			move		= moves[i];
		piece::Type	promoted	= move.promoted();

		// all moves but under-promotions:
		if (promoted == piece::None || promoted == piece::Queen)
		{
			if (scoreList[i] < mstl::max(0, alpha - value - PawnValue))
				return alpha;	// we cannot improve alpha

			doMove(move);
			//M_ASSERT(isLegal());
			score = -quiesce(-beta, -alpha);
			TRACE(::printf("(%d): quiesce(%s) = %d\n", m_ply, move.asString().c_str(), score));
			undoMove(move);

			if (score >= beta)
				return score;

			if (score > alpha)
				alpha = score;
		}
	}

	return alpha;
}


// ------------------------------------------------------------------------
// adopted from crafty-15.17/swap.c:Swap()
// ------------------------------------------------------------------------
// A Static Exchange Evaluator (or SEE for short).
//
// SSE is used to analyze capture moves to see whether or not they appear
// to be profitable. The basic algorithm is extremely fast since it uses the
// bitmaps to determine which squares are attacking the <target> square.
//
// The algorithm is quite simple. Using the attack bitmaps, we enumerate all
// the pieces that are attacking <target> for either side. Then we simply
// use the lowest piece (value) for the correct side to capture on <target>.
// We continually "flip" sides taking the lowest piece each time.
//
// As a piece is used, if it is a sliding piece (pawn, bishop, rook or queen)
// we "peek" behind it to see if it is attacked by a sliding piece in the
// direction away from the piece being captured. If so, and that sliding
// piece moves in this direction, then it is added to the list of attackers
// since its attack has been "uncovered" by moving the capturing piece.
int
db::Guess::staticExchangeEvaluator(Move const& move)
{
	int attackedPiece;
	int swapList[65];

	// Initialize by placing the piece on target first in
	// the list as it is being captured to start things off.
	if (move.isPromotion())
	{
		attackedPiece = Piece[move.promoted()];
		swapList[0] = attackedPiece - PawnValue;
	}
	else
	{
		attackedPiece = Piece[move.pieceMoved()];
		swapList[0] = Piece[move.captured()];

		if (attackedPiece != RookValue)
		{
			// Find the estimated result assuming one recapture:
			int fastResult = swapList[0] - attackedPiece;

			// We can do quick estimation for a big gain, but have to be
			// careful since move ordering is very sensitive to positive SEE
			// scores. Only return a fast estimate for PxQ, NxQ, BxQ and PxR:
			if (fastResult > KnightValue)
				return fastResult;
		}
	}

	int target	= move.to();
	int n			= 1;

	uint64_t fromMask		= ::setBit(move.from());
	uint64_t occupied		= m_occupiedBy[m_stm ^ 1] & ~fromMask;
	uint64_t occupied2	= m_occupiedBy[m_stm] & ~fromMask;
	uint64_t pawns			= PawnAttacks[m_stm][target] & m_pawns;
	uint64_t pawns2		= PawnAttacks[m_stm ^ 1][target] & m_pawns;
	uint64_t knights		= knightAttacks(target) & m_knights;
	uint64_t bishops		= bishopAttacks(target) & m_bishops;
	uint64_t rooks			= rookAttacks(target) & m_rooks;
	uint64_t queens		= queenAttacks(target) & m_queens;
	uint64_t kings			= kingAttacks(target) & m_kings;

	occupied |= addXrayPiece(move.from(), target);

	// Now pick out the least valuable piece for the correct
	// side that is bearing on <target>. As we find one, we
	// call addXrayPiece() to add the piece behind this piece
	// that is indirectly bearing on <target> (if any).
	for ( ; occupied; ++n)
	{
		if (pawns & occupied)
		{
			int square = lsb(pawns & occupied);
			occupied &= ~::setBit(square);
			occupied |= addXrayPiece(square, target);
			swapList[n] = -swapList[n - 1] + attackedPiece;
			attackedPiece = PawnValue;
		}
		else if (knights & occupied)
		{
			occupied &= ~::setBit(lsb(knights & occupied));
			swapList[n] = -swapList[n - 1] + attackedPiece;
			attackedPiece = KnightValue;
		}
		else if (bishops & occupied)
		{
			int square = lsb(bishops & occupied);
			occupied &= ~::setBit(square);
			occupied |= addXrayPiece(square, target);
			swapList[n] = -swapList[n - 1] + attackedPiece;
			attackedPiece = BishopValue;
		}
		else if (rooks & occupied)
		{
			int square = lsb(rooks & occupied);
			occupied &= ~::setBit(square);
			occupied |= addXrayPiece(square, target);
			swapList[n] = -swapList[n - 1] + attackedPiece;
			attackedPiece = RookValue;
		}
		else if (queens & occupied)
		{
			int square = lsb(queens & occupied);
			occupied &= ~::setBit(square);
			occupied |= addXrayPiece(square, target);
			swapList[n] = -swapList[n - 1] + attackedPiece;
			attackedPiece = QueenValue;
		}
		else if (kings & occupied)
		{
			occupied &= ~::setBit(lsb(kings & occupied));
			swapList[n] = -swapList[n - 1] + attackedPiece;
			attackedPiece = KingValue;
		}
		else
		{
			break;
		}

		mstl::swap(occupied, occupied2);
		mstl::swap(pawns, pawns2);
	}

	// Starting at the end of the sequence of values, use a
	// "minimax" like procedure to decide where the captures
	// will stop.
	while (--n)
	{
		if (swapList[n] > -swapList[n - 1])
			swapList[n - 1] = -swapList[n];
	}

	TRACE(::printf("(%d): SEE(%s) = %d\n", m_ply, move.asString().c_str(), swapList[0]));
	return swapList[0];
}


// ------------------------------------------------------------------------
// adopted from crafty-15.17/swap.c:SwapXray()
// ------------------------------------------------------------------------
// addXrayPiece() is used to determine if a piece is "behind" the piece on
// <from>, and this piece would attack <target> if the piece on <from> were
// moved (as in playing out sequences of swaps). If so, this indirect
// attacker is added to the list of attackers bearing to <target>.
uint64_t
db::Guess::addXrayPiece(unsigned from, unsigned target)
{
	switch (::Directions[target][from])
	{
		case  1: return rankAttacks(from) & (m_rooks | m_queens) & Plus1Dir[from];
		case  7: return diagA1H8Attacks(from) & (m_bishops | m_queens) & Plus7Dir[from];
		case  8: return fyleAttacks(from) & (m_rooks | m_queens) & Plus8Dir[from];
		case  9: return diagH1A8Attacks(from) & (m_bishops | m_queens) & Plus9Dir[from];
		case -1: return rankAttacks(from) & (m_queens | m_rooks) & Minus1Dir[from];
		case -7: return diagA1H8Attacks(from) & (m_bishops | m_queens) & Minus7Dir[from];
		case -8: return fyleAttacks(from) & (m_rooks | m_queens) & Minus8Dir[from];
		case -9: return diagH1A8Attacks(from) & (m_bishops | m_queens) & Minus9Dir[from];
	}

	return 0;
}


#if 0
db::Guess::Recognized
db::Guess::recognize()
{
	static MatSig const K		= { .queen = 0, .rook = 0, .bishop = 0, .knight = 0, .pawn = 0 };
	static MatSig const KQ		= { .queen = 1, .rook = 0, .bishop = 0, .knight = 0, .pawn = 0 };
	static MatSig const KR		= { .queen = 0, .rook = 1, .bishop = 0, .knight = 0, .pawn = 0 };
	static MatSig const KB		= { .queen = 0, .rook = 0, .bishop = 1, .knight = 0, .pawn = 0 };
	static MatSig const KN		= { .queen = 0, .rook = 0, .bishop = 0, .knight = 1, .pawn = 0 };
	static MatSig const KP		= { .queen = 0, .rook = 0, .bishop = 0, .knight = 0, .pawn = 1 };
	static MatSig const KBP		= { .queen = 0, .rook = 0, .bishop = 1, .knight = 0, .pawn = 1 };
	static MatSig const KNN		= { .queen = 0, .rook = 0, .bishop = 0, .knight = 2, .pawn = 0 };
	static MatSig const KRP		= { .queen = 0, .rook = 1, .bishop = 0, .knight = 0, .pawn = 0 };
	static MatSig const KBPP	= { .queen = 0, .rook = 0, .bishop = 1, .knight = 0, .pawn = 2 };
	static MatSig const KBPPP	= { .queen = 0, .rook = 0, .bishop = 1, .knight = 0, .pawn = 3 };

	union Sig { matsig::MatSig m; uint32_t i; };

#define SIGNATURE(w,b) ((uint64_t(Sig(w).i) << 32) | uint64_t(Sig(b).i))

	switch (SIGNATURE(m_matCount[White], m_matCount[Black]))
	{
		// ###########################################################
		// 2 pieces
		// ###########################################################
		case SIGNATURE(K,		K		):	return RecogDraw;

		// ###########################################################
		// 3 pieces
		// ###########################################################
		case SIGNATURE(KB,	K		):	return RecogDraw;
		case SIGNATURE(KN,	K		):	return RecogDraw;
		// white -----------------------------------------------------
		case SIGNATURE(KP,	K		):	return recogKPK(White);
		// black -----------------------------------------------------
		case SIGNATURE(K,		KP		):	return recogKPK(Black);

		// ###########################################################
		// 4 pieces
		// ###########################################################
		case SIGNATURE(KB,	KB		):	return recogMateInCorner();
		case SIGNATURE(KN,	KN		):	return recogMateInCorner();
		// white -----------------------------------------------------
		case SIGNATURE(KQ,	KP		):	return recogKQKP(White);
		case SIGNATURE(KR,	KP		):	return recogKRKP(White);
		case SIGNATURE(KB,	KP		):	return recogKMKP(White);
		case SIGNATURE(KN,	KP		):	return recogKMKP(White);
		case SIGNATURE(KR,	KB		):	return recogKRKB(White);
		case SIGNATURE(KR,	KN		):	return recogKRKN(White);
		case SIGNATURE(KBP,	K		):	return recogKBPK(White);
		case SIGNATURE(KNN,	K		):	return recogMateInCorner();
		case SIGNATURE(KN,	KB		):	return recogMateInCorner();
		// black -----------------------------------------------------
		case SIGNATURE(KP,	KQ		):	return recogKQKP(Black);
		case SIGNATURE(KP,	KR		):	return recogKRKP(Black);
		case SIGNATURE(KP,	KB		):	return recogKMKP(Black);
		case SIGNATURE(KP,	KN		):	return recogKMKP(Black);
		case SIGNATURE(KB,	KR		):	return recogKRKB(Black);
		case SIGNATURE(KN,	KR		):	return recogKRKN(Black);
		case SIGNATURE(K,		KBP	):	return recogKBPK(Black);
		case SIGNATURE(K,		KNN	):	return recogMateInCorner();
		case SIGNATURE(KB,	KN		):	return recogMateInCorner();

		// ###########################################################
		// 5 pieces
		// ###########################################################
		// white -----------------------------------------------------
		case SIGNATURE(KRP,	KR		):	return recogKRPKR(White);
		case SIGNATURE(KBPP,	K		):	return recogKBPK(White);
		// black -----------------------------------------------------
		case SIGNATURE(KR,	KRP	):	return recogKRPKR(Black);
		case SIGNATURE(K,		KBPP	):	return recogKBPK(Black);

		// ###########################################################
		// 6 pieces
		// ###########################################################
		// white -----------------------------------------------------
		case SIGNATURE(KBPPP,K		):	return recogKBPK(White);
		// black -----------------------------------------------------
		case SIGNATURE(K,		KBPPP	):	return recogKBPK(Black);
	}

#undef SIGNATURE

	return RecogUnknown;
}


// KBKB, KBKN, KNKN and KNNK are all draws, but may have a
// trivial mate-in-one if the non-side-to-move king is in a
// corner
Recognized
db::Guess::recogMateInCorner()
{
	return king(opposite(sideToMove())) & (A1 | H1 | A8 | H8) ? RecogUnknown : RecogDraw;
}


Recognized
db::Guess:recogKPK(color::ID side)
{
	//M_ASSERT(count(pawns(side)) == 1);
	//M_ASSERT(count(m_occupied) == 3);

	int wp	= lsb(pawns(side));
	int wk	= m_ksq[side];
	int bk	= m_ksq[opposite(side)];
	int stm	= isWhite(side) ? sideToMove() : opposite(sideToMove());

	// If the enemy king can capture the pawn: draw.
	if (isBlack(stm) && sq::isAdjacent(bk, wp) && !sq::isAdjacent(wk, wp))
		return recogDraw;

	if (isBlack(side))
	{
		wp = sq::flipRank(wp);
		wk = sq::flipRank(wk);
		bk = sq::flipRank(bk);
	}

	int wpRank = ::rank(wp);
	int bkRank = ::rank(bk);

	// If the enemy king is behind or equal to the pawn rank: win or unknown.
	if (bkRank <= wpRank)
	{
		// Runaway pawn wins:
		if (wpRank > bkRank + (stm == Black))
			return RecogWin;

		// Cannot easily determine the result of this position:
		return RecogUnknown;
	}

	// Black king is clearly the closest king to the pawn: draw.
	// TODO: check if the "+ 2" below can safely be "+ 1".
	if (sq::distance(bk, wp) + (stm == White) + 2 < sq::distance(wk, wp))
		return RecogDraw;

	int wpFyle = ::fyle(pawn);
	int bkFyle = ::fyle(bk);

	// Black king in front of a rook pawn: safe draw.
	if ((wpFyle == FyleA || wpFyle == FyleH) && wpFyle == bkFyle)
		return RecogDraw;

	// King the two squares in front of any pawn before the 6th rank: draw.
	if (	wpRank < Rank6
		&& wpFyle == bkFyle
		&& (wpRank + 1 == bkRank || wpRank + 2 == bkRank))
	{
		return RecogDraw;
	}

	// Pawn on 6th rank, enemy king blocking it on 7th: draw.
	if (wpRank == Rank6 && wpFyle == bkFyle && wpRank + 1 == bkRank)
		return RecogDraw;

	int wkRank = ::rank(wk);
	int wkFyle = ::fyle(wk);

	// White king two ranks in front of the pawn, on the same file or an
	// adjacent file: win.
	if (wpRank + 2 == wkRank && mstl::abs(wpFyle - wkFyle) <= 1)
		return RecogWin;

	// Pawn-King-space-EnemyKing formation, draw if pawn is before 5th rank
	// and side with the pawn is to move; otherwise a win.
	if (	wpFyle == wkFyle
		&& wpFyle == bkFyle
		&& wpRank + 1 == wkRank
		&& wpRank + 3 == bkRank)
	{
		return wpRank < Rank5 && isWhite(stm) ? RecogDraw : RecogWin;
	}

	// No key KPK position was found:
	return RecogUnknown;
}


Recognized
db::Guess:recogKBPK(color::ID side)
{
	static uint64_t const NotRookPawns	= FyleMaskB | FyleMaskC | FyleMaskD
													| FyleMaskE | FyleMaskF | FyleMaskG;

	//M_ASSERT(count(pawns(side)));
	//M_ASSERT(count(bishops(side)));
	//M_ASSERT(count(knights(side) | rooks(side) | queens(side)) == 0);
	//M_ASSERT(count(m_occupied_co[opposite(side)]) == 1);

	uint64_t pawns = this->pawns(side);

	// Check if all pwans are on the same rook fyle.
	if (!(pawns & ::NotRookPawns) && !((pawns & FyleMaskA) && (pawns & FyleMaskH)))
	{
		int wp		= isWhite(side) ? msb(pawns) : lsb(pawns);
		int promoSq	= sq::make(::fyle(wp), HomeRank[opposite(side)]);

		// Recognise a draw if the black king controls the
		// queening square and bishop has the wrong color.
		if (	(isWhite(sq::color(promoSq))) == !hasBishopOnLite(side)
			&& sq::isAdjacent(m_ksq[opposite(side)], promoSq))
		{
			return RecogDraw;
		}
	}

	return RecogUnknown;
}


Recognized
db::Guess::recogKQKP(color::ID side)
{
	//M_ASSERT(count(m_occupied) == 4);
	//M_ASSERT(count(queens(side)) == 1);
	//M_ASSERT(count(pawns(opposite(side))) == 1);

	int wk = m_ksq[side];
	int bk = m_ksq[opposite(side)];
	int wq = lsb(queens(side));
	int bp = lsb(king(opposite(side)));

	if (isBlack(side))
	{
		wk = sq::flipRank(wk);
		bk = sq::flipRank(bk);
		wq = sq::flipRank(wq);
		bp = sq::flipRank(bp);
	}

	// There are only recognizable draws with a pawn on its 2nd rank,
	// defended by its king.
	if (::rank(bp) != Rank2 || ! sq::isAdjacent(bk, bp))
		return RecogUnknown;

	// Make sure the pawn is on the queenside,
	if (::fyle(bp > FyleD))
	{
		wk = sq::flipFyle(wk);
		bk = sq::flipFyle(bk);
		wq = sq::flipFyle(wq);
		bp = sq::flipFyle(bp);
	}

	switch (bp)
	{
		case a2:
			{
				int distance = 4;

				switch (bk)
				{
					case b1:
					case b2:	// best case
						if (sideToMove != side)
							--distance;
						break;

					case a1:
						if (sideToMove() == side && ::fyle(wq) != FyleB)	// black loses a tempo
							return RecogWin;
						break;

					case c1:
					case c2:
						if (wq != a1 && sideToMove() == side && ::fyle(wq) != FyleB)	// black loses a tempo
							return RecogWin;
						break;

					default:
						return RecogWin;
				}

				if (sq::distance(wk, a1) > distance)
					return RecogDraw;
			}
			break;

		case c2:
			{
				static uint64_t WhiteRightSide const = D3 | E4 | F5 | G6 | H7;
				static uint64_t BlackRightSide const = D5 | E5 | F4 | G3 | H2;

				int distance = 4;

				switch (bk)
				{
					case c1:							// distance = 0
						if (sideToMove() == side)	// no right-to-move bonus
							++distance;				// self-blocking penalty
						break;

					case b1:
						--distance;					// right-side bonus
						if (queens(side) & (isWhite(side) ? WhiteRightSide : BlackRightSide))
							++distance;				// pinned-pawn penalty
						if (sideToMove() != side)
							--distance;				// right-to-move bonus
						break;

					case b2:							// distance = 1, right side
						--distance;					// right-side bonus
						if (queens(side) & PawnRankMask[side])
							++distance;				// pinned-pawn penalty
						if (sideToMove() != side)
							--distance;				// right-to-move bonus
						break;

					case d1:
						if (wq == b3 || wq == a4)
							++distance;				// pinned-pawn penalty
						if (sideToMove() != side)
							--distance;				// right-to-move bonus
						break;

					case d2:
						if (wq == a2 || wq == b2)
							++distance;				// pinned-pawn penalty
						if (sideToMove() != side)
							--distance;				// right-to-move bonus
						break;

					case a2:
					case a1:
						if (bq != c1)				// distance = 2, right side
						{
							if (sideToMove() == side && ::fyle(wq) != FyleB)
								return RecogWin;
							--distance;				// right-side bonus
						}
						break;

					case e1:
					case e2:
						if (wq != c1)				// distance = 2, wrong side
						{
							if (sideToMove() == side && ::fyle(wq) != FyleD)
								return RecogWin;
						}
						break;

					default:
						return RecogWin;
				}

				if (sq::distance(wk, c1) > distance)
					return RecogDraw;
			}
			break;
	}

	return RecogWin;
}
#endif

// vi:set ts=3 sw=3:
