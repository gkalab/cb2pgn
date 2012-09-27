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
// The implementation is loosely based on chessx/src/database/move.h
// ======================================================================

#include "u_crc.h"

#include "m_utility.h"
#include "m_assert.h"

namespace db {

inline Move::Move() :m(0), u(0) {}
inline Move::Move(uint32_t m) :m(m), u(0) {}

inline Move::operator bool () const { return m != 0; }
inline bool Move::operator!() const { return m == 0; }

inline Move const& Move::empty()					{ return m_empty; }
inline Move const& Move::null()					{ return m_null; }
inline Move const& Move::invalid()				{ return m_invalid; }

inline Square Move::from() const					{ return m & 0x3f; }
inline Square Move::to() const					{ return (m >> 6) & 0x3f; }
inline Square Move::castlingKingFrom() const	{ return from(); }
inline Square Move::castlingRookFrom() const	{ return to(); }
inline Square Move::enPassantSquare() const	{ return from() > 31 ? to() - 8 : to() + 8; }

inline uint16_t Move::prevHalfMoves() const	{ return u; }
inline Square Move::prevEpSquare() const		{ return u >> 16; }
inline Byte Move::prevCastlingRights() const	{ return Square((u >> 24) & 0x0f); }
inline bool Move::prevEpSquareExists() const	{ return u & (1 << 28); }

inline uint32_t Move::action() const			{ return (m >> Shift_Action) & Mask_Action; }
inline uint32_t Move::capturedType() const	{ return  (m >> Shift_Capture) & Mask_PieceType; }
inline uint32_t Move::removal() const			{ return (m >> Shift_Removal) & Mask_Removal; }
inline uint32_t Move::data() const				{ return m; }
inline unsigned Move::index() const				{ return m & 0x3fff; }

inline bool Move::givesCheck() const					{ return m & Bit_Check; }
inline bool Move::givesMate() const						{ return m & Bit_Mate; }
inline bool Move::isCastling() const					{ return m & Bit_Castling; }
inline bool Move::isDoubleAdvance() const				{ return m & Bit_TwoForward; }
inline bool Move::isEmpty() const						{ return m == 0; }
inline bool Move::isEnPassant() const					{ return m & Bit_EnPassant; }
inline bool Move::isLegal() const						{ return m & Bit_Legality; }
inline bool Move::isIllegal() const						{ return (m & Bit_Legality) == 0; }
inline bool Move::isNull() const							{ return isLegal() && index() == 0; }
inline bool Move::isInvalid() const						{ return index() == Invalid; }
inline bool Move::isPromotion() const					{ return m & Bit_Promote; }
inline bool Move::isSpecial() const						{ return m & Bits_Special; }
inline bool Move::isPrintable() const					{ return m & Bit_Printable; }
inline bool Move::needsFyle() const						{ return m & Bit_Fyle; }
inline bool Move::needsRank() const						{ return m & Bit_Rank; }
inline bool Move::needsDestinationSquare() const	{ return m & Bit_Destination; }

inline void Move::clear()							{ m = 0; }
inline bool Move::preparedForUndo() const		{ return u & 0x80000000; }

inline piece::Type Move::captured() const { return piece::Type((m >> Shift_Capture) & Mask_PieceType); }
inline piece::Type Move::pieceMoved() const { return piece::Type(Mask_PieceType & (m >> Shift_Piece)); }

inline color::ID Move::color() const			{ return color::ID((m >> Shift_SideToMove) & 1); }
inline piece::ID Move::piece() const			{ return piece::piece(pieceMoved(), color()); }

inline void Move::setFrom(uint32_t from)		{ m = ((m & (~0x3f)) | from) & ~Bit_Legality; }
inline void Move::setTo(uint32_t to)			{ m = ((m & (~(0x3f << 6))) | (to << 6)) & ~Bit_Legality; }
inline void Move::setCheck()						{ m |= Bit_Check; }
inline void Move::setLegalMove()					{ m |= Bit_Legality; }
inline void Move::setIllegalMove()				{ m &= ~Bit_Legality; }
inline void Move::setMate()						{ m |= Bit_Mate; }
inline void Move::setNeedsFyle()					{ m |= Bit_Fyle; }
inline void Move::setNeedsRank()					{ m |= Bit_Rank; }
inline void Move::setNeedsDestinationSquare(){ m |= Bit_Destination; }
inline void Move::setPrintable()					{ m |= Bit_Printable; }
inline void Move::setTwoForward()				{ m |= Bit_TwoForward; }


inline
Move::Move(Square from, Square to, unsigned color)
	:m(from | (to << 6) | (color << Shift_SideToMove))
	,u(0)
{
}


inline
bool
operator==(Move const& m1, Move const& m2)
{
	return (m1.m & Move::Mask_Compare) == (m2.m & Move::Mask_Compare);
}


inline
bool
operator<(Move const& m1, Move const& m2)
{
	return (m1.m & Move::Mask_Compare) <  (m2.m & Move::Mask_Compare);
}


inline
void
Move::setLegalMove(bool flag)
{
	if (flag)
		setLegalMove();
	else
		setIllegalMove();
}


inline
bool
Move::isShortCastling() const
{
	//M_REQUIRE(isCastling());
	//M_ASSERT(from() != to());

	return from() < to();
}


inline bool
Move::isLongCastling() const
{
	//M_REQUIRE(isCastling());
	//M_ASSERT(from() != to());

	return from() > to();
}


inline
bool
Move::isCaptureOrPromotion() const
{
	static_assert(piece::None == 0, "reimplementation required");
	return ((m >> Shift_Capture) | (m & Bit_Promote)) != 0;
}


inline
Square
Move::capturedSquare() const
{
	return isEnPassant() ? enPassantSquare() : to();
}


inline
Square
Move::castlingKingTo() const
{
	return sq::make(from() < to() ? sq::FyleG : sq::FyleC, sq::rank(from()));
}


inline
Square
Move::castlingRookTo() const
{
	return sq::make(from() < to() ? sq::FyleF : sq::FyleD, sq::rank(from()));
}


inline
void
Move::setSquares(uint32_t from, uint32_t to)
{
	m = ((m & (~0xfff)) | from | (to << 6)) & ~Bit_Legality;
}


inline
void
Move::setColor(unsigned color)
{
	m |= (color << Shift_SideToMove);
}


inline
void
Move::setPieceType(uint32_t p)
{
	(m &= Clear_PieceType) |= (p & Mask_PieceType) << Shift_Piece;
}


inline
void
Move::setCaptureType(uint32_t p)
{
	(m &= Clear_CaptureType) |= (p & Mask_PieceType) << Shift_Capture;
}


inline
void
Move::setPromotionPiece(piece::Type type)
{
	//M_REQUIRE(type != piece::None && type != piece::King && type != piece::Pawn);
	//M_REQUIRE(isPromotion());

	m &= ~(Mask_Promoted << Shift_Promotion);
	m |= (uint32_t(type - 2) & Mask_Promoted) << Shift_Promotion;
}


inline
piece::ID
Move::capturedPiece() const
{
	piece::Type type = captured();
	return type == piece::None ? piece::Empty : piece::piece(type, color::opposite(color()));
}


inline
piece::Type
Move::promoted() const
{
	static_assert(piece::Queen  >= 2 && piece::Queen  <= 5, "reimplementation required");
	static_assert(piece::Rook   >= 2 && piece::Rook   <= 5, "reimplementation required");
	static_assert(piece::Bishop >= 2 && piece::Bishop <= 5, "reimplementation required");
	static_assert(piece::Knight >= 2 && piece::Knight <= 5, "reimplementation required");

	return piece::Type(((m >> Shift_Promotion) & Mask_Promoted) + 2);
}


inline
piece::ID
Move::promotedPiece() const
{
	return piece::piece(promoted(), color());
}


inline
Move
Move::genOneForward(uint32_t from, uint32_t to)
{
	return Move(from | (to << 6) | (uint32_t(piece::Pawn) << Shift_Piece));
}


inline
Move
Move::genTwoForward(uint32_t from, uint32_t to)
{
	return Move(from | (to << 6) | (uint32_t(piece::Pawn) << Shift_Piece) | Bit_TwoForward);
}


inline
Move
Move::genPromote(uint32_t from, uint32_t to, uint32_t type)
{
	//M_REQUIRE(type != piece::None && type != piece::King && type != piece::Pawn);

	return Move(	from
					 | (to << 6)
					 | ((type - 2) << Shift_Promotion)
					 | (uint32_t(piece::Pawn) << Shift_Piece)
					 | Bit_Promote);
}


inline
Move
Move::genCapturePromote(uint32_t from, uint32_t to, uint32_t type, uint32_t captured)
{
	//M_REQUIRE(type != piece::None && type != piece::King && type != piece::Pawn);

	return Move(	from
					 | (to << 6)
					 | (captured << Shift_Capture)
					 | ((type - 2) << Shift_Promotion)
					 | (uint32_t(piece::Pawn) << Shift_Piece)
					 | Bit_Promote);
}


inline
Move
Move::genEnPassant(uint32_t from, uint32_t to)
{
	return Move(	from
					 | (to << 6)
					 | (uint32_t(piece::Pawn) << Shift_Piece)
					 | (uint32_t(piece::Pawn) << Shift_Capture)
					 | Bit_EnPassant);
}


inline
Move
Move::genPawnCapture(uint32_t from, uint32_t to, uint32_t captured)
{
	//M_ASSERT(captured <= piece::Pawn);

	return Move(	from
					 | (to << 6)
					 | (captured << Shift_Capture)
					 | (uint32_t(piece::Pawn) << Shift_Piece));
}


inline
Move
Move::genKnightMove(uint32_t from, uint32_t to, uint32_t captured)
{
	//M_ASSERT(captured <= piece::Pawn);

	return Move(	from
					 | (to << 6)
					 | (captured << Shift_Capture)
					 | (uint32_t(piece::Knight) << Shift_Piece));
}


inline
Move
Move::genBishopMove(uint32_t from, uint32_t to, uint32_t captured)
{
	//M_ASSERT(captured <= piece::Pawn);

	return Move(	from
					 | (to << 6)
					 | (captured << Shift_Capture)
					 | (uint32_t(piece::Bishop) << Shift_Piece));
}


inline
Move
Move::genRookMove(uint32_t from, uint32_t to, uint32_t captured)
{
	//M_ASSERT(captured <= piece::Pawn);
	return Move(from | (to << 6) | (captured << Shift_Capture) | (uint32_t(piece::Rook) << Shift_Piece));
}


inline
Move
Move::genQueenMove(uint32_t from, uint32_t to, uint32_t captured)
{
	//M_ASSERT(captured <= piece::Pawn);
	return Move(from | (to << 6) | (captured << Shift_Capture) | (uint32_t(piece::Queen) << Shift_Piece));
}


inline
Move
Move::genKingMove(uint32_t from, uint32_t to, uint32_t captured)
{
	//M_ASSERT(captured <= piece::Pawn);
	return Move(from | (to << 6) | (captured << Shift_Capture) | (uint32_t(piece::King) << Shift_Piece));
}


inline
Move
Move::genCastling(Square from, Square to)
{
	return Move(	uint32_t(from)
					 | uint32_t(to) << 6
					 | (uint32_t(piece::King) << Shift_Piece)
					 | Bit_Castling);
}


inline
void
Move::setEnPassant()
{
	m |= Bit_EnPassant;
	setCaptureType(piece::Pawn);
}


inline
void
Move::setPromoted(uint32_t p)
{
	//M_REQUIRE(p != piece::None && p != piece::King && p != piece::Pawn);

	m &= Clear_Promote;
	m |= Bit_Promote | (((p - 2) & Mask_Promoted) << Shift_Promotion);
}


inline
void
Move::setUndo(uint32_t halfMoves, uint32_t epSquare, bool epSquareExists, uint32_t castlingRights)
{
	u = halfMoves | epSquare << 16 | epSquareExists << 28 | castlingRights << 24 | 0x80000000;
}


inline
util::crc::checksum_t
Move::computeChecksum(util::crc::checksum_t crc) const
{
	return ::util::crc::compute(crc, reinterpret_cast<char const*>(&m), sizeof(m));
}

} // namespace db

// vi:set ts=3 sw=3:
