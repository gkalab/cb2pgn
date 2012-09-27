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

#include "db_html_writer.h"
#include "db_annotation.h"
#include "db_mark_set.h"
#include "db_move.h"

#include "T_Controller.h"

using namespace db;
using namespace TeXt;


HtmlWriter::HtmlWriter(	format::Type srcFormat,
								mstl::string fname,
								mstl::string const& encoding, unsigned flags)
	:Writer(srcFormat, flags, encoding)
	,m_controller(new Controller)
	,m_fname(fname)
{
}


HtmlWriter::~HtmlWriter() throw()
{
	// only needed because of throw()
}


void
HtmlWriter::writeTag(mstl::string const& name, mstl::string const& value)
{
}


void
HtmlWriter::writeComment(Comment const& comment, MarkSet const& marks)
{
}


void
HtmlWriter::writeMove(Move const& move,
							mstl::string const& moveNumber,
							Annotation const& annotation,
							MarkSet const& marks,
							Comment const& preComment,
							Comment const& comment)
{
}


void
HtmlWriter::writeBeginGame(unsigned number)
{
}


void
HtmlWriter::writeEndGame()
{
}


void
HtmlWriter::writeBeginMoveSection()
{
}


void
HtmlWriter::writeEndMoveSection(result::ID result)
{
}


void
HtmlWriter::writeBeginVariation(unsigned level)
{
}


void
HtmlWriter::writeEndVariation(unsigned level)
{
}


void
HtmlWriter::writeBeginComment()
{
}


void
HtmlWriter::writeEndComment()
{
}


void HtmlWriter::start() {}
void HtmlWriter::finish() {}

// vi:set ts=3 sw=3:
