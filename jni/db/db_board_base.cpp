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

#include "db_board_base.h"

#include <string.h>

using namespace db;
using namespace db::sq;
using namespace db::color;
using namespace db::board;

using mstl::abs;


Byte board::NextRank[2][64];
Byte board::PrevRank[2][64];

int8_t board::Directions[64][64];

sq::Rank board::EpRank[2];
sq::Rank board::HomeRank[2];
sq::Rank board::PawnRank[2];

uint64_t board::PawnRankMask[2];
uint64_t board::FyleAttacks[64][64];
uint64_t board::HomeRankMask[2];
uint64_t board::KingAttacks[64];
uint64_t board::KnightAttacks[64];
uint64_t board::L45Attacks[64][64];
uint64_t board::MaskDuoPawn[64];
uint64_t board::MaskHiddenLeft[2][8];
uint64_t board::MaskHiddenRight[2][8];
uint64_t board::MaskIsolatedPawn[64];
uint64_t board::MaskL45[64];
uint64_t board::MaskL90[64];
uint64_t board::MaskNoAttacks[2][64];
uint64_t board::MaskPassedPawnConnected[64];
uint64_t board::MaskPassedPawn[2][64];
uint64_t board::MaskProtectedPawn[2][64];
uint64_t board::MaskR45[64];
uint64_t board::Minus1Dir[64];
uint64_t board::Minus7Dir[64];
uint64_t board::Minus8Dir[64];
uint64_t board::Minus9Dir[64];
uint64_t board::PawnAttacks[2][64];
uint64_t board::PawnF1[2][64];
uint64_t board::PawnF2[2][64];
uint64_t board::Plus1Dir[64];
uint64_t board::Plus7Dir[64];
uint64_t board::Plus8Dir[64];
uint64_t board::Plus9Dir[64];
uint64_t board::R45Attacks[64][64];
uint64_t board::RaceMask[2][2][64];
uint64_t board::RankAttacks[64][64];
uint64_t board::Obstructed[64][64];

uint8_t board::IsOutside[256];


static void __attribute__((constructor)) initialize() { board::base::initialize(); }


template <typename T> inline static T div8(T x) { return x >> 3; }
template <typename T> inline static T mod8(T x) { return x & 7; }


static bool
kingPawnSquare(sq::ID pawn, sq::ID king, sq::ID queen, bool toMove)
{
	int rankd = sq::rankDistance(king, queen);
	int fyled = sq::fyleDistance(king, queen);

	int pdist = sq::rankDistance(pawn, queen);
	int kdist = rankd > fyled ? rankd : fyled;

	return (toMove ? pdist : pdist + 1) >= kdist;
}


void
board::base::initialize()
{
	static Byte const RotateL90[64] =
	{
		h1, h2, h3, h4, h5, h6, h7, h8,
		g1, g2, g3, g4, g5, g6, g7, g8,
		f1, f2, f3, f4, f5, f6, f7, f8,
		e1, e2, e3, e4, e5, e6, e7, e8,
		d1, d2, d3, d4, d5, d6, d7, d8,
		c1, c2, c3, c4, c5, c6, c7, c8,
		b1, b2, b3, b4, b5, b6, b7, b8,
		a1, a2, a3, a4, a5, a6, a7, a8,
	};

	static Byte const RotateR45[64] =
	{
		a1, b8, c7, d6, e5, f4, g3, h2,
		a2, b1, c8, d7, e6, f5, g4, h3,
		a3, b2, c1, d8, e7, f6, g5, h4,
		a4, b3, c2, d1, e8, f7, g6, h5,
		a5, b4, c3, d2, e1, f8, g7, h6,
		a6, b5, c4, d3, e2, f1, g8, h7,
		a7, b6, c5, d4, e3, f2, g1, h8,
		a8, b7, c6, d5, e4, f3, g2, h1
	};

	static Byte const RotateL45[64] =
	{
		a2, b3, c4, d5, e6, f7, g8, h1,
		a3, b4, c5, d6, e7, f8, g1, h2,
		a4, b5, c6, d7, e8, f1, g2, h3,
		a5, b6, c7, d8, e1, f2, g3, h4,
		a6, b7, c8, d1, e2, f3, g4, h5,
		a7, b8, c1, d2, e3, f4, g5, h6,
		a8, b1, c2, d3, e4, f5, g6, h7,
		a1, b2, c3, d4, e5, f6, g7, h8
	};

	// Square masks
	for (int i = 0; i < 64; ++i)
	{
		// Square masks
		MaskL90[i] = setBit(RotateL90[i]);
		MaskL45[i] = setBit(RotateL45[i]);
		MaskR45[i] = setBit(RotateR45[i]);

		// Pawn moves and attacks
		uint64_t mask = setBit(i);

		PawnAttacks[White][i]  = shiftUpLeft(mask);
		PawnAttacks[White][i] |= shiftUpRight(mask);

		PawnAttacks[Black][i]  = shiftDownLeft(mask);
		PawnAttacks[Black][i] |= shiftDownRight(mask);

		PawnF1[White][i] = shiftUp(mask);
		PawnF2[White][i] = shift2Up(mask) & RankMask4;

		PawnF1[Black][i] = shiftDown(mask);
		PawnF2[Black][i] = shift2Down(mask) & RankMask5;

		// Knight attacks
		KnightAttacks[i]  = shiftLeft(shift2Up(mask))
								| shiftRight(shift2Up(mask))
								| shiftLeft(shift2Down(mask))
								| shiftRight(shift2Down(mask))
								| shift2Left(shiftUp(mask))
								| shift2Right(shiftUp(mask))
								| shift2Left(shiftDown(mask))
								| shift2Right(shiftDown(mask));

		// King attacks
		KingAttacks[i] = shiftLeft(mask)
							| shiftRight(mask)
							| shiftUp(mask)
							| shiftDown(mask)
							| shiftUpLeft(mask)
							| shiftUpRight(mask)
							| shiftDownLeft(mask)
							| shiftDownRight(mask);
	}

	// Diagonal attacks
	for (unsigned s = 0; s < 64; ++s)
	{
		for (unsigned b = 0; b < 64; ++b)
		{
			int		q		= s;
			uint64_t	mask	= 0;

			while (fyle(q) > 0 && rank(q) < 7)
			{
				q += 7;
				mask |= setBit(q);

				if (b & (setBit(RotateL45[q]) >> ShiftL45[s]))
					break;
			}

			q = s;

			while (fyle(q) < 7 && rank(q) > 0)
			{
				q -= 7;
				mask |= setBit(q);

				if (b & (setBit(RotateL45[q]) >> ShiftL45[s]))
					break;
			}

			L45Attacks[s][b] = mask;
			mask = 0;
			q = s;

			while (fyle(q) < 7 && rank(q) < 7)
			{
				q += 9;
				mask |= setBit(q);

				if (b & (setBit(RotateR45[q]) >> ShiftR45[s]))
					break;
			}

			q = s;

			while (fyle(q) > 0 && rank(q) > 0)
			{
				q -= 9;
				mask |= setBit(q);

				if (b & (setBit(RotateR45[q]) >> ShiftR45[s]))
					break;
			}

			R45Attacks[s][b] = mask;
		}
	}

	// Rank and Fyle attacks
	memset(RankAttacks, 0, sizeof(RankAttacks));
	memset(FyleAttacks, 0, sizeof(FyleAttacks));

	for (unsigned sq = 0; sq < 64; ++sq)
	{
		for (unsigned bitrow = 0; bitrow < 64; ++bitrow)
		{
			int fyle	= ::fyle(sq);
			int q		= sq + 1;

			while (++fyle < 8)
			{
				RankAttacks[sq][bitrow] |= setBit(q);

				if ((1 << fyle) & (bitrow << 1))
					break;

				++q;
			}

			fyle = ::fyle(sq);
			q = sq - 1;

			while (--fyle >= 0)
			{
				RankAttacks[sq][bitrow] |= setBit(q);

				if ((1 << fyle) & (bitrow << 1))
					break;

				--q;
			}

			int rank = ::rank(sq);
			q = sq + 8;

			while (++rank < 8)
			{
				FyleAttacks[sq][bitrow] |= setBit(q);

				if ((1 << (7 - rank)) & (bitrow << 1))
					break;

				q += 8;
			}

			rank = ::rank(sq);
			q = sq - 8;

			while (--rank >= 0)
			{
				FyleAttacks[sq][bitrow] |= setBit(q);

				if ((1 << (7 - rank)) & (bitrow << 1))
					break;

				q -= 8;
			}
		}
	}

	// init directions
	static int RookSq[4]		= { -8, -1, 1, 8 };
	static int BishopSq[4]	= { -9, -7, 7, 9 };

	memset(Plus1Dir, 0, sizeof(Plus1Dir));
	memset(Plus7Dir, 0, sizeof(Plus7Dir));
	memset(Plus8Dir, 0, sizeof(Plus8Dir));
	memset(Plus9Dir, 0, sizeof(Plus9Dir));
	memset(Minus1Dir, 0, sizeof(Minus1Dir));
	memset(Minus7Dir, 0, sizeof(Minus7Dir));
	memset(Minus8Dir, 0, sizeof(Minus8Dir));
	memset(Minus9Dir, 0, sizeof(Minus9Dir));

	for (unsigned i = 0; i < 64; ++i)
	{
		for (unsigned j = 0; j < 4; ++j)
		{
			int lastSq	= i;
			int dir		= RookSq[j];
			int sq		= i + dir;

			while (	sq >= 0
					&& sq < 64
					&& (	((abs(div8(sq) - div8(lastSq)) == 1) && (abs(mod8(sq) - mod8(lastSq)) == 0))
						|| ((abs(div8(sq) - div8(lastSq)) == 0) && (abs(mod8(sq) - mod8(lastSq)) == 1))))
			{
				switch (dir)
				{
					case  1: Plus1Dir[i] |= setBit(sq); break;
					case  8: Plus8Dir[i] |= setBit(sq); break;
					case -1: Minus1Dir[i] |= setBit(sq); break;
					case -8: Minus8Dir[i] |= setBit(sq); break;
				}

				lastSq = sq;
				sq += dir;
			}

			dir = BishopSq[j];
			lastSq = i;
			sq = i + dir;

			while (	sq >= 0
					&& sq < 64
					&& abs(div8(sq) - div8(lastSq)) == 1
					&& abs(mod8(sq) - mod8(lastSq)) == 1)
			{
				switch (dir)
				{
					case  7: Plus7Dir[i] |= setBit(sq); break;
					case  9: Plus9Dir[i] |= setBit(sq); break;
					case -7: Minus7Dir[i] |= setBit(sq); break;
					case -9: Minus9Dir[i] |= setBit(sq); break;
				}

				lastSq = sq;
				sq += dir;
			}
		}
	}

	// Directions[sq1][sq2] gives the "move direction" to move from
	// sq1 to sq2. Obstructed[sq1][sq2] gives a bit vector that indicates
	// which squares must be unoccupied in order for <sq1> to attack <sq2>,
	// assuming a sliding piece is involved. To use this, you simply have
	// to 'Obstructed[sq1][sq2] | occupiedSquares)' and if the result is
	// "0" then a sliding piece on sq1 would attack sq2 and vice-versa.
	memset(Directions, 0, sizeof(Directions));
	memset(Obstructed, 0xff, sizeof(Obstructed));

	for (unsigned i = 0; i < 64; ++i)
	{
		uint64_t squares;

		squares = Plus1Dir[i];
		while (squares)
		{
			unsigned j = lsbClear(squares);
			Directions[i][j] = 1;
			Obstructed[i][j] = Plus1Dir[i] ^ Plus1Dir[j - 1];
		}

		squares = Plus7Dir[i];
		while (squares)
		{
			unsigned j = lsbClear(squares);
			Directions[i][j] = 7;
			Obstructed[i][j] = Plus7Dir[i] ^ Plus7Dir[j - 7];
		}

		squares = Plus8Dir[i];
		while (squares)
		{
			unsigned j = lsbClear(squares);
			Directions[i][j] = 8;
			Obstructed[i][j] = Plus8Dir[i] ^ Plus8Dir[j - 8];
		}

		squares = Plus9Dir[i];
		while (squares)
		{
			unsigned j = lsbClear(squares);
			Directions[i][j] = 9;
			Obstructed[i][j] = Plus9Dir[i] ^ Plus9Dir[j - 9];
		}

		squares = Minus1Dir[i];
		while (squares)
		{
			unsigned j = lsbClear(squares);
			Directions[i][j] = -1;
			Obstructed[i][j] = Minus1Dir[i] ^ Minus1Dir[j + 1];
		}

		squares = Minus7Dir[i];
		while (squares)
		{
			unsigned j = lsbClear(squares);
			Directions[i][j] = -7;
			Obstructed[i][j] = Minus7Dir[i] ^ Minus7Dir[j + 7];
		}

		squares = Minus8Dir[i];
		while (squares)
		{
			unsigned j = lsbClear(squares);
			Directions[i][j] = -8;
			Obstructed[i][j] = Minus8Dir[i] ^ Minus8Dir[j + 8];
		}

		squares = Minus9Dir[i];
		while (squares)
		{
			unsigned j = lsbClear(squares);
			Directions[i][j] = -9;
			Obstructed[i][j] = Minus9Dir[i] ^ Minus9Dir[j + 9];
		}
	}

	// init pawn attacks
	memset(MaskNoAttacks, 0, sizeof(MaskNoAttacks));

	for (unsigned i = 8; i < 56; ++i)
	{
		switch (fyle(i))
		{
			case 0:
				MaskNoAttacks[White][i] = Minus8Dir[i + 1];
				MaskNoAttacks[Black][i] = Plus8Dir[i + 1];
				break;

			case 7:
				MaskNoAttacks[White][i] = Minus8Dir[i - 1];
				MaskNoAttacks[Black][i] = Plus8Dir[i - 1];
				break;

			default:
				MaskNoAttacks[White][i] = Minus8Dir[i - 1] | Minus8Dir[i + 1];
				MaskNoAttacks[Black][i] = Plus8Dir[i - 1] | Plus8Dir[i + 1];
				break;
		}
	}

	// init pawn masks
	for (unsigned i = 0; i < 64;++i)
	{
		switch (fyle(i))
		{
			case 0:
				MaskIsolatedPawn[i] = FyleMask[1];
				MaskPassedPawn[White][i] = Plus8Dir[i] | Plus8Dir[i + 1];
				MaskPassedPawn[Black][i] = Minus8Dir[i] | Minus8Dir[i + 1];
				break;

			case 7:
				MaskIsolatedPawn[i] = FyleMask[6];
				MaskPassedPawn[White][i] = Plus8Dir[i - 1] | Plus8Dir[i];
				MaskPassedPawn[Black][i] = Minus8Dir[i - 1] | Minus8Dir[i];
				break;

			default:
				MaskIsolatedPawn[i] = FyleMask[fyle(i) - 1] | FyleMask[fyle(i) + 1];
				MaskPassedPawn[White][i] = Plus8Dir[i - 1] | Plus8Dir[i] | Plus8Dir[i + 1];
				MaskPassedPawn[Black][i] = Minus8Dir[i - 1] | Minus8Dir[i] | Minus8Dir[i + 1];
				break;
		}
	}

	memset(MaskProtectedPawn, 0, sizeof(MaskProtectedPawn));

	for (unsigned i = 0; i < 56; ++i)
	{
		MaskProtectedPawn[White][i] = setBit(i - 1) | setBit(i + 1);
		MaskProtectedPawn[Black][i] = setBit(i - 1) | setBit(i + 1);

		if (i > 15)
			MaskProtectedPawn[White][i] |= setBit(i - 7) | setBit(i - 9);

		if (i < 48)
			MaskProtectedPawn[Black][i] |= setBit(i + 7) | setBit(i + 9);
	}

	memset(MaskDuoPawn, 0, sizeof(MaskDuoPawn));
	memset(MaskPassedPawnConnected, 0, sizeof(MaskPassedPawnConnected));

	for (unsigned i = 8; i < 56; ++i)
	{
		switch (fyle(i))
		{
			case 0:
				MaskDuoPawn[i] = setBit(i + 1);
				MaskPassedPawnConnected[i] = setBit(i + 1) | setBit(i - 7) | setBit(i + 9);
				break;

			case 7:
				MaskDuoPawn[i] = setBit(i - 1);
				MaskPassedPawnConnected[i] = setBit(i - 1) | setBit(i - 9) | setBit(i + 7);
				break;

			default:
				MaskDuoPawn[i] = setBit(i - 1) | setBit(i + 1);
				MaskPassedPawnConnected[i] = setBit(i - 1) | setBit(i + 1)
															| setBit(i - 9) | setBit(i - 7)
															| setBit(i + 9) | setBit(i + 7);
				break;
		}
	}

	memset(MaskHiddenLeft, 0, sizeof(MaskHiddenLeft));
	memset(MaskHiddenRight, 0, sizeof(MaskHiddenRight));

	for (unsigned i = 0; i < 8; ++i)
	{
		if (i > 0)
		{
			MaskHiddenLeft[White][i] |= setBit(39 + i) | setBit(47 + i);
			MaskHiddenLeft[Black][i] |= setBit(15 + i) | setBit( 7 + i);
		}
		if (i > 1)
		{
			MaskHiddenLeft[White][i] |= setBit(46 + i) | setBit(38 + i);
			MaskHiddenLeft[Black][i] |= setBit( 6 + i) | setBit(14 + i);
		}
		if (i < 6)
		{
			MaskHiddenRight[White][i] |= setBit(50 + i) | setBit(42 + i);
			MaskHiddenRight[Black][i] |= setBit(10 + i) | setBit(18 + i);
		}
		if (i < 7)
		{
			MaskHiddenRight[White][i] |= setBit(41 + i) | setBit(49 + i);
			MaskHiddenRight[Black][i] |= setBit(17 + i) | setBit( 9 + i);
		}
	}

	// Argument must have left-most or right-most bit set when compared to
	// mask. And this bit must be separated from the next bit by at least
	// one fyle (ie the outside passed pawn is 2 files from the rest of the
	// pawns, at least.
	IsOutside[0] = 0;

	for (unsigned i = 1; i < 256; ++i)
	{
		int ppSq1 = lsb(uint8_t(i));	// square that contains a (potential) passed pawn
		int ppSq2 = msb(uint8_t(i));	// dito

		IsOutside[i] = 0xff;

		for (unsigned j = 1; j < 256; ++j)
		{
			int pSqL = lsb(uint8_t(j));	// leftmost pawn
			int pSqR = msb(uint8_t(j));	// rightmost pawn

			if (pSqL < ppSq1 - 1)
				IsOutside[i] &= ~(1 << pSqL);

			if (ppSq2 > pSqR + 1)
				IsOutside[i] &= ~(1 << pSqR);
		}
	}

	// Initialize masks used to evaluate pawn races. These masks are
	// used to determine if the opposing king is in a position to stop a
	// passed pawn from racing down and queening.
	memset(RaceMask, 0, sizeof(RaceMask));

	for (unsigned j = 8; j < 56; ++j)
	{
		for (unsigned i = 0; i < 64; ++i)
		{
			// white pawn, wtm
			if (j < 16)
			{
				if (kingPawnSquare(sq::ID(j + 8), sq::ID(i), sq::ID(fyle(j) + 56), true))
					RaceMask[White][White][j] |= setBit(i);
			}
			else
			{
				if (kingPawnSquare(sq::ID(j), sq::ID(i), sq::ID(fyle(j) + 56), true))
					RaceMask[White][White][j] |= setBit(i);
			}
			// white pawn, btm
			if (j < 16)
			{
				if (kingPawnSquare(sq::ID(j + 8), sq::ID(i), sq::ID(fyle(j) + 56), false))
					RaceMask[White][Black][j] |= setBit(i);
			}
			else
			{
				if (kingPawnSquare(sq::ID(j), sq::ID(i), sq::ID(fyle(j) + 56), false))
					RaceMask[White][Black][j] |= setBit(i);
			}
			// black pawn, wtm
			if (j > 47)
			{
				if (kingPawnSquare(sq::ID(j - 8), sq::ID(i), sq::ID(fyle(j)), false))
					RaceMask[Black][White][j] |= setBit(i);
			}
			else
			{
				if (kingPawnSquare(sq::ID(j), sq::ID(i), sq::ID(fyle(j)), false))
					RaceMask[Black][White][j] |= setBit(i);
			}
			// black pawn, btm
			if (j > 47)
			{
				if (kingPawnSquare(sq::ID(j - 8), sq::ID(i), sq::ID(fyle(j)), true))
					RaceMask[Black][Black][j] |= setBit(i);
			}
			else
			{
				if (kingPawnSquare(sq::ID(j), sq::ID(i), sq::ID(fyle(j)), true))
					RaceMask[Black][Black][j] |= setBit(i);
			}
		}
	}

	// Ranks
	HomeRankMask[White] = RankMask1;
	HomeRankMask[Black] = RankMask8;
	PawnRankMask[White] = RankMask2;
	PawnRankMask[Black] = RankMask7;

	HomeRank[White] = Rank1;
	HomeRank[Black] = Rank8;
	PawnRank[White] = Rank2;
	PawnRank[Black] = Rank7;

	EpRank[White] = Rank6;
	EpRank[Black] = Rank3;

	for (unsigned i = 0; i < 64; ++i)
	{
		NextRank[White][i] = PrevRank[Black][i] = (i + 8) & 63;
		NextRank[Black][i] = PrevRank[White][i] = (i - 8) & 63;
	}
}

// vi:set ts=3 sw=3:
