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

#include "db_pgn_writer.h"
#include "db_annotation.h"
#include "db_move_info_set.h"
#include "db_mark_set.h"
#include "db_move.h"
#include "db_comment.h"

#include "m_ostream.h"
#include "m_assert.h"

#include <string.h>
#include <stdlib.h>
#include <ctype.h>

using namespace db;


enum { IndentSize = 4, SpaceAfterBracket = 0 };


static mstl::string&
replaceCurlyBraces(mstl::string& s)
{
	mstl::string::size_type n = s.find_first_of("{}");

	while (n != mstl::string::npos)
	{
		s[n] = (s[n] == '{' ? '[' : ']');
		n = s.find_first_of("{}", n + 1);
	}

	return s;
}


PgnWriter::PgnWriter(format::Type srcFormat,
							mstl::ostream& strm,
							mstl::string const& encoding,
							unsigned flags,
							unsigned lineLength)
	:Writer(srcFormat, flags, encoding)
	,m_strm(strm)
	,m_lineLength(lineLength)
	,m_length(0)
	,m_pendingSpace(0)
	,m_needPreComment(false)
	,m_needPostComment(false)
	,m_hasPrecedingComment(false)
{
	if (test(Flag_Use_Scidb_Import_Format))
	{
		addFlag(Flag_Include_Variations);
		addFlag(Flag_Include_Comments);
		addFlag(Flag_Include_Annotation);
		addFlag(Flag_Include_Move_Info);
		addFlag(Flag_Include_Marks);
		addFlag(Flag_Include_Termination_Tag);
		addFlag(Flag_Include_Mode_Tag);
		addFlag(Flag_Include_Variation_Tag);
		addFlag(Flag_Include_Sub_Variation_Tag);
		addFlag(Flag_Include_Setup_Tag);
		addFlag(Flag_Include_Variant_Tag);
		addFlag(Flag_Include_Position_Tag);
		addFlag(Flag_Include_Time_Mode_Tag);
		addFlag(Flag_Comment_To_Html);

		removeFlag(Flag_Exclude_Extra_Tags);
		removeFlag(Flag_Symbolic_Annotation_Style);
		removeFlag(Flag_Extended_Symbolic_Annotation_Style);
		removeFlag(Flag_Convert_Null_Moves_To_Comments);
		removeFlag(Flag_Use_Shredder_FEN);
		removeFlag(Flag_Convert_Lost_Result_To_Comment);
		removeFlag(Flag_Append_Mode_To_Event_Type);
		removeFlag(Flag_Use_ChessBase_Format);
	}
	else if (test(Flag_Use_ChessBase_Format))
	{
		addFlag(Flag_Include_Variations);
		addFlag(Flag_Include_Comments);
		addFlag(Flag_Include_Annotation);
		addFlag(Flag_Include_Marks);
		addFlag(Flag_Include_Termination_Tag);
		addFlag(Flag_Include_Mode_Tag);
		addFlag(Flag_Include_Variation_Tag);
		addFlag(Flag_Include_Sub_Variation_Tag);
		addFlag(Flag_Include_Setup_Tag);
		addFlag(Flag_Include_Variant_Tag);
		addFlag(Flag_Include_Position_Tag);
		addFlag(Flag_Include_Time_Mode_Tag);
		addFlag(Flag_Use_Shredder_FEN);
		addFlag(Flag_Write_UTF8_BOM);

		removeFlag(Flag_Exclude_Extra_Tags);
		removeFlag(Flag_Symbolic_Annotation_Style);
		removeFlag(Flag_Extended_Symbolic_Annotation_Style);
		removeFlag(Flag_Convert_Null_Moves_To_Comments);
		removeFlag(Flag_Convert_Lost_Result_To_Comment);
		removeFlag(Flag_Append_Mode_To_Event_Type);
		removeFlag(Flag_Use_Scidb_Import_Format);
	}

	if (	test(Flag_Write_UTF8_BOM)
		&& !test(Mode_PGN_Standard)
		&& encoding == "utf-8")
	{
		m_strm.write("\xef\xbb\xbf\n"); // UTF-8 BOM
	}
}


void
PgnWriter::writeCommnentLine(mstl::string const& content)
{
	if (!content.empty())
	{
		mstl::string text;
		text = content;
		m_strm.write("; ");
		m_strm.write(content);
		m_strm.write("\n\n");
	}
}


format::Type
PgnWriter::format() const
{
	return format::Pgn;
}


void
PgnWriter::putSpace()
{
	if (m_length > 0)
		m_pendingSpace = 1;
}


void
PgnWriter::putNewline()
{
	if (m_length)
	{
		if (m_pendingSpace)
			--m_pendingSpace;

		m_length = 0;
		m_strm.put('\n');
	}

	if (test(Flag_Indent_Variations))
		m_pendingSpace = variationLevel()*::IndentSize;
	else
		m_pendingSpace = 0;
}


void
PgnWriter::putDelim(char c)
{
	if (m_length >= m_lineLength)
		putNewline();

	m_length += m_pendingSpace + 1;

	while (m_pendingSpace)
	{
		m_strm.put(' ');
		--m_pendingSpace;
	}

	m_strm.put(c);
}


void
PgnWriter::putToken(char const* s, unsigned length)
{
	if (length == 0)
		return;

	if (m_length + length >= m_lineLength)
		putNewline();

	m_length += m_pendingSpace;

	while (m_pendingSpace)
	{
		m_strm.put(' ');
		--m_pendingSpace;
	}

	m_strm.write(s, length);
	m_length += length;
}


void
PgnWriter::putToken(mstl::string const& s)
{
	putToken(s, s.size());
}


void
PgnWriter::putTokens(mstl::string const& s)
{
	char const* p = s.c_str();
	char const* q = ::strchr(p, ' ');

	while (q)
	{
		while (*q == ' ')
			++q;

		putToken(p, q - p);
		p = q;
		q = ::strchr(p, ' ');
	}

	putToken(p, s.size() - (p - s.c_str()));
}


void
PgnWriter::writeBeginGame(unsigned number)
{
	if (number > 1 || test(Flag_Append_Games))
		m_strm.put('\n');

	m_pendingSpace = 0;
	m_length = 0;
	m_needPreComment = false;
	m_needPostComment = false;
	m_hasPrecedingComment = false;

	m_move.clear();
	m_annotation.clear();
	m_marks.clear();
}


void
PgnWriter::writeEndGame()
{
	m_strm.put('\n');
}


void
PgnWriter::writeTag(mstl::string const& name, mstl::string const& value)
{
	m_strm.write("[", 1);
	m_strm.write(name, name.size());
	m_strm.write(" \"", 2);
	m_strm.write(value, value.size());
	m_strm.write("\"]\n", 3);
}


void
PgnWriter::writeBeginMoveSection()
{
	m_strm.put('\n');
	m_length = 0;
}


void
PgnWriter::writeEndMoveSection(result::ID result)
{
	putSpace();
	putToken(result::toString(result));
	m_strm.put('\n');
}


void
PgnWriter::writeBeginComment()
{
	if (!insideComment())
		putDelim('{');
}


void
PgnWriter::writeEndComment()
{
	if (!insideComment())
		putDelim('}');
}


void
PgnWriter::putComment(mstl::string const& comment, char ldelim, char rdelim)
{
	unsigned indent = mstl::numeric_limits<unsigned>::max();

	if (!insideComment())
	{
		if (m_length > 0 && test(Flag_Indent_Comments))
		{
			putNewline();
		}
		else
		{
			putSpace();
			indent = variationLevel()*::IndentSize;
		}
	}

	if (mstl::min(m_length, indent) + m_pendingSpace + comment.size() + 2 < m_lineLength)
	{
		if (m_length + m_pendingSpace + comment.size() + 2 >= m_lineLength)
			putNewline();

		m_length += m_pendingSpace;

		while (m_pendingSpace)
		{
			m_strm.put(' ');
			--m_pendingSpace;
		}

		m_strm.put(insideComment() ? ' ' : ldelim);
		m_strm.write(comment, comment.size());
		m_strm.put(insideComment() ? ' ' : rdelim);
		m_length += comment.size() + 2;
	}
	else
	{
		char const* s = comment.c_str();
		char const* e = s + comment.size();
		char const* t = s;

		while (t < e && *t == ' ')
			++t;
		while (t < e && *t != ' ')
			++t;

		unsigned length = t - s;

		if (m_length + length + m_pendingSpace + 1 + (t == e ? 1 : 0) >= m_lineLength)
		{
			if (insideComment())
				m_strm.put('\n');
			else
				putNewline();
		}

		m_length += m_pendingSpace;

		while (m_pendingSpace)
		{
			m_strm.put(' ');
			--m_pendingSpace;
		}

		if (!insideComment())
			m_strm.put(ldelim);
		else if (m_length > 0)
			m_strm.put(' ');

		m_strm.write(s, t - s);
		m_length += t - s + 1;

		if ((s = t) == e)
		{
			m_strm.put(insideComment() ? ' ' : rdelim);
			++m_length;
		}
		else
		{
			while (s < e)
			{
				unsigned spaces = 0;

				for ( ; s < e && *s == ' '; ++s)
					++spaces;

				char const* t = s + 1;

				while (t < e && *t != ' ')
					++t;

				length = t - s;

				unsigned delim = (t == e && !insideComment() ? 1 : 0);

				if (m_length + length + spaces + delim >= m_lineLength)
				{
					m_strm.put('\n');
					m_length = 0;

					if (spaces)
						--spaces;
				}

				m_length += spaces;

				while (spaces--)
					m_strm.put(' ');

				m_strm.write(s, length);
				m_length += length;

				if ((s = t) == e)
				{
					m_strm.put(insideComment() ? ' ' : rdelim);
					++m_length;
				}
			}
		}
	}

	if (test(Flag_Indent_Comments))
		putNewline();
	else
		putSpace();
}


void
PgnWriter::putComment(Comment const& comment)
{
	mstl::string text;

	if (!comment.isEmpty())
	{
		{
			comment.flatten(text, encoding::Latin1);
			//codec().fromUtf8(text);
			replaceCurlyBraces(text);
		}
	}

	putComment(text);
}


void
PgnWriter::putComment(Comment const& comment, MarkSet const& marks)
{
	if (comment.isEmpty())
	{
		if (!marks.isEmpty())
		{
			putMarks(marks);
			putComment(m_marks);
		}
	}
	else if (!marks.isEmpty())
	{
		putMarks(marks);
		Comment buf(m_marks, false, false);
		buf.append(comment, ' ');
		putComment(buf);
	}
	else if (!comment.isEmpty())
	{
		putComment(comment);
	}
}


void
PgnWriter::putMarks(MarkSet const& marks)
{
	//M_ASSERT(!marks.isEmpty());

	m_marks.clear();

	for (unsigned i = 0; i < marks.count(); ++i)
	{
		if (i > 0)
			m_marks += ' ';

		marks[i].toString(m_marks);
	}
}


void
PgnWriter::writePrecedingComment(Comment const& comment, MarkSet const& marks)
{
	if (!comment.isEmpty())
	{
		m_hasPrecedingComment = true;
		m_needPreComment = true;
	}
	else if (!marks.isEmpty())
	{
		m_needPreComment = true;
	}

	putComment(comment, marks);
}


void
PgnWriter::writeTrailingComment(Comment const& comment)
{
	if (!comment.isEmpty())
	{
		if (m_needPostComment || !m_hasPrecedingComment)
			putComment(mstl::string::empty_string);

		putComment(comment);
	}
}


void
PgnWriter::writeMoveInfo(MoveInfoSet const& moveInfo)
{
	if (!moveInfo.isEmpty())
	{
		bool needSpace = this->needSpace();

		for (unsigned i = 0; i < moveInfo.count(); ++i)
		{
			mstl::string result;

			if (needSpace)
				putSpace();

			moveInfo[i].print(engines(), result, MoveInfo::Pgn);

			if (test(Flag_Use_Scidb_Import_Format))
				putComment(result, '<', '>');
			else
				putComment(result);

			result.clear();
		}

		m_needPostComment = false;
	}
}


void
PgnWriter::writeMove(Move const& move,
							mstl::string const& moveNumber,
							Annotation const& annotation,
							MarkSet const& marks,
							Comment const& preComment,
							Comment const& comment)
{
	if (needSpace())
		putSpace();

	m_annotation.clear();
	m_move.clear();

	if (m_needPreComment || !preComment.isEmpty())
	{
		if (m_needPostComment)
			putComment(mstl::string::empty_string);

		putComment(preComment);
		m_needPreComment = false;
	}

	if (test(Flag_Space_After_Move_Number))
	{
		putToken(moveNumber);
		putSpace();
	}
	else
	{
		m_move += moveNumber;
	}

	move.printSan(m_move);

	if (!annotation.isEmpty())
	{
		unsigned f = 0;

		if (test(Flag_Symbolic_Annotation_Style))
			f |= Annotation::Flag_Symbolic_Annotation_Style;
		if (test(Flag_Extended_Symbolic_Annotation_Style))
			f |= Annotation::Flag_Extended_Symbolic_Annotation_Style;

		// XXX what should we do with non-PGN-Standard Nags (>= Pgn_Last) ?
		annotation.print(m_move, f);
	}

	putToken(m_move);
	putTokens(m_annotation);
	putSpace();

	m_hasPrecedingComment = false;

	if (comment.isEmpty() && marks.isEmpty())
	{
		m_needPostComment = true;
	}
	else
	{
		putComment(comment, marks);
		m_needPostComment = false;
	}
}


void
PgnWriter::writeBeginVariation(unsigned)
{
	if (test(Flag_Indent_Variations))
	{
		m_pendingSpace = 0;
		putNewline();
		putDelim('(');
	}
	else
	{
		putSpace();
		putDelim('(');
		m_pendingSpace = 0;
	}

	m_needPostComment = false;
	m_needPreComment = false;
	m_hasPrecedingComment = false;

	if (::SpaceAfterBracket)
		putSpace();
}


void
PgnWriter::writeEndVariation(unsigned)
{
	if (::SpaceAfterBracket)
		putSpace();

	putDelim(')');
	m_needPostComment = false;
	m_needPreComment = false;
	m_hasPrecedingComment = false;

	if (test(Flag_Indent_Variations))
	{
		putNewline();
		m_pendingSpace -= ::IndentSize;
	}
}


void PgnWriter::start() {}
void PgnWriter::finish() {}

// vi:set ts=3 sw=3:
