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

#include "db_writer.h"
#include "db_tag_set.h"
#include "db_mark_set.h"
#include "db_annotation.h"
#include "db_move.h"
#include "db_comment.h"

#include "m_string.h"
#include "m_assert.h"
#include "m_utility.h"
#include "m_stdio.h"

#include <string.h>
#include <stdlib.h>

using namespace db;


static Comment const lostResult("The game was declared lost for both players", false, false);


Writer::Writer(format::Type srcFormat, unsigned flags, mstl::string const& encoding)
	:Consumer(srcFormat, encoding, TagBits(true), true)
	,m_flags(flags)
	,m_count(0)
	,m_level(0)
	,m_nullLevel(0)
	,m_needMoveNumber(false)
	,m_needSpace(false)
	,m_result(result::Unknown)
{
	if (test(Flag_Use_ChessBase_Format))
	{
		m_flags &= ~Flag_Convert_Lost_Result_To_Comment;
		m_flags &= ~Flag_Convert_Null_Moves_To_Comments;

		m_flags |= Flag_Use_Shredder_FEN;
		m_flags |= Flag_Append_Mode_To_Event_Type;
	}
}


mstl::string const&
Writer::conv(mstl::string const& comment)
{
	//if (codec().isUtf8())
		return comment;
/*
	codec().fromUtf8(comment, m_stringBuf);
	return m_stringBuf;*/
}


void
Writer::sendPrecedingComment(Comment const& comment, Annotation const& annotation, MarkSet const& marks)
{
	if (test(Flag_Include_Comments))
	{
		if (	!m_needSpace
			&& (annotation.contains(nag::Diagram) || annotation.contains(nag::DiagramFromBlack)))
		{
			writePrecedingComment(Comment("#", false, false), MarkSet());
			m_needSpace = true;
		}

		if (!comment.isEmpty() || !marks.isEmpty())
		{
			writePrecedingComment(comment, marks);
			m_needSpace = true;
		}
	}
}


void
Writer::sendTrailingComment(Comment const& comment, bool variationIsEmpty)
{
	if (test(Flag_Include_Comments) && !comment.isEmpty())
	{
		writeTrailingComment(comment);
		m_needSpace = true;
	}
}



void
Writer::sendComment(Comment const& comment)
{
	// Normally this method shouldn't be called

	if (test(Flag_Include_Comments) && !comment.isEmpty())
	{
		writeTrailingComment(comment);
		m_needSpace = true;
	}
}


void
Writer::sendMoveInfo(MoveInfoSet const& moveInfo)
{
	if (test(Flag_Include_Move_Info) && !moveInfo.isEmpty())
	{
		writeMoveInfo(moveInfo);
		m_needSpace = true;
	}
}


void
Writer::beginVariation()
{
	++m_level;

	if (m_nullLevel)
		++m_nullLevel;

	if (test(Flag_Include_Variations))
	{
		m_needSpace = false;
		writeBeginVariation(m_level);
		m_needMoveNumber = true;
	}
}


void
Writer::endVariation(bool)
{
	//M_REQUIRE(level() > 0);

	if (m_flags & Flag_Include_Variations)
	{
		m_needSpace = false;
		writeEndVariation(m_level);
		m_needMoveNumber = true;
		m_needSpace = true;
	}

	--m_level;

	if (m_nullLevel > 0 && --m_nullLevel == 0)
		writeEndComment();
}


void
Writer::writeTag(tag::ID tag, mstl::string const& value)
{
	writeTag(tag::toName(tag), conv(value));
}


bool
Writer::beginGame(TagSet const& tags)
{
	static mstl::string const query("?");
	static mstl::string const one("1");

	writeBeginGame(++m_count);
	m_needSpace = false;

	// TODO: the PGN standard sasy that additional tags (non-STR tags) have
	// to be exported in ASCII order by tag name.

	for (int i = 0; i < tag::ExtraTag; ++i)
	{
		bool isEmpty = !tags.contains(tag::ID(i));
		mstl::string const& value = tags.value(tag::ID(i));

		switch (i)
		{
			case tag::Event: case tag::Site: case tag::Date: case tag::Round:
				writeTag(tag::ID(i), isEmpty ? query : value);
				break;

			case tag::Result:
				m_result = result::fromString(value);

				if (m_result == result::Lost && test(Flag_Convert_Lost_Result_To_Comment))
					writeTag(tag::ID(i), result::toString(result::Unknown));
				else
					writeTag(tag::ID(i), value);
				break;

			case tag::White:
			case tag::Black:
				if (isEmpty)
					writeTag(tag::ID(i), query);
				else
					writeTag(tag::ID(i), value);
				break;

			case tag::WhiteCountry:
			case tag::BlackCountry:
				if (!isEmpty)
				{
					if (test(Flag_Use_ChessBase_Format))
						writeTag(tag::ID(i), country::toChessBaseCode(country::fromString(value)));
					else
						writeTag(tag::ID(i), value);
				}
				break;

			case tag::SetUp:
				if (!isEmpty && test(Flag_Include_Setup_Tag))
					writeTag(tag::ID(i), value);
				break;

			case tag::Fen:
				if (!isEmpty)
				{
					if (!tags.contains(tag::SetUp) && test(Flag_Include_Setup_Tag))
						writeTag(tag::toName(tag::SetUp), one);

					if (test(Flag_Use_Shredder_FEN))
					{
						mstl::string fen;
						startBoard().toFen(fen, Board::Shredder);
						writeTag(tag::ID(i), fen);
					}
					else
					{
						mstl::string fen;
						startBoard().toFen(fen, Board::XFen);
						writeTag(tag::ID(i), fen);
					}
				}
				break;

			case tag::Idn:
				if (!isEmpty && test(Flag_Include_Position_Tag))
				{
					mstl::string buf;
					buf.format(	"%s %s",
									value.c_str(),
									shuffle::position(::strtoul(value.c_str(), nullptr, 10)).c_str());
					writeTag(tag::ID(i), buf);
				}
				break;

			case tag::TimeMode:
				if (!isEmpty && test(Flag_Include_Time_Mode_Tag))
					writeTag(tag::ID(i), value);
				break;

			case tag::Variant:
				if (!isEmpty && test(Flag_Include_Variant_Tag))
					writeTag(tag::ID(i), value);
				break;

			case tag::Termination:
				if (!isEmpty && test(Flag_Include_Termination_Tag))
					writeTag(tag::ID(i), value);
				break;

			case tag::Mode:
				if (!isEmpty && test(Flag_Include_Mode_Tag))
					writeTag(tag::ID(i), value);
				break;

			case tag::EventType:
				if (!isEmpty)
				{
					if (test(Flag_Append_Mode_To_Event_Type) && tags.contains(tag::Mode))
					{
						mstl::string v = value;

						if (!v.back() == ')')
						{
							v += " (";
							v += tags.value(tag::Mode);
							v += ')';

							writeTag(tag::EventType, v);
						}
					}
					else
					{
						writeTag(tag::ID(i), value);
					}
				}
				break;

			case tag::Opening:
				if (!isEmpty && test(Flag_Include_Opening_Tag))
					writeTag(tag::ID(i), value);
				break;

			case tag::Variation:
				if (!isEmpty && test(Flag_Include_Variation_Tag))
					writeTag(tag::ID(i), value);
				break;

			case tag::SubVariation:
				if (!isEmpty && test(Flag_Include_Sub_Variation_Tag))
					writeTag(tag::ID(i), value);
				break;

			case tag::PlyCount:
				if (!isEmpty && test(Flag_Use_ChessBase_Format))
					writeTag(tag::ID(i), value);
				break;

			default:
				if (!isEmpty && !test(Flag_Exclude_Extra_Tags))
					writeTag(tag::ID(i), value);
				break;
		}
	}

	if (!test(Flag_Exclude_Extra_Tags))
	{
		for (unsigned i = 0; i < tags.countExtra(); ++i)
			writeTag(tags.extra(i).name, conv(tags.extra(i).value));
	}

	return true;
}



save::State
Writer::endGame(TagSet const&)
{
	writeEndGame();
	return save::Ok;
}


void
Writer::beginMoveSection()
{
	m_needMoveNumber = true;
	writeBeginMoveSection();
}


void
Writer::endMoveSection(result::ID result)
{
	if (m_nullLevel)
	{
		m_needSpace = false;
		--m_nullLevel;
		writeEndComment();
		m_needSpace = true;
	}

	if (result == result::Lost && test(Flag_Convert_Lost_Result_To_Comment))
	{
		sendPrecedingComment(::lostResult, Annotation(), MarkSet());
		result = result::Unknown;
	}

	m_needSpace = false;
	writeEndMoveSection(result);
}


void
Writer::writeMove(Move const& move,
						Annotation const& annotation,
						MarkSet const& marks,
						Comment const& preComment,
						Comment const& comment)
{
	if (m_level && !test(Flag_Include_Variations))
		return;

	if (!preComment.isEmpty() && test(Flag_Include_Comments))
		m_needMoveNumber = true;

	if (m_needMoveNumber)
	{
		m_moveNumber.resize(32);
		m_moveNumber.resize(::sprintf(m_moveNumber.data(), "%u.", board().moveNumber()));

		if (color::isBlack(move.color()))
			m_moveNumber += "..";
	}
	else
	{
		m_moveNumber.clear();
	}

	if (m_nullLevel == 0 && move.isNull() && test(Flag_Convert_Null_Moves_To_Comments))
	{
		writeBeginComment();
		m_nullLevel = 1;
	}

	if (!move.isPrintable())
	{
		Move m(move);
		board().prepareForPrint(m);
		writeMove(m, m_moveNumber, annotation, marks, preComment, comment);
	}
	else
	{
		writeMove(move, m_moveNumber, annotation, marks, preComment, comment);
	}

	m_needSpace = true;
	m_needMoveNumber =	color::isBlack(move.color())
							|| (!comment.isEmpty() && test(Flag_Include_Comments));
}


bool
Writer::sendMove(	Move const& move,
						Annotation const& annotation,
						MarkSet const& marks,
						Comment const& preComment,
						Comment const& comment)
{
	writeMove(move, annotation, marks, preComment, comment);
	return true;
}


bool
Writer::sendMove(Move const& move)
{
	writeMove(move, Annotation(), MarkSet(), Comment(), Comment());
	return true;
}

// vi:set ts=3 sw=3:
