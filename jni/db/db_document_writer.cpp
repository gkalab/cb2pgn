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
// Copyright: (C)2011-2012 Gregor Cramer
// ======================================================================

// ======================================================================
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
// ======================================================================

#include "db_document_writer.h"

#include "m_ostream.h"
#include "m_assert.h"

#include <string.h>

using namespace db;


DocumentWriter::DocumentWriter(	format::Type srcFormat,
											unsigned flags,
											unsigned options,
											NagMap const& nagMap,
											Languages const& languages,
											unsigned significantLanguages)
	:Writer(srcFormat, flags, "utf-8")
	,m_options(options)
	,m_significant(significantLanguages)
{
	// consider these flags:
	// ----------------------
	// Flag_Include_Variations
	// Flag_Include_Annotation
	// Flag_Include_Move_Info
	// Flag_Include_Marks
	// Flag_Column_Style

	::memcpy(m_nagMap, nagMap, sizeof(nagMap));

	m_languages[0] = languages[0];
	m_languages[1] = languages[1];
	m_languages[2] = languages[2];
	m_languages[3] = languages[3];
}


void DocumentWriter::start()  {}
void DocumentWriter::finish() {}


void
DocumentWriter::writeBeginMoveSection()
{
}


void
DocumentWriter::writeEndMoveSection(result::ID result)
{
}


void
DocumentWriter::writeMove(	Move const& move,
									mstl::string const& moveNumber,
									Annotation const& annotation,
									MarkSet const& marks,
									Comment const& preComment,
									Comment const& comment)
{
}


void
DocumentWriter::writePrecedingComment(Comment const& comment, MarkSet const& marks)
{
}


void
DocumentWriter::writeTrailingComment(Comment const& comment)
{
}


void
DocumentWriter::writeMoveInfo(MoveInfoSet const& moveInfo)
{
}


void
DocumentWriter::writeBeginVariation(unsigned level)
{
}


void
DocumentWriter::writeEndVariation(unsigned level)
{
}


void
DocumentWriter::writeBeginGame(unsigned number)
{
	//M_ASSERT(!"should not be called");
}


void
DocumentWriter::writeEndGame()
{
	//M_ASSERT(!"should not be called");
}


void
DocumentWriter::writeTag(mstl::string const& name, mstl::string const& value)
{
	//M_ASSERT(!"should not be called");
}


void
DocumentWriter::writeTag(tag::ID tag, mstl::string const& value)
{
	//M_ASSERT(!"should not be called");
}


void
DocumentWriter::writeBeginComment()
{
	//M_ASSERT(!"should not be called");
}


void
DocumentWriter::writeEndComment()
{
	//M_ASSERT(!"should not be called");
}

// vi:set ts=3 sw=3:
