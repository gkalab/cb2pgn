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

#include "cbh_decoder.h"

#include "db_consumer.h"
#include "db_game_data.h"
#include "db_game_info.h"
#include "db_database_codec.h"
#include "db_move_node.h"
#include "db_mark_set.h"
#include "db_annotation.h"
#include "db_exception.h"

#include "u_byte_stream.h"
#include "u_bit_stream.h"

#include "m_vector.h"
#include "m_limits.h"
#include "m_stdio.h"

#include <ctype.h>

using namespace util;
using namespace db;
using namespace db::cbh;


enum { Move, Push, Pop, Skip };

static int const MaxMoveNo = mstl::numeric_limits<int>::max();


# define ____ 0
static char const* LangMap[] =
{
	____, ____, ____, ____, ____, ____, ____, ____, // x00 - x07
	____, ____, ____, ____, ____, ____, ____, ____, // x08 - x0f
	____, ____, ____, ____, ____, ____, ____, ____, // x10 - x17
	____, ____, ____, ____, ____, ____, ____, ____, // x18 - x1f
	____, ____, ____, ____, ____, ____, ____, ____, // x20 - x27
	____, ____, "en", "es", ____, ____, ____, ____, // x28 - x2f
	____, "fr", ____, ____, ____, "de", ____, ____, // x30 - x37
	____, ____, ____, ____, ____, ____, ____, ____, // x38 - x3f
	____, ____, ____, ____, ____, ____, "it", ____, // x40 - x47
	____, ____, ____, ____, ____, ____, ____, ____, // x48 - x4f
	____, ____, ____, ____, ____, ____, ____, ____, // x50 - x57
	____, ____, ____, ____, ____, ____, ____, ____, // x58 - x5f
	____, ____, ____, ____, ____, ____, ____, "nl", // x60 - x67
	____, ____, ____, ____, ____, ____, ____, ____, // x68 - x6f
	____, ____, ____, ____, "pl", "pt", ____, ____, // x70 - x77
	____, ____, ____, ____, ____, ____, ____, ____, // x78 - x7f
};
#undef ____


inline
static Byte
mapSquare(Byte sq)
{
	return sq::make(sq >> 3, sq & 7);
}


static mark::Color
mapColor(Byte c)
{
	switch (c)
	{
		case 2: return mark::Green;
		case 3: return mark::Yellow;
		case 4: return mark::Red;
	}

	return Mark::DefaultColor;
}


static Byte MoveNumberLookup[256] =
{
	0xa2, 0x95, 0x43, 0xf5, 0xc1, 0x3d, 0x4a, 0x6c,	//   0 -   7
	0x53, 0x83, 0xcc, 0x7c, 0xff, 0xae, 0x68, 0xad,	//   8 -  15
	0xd1, 0x92, 0x8b, 0x8d, 0x35, 0x81, 0x5e, 0x74,	//  16 -  23
	0x26, 0x8e, 0xab, 0xca, 0xfd, 0x9a, 0xf3, 0xa0,	//  24 -  31
	0xa5, 0x15, 0xfc, 0xb1, 0x1e, 0xed, 0x30, 0xea,	//  32 -  39
	0x22, 0xeb, 0xa7, 0xcd, 0x4e, 0x6f, 0x2e, 0x24,	//  40 -  47
	0x32, 0x94, 0x41, 0x8c, 0x6e, 0x58, 0x82, 0x50,	//  48 -  55
	0xbb, 0x02, 0x8a, 0xd8, 0xfa, 0x60, 0xde, 0x52,	//  56 -  63
	0xba, 0x46, 0xac, 0x29, 0x9d, 0xd7, 0xdf, 0x08,	//  64 -  71
	0x21, 0x01, 0x66, 0xa3, 0xf1, 0x19, 0x27, 0xb5,	//  72 -  79
	0x91, 0xd5, 0x42, 0x0e, 0xb4, 0x4c, 0xd9, 0x18,	//  80 -  87
	0x5f, 0xbc, 0x25, 0xa6, 0x96, 0x04, 0x56, 0x6a,	//  88 -  95
	0xaa, 0x33, 0x1c, 0x2b, 0x73, 0xf0, 0xdd, 0xa4,	//  96 - 103
	0x37, 0xd3, 0xc5, 0x10, 0xbf, 0x5a, 0x23, 0x34,	// 104 - 111
	0x75, 0x5b, 0xb8, 0x55, 0xd2, 0x6b, 0x09, 0x3a,	// 112 - 119
	0x57, 0x12, 0xb3, 0x77, 0x48, 0x85, 0x9b, 0x0f,	// 120 - 127
	0x9e, 0xc7, 0xc8, 0xa1, 0x7f, 0x7a, 0xc0, 0xbd,	// 128 - 135
	0x31, 0x6d, 0xf6, 0x3e, 0xc3, 0x11, 0x71, 0xce,	// 136 - 143
	0x7d, 0xda, 0xa8, 0x54, 0x90, 0x97, 0x1f, 0x44,	// 144 - 151
	0x40, 0x16, 0xc9, 0xe3, 0x2c, 0xcb, 0x84, 0xec,	// 152 - 159
	0x9f, 0x3f, 0x5c, 0xe6, 0x76, 0x0b, 0x3c, 0x20,	// 160 - 167
	0xb7, 0x36, 0x00, 0xdc, 0xe7, 0xf9, 0x4f, 0xf7,	// 168 - 175
	0xaf, 0x06, 0x07, 0xe0, 0x1a, 0x0a, 0xa9, 0x4b,	// 176 - 183
	0x0c, 0xd6, 0x63, 0x87, 0x89, 0x1d, 0x13, 0x1b,	// 184 - 191
	0xe4, 0x70, 0x05, 0x47, 0x67, 0x7b, 0x2f, 0xee,	// 192 - 199
	0xe2, 0xe8, 0x98, 0x0d, 0xef, 0xcf, 0xc4, 0xf4,	// 200 - 207
	0xfb, 0xb0, 0x17, 0x99, 0x64, 0xf2, 0xd4, 0x2a,	// 208 - 215
	0x03, 0x4d, 0x78, 0xc6, 0xfe, 0x65, 0x86, 0x88,	// 216 - 223
	0x79, 0x45, 0x3b, 0xe5, 0x49, 0x8f, 0x2d, 0xb9,	// 224 - 231
	0xbe, 0x62, 0x93, 0x14, 0xe9, 0xd0, 0x38, 0x9c,	// 232 - 239
	0xb2, 0xc2, 0x59, 0x5d, 0xb6, 0x72, 0x51, 0xf8,	// 240 - 247
	0x28, 0x7e, 0x61, 0x39, 0xe1, 0xdb, 0x69, 0x80,	// 248 - 255
};


#if 0
#define ____ 0xff
static Byte MoveNumberLookup960[256] =
{
	____, ____, ____, ____, ____, ____, 0x6f, 0x73,	//   0 -   7
	____, ____, ____, ____, ____, ____, ____, ____,	//   8 -  15
	____, ____, ____, ____, ____, ____, ____, ____,	//  16 -  23
	____, ____, ____, ____, ____, ____, ____, ____,	//  24 -  31
	____, 0x7c, ____, 0xff, ____, ____, ____, ____,	//  32 -  39
	____, ____, ____, ____, ____, ____, ____, ____,	//  40 -  47
	____, ____, 0x70, ____, ____, ____, ____, ____,	//  48 -  55
	____, ____, ____, ____, ____, ____, ____, ____,	//  56 -  63
	____, ____, ____, ____, ____, ____, ____, ____,	//  64 -  71
	____, ____, ____, ____, ____, ____, ____, ____,	//  72 -  79
	____, ____, ____, ____, ____, ____, ____, ____,	//  80 -  87
	____, ____, ____, ____, ____, ____, ____, ____,	//  88 -  95
	____, ____, ____, 0xfe, ____, ____, ____, ____,	//  96 - 103
	____, ____, ____, ____, ____, 0x8b, ____, ____,	// 104 - 111
	____, ____, ____, 0x78, ____, ____, ____, ____,	// 112 - 119
	____, ____, ____, ____, ____, ____, ____, ____,	// 120 - 127
	____, ____, ____, ____, ____, ____, ____, ____,	// 128 - 135
	____, ____, ____, ____, ____, ____, ____, 0x77,	// 136 - 143
	____, ____, ____, ____, ____, ____, ____, ____,	// 144 - 151
	____, ____, ____, ____, ____, ____, 0x74, 0x80,	// 152 - 159
	____, 0x7b, ____, ____, ____, ____, ____, ____,	// 160 - 167
	____, ____, ____, ____, ____, ____, ____, ____,	// 168 - 175
	____, ____, ____, ____, ____, ____, ____, ____,	// 176 - 183
	____, 0x88, ____, ____, 0x87, ____, ____, ____,	// 184 - 191
	____, ____, ____, 0x84, ____, ____, ____, ____,	// 192 - 199
	____, ____, ____, ____, ____, ____, ____, ____,	// 200 - 207
	0x8c, ____, ____, ____, ____, 0x83, ____, ____,	// 208 - 215
	____, ____, ____, ____, ____, ____, ____, ____,	// 216 - 223
	____, ____, ____, ____, ____, ____, ____, ____,	// 224 - 231
	____, ____, ____, ____, ____, ____, ____, ____,	// 232 - 239
	____, ____, ____, ____, ____, ____, ____, ____,	// 240 - 247
	____, ____, ____, ____, ____, 0x7f, ____, ____,	// 248 - 255
};
#undef ____
#endif


Decoder::Decoder(ByteStream& gStrm, ByteStream& aStrm, bool isChess960)
	:m_gStrm(gStrm)
	,m_aStrm(aStrm)
	,m_moveNo(MaxMoveNo)
	,m_isChess960(isChess960)
	,m_lookup(/*isChess960 ? MoveNumberLookup960 : */MoveNumberLookup)
{
	if (!m_aStrm.isEmpty())
	{
		if ((m_moveNo = m_aStrm.uint24()) == 0xffffff)
			m_moveNo = -1;
	}
}


unsigned
Decoder::decodeMove(Move& move, unsigned& count)
{
	switch (m_lookup[Byte(m_gStrm.get() - count)])
	{
#define OFFSET(x, y) ((x) + (y)*8)

		// Null move ###########################
		case 0x00: move = m_position.doNullMove(); break;

		// King ################################
		case 0x01: move = m_position.doKingMove(OFFSET(0, 1)); break;
		case 0x02: move = m_position.doKingMove(OFFSET(1, 1)); break;
		case 0x03: move = m_position.doKingMove(OFFSET(1, 0)); break;
		case 0x04: move = m_position.doKingMove(OFFSET(1, 7)); break;
		case 0x05: move = m_position.doKingMove(OFFSET(0, 7)); break;
		case 0x06: move = m_position.doKingMove(OFFSET(7, 7)); break;
		case 0x07: move = m_position.doKingMove(OFFSET(7, 0)); break;
		case 0x08: move = m_position.doKingMove(OFFSET(7, 1)); break;
		case 0x09: move = m_position.doCastling(Byte(+2)); break;
		case 0x0a: move = m_position.doCastling(Byte(-2)); break;

		// First Queen #########################
		case 0x0b: move = m_position.doQueenMove(0, OFFSET(0, 1)); break;
		case 0x0c: move = m_position.doQueenMove(0, OFFSET(0, 2)); break;
		case 0x0d: move = m_position.doQueenMove(0, OFFSET(0, 3)); break;
		case 0x0e: move = m_position.doQueenMove(0, OFFSET(0, 4)); break;
		case 0x0f: move = m_position.doQueenMove(0, OFFSET(0, 5)); break;
		case 0x10: move = m_position.doQueenMove(0, OFFSET(0, 6)); break;
		case 0x11: move = m_position.doQueenMove(0, OFFSET(0, 7)); break;
		case 0x12: move = m_position.doQueenMove(0, OFFSET(1, 0)); break;
		case 0x13: move = m_position.doQueenMove(0, OFFSET(2, 0)); break;
		case 0x14: move = m_position.doQueenMove(0, OFFSET(3, 0)); break;
		case 0x15: move = m_position.doQueenMove(0, OFFSET(4, 0)); break;
		case 0x16: move = m_position.doQueenMove(0, OFFSET(5, 0)); break;
		case 0x17: move = m_position.doQueenMove(0, OFFSET(6, 0)); break;
		case 0x18: move = m_position.doQueenMove(0, OFFSET(7, 0)); break;
		case 0x19: move = m_position.doQueenMove(0, OFFSET(1, 1)); break;
		case 0x1a: move = m_position.doQueenMove(0, OFFSET(2, 2)); break;
		case 0x1b: move = m_position.doQueenMove(0, OFFSET(3, 3)); break;
		case 0x1c: move = m_position.doQueenMove(0, OFFSET(4, 4)); break;
		case 0x1d: move = m_position.doQueenMove(0, OFFSET(5, 5)); break;
		case 0x1e: move = m_position.doQueenMove(0, OFFSET(6, 6)); break;
		case 0x1f: move = m_position.doQueenMove(0, OFFSET(7, 7)); break;
		case 0x20: move = m_position.doQueenMove(0, OFFSET(1, 7)); break;
		case 0x21: move = m_position.doQueenMove(0, OFFSET(2, 6)); break;
		case 0x22: move = m_position.doQueenMove(0, OFFSET(3, 5)); break;
		case 0x23: move = m_position.doQueenMove(0, OFFSET(4, 4)); break;
		case 0x24: move = m_position.doQueenMove(0, OFFSET(5, 3)); break;
		case 0x25: move = m_position.doQueenMove(0, OFFSET(6, 2)); break;
		case 0x26: move = m_position.doQueenMove(0, OFFSET(7, 1)); break;

		// First Rook ##########################
		case 0x27: move = m_position.doRookMove(0, OFFSET(0, 1)); break;
		case 0x28: move = m_position.doRookMove(0, OFFSET(0, 2)); break;
		case 0x29: move = m_position.doRookMove(0, OFFSET(0, 3)); break;
		case 0x2a: move = m_position.doRookMove(0, OFFSET(0, 4)); break;
		case 0x2b: move = m_position.doRookMove(0, OFFSET(0, 5)); break;
		case 0x2c: move = m_position.doRookMove(0, OFFSET(0, 6)); break;
		case 0x2d: move = m_position.doRookMove(0, OFFSET(0, 7)); break;
		case 0x2e: move = m_position.doRookMove(0, OFFSET(1, 0)); break;
		case 0x2f: move = m_position.doRookMove(0, OFFSET(2, 0)); break;
		case 0x30: move = m_position.doRookMove(0, OFFSET(3, 0)); break;
		case 0x31: move = m_position.doRookMove(0, OFFSET(4, 0)); break;
		case 0x32: move = m_position.doRookMove(0, OFFSET(5, 0)); break;
		case 0x33: move = m_position.doRookMove(0, OFFSET(6, 0)); break;
		case 0x34: move = m_position.doRookMove(0, OFFSET(7, 0)); break;

		// Second Rook #########################
		case 0x35: move = m_position.doRookMove(1, OFFSET(0, 1)); break;
		case 0x36: move = m_position.doRookMove(1, OFFSET(0, 2)); break;
		case 0x37: move = m_position.doRookMove(1, OFFSET(0, 3)); break;
		case 0x38: move = m_position.doRookMove(1, OFFSET(0, 4)); break;
		case 0x39: move = m_position.doRookMove(1, OFFSET(0, 5)); break;
		case 0x3a: move = m_position.doRookMove(1, OFFSET(0, 6)); break;
		case 0x3b: move = m_position.doRookMove(1, OFFSET(0, 7)); break;
		case 0x3c: move = m_position.doRookMove(1, OFFSET(1, 0)); break;
		case 0x3d: move = m_position.doRookMove(1, OFFSET(2, 0)); break;
		case 0x3e: move = m_position.doRookMove(1, OFFSET(3, 0)); break;
		case 0x3f: move = m_position.doRookMove(1, OFFSET(4, 0)); break;
		case 0x40: move = m_position.doRookMove(1, OFFSET(5, 0)); break;
		case 0x41: move = m_position.doRookMove(1, OFFSET(6, 0)); break;
		case 0x42: move = m_position.doRookMove(1, OFFSET(7, 0)); break;

		// First Bishop ########################
		case 0x43: move = m_position.doBishopMove(0, OFFSET(1, 1)); break;
		case 0x44: move = m_position.doBishopMove(0, OFFSET(2, 2)); break;
		case 0x45: move = m_position.doBishopMove(0, OFFSET(3, 3)); break;
		case 0x46: move = m_position.doBishopMove(0, OFFSET(4, 4)); break;
		case 0x47: move = m_position.doBishopMove(0, OFFSET(5, 5)); break;
		case 0x48: move = m_position.doBishopMove(0, OFFSET(6, 6)); break;
		case 0x49: move = m_position.doBishopMove(0, OFFSET(7, 7)); break;
		case 0x4a: move = m_position.doBishopMove(0, OFFSET(1, 7)); break;
		case 0x4b: move = m_position.doBishopMove(0, OFFSET(2, 6)); break;
		case 0x4c: move = m_position.doBishopMove(0, OFFSET(3, 5)); break;
		case 0x4d: move = m_position.doBishopMove(0, OFFSET(4, 4)); break;
		case 0x4e: move = m_position.doBishopMove(0, OFFSET(5, 3)); break;
		case 0x4f: move = m_position.doBishopMove(0, OFFSET(6, 2)); break;
		case 0x50: move = m_position.doBishopMove(0, OFFSET(7, 1)); break;

		// Second Bishop #######################
		case 0x51: move = m_position.doBishopMove(1, OFFSET(1, 1)); break;
		case 0x52: move = m_position.doBishopMove(1, OFFSET(2, 2)); break;
		case 0x53: move = m_position.doBishopMove(1, OFFSET(3, 3)); break;
		case 0x54: move = m_position.doBishopMove(1, OFFSET(4, 4)); break;
		case 0x55: move = m_position.doBishopMove(1, OFFSET(5, 5)); break;
		case 0x56: move = m_position.doBishopMove(1, OFFSET(6, 6)); break;
		case 0x57: move = m_position.doBishopMove(1, OFFSET(7, 7)); break;
		case 0x58: move = m_position.doBishopMove(1, OFFSET(1, 7)); break;
		case 0x59: move = m_position.doBishopMove(1, OFFSET(2, 6)); break;
		case 0x5a: move = m_position.doBishopMove(1, OFFSET(3, 5)); break;
		case 0x5b: move = m_position.doBishopMove(1, OFFSET(4, 4)); break;
		case 0x5c: move = m_position.doBishopMove(1, OFFSET(5, 3)); break;
		case 0x5d: move = m_position.doBishopMove(1, OFFSET(6, 2)); break;
		case 0x5e: move = m_position.doBishopMove(1, OFFSET(7, 1)); break;

		// First Knight ########################
		case 0x5f: move = m_position.doKnightMove(0, OFFSET(+2, +1)); break;
		case 0x60: move = m_position.doKnightMove(0, OFFSET(+1, +2)); break;
		case 0x61: move = m_position.doKnightMove(0, OFFSET(-1, +2)); break;
		case 0x62: move = m_position.doKnightMove(0, OFFSET(-2, +1)); break;
		case 0x63: move = m_position.doKnightMove(0, OFFSET(-2, -1)); break;
		case 0x64: move = m_position.doKnightMove(0, OFFSET(-1, -2)); break;
		case 0x65: move = m_position.doKnightMove(0, OFFSET(+1, -2)); break;
		case 0x66: move = m_position.doKnightMove(0, OFFSET(+2, -1)); break;

		// Second Knight #######################
		case 0x67: move = m_position.doKnightMove(1, OFFSET(+2, +1)); break;
		case 0x68: move = m_position.doKnightMove(1, OFFSET(+1, +2)); break;
		case 0x69: move = m_position.doKnightMove(1, OFFSET(-1, +2)); break;
		case 0x6a: move = m_position.doKnightMove(1, OFFSET(-2, +1)); break;
		case 0x6b: move = m_position.doKnightMove(1, OFFSET(-2, -1)); break;
		case 0x6c: move = m_position.doKnightMove(1, OFFSET(-1, -2)); break;
		case 0x6d: move = m_position.doKnightMove(1, OFFSET(+1, -2)); break;
		case 0x6e: move = m_position.doKnightMove(1, OFFSET(+2, -1)); break;

		// a2/a7 Pawn ##########################
		case 0x6f: move = m_position.doPawnOneForward(0); break;
		case 0x70: move = m_position.doPawnTwoForward(0); break;
		case 0x71: move = m_position.doCaptureRight(0); break;
		case 0x72: move = m_position.doCaptureLeft(0); break;

		// b2/b7 Pawn ##########################
		case 0x73: move = m_position.doPawnOneForward(1); break;
		case 0x74: move = m_position.doPawnTwoForward(1); break;
		case 0x75: move = m_position.doCaptureRight(1); break;
		case 0x76: move = m_position.doCaptureLeft(1); break;

		// c2/c7 Pawn ##########################
		case 0x77: move = m_position.doPawnOneForward(2); break;
		case 0x78: move = m_position.doPawnTwoForward(2); break;
		case 0x79: move = m_position.doCaptureRight(2); break;
		case 0x7a: move = m_position.doCaptureLeft(2); break;

		// d2/d7 Pawn ##########################
		case 0x7b: move = m_position.doPawnOneForward(3); break;
		case 0x7c: move = m_position.doPawnTwoForward(3); break;
		case 0x7d: move = m_position.doCaptureRight(3); break;
		case 0x7e: move = m_position.doCaptureLeft(3); break;

		// e2/e7 Pawn ##########################
		case 0x7f: move = m_position.doPawnOneForward(4); break;
		case 0x80: move = m_position.doPawnTwoForward(4); break;
		case 0x81: move = m_position.doCaptureRight(4); break;
		case 0x82: move = m_position.doCaptureLeft(4); break;

		// f2/f7 Pawn ##########################
		case 0x83: move = m_position.doPawnOneForward(5); break;
		case 0x84: move = m_position.doPawnTwoForward(5); break;
		case 0x85: move = m_position.doCaptureRight(5); break;
		case 0x86: move = m_position.doCaptureLeft(5); break;

		// g2/g7 Pawn ##########################
		case 0x87: move = m_position.doPawnOneForward(6); break;
		case 0x88: move = m_position.doPawnTwoForward(6); break;
		case 0x89: move = m_position.doCaptureRight(6); break;
		case 0x8a: move = m_position.doCaptureLeft(6); break;

		// h2/h7 Pawn ##########################
		case 0x8b: move = m_position.doPawnOneForward(7); break;
		case 0x8c: move = m_position.doPawnTwoForward(7); break;
		case 0x8d: move = m_position.doCaptureRight(7); break;
		case 0x8e: move = m_position.doCaptureLeft(7); break;

		// Second Queen #########################
		case 0x8f: move = m_position.doQueenMove(1, OFFSET(0, 1)); break;
		case 0x90: move = m_position.doQueenMove(1, OFFSET(0, 2)); break;
		case 0x91: move = m_position.doQueenMove(1, OFFSET(0, 3)); break;
		case 0x92: move = m_position.doQueenMove(1, OFFSET(0, 4)); break;
		case 0x93: move = m_position.doQueenMove(1, OFFSET(0, 5)); break;
		case 0x94: move = m_position.doQueenMove(1, OFFSET(0, 6)); break;
		case 0x95: move = m_position.doQueenMove(1, OFFSET(0, 7)); break;
		case 0x96: move = m_position.doQueenMove(1, OFFSET(1, 0)); break;
		case 0x97: move = m_position.doQueenMove(1, OFFSET(2, 0)); break;
		case 0x98: move = m_position.doQueenMove(1, OFFSET(3, 0)); break;
		case 0x99: move = m_position.doQueenMove(1, OFFSET(4, 0)); break;
		case 0x9a: move = m_position.doQueenMove(1, OFFSET(5, 0)); break;
		case 0x9b: move = m_position.doQueenMove(1, OFFSET(6, 0)); break;
		case 0x9c: move = m_position.doQueenMove(1, OFFSET(7, 0)); break;
		case 0x9d: move = m_position.doQueenMove(1, OFFSET(1, 1)); break;
		case 0x9e: move = m_position.doQueenMove(1, OFFSET(2, 2)); break;
		case 0x9f: move = m_position.doQueenMove(1, OFFSET(3, 3)); break;
		case 0xa0: move = m_position.doQueenMove(1, OFFSET(4, 4)); break;
		case 0xa1: move = m_position.doQueenMove(1, OFFSET(5, 5)); break;
		case 0xa2: move = m_position.doQueenMove(1, OFFSET(6, 6)); break;
		case 0xa3: move = m_position.doQueenMove(1, OFFSET(7, 7)); break;
		case 0xa4: move = m_position.doQueenMove(1, OFFSET(1, 7)); break;
		case 0xa5: move = m_position.doQueenMove(1, OFFSET(2, 6)); break;
		case 0xa6: move = m_position.doQueenMove(1, OFFSET(3, 5)); break;
		case 0xa7: move = m_position.doQueenMove(1, OFFSET(4, 4)); break;
		case 0xa8: move = m_position.doQueenMove(1, OFFSET(5, 3)); break;
		case 0xa9: move = m_position.doQueenMove(1, OFFSET(6, 2)); break;
		case 0xaa: move = m_position.doQueenMove(1, OFFSET(7, 1)); break;

		// Third Queen ##########################
		case 0xab: move = m_position.doQueenMove(2, OFFSET(0, 1)); break;
		case 0xac: move = m_position.doQueenMove(2, OFFSET(0, 2)); break;
		case 0xad: move = m_position.doQueenMove(2, OFFSET(0, 3)); break;
		case 0xae: move = m_position.doQueenMove(2, OFFSET(0, 4)); break;
		case 0xaf: move = m_position.doQueenMove(2, OFFSET(0, 5)); break;
		case 0xb0: move = m_position.doQueenMove(2, OFFSET(0, 6)); break;
		case 0xb1: move = m_position.doQueenMove(2, OFFSET(0, 7)); break;
		case 0xb2: move = m_position.doQueenMove(2, OFFSET(1, 0)); break;
		case 0xb3: move = m_position.doQueenMove(2, OFFSET(2, 0)); break;
		case 0xb4: move = m_position.doQueenMove(2, OFFSET(3, 0)); break;
		case 0xb5: move = m_position.doQueenMove(2, OFFSET(4, 0)); break;
		case 0xb6: move = m_position.doQueenMove(2, OFFSET(5, 0)); break;
		case 0xb7: move = m_position.doQueenMove(2, OFFSET(6, 0)); break;
		case 0xb8: move = m_position.doQueenMove(2, OFFSET(7, 0)); break;
		case 0xb9: move = m_position.doQueenMove(2, OFFSET(1, 1)); break;
		case 0xba: move = m_position.doQueenMove(2, OFFSET(2, 2)); break;
		case 0xbb: move = m_position.doQueenMove(2, OFFSET(3, 3)); break;
		case 0xbc: move = m_position.doQueenMove(2, OFFSET(4, 4)); break;
		case 0xbd: move = m_position.doQueenMove(2, OFFSET(5, 5)); break;
		case 0xbe: move = m_position.doQueenMove(2, OFFSET(6, 6)); break;
		case 0xbf: move = m_position.doQueenMove(2, OFFSET(7, 7)); break;
		case 0xc0: move = m_position.doQueenMove(2, OFFSET(1, 7)); break;
		case 0xc1: move = m_position.doQueenMove(2, OFFSET(2, 6)); break;
		case 0xc2: move = m_position.doQueenMove(2, OFFSET(3, 5)); break;
		case 0xc3: move = m_position.doQueenMove(2, OFFSET(4, 4)); break;
		case 0xc4: move = m_position.doQueenMove(2, OFFSET(5, 3)); break;
		case 0xc5: move = m_position.doQueenMove(2, OFFSET(6, 2)); break;
		case 0xc6: move = m_position.doQueenMove(2, OFFSET(7, 1)); break;

		// Third Rook ##########################
		case 0xc7: move = m_position.doRookMove(2, OFFSET(0, 1)); break;
		case 0xc8: move = m_position.doRookMove(2, OFFSET(0, 2)); break;
		case 0xc9: move = m_position.doRookMove(2, OFFSET(0, 3)); break;
		case 0xca: move = m_position.doRookMove(2, OFFSET(0, 4)); break;
		case 0xcb: move = m_position.doRookMove(2, OFFSET(0, 5)); break;
		case 0xcc: move = m_position.doRookMove(2, OFFSET(0, 6)); break;
		case 0xcd: move = m_position.doRookMove(2, OFFSET(0, 7)); break;
		case 0xce: move = m_position.doRookMove(2, OFFSET(1, 0)); break;
		case 0xcf: move = m_position.doRookMove(2, OFFSET(2, 0)); break;
		case 0xd0: move = m_position.doRookMove(2, OFFSET(3, 0)); break;
		case 0xd1: move = m_position.doRookMove(2, OFFSET(4, 0)); break;
		case 0xd2: move = m_position.doRookMove(2, OFFSET(5, 0)); break;
		case 0xd3: move = m_position.doRookMove(2, OFFSET(6, 0)); break;
		case 0xd4: move = m_position.doRookMove(2, OFFSET(7, 0)); break;

		// Third Bishop ########################
		case 0xd5: move = m_position.doBishopMove(2, OFFSET(1, 1)); break;
		case 0xd6: move = m_position.doBishopMove(2, OFFSET(2, 2)); break;
		case 0xd7: move = m_position.doBishopMove(2, OFFSET(3, 3)); break;
		case 0xd8: move = m_position.doBishopMove(2, OFFSET(4, 4)); break;
		case 0xd9: move = m_position.doBishopMove(2, OFFSET(5, 5)); break;
		case 0xda: move = m_position.doBishopMove(2, OFFSET(6, 6)); break;
		case 0xdb: move = m_position.doBishopMove(2, OFFSET(7, 7)); break;
		case 0xdc: move = m_position.doBishopMove(2, OFFSET(1, 7)); break;
		case 0xdd: move = m_position.doBishopMove(2, OFFSET(2, 6)); break;
		case 0xde: move = m_position.doBishopMove(2, OFFSET(3, 5)); break;
		case 0xdf: move = m_position.doBishopMove(2, OFFSET(4, 4)); break;
		case 0xe0: move = m_position.doBishopMove(2, OFFSET(5, 3)); break;
		case 0xe1: move = m_position.doBishopMove(2, OFFSET(6, 2)); break;
		case 0xe2: move = m_position.doBishopMove(2, OFFSET(7, 1)); break;

		// Third Knight ########################
		case 0xe3: move = m_position.doKnightMove(2, OFFSET(+2, +1)); break;
		case 0xe4: move = m_position.doKnightMove(2, OFFSET(+1, +2)); break;
		case 0xe5: move = m_position.doKnightMove(2, OFFSET(-1, +2)); break;
		case 0xe6: move = m_position.doKnightMove(2, OFFSET(-2, +1)); break;
		case 0xe7: move = m_position.doKnightMove(2, OFFSET(-2, -1)); break;
		case 0xe8: move = m_position.doKnightMove(2, OFFSET(-1, -2)); break;
		case 0xe9: move = m_position.doKnightMove(2, OFFSET(+1, -2)); break;
		case 0xea: move = m_position.doKnightMove(2, OFFSET(+2, -1)); break;

		// Multiple byte move ##################
		case 0xeb:
			{
				unsigned word = m_lookup[Byte(m_gStrm.get() - count)] << 8;
				word |= m_lookup[Byte(m_gStrm.get() - count)];
				move = m_position.doMove(word & 63, (word >> 6) & 63, ((word >> 12) & 3) + piece::Queen);
			}
			break;

		// Padding #############################
		case 0xec: return ::Skip;

		// Unused ##############################
		case 0xed: // fallthru
		case 0xee: // fallthru
		case 0xef: // fallthru
		case 0xf0: // fallthru
		case 0xf1: // fallthru
		case 0xf2: // fallthru
		case 0xf3: // fallthru
		case 0xf4: // fallthru
		case 0xf5: // fallthru
		case 0xf6: // fallthru
		case 0xf7: // fallthru
		case 0xf8: // fallthru
		case 0xf9: // fallthru
		case 0xfa: // fallthru
		case 0xfb: // fallthru
		case 0xfc: // fallthru
		case 0xfd: return ::Skip;

		// Push position #######################
		case 0xfe:
			m_position.push();
			return ::Push;

		// Pop position ########################
		case 0xff:
			if (m_position.variationLevel())
				m_position.pop();
			return ::Pop;

#undef OFFSET
	}

	++count;
	return ::Move;
}


void
Decoder::traverse(Consumer& consumer, MoveNode const* node)
{
	//M_ASSERT(node);

	if (node->hasNote())
		consumer.putPrecedingComment(node->comment(move::Post), node->annotation(), node->marks());

	for (node = node->next(); node->isBeforeLineEnd(); node = node->next())
	{
		if (node->hasNote())
		{
			consumer.putMove(	node->move(),
									node->annotation(),
									node->comment(move::Ante),
									node->comment(move::Post),
									node->marks());
		}
		else
		{
			consumer.putMove(node->move());
		}

		for (unsigned i = 0; i < node->variationCount(); ++i)
		{
			consumer.startVariation();
			traverse(consumer, node->variation(i));
			consumer.finishVariation();
		}
	}
}


void
Decoder::decodeComment(MoveNode* node, unsigned length, move::Position position)
{
	//M_ASSERT(node);

	if (length < 2)
	{
		m_aStrm.skip(length);
		return;
	}

	uint16_t country = m_aStrm.uint16();
	length -= 2;

	unsigned char const* p = m_aStrm.data();

	mstl::string str;

	char const* lang = 0;

	if (country < U_NUMBER_OF(::LangMap))
		lang = ::LangMap[country];

	bool useXml = lang != 0 || node->comment(position).isXml();

	// BUG: ChessBase uses country code 0 for ALL and for Pol.
	//      How should we distinguish between these countries?

	str.reserve(length + 200);

	unsigned i = 0;
	while (::isspace(p[i]))
		++i;

	for ( ; i < length; ++i)
	{
		Byte c = p[i];

		if (::isprint(c))
		{
			switch (c)
			{
				case '<':	str.append("&lt;",   4); break;
				case '>':	str.append("&gt;",   4); break;
				case '&':	str.append("&amp;",  5); break;
				case '\'':	str.append("&apos;", 6); break;
				case '"':	str.append("&quot;", 6); break;
				default:		str += c; break;
			}
		}
		else
		{
			nag::ID nag = nag::Null;

			switch (c)
			{
				case 0x0d:
					str.append(0x0a);
					if (p[i + 1] == 0x0a)
						++i;
					break;

				case 0xa2: str.append("<sym>K</sym>", 12); useXml = true; break;
				case 0xa3: str.append("<sym>Q</sym>", 12); useXml = true; break;
				case 0xa4: str.append("<sym>N</sym>", 12); useXml = true; break;
				case 0xa5: str.append("<sym>B</sym>", 12); useXml = true; break;
				case 0xa6: str.append("<sym>R</sym>", 12); useXml = true; break;
				case 0xa7: str.append("<sym>P</sym>", 12); useXml = true; break;

				case 0x82: nag = nag::Attack; break; 								// "->"
				case 0x83: nag = nag::Initiative; break;							// "|^"
				case 0x84: nag = nag::Counterplay; break;							// "<=>"
				case 0x85: nag = nag::WithTheIdea; break;							// "/\"
				case 0x86: nag = nag::Space; break; 								// "()"
				case 0x87: nag = nag::Zugzwang; break;								// "(.)"
				case 0x91: nag = nag::Line; break; 									// "<->"
				case 0x92: nag = nag::Diagonal; break; 							// "/^"
				case 0x93: nag = nag::Zeitnot; break;								// "(+)"
				case 0x94: nag = nag::Center; break;								// "[+]"
				case 0x99: nag = nag::SingularMove; break;						// "[]"
				case 0xaa: nag = nag::With; break; 									// "|_"
				case 0xab: nag = nag::Queenside; break; 							// "<<"
				case 0xac: nag = nag::Endgame; break; 								// "_|_"
				case 0xad: nag = nag::PairOfBishops; break; 						// "^^"
				case 0xae: nag = nag::BishopsOfOppositeColor; break; 			// "^_"
				case 0xaf: nag = nag::BishopsOfSameColor; break;				// "^="
				case 0xb1: nag = nag::WhiteHasAModerateAdvantage; break; 	// "+/-"
				case 0xb2: nag = nag::WhiteHasASlightAdvantage; break; 		// "+/="
				case 0xb3: nag = nag::BlackHasASlightAdvantage; break; 		// "=/+"
				case 0xb5: nag = nag::BlackHasAModerateAdvantage; break; 	// "-/+"
				case 0xb9: nag = nag::BetterMove; break; 							// ">="
				case 0xba: nag = nag::Without; break; 								// "_|"
				case 0xbb: nag = nag::Kingside; break; 							// ">>"
				case 0xd7: nag = nag::WeakPoint; break; 							// "><"
				case 0xf7: nag = nag::UnclearPosition; break; 					// "~~"
				case 0xfe: nag = nag::PassedPawn; break; 							// "o^"

//				It seems to be impossible to use these symbols in ChessBase.
//				case 0x??: nag = nag::WithCompensationForMaterial; break;	// "~/="
//				case 0x??: nag = nag::Development; break;							// "@" ???

				case 0x9e:
					node->addAnnotation(nag::Diagram);								// "#"
					break;

				default:
					if (length == 1)
					{
						m_aStrm.skip(1);
						return; // seems to be a special instruction
					}
					str += c;
					break;
			}

			if (nag != nag::Null)
			{
				str.format("<nag>%u</nag>", unsigned(nag));
				useXml = true;
			}
		}
	}

	str.rtrim();

	if (!str.empty())
	{
		// TODO: use character encoding according to language code ?!
		//m_codec.toUtf8(str);

		//if (!sys::utf8::validate(str))
		//	m_codec.forceValidUtf8(str);

		if (useXml)
		{
			str.insert(str.begin(), mstl::string("<xml><:") + (lang ? lang : "") + '>');
			str.append("</:");
			if (lang)
				str.append(lang);
			str.append("></xml>");
		}

		bool isEnglish	= lang && ::strcmp(lang, "en") == 0;
		bool isOther	= lang && !isEnglish;

		Comment comment;
		comment.swap(str, isEnglish, isOther);
		comment.normalize();

		if (node->hasComment(position))
		{
			Comment c;
			node->swapComment(c, position);
			c.append(comment, '\n');
			node->swapComment(c, position);
		}
		else
		{
			node->setComment(comment, position);
		}
	}

	m_aStrm.skip(length);
}


void
Decoder::decodeSymbols(MoveNode* node, unsigned length)
{
#define NAG(code) wtm ? nag::White##code : nag::Black##code

	static_assert(Annotation::Max_Nags >= 4, "ChessBase need at least four entries");

	if (length == 0)
		return;

	bool wtm = color::isWhite(node->move().color());	// white to move

	switch (m_aStrm.get())
	{
		case 0x01: node->addAnnotation(nag::GoodMove); break;
		case 0x02: node->addAnnotation(nag::PoorMove); break;
		case 0x03: node->addAnnotation(nag::VeryGoodMove); break;
		case 0x04: node->addAnnotation(nag::VeryPoorMove); break;
		case 0x05: node->addAnnotation(nag::SpeculativeMove); break;
		case 0x06: node->addAnnotation(nag::QuestionableMove); break;
		case 0x08: node->addAnnotation(nag::SingularMove); break;
		case 0x16: node->addAnnotation(NAG(IsInZugzwang)); break;
	}

	if (length == 1)
		return;

	switch (m_aStrm.get())
	{
		case 0x0b: node->addAnnotation(nag::DrawishPosition); break;
		case 0x0d: node->addAnnotation(nag::UnclearPosition); break;
		case 0x0e: node->addAnnotation(nag::WhiteHasASlightAdvantage); break;
		case 0x0f: node->addAnnotation(nag::BlackHasASlightAdvantage); break;
		case 0x10: node->addAnnotation(nag::WhiteHasAModerateAdvantage); break;
		case 0x11: node->addAnnotation(nag::BlackHasAModerateAdvantage); break;
		case 0x12: node->addAnnotation(nag::WhiteHasADecisiveAdvantage); break;
		case 0x13: node->addAnnotation(nag::BlackHasADecisiveAdvantage); break;
		case 0x20: node->addAnnotation(NAG(HasAModerateTimeAdvantage)); break;
		case 0x24: node->addAnnotation(NAG(HasTheInitiative )); break;
		case 0x28: node->addAnnotation(NAG(HasTheAttack)); break;
		case 0x2c: node->addAnnotation(NAG(HasSufficientCompensationForMaterialDeficit)); break;
		case 0x84: node->addAnnotation(NAG(HasModerateCounterplay)); break;
		case 0x8a: node->addAnnotation(nag::TimeLimit); break;
		case 0x92: node->addAnnotation(nag::Novelty); break;
	}

	if (length == 2)
		return;

	switch (m_aStrm.get())
	{
		case 0x8C: node->addAnnotation(nag::WithTheIdea); break;
		case 0x8D: node->addAnnotation(nag::AimedAgainst); break;
		case 0x8E: node->addAnnotation(nag::BetterMove); break;
		case 0x8F: node->addAnnotation(nag::WorseMove); break;
		case 0x90: node->addAnnotation(nag::EquivalentMove); break;
		case 0x91: node->addAnnotation(nag::EditorsRemark); break;
	}

#undef NAG
}


void
Decoder::decodeSquares(MoveNode* node, unsigned length)
{
	for ( ; length >= 2; length -= 2)
	{
		mark::Color c = ::mapColor(m_aStrm.get());
		Square s = ::mapSquare(m_aStrm.get() - 1);

		if (s <= sq::h8)
			node->addMark(Mark(mark::Full, c, s));
	}

	m_aStrm.skip(length);	// to be sure
}


void
Decoder::decodeArrows(MoveNode* node, unsigned length)
{
	for ( ; length >= 3; length -= 3)
	{
		mark::Color c = ::mapColor(m_aStrm.get());
		Square s = ::mapSquare(m_aStrm.get() - 1);
		Square t = ::mapSquare(m_aStrm.get() - 1);

		if (s <= sq::h8 && t <= sq::h8)
			node->addMark(Mark(mark::Arrow, c, s, t));
	}

	m_aStrm.skip(length);	// to be sure
}


void
Decoder::getAnnotation(MoveNode* node, int moveNo)
{
	//M_ASSERT(moveNo != MaxMoveNo);
	//M_ASSERT(node);

	if (moveNo < m_moveNo)
		return;

	//M_ASSERT(!m_aStrm.isEmpty());

	while (moveNo == m_moveNo)
	{
		Byte		type		= m_aStrm.get();
		unsigned length	= m_aStrm.uint16() - 6;

		if (length > m_aStrm.remaining())
		{
			m_aStrm.skip(m_aStrm.remaining());
			return;
		}

		switch (type)
		{
			case 0x82:	// text before move
				if (!node->atLineStart())
				{
					decodeComment(node, length, move::Ante);
					break;
				}
				// fallthru

			case 0x02:	// text after move
				decodeComment(node, length, move::Post);
				break;

			case 0x03:	// symbols
				//M_ASSERT(length <= 3);
				decodeSymbols(node, length);
				break;

			case 0x04:	// squares
				decodeSquares(node, length);
				break;

			case 0x05:	// arrows
				decodeArrows(node, length);
				break;

			default:
				m_aStrm.skip(length);
				break;
		}

		if (m_aStrm.isEmpty())
			m_moveNo = MaxMoveNo;
		else if ((m_moveNo = m_aStrm.uint24()) == 0xffffff)
			m_moveNo = -1;
	}
}


void
Decoder::decodeMoves(Consumer& consumer)
{
	MoveNode	node;
	Comment	comment;
	unsigned	count = 0;
	Move		move;

	while (true)
	{
		switch (decodeMove(move, count))
		{
			case ::Move:
				if (!move)
					return;
				getAnnotation(&node, int(count) - 1);
				if (node.hasNote())
				{
					consumer.putMove(move, node.annotation(), comment, comment, node.marks());
					node.clearAnnotation();
					node.clearMarks();
				}
				else
				{
					consumer.putMove(move);
				}
				break;

			case ::Push:
				//IO_RAISE(Game, Corrupted, "unexpected PUSH");
				break;

			case ::Pop:
				return;
		}
	}
}


void
Decoder::decodeMoves(MoveNode* root, unsigned& count)
{
	typedef mstl::vector<MoveNode*> Vars;

	Vars varList;
	Move move;

	while (true)
	{
		MoveNode* node;

		switch (decodeMove(move, count))
		{
			case ::Move:
				if (move)
				{
					node = new MoveNode(move);

					if (varList.empty())
					{
						root->setNext(node);
					}
					else
					{
						// first variation is main line
						// main line is last variation

						MoveNode* main = varList[0];

						if (!main->next()){}
							// IO_RAISE(Game, Corrupted, "bad data");

						root->setNext(main->removeNext());
						root = root->next();

						for (unsigned i = 1; i < varList.size(); ++i)
						{
							MoveNode* var  = varList[i];
							MoveNode* next = var->next();

#ifndef ALLOW_EMPTY_VARS
							if (next->atLineEnd())
							{
								// Scidb does not support empty variations,
								// but we cannot delete the variation if a
								// comment/annotation/mark exists. As a
								// workaround we insert a null move.
								// Note: Possibly it isn't possible to enter
								// empty variations in ChessBase, but we like
								// to handle this case for safety reasons.
								if (var->hasSupplement() || next->hasSupplement())
								{
									Move null(Move::null());
									null.setColor(move.color());
									next = new MoveNode(null);
									next->setNext(var->removeNext());
									var->setNext(next);
								}
							}
#endif

							if (next->isBeforeLineEnd())
								root->addVariation(var);
						}

						root->addVariation(main);
						main->setNext(node);
						varList.clear();
					}

					getAnnotation(node, int(count) - 1);
					root = node;
				}
				else
				{
					MoveNode dummy;
					getAnnotation(&dummy, int(count) - 1);
				}
				break;

			case ::Push:
				node = new MoveNode;
				varList.push_back(node);
				decodeMoves(node, count);
				break;

			case ::Pop:
				root->setNext(new MoveNode);
				return;
		}
	}
}


void
Decoder::decodeMoves(MoveNode* root)
{
	unsigned count = 0;
	decodeMoves(root, count);
}


void
Decoder::startDecoding(TagSet* tags)
{
	unsigned word = m_gStrm.uint32();

	if (word & 0x40000000)
	{
		unsigned size = m_isChess960 ? 36 : 28;

		BitStream bstrm(m_gStrm.data(), size);
		m_position.setup(bstrm);
		m_gStrm.skip(size);

		if (tags)
		{
			tags->set(tag::SetUp, "1");	// bad PGN design
			tags->set(tag::Fen, m_position.board().toFen(Board::Shredder));
		}
	}
	else
	{
		m_position.setup();
	}
}


unsigned
Decoder::doDecoding(GameData& data)
{
	startDecoding(&data.m_tags);
	data.m_startBoard = m_position.board();
	unsigned plyNumber = m_position.board().plyNumber();
	decodeMoves(data.m_startNode);
	return m_position.board().plyNumber() - plyNumber;
}


save::State
Decoder::doDecoding(Consumer& consumer, TagSet& tags, GameInfo const& info)
{
	startDecoding(&tags);

	if (!consumer.startGame(tags, m_position.board()))
		return save::UnsupportedVariant;

	unsigned plyNumber = m_position.board().plyNumber();
	consumer.startMoveSection();

	if (info.countVariations() == 0 && info.countComments() == 0)
	{
		// fast decoding
		decodeMoves(consumer);
	}
	else
	{
		// slow decoding
		MoveNode root;
		decodeMoves(&root);
		traverse(consumer, &root);
	}

	char buf[32];
	::sprintf(buf, "%u", m_position.board().plyNumber() - plyNumber);
	tags.set(tag::PlyCount, buf);

	consumer.finishMoveSection(result::fromString(tags.value(tag::Result)));
	return consumer.finishGame(tags);
}


db::Move
Decoder::findExactPosition(Board const& position, bool skipVariations)
{
	startDecoding();

	unsigned	count = 0;
	bool		found	= false;
	Move		move;

	if (m_position.board().isEqualPosition(position))
		found = true;

	while (true)
	{
		unsigned tag = decodeMove(move, count);

		if (found)
			return move;

		switch (tag)
		{
			case ::Move:
				if (!move || !m_position.board().signature().isReachablePawns(position.signature()))
					return Move::invalid();
				if (m_position.board().isEqualPosition(position))
					found = true;
				break;

			case ::Pop:
				if (skipVariations)
					return Move::invalid();
				break;
		}
	}

	return move;	// not reached
}

// vi:set ts=3 sw=3:
