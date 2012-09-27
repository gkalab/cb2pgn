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

#ifndef _sci_encoder_included
#define _sci_encoder_included

#include "sci_encoder_position.h"

#include "db_consumer.h"

#include "u_byte_stream.h"

namespace util { class ByteStream; }
namespace mstl { template <typename T> class bitfield; }

namespace db {

class Move;
class MoveNode;
class MoveInfoTable;
class EngineList;
class Signature;
class GameData;
class Board;
class TagSet;

namespace sci {

class Encoder
{
public:

	Encoder(util::ByteStream& strm);

	void doEncoding(	Signature const& signature,
							GameData const& data,
							db::Consumer::TagBits const& allowedTags,
							bool allowExtraTags);

	static bool skipTag(tag::ID tag);
	static bool isExtraTag(tag::ID tag);
	static mstl::bitfield<uint64_t> const& extraTags();

protected:

	typedef encoder::Position Position;

	void encodeNullMove(Move const& move);
	void encodeKing(Move const& move);
	void encodeQueen(Move const& move);
	void encodeRook(Move const& move);
	void encodeBishop(Move const& move);
	void encodeKnight(Move const& move);
	void encodePawn(Move const& move);

	bool encodeMove(Move const& move);
	void encodeTag(TagSet const& tags, tag::ID tagID);
	void encodeTags(TagSet const& tags, db::Consumer::TagBits allowedTags, bool allowExtraTags);
	void encodeTextSection();
	void encodeDataSection(EngineList const& engines);
	void encodeMainline(MoveNode const* node);
	void encodeVariation(MoveNode const* node);
	void encodeNote(MoveNode const* node);
	void encodeComment(MoveNode const* node);

	void setup(Board const& board);
	Byte makeMoveByte(Square from, Byte value);

	util::ByteStream&	m_strm;
	util::ByteStream	m_data;
	util::ByteStream	m_text;
	Position				m_position;
	uint16_t				m_runLength;
	unsigned char		m_buffer[2][4096];
};


} // namespace sci
} // namespace db

#endif // _sci_encoder_included

// vi:set ts=3 sw=3:
