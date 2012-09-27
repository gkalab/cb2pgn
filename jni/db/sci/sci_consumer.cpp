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

#include "sci_consumer.h"
#include "sci_codec.h"
#include "sci_common.h"

#include "db_game_info.h"
#include "db_tag_set.h"
#include "db_mark_set.h"
#include "db_move_info_set.h"
#include "db_annotation.h"
#include "db_move.h"
#include "db_pgn_reader.h"

#include <ctype.h>
#include <string.h>

#ifdef NREQ
# define DEBUG(x)
#else
# define DEBUG(x) x
#endif


using namespace util;

namespace db {
namespace sci {

typedef ByteStream::uint24_t uint24_t;


Consumer::Consumer(format::Type srcFormat, Codec& codec, TagBits const& allowedTags, bool allowExtraTags)
	:Encoder(m_stream)
	,db::InfoConsumer(srcFormat, "utf-8", allowedTags, allowExtraTags)
	,m_stream(m_buffer, sizeof(m_buffer))
	,m_codec(codec)
	,m_streamPos(0)
	,m_runLength(0)
	,m_endOfRun(false)
	,m_danglingPop(false)
	,m_danglingEndMarker(0)
	,m_lastCommentPos(0)
{
}


format::Type
Consumer::format() const
{
	return format::Scidb;
}


bool
Consumer::beginGame(TagSet const& tags)
{
	if (board().notDerivableFromChess960())
		return false;

	m_stream.reset(sizeof(m_buffer));
	m_stream.resetp();
	m_data.resetp();
	m_text.resetp();
	Encoder::setup(board());
	m_streamPos = m_strm.tellp();
	m_strm << uint24_t(0);			// place holder for offset to text section
	m_strm << uint16_t(0);			// place holder for run length
	m_move = Move::empty();
	m_runLength = 0;
	m_endOfRun = false;
	m_danglingPop = false;
	m_danglingEndMarker = 1;
	m_lastCommentPos = 0;

	return true;
}

#if 0

save::State
Consumer::endGame(TagSet const& tags)
{
	TagSet const* tagSet = &tags;
	TagSet* myTags = 0;

	if (m_text.tellp() > 0 && m_lastCommentPos == plyCount() && !tags.contains(tag::Termination))
	{
		result::ID result = result::fromString(tags.value(tag::Result));

		if (result == result::White || result == result::Black)
		{
			Byte const* s = m_text.data() + m_text.tellp() - 1;

			while (s > m_text.base() && s[-1])
				--s;

			if (	::strcasecmp(reinterpret_cast<char const*>(s), "time") == 0
				|| ::strcasecmp(reinterpret_cast<char const*>(s), "time/") == 0
				|| ::strcasecmp(reinterpret_cast<char const*>(s), "time!") == 0
				|| ::strcasecmp(reinterpret_cast<char const*>(s), "time forfeit") == 0)
			{
				myTags = new TagSet(tags);
				myTags->set(tag::Termination, termination::toString(termination::TimeForfeit));
				tagSet = myTags;
			}
		}
	}

	unsigned dataOffset = m_strm.tellp();

	encodeTextSection();
	encodeDataSection(engines());
	encodeTags(*tagSet, allowedTags(), allowExtraTags());
	ByteStream::set(m_strm.base() + m_streamPos, uint24_t(dataOffset));

	m_stream.provide();
	save::State state = m_codec.addGame(m_stream, *tagSet, *this);

	if (myTags)
		delete myTags;

	return state;
}

#else

save::State
Consumer::endGame(TagSet const& tags)
{
	unsigned dataOffset = m_strm.tellp();

	encodeTextSection();
	encodeDataSection(engines());
	encodeTags(tags, allowedTags(), allowExtraTags());

	ByteStream::set(m_strm.base() + m_streamPos, uint24_t(dataOffset));
	m_stream.provide();

	return m_codec.addGame(m_stream, tags, *this);
}

#endif

void Consumer::start() {}
void Consumer::finish() {}
void Consumer::beginMoveSection() {}


void
Consumer::endMoveSection(result::ID)
{
	while (m_danglingEndMarker--)
	{
		m_strm.put(token::End_Marker);
		m_strm.put(token::End_Marker);
	}

	ByteStream::set(m_stream.base() + m_streamPos + 3, uint16_t(m_runLength));
}


Byte
Consumer::writeComment(Byte position, Comment const& comment)
{
	//M_ASSERT(position == comm::Ante || position == comm::Post);

	Byte flag = 0;

	if (!comment.isEmpty())
	{
		if (comment.engFlag())
			flag |= comm::Ante_Eng;
		if (comment.othFlag())
			flag |= comm::Ante_Oth;
		flag |= position;

		m_text.put(comment.content(), comment.size() + 1);
	}

	return flag;
}


void
Consumer::writeComment(	Comment const& preComment,
								Comment const& comment,
								Annotation const& annotation,
								MarkSet const& marks)
{
	if (!annotation.isEmpty())
	{
		for (unsigned i = 0; i < annotation.count(); ++i)
		{
			m_strm.put(token::Nag);
			m_strm.put(annotation[i]);
		}

		m_endOfRun = true;
	}

	if (!marks.isEmpty())
	{
		for (unsigned i = 0; i < marks.count(); ++i)
		{
			m_strm.put(token::Mark);
			marks[i].encode(m_data);
		}

		m_endOfRun = true;
	}

	Byte flag = writeComment(comm::Ante, preComment) | writeComment(comm::Post, comment);

	if (flag)
	{
		m_strm.put(token::Comment);
		m_data.put(flag);
		m_endOfRun = true;

		if (isMainline())
			m_lastCommentPos = plyCount() + 1;
	}
}


void
Consumer::sendPrecedingComment(	Comment const& comment,
											Annotation const& annotation,
											MarkSet const& marks)
{
	writeComment(Comment(), comment, annotation, marks);
}


void
Consumer::sendTrailingComment(Comment const& comment, bool variationIsEmpty)
{
	if (!comment.isEmpty())
	{
#ifndef ALLOW_EMPTY_VARS
		if (variationIsEmpty)
			putMove(m_move = Move::null());
#endif

		if (m_danglingEndMarker)
		{
			m_strm.put(token::End_Marker);
			m_danglingEndMarker--;
		}

		Byte flag = writeComment(comm::Post, comment);

		//M_ASSERT(flag);

		m_strm.put(token::Comment);
		m_data.put(flag);
		m_endOfRun = true;

		if (isMainline())
			m_lastCommentPos = plyCount() + 1;
	}
}


void
Consumer::sendMoveInfo(MoveInfoSet const& moveInfo)
{
	for (unsigned i = 0; i < moveInfo.count(); ++i)
	{
		m_strm.put(token::Mark);
		moveInfo[i].encode(m_data);

		if (!m_endOfRun)
		{
			m_endOfRun = true;

			if (m_runLength)
				--m_runLength;	// otherwise sci_decoder::decodeVariation() won't work
		}
	}
}


void
Consumer::preparseComment(mstl::string& comment)
{
	char const* str = comment;

	if (	str[0] == 'S'
		&& str[1] == 'P'
		&& str[2] == ' '
		&& ::isdigit(str[3])
		&& ::isdigit(str[4])
		&& ::isdigit(str[5])
		&& (str[6] == ' ' || str[6] == '\0'))
	{
		// Eliminate this silly "SP 386" comment from "Week in chess" PGN files.
		if (str[6] == '\0')
		{
			comment.clear();
			return;
		}

		str += 6;
		while (*str == ' ' || *str == '-')
			++str;

		comment.erase(comment.begin(), str);

		if (comment.empty())
			return;
	}

	InfoConsumer::preparseComment(comment);
}


void
Consumer::beginVariation()
{
	if (m_danglingPop)
	{
		m_danglingPop = false;
		m_move.clear();
	}
	else
	{
		if (m_move)
		{
			m_position.push();
			m_position.doMove(m_position.previous(), m_move);
			m_move.clear();
		}
		else
		{
			m_position.push();
		}
	}

	if (!m_endOfRun)
	{
		if (m_runLength)
			--m_runLength;	// otherwise sci_decoder::decodeVariation() won't work

		m_endOfRun = true;
	}

	m_position.push();
	m_strm.put(token::Start_Marker);
	m_danglingEndMarker++;
}


void
Consumer::endVariation(bool isEmpty)
{
#ifndef ALLOW_EMPTY_VARS
	if (isEmpty)
		putMove(Move::null());
#endif

	if (m_danglingEndMarker > 1)
	{
		if (m_danglingPop)
			m_position.pop();

		m_position.pop();
		m_strm.put(token::End_Marker);
		m_strm.put(token::End_Marker);
		m_danglingEndMarker--;
	}

	m_danglingPop = true;
}


bool
Consumer::sendMove(Move const& move)
{
	if (m_danglingPop)
	{
		m_position.pop();
		m_danglingPop = false;
	}
	else if (__builtin_expect(m_move, 1))
	{
		m_position.doMove(m_move);
	}

	if (!encodeMove(m_move = move))
		m_endOfRun = true;

	if (!m_endOfRun)
		++m_runLength;

	return true;
}


bool
Consumer::sendMove(	Move const& move,
							Annotation const& annotation,
							MarkSet const& marks,
							Comment const& preComment,
							Comment const& comment)
{
	if (m_danglingPop)
	{
		m_position.pop();
		m_danglingPop = false;
	}
	else if (__builtin_expect(m_move, 1))
	{
		m_position.doMove(m_move);
	}

	if (!encodeMove(m_move = move))
		m_endOfRun = true;

	writeComment(preComment, comment, annotation, marks);

	if (!m_endOfRun)
		++m_runLength;

	return true;
}

} // namespace sci
} // namespace db

// vi:set ts=3 sw=3:
