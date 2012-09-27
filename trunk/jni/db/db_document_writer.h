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

#ifndef _db_document_writer_included
#define _db_document_writer_included

#include "db_writer.h"

namespace db {

class DocumentWriter : public Writer
{
public:

	struct Provider
	{
		virtual ~Provider() = 0;
		virtual mstl::string const& commentForNag(nag::ID nag, mstl::string const& langID) = 0;
	};

	typedef Byte NagMap[nag::Scidb_Last];
	typedef mstl::string Languages[4];

	static unsigned const Option_Diagram_From_Whites_Perspective	= 1 << 0;
	static unsigned const Option_Diagram_From_Blacks_Perspective	= 1 << 1;
	static unsigned const Option_Diagram_Hide								= 1 << 2;
	static unsigned const Option_Diagram_Show_Mover						= 1 << 3;
	static unsigned const Option_Diagram_Show_Coordinates				= 1 << 4;

	static unsigned const Option_Moves_Notation_Short					= 1 << 5;
	static unsigned const Option_Moves_Notation_Long					= 1 << 6;
	static unsigned const Option_Moves_Notation_Algebraic				= 1 << 7;
	static unsigned const Option_Moves_Notation_Correspondence		= 1 << 8;
	static unsigned const Option_Moves_Notation_Telegraphic			= 1 << 9;

	static unsigned const Option_Annotation_Map_Unusual				= 1 << 10;
	static unsigned const Option_Annotation_Map_All						= 1 << 11;

	static unsigned const Option_Comment_All								= 1 << 12;

	DocumentWriter(format::Type srcFormat,
						unsigned flags,
						unsigned options,
						NagMap const& nagMap,
						Languages const& languages,
						unsigned significantLanguages);

protected:

	void start() override;
	void finish() override;

	void writePrecedingComment(Comment const& comment, MarkSet const& marks) override;
	void writeTrailingComment(Comment const& comment) override;
	void writeMoveInfo(MoveInfoSet const& moveInfo) override;
	void writeMove(Move const& move,
						mstl::string const& moveNumber,
						Annotation const& annotation,
						MarkSet const& marks,
						Comment const& preComment,
						Comment const& comment) override;
	void writeBeginMoveSection() override;
	void writeEndMoveSection(result::ID result) override;
	void writeBeginVariation(unsigned level) override;
	void writeEndVariation(unsigned level) override;

private:

	void writeBeginGame(unsigned number) override;
	void writeEndGame() override;
	void writeTag(mstl::string const& name, mstl::string const& value) override;
	void writeTag(tag::ID tag, mstl::string const& value) override;
	void writeBeginComment() override;
	void writeEndComment() override;

	unsigned		m_options;
	NagMap		m_nagMap;
	Languages	m_languages;
	unsigned		m_significant;
};

} // namespace db

//#include "db_document_writer.ipp"

#endif // _db_document_writer_included

// vi:set ts=3 sw=3:
