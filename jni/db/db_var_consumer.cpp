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

#include "db_var_consumer.h"
#include "db_move_node.h"
#include "db_annotation.h"
#include "db_mark_set.h"
#include "db_move_info_set.h"

#include "m_assert.h"

using namespace db;


VarConsumer::VarConsumer(Board const& startBoard, mstl::string const& encoding)
	:InfoConsumer(format::Pgn, encoding, TagBits(true), true)
	,m_result(new MoveNode)
	,m_current(m_result)
	,m_nullMoveInserted(false)
{
	setStartBoard(startBoard);
}


VarConsumer::~VarConsumer() throw()
{
	delete m_result;
}


format::Type VarConsumer::format() const { return format::Pgn; }
bool VarConsumer::beginGame(TagSet const&) { return true; }
save::State VarConsumer::endGame(TagSet const&) { return save::Ok; }
void VarConsumer::start() {}
void VarConsumer::finish() {}
void VarConsumer::beginMoveSection() {}
void VarConsumer::endMoveSection(result::ID) {}


void
VarConsumer::sendComments(	Comment const& preComment,
									Comment const& comment,
									Annotation const& annotation,
									MarkSet const& marks)
{
	if (!preComment.isEmpty())
		m_current->setComment(comment, move::Ante);
	if (!comment.isEmpty())
		m_current->setComment(comment, move::Post);
	if (!annotation.isEmpty())
		m_current->replaceAnnotation(annotation);
	if (!marks.isEmpty())
		m_current->replaceMarks(marks);
}


void
VarConsumer::sendPrecedingComment(Comment const& comment,
											Annotation const& annotation,
											MarkSet const& marks)
{
	sendComments(Comment(), comment, annotation, marks);
}


void
VarConsumer::sendTrailingComment(Comment const& comment, bool variationIsEmpty)
{
	if (!comment.isEmpty())
	{
		move::Position pos = m_current->atLineStart() ? move::Ante : move::Post;

		if (variationIsEmpty)
		{
			sendMove(Move::null());
			m_nullMoveInserted = true;
		}

		if (m_current->comment(pos).isEmpty())
		{
			m_current->setComment(comment, pos);
		}
		else
		{
			Comment comm;
			m_current->swapComment(comm, pos);
			comm.append(comment, '\n');
			m_current->swapComment(comm, pos);
		}
	}
}


void
VarConsumer::sendMoveInfo(MoveInfoSet const& moveInfo)
{
	m_current->replaceMoveInfo(moveInfo);
}


bool
VarConsumer::sendMove(Move const& move)
{
	//M_REQUIRE(move);

	Board const& board = this->board();

	if (!board.isValidMove(move))
		return false;

	MoveNode* node = new MoveNode(board, move);

	m_current->setNext(node);
	m_current = node;

	return true;
}


bool
VarConsumer::sendMove(	Move const& move,
								Annotation const& annotation,
								MarkSet const& marks,
								Comment const& preComment,
								Comment const& comment)
{
	if (!sendMove(move))
		return false;

	sendComments(preComment, comment, annotation, marks);
	return true;
}


void
VarConsumer::beginVariation()
{
	MoveNode* node = new MoveNode;
	m_current->addVariation(node);
	m_current = node;
	m_nullMoveInserted = false;
}


void
VarConsumer::endVariation(bool isEmpty)
{
#ifndef ALLOW_EMPTY_VARS
	if (isEmpty && !m_nullMoveInserted)
		sendMove(Move::null());
#endif

	m_current = m_current->getLineStart();
	//M_ASSERT(m_current->prev());
	m_current = m_current->prev();
	m_nullMoveInserted = false;
}


MoveNode*
VarConsumer::release()
{
	//M_REQUIRE(notReleased());

	MoveNode* result = m_result;
	m_result = 0;
	return result;
}

// vi:set ts=3 sw=3:
