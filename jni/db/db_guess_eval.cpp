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
#include "db_board_base.h"

#include "m_assert.h"
#include "m_stdio.h"

#include <string.h>

using namespace db;
using namespace db::sq;
using namespace db::color;
using namespace db::castling;
using namespace db::board;

//#define TRACE(fmt,args...) 	printf(fmt,##args)
//#define TRACE_2(fmt,args...)	printf(fmt,##args)

#ifndef TRACE
# define TRACE(fmt, args...)
#endif
#ifndef TRACE_2
# define TRACE_2(fmt, args...)
#endif
#define TRACE_1(fmt,args...)	TRACE(fmt,##args)

static uint64_t const NotRookPawns	= FyleMaskB | FyleMaskC | FyleMaskD
												| FyleMaskE | FyleMaskF | FyleMaskG;

enum { LazyEvalCutoff = 0 };	// use 125 if wanted
enum { DrawScore = 0 };


namespace { typedef db::Guess::Score Score; }

namespace bonus
{
	static int const BishopPair			= 13;
	static int const RookHalfOpenFyle	= 10;
	static int const KingKingTropism		= 10;

	static Score const WhiteToMove				( 5,  8);
	static Score const BishopWithWingPawns		(18, 36);
	static Score const RookOpenFyle				(40, 20);
	static Score const RookOn7th					(20, 40);
	static Score const RookBehindPassedPawn	(10, 36);

	static int const BishopKingTropism[8]	= { 0, 2, 2, 1, 0, 0, 0, 0 };
	static int const KnightKingTropism[8]	= { 0, 3, 3, 2, 1, 0, 0, 0 };
	static int const RookKingTropism[8]		= { 0, 4, 3, 2, 1, 1, 1, 1 };
	static int const QueenKingTropism[8]	= { 0, 6, 5, 4, 3, 2, 2, 2 };

	static int8_t const KingSquareN[64] =
	{
		-40, -40, -40, -40, -40, -40, -40, -40,
		-40, -10, -10, -10, -10, -10, -10, -40,
		-40, -10,  20,  20,  20,  20, -10, -40,
		-40, -10,  40,  40,  40,  40, -10, -40,
		-40, -10,  60,  60,  60,  60, -10, -40,
		-40, -10,  60,  60,  60,  60, -10, -40,
		-40, -10, -10, -10, -10, -10, -10, -40,
		-40, -40, -40, -40, -40, -40, -40, -40,
	};

	static int8_t const KingSquareK[64] =
	{
		-60, -40, -20, -20, -20, -20, -20, -20,
		-60, -40, -20,   0,   0,   0,   0,   0,
		-60, -40, -20,  20,  20,  20,  20,  20,
		-60, -40, -20,  20,  40,  40,  40,  40,
		-60, -40, -20,  20,  60,  60,  60,  40,
		-60, -40, -20,  20,  60,  60,  60,  40,
		-60, -40, -20,  20,  40,  40,  40,  40,
		-60, -40, -20, -20, -20, -20, -20, -20,
	};

	static int8_t const KingSquareQ[64] =
	{
		-20, -20, -20, -20, -20, -20, -40, -60,
		  0,   0,   0,   0,   0, -20, -40, -60,
		 20,  20,  20,  20,  20, -20, -40, -60,
		 40,  40,  40,  40,  20, -20, -40, -60,
		 40,  60,  60,  60,  20, -20, -40, -60,
		 40,  60,  60,  60,  20, -20, -40, -60,
		 40,  40,  40,  40,  20, -20, -40, -60,
		-20, -20, -20, -20, -20, -20, -40, -60,
	};

	static int8_t const KnightSquare[64] =
	{
		-20, -20, -20, -20, -20, -20, -20, -20,
		  0,   0,   0,   0,   0,   0,   0,   0,
		  0,   0,  16,  14,  14,  16,   0,   0,
		  0,  10,  18,  20,  20,  18,  10,   0,
		  0,  12,  20,  24,  24,  20,  12,   0,
		  0,  12,  20,  24,  24,  20,  12,   0,
		  0,  10,  16,  20,  20,  16,  10,   0,
		-30, -20, -20, -10, -10, -20, -20, -30,
	} ;

	static int8_t const BishopSquare[64] =
	{
		-10, -10,  -8,  -6,  -6,  -8, -10, -10,
		  0,   8,   6,   8,   8,   6,   8,   0,
		  2,   6,  12,  10,  10,  12,   6,   2,
		  4,   8,  10,  16,  16,  10,   8,   4,
		  4,   8,  10,  16,  16,  10,   8,   4,
		  2,   6,  12,  10,  10,  12,   6,   2,
		  0,   8,   6,   8,   8,   6,   8,   0,
		  0,   0,   2,   4,   4,   2,   0,   0,
	};

	static int8_t const RookSquare[64] =
	{
		 0,  2,  3,  4,  4,  3,  2,  0,
		-4,  2,  3,  4,  4,  3,  2, -4,
		-4,  2,  3,  4,  4,  3,  2, -4,
		-4,  2,  3,  4,  4,  3,  2, -4,
		 0,  2,  3,  4,  4,  3,  2,  0,
		 0,  2,  3,  4,  4,  3,  2,  0,
		 0,  2,  3,  4,  4,  3,  2,  0,
		 0,  2,  3,  4,  4,  3,  2,  0,
	};

	static int8_t const QueenSquare[64] =
	{
		0,  0,  0,  0,  0,  0,  0,  0,
		0,  0,  4,  4,  4,  4,  0,  0,
		0,  4,  4,  6,  6,  4,  4,  0,
		0,  4,  6,  8,  8,  6,  4,  0,
		0,  4,  6,  8,  8,  6,  4,  0,
		0,  4,  4,  6,  6,  4,  4,  0,
		0,  0,  4,  4,  4,  4,  0,  0,
		0,  0,  0,  0,  0,  0,  0,  0,
	};

	static int8_t const PawnSquare[64] =
	{
		 0,   0,   0,   0,   0,   0,   0,   0,
		 0,   0,   0, -12, -12,   0,   0,   0,
		 1,   1,   1,  10,  10,   1,   1,   1,
		 3,   3,   3,  13,  13,   3,   3,   3,
		 6,   6,   6,  16,  16,   6,   6,   6,
		10,  10,  10,  30,  30,  10,  10,  10,
		70,  70,  70,  70,  70,  70,  70,  70,
		 0,   0,   0,   0,   0,   0,   0,   0,
	};

	static int8_t const PawnStorm[64] =
	{
		 0,   0,   0,   0,   0,   0,   0,   0,
		12,  12,  12,   0,   0, -12, -12, -12,
		 8,   8,   8,   0,   0,  -8,  -8,  -8,
		 4,   4,   4,   4,   4,  -4,  -4,  -4,
		 0,   0,   0,   4,   6,   0,   0,   0,
		 0,   0,   0,   0,   4,   2,   2,   2,
		 0,   0,   0,   0,   2,   2,   2,   2,
		 0,   0,   0,   0,   0,   0,   0,   0,
	};

	static int8_t const KnightOutpostSquare[64] =
	{
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 1, 4, 4, 4, 4, 1, 0,
		0, 2, 6, 8, 8, 6, 2, 0,
		0, 1, 4, 4, 4, 4, 1, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
	};

	static int8_t const BishopOutpostSquare[64] =
	{
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 1, 2, 2, 2, 2, 1, 0,
		0, 3, 5, 5, 5, 5, 3, 0,
		0, 1, 3, 3, 3, 3, 1, 0,
		0, 0, 1, 1, 1, 1, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
	};

}; // namespace bonus

namespace mate
{
	// dark squares
	static uint8_t const BishopKnightSquare[64] =
	{
		99, 90, 80, 70, 60, 50, 40, 30,
		90, 80, 70, 60, 50, 40, 30, 40,
		80, 70, 60, 50, 40, 30, 40, 50,
		70, 60, 50, 40, 30, 40, 50, 60,
		60, 50, 40, 30, 40, 50, 60, 70,
		50, 40, 30, 40, 50, 60, 70, 80,
		40, 30, 40, 50, 60, 70, 80, 90,
		30, 40, 50, 60, 70, 80, 90, 99,
	};

	static uint8_t const Square[64] =
	{
		200, 180, 160, 140, 140, 160, 180, 200,
		180, 160, 140, 120, 120, 140, 160, 180,
		160, 140, 120, 100, 100, 120, 140, 160,
		140, 120, 100, 100, 100, 100, 120, 140,
		140, 120, 100, 100, 100, 100, 120, 140,
		160, 140, 120, 100, 100, 120, 140, 160,
		180, 160, 140, 120, 120, 140, 160, 180,
		200, 180, 160, 140, 140, 160, 180, 200,
	};

} // namespace mate

namespace penalty
{
	static int const BishopTrapped			= 174;
	static int const RookTrapped				=  60;
	static int const KingSafetyMateThreat	= 600;
	static int const LowerBishop				=  10;
	static int const LowerKnight				=  16;
	static int const LowerRook					=  16;

	static int const FriendlyQueen[8] = { 2, 2, 2, 1, 0, 0, -1, -1 };

	static int const Imbalance[9][9] =
	{
		{ -105, -105, -105, -105, -105, -105, -105,  -70, -35 },
		{ -105, -105, -105, -105, -105, -105,  -70,  -35,  35 },
		{ -105, -105, -105, -105, -105,  -70,  -35,   35,  70 },
		{ -105, -105, -105, -105,  -87,  -35,   35,   70, 105 },
		{ -105, -105, -105,  -74,    0,   74,  105,  105, 105 },
		{ -105,  -70,  -35,   35,   87,  105,  105,  105, 105 },
		{  -70,  -35,   35,   70,  105,  105,  105,  105, 105 },
		{  -35,   35,   70,  105,  105,  105,  105,  105, 105 },
		{   35,   70,  105,  105,  105,  105,  105,  105, 105 },
	};

}; // namespace penalty

namespace safety
{
	static int const Safety[16] =
	{
		0, 7, 14, 21, 28, 35, 42, 49, 56, 63, 70, 77, 84, 91, 98, 105,
	};

	static int const Tropism[16] =
	{
		0, 1, 2, 3, 4, 5, 11, 20, 32, 47, 65, 86, 110, 137, 167, 200,
	};

	static int const KingSafety[16][16] =
	{
		{   0,   1,   3,   5,   7,   9,  19,  36,  57,  84, 117, 154, 198, 246, 300, 360 },
		{  12,  14,  16,  18,  19,  21,  32,  50,  73, 102, 136, 178, 223, 275, 333, 397 },
		{  25,  27,  28,  30,  32,  34,  46,  64,  90, 120, 158, 201, 250, 306, 367, 435 },
		{  37,  39,  41,  43,  45,  48,  61,  81, 106, 138, 178, 225, 277, 334, 401, 473 },
		{  50,  52,  54,  55,  59,  61,  75,  95, 122, 158, 199, 248, 302, 365, 433, 511 },
		{  63,  64,  66,  70,  72,  73,  88, 111, 140, 176, 219, 271, 329, 394, 468, 549 },
		{  75,  77,  79,  82,  84,  88, 102, 126, 156, 194, 241, 295, 356, 424, 502, 586 },
		{  88,  90,  91,  95,  97, 100, 117, 140, 172, 214, 261, 318, 381, 455, 534, 624 },
		{ 100, 102, 106, 108, 111, 113, 131, 156, 189, 232, 282, 342, 408, 484, 568, 662 },
		{ 113, 115, 118, 120, 124, 127, 144, 171, 207, 250, 302, 365, 435, 514, 603, 700 },
		{ 126, 127, 131, 135, 136, 140, 158, 187, 223, 268, 324, 388, 462, 543, 635, 738 },
		{ 138, 140, 144, 147, 151, 153, 172, 201, 239, 288, 345, 412, 487, 574, 669, 775 },
		{ 151, 153, 156, 160, 163, 167, 187, 216, 255, 306, 365, 435, 514, 604, 703, 813 },
		{ 163, 165, 169, 172, 176, 180, 201, 232, 273, 324, 387, 459, 541, 633, 736, 851 },
		{ 176, 178, 181, 185, 189, 192, 214, 246, 289, 343, 406, 482, 567, 664, 770, 889 },
		{ 189, 192, 196, 199, 203, 207, 228, 262, 306, 361, 428, 505, 594, 693, 804, 927 },
	};

} // namespace safety

namespace pawn
{
	static int const CanPromote = 525;

	static Score const Duo						( 4,  8);
	static Score const Isolated				(12, 18);
	static Score const Weak						(16, 24);
	static Score const Doubled					( 5,  6);
	static Score const PassedPawnHidden		( 0, 40);
	static Score const PassedPawnConnected	( 1,  3);
	static Score const OutsidePassed			(20, 60);

	static Score const PassedPawnCandidate[8] =
	{
		Score( 0,  0),
		Score( 4,  9),
		Score( 4,  9),
		Score( 8, 16),
		Score(16, 32),
		Score(36, 28),
		Score( 0,  0),
		Score( 0,  0),
	};

	static Score const BlockadingPassedPawnValue[8] =
	{
		Score(  0,   0),
		Score(  0,   0),
		Score(  0,   0),
		Score( 10,  10),
		Score( 40,  40),
		Score( 60,  60),
		Score(100, 100),
	};

	static uint8_t const PassedPawnValue[8] =
	{
		0, 0, 0, 20, 80, 120, 200, 0,
	};

	static uint8_t const OpenFyle[8] =
	{
		6, 5, 4, 4, 4, 4, 5, 6,
	};

	static uint8_t const HalfOpenFyle[8] =
	{
		4, 4, 3, 3, 3, 3, 4, 4,
	};

	static uint8_t const Defects[8] =
	{
		0, 0, 3, 2, 1, 0, 0, 0,
	};

} // namespace pawn

namespace development
{
	static int const Thematic				= 12;
	static int const LosingCastle			= 20;
	static int const NotCastled			= 20;
	static int const UndevelopedPiece	= 12;

} // namespace development


inline static int mul2(int x)		{ return x << 1; }
inline static int mul3(int x)		{ return x*3; }
inline static int mul4(int x)		{ return x << 2; }
inline static int div2(int x)		{ return x >> 1; }

static sq::ID identicalSquare(sq::ID s) { return s; }


static sq::ID (*wflip)(sq::ID) = identicalSquare;
static sq::ID (*bflip)(sq::ID) = sq::flipRank;


inline
bool
isInFrontOf(color::ID color, int kingSquare, int pawnSquare)
{
	return isWhite(color) ? rank(kingSquare) > rank(pawnSquare) : rank(kingSquare) < rank(pawnSquare);
}


inline
bool
isBehind(color::ID color, int kingSquare, int pawnSquare)
{
	return isWhite(color) ? rank(kingSquare) < rank(pawnSquare) : rank(kingSquare) > rank(pawnSquare);
}


// hasOpposition() is used to determine if one king stands in "opposition"
// to the other. If the kings are opposed on the same fyle or else are
// opposed on the same diagonal, then the side not-to-move has the opposition
// and the side-to-move must give way.
static bool
hasOpposition(bool onMove, sq::ID king, sq::ID enemyKing)
{
	int rankDistance = sq::rankDistance(king, enemyKing);

	if (rankDistance < 2)
		return true;

	int fyleDistance = sq::fyleDistance(king, enemyKing);

	if (onMove)
	{
		if (rankDistance & 8)
			return (rankDistance & 1) && (fyleDistance & 1);

		if (fyleDistance & 1)
			return rankDistance & 1;
	}

	return !(fyleDistance & 1) && !(rankDistance & 1);
}


// ------------------------------------------------------------------------
// adopted from crafty-22.8/preeval.c
// ------------------------------------------------------------------------
// preEvaluate() is used to recognize positions where the "Trojan horse -
// attack" is threatened, and it enables the specific code to handle that
// case.
void
db::Guess::preEvaluate()
{
	::memset(m_pawnData = m_pawnTable, 0, sizeof(m_pawnTable));

	m_root.m_castle	= m_castle;
	m_root.m_stm		= m_stm;

	// Now check to see if the "trojan check" code should be
	// turned on. Basically if the king is in the corner,
	// the opponent has placed a piece on g4/g5, and both
	// sides have pawns attacking that piece, and queens are
	// still on the board, then it is a threat that must be
	// handled.
	//
	// This is handled as 4 separate cases for each corner of
	// the board, for simplicity.
	m_trojanCheck = false;

	if (m_matCount[Black].queen && m_matCount[Black].rook)
	{
		uint64_t king = m_kings & m_occupiedBy[White];

		if (king & (G1 | H1))
		{
			if (((knights(Black) | bishops(Black)) & G4) && (pawns(White) & (H3 | H5)) == (H3 | H5))
				m_trojanCheck = true;
		}
		else if (king & (B1 | A1))
		{
			if (((knights(Black) | bishops(Black)) & B4) && (pawns(White) & (A3 | A5)) == (A3 | A5))
				m_trojanCheck = true;
		}
	}

	if (m_matCount[White].queen && m_matCount[White].rook)
	{
		uint64_t king = m_kings & m_occupiedBy[Black];

		if (king & (G8 | H8))
		{
			if (((knights(White) | bishops(White)) & G5) && (pawns(Black) & (H4 | H6)) == (H4 | H6))
				m_trojanCheck = true;
		}
		else if (king & (B8 | A8))
		{
			if (((knights(White) | bishops(White)) & B5) && (pawns(Black) & (A4 | A6)) == (A4 | A6))
				m_trojanCheck = true;
		}
	}
}


bool
db::Guess::doScorePieces(Score score, int alpha, int beta) const
{
	if (::LazyEvalCutoff)
	{
		int pscore	= score.weightedScore(m_totalPieces[White], m_totalPieces[Black]);
		int lscore	= whiteToMove() ? pscore : -pscore;
		int total	= m_matCount[White].bishop + m_matCount[White].rook
						+ m_matCount[Black].bishop + m_matCount[Black].rook
						+ m_matCount[White].knight + ::mul2(m_matCount[White].queen)
						+ m_matCount[Black].knight + ::mul2(m_matCount[Black].queen);
		int cutoff	= ::LazyEvalCutoff + ::mul4(total);

		if ((lscore - cutoff) >= beta || alpha >= (lscore + cutoff))
			return false;
	}

	return true;
}


// ------------------------------------------------------------------------
// adopted from crafty-22.8/evaluate.c:Evaluate()
// ------------------------------------------------------------------------
// evaluate() is used to evaluate the chess board. Broadly, it addresses
// four distinct areas:
//
// (1) Material score which is simply a summing of piece types multiplied
//     by piece values.
// (2) Pawn scoring which considers placement of pawns and also evaluates
//     passed pawns, particularly in endgame situations.
// (3) Piece scoring which evaluates the placement of each piece as well
//     as things like piece mobility.
// (4) King safety which considers the pawn shelter around the king along
//     with material present to facilitate an attack.
//
// Alpha and beta cutoff scores are specified for performance. If simple
// material counting produces a score much lower than alpha or much greater
// than beta, the score is returned without slower square-based evaluation.
int
db::Guess::evaluate(int alpha, int beta)
{
	// Initialize.
	m_totalPieces[White]	= total(m_matCount[White]);
	m_totalPieces[Black]	= total(m_matCount[Black]);
	m_dangerous[White]	= 	(m_matCount[White].queen && m_totalPieces[White] > 13)
								|| (m_matCount[White].rook > 1 && m_totalPieces[White] > 15);
	m_dangerous[Black]	= 	(m_matCount[Black].queen && m_totalPieces[Black] > 13)
								|| (m_matCount[Black].rook > 1 && m_totalPieces[Black] > 15);
	m_majors					= major(m_matCount[White]) - major(m_matCount[Black]);
	m_minors					= minor(m_matCount[White]) - minor(m_matCount[Black]);
	m_tropism[White]		= 0;
	m_tropism[Black]		= 0;

	Score score = evaluateMaterial();

	// Check for draws due to insufficient material and adjust the
	// score as necessary. This code also handles a special endgame
	// case where one side has only a lone king, and the king has no
	// legal moves. This has been shown to break a few evaluation
	// terms such as bishop + wrong color rook pawn. If this case is
	// detected, a drawscore is returned.
	WinningChances canWin = evaluateWinningChances();

	// Determine if this is position should be evaluated to force mate
	// (neither side has pawns) or if it should be evaluated normally.
	//
	// Note the special case of no pawns, one side is ahead in total
	// material, but the game is a hopeless draw. KRN vs KR is one
	// example. If evaluateWinningChances() determines that the side
	// with extra material can not win, the score is pulled closer to a
	// draw although it can not collapse completely to the drawscore as
	// it is possible to lose KRB vs KR if the KR side lets the king
	// get trapped on the edge of the board.
	if ((m_matCount[White].pawn | m_matCount[Black].pawn) == 0)
	{
		if (m_totalPieces[White] > piece::value::Minor || m_totalPieces[Black] > piece::value::Minor)
		{
			int material = m_totalPieces[White] - m_totalPieces[Black];

			score += evaluateMate(material > 0 ? White : Black);

			if (	(score.endGame >  ::DrawScore && !(canWin & OnlyWhiteCanWin))
				||	(score.endGame < -::DrawScore && !(canWin & OnlyBlackCanWin)))
			{
				score.endGame /= 4;
			}

			TRACE_2("score[no pawns]                      = %d\n", score.endGame);

			return score.endGame;
		}
	}
	else
	{
		if (m_pawnData->key != m_pawnHash)
		{
			PawnHashEntry* entry = m_pawnTable + (m_pawnHash & (PawnTableSize - 1));

			if (entry->key != m_pawnHash)
			{
				// initialize pawn score structure
				::memset(entry->eval, 0, sizeof(entry->eval));
				entry->eval[White].openFyle = 0xff;	// assume all fyles are open
				entry->eval[Black].openFyle = 0xff;	// dito
				entry->key = m_pawnHash;
				entry->score = 0;

				// evaluatePawns() does all of the analysis for information
				// specifically regarding only pawns. In many cases, it merely
				// records the presence/absence of positional pawn feature
				// because that feature also depends on pieces. Note that
				// anything put into EvaluatePawns() can only consider the
				// placement of pawns.
				entry->score += evaluatePawns(White);
				entry->score -= evaluatePawns(Black);
			}

			m_pawnData = entry;
		}

		score += m_pawnData->score;

		TRACE_2("score[evaluate pawns]                = %d, %d\n", score.middleGame, score.endGame);
	}

	Eval const& eval = m_pawnData->eval;

	// If there are any passed pawns, first call evaluatePassedPawns()
	// to evaluate them. Then, if one side has a passed pawn and the
	// other side has no pieces, call evaluatePassedPawnRaces() to see
	// if the passed pawn can be stopped from promoting.
	if (eval[Black].passedPawn || eval[White].passedPawn)
	{
		if (eval[White].passedPawn)
			score += evaluatePassedPawns(White, ::wflip);

		if (eval[Black].passedPawn)
			score -= evaluatePassedPawns(Black, ::bflip);

		if (	(m_totalPieces[White] == 0 && eval[Black].passedPawn)
			|| (m_totalPieces[Black] == 0 && eval[White].passedPawn))
		{
			score += evaluatePassedPawnRaces();
		}

		TRACE_2("score[passed pawns]                  = %d, %d\n", score.middleGame, score.endGame);
	}

	// Add bonus (penalties) for pawn storms.
	if (fyle(m_ksq[White]) <= FyleC && fyle(m_ksq[Black] >= FyleF))
		score.middleGame += eval[White].longVsShortScore - eval[Black].shortVsLongScore;
	else if (fyle(m_ksq[Black]) <= FyleC && fyle(m_ksq[White] >= FyleF))
		score.middleGame += eval[White].shortVsLongScore - eval[Black].longVsShortScore;

	TRACE_2("score[pawn storm]                    = %d, %d\n", score.middleGame, score.endGame);
	TRACE_1("score[pawns]                         = %d, %d\n", score.middleGame, score.endGame);

	// Call EvaluateDevelopment() to evaluate development. Note that
	// we only do this when either side has not castled at the root.
	if (m_root.canCastle(White))
		score.middleGame += evaluateDevelopment(White);
	if (m_root.canCastle(Black))
		score.middleGame -= evaluateDevelopment(Black);

	TRACE("score[development]                   = %d, %d\n", score.middleGame, score.endGame);

	// Now evaluate pieces.
	if (doScorePieces(score, alpha, beta))
	{
		score += evaluateKnights(White, ::wflip);
		score -= evaluateKnights(Black, ::bflip);

		score += evaluateBishops(White, ::wflip);
		score -= evaluateBishops(Black, ::bflip);

		score += evaluateRooks(White, ::wflip);
		score -= evaluateRooks(Black, ::bflip);

		score += evaluateQueens(White, ::wflip);
		score -= evaluateQueens(Black, ::bflip);

		score += evaluateKings(White, ::wflip);
		score -= evaluateKings(Black, ::bflip);
	}

	TRACE("score[pieces]                        = %d, %d\n", score.middleGame, score.endGame);

	// Now adjust the score if the game is drawish but one side appears
	// to be significantly better according to the computed score.
	return evaluateDraws(canWin, score.weightedScore(m_totalPieces[White], m_totalPieces[Black]));
}


// ------------------------------------------------------------------------
// adopted from crafty-22.8/evaluate.c:EvaluateMaterial()
// ------------------------------------------------------------------------
// evaluateMaterial() is used to evaluate material on the board. It really
// accomplishes detecting cases where one side has made a 'bad trade' as the
// comments below show.
db::Guess::Score
db::Guess::evaluateMaterial()
{
	// We start with the raw Material balance for the current position.
	int material	= (int(m_matCount[White].queen ) - int(m_matCount[Black].queen ))*QueenValue
						+ (int(m_matCount[White].rook  ) - int(m_matCount[Black].rook  ))*RookValue
						+ (int(m_matCount[White].bishop) - int(m_matCount[Black].bishop))*BishopValue
						+ (int(m_matCount[White].knight) - int(m_matCount[Black].knight))*KnightValue
						+ (int(m_matCount[White].pawn  ) - int(m_matCount[Black].pawn  ))*PawnValue;

	Score score = material + (whiteToMove() ? bonus::WhiteToMove : -bonus::WhiteToMove);

	TRACE("score[material]                      = %d, %d\n", score.middleGame, score.endGame);

	score += evaluateMaterialDynamic(White);
	score -= evaluateMaterialDynamic(Black);

	TRACE("score[material dynamic]              = %d, %d\n", score.middleGame, score.endGame);

	// Test 1.  If majors or minors are not balanced, then if one side
	// is only an exchange up or down, we do not give any sort of bad
	// trade penalty/bonus.
	//
	// Test 2.  If majors or minors are not balanced, then if one side
	// has more piece material points than the other (using normal
	// piece values of 3, 3, 5, 9 for N, B, R and Q) then the side that
	// is behind in piece material gets a penalty.

	int majors = mstl::min(4, mstl::max(-4, m_majors));
	int minors = mstl::min(4, mstl::max(-4, m_minors));

	score += penalty::Imbalance[majors + 4][minors + 4];

	TRACE("score[imbalance]                     = %d, %d\n", score.middleGame, score.endGame);

	return score;
}


// ------------------------------------------------------------------------
// adopted from crafty-22.8/evaluate.c:EvaluateMaterialDynamic()
// ------------------------------------------------------------------------
// evaluateMaterialDynamic() adds bonuses/penalties for each piece based on
// the number of pawns on the board, and the number of pieces of each type.
// This comes from IM Larry Kaufman's ideas on how piece values change with
// relation to what other pieces are present to complement or overlap with
// the specific piece type.
int
db::Guess::evaluateMaterialDynamic(color::ID side)
{
	// We start by counting the number of pawns on the board and then
	// computing each type of piece bonus based on this number.

	int score	= 0;
	int np		= m_matCount[White].pawn + m_matCount[Black].pawn;

	if (m_matCount[side].knight)
	{
		score += ::mul2(np) - 16;

		if (m_matCount[side].knight > 1)
			score -= 11;
	}

	// Bonus for bishop pair, but only if they have opposite colors.
	if (hasBishopOnLite(side) && hasBishopOnDark(side))
		score += bonus::BishopPair;

	if (m_matCount[side].queen)
	{
		score +=	np
				+  m_matCount[White].knight + m_matCount[White].bishop
				+  m_matCount[Black].knight + m_matCount[Black].bishop;
	}

	return score + m_matCount[side].rook*(32 - ::mul2(np));
}


// ------------------------------------------------------------------------
// adopted from crafty-22.8/evaluate.c:Evaluate()
// ------------------------------------------------------------------------
// evaluateWinningChances() is used to determine if one side (or both) are
// in a position where winning is impossible.
db::Guess::WinningChances
db::Guess::evaluateWinningChances()
{
	if (m_totalPieces[White] >= 13 || m_totalPieces[Black] >= 13)
		return BothSidesCanWin;

	// If neither side has any pieces, and both sides have
	// non-rookpawns, then either side can win.
	if (	m_totalPieces[White] + m_totalPieces[Black] == 0
		&& (pawns(White) & ::NotRookPawns)
		&& (pawns(Black) & ::NotRookPawns))
	{
		return BothSidesCanWin;
	}

	WinningChances canWin = BothSidesCanWin;

	// If one side is an exchange up, but has no pawns, then
	// that side can not possibly win.
	if (mstl::abs(m_majors) == 1)
	{
		if (m_majors == -m_minors)
		{
			if (m_matCount[White].pawn == 0)
				canWin = WinningChances(canWin & OnlyBlackCanWin);
			if (m_matCount[Black].pawn == 0)
				canWin = WinningChances(canWin & OnlyWhiteCanWin);

			if (canWin == NeitherSideCanWin)
				return canWin;
		}
	}

	// Now check several special cases, such as bishop + the
	// wrong rook pawn and adjust canWin accordingly.
	if (!evaluateWinningChances(White))
		canWin = WinningChances(canWin & OnlyBlackCanWin);
	if (!evaluateWinningChances(Black))
		canWin = WinningChances(canWin & OnlyWhiteCanWin);

	return canWin;
}


// evaluateWinningChances() is used to determine if one side has reached a
// position which can not be won, period, even though side may be ahead in
// material in some way.
bool
db::Guess::evaluateWinningChances(color::ID side)
{
	static uint64_t const NotEdge = ~(RankMask1 | RankMask8 | FyleMaskA | FyleMaskH);

	color::ID opponent = opposite(side);

	// If one side is a piece up, but has no pawns, then that
	// side can not possibly win. We recognize an exception
	// where the weaker side's king is trapped on the edge
	// where it might be checkmated.
	if (	m_matCount[side].pawn == 0
		&& (	m_totalPieces[side] <= piece::value::Minor
			|| (	m_totalPieces[side] - m_totalPieces[opponent] <= piece::value::Minor
				&& (king(opponent) & NotEdge))))
	{
		TRACE_2("%s piece up, but has no pawns\n", printColor(side));
		return false;
	}

	// If "side" has a pawn, then either the pawn had better
	// not be a rook pawn, or else "side" had better have the
	// right color bishop or any other piece, otherwise it is
	// not winnable if the opposite king can get to the queening
	// square first.
	uint64_t pawns = this->pawns(side);

#if 0	// XXX something is wrong!!
	if (	m_matCount[side].pawn
		&& !(pawns & ::NotRookPawns)
		&& m_totalPieces[side] <= piece::value::Minor
		&& m_matCount[side].knight == 0
		&& m_matCount[side].bishop
				?		m_totalPieces[opponent]
					&& !(FyleMask[hasBishopOnDark(side) == isWhite(side) ? FyleH : FyleA] & pawns)
				: !(pawns & FyleMaskA) || !(pawns & FyleMaskH))
	{
		TRACE_2(	"%s %s\n",
					printColor(side),
					m_matCount[side].bishop ? "has wrong color bishop" : "has rook pawns");

		sq::ID	promote	= sq::make(pawns & FyleMaskA ? FyleA : FyleH, HomeRank[opponent]);
		int		ekd		= sq::distance(kingSq(opponent), promote) - (sideToMove() != side);

		if (ekd <= 1)
			return false;

		uint64_t	fpawns	= pawns & FyleMask[::fyle(promote)];
		sq::ID	advanced	= sq::ID(isWhite(side) ? msb(fpawns) : lsb(fpawns));
		int		fkd		= sq::distance(kingSq(side), promote) - (sideToMove() == side);
		int		pd			= sq::distance(advanced, promote) - (sideToMove() == side);

		if (ekd - 1 <= mstl::min(fkd, pd))
			return false;
	}
#endif

	// If both sides have pawns, and we have made it through
	// the previous tests, then this side has winning
	// chances.
	if (m_matCount[side].pawn)
		return true;

	// If side has two bishops, and the other side has
	// a single kinght, the two bishops win.
	if (	m_matCount[side].pawn == 0
		&& minor(m_matCount[side]) == 2
		&& minor(m_matCount[opponent]) == 1
		&& (m_matCount[side].knight > 0 || m_matCount[opponent].knight == 0))
	{
		return false;
	}

	// If one side is two knights ahead and the opponent has
	// no remaining material, it is a draw.
	if (	m_matCount[side].pawn == 0
		&& m_matCount[side].knight == 2
		&& m_totalPieces[opponent] + m_matCount[opponent].pawn == 0)
	{
		return false;
	}

	// Check to see if this is a KRP vs KR or KQP vs KQ type
	// ending. If so, and the losing king is in front of the
	// passer, then this is a drawish ending.
	if (	m_matCount[side].pawn == 1
		&& m_matCount[opponent].pawn == 0
		&& m_majors == 0
		&& m_matCount[side].rook + m_matCount[side].queen == 1)
	{
		sq::ID sq = sq::ID(lsb(pawns));	// this side has only one pawn

		if (	(	sq::fyleDistance(kingSq(opponent), sq) <= 1
				&& ::isInFrontOf(side, kingSq(opponent), sq))
			|| sq::fyleDistance(kingSq(side), sq) > 1
			|| ::isBehind(side, kingSq(side), sq))
		{
			return false;
		}
	}

	return true;
}


// ------------------------------------------------------------------------
// adopted from crafty-22.8/evaluate.c:EvaluateDraws()
// ------------------------------------------------------------------------
// evaluateDraws() is used to adjust the score based on whether the side
// that appears to be better according the computed score can actually win
// the game or not. If the answer is "no" then the score is reduced
// significantly to reflect the lack of winning chances.
int
db::Guess::evaluateDraws(WinningChances canWin, int score)
{
	if ((canWin == OnlyBlackCanWin && score > 0) || (canWin == OnlyWhiteCanWin && score < 0))
		return ::DrawScore;

	// If the ending has only bishops of opposite colors, the
	// score is pulled closer to a draw. If the score says
	// one side is winning, but that side doesn't have enough
	// material to win, the score is set to DRAW.
	//
	// If this is a pure BOC ending, it is very drawish un-
	// less one side has at least 4 pawns. More pawns makes
	// it harder for a bishop and king to stop them all from
	// advancing.
	if (	m_totalPieces[White] <= 8
		&& m_totalPieces[Black] <= 8
		&& m_matCount[White].bishop == 1
		&& m_matCount[Black].bishop == 1
		&& hasBishopOnDark(White) != hasBishopOnDark(Black))
	{
		if (	m_totalPieces[White] == 3
			&&	m_totalPieces[Black] == 3
			&& (	(	m_matCount[White].pawn < 4
					&& m_matCount[Black].pawn < 4)
				|| mstl::abs(m_matCount[White].pawn - m_matCount[Black].pawn) < 2))
		{
			score = score/2;
		}
		else if (m_totalPieces[White] == m_totalPieces[Black])
		{
			score = (3*score)/4;
		}
	}

	// If we are running into the 50-move rule, then start
	// dragging the score toward draw. This is the idea of a
	// "weariness factor" as mentioned by Dave Slate many
	// times. This avoids slamming into a draw at move 50
	// and having to move something quickly, rather than
	// slowly discovering that the score is dropping and that
	// pushing a pawn or capturing something will cause it to
	// go back to its correct value a bit more smoothly.
	if (m_halfMoveClock > 80)
		score = (score*(101 - m_halfMoveClock))/20;

	return score;
}


// ------------------------------------------------------------------------
// adopted from crafty-22.8/evaluate.c:EvaluateMate()
// ------------------------------------------------------------------------
// evaluateMate() is used to evaluate positions where neither side has pawns
// and one side has enough material to force checkmate. It simply trys to
// force the losing king to the edge of the board, and then to the corner
// where mates are easier to find.
int
db::Guess::evaluateMate(color::ID side)
{
	int score = 0;

	color::ID opponent = opposite(side);

	// If one side has a bishop+knight and the other side has
	// no pieces or pawns, then use the special bishop_knight
	// scoring board for the losing king to force it to the
	// right corner for mate.
	if (m_matCount[opponent].value == 0 && m_matCount[side].bishop == 1 && m_matCount[side].knight == 1)
	{
		sq::ID square = kingSq(opponent);

		if (hasBishopOnLite(side))
			square = flipFyle(square);

		score = mate::BishopKnightSquare[square];
	}
	else
	{
		// If one side is winning, force the enemy king to the edge of the board.
		score = mate::Square[kingSq(opponent)]
				- (sq::distance(kingSq(side), kingSq(opponent)) - 3)*bonus::KingKingTropism;
	}

	TRACE("score[mate]                         = %d\n", score);

	return score;
}


// ------------------------------------------------------------------------
// adopted from crafty-22.8/evaluate.c:EvaluateDevelopment()
// ------------------------------------------------------------------------
// evaluateDevelopment() is used to encourage the program to develop its
// pieces before moving its queen. Standard developmental principles are
// applied. They include:
//
// 	(1) don't move the queen until minor pieces are developed
// 	(2) advance the center pawns as soon as possible
// 	(3) don't move the king unless its a castling move.
int
db::Guess::evaluateDevelopment(color::ID side)
{
	//M_ASSERT(m_root.canCastle(side));

	int		score	= 0;
	uint64_t	pawns	= this->pawns(side);

	// First, some "thematic" things, which includes don't
	// block the c-pawn in queen-pawn openings.
	if (m_idn == chess960::StandardIdn)
	{
		if (isWhite(side))
		{
			if (!(pawns & E4) && (pawns & D4) && (pawns & C2) && ((knights(side) | bishops(side)) & C3))
				score -= development::Thematic;
		}
		else
		{
			if (!(pawns & E5) && (pawns & D5) && (pawns & C7) && ((knights(side) | bishops(side)) & C6))
				score -= development::Thematic;
		}
	}

	TRACE_2("%s development[thematic]          = %d\n", printColor(side), score);

	// If the king hasn't moved at the beginning of the
	// search, but it has moved somewhere in the current
	// search path, make *sure* it's a castle move or else
	// penalize the loss of castling privilege.
	Rights rights = castling::bothSides(side);

	if ((m_root.m_castle & rights) && (m_castling & rights) == 0)
	{
		if ((m_root.m_castle & rights) == (m_castle & rights))
		{
			score -= development::NotCastled;
		}
		else
		{
			int penalty = (m_matCount[side].queen ? 3 : 1)*development::LosingCastle;

			if (m_castle & rights)
				penalty = ::div2(penalty);

			score -= penalty;
		}
	}
	TRACE_2("%s development[castling]          = %d\n", printColor(side), score);

	// Check for an undeveloped knight/rook combo
	int rooks	= count(this->rooks(side) & HomeRankMask[side]);
	int knights	= count(this->knights(side) & HomeRankMask[side]);

	if (rooks + knights == 4)
		score -= ::mul2(development::UndevelopedPiece);
	else if (rooks & knights)
		score -= development::UndevelopedPiece;

	TRACE_2("%s development[undeveloped]       = %d\n", printColor(side), score);
	TRACE_1("%s development                    = %d\n", printColor(side), score);

	return score;
}


// ------------------------------------------------------------------------
// adopted from crafty-22.8/evaluate.c:EvaluateKings()
// ------------------------------------------------------------------------
// evaluateKings() is used to evaluate black/white kings.
db::Guess::Score
db::Guess::evaluateKings(color::ID side, Flip flip)
{
	Score			score;
	color::ID	opponent			= opposite(side);
	sq::ID		kingSq			= this->kingSq(side);
	sq::ID		flippedSquare	= flip(kingSq);
	int			kingFyle			= ::fyle(kingSq);
	int			defects			= 0;
	Eval const&	eval				= m_pawnData->eval;

	// First, check for where the king should be if this is
	// an endgame. I.e. with pawns on one wing, the king needs
	// to be on that wing. With pawns on both wings, the
	// king belongs in the center.
	if (	(m_pawns & (FyleMaskE | FyleMaskF | FyleMaskG | FyleMaskH))
		&& (m_pawns & (FyleMaskA | FyleMaskB | FyleMaskC | FyleMaskD)))
	{
		score.endGame += bonus::KingSquareN[flippedSquare];
	}
	else if (m_pawns & (FyleMaskE | FyleMaskF | FyleMaskG | FyleMaskH))
	{
		score.endGame += bonus::KingSquareK[flippedSquare];
	}
	else if (m_pawns)
	{
		score.endGame += bonus::KingSquareQ[flippedSquare];
	}

	TRACE_2("%s kings[end game]                = %d\n", printColor(side), score.endGame);

	if (m_dangerous[opponent])
	{
		// Now, check for the "trojan horse" attack where the
		// opponent offers a piece to open the h-file with a very
		// difficult to refute attack.
		if (m_trojanCheck && m_root.sideToMove() == side && ::fyle(m_ksq[side]) >= FyleE)
		{
			if (!(m_pawns & FyleMaskH) && m_matCount[opponent].rook && m_matCount[opponent].queen)
				score.middleGame -= penalty::KingSafetyMateThreat;
		}

		TRACE_2("%s kings[trojan horse]            = %d\n", printColor(side), score.middleGame);

		// Now do castle scoring, if the king has castled, the
		// pawns in front are important. If not castled yet, the
		// pawns on the kingside should be preserved for this.
		if (canCastleShort(side) && canCastleLong(side))
		{
			defects = mstl::min(	mstl::min(	eval[side].defects[FyleG],
														eval[side].defects[kingFyle]),
										eval[side].defects[FyleB]);
		}
		else if (canCastleShort(side))
		{
			defects = mstl::min(eval[side].defects[FyleG], eval[side].defects[kingFyle]);
		}
		else if (canCastleLong(side))
		{
			defects = mstl::min(eval[side].defects[FyleB], eval[side].defects[kingFyle]);
		}
		else if (kingFyle >= FyleE)
		{
			defects = eval[side].defects[kingFyle > FyleE ? FyleG : FyleE];
		}
		else
		{
			defects = eval[side].defects[kingFyle < FyleD ? FyleB : FyleD];
		}

		if (canCastle(side) && defects < 3)
			defects = 3;

		// Now fold in the king tropism and king pawn shelter
		// scores together. Also add in an enemy pawn tropism
		// score.
		if (	(king(side) & (HomeRankMask[side] | PawnRankMask[side]))
			&& (kingAttacks(kingSq) | kingAttacks(NextRank[side][kingSq])) & pawns(opponent))
		{
			m_tropism[opponent] += 3;
		}

		m_tropism[opponent] = mstl::max(0, mstl::min(15, m_tropism[opponent]));
		defects = mstl::min(15, defects);

		score.middleGame -= safety::KingSafety[defects][m_tropism[opponent]];
	}

	TRACE("%s score[defects]                 = %d\n", printColor(side), defects);
	TRACE("%s score[tropism]                 = %d\n", printColor(opponent), m_tropism[opponent]);
	TRACE("%s score[kings]                   = %d, %d\n",
			printColor(side),
			score.middleGame, score.endGame);

	return score;
}


// ------------------------------------------------------------------------
// adopted from crafty-22.8/evaluate.c:EvaluateKnights()
// ------------------------------------------------------------------------
// evaluateKnights() is used to evaluate black/white knights.
db::Guess::Score
db::Guess::evaluateKnights(color::ID side, Flip flip)
{
	Score			score;
	color::ID	opponent		= opposite(side);
	uint64_t		knights		= this->knights(side);
	uint64_t		enemyPawns	= pawns(opponent);

	// First, evaluate for "outposts" which is a knight that
	// can't be driven off by an enemy pawn, and which is
	// supported by a friendly pawn.
	while (knights)
	{
		sq::ID square			= sq::ID(lsbClear(knights));
		sq::ID flippedSquare	= flip(square);

		// First fold in centralization score.
		score += bonus::KnightSquare[flippedSquare];

		TRACE_2(	"%s knights[central]               = %d, %d\n",
					printColor(side),
					score.middleGame, score.endGame);

		// Now, evaluate for "outposts" which is a knight that
		// can't be driven off by an enemy pawn, and which is
		// supported by a friendly pawn.
		//
		// If the enemy has NO minor to take this knight, then
		// increase the bonus.
		if (!(enemyPawns & MaskNoAttacks[opponent][square]))
		{
			int outpost = bonus::KnightOutpostSquare[flippedSquare];

			if (outpost)
			{
				score += outpost;

				if (pawns(side) & PawnAttacks[opponent][square])
				{
					score += ::div2(outpost);

					if (!this->knights(opponent))
					{
						if (!(isWhite(sq::color(square)) ?
									hasBishopOnLite(opponent) : hasBishopOnDark(opponent)))
						{
							score += outpost;
						}
					}
				}
			}
		}

		TRACE_2("%s knights[outpost]               = %d, %d\n",
				printColor(side),
				score.middleGame, score.endGame);

		// Mobility counts the number of squares the piece
		// attacks, excluding squares with friendly pieces, and
		// weighs each square according to centralization.
		{
			static uint64_t const MobilityMask[4] =
			{
				RankMask1 | RankMask8 | FyleMaskA | FyleMaskH,
				((RankMask2 | RankMask7) & ~(A2 | H2 | A7 | H7)) | B3 | B4 | B5 | B6 | G3 | G4 | G5 | G6,
				C3 | D3 | E3 | F3 | C6 | D6 | E6 | F6 | C4 | C5 | F4 | F5,
				D4 | E4 | D5 | E5,
			};

			uint64_t moves = (knightAttacks(square) & ~m_occupiedBy[side]) | ::setBit(square);

			score	-= penalty::LowerKnight;
			score += count(moves & MobilityMask[0]);
			score += ::mul2(count(moves & MobilityMask[1]));
			score += ::mul3(count(moves & MobilityMask[2]));
			score += ::mul4(count(moves & MobilityMask[3]));

			TRACE_2("%s knights[mobility]              = %d, %d\n",
					printColor(side),
					score.middleGame, score.endGame);
		}

		// Adjust the tropism count for this piece.
		if (m_dangerous[side])
			m_tropism[side] += bonus::KnightKingTropism[sq::distance(square, kingSq(opponent))];
	}

	TRACE("%s score[knights]                 = %d, %d\n",
			printColor(side),
			score.middleGame, score.endGame);
	TRACE("%s tropism[knights]               = %d\n",
			printColor(side),
			m_tropism[side]);

	return score;
}


// ------------------------------------------------------------------------
// adopted from crafty-22.8/evaluate.c:EvaluateBishops()
// ------------------------------------------------------------------------
// evaluateBishops() is used to evaluate black/white bishops.
db::Guess::Score
db::Guess::evaluateBishops(color::ID side, Flip flip)
{
	Score			score;
	color::ID	opponent		= opposite(side);
	int			pair			= hasBishopOnLite(side) && hasBishopOnDark(side);
	uint64_t		bishops		= this->bishops(side);
	uint64_t		enemyPawns	= pawns(opponent);

	TRACE_2(	"%s bishops[pawn on wings]         = %d, %d\n",
				printColor(side),
				score.middleGame, score.endGame);

	// Check for pawns on both wings, which makes a bishop
	// even more valuable against an enemy knight.
	if (	(m_pawns & (FyleMaskF | FyleMaskG | FyleMaskH))
		&& (m_pawns & (FyleMaskA | FyleMaskB | FyleMaskC)))
	{
		score += count(bishops)*bonus::BishopWithWingPawns;
	}

	// First, locate each bishop and add in its static score
	// from the bishop piece/square table.
	while (bishops)
	{
		sq::ID square			= sq::ID(lsbClear(bishops));
		sq::ID flippedSquare	= flip(square);

		score += bonus::BishopSquare[flippedSquare];

		TRACE_2(	"%s bishops[central]               = %d, %d\n",
					printColor(side),
					score.middleGame, score.endGame);

		// Now, evaluate for "outposts" which is a bishop that
		// can't be driven off by an enemy pawn, and which is
		// supported by a friendly pawn.
		//
		// If the enemy has NO minor to take this bishop, then
		// increase the bonus.
		if (!(enemyPawns & MaskNoAttacks[opponent][square]))
		{
			int outpost = bonus::BishopOutpostSquare[flippedSquare];

			if (outpost)
			{
				score += outpost;

				if (pawns(side) & PawnAttacks[opponent][square])
				{
					score += ::div2(outpost);

					if (!knights(opponent))
					{
						if (!(isWhite(sq::color(square)) ?
									hasBishopOnLite(opponent) : hasBishopOnDark(opponent)))
						{
							score += outpost;
						}
					}
				}
			}
		}

		TRACE_2("%s bishops[outpost]               = %d, %d\n",
				printColor(side),
				score.middleGame, score.endGame);

		// Check to see if the bishop is trapped at a7 or h7 with
		// a pawn at b6 or g6 that can advance one square and
		// trap the bishop, or a pawn at b6 or g6 that has
		// trapped the bishop already. Also test for the bishop
		// at b8 or g8 as that might not be an escape.
		if (	(flippedSquare == a7 && (enemyPawns & ::setBit(flip(b6))))
			||	(flippedSquare == b8 && (enemyPawns & ::setBit(flip(c7))))
			||	(flippedSquare == h7 && (enemyPawns & ::setBit(flip(g6))))
			||	(flippedSquare == g8 && (enemyPawns & ::setBit(flip(f7)))))
		{
			score -= penalty::BishopTrapped;
		}

		TRACE_2("%s bishops[trapped]               = %d, %d\n",
				printColor(side),
				score.middleGame, score.endGame);

		// Mobility counts the number of squares the piece
		// attacks, excluding squares with friendly pieces, and
		// weighs each square according to centralization.
		{
			static uint64_t const MobilityMask[4] =
			{
				RankMask1 | RankMask8 | FyleMaskA | FyleMaskH,
				((RankMask2 | RankMask7) & ~(A2 | H2 | A7 | H7)) | B3 | B4 | B5 | B6 | G3 | G4 | G5 | G6,
				C3 | D3 | E3 | F3 | C6 | D6 | E6 | F6 | C4 | C5 | F4 | F5,
				D4 | E4 | D5 | E5,
			};

			uint64_t moves = (bishopAttacks(square) & ~m_occupiedBy[side]) | ::setBit(square);

			score	-= penalty::LowerBishop;
			score += count(moves & MobilityMask[0])*(1 + pair);
			score += count(moves & MobilityMask[1])*(2 + pair);
			score += count(moves & MobilityMask[2])*(3 + pair);
			score += count(moves & MobilityMask[3])*(4 + pair);

			TRACE_2("%s bishops[mobility]              = %d, %d\n",
					printColor(side),
					score.middleGame, score.endGame);
		}

		// Adjust the tropism count for this piece.
		if (m_dangerous[side])
			m_tropism[side] += bonus::BishopKingTropism[sq::distance(square, kingSq(opponent))];
	}

	TRACE("%s score[bishops]                 = %d, %d\n",
			printColor(side),
			score.middleGame, score.endGame);
	TRACE("%s tropism[bishops]               = %d\n",
			printColor(side),
			m_tropism[side]);

	return score;
}


// ------------------------------------------------------------------------
// adopted from crafty-22.8/evaluate.c:EvaluateRooks()
// ------------------------------------------------------------------------
// evaluateRooks() is used to evaluate black/white rooks.
db::Guess::Score
db::Guess::evaluateRooks(color::ID side, Flip flip)
{
	Score			score;
	color::ID	opponent	= opposite(side);
	uint64_t		rooks		= this->rooks(side);
	Eval const&	eval		= m_pawnData->eval;

	while (rooks)
	{
		sq::ID	square			= sq::ID(lsbClear(rooks));
		sq::ID	flippedSquare	= flip(square);
		int		fyle				= ::fyle(square);

		// Determine if the rook is on an open fyle or on a half-
		// open fyle, either of which increases its ability to
		// attack important squares.
		if (!(pawns(side) & FyleMask[fyle]))
		{
			if (!(pawns(opponent) & FyleMask[fyle]))
				score += bonus::RookOpenFyle;
			else
				score += bonus::RookHalfOpenFyle;
		}

		TRACE_2(	"%s rooks[(half) open fyle]        = %d, %d\n",
					printColor(side),
					score.middleGame, score.endGame);

		// Determine if the rook is on the 7th rank, with the
		// enemy king trapped on the 8th rank. If so the rook
		// exerts a "cramping" effect that is valuable.
		if (::rank(flippedSquare) == Rank7)
			score += bonus::RookOn7th;

		TRACE_2(	"%s rooks[on 7th rank]             = %d, %d\n",
					printColor(side),
					score.middleGame, score.endGame);

		// See if the rook is behind a passed pawn. If it is,
		// it is given a bonus.
		if (eval[White].passedPawn & (1 << fyle))
		{
			int pawnSq = msb(pawns(White) & FyleMask[fyle]);

			if (msb(fyleAttacks(square) & ::Plus8Dir[square]) == pawnSq)
				score += bonus::RookBehindPassedPawn;
		}

		if (eval[Black].passedPawn & (1 << fyle))
		{
			int pawnSq = lsb(pawns(Black) & FyleMask[fyle]);

			if (lsb(fyleAttacks(square) & ::Minus8Dir[square]) == pawnSq)
				score += bonus::RookBehindPassedPawn;
		}

		TRACE_2(	"%s rooks[behind passed pawn]      = %d, %d\n",
					printColor(side),
					score.middleGame, score.endGame);

		// Check to see if the king has been forced to move and
		// has trapped a rook at a1/a2/b1/g1/h1/h2, if so, then
		// penalize the trapped rook to help extricate it.
		if (::rank(m_ksq[side]) == HomeRank[side])
		{
			if (::fyle(m_ksq[side]) < FyleD)
			{
				if (flippedSquare < flip(kingSq(side)))
					score -= penalty::RookTrapped;
			}
			else if (::fyle(kingSq(side)) > FyleE)
			{
				if (flippedSquare <= h1 && flip(kingSq(side)) < flippedSquare)
					score -= penalty::RookTrapped;
			}
		}

		TRACE_2(	"%s rooks[trapped]                 = %d, %d\n",
					printColor(side),
					score.middleGame, score.endGame);

		// Mobility counts the number of squares the piece
		// attacks, excluding squares with friendly pieces, and
		// weighs each square according to centralization (fyle).
		{
			uint64_t moves = (rookAttacks(square) & ~m_occupiedBy[side]) | ::setBit(square);

			score	-= penalty::LowerRook;
			score += count(moves & (FyleMaskA | FyleMaskH));
			score += ::mul2(count(moves & (FyleMaskB | FyleMaskG)));
			score += ::mul3(count(moves & (FyleMaskC | FyleMaskF)));
			score += ::mul4(count(moves & (FyleMaskD | FyleMaskE)));

			TRACE_2(	"%s rooks[mobility]                = %d, %d\n",
						printColor(side),
						score.middleGame, score.endGame);
		}

		// Adjust the tropism count for this piece.
		if (m_dangerous[side])
		{
			uint64_t mask			= (m_queens | m_rooks) & m_occupiedBy[side];
			uint64_t rookAttacks	= rankAttacks(square, m_occupied & ~mask);
			uint64_t kingAttacks	= this->kingAttacks(m_ksq[opponent]);

			if (rookAttacks & kingAttacks)
			{
				m_tropism[side] += bonus::RookKingTropism[1];
			}
			else
			{
				uint64_t occupied = m_occupiedL90;

				while (mask)
					occupied &= ~MaskL90[lsbClear(mask)];

				rookAttacks = fyleAttacks(square, occupied);

				if (rookAttacks & kingAttacks)
					m_tropism[side] += bonus::RookKingTropism[1];
				else
					m_tropism[side] += bonus::RookKingTropism[sq::distance(square, kingSq(opponent))];
			}
		}
	}

	TRACE("%s score[rooks]                   = %d, %d\n",
			printColor(side),
			score.middleGame, score.endGame);
	TRACE("%s tropism[rooks]                 = %d\n",
			printColor(side),
			m_tropism[side]);

	return score;
}


// ------------------------------------------------------------------------
// adopted from crafty-22.8/evaluate.c:EvaluateQueens()
// ------------------------------------------------------------------------
// evaluateQueens() is used to evaluate black/white queens.
db::Guess::Score
db::Guess::evaluateQueens(color::ID side, Flip flip)
{
	Score			score;
	color::ID	opponent	= opposite(side);
	uint64_t		queens	= this->queens(side);

	// First locate each queen and obtain it's centralization
	// score from the static piece/square table for queens.
	// Then, if the opposing side's king safety is much worse
	// than the king safety for this side, add in a bonus to
	// keep the queen around.

	while (queens)
	{
		sq::ID square			= sq::ID(lsbClear(queens));
		sq::ID flippedSquare	= flip(square);

		score += bonus::QueenSquare[flippedSquare];

		TRACE_2(	"%s queens[central]                = %d, %d\n",
					printColor(side),
					score.middleGame, score.endGame);

		// Adjust the tropism count for this piece.
		//
		// Now we notice whether the queen is on a file that is
		// bearing on the enemy king and adjust tropism if so.
		if (m_dangerous[side])
			m_tropism[side] += bonus::QueenKingTropism[sq::distance(square, kingSq(opponent))];

		m_tropism[opponent] -= penalty::FriendlyQueen[sq::distance(square, kingSq(side))];
	}

	TRACE("%s score[queens]                  = %d, %d\n",
			printColor(side),
			score.middleGame, score.endGame);
	TRACE("%s tropism[queens]                = %d\n",
			printColor(side),
			m_tropism[side]);

	return score;
}


// ------------------------------------------------------------------------
// adopted from crafty-22.8/evaluate.c:EvaluatePawns()
// ------------------------------------------------------------------------
// evaluatePawns() is used to evaluate pawns. It evaluates pawns for only
// one side, and fills in the pawn hash entry information. It requires two
// calls to evaluate all pawns on the board. Comments below indicate the
// particular pawn structure features that are evaluated.
//
// This procedure also fills in information (without scoring) that other
// evaluation procedures use, such as which pawns are passed or candidates,
// which pawns are weak, which files are open, and so forth.
db::Guess::Score
db::Guess::evaluatePawns(color::ID side)
{
	uint64_t		pawnMoves	= 0;
	color::ID	opponent		= opposite(side);
	Score			score			= 0;
	int			dir			= isWhite(side) ? 8 : -8;
	uint64_t 	myPawns		= pawns(side);
	uint64_t 	enemyPawns	= pawns(opponent);
	uint64_t 	pawns			= myPawns;
	Eval&			eval			= m_pawnData->eval;

	uint64_t const* pawnAttacks			= PawnAttacks[side];
	uint64_t const* pawnAttacksOpponent	= PawnAttacks[opponent];

	Rank pawnRankOpponent = PawnRank[opponent];

	// First, determine which squares pawns can reach.
	while (pawns)
	{
		int square	= lsbClear(pawns);
		int last		= sq::make(::fyle(square), pawnRankOpponent);
		int next		= square + dir;

		eval[side].all |= 1 << ::fyle(square);

		for (int sq = square; sq != last; sq = next, next += dir)
		{
			pawnMoves |= ::setBit(sq);

			if (::setBit(next) & m_pawns)
				break;

			int defenders = count(pawnAttacksOpponent[next] & myPawns);
			int attackers = count(pawnAttacks[next] & enemyPawns);

			if (attackers - defenders > 0)
				break;
		}
	}

	pawns = myPawns;

	PawnEval& pawnEval = eval[side];

	uint64_t const* maskPassedPawn	= MaskPassedPawn[side];
	uint64_t const* maskHiddenRight	= MaskHiddenRight[side];
	uint64_t const* maskHiddenLeft	= MaskHiddenLeft[side];

	while (pawns)
	{
		int square	= lsbClear(pawns);
		int fyle		= ::fyle(square);
		int rank		= isWhite(side) ? ::rank(square) : Rank8 - ::rank(square);
		int sqIndex	= sq::make(fyle, rank);

		// The first thing we do is make a note that the current
		// fyle can't be open since there is a pawn on it.
		pawnEval.openFyle &= ~(1 << fyle);

		// Evaluate pawn advances. Center pawns are encouraged
		// to advance, while wing pawns are pretty much neutral.
		score += bonus::PawnSquare[sqIndex];

		TRACE_2(	"%s pawn[static]   fyle %c, score   = %d, %d\n",
					printColor(side), printFyle(fyle),
					score.middleGame, score.endGame);

		pawnEval.longVsShortScore += bonus::PawnStorm[sqIndex];
		pawnEval.shortVsLongScore += bonus::PawnStorm[::flipFyle(sq::ID(sqIndex))];

		// Evaluate isolated pawns, which are penalized based on
		// the fyle, with central isolani being worse than when
		// on the wings.
		if (!(myPawns & MaskIsolatedPawn[square]))
		{
			score -= pawn::Isolated;

			if (!(enemyPawns & FyleMask[fyle]))
				score -= pawn::Isolated/2;

			TRACE_2(	"%s pawn[isolated] fyle %c, score   = %d, %d\n",
						printColor(side), printFyle(fyle),
						score.middleGame, score.endGame);
		}
		else
		{
			score += evaluateWeakPawns(side, square, pawnMoves);

			TRACE_2(	"%s pawn[weak]     fyle %c, score   = %d, %d\n",
						printColor(side), printFyle(fyle),
						score.middleGame, score.endGame);

			// Evaluate doubled pawns. If there are other pawns on
			// this fyle, penalize this pawn.
			if (count(myPawns & FyleMask[fyle]) > 1)
				score -= pawn::Doubled;

			TRACE_2(	"%s pawn[doubled]  fyle %c, score   = %d, %d\n",
						printColor(side), printFyle(fyle),
						score.middleGame, score.endGame);

			// Test the pawn to see it if forms a "duo" which is two
			// pawns side-by-side.
			if (MaskDuoPawn[square] & myPawns)
				score += pawn::Duo;

			TRACE_2(	"%s pawn[duo]      fyle %c, score   = %d, %d\n",
						printColor(side), printFyle(fyle),
						score.middleGame, score.endGame);
		}

		// Discover and flag passed pawns for use later.
		if (!(maskPassedPawn[square] & enemyPawns))
		{
			pawnEval.passedPawn |= 1 << fyle;
			TRACE_2("%s pawn[passed]   fyle %c\n", printColor(side), printFyle(fyle));
		}
		// Now determine if this pawn is a candidate passer,
		// since we now know it isn't passed. A candidate is a
		// pawn on a fyle with no enemy pawns in front of it, and
		// if it advances until it contacts an enemy pawn, and it
		// is defended as many times as it is attacked when it
		// reaches that pawn, then all that is left is to see if
		// it is passed when the attacker(s) get removed.
		else if (	!(FyleMask[fyle] & enemyPawns)
					&& (MaskIsolatedPawn[square] & myPawns)
					&& !(pawnAttacks[square] & enemyPawns))
		{
			int attackers	= 1;
			int defenders	= 0;
			int next			= square + dir;
			int last			= sq::make(fyle, isWhite(side) ? Rank7 : Rank2);

			for (int sq = square; sq != last; sq = next, next += dir)
			{
				if (m_pawns & ::setBit(next))
					break;

				defenders = count(pawnAttacksOpponent[sq] & pawnMoves);
				attackers = count(pawnAttacks[sq] & enemyPawns);

				if (attackers)
					break;
			}

			if (attackers <= defenders)
			{
				if (!(maskPassedPawn[next] & enemyPawns))
				{
					pawnEval.candidates |= 1 << fyle;
					score += pawn::PassedPawnCandidate[rank];

					TRACE_2(	"%s pawn[candiate]  sq %s, score  = %d, %d\n",
								printColor(side), printAlgebraic(square),
								score.middleGame, score.endGame);
				}
			}
		}

		// Evaluate "hidden" passed pawns. Simple case is a pawn
		// chain (white) at b5, a6, with a black pawn at a7.
		// It appears the b-pawn is backward, with a ram at a6/a7
		// but this is misleading, because the pawn at a6 is
		// really passed when white plays b6.
		if (	rank == Rank3
			&& (enemyPawns & ::setBit(square + dir))
			&& (	(	fyle < FyleH
					&& (myPawns & ::setBit(square + (1 - dir)))
					&& !(enemyPawns & maskHiddenRight[fyle]))
				|| (	fyle > FyleA
					&& (myPawns & ::setBit(square + (1 - dir)))
					&& !(enemyPawns & maskHiddenLeft[fyle]))))
		{
			score += pawn::PassedPawnHidden;

			TRACE_2(	"%s pawn[hidden]   fyle %c, score   = %d, %d\n",
						printColor(side), printFyle(fyle),
						score.middleGame, score.endGame);
		}
	}

	// Now evaluate king safety.
	//
	// The first step is to step across the board and note
	// which files are open/half open. Since this is common
	// to both kings, this is only done once since it is the
	// same for both players.
	//
	// At the same time we note if any of the three pawns in
	// front of the king have moved, and count those as well.
	//
	pawnEval.defects[FyleB] = evaluateKingsFyle(side, FyleB);
	pawnEval.defects[FyleG] = evaluateKingsFyle(side, FyleG);
	pawnEval.defects[FyleD] = evaluateKingsFyle(side, FyleD);
	pawnEval.defects[FyleE] = evaluateKingsFyle(side, FyleE);

	if (m_idn != chess960::StandardIdn)
	{
		int fyle = ::fyle(m_ksq[side]);

		if (fyle == FyleC || fyle == FyleF)
			pawnEval.defects[fyle] = evaluateKingsFyle(side, fyle);
	}

	return score;
}


// ------------------------------------------------------------------------
// adopted from crafty-22.8/evaluate.c:EvaluateKingsFile()
// ------------------------------------------------------------------------
// evaluateKingsFyle() computes defects for a fyle, based on whether
// the fyle is open or half-open. If there are friendly pawns still
// on the fyle, they are penalized for advancing in front of the king.
int
db::Guess::evaluateKingsFyle(color::ID side, int whichFyle)
{
	//M_ASSERT(whichFyle > FyleA && whichFyle < FyleH);

	color::ID	opponent		= opposite(side);
	uint64_t		myPawns		= pawns(side);
	uint64_t		enemyPawns	= pawns(opponent);

	int defects = 0;

	for (int fyle = whichFyle - 1; fyle <= whichFyle + 1; ++fyle)
	{
		if (!(m_pawns & FyleMask[fyle]))
		{
			defects += pawn::OpenFyle[fyle];
		}
		else
		{
			if (!(enemyPawns & FyleMask[fyle]))
			{
				defects += ::div2(pawn::HalfOpenFyle[fyle]);
			}
			else
			{
				uint64_t	pawns		= enemyPawns & FyleMask[fyle];
				int		advanced	= isWhite(side) ? lsb(pawns) : msb(pawns);
				int		rank		= ::rank(advanced);

				if (isBlack(side))
					rank = Rank8 - rank;

				defects += pawn::Defects[rank];
			}

			if (!(myPawns & FyleMask[fyle]))
			{
				defects += pawn::HalfOpenFyle[fyle];
			}
			else if (!(myPawns & ::setBit(sq::make(fyle, PawnRank[side]))))
			{
				++defects;

				if (!(myPawns & ::setBit(sq::make(fyle, EpRank[side]))))//XXX ok?
					++defects;
			}
		}
	}

	TRACE_2("%s pawn[safety]   fyle %c, defects = %d\n", printColor(side), printFyle(whichFyle), defects);

	return defects;
}


// ------------------------------------------------------------------------
// adopted from crafty-22.8/evaluate.c:EvaluatePawns()
// ------------------------------------------------------------------------
// Evaluate weak pawns. Weak pawns are evaluated by the
// following rules:
//
//   (1) If a pawn is defended by a pawn, it isn't weak.
//   (2) If a pawn is undefended by a pawn and advances
//       one (or two if it hasn't moved yet) ranks and
//       is defended fewer times than it is attacked, it
//       is weak.
//
// Note that the penalty is greater if the pawn is on an
// open fyle. Note that an isolated pawn is just another
// case of a weak pawn, since it can never be defended by
// a pawn.
db::Guess::Score
db::Guess::evaluateWeakPawns(color::ID side, int square, uint64_t pawnMoves)
{
	color::ID	opponent		= opposite(side);
	uint64_t		myPawns		= pawns(side);
	uint64_t		enemyPawns	= pawns(opponent);

	// First, test the pawn where it sits to determine if it
	// is defended more times than attacked. If so, it is not
	// weak and we are done. If it is weak where it sits, can
	// it advance one square and become not weak. If so we
	// are again finished with this pawn.

	uint64_t r = pawnMoves & (isWhite(side) ? ::Plus8Dir[square] : ::Minus8Dir[square]);

	while (r)
	{
		int sq			= lsbClear(r);
		int defenders	= count(PawnAttacks[opponent][sq] & myPawns);
		int attackers	= count(PawnAttacks[side][sq] & enemyPawns);

		if (defenders && defenders >= attackers)
			return 0;
	}

	if (!(PawnAttacks[opponent][square] & pawnMoves))
	{
		// If the pawn can be defended by a pawn, and that pawn
		// can safely advance to actually defend this pawn, then
		// this pawn is not weak.
		Score score = -pawn::Weak;

		if (!(enemyPawns & FyleMask[::fyle(square)]))
		{
			static Score const Weak = pawn::Weak/2;
			score += Weak;
		}

		return score;
	}

	return 0;
}


// ------------------------------------------------------------------------
// adopted from crafty-22.8/evaluate.c:EvaluatePassedPawns()
// ------------------------------------------------------------------------
// evaluatePassedPawns() is used to evaluate passed pawns and the danger
// they produce. This code considers pieces as well.
db::Guess::Score
db::Guess::evaluatePassedPawns(color::ID side, Flip flip)
{
	//M_ASSERT(m_pawnData->eval[side].passedPawn);

	Score			score;
	color::ID	opponent	= opposite(side);
	uint64_t		myPawns	= pawns(side);
	Eval const&	eval		= m_pawnData->eval;
	uint8_t		pawns		= eval[side].passedPawn;

	while (pawns)
	{
		static int RankBonus[8] = { 0, 0, 0, 2, 6, 12, 20, 35 };

		uint64_t	r				= myPawns & FyleMask[lsbClear(pawns)];
		sq::ID	square		= sq::ID(isWhite(side) ? msb(r) : lsb(r));
		int		rank			= ::rank(flip(square));
		int		rankBonus	= RankBonus[rank];

		TRACE_2("%s score[passed pawns] on square %s\n", printColor(side), printAlgebraic(square));

		// We have located the most advanced pawn on this
		// fyle, which is the only one that will get any
		// sort of bonus. Add in the scores first.
		score += pawn::PassedPawnValue[rank];

		TRACE_2(	"%s score[passed rank]            = %d, %d\n",
					printColor(side),
					score.middleGame, score.endGame);

		// Add in a bonus if the passed pawn is connected
		// with another pawn for support.
		if (myPawns & MaskPassedPawnConnected[square])
			score += pawn::PassedPawnConnected*rankBonus;

		TRACE_2(	"%s score[passed pawn connected] =    %d, %d\n",
					printColor(side), score.middleGame, score.endGame);

		sq::ID blocking = sq::ID(NextRank[side][square]);

		// If the pawn is blockaded by an enemy piece, it
		// cannot move and is therefore not nearly as
		// valuable as if it were free to advance.
		if (m_piece[blocking] != piece::Empty && (m_occupiedBy[opponent] & ::setBit(blocking)))
		{
			int rank = ::rank(flip(blocking));
			score -= pawn::BlockadingPassedPawnValue[rank];
		}

		TRACE_2(	"%s score[passed blocked]          = %d, %d\n",
					printColor(side),
					score.middleGame, score.endGame);

		// Add in a bonus based on how close the friendly
		// king is, and a penalty based on how close the
		// enemy king is. The bonus/penalty is based on
		// how advanced the pawn is to attract the kings
		// toward the most advanced (and most dangerous)
		// passed pawn.
		int distance = sq::distance(blocking, kingSq(side)) - sq::distance(blocking, kingSq(opponent));
		score.endGame -= distance*rankBonus;

		TRACE_2("%s score[passed supported]        = %d, %d\n",
					printColor(side),
					score.middleGame, score.endGame);
	}

	// Check to see if side has an outside passed pawn.
	if (IsOutside[eval[side].passedPawn] & ~eval[opponent].all)
		score += pawn::OutsidePassed;

	TRACE("%s score[outside]                 = %d, %d\n",
			printColor(side),
			score.middleGame, score.endGame);

	return score;
}


// ------------------------------------------------------------------------
// adopted from crafty-22.8/evaluate.c:EvaluatePassedPawnRaces()
// adopted from crafty-20.9/evaluate.c:EvaluatePassedPawnRaces()
// ------------------------------------------------------------------------
// evaluatePassedPawnRaces() is used to evaluate passed pawns when one
// side has passed pawns and the other side (or neither) has pieces. In
// such a case, the critical question is can the defending king stop the pawn
// from queening or is it too far away? If only one side has pawns that can
// "run" then the situation is simple. When both sides have pawns that can
// "run" it becomes more complex as it then becomes necessary to see if
// one side can use a forced king move to stop the other side, while the
// other side doesn't have the same ability to stop ours.
//
// In the case of king and pawn endings with exactly one pawn, the simple
// evaluation rules are used: If the king is two squares in front of the
// pawn then it is a win, if the king is one one square in front with the
// opposition, then it is a win, if the king is on the 6th rank with the
// pawn close by, it is a win. Rook pawns are handled separately and are
// more difficult to queen because the king can get trapped in front of the
// pawn blocking promotion.
db::Guess::Score
db::Guess::evaluatePassedPawnRaces()
{
	int			queener[2]		= { 8, 8 };
	Square		promoSq[2]		= { 0, 0 };
	Square		passedPawn[2]	= { 0, 0 };
	bool			realize[2]		= { false, false };
	Eval const&	eval				= m_pawnData->eval;

	for (int color = 0; color < 2; ++color)
	{
		sq::ID (*flip)(sq::ID) = (color ? ::wflip : ::bflip);

		color::ID	side		= color ? White : Black;
		color::ID	opponent	= opposite(side);
		uint64_t		myPawns	= pawns(side);

		// Check to see if side has one pawn and neither side
		// has any pieces. If so, use the simple pawn evaluation
		// logic.
		if (	m_matCount[side].pawn
			&& !m_matCount[opponent].pawn
			&& m_totalPieces[White] + m_totalPieces[Black] == 0)
		{
			uint64_t	pawns		= myPawns;
			int		sign		= isWhite(side) ? 1 : -1;

			while (pawns)
			{
				sq::ID square	= sq::ID(lsbClear(pawns));
				sq::ID flipped	= flip(square);

				// King must be in front of the pawn or we
				// go no further.
				if (::rank(flip(kingSq(side))) > ::rank(flipped))
				{
					int fyle = ::fyle(square);

					// First a special case. If this is a rook
					// pawn, then the king must be on the adjacent
					// fyle, and be closer to the queening square
					// than the opposing king.
					if (fyle == FyleA)
					{
						sq::ID edge = flip(a8);

						if (	::fyle(m_ksq[side]) == FyleB
							&& sq::distance(kingSq(side), edge) < sq::distance(kingSq(opponent), edge))
						{
							return Score(0, sign*pawn::CanPromote);
						}
					}
					else if (fyle == FyleH)
					{
						sq::ID edge = flip(h8);

						if (	::fyle(kingSq(side)) == FyleG
							&& sq::distance(kingSq(side), edge) < sq::distance(kingSq(opponent), edge))
						{
							return Score(0, sign*pawn::CanPromote);
						}
					}
					else
					{
						// If king is two squares in front of the pawn
						// then it's a win immediately. If the king is
						// on the 6th/3rd rank and closer to the pawn
						// than the opposing king, it's also a win.
						if (	sq::distance(kingSq(side), square) < sq::distance(kingSq(opponent), square)
							&& (	::rank(flip(kingSq(side))) > (::rank(flipped) - 1 + ::mul2(side == Black))
								|| ::rank(kingSq(side)) == (EpRank[opponent])))//XXX ok?
						{
							return Score(0, sign*pawn::CanPromote);
						}

						// Last chance: If the king is one square in
						// front of the pawn and has the opposition,
						// then it's still a win.
						if (	::rank(kingSq(side)) == ::rank(square) - 1 + ::mul2(side == Black)
							&& ::hasOpposition(sideToMove() == side, kingSq(side), kingSq(opponent)))
						{
							return Score(0, sign*pawn::CanPromote);
						}
					}
				}
			}
		}

		// Check to see if enemy is out of pieces and 'side' has
		// passed pawns. If so, see if any of these passed pawns
		// can outrun the defending king and promote.
		if (m_totalPieces[opponent] == 0)
		{
			uint8_t	passed	= eval[side].passedPawn;
			int		lastRank	= HomeRank[opponent];
			int		pawnRank	= PawnRank[side];

			uint64_t const* plus8Dir	= isWhite(side) ? ::Plus8Dir : ::Minus8Dir;
			int (*bitIndex)(uint64_t)	= (isWhite(side)
														? static_cast<int (*)(uint64_t)>(msb)
														: static_cast<int (*)(uint64_t)>(lsb));

			while (passed)
			{
				int		fyle		= lsbClear(passed);
				uint64_t r			= myPawns & FyleMask[fyle];
				sq::ID	square	= sq::ID(bitIndex(r));

				if (!(RaceMask[side][sideToMove()][square]))
				{
					int queenDistance = mstl::abs(lastRank - ::rank(square));

					if (king(side) & plus8Dir[square])
					{
						if (fyle == FyleA || fyle == FyleH)
							queenDistance = 100;
						else
							++queenDistance;
					}

					if (::rank(square) == pawnRank)
						--queenDistance;

					if (queenDistance < queener[side])
					{
						queener[side] = queenDistance;
						promoSq[side] = sq::make(fyle, lastRank);
						passedPawn[side] = square;
						realize[side] = true;
					}
				}
			}

			if (count(eval[side].passedPawn) > 1)
			{
				uint64_t l = myPawns & FyleMask[lsb(eval[side].passedPawn)];
				uint64_t r = myPawns & FyleMask[lsb(eval[side].passedPawn)];

				int left		= bitIndex(l);
				int right	= bitIndex(r);

				if (::fyle(right) - ::fyle(left) > 1)
				{
					if (!(RaceMask[side][sideToMove()][left] & ::setBit(right)))
					{
						int queenDistance = lastRank - ::rank(left);

						if (::rank(left) == pawnRank)
							--queenDistance;

						if (queenDistance < queener[side])
						{
							queener[side] = queenDistance;
							promoSq[side] = sq::make(::fyle(left), lastRank);
							passedPawn[side] = left;
						}
					}

					if (!(RaceMask[side][sideToMove()][right] & ::setBit(left)))
					{
						int queenDistance = lastRank - ::rank(right);

						if (::rank(right) == pawnRank)
							queenDistance--;

						if (queenDistance < queener[side])
						{
							queener[side] = queenDistance;
							promoSq[side] = sq::make(::fyle(right), lastRank);
							passedPawn[side] = right;
						}
					}
				}
			}
		}

		TRACE("%s pawn on %s can promote at %s in %d moves\n",
				printColor(side),
				printSquare(passedPawn[side]),
				printAlgebraic(promoSq[side]),
				queener[side]);
	}

	if (realize[White] && !realize[Black])
	{
		queener[Black] = 8;
		promoSq[Black] = 0;
	}

	if (realize[Black] && !realize[White])
	{
		queener[White] = 8;
		promoSq[White] = 0;
	}

	if (queener[White] == 8 && queener[Black] == 8)
		return 0;

	// Now that we know which pawns can outrun the kings for
	// each side, we need to do the following: If one side
	// queens before the other (two moves or more) then that
	// side wins.

	if (queener[White] < 8 && queener[Black] == 8)
		return Score(0, pawn::CanPromote + (5 - queener[White])*10);

	if (queener[Black] < 8 && queener[White] == 8)
		return Score(0, -(pawn::CanPromote + (5 - queener[White])*10));

	if (blackToMove())
		--queener[Black];

	if (queener[White] < queener[Black])
		return Score(0, pawn::CanPromote + (5 - queener[White])*10);

	if (queener[Black] < queener[White] - 1)
		return Score(0, -(pawn::CanPromote + (5 - queener[White])*10));

	if (queener[White] < 8 || queener[Black] < 8)
		return 0;

	// If the white pawn queens one move before black, then
	// if the new queen checks the black king, or the new
	// queen attacks the queening square of black, white wins
	// unless the black king is protecting the black queening
	// square in which case it's a draw.
	if (queener[White] == queener[Black])
	{
		uint64_t occupied = (whitePieces() & ~::setBit(passedPawn[White])) | ::setBit(promoSq[White])
								| (blackPieces() & ~::setBit(passedPawn[Black])) | ::setBit(promoSq[Black]);

		if (	!(::Obstructed[kingSq(Black)][promoSq[White]] & occupied)
			|| (	!(::Obstructed[promoSq[Black]][promoSq[White]] & occupied)
				&& (kingAttacks(promoSq[Black] & king(Black)))))
		{
			return Score(0, pawn::CanPromote + (5 - queener[White])*10);
		}
	}

	// If the black pawn queens one move before white, then
	// if the new queen checks the white king, or the new
	// queen attacks the queening square of white, black wins
	// unless the white king is protecting the white queening
	// square in which case it's a draw.
	if (queener[Black] == queener[White] - 1)
	{
		uint64_t occupied = (whitePieces() & ~::setBit(passedPawn[White])) | ::setBit(promoSq[White])
								| (blackPieces() & ~::setBit(passedPawn[Black])) | ::setBit(promoSq[Black]);

		if (	!(::Obstructed[kingSq(White)][promoSq[Black]] & occupied)
			|| (	!(::Obstructed[promoSq[White]][promoSq[Black]] & occupied)
				&& (kingAttacks(promoSq[White] & king(White)))))
		{
			return Score(0, -(pawn::CanPromote + (5 - queener[Black])*10));
		}
	}

	return 0;
}


#if 0

static void
__attribute__((constructor))
initialize()
{
	// init king safety
	for (int safety = 0; safety < 16; ++safety)
	{
		for (int tropism = 0; tropism < 16; ++tropism)
		{
			safety::KingSafety[safety][tropism] =
				180*(((safety::Safety[safety] + 100)*(safety::Tropism[tropism] + 100))/100 - 100)/100;
		}
	}
}

#endif

// vi:set ts=3 sw=3:
