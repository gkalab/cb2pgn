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

#include "db_common.h"

namespace db {
namespace board {

#define BIT(n) (uint64_t(1) << sq::n)
static uint64_t const
	A1 = BIT(a1), B1 = BIT(b1), C1 = BIT(c1), D1 = BIT(d1),
	E1 = BIT(e1), F1 = BIT(f1), G1 = BIT(g1), H1 = BIT(h1),
	A2 = BIT(a2), B2 = BIT(b2), C2 = BIT(c2), D2 = BIT(d2),
	E2 = BIT(e2), F2 = BIT(f2), G2 = BIT(g2), H2 = BIT(h2),
	A3 = BIT(a3), B3 = BIT(b3), C3 = BIT(c3), D3 = BIT(d3),
	E3 = BIT(e3), F3 = BIT(f3), G3 = BIT(g3), H3 = BIT(h3),
	A4 = BIT(a4), B4 = BIT(b4), C4 = BIT(c4), D4 = BIT(d4),
	E4 = BIT(e4), F4 = BIT(f4), G4 = BIT(g4), H4 = BIT(h4),
	A5 = BIT(a5), B5 = BIT(b5), C5 = BIT(c5), D5 = BIT(d5),
	E5 = BIT(e5), F5 = BIT(f5), G5 = BIT(g5), H5 = BIT(h5),
	A6 = BIT(a6), B6 = BIT(b6), C6 = BIT(c6), D6 = BIT(d6),
	E6 = BIT(e6), F6 = BIT(f6), G6 = BIT(g6), H6 = BIT(h6),
	A7 = BIT(a7), B7 = BIT(b7), C7 = BIT(c7), D7 = BIT(d7),
	E7 = BIT(e7), F7 = BIT(f7), G7 = BIT(g7), H7 = BIT(h7),
	A8 = BIT(a8), B8 = BIT(b8), C8 = BIT(c8), D8 = BIT(d8),
	E8 = BIT(e8), F8 = BIT(f8), G8 = BIT(g8), H8 = BIT(h8);
#undef BIT

static uint64_t const FyleMaskA = A1 | A2 | A3 | A4 | A5 | A6 | A7 | A8;
static uint64_t const FyleMaskB = B1 | B2 | B3 | B4 | B5 | B6 | B7 | B8;
static uint64_t const FyleMaskC = C1 | C2 | C3 | C4 | C5 | C6 | C7 | C8;
static uint64_t const FyleMaskD = D1 | D2 | D3 | D4 | D5 | D6 | D7 | D8;
static uint64_t const FyleMaskE = E1 | E2 | E3 | E4 | E5 | E6 | E7 | E8;
static uint64_t const FyleMaskF = F1 | F2 | F3 | F4 | F5 | F6 | F7 | F8;
static uint64_t const FyleMaskG = G1 | G2 | G3 | G4 | G5 | G6 | G7 | G8;
static uint64_t const FyleMaskH = H1 | H2 | H3 | H4 | H5 | H6 | H7 | H8;

static uint64_t const RankMask1 = A1 | B1 | C1 | D1 | E1 | F1 | G1 | H1;
static uint64_t const RankMask2 = A2 | B2 | C2 | D2 | E2 | F2 | G2 | H2;
static uint64_t const RankMask3 = A3 | B3 | C3 | D3 | E3 | F3 | G3 | H3;
static uint64_t const RankMask4 = A4 | B4 | C4 | D4 | E4 | F4 | G4 | H4;
static uint64_t const RankMask5 = A5 | B5 | C5 | D5 | E5 | F5 | G5 | H5;
static uint64_t const RankMask6 = A6 | B6 | C6 | D6 | E6 | F6 | G6 | H6;
static uint64_t const RankMask7 = A7 | B7 | C7 | D7 | E7 | F7 | G7 | H7;
static uint64_t const RankMask8 = A8 | B8 | C8 | D8 | E8 | F8 | G8 | H8;

static uint64_t const FyleMask[8] =
{
	FyleMaskA, FyleMaskB, FyleMaskC, FyleMaskD, FyleMaskE, FyleMaskF, FyleMaskG, FyleMaskH
};
static uint64_t const RankMask[8] =
{
	RankMask1, RankMask2, RankMask3, RankMask4, RankMask5, RankMask6, RankMask7, RankMask8
};

static unsigned const ShiftR45[64] =
{
	 1, 58, 51, 44, 37, 30, 23, 16,
	 9,  1, 58, 51, 44, 37, 30, 23,
	17,  9,  1, 58, 51, 44, 37, 30,
	25, 17,  9,  1, 58, 51, 44, 37,
	33, 25, 17,  9,  1, 58, 51, 44,
	41, 33, 25, 17,  9,  1, 58, 51,
	49, 41, 33, 25, 17,  9,  1, 58,
	57, 49, 41, 33, 25, 17,  9,  1,
};

static unsigned const ShiftL45[64] =
{
	 9, 17, 25, 33, 41, 49, 57,  1,
	17, 25, 33, 41, 49, 57,  1, 10,
	25, 33, 41, 49, 57,  1, 10, 19,
	33, 41, 49, 57,  1, 10, 19, 28,
	41, 49, 57,  1, 10, 19, 28, 37,
	49, 57,  1, 10, 19, 28, 37, 46,
	57,  1, 10, 19, 28, 37, 46, 55,
	 1, 10, 19, 28, 37, 46, 55, 64,
};

extern uint64_t HomeRankMask[2];
extern uint64_t PawnRankMask[2];

extern Byte NextRank[2][64];
extern Byte PrevRank[2][64];

extern sq::Rank EpRank[2];
extern sq::Rank HomeRank[2];
extern sq::Rank PawnRank[2];

extern uint64_t Plus1Dir[64];
extern uint64_t Plus7Dir[64];
extern uint64_t Plus8Dir[64];
extern uint64_t Plus9Dir[64];

extern uint64_t Minus1Dir[64];
extern uint64_t Minus7Dir[64];
extern uint64_t Minus8Dir[64];
extern uint64_t Minus9Dir[64];

extern int8_t Directions[64][64];

extern uint64_t Obstructed[64][64];
extern uint64_t PawnAttacks[2][64];
extern uint64_t KnightAttacks[64];
extern uint64_t R45Attacks[64][64];
extern uint64_t L45Attacks[64][64];
extern uint64_t KingAttacks[64];
extern uint64_t RankAttacks[64][64];
extern uint64_t FyleAttacks[64][64];
extern uint64_t PawnF1[2][64];
extern uint64_t PawnF2[2][64];
extern uint64_t MaskL90[64];
extern uint64_t MaskL45[64];
extern uint64_t MaskR45[64];

extern uint64_t MaskIsolatedPawn[64];
extern uint64_t MaskDuoPawn[64];
extern uint64_t MaskPassedPawn[2][64];
extern uint64_t MaskPassedPawnConnected[64];
extern uint64_t MaskNoAttacks[2][64];
extern uint64_t MaskProtectedPawn[2][64];
extern uint64_t MaskHiddenLeft[2][8];
extern uint64_t MaskHiddenRight[2][8];
extern uint64_t RaceMask[2][2][64];

extern uint8_t IsOutside[256];


uint64_t setBit(int s);

uint64_t shiftDown(uint64_t m);
uint64_t shift2Down(uint64_t m);
uint64_t shiftUp(uint64_t m);
uint64_t shift2Up(uint64_t m);
uint64_t shiftUpLeft(uint64_t m);
uint64_t shiftUpRight(uint64_t m);
uint64_t shiftDownLeft(uint64_t m);
uint64_t shiftDownRight(uint64_t m);
uint64_t shiftLeft(uint64_t m);
uint64_t shiftRight(uint64_t m);
uint64_t shift2Left(uint64_t m);
uint64_t shift2Right(uint64_t m);

int lsb(uint8_t n);
int lsb(uint64_t n);

int msb(uint8_t n);
int msb(uint64_t n);

template <typename T> int count(T n);

int lsbClear(uint8_t& n);
int lsbClear(uint64_t& n);

namespace base { void initialize(); }

} // namespace board
} // namespace db

#include "db_board_base.ipp"

// vi:set ts=3 sw=3:
