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

#include "sci_encoder.h"
#include "sci_common.h"

#include "db_move.h"
#include "db_move_node.h"
#include "db_mark_set.h"
#include "db_move_info_set.h"
#include "db_annotation.h"
#include "db_board.h"
#include "db_game_data.h"
#include "db_exception.h"

#include "m_bitfield.h"
#include "m_assert.h"

using namespace db;
using namespace db::sci;
using namespace util;


typedef ByteStream::uint24_t uint24_t;


namespace {

struct TagLookup
{
	TagLookup()
	{
		static_assert(tag::ExtraTag <= 8*sizeof(uint64_t), "BitField size exceeded");

		m_lookup.set(tag::Event);
		m_lookup.set(tag::Site);
		m_lookup.set(tag::Date);
		m_lookup.set(tag::Round);
		m_lookup.set(tag::White);
		m_lookup.set(tag::Black);
		m_lookup.set(tag::Result);
		m_lookup.set(tag::Annotator);
		m_lookup.set(tag::Eco);
		m_lookup.set(tag::WhiteElo);
		m_lookup.set(tag::BlackElo);
		m_lookup.set(tag::WhiteCountry);
		m_lookup.set(tag::BlackCountry);
		m_lookup.set(tag::WhiteTitle);
		m_lookup.set(tag::BlackTitle);
		m_lookup.set(tag::WhiteType);
		m_lookup.set(tag::BlackType);
		m_lookup.set(tag::WhiteSex);
		m_lookup.set(tag::BlackSex);
		m_lookup.set(tag::WhiteFideId);
		m_lookup.set(tag::BlackFideId);
		m_lookup.set(tag::EventDate);
		m_lookup.set(tag::EventCountry);
		m_lookup.set(tag::EventType);
		m_lookup.set(tag::Mode);
		m_lookup.set(tag::TimeMode);
		m_lookup.set(tag::Termination);
	}

	static mstl::bitfield<uint64_t> m_lookup;

	bool skipTag(tag::ID tag) const { return m_lookup.test(tag); }
};

mstl::bitfield<uint64_t> TagLookup::m_lookup;
static TagLookup tagLookup;

} // namespace


Encoder::Encoder(ByteStream& strm)
	:m_strm(strm)
	,m_data(m_buffer[0], sizeof(m_buffer[0]))
	,m_text(m_buffer[1], sizeof(m_buffer[1]))
	,m_runLength(0)
{
}


inline
Byte
Encoder::makeMoveByte(Square from, Byte value)
{
    //M_ASSERT(value <= 15);
	 //M_ASSERT(m_position[from] <= 15);

    return Byte(m_position[from] << 4) | Byte(value);
}


inline
void
Encoder::encodeNullMove(Move const&)
{
	m_strm.put(5);
}


inline
void
Encoder::encodeKing(Move const& move)
{
	// Valid King difference-from-old-square values are:
	// -9, -8, -7, -1, 1, 7, 8, 9 (w/o castling).
	// To convert this to a value in the range [1-9], we add 9 and
	// then look up the Value[] table.

	static Byte const Value[] =
	{
	//-9  -8  -7  -6  -5  -4  -3  -2  -1   0   1   2   3   4   5   6   7   8   9
		6,  7,  8,  0,  0,  0,  0,  0,  9,  0, 10,  0,  0,  0,  0,  0, 11, 12, 13
	};

	//M_ASSERT(m_position[move.from()] == 0);	// Kings MUST be piece number zero.

	Square from = move.from();

	if (move.isCastling())
	{
		m_strm.put(move.isShortCastling() ? 14 : 15);
	}
	else
	{
		unsigned diff = 9 + move.to() - from;

		// Verify we have a valid King move:
		//M_ASSERT(diff < U_NUMBER_OF(Value) && Value[diff] != 0);
		m_strm.put(Value[diff]);
	}
}


inline
void
Encoder::encodeQueen(Move const& move)
{
    // We cannot fit all Queen moves in one byte, so Rooklike moves
    // are in one byte (encoded the same way as Rook moves),
    // while diagonal moves are in two bytes.

    //M_ASSERT(move.to() <= sq::h8 && move.from() <= sq::h8);

    if (sq::rank(move.from()) == sq::rank(move.to()))
	 {
        // Rook-horizontal move
        m_strm.put(makeMoveByte(move.from(), sq::fyle(move.to())));
    }
	 else if (sq::fyle(move.from()) == sq::fyle(move.to()))
	 {
        // Rook-vertical move
        m_strm.put(makeMoveByte(move.from(), sq::rank(move.to()) + 8));
    }
	 else
	 {
        // Diagonal move:
        // First, we put a rook-horizontal move to the from square (which
        // is illegal of course) to indicate it is NOT a rooklike move.
        m_strm.put(makeMoveByte(move.from(), sq::fyle(move.from())));

        // Now we put the to-square in the next byte. We add a 64 to it
        // to make sure that it cannot clash with the Special tokens (which
        // are in the range 0 to 15, since they are special King moves).
        m_strm.put(move.to() + 64);
    }
}


inline
void
Encoder::encodeRook(Move const& move)
{
    // Valid Rook moves are to same rank, OR to same fyle.
    // We encode the 8 squares on the same rank 0-8, and the 8
    // squares on the same fyle 9-15. This means that for any particular
    // rook move, two of the values in the range [0-15] will be
    // meaningless, as they will represent the from-square.

    Byte value;

    // Check if the two squares share the same rank
    if (sq::rank(move.from()) == sq::rank(move.to()))
        value = sq::fyle(move.to());
    else
        value = sq::rank(move.to()) + 8;

    m_strm.put(makeMoveByte(move.from(), value));
}


inline
void
Encoder::encodeBishop(Move const& move)
{
    // We encode a Bishop move as the Fyle moved to, plus
    // a one-bit flag to indicate if the direction was
    // up-right/down-left or vice versa.

    Byte	value		= sq::fyle(move.to());
    int	rankDiff	= int(sq::rank(move.to())) - int(sq::rank(move.from()));
    int	fyleDiff	= int(sq::fyle(move.to())) - int(sq::fyle(move.from()));

    // If (rankdiff*fylediff) is negative, it's up-left/down-right
	 if ((rankDiff ^ fyleDiff) < 0)
		 value += 8;

    m_strm.put(makeMoveByte(move.from(), value));
}


inline
void
Encoder::encodeKnight(Move const& move)
{
	// Valid Knight difference-from-old-square values are:
	// -17, -15, -10, -6, 6, 10, 15, 17.
	// To convert this to a value in the range [1-8], we add 17 to
	// the difference and then look up the value[] table.

	static Byte const Value[] =
	{
	//-17 -16 -15 -14 -13 -12 -11 -10  -9  -8  -7  -6  -5  -4  -3  -2  -1   0
		 1,  0,  2,  0,  0,  0,  0,  3,  0,  0,  0,  4,  0,  0,  0,  0,  0,  0,
	//  1   2   3   4   5   6   7   8   9  10  11  12  13  14  15  16  17
		 0,  0,  0,  0,  0,  5,  0,  0,  0,  6,  0,  0,  0,  0,  7,  0 , 8
	};

	int diff = int(move.to()) - int(move.from());

	// Verify we have a valid knight move:
	//M_ASSERT(-17 <= diff && diff <= 17 && Value[diff + 17] != 0);
	m_strm.put(makeMoveByte(move.from(), Value[diff + 17]));
}


void
Encoder::encodePawn(Move const& move)
{
	// Pawn moves require a promotion encoding.
	// The pawn moves are:
	// 0 = capture-left,
	// 1 = forward,
	// 2 = capture-right (all no promotion);
	//         3/4/5 = 0/1/2 with Queen promo;
	//         6/7/8 = 0/1/2 with Rook promo;
	//       9/10/11 = 0/1/2 with Bishop promo;
	//      12/13/14 = 0/1/2 with Knight promo;
	// 15 = forward TWO squares.

	Byte value = 0;	// suppress gcc warning

	switch (mstl::abs(int(move.to()) - int(move.from())))
	{
		case  7:	value =  0; break;
		case  8:	value =  1; break;
		case  9: value =  2; break;
		case 16:	value = 15; break;	// move forward two squares
	}

	if (move.isPromotion())
	{
		// Handle promotions.
		// move.promotedPiece() must be Queen=2, Rook=3, Bishop=4 or Knight=5.
		// We add 3 for Queen, 6 for Rook, 9 for Bishop, 12 for Knight.

		static_assert(		piece::Queen == 2
							&& piece::Rook == 3
							&& piece::Bishop == 4
							&& piece::Knight == 5,
							"reimplementation required");

		//M_ASSERT(2 <= move.promoted() && move.promoted() <= 5);
		value += 3*(move.promoted() - 1);
	}

	m_strm.put(makeMoveByte(move.from(), value));
}


bool
Encoder::encodeMove(Move const& move)
{
	//M_ASSERT((move.pieceMoved() == piece::None) == move.isNull());

	switch (move.pieceMoved())
	{
		case piece::None:		encodeNullMove(move); break;
		case piece::King:		encodeKing(move); break;
		case piece::Queen:	encodeQueen(move); break;
		case piece::Rook:		encodeRook(move); break;
		case piece::Bishop:	encodeBishop(move); break;
		case piece::Knight:	encodeKnight(move); break;
		case piece::Pawn:		encodePawn(move); break;
	}

	if (move.isLegal())
		return true;

	m_strm.put(token::Nag);
	m_strm.put(0);

	return false;
}


void
Encoder::encodeMainline(MoveNode const* node)
{
	if (node->hasSupplement())
	{
		encodeNote(node);
		node = node->next();
	}
	else
	{
		node = node->next();

		for ( ; node->isBeforeLineEnd(); node = node->next())
		{
			if (node->hasSupplement())
				return encodeVariation(node);

			if (encodeMove(node->move()))
			{
				m_position.doMove(node->move());
				++m_runLength;
			}
			else
			{
				m_position.doMove(node->move());
				encodeVariation(node->next());
				return;
			}
		}
	}

	encodeVariation(node);
}


void
Encoder::encodeVariation(MoveNode const* node)
{
	for ( ; node->isBeforeLineEnd(); node = node->next())
	{
		encodeMove(node->move());

		if (node->hasSupplement())
		{
			if (node->hasNote())
				encodeNote(node);

			for (unsigned i = 0; i < node->variationCount(); ++i)
			{
				MoveNode const* var = node->variation(i);

				m_position.push();
				m_strm.put(token::Start_Marker);
				if (var->hasNote())
					encodeNote(var);
				encodeVariation(var->next());
				m_position.pop();
			}
		}

		m_position.doMove(node->move());
	}

	m_strm.put(token::End_Marker);

	if (node->hasComment(move::Post))
		encodeComment(node);
	else
		m_strm.put(token::End_Marker);
}


void
Encoder::encodeNote(MoveNode const* node)
{
	if (node->hasAnnotation())
	{
		Annotation const& annotation = node->annotation();

		for (unsigned i = 0; i < annotation.count(); ++i)
		{
			m_strm.put(token::Nag);
			m_strm.put(annotation[i]);
		}
	}

	if (node->hasMark())
	{
		MarkSet const& marks = node->marks();

		for (unsigned i = 0; i < marks.count(); ++i)
		{
			m_strm.put(token::Mark);
			marks[i].encode(m_data);
		}
	}

	if (node->hasMoveInfo())
	{
		MoveInfoSet const& moveInfo = node->moveInfo();

		for (unsigned i = 0; i < moveInfo.count(); ++i)
		{
			m_strm.put(token::Mark);
			moveInfo[i].encode(m_data);
		}
	}

	if (node->hasAnyComment())
		encodeComment(node);
}


void
Encoder::encodeComment(MoveNode const* node)
{
	uint8_t flag = 0;

	if (node->hasComment(move::Ante))
	{
		flag |= comm::Ante;
		if (node->comment(move::Ante).engFlag())
			flag |= comm::Ante_Eng;
		if (node->comment(move::Ante).othFlag())
			flag |= comm::Ante_Oth;
	}

	if (node->hasComment(move::Post))
	{
		flag |= comm::Post;
		if (node->comment(move::Post).engFlag())
			flag |= comm::Post_Eng;
		if (node->comment(move::Post).othFlag())
			flag |= comm::Post_Oth;
	}

	if (flag & comm::Ante)
		m_text.put(node->comment(move::Ante).content(), node->comment(move::Ante).size() + 1);

	if (flag & comm::Post)
		m_text.put(node->comment(move::Post).content(), node->comment(move::Post).size() + 1);

	m_data.put(flag);
	m_strm.put(token::Comment);
}


mstl::bitfield<uint64_t> const&
Encoder::extraTags()
{
	return ::tagLookup.m_lookup;
}


bool
Encoder::skipTag(tag::ID tag)
{
	return ::tagLookup.skipTag(tag);
}


bool
Encoder::isExtraTag(tag::ID tag)
{
	switch (int(tag))
	{
		case tag::Fen:
		case tag::Idn:
		case tag::PlyCount:
		case tag::SetUp:
		case tag::EventDate:
		case tag::WhiteElo:
		case tag::BlackElo:
			return false;

		case tag::ExtraTag:
			return true;
	}

	if (isRatingTag(tag))
		return true;

	return !skipTag(tag);
}


void
Encoder::setup(Board const& board)
{
	uint16_t idn = board.computeIdn();

	if (idn)
	{
		m_strm << uint16_t(idn);
	}
	else
	{
		m_strm << uint16_t(0);
		mstl::string fen;
		board.toFen(fen);
		m_strm.put(fen);
	}

	m_position.setup(board);
}


void
Encoder::encodeTag(TagSet const& tags, tag::ID tagID)
{
	//M_ASSERT(tagID != 0);

	mstl::string const& value = tags.value(tagID);

	m_data.put(tagID);
	m_data.put(value, value.size() + 1);
}


void
Encoder::encodeTags(TagSet const& tags, db::Consumer::TagBits allowedTags, bool allowExtraTags)
{
	unsigned pos = m_strm.tellp();

	allowedTags -= ::tagLookup.m_lookup;

	for (tag::ID tag = tags.findFirst(); tag < tag::ExtraTag; tag = tags.findNext(tag))
	{
		if (allowedTags.test(tag))
		{
			switch (tag)
			{
				// not needed
				case tag::SetUp:		break;
				case tag::Fen:			break;
				case tag::Idn:			break;
				case tag::PlyCount:	break;

				// makes the compiler shut up
				case tag::ExtraTag:	break;

				default:
					if (tags.isUserSupplied(tag) && (!tag::isRatingTag(tag) || tags.significance(tag) == 0))
						encodeTag(tags, tag);
					break;
			}
		}
	}

	if (allowExtraTags)
	{
		m_data.resetp();

		for (unsigned i = 0; i < tags.countExtra(); ++i)
		{
			mstl::string const& name  = tags.extra(i).name;
			mstl::string const& value = tags.extra(i).value;

			m_data.put(tag::ExtraTag);
			m_data.put(name, name.size() + 1);
			m_data.put(value, value.size() + 1);
		}

		m_strm.put(m_data.base(), m_data.tellp());
	}

	if (pos < m_strm.tellp())
		m_strm.put(0);
}


void
Encoder::encodeTextSection()
{
	unsigned size = m_text.tellp();

	if (size > 0)
	{
		unsigned textOffset = m_strm.tellp();

		if (textOffset < (1 << 24))
		{
			if (size < (1 << 24))
			{
				ByteStream::set(m_strm.base(), uint16_t(ByteStream::uint16(m_strm.base()) | 0x8000));

				m_strm << uint24_t(size);
				m_strm.put(m_text.base(), size);
			}
			else
			{return;
				// IO_RAISE(Game, Encoding_Failed, "text section is too large");
			}
		}
		else
		{return;
			// IO_RAISE(Game, Encoding_Failed, "move data section is too large");
		}
	}
}


void
Encoder::encodeDataSection(EngineList const& engines)
{
	if (engines.count())
	{
		m_strm.put(engines.count());
		ByteStream::set(m_strm.base(), uint16_t(ByteStream::uint16(m_strm.base()) | 0x7000));

		for (unsigned i = 0; i < engines.count(); ++i)
			m_strm.put(engines[i]);
	}

	m_strm.put(m_data.base(), m_data.tellp());
}


void
Encoder::doEncoding(	Signature const&,
							GameData const& data,
							db::Consumer::TagBits const& allowedTags,
							bool allowExtraTags)
{
	m_runLength = 0;

	setup(data.m_startBoard);

	unsigned offset = m_strm.tellp();

	m_strm << uint24_t(0);	// place holder for offset to data section
	m_strm << uint16_t(0);	// place holder for run length

	encodeMainline(data.m_startNode);

	ByteStream::set(m_strm.base() + offset + 3, uint16_t(m_runLength));

	unsigned dataOffset = m_strm.tellp();

	encodeTextSection();
	encodeDataSection(data.m_engines);
	encodeTags(data.m_tags, allowedTags, allowExtraTags);
	ByteStream::set(m_strm.base() + offset, uint24_t(dataOffset));

	m_strm.provide();
}

// vi:set ts=3 sw=3:
