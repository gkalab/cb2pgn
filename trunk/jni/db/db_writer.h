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

#ifndef _db_writer_included
#define _db_writer_included

#include "db_consumer.h"
#include "db_common.h"

#include "m_string.h"

namespace db {

class Move;
class Annotation;
class MoveInfoSet;
class MarkSet;
class TagSet;
class Comment;

class Writer : public Consumer
{
public:

	static unsigned const Flag_Include_Variations						= 1 << 0;
	static unsigned const Flag_Include_Comments							= 1 << 1;
	static unsigned const Flag_Include_Annotation						= 1 << 2;
	static unsigned const Flag_Include_Move_Info							= 1 << 3;
	static unsigned const Flag_Include_Marks								= 1 << 4;
	static unsigned const Flag_Include_Termination_Tag					= 1 << 5;
	static unsigned const Flag_Include_Mode_Tag							= 1 << 6;
	static unsigned const Flag_Include_Opening_Tag						= 1 << 7;
	static unsigned const Flag_Include_Variation_Tag					= 1 << 8;
	static unsigned const Flag_Include_Sub_Variation_Tag				= 1 << 9;
	static unsigned const Flag_Include_Setup_Tag							= 1 << 10;
	static unsigned const Flag_Include_Variant_Tag						= 1 << 11;
	static unsigned const Flag_Include_Position_Tag						= 1 << 12;
	static unsigned const Flag_Include_Time_Mode_Tag					= 1 << 13;
	static unsigned const Flag_Exclude_Extra_Tags						= 1 << 14;
	static unsigned const Flag_Indent_Variations							= 1 << 15;
	static unsigned const Flag_Indent_Comments							= 1 << 16;
	static unsigned const Flag_Column_Style								= 1 << 17;
	static unsigned const Flag_Symbolic_Annotation_Style				= 1 << 18;
	static unsigned const Flag_Extended_Symbolic_Annotation_Style	= 1 << 19;
	static unsigned const Flag_Convert_Null_Moves_To_Comments		= 1 << 20;
	static unsigned const Flag_Space_After_Move_Number					= 1 << 21;
	static unsigned const Flag_Use_Shredder_FEN							= 1 << 22;
	static unsigned const Flag_Convert_Lost_Result_To_Comment		= 1 << 23;
	static unsigned const Flag_Append_Mode_To_Event_Type				= 1 << 24;
	static unsigned const Flag_Comment_To_Html							= 1 << 25;
	static unsigned const Flag_Use_ChessBase_Format						= 1 << 26;
	static unsigned const Flag_Use_Scidb_Import_Format					= 1 << 27;
	static unsigned const Flag_Write_UTF8_BOM								= 1 << 28;
	static unsigned const Flag_LAST											= Flag_Write_UTF8_BOM;

	Writer(format::Type srcFormat, unsigned flags, mstl::string const& encoding);

	bool needSpace() const;
	bool insideComment() const;
	bool test(unsigned flags) const;

	unsigned level() const;
	unsigned flags() const;

	bool beginGame(TagSet const& tags) override;
	save::State endGame(TagSet const& tags) override;

	void sendPrecedingComment(	Comment const& comment,
										Annotation const& annotation,
										MarkSet const& marks) override;
	void sendComment(Comment const& comment) override;
	void sendTrailingComment(Comment const& comment, bool variationIsEmpty) override;
	void sendMoveInfo(MoveInfoSet const& moveInfo) override;

	bool sendMove(Move const& move) override;
	bool sendMove(	Move const& move,
						Annotation const& annotation,
						MarkSet const& marks,
						Comment const& preComment,
						Comment const& comment) override;

	void beginMoveSection() override;
	void endMoveSection(result::ID result) override;

	void beginVariation() override;
	void endVariation(bool isEmpty) override;

protected:

	void addFlag(unsigned flag);
	void removeFlag(unsigned flag);

	virtual void writeBeginGame(unsigned number) = 0;
	virtual void writeEndGame() = 0;
	virtual void writePrecedingComment(Comment const& comment, MarkSet const& marks) = 0;
	virtual void writeTrailingComment(Comment const& comment) = 0;
	virtual void writeTag(mstl::string const& name, mstl::string const& value) = 0;
	virtual void writeTag(tag::ID tag, mstl::string const& value);
	virtual void writeMoveInfo(MoveInfoSet const& moveInfo) = 0;
	virtual void writeMove(	Move const& move,
									mstl::string const& moveNumber,
									Annotation const& annotation,
									MarkSet const& marks,
									Comment const& preComment,
									Comment const& comment) = 0;
	virtual void writeBeginMoveSection() = 0;
	virtual void writeEndMoveSection(result::ID result) = 0;
	virtual void writeBeginVariation(unsigned level) = 0;
	virtual void writeEndVariation(unsigned level) = 0;
	virtual void writeBeginComment() = 0;
	virtual void writeEndComment() = 0;

private:

	mstl::string const& conv(mstl::string const& comment);

	void writeMove(Move const& move,
						Annotation const& annotation,
						MarkSet const& marks,
						Comment const& preComment,
						Comment const& comment);

	unsigned			m_flags;
	unsigned			m_count;
	unsigned			m_level;
	unsigned			m_nullLevel;
	mstl::string	m_moveNumber;
	bool				m_needMoveNumber;
	bool				m_needSpace;
	result::ID		m_result;
	mstl::string	m_stringBuf;
};

} // namespace db

#include "db_writer.ipp"

#endif // _db_writer_included

// vi:set ts=3 sw=3:
