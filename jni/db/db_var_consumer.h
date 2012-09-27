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

#ifndef _db_var_consumer_included
#define _db_var_consumer_included

#include "db_info_consumer.h"

#include "m_vector.h"

namespace db {

class Board;
class MoveNode;

class VarConsumer : public InfoConsumer
{
public:

	VarConsumer(Board const& startBoard, mstl::string const& encoding = mstl::string::empty_string);
	~VarConsumer() throw();

	bool notReleased() const;

	MoveNode const* result() const;
	MoveNode* release();

	format::Type format() const override;

	void start() override;
	void finish() override;

protected:

	bool beginGame(TagSet const& tags) override;
	save::State endGame(TagSet const& tags) override;

	void sendPrecedingComment(	Comment const& comment,
										Annotation const& annotation,
										MarkSet const& marks) override;
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

private:

	void sendComments(Comment const& preComment,
							Comment const& comment,
							Annotation const& annotation,
							MarkSet const& marks);

	MoveNode*	m_result;
	MoveNode*	m_current;
	bool			m_nullMoveInserted;
};

} // namespace db

#include "db_var_consumer.ipp"

#endif // _db_move_consumer_included

// vi:set ts=3 sw=3:
