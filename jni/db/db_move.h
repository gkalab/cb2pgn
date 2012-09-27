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
// This class is loosely based on chessx/src/database/move.h
// ======================================================================

#ifndef _db_move_included
#define _db_move_included

#include "db_common.h"

#include "u_crc.h"

#include "m_string.h"

namespace db {

class Board;
class MoveList;

/** @ingroup Core
   Moves are dependent on current position, (remembers piece, check, capture etc)
   and don't make much sense when considered without a Board.
   However, you can create a move with only a source and destination square,
   Such moves are considered "illegal", but can be convenient when dealing with move entry.
*/

class Move
{
private:

	enum
	{
		Shift_Promotion	= 12,
		Shift_Destination	= 14,
		Shift_Piece			= 15,	Shift_Action = Shift_Piece,
		Shift_Castling		= 18,
		Shift_TwoForward	= 19,
		Shift_Promote		= 20,
		Shift_Capture		= 21,	Shift_Removal = Shift_Capture,
		Shift_EnPassant	= 24,
		Shift_SideToMove	= 25,
		Shift_Fyle			= 26,
		Shift_Rank			= 27,
		Shift_Mate			= 28,
		Shift_Check			= 29,
		Shift_Printable	= 30,
		Shift_Legality		= 31,
	};

	enum
	{
		Bit_Castling		= 1 << Shift_Castling,
		Bit_TwoForward		= 1 << Shift_TwoForward,
		Bit_Promote			= 1 << Shift_Promote,
		Bit_Destination	= 1 << Shift_Destination,
		Bit_EnPassant		= 1 << Shift_EnPassant,
		Bit_BlackToMove	= 1 << Shift_SideToMove,
		Bit_Fyle				= 1 << Shift_Fyle,
		Bit_Rank				= 1 << Shift_Rank,
		Bit_Mate				= 1 << Shift_Mate,
		Bit_Check			= 1 << Shift_Check,
		Bit_Printable		= 1 << Shift_Printable,
		Bit_Legality		= 1 << Shift_Legality,
	};

	static uint32_t const Mask_PieceType		= (1u << 3) - 1;
	static uint32_t const Mask_Promoted 		= (1u << 2) - 1;
	static uint32_t const Mask_Removal			= (1u << 4) - 1;
	static uint32_t const Mask_Action			= (1u << 6) - 1;
	static uint32_t const Mask_Compare			= uint32_t(~0) >> (31 - Shift_SideToMove);

	static uint32_t const Clear_PieceType		= ~(Mask_PieceType << Shift_Piece);
	static uint32_t const Clear_CaptureType	= ~(Mask_PieceType << Shift_Capture);
	static uint32_t const Clear_Promote			= ~((Mask_Promoted << Shift_Promotion) | Bit_Promote);

	static uint32_t const Bits_Special	= Bit_Castling | Bit_TwoForward | Bit_Promote | Bit_EnPassant;

public:

	static uint16_t const Null		= 0;
	static uint16_t const Empty	= 0;
	static uint16_t const Invalid	= piece::King << Shift_Promotion;

	// action
	static unsigned const One_Forward	= piece::Pawn;
	static unsigned const Two_Forward	= piece::Pawn | (1u << (Shift_TwoForward - Shift_Action));
	static unsigned const Promote			= piece::Pawn | (1u << (Shift_Promote - Shift_Action));
	static unsigned const Castle			= piece::King | (1u << (Shift_Castling - Shift_Action));

	enum { Max_SAN_Length = 12 };
	enum { Index_Bit_Length = 14 };

	/// Default constructor, creates an empty move
	Move();
	/// Create move from move data (w/o undo data)
	explicit Move(uint32_t m);

	/// Check whether move is not empty.
	operator bool () const;
	/// Check whether move is empty.
	bool operator!() const;
	/// Check whether move is prepared for undo.
	bool preparedForUndo() const;

	/// Set promotion piece. Default promotion is to Queen, use this to change piece afterward.
	void setPromotionPiece(piece::Type type);

	/// Return square piece sits on after move.
	Square to() const;
	/// Return Square piece sat on before move.
	Square from() const;
	/// Return square where king was placed before castling.
	Square castlingKingFrom() const;
	/// Return square where rook was placed before castling.
	Square castlingRookFrom() const;
	/// Return square where king is placed after castling.
	Square castlingKingTo() const;
	/// Return square where rook is placed after castling.
	Square castlingRookTo() const;
	/// Return square when en-passant is captured. Undefined if there is no en-passant.
	Square enPassantSquare() const;
	/// Return square where captured piece was placed.
	Square capturedSquare() const;
	/// Return a value usable as an index (14 bit wide).
	unsigned index() const;
	/// Return move data (w/o undo data).
	uint32_t data() const;
	/// Return action of this move.
	uint32_t action() const;
	/// Compute checksum fot this move.
	util::crc::checksum_t computeChecksum(util::crc::checksum_t crc) const;

	/// Get the piece type moving.
	piece::Type pieceMoved() const;
	/// Get the piece moving.
	piece::ID piece() const;
	/// Piece type captured from the opponent by this move.
	piece::Type captured() const;
	/// Piece captured from the opponent by this move.
	piece::ID capturedPiece() const;
	/// If move is promotion, get promotion piece type. Result is undefined if there is no promotion.
	piece::Type promoted() const;
	/// If move is promotion, get promotion piece. Result is undefined if there is no promotion.
	piece::ID promotedPiece() const;

	/// Get the color of the moving piece.
	color::ID color() const;

	/// Check whether move is empty.
	bool isEmpty() const;
	/// Check whether move is null move.
	bool isNull() const;
	/// Check whether move is an invalid move.
	bool isInvalid() const;
	/// Check whether move is special (promotion, castling, en passant).
	bool isSpecial() const;
	/// Check whether move is capture or promotion.
	bool isCaptureOrPromotion() const;
	/// Check whether move is a promotion.
	bool isPromotion() const;
	/// Check whether move is a castling.
	bool isCastling() const;
	/// Check whether move is a short castling.
	bool isShortCastling() const;
	/// Check whether move is a long castling.
	bool isLongCastling() const;
	/// Check whether the move is a pawn double advance.
	bool isDoubleAdvance() const;
	/// Check whether move is an en passant.
	bool isEnPassant() const;
	/// Check if move is completely legal in the context it was created.
	bool isLegal() const;
	/// Check if move is illlegal in the position.
	bool isIllegal() const;
	/// Check is move is prepared for printing.
	bool isPrintable() const;

	/// Check whether move is giving check.
	bool givesCheck() const;
	/// Check whether move is giving checkmate.
	bool givesMate() const;
	/// Check whether fyle is needed to dissolve disambiguation.
	bool needsFyle() const;
	/// Check whether rank is needed to dissolve disambiguation.
	bool needsRank() const;
	// Check whether square is needed to dissolve disambiguation of capturing.
	bool needsDestinationSquare() const;

	/// Print algebraic form.
	mstl::string& printAlgebraic(mstl::string& s) const;
	/// Print LAN (long algebraic noatation).
	mstl::string& printLan(mstl::string& s, encoding::CharSet charSet = encoding::Latin1) const;
	/// Print SAN (short algebraic noatation).
	mstl::string& printSan(mstl::string& s, encoding::CharSet charSet = encoding::Latin1) const;
	/// Print descriptive (english) form.
	mstl::string& printDescriptive(mstl::string& s) const;
	/// Print correspondence form.
	mstl::string& printNumeric(mstl::string& s) const;
	/// Print telegraphic form.
	mstl::string& printAlphabetic(mstl::string& s) const;
	/// Print in given notation style.
	mstl::string& print(	mstl::string& s,
								move::Notation style,
								encoding::CharSet charSet = encoding::Latin1) const;

	/// Make empty move.
	void clear();
	/// Mark this move as validated and fully legal in position
	void setLegalMove();
	/// Mark this move as illegal in position
	void setIllegalMove();
	/// Mark this move as legal or illegal in position
	void setLegalMove(bool flag);
	/// Set the side to move (only works if color is black)
	void setColor(unsigned color);
	/// Set source and destination squares
	void setSquares(uint32_t from, uint32_t to);
	/// Transpose fyle.
	void transpose();

	/// Returns an empty move.
	static Move const& empty();
	/// Returns a null move.
	static Move const& null();
	/// Returns an invalid move.
	static Move const& invalid();
	/// Returns a Pawn move of one forward.
	static Move genOneForward(uint32_t from, uint32_t to);
	/// Returns a two-squares forward move of pawn.
	static Move genTwoForward(uint32_t from, uint32_t to);
	/// Returns a pawn promotion move to given Piecetype.
	static Move genPromote(uint32_t from, uint32_t to, uint32_t type);
	/// Returns a pawn capture and promotion, promote to piece type, capturing type.
	static Move genCapturePromote(uint32_t from, uint32_t to, uint32_t type, uint32_t captured);
	/// Returns a pawn en passant capture of opponent pawn.
	static Move genEnPassant(uint32_t from, uint32_t to);
	/// Returns a simple pawn move with capture of piece type.
	static Move genPawnCapture(uint32_t from, uint32_t to, uint32_t captured);
	/// Returns a knight move, capturing piece type.
	static Move genKnightMove(uint32_t from, uint32_t to, uint32_t captured);
	/// Returns a bishop move, capturing piece type.
	static Move genBishopMove(uint32_t from, uint32_t to, uint32_t captured);
	/// Returns a rook move, capturing piece type.
	static Move genRookMove(uint32_t from, uint32_t to, uint32_t captured);
	/// Returns a queen move, capturing piece type.
	static Move genQueenMove(uint32_t from, uint32_t to, uint32_t captured);
	/// Returns a king move, capturing piece type.
	static Move genKingMove(uint32_t from, uint32_t to, uint32_t captured);
	/// Returns a castling move.
	static Move genCastling(Square from, Square to);

	// Convert to string.
	mstl::string asString() const;

	/// Dump move.
	mstl::string& dump(mstl::string& result) const;
	void dump() const;

	/// Moves are considered the same only if they match exactly (discarding info values).
	friend bool operator==(Move const& m1, Move const& m2);
	/// Required for keeping moves in some map-like structures (discarding info values).
	friend bool operator<(Move const& m1, Move const& m2);

	friend class Board;
	friend class MoveList;

private:

	// removal
	static unsigned const En_Passant = piece::Pawn | (1 << (Shift_EnPassant - Shift_Removal));

	/// Move entry constructor, untested (illegal) move with only from, and to squares set.
	Move(Square from, Square to, unsigned color);

	/// Set source square for this move.
	void setFrom(uint32_t from);
	/// Set destination square for this move.
	void setTo(uint32_t to);
	/// Set type of piece (Queen, Rook, Bishop, Knight) pawn promoted to.
	void setPromoted(uint32_t p);
	/// Set type of piece (Queen, Rook, Bishop, Knight, Pawn) making move.
	void setPieceType(uint32_t p);
	/// Set type of piece (Queen, Rook, Bishop, Knight, Pawn) captured.
	void setCaptureType(uint32_t p);
	/// Mark this move as an initial pawn move of 2 squares.
	void setTwoForward();
	/// Mark this move capturing a pawn en passant.
	void setEnPassant();
	/// Mark this move as a capturing move by a pawn.
	void setPawnCapture();
	/// Mark this move as giving check.
	void setCheck();
	/// Mark this move as giving checkmate.
	void setMate();
	/// Mark this move that SAN needs fyle.
	void setNeedsFyle();
	/// Mark this move that SAN needs rank.
	void setNeedsRank();
	/// Mark this capturing move that descriptive notation needs destination square.
	void setNeedsDestinationSquare();
	/// Mark move as prepared for printing.
	void setPrintable();

	/// Return captured piece or en-passant for doMove() and undoMove().
	uint32_t removal() const;
	/// Return piece type of captured piece (or 0 if none).
	uint32_t capturedType() const;

	void setUndo(uint32_t halfMoves, uint32_t epSquare, bool epSquareExists, uint32_t castlingRights);

	uint16_t prevHalfMoves() const;
	Square prevEpSquare() const;
	Byte prevCastlingRights() const;
	bool prevEpSquareExists() const;

	// The move definition 'm' bitfield layout:
	// - move is empty if all bits are zero
	// - move is null if only the legality status is set
	// - the first 12/14 bits are usable as a short move value
	// 00000000 00000000 00000000 00111111 = from square       = bits  1-6
	// 00000000 00000000 00001111 11000000 = to square         = bits  7-12
	// 00000000 00000000 00110000 00000000 = promotion piece   = bits 13-14
	// 00000000 00000000 01000000 00000000 = needs destination = bits 15
	// 00000000 00000011 10000000 00000000 = piece type        = bits 16-18
	// 00000000 00000100 00000000 00000000 = castle            = bit  19
	// 00000000 00001000 00000000 00000000 = pawn 2 forward    = bit  20
	// 00000000 00010000 00000000 00000000 = promotion         = bit  21
	// 00000000 11100000 00000000 00000000 = capture piece     = bits 22-24
	// 00000001 00000000 00000000 00000000 = en passant        = bit  25
	// 00000010 00000000 00000000 00000000 = side to move      = bit  26
	// 00000100 00000000 00000000 00000000 = SAN needs fyle    = bit  27
	// 00001000 00000000 00000000 00000000 = SAN needs rank    = bit  28
	// 00010000 00000000 00000000 00000000 = gives mate?       = bit  29
	// 00100000 00000000 00000000 00000000 = gives check?      = bit  30
	// 01000000 00000000 00000000 00000000 = printable?		  = bit  31
	// 10000000 00000000 00000000 00000000 = legality status   = bit  32
	uint32_t m;

	// The undo definition 'u' bitfield layout:
	// 00000000 00000000 11111111 11111111 = half move clock   = bits  1-16
	// 00000000 11111111 00000000 00000000 = prev. ep square   = bits 17-24
	// 00001111 00000000 00000000 00000000 = castling rights   = bits 25-28
	// 00010000 00000000 00000000 00000000 = ep sq exists?     = bits 29
	// 01100000 00000000 00000000 00000000 = <unused>          = bits 30-31
	// 10000000 00000000 00000000 00000000 = prepared?         = bits 32
	uint32_t u;

	static Move const m_null;
	static Move const m_empty;
	static Move const m_invalid;
};

} // namespace db

namespace mstl {

template <typename T> struct is_pod;
template <> struct is_pod<db::Move> { enum { value = 1 }; };

} // namespace mstl

#include "db_move.ipp"

#endif // _db_move_included

// vi:set ts=3 sw=3:
