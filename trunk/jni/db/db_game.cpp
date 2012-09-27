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

#include "db_game.h"
#include "db_game_info.h"
#include "db_move_node.h"
#include "db_annotation.h"
#include "db_mark_set.h"
#include "db_eco_table.h"
#include "db_home_pawns.h"
#include "db_edit_node.h"
#include "db_exception.h"

#include "u_crc.h"

#include "m_string.h"
#include "m_vector.h"
#include "m_stack.h"
#include "m_hash.h"
#include "m_utility.h"
#include "m_auto_ptr.h"
#include "m_stdio.h"
#include "m_assert.h"

#include <string.h>
#include <stdlib.h>
#include <ctype.h>

using namespace db;


namespace {

struct UndoApplyWatcher
{
	typedef Game::Command Command;

	Command& m_var;

	UndoApplyWatcher(Command& var, Command command)
		:m_var(var)
	{
		////M_ASSERT(var == Game::None);
		m_var = command;
	}

	~UndoApplyWatcher()
	{
		m_var = Game::None;
	}
};

} // namespace


Game::Subscriber::~Subscriber() throw() {}
bool Game::Subscriber::mainlineOnly() { return false; }


struct Game::Undo
{
	typedef UndoAction Action;
	typedef mstl::auto_ptr<MoveNode> Node;

	Undo();
	~Undo();

	void apply(Game& game);
	void clear();

	Action			action;
	Command			command;
	edit::Key		key;
	Node				node;
	Comment*			comment;
	move::Position	position;
	MarkSet*			marks;
	Annotation*		annotation;
	unsigned			varNo;
	unsigned			varNo2;
	Board				board;
};


Game::Undo::Undo()
	:command(None)
	,comment(0)
	,position(move::Post)
	,marks(0)
	,annotation(0)
	,varNo(0)
	,varNo2(0)
	,board(Board::emptyBoard())
{
}


Game::Undo::~Undo()
{
	delete comment;
	delete marks;
	delete annotation;
}


void
Game::Undo::clear()
{
	node.reset();
	key.clear();
	command = None;
	board = Board::emptyBoard();

	delete comment;
	delete marks;
	delete annotation;

	comment = 0;
	marks = 0;
	annotation = 0;
}


Game::Game()
	:Provider(format::Scidb)
	,GameData()
	,m_currentNode(m_startNode)
	,m_editNode(0)
	,m_currentBoard(m_startBoard)
	,m_idn(0)
	,m_undoIndex(0)
	,m_maxUndoLevel(0)
	,m_combinePredecessingMoves(0)
	,m_undoCommand(None)
	,m_redoCommand(None)
	,m_flags(0)
	,m_isIrreversible(false)
	,m_isModified(false)
	,m_wasModified(false)
	,m_finalBoardIsValid(false)
	,m_line(m_lineBuf[0])
	,m_linebreakThreshold(0)
	,m_linebreakMaxLineLengthMain(0)
	,m_linebreakMaxLineLengthVar(0)
	,m_linebreakMinCommentLength(0)
	,m_displayStyle(display::CompactStyle)
	,m_moveStyle(move::ShortAlgebraic)
{
}


Game::Game(Game const& game)
	:Provider(format::Scidb)
	,GameData()
	,m_editNode(0)
	,m_line(m_lineBuf[0])
{
	*this = game;
	m_tags = game.m_tags;
	m_engines = game.m_engines;
}


Game::~Game() throw()
{
	delete m_editNode;

	for (unsigned i = 0; i < m_undoList.size(); ++i)
		delete m_undoList[i];
}


Game&
Game::operator=(Game const& game)
{
	if (this != &game)
	{
		m_startNode							= game.m_startNode ? game.m_startNode->clone() : 0;
		m_startBoard						= game.m_startBoard;
		m_currentBoard						= game.m_startBoard;
		m_finalBoard						= game.m_finalBoard;
		m_currentNode						= m_startNode;
		m_editNode							= 0;
		m_currentKey						= edit::Key(game.m_startBoard.plyNumber());
		m_idn									= game.m_idn;
		m_eco									= game.m_eco;
		m_languageSet						= game.m_languageSet;
		m_wantedLanguages					= game.m_wantedLanguages;
		m_isIrreversible					= false;
		m_isModified						= false;
		m_wasModified						= false;
		m_finalBoardIsValid				= false;
		m_subscriber						= game.m_subscriber;
		m_undoIndex							= 0;
		m_maxUndoLevel						= game.m_maxUndoLevel;
		m_combinePredecessingMoves		= game.m_combinePredecessingMoves;
		m_undoCommand						= None;
		m_redoCommand						= None;
		m_flags								= game.m_flags;
		m_linebreakThreshold				= game.m_linebreakThreshold;
		m_linebreakMaxLineLengthMain	= game.m_linebreakMaxLineLengthMain;
		m_linebreakMaxLineLengthVar	= game.m_linebreakMaxLineLengthVar;
		m_linebreakMinCommentLength	= game.m_linebreakMinCommentLength;
		m_displayStyle						= game.m_displayStyle;
		m_moveStyle							= game.m_moveStyle;

		m_line.copy(game.m_line);

		for (unsigned i = 0; i < m_undoList.size(); ++i)
			delete m_undoList[i];

		m_undoList.clear();
	}

	return *this;
}


Game::Undo&
Game::newUndo(UndoAction action, Command command)
{
	//M_ASSERT(m_maxUndoLevel > 0);

	if (m_redoCommand != None)
	{
		//M_ASSERT(m_undoIndex < m_undoList.size());
		++m_undoIndex;
	}
	else if (m_undoCommand == None)
	{
		if (m_undoIndex < m_undoList.size())
		{
			m_undoList.resize(++m_undoIndex);
		}
		else if (m_undoList.size() == m_maxUndoLevel)
		{
			Undo* undo = m_undoList.front();
			m_undoList.pop_front();
			m_undoList.push_back(undo);
			undo->clear();
			m_isIrreversible = true;
		}
		else
		{
			m_undoList.push_back(new Undo);
			++m_undoIndex;
		}
	}

	//M_ASSERT(m_undoIndex > 0);
	//M_ASSERT(m_undoList[m_undoIndex - 1]);

	Undo& undo = *m_undoList[m_undoIndex - 1];
	undo.action = action;
	undo.key = m_currentKey;

	if (m_undoCommand != None)
	{
		if (--m_undoIndex == 0 && !m_isIrreversible)
			m_isModified = false;
	}
	else if (m_redoCommand == None)
	{
		undo.command = command;
		m_isModified = true;
	}

	return undo;
}


Game::Undo*
Game::prevUndo(unsigned back)
{
	//M_ASSERT(m_maxUndoLevel > 0);
	//M_ASSERT(back > 0);

	if (m_undoCommand != None)
		return m_undoIndex + 1 - back < m_undoList.size() ? m_undoList[m_undoIndex - back] : 0;

	if (m_undoIndex < back)
		return 0;

	return m_undoList[m_undoIndex - back];
}


void
Game::insertUndo(UndoAction action, Command command)
{
	//M_ASSERT(action == Unstrip_Moves || action == Truncate_Variation || action == Remove_Mainline);

	if (m_maxUndoLevel)
	{
		Undo* prev = prevUndo();

		if (prev && prev->command == AddMove)
		{
			// Combine precedent AddMove's, in this way
			// we prevent a quick fill of the undo slots.

			Undo* pprev = prevUndo(2);

			if (	pprev
				&& (pprev->command == AddMove || pprev->command == AddMoves)
				&& pprev->key.level() == m_currentKey.level()
				&& unsigned(m_currentKey.computeDistance(pprev->key)) <= m_combinePredecessingMoves)
			{
				prev->key = m_currentKey;
				pprev->command = AddMoves;
				return;
			}
		}

		newUndo(action, command);
	}
	else
	{
		m_isIrreversible = true;
	}
}


void
Game::insertUndo(	UndoAction action,
						Command command,
						Comment const& oldComment,
						Comment const& newComment,
						move::Position position)
{
	//M_ASSERT(action == Set_Annotation);

	if (m_maxUndoLevel)
	{
		Undo* prev = prevUndo();

		if (	prev == 0
			|| prev->action != action
			|| prev->key != m_currentKey
			|| prev->position != position)
		{
			Undo& undo = newUndo(action, command);
			undo.comment = new Comment(oldComment);
			undo.position = position;
		}
		else if (prev->comment == 0)
		{
			prev->comment = new Comment(oldComment);
			prev->position = position;
		}
		else if (m_undoCommand != None)
		{
			Undo& undo = newUndo(action, command);
			undo.comment = new Comment(oldComment);
			undo.position = position;
		}
		else if (	prev->position == position
					&& *prev->comment == newComment
					&& (prev->annotation == 0 || *prev->annotation == m_currentNode->annotation())
					&& (prev->marks == 0 || *prev->marks == m_currentNode->marks()))
		{
			prev->clear();

			if (--m_undoIndex == 0 && !m_isIrreversible)
				m_isModified = false;
		}
	}
	else
	{
		m_isIrreversible = true;
	}
}


void
Game::insertUndo(	UndoAction action,
						Command command,
						Comment const& oldComment,
						Comment const& newComment)
{
	//M_ASSERT(action == Set_Trailing_Comment);

	if (m_maxUndoLevel)
	{
		Undo* prev = prevUndo();

		if (prev == 0 || prev->action != action)
		{
			Undo& undo = newUndo(action, command);
			undo.comment = new Comment(oldComment);
		}
		else if (prev->comment == 0)
		{
			prev->comment = new Comment(oldComment);
		}
		else if (m_undoCommand != None)
		{
			Undo& undo = newUndo(action, command);
			undo.comment = new Comment(oldComment);
		}
		else if (*prev->comment == newComment)
		{
			prev->clear();

			if (--m_undoIndex == 0 && !m_isIrreversible)
				m_isModified = false;
		}
	}
	else
	{
		m_isIrreversible = true;
	}
}


void
Game::insertUndo(UndoAction action, Command command, MarkSet const& oldMarks, MarkSet const& newMarks)
{
	//M_ASSERT(action == Set_Annotation);

	if (m_maxUndoLevel)
	{
		Undo* prev = prevUndo();

		if (prev == 0 || prev->action != action || prev->key != m_currentKey)
		{
			newUndo(action, command).marks = new MarkSet(oldMarks);
		}
		else if (prev->marks == 0)
		{
			prev->marks = new MarkSet(oldMarks);
		}
		else if (m_undoCommand != None)
		{
			newUndo(action, command).marks = new MarkSet(oldMarks);
		}
		else if (	*prev->marks == newMarks
					&& (prev->comment == 0 || *prev->comment == m_currentNode->comment(prev->position))
					&& (prev->annotation == 0 || *prev->annotation == m_currentNode->annotation()))
		{
			prev->clear();

			if (--m_undoIndex == 0 && !m_isIrreversible)
				m_isModified = false;
		}
	}
	else
	{
		m_isIrreversible = true;
	}
}


void
Game::insertUndo(	UndoAction action,
						Command command,
						Annotation const& oldAnnotation,
						Annotation const& newAnnotation)
{
	//M_ASSERT(action == Set_Annotation);

	if (m_maxUndoLevel)
	{
		Undo* prev = prevUndo();

		if (prev == 0 || prev->action != action || prev->key != m_currentKey)
		{
			newUndo(action, command).annotation = new Annotation(oldAnnotation);
		}
		else if (prev->annotation == 0)
		{
			prev->annotation = new Annotation(oldAnnotation);
		}
		else if (m_undoCommand != None)
		{
			newUndo(action, command).annotation = new Annotation(oldAnnotation);
		}
		else if (	*prev->annotation == newAnnotation
					&& (prev->comment == 0 || *prev->comment == m_currentNode->comment(prev->position))
					&& (prev->marks == 0 || *prev->marks == m_currentNode->marks()))
		{
			prev->clear();

			if (--m_undoIndex == 0 && !m_isIrreversible)
				m_isModified = false;
		}
	}
	else
	{
		m_isIrreversible = true;
	}
}


void
Game::insertUndo(UndoAction action, Command command, MoveNode* node)
{
	//M_ASSERT(action == Replace_Node || action == Revert_Game);

	if (m_maxUndoLevel)
	{
		Undo* prev = prevUndo();

		if (prev == 0 || prev->command != command || command != StripComments)
			newUndo(action, command).node = node;
		else
			prev->node.reset(node);
	}
	else
	{
		m_isIrreversible = true;
		delete node;
	}
}


void
Game::insertUndo(UndoAction action, Command command, MoveNode* node, unsigned varNo)
{
	//M_ASSERT(action == Insert_Variation || action == New_Mainline);

	if (m_maxUndoLevel)
	{
		Undo& undo = newUndo(action, command);
		undo.node = node;
		undo.varNo = varNo;
	}
	else
	{
		m_isIrreversible = true;
		delete node;
	}
}


void
Game::insertUndo(UndoAction action, Command command, unsigned varNo)
{
	//M_ASSERT(action == Remove_Variation);

	if (m_maxUndoLevel)
		newUndo(action, command).varNo = varNo;
	else
		m_isIrreversible = true;
}


void
Game::insertUndo(UndoAction action, Command command, unsigned varNo1, unsigned varNo2)
{
	//M_ASSERT(action == Swap_Variations || action == Promote_Variation);

	if (m_maxUndoLevel)
	{
		Undo& undo = newUndo(action, command);
		undo.varNo = varNo1;
		undo.varNo2 = varNo2;
	}
	else
	{
		m_isIrreversible = true;
	}
}


void
Game::insertUndo(UndoAction action, Command command, MoveNode* node, Board const& board)
{
	//M_ASSERT(action == Strip_Moves || action == Set_Start_Position);

	if (m_maxUndoLevel)
	{
		Undo& undo = newUndo(action, command);
		undo.node = node;
		undo.board = board;
	}
	else
	{
		m_isIrreversible = true;
	}
}


Game::Command
Game::undoCommand() const
{
	return m_undoIndex == 0 ? None : m_undoList[m_undoIndex - 1]->command;
}


Game::Command
Game::redoCommand() const
{
	return m_undoIndex < m_undoList.size() ? m_undoList[m_undoIndex]->command : None;
}


void
Game::clearUndo()
{
	m_undoList.clear();
	m_undoIndex= 0;
}


void
Game::applyUndo(Undo& undo, bool redo)
{
	if (redo || undo.action != Set_Start_Position)
		tryMoveTo(undo.key);

	switch (undo.action)
	{
		case Replace_Node:			replaceNode(undo.node.release(), undo.command); break;
		case Truncate_Variation:	truncateVariation(move::Post); break;
		case Swap_Variations:		moveVariation(undo.varNo, undo.varNo2, undo.command); break;
		case Insert_Variation:		insertVariation(undo.node.release(), undo.varNo); break;
		case Promote_Variation:		promoteVariation(undo.varNo, undo.varNo2); break;
		case Remove_Variation:		removeVariation(undo.varNo); break;
		case Remove_Mainline:		removeMainline(); break;
		case New_Mainline:			newMainline(undo.node.release()); break;
		case Strip_Moves:				unstripMoves(undo.node.release(), undo.board, undo.key); break;
		case Unstrip_Moves:			stripMoves(move::Post); break;
		case Revert_Game:				revertGame(undo.node.release(), undo.command); break;
		case Set_Start_Position:	resetGame(undo.node.release(), undo.board, undo.key.id()); break;

		case Set_Annotation:
			{
				unsigned flags = UpdatePgn;

				if (undo.annotation)
				{
					Annotation annotation(*undo.annotation);
					insertUndo(Set_Annotation, SetAnnotation, m_currentNode->annotation(), annotation);
					m_currentNode->replaceAnnotation(annotation);
				}
				if (undo.comment)
				{
					Comment comment(*undo.comment);
					insertUndo(	Set_Annotation,
									SetAnnotation,
									m_currentNode->comment(undo.position),
									comment,
									undo.position);
					m_currentNode->swapComment(comment, undo.position);
					if (updateLanguageSet())
						flags |= UpdateLanguageSet;
				}
				if (undo.marks)
				{
					MarkSet marks(*undo.marks);
					insertUndo(Set_Annotation, SetAnnotation, m_currentNode->marks(), marks);
					m_currentNode->replaceMarks(marks);
				}

				updateSubscriber(flags);
			}
			break;

		case Set_Trailing_Comment:
			{
				unsigned flags = UpdatePgn;
				Comment comment(*undo.comment);
				insertUndo(	Set_Trailing_Comment,
								SetAnnotation,
								m_currentNode->getLineEnd()->comment(undo.position),
								comment);
				m_currentNode->getLineEnd()->swapComment(comment, undo.position);
				if (updateLanguageSet())
					flags |= UpdateLanguageSet;
				updateSubscriber(flags);
			}
			break;
	}

	if (!redo || undo.action != Set_Start_Position)
		tryMoveTo(undo.key);

	switch (int(undo.action))
	{
		case Promote_Variation:
		case Remove_Variation:
		case Insert_Variation:
			backward();
			break;
	}

	if (redo)
	{
		switch (int(undo.command))
		{
			case AddMove:
			case AddMoves:
			case ReplaceVariation:
				forward();
				break;
		}
	}

	goToCurrentMove();
}


void
Game::undo()
{
	//M_REQUIRE(hasUndo());

	Undo& undo = *m_undoList[m_undoIndex - 1];
	UndoApplyWatcher watcher(m_undoCommand, undo.command);
	applyUndo(undo, false);
}


void
Game::redo()
{
	//M_REQUIRE(hasRedo());

	Undo& redo = *m_undoList[m_undoIndex];
	UndoApplyWatcher watcher(m_redoCommand, redo.command);
	m_isModified = true;
	applyUndo(redo, true);
}



bool
Game::containsIllegalMoves() const
{
	return m_flags & GameInfo::Flag_Illegal_Move;
}


bool
Game::containsIllegalCastlings() const
{
	return m_flags & GameInfo::Flag_Illegal_Castling;
}


bool
Game::isEmpty() const
{
	return m_startNode->isEmptyLine();
}


bool
Game::atMainlineEnd() const
{
	return isMainline() && m_currentNode->isOneBeforeLineEnd();
}


bool
Game::atLineStart() const
{
	return m_currentNode->atLineStart();
}


bool
Game::atLineEnd() const
{
	return m_currentNode->isOneBeforeLineEnd();
}


bool
Game::isBeforeLineEnd() const
{
	return m_currentNode->next() && m_currentNode->next()->isBeforeLineEnd();
}


bool
Game::isFirstVariation() const
{
	if (m_currentKey.level() == 0)
		return false;

	MoveNode* node = m_currentNode;

	while (!node->atLineStart())
		node = node->prev();

	MoveNode*	prev	= node->prev();
	unsigned		n		= prev->variationNumber(node);

	while (n > 0 && prev->variation(n - 1)->isEmptyLine())
		--n;

	return n == 0;
}


bool
Game::isLastVariation() const
{
	if (m_currentKey.level() == 0)
		return true;

	MoveNode* node = m_currentNode;

	while (!node->atLineStart())
		node = node->prev();

	MoveNode*	prev	= node->prev();
	unsigned		n		= prev->variationNumber(node) + 1;

	while (n < prev->variationCount() && prev->variation(n)->isEmptyLine())
		++n;

	return n == prev->variationCount();
}


Board const&
Game::getFinalBoard() const
{
	//M_ASSERT(m_startNode->next());

	if (!m_finalBoardIsValid)
	{
		HomePawns hp;

		m_finalBoard = m_startBoard;

		for (MoveNode const* node = m_startNode->next(); node->isBeforeLineEnd(); node = node->next())
		{
			m_finalBoard.doMove(node->move());
			hp.move(node->move());
		}

		m_finalBoard.signature().setHomePawns(hp.used(), hp.data());
		m_finalBoardIsValid = true;
	}

	return m_finalBoard;
}


Board const&
Game::getStartBoard() const
{
	return startBoard();
}

Move const&
Game::currentMove() const
{
	return m_currentNode->move();
}


Move const&
Game::nextMove() const
{
	return isBeforeLineEnd() ? m_currentNode->next()->move() : Move::empty();
}


mstl::string
Game::startKey() const
{
	return edit::Key(m_startBoard.plyNumber()).id();
}


mstl::string
Game::successorKey() const
{
	return m_currentKey.successorKey(m_currentNode).id();
}


Comment const&
Game::comment(move::Position position) const
{
	return m_currentNode->comment(position);
}


Comment const&
Game::trailingComment() const
{
	return m_currentNode->getLineEnd()->comment(move::Post);
}


mstl::string&
Game::infix(mstl::string& result) const
{
	return m_currentNode->annotation().infix(result);
}


mstl::string&
Game::prefix(mstl::string& result) const
{
	return m_currentNode->annotation().prefix(result);
}


mstl::string&
Game::suffix(mstl::string& result) const
{
	return m_currentNode->annotation().suffix(result);
}


MarkSet const&
Game::marks() const
{
	return m_currentNode->marks();
}


MarkSet const&
Game::marks(edit::Key const& key) const
{
	//M_REQUIRE(isValidKey(key));

	MoveNode* node = key.findPosition(m_startNode, m_startBoard.plyNumber());

	//M_ASSERT(node);
	return node->marks();
}


mstl::string&
Game::printMove(	Board const& board,
						MoveNode* node,
						mstl::string& result,
						unsigned flags,
						move::Notation form)
{
	//M_ASSERT(node);

	Move& move = node->move();

	if (!move)
		return result;

	//M_ASSERT(move.isPrintable());

	// move number
	if (board.blackToMove())
	{
		if (flags & BlackNumbers)
		{
			result.format("%u", board.moveNumber());
			result += "...";

			if (flags & ExportFormat)
				result += " ";
			else if (flags & UseZeroWidthSpace)
				result += "\xE2\x80\x8B";
			else if (!(flags & SuppressSpace))
				result += " ";
		}
	}
	else
	{
		if (flags & WhiteNumbers)
		{
			result.format("%u", board.moveNumber());
			result += '.';

			if (flags & ExportFormat)
				result += " ";
			else if (flags & UseZeroWidthSpace)
				result += "\xE2\x80\x8B";
			else if (!(flags & SuppressSpace))
				result += " ";
		}
	}

	// move
	switch (form)
	{
		case move::Algebraic:
			move.printAlgebraic(result);
			break;
		case move::ShortAlgebraic:
			move.printSan(result, flags & ExportFormat ? encoding::Latin1 : encoding::Utf8);
			break;
		case move::LongAlgebraic:
			move.printLan(result, flags & ExportFormat ? encoding::Latin1 : encoding::Utf8);
			break;
		case move::Descriptive:
			move.printDescriptive(result);
			break;
		case move::Correspondence:
			move.printNumeric(result);
			break;
		case move::Telegraphic:
			move.printAlphabetic(result);
			break;
	}

	if (	!(flags & ExportFormat)
		&& form != move::Descriptive
		&& !move.givesMate()
		&& board.isDoubleCheck())
	{
		result += '+';
	}

	// annotation
	if (flags & IncludeAnnotation)
	{
		node->annotation().print(
			result,
			flags & ExportFormat ? 0 : Annotation::Flag_Extended_Symbolic_Annotation_Style);
	}

	return result;
}


mstl::string&
Game::printMove(mstl::string& result, unsigned flags, move::Notation style) const
{
	return printMove(m_currentBoard, m_currentNode, result, flags, style);
}


mstl::string
Game::getNextMove(unsigned flags)
{
	//M_ASSERT(isBeforeLineEnd());

	mstl::string san;

	m_currentNode = m_currentNode->next();
	doMove();
	printMove(san, flags);
	undoMove();
	m_currentNode = m_currentNode->prev();

	return san;
}


void
Game::setComment(mstl::string const& comment, move::Position position)
{
	//M_REQUIRE(position == move::Post || !atLineStart());

	Comment comm(comment, false, false);
	comm.normalize();

	if (comm != m_currentNode->comment(position))
	{
		insertUndo(Set_Annotation, SetAnnotation, m_currentNode->comment(position), comm, position);
		m_currentNode->setComment(comm, position);

		unsigned flags = UpdatePgn | UpdateBoard;

		if (updateLanguageSet())
			flags |= UpdateLanguageSet;

		updateSubscriber(flags);
	}
}


void
Game::setTrailingComment(mstl::string const& comment)
{
	Comment comm(comment, false, false);
	comm.normalize();

	MoveNode* node = m_currentNode->getLineEnd();

	if (comm != node->comment(move::Post))
	{
		insertUndo(Set_Trailing_Comment, SetAnnotation, node->comment(move::Post), comm);
		node->setComment(comm, move::Post);

		unsigned flags = UpdatePgn | UpdateBoard;

		if (updateLanguageSet())
			flags |= UpdateLanguageSet;

		updateSubscriber(flags);
	}
}


void
Game::setMarks(MarkSet const& marks)
{
	if (marks != m_currentNode->marks())
	{
		insertUndo(Set_Annotation, SetAnnotation, m_currentNode->marks(), marks);
		m_currentNode->replaceMarks(marks);

		if (m_subscriber)
		{
			mstl::string s;
			m_subscriber->updateMarks(m_currentNode->marks().print(s));
			updateSubscriber(UpdatePgn);
		}
	}
}


void
Game::setAnnotation(Annotation const& annotation)
{
	if (annotation == m_currentNode->annotation())
		return;

	insertUndo(Set_Annotation, SetAnnotation, m_currentNode->annotation(), annotation);
	m_currentNode->replaceAnnotation(annotation);
	updateSubscriber(UpdatePgn | UpdateBoard);
}


void
Game::doMove()
{
	m_currentBoard.doMove(m_currentNode->move());
	m_currentKey.exchangePly(m_currentBoard.plyNumber());
}


void
Game::undoMove()
{
	m_currentBoard.undoMove(m_currentNode->move());
	m_currentKey.exchangePly(m_currentBoard.plyNumber());
}


void
Game::tryMoveTo(edit::Key const& key)
{
	//M_REQUIRE(isValidKey(key));

	edit::Key wantedKey(key);
	edit::Key currentKey(m_currentKey);

	moveToMainlineStart();

	if (!wantedKey.setPosition(*this))
		currentKey.setPosition(*this);
}


bool
Game::forward()
{
	if (atLineEnd())
		return false;

	//M_ASSERT(isBeforeLineEnd());

	m_currentNode = m_currentNode->next();
	doMove();
	return true;
}


bool
Game::backward()
{
	if (atLineStart())
		return false;

	undoMove();
	m_currentNode = m_currentNode->prev();

	//M_ASSERT(m_currentNode);

	return true;
}


unsigned
Game::forward(unsigned count)
{
	unsigned n = 0;

	while (n < count && forward())
		++n;

	return n;
}


unsigned
Game::backward(unsigned count)
{
	unsigned n = 0;

	while (n < count && backward())
		++n;

	return n;
}


void
Game::moveToStart()
{
	while (backward())
		;
}


void
Game::moveToEnd()
{
	while (forward())
		;
}


void
Game::moveToMainlineStart()
{
	m_currentKey.reset(m_startBoard.plyNumber());
	m_currentNode = m_startNode;
	m_currentBoard = m_startBoard;
}


void
Game::moveToMainlineEnd()
{
	if (!isMainline())
		moveToMainlineStart();

	moveToEnd();
}


void
Game::exitToMainline()
{
	while (isVariation())
		exitVariation();
}


void
Game::goToCurrentMove() const
{
	if (m_subscriber)
	{
		m_subscriber->boardSetup(m_currentBoard);
		m_subscriber->gotoMove(m_currentKey.id(), successorKey());
	}
}


void
Game::goToCurrentMove(bool forward) const
{
	//M_ASSERT(forward || isBeforeLineEnd());

	if (m_subscriber)
	{
		Move move = forward ? m_currentNode->move() : m_currentNode->next()->move();

		m_subscriber->boardMove(m_currentBoard, move, forward);
		m_subscriber->gotoMove(m_currentKey.id(), successorKey());
	}
}


void
Game::goToPosition(mstl::string const& fen)
{
	Board position;
	position.setup(fen);

	moveToMainlineStart();

	while (!atMainlineEnd() && !position.isEqualPosition(m_currentBoard))
		forward();

	goToCurrentMove();
}


void
Game::moveTo(edit::Key const& key)
{
	//M_REQUIRE(isValidKey(key));

	edit::Key wantedKey(key);
	edit::Key currentKey(m_currentKey);

	moveToMainlineStart();

	if (!wantedKey.setPosition(*this))
	{
		currentKey.setPosition(*this);
		//DB_RAISE("invalid key '%s'", key.id().c_str());
	}
}


void
Game::moveTo(mstl::string const& key)
{
	//M_REQUIRE(edit::Key::isValid(key));
	moveTo(edit::Key(key));
}


void
Game::getMoves(StringList& result, unsigned flags)
{
	result.clear();

	if (atLineEnd())
		return;

	forward();
	result.push_back();
	printMove(result.back(), flags);

	for (unsigned i = 0; i < m_currentNode->variationCount(); ++i)
	{
		enterVariation(i);

		if (isBeforeLineEnd())
		{
			forward();
			result.push_back();
			printMove(result.back(), flags);
		}

		exitVariation();
	}

	backward();
}


void
Game::getKeys(StringList& result)
{
	result.clear();

	if (atLineEnd())
		return;

	forward();
	result.push_back(m_currentKey.id());

	for (unsigned i = 0; i < m_currentNode->variationCount(); ++i)
	{
		enterVariation(i);

		if (isBeforeLineEnd())
		{
			forward();
			result.push_back(m_currentKey.id());
		}

		exitVariation();
	}

	backward();
}


unsigned
Game::variationCount() const
{
	return m_currentNode->variationCount();
}


unsigned
Game::subVariationCount() const
{
	return isBeforeLineEnd() ? m_currentNode->next()->variationCount() : 0;
}


unsigned
Game::countLength() const
{
	return m_startNode->countHalfMoves();
}


unsigned
Game::countHalfMoves() const
{
	return m_currentNode->countHalfMoves();
}


unsigned
Game::countHalfMoves(unsigned varNo) const
{
	//M_REQUIRE(varNo < variationCount());
	return m_currentNode->variation(varNo)->countHalfMoves();
}


unsigned
Game::lengthOfCurrentLine() const
{
	return m_currentNode->getLineStart()->countHalfMoves();
}


unsigned
Game::countAnnotations() const
{
	return m_startNode->countAnnotations();
}


unsigned
Game::countMoveInfo() const
{
	return m_startNode->countMoveInfo();
}


unsigned
Game::countMarks() const
{
	return m_startNode->countMarks();
}


unsigned
Game::countComments() const
{
	return m_startNode->countComments();
}


unsigned
Game::countVariations() const
{
	return m_startNode->countVariations();
}


unsigned
Game::variationNumber() const
{
	//M_REQUIRE(isVariation());

	if (m_currentKey.level() == 0)
		return 0;

	MoveNode* node = m_currentNode->getLineStart();
	MoveNode* prev = node->prev();
	return prev->variationNumber(node);
}


unsigned
Game::plyCount() const
{
	return getFinalBoard().plyNumber() - m_startBoard.plyNumber();
}


unsigned
Game::plyNumber() const
{
	return m_currentBoard.plyNumber();
}


unsigned
Game::moveNumber() const
{
	return m_currentBoard.moveNumber();
}


void
Game::enterVariation(unsigned variationNumber)
{
	//M_REQUIRE(variationNumber < variationCount());

	m_currentKey.addVariation(variationNumber);
	m_currentKey.addPly(m_currentBoard.plyNumber());
	undoMove();
	m_currentNode = m_currentNode->variation(variationNumber);
}


void
Game::exitVariation()
{
	//M_REQUIRE(isVariation());

	moveToStart();
	m_currentNode = m_currentNode->prev();
	m_currentKey.removePly();
	m_currentKey.removeVariation();
	doMove();

	//M_ASSERT(m_currentNode);
}


void
Game::goToMainlineStart()
{
	if (!atMainlineStart())
	{
		moveToMainlineStart();
		goToCurrentMove();
	}
}


void
Game::goToMainlineEnd()
{
	if (!atMainlineEnd())
	{
		moveToMainlineEnd();
		goToCurrentMove();
	}
}


void
Game::goToFirst()
{
	backward(mstl::numeric_limits<unsigned>::max());
	forward();
	goToCurrentMove();
}


void
Game::goToStart()
{
	backward(mstl::numeric_limits<unsigned>::max());
	goToCurrentMove();
}


void
Game::goToEnd()
{
	if (forward(mstl::numeric_limits<unsigned>::max()))
		goToCurrentMove();
}


void
Game::goTo(mstl::string const& key)
{
	moveTo(key);
	goToCurrentMove();
}


void
Game::goTo(edit::Key const& key)
{
	//M_REQUIRE(isValidKey(key));

	moveTo(key);
	goToCurrentMove();
}


void
Game::goForward(unsigned count)
{
	if (forward(count))
	{
		if (count == 1)
			goToCurrentMove(true);
		else
			goToCurrentMove();
	}
}


void
Game::goBackward(unsigned count)
{
	if (count == 1)
	{
		if (backward(1))
			goToCurrentMove(false);
	}
	else if (backward(count))
	{
		goToCurrentMove();
	}
}


void
Game::goIntoVariation(unsigned variationNumber)
{
	//M_REQUIRE(isBeforeLineEnd());

	forward();
	enterVariation(variationNumber);
	goToCurrentMove();
}


void
Game::goOutOfVariation()
{
	exitVariation();
	goToCurrentMove();
}


void
Game::goIntoNextVariation()
{
	if (m_currentKey.level() == 0 && atLineEnd())
		return;

	if (m_currentKey.level() == 0 && m_currentNode->next()->variationCount() == 0)
	{
		goForward();
	}
	else
	{
		forward();

		MoveNode*	node	= m_currentNode;
		unsigned		n		= 0;

		if (node->variationCount() == 0)
		{
			if (m_currentKey.level() > 0)
				exitVariation();

			node = node->getLineStart();

			if (node->prev() == m_currentNode)
			{
				n = m_currentNode->variationNumber(node) + 1;

				while (n < m_currentNode->variationCount())
					++n;
			}
		}

		if (n < m_currentNode->variationCount())
		{
			enterVariation(n);
			forward();
		}

		goToCurrentMove();
	}
}


void
Game::goIntoPrevVariation()
{
	if (m_currentKey.level() == 0 && m_currentNode->atLineStart())
		return;

	if (m_currentKey.level() == 0)
	{
		goBackward();
	}
	else
	{
		backward();

		MoveNode*	node	= m_currentNode->getLineStart();
		unsigned		n		= 0;

		exitVariation();

		if (node->prev() == m_currentNode)
			n = m_currentNode->variationNumber(node);

		if (n > 0)
		{
			enterVariation(n - 1);
			forward();
		}

		goToCurrentMove();
	}
}


void
Game::goToMainline()
{
	MoveNode* node = m_currentNode;

	exitToMainline();

	if (node != m_currentNode)
		goToCurrentMove();
}


bool
Game::updateLanguageSet()
{
	LanguageSet set;
	m_startNode->collectLanguages(set);

	if (set == m_languageSet)
		return false;

	set.swap(m_languageSet);
	return true;
}


Move
Game::parseMove(mstl::string const& san) const
{
	Move move = m_currentBoard.parseMove(san, move::AllowIllegalMove);

	if (!move)
	{
		Board			board	= m_currentBoard;
		color::ID	side	= board.sideToMove();

		board.tryCastleShort(side);
		board.tryCastleLong(side);

		move = board.parseMove(san, move::AllowIllegalMove);
	}

	return move;
}


void
Game::replaceNode(MoveNode* newNode, Command command)
{
	//M_ASSERT(newNode);

	bool truncate = m_currentNode->next()->isEmptyLine();

	if (truncate)
		insertUndo(Truncate_Variation, command);
	else
		insertUndo(Replace_Node, command, m_currentNode->removeNext());

	m_currentNode->setNext(newNode);

	unsigned flags = UpdatePgn | UpdateIllegalMoves | UpdateBoard;

	if (isMainline())
		flags |= UpdateOpening;

	updateSubscriber(flags);
}


void
Game::insertVariation(MoveNode* variation, unsigned number)
{
	//M_ASSERT(variation);
	//M_ASSERT(!atLineStart());
//	//M_ASSERT(isBeforeLineEnd());

	mstl::auto_ptr<MoveNode> node(variation);

	insertUndo(Remove_Variation, RemoveVariation, number);

	node->fold(false);
	m_currentNode->addVariation(node.release());
	m_currentNode->swapVariations(number, m_currentNode->variationCount() - 1);

	unsigned flags = UpdatePgn | UpdateIllegalMoves | UpdateLanguageSet | UpdateBoard;

	if (isMainline())
		flags |= UpdateOpening;

	updateSubscriber(flags);
}


bool
Game::exchangeMove(Move move, Force flag)
{
	//M_REQUIRE(isBeforeLineEnd());
	//M_REQUIRE(currentBoard().checkMove(move));

	if (move == m_currentNode->next()->move())
		return true;

	Board board(m_currentBoard);

	mstl::auto_ptr<MoveNode> node(m_currentNode->next()->clone());
	node->setMove(m_currentBoard, move);

	if (!checkConsistency(node.get(), board, flag))
		return false;

	replaceNode(node.release(), ExchangeMove);

	return true;
}


bool
Game::exchangeMove(mstl::string const& san, Force flag)
{
	return exchangeMove(parseMove(san), flag);
}


void
Game::addMove(Move const& move)
{
	//M_REQUIRE(atLineEnd());
	//M_REQUIRE(m_currentBoard.checkMove(move));

	insertUndo(Truncate_Variation, AddMove);

	MoveNode* node = new MoveNode(m_currentBoard, move);
	node->setNext(m_currentNode->removeNext());
	m_currentNode->setNext(node);

	unsigned flags = UpdatePgn | UpdateBoard | UpdateIllegalMoves;

	if (isMainline())
		flags |= UpdateOpening;

	updateSubscriber(flags);
}


void
Game::addMove(mstl::string const& san)
{
	addMove(parseMove(san));
}


void
Game::addMoves(MoveNodeP node)
{
	//M_REQUIRE(atLineEnd());
	//M_REQUIRE(isValidVariation(node.get()));
	//M_REQUIRE(node->atLineStart());
	//M_REQUIRE(node->getLineEnd());

	if (node->countHalfMoves() == 0)
		return;

	insertUndo(Truncate_Variation, AddMoves);
	m_currentNode->setNext(node->removeNext());

	unsigned flags = UpdatePgn | UpdateBoard | UpdateIllegalMoves;

	if (isMainline())
		flags |= UpdateOpening;

	updateSubscriber(flags);
}


bool
Game::isValidKey(edit::Key const& key) const
{
	return key.findPosition(m_startNode, m_startBoard.plyNumber()) != 0;
}


bool
Game::isValidVariation(MoveNode const* node) const
{
	//M_REQUIRE(node);
	//M_REQUIRE(node->atLineStart());

	Board board(m_currentBoard);

	for (node = node->next(); node->isBeforeLineEnd(); node = node->next())
	{
		if (!board.isValidMove(node->move()))
			return false;

		board.doMove(node->move());
	}

	return true;
}


unsigned
Game::addVariation(MoveNodeP node)
{
	//M_REQUIRE(node);
	//M_REQUIRE(isBeforeLineEnd());
	//M_REQUIRE(node->atLineStart());
	//M_REQUIRE(node->getLineEnd());
	//M_REQUIRE(node->countHalfMoves() > 0);
	//M_REQUIRE(isValidVariation(node.get()));

	forward();
	insertUndo(Remove_Variation, AddVariation, m_currentNode->variationCount());
	backward();

	Board board(m_currentBoard);

	for (MoveNode* n = node->next(); n->isBeforeLineEnd(); n = n->next())
	{
		board.prepareUndo(n->move());
		board.prepareForPrint(n->move());
		board.doMove(n->move());
	}

	MoveNode* varNode = node.release();
	varNode->setFolded(false);
	m_currentNode->next()->addVariation(varNode);
	updateSubscriber(UpdatePgn | UpdateBoard);

	return m_currentNode->next()->variationCount() - 1;
}


unsigned
Game::addVariation(Move const& move)
{
	//M_REQUIRE(isBeforeLineEnd());
	//M_REQUIRE(currentBoard().checkMove(move));

	MoveNode* node = new MoveNode(m_currentBoard, move);
	node->setNext(new MoveNode);
	MoveNodeP var(new MoveNode(node));

	return addVariation(var);
}


unsigned
Game::addVariation(mstl::string const& san)
{
	return addVariation(parseMove(san));
}


void
Game::newMainline(MoveNode* node)
{
	//M_REQUIRE(isBeforeLineEnd());

	edit::Key currentKey = m_currentKey;

	forward();
	insertUndo(Remove_Mainline, NewMainline);
	node->setFolded(false);
	m_currentNode->addVariation(node);
	unsigned varNo = m_currentNode->variationCount() - 1;
	promoteVariation(varNo, varNo, false);
	moveTo(currentKey);
	updateSubscriber(UpdateAll);
}


void
Game::newMainline(Move const& move)
{
	//M_REQUIRE(isBeforeLineEnd());
	//M_REQUIRE(currentBoard().checkMove(move));

	MoveNode* node = new MoveNode(m_currentBoard, move);
	node->setNext(new MoveNode);
	newMainline(new MoveNode(node));
}


void
Game::newMainline(mstl::string const& san)
{
	newMainline(parseMove(san));
}


void
Game::removeMainline()
{
	//M_ASSERT(!atLineStart());
	//M_ASSERT(m_currentNode->variationCount());

	unsigned varNo = m_currentNode->variationCount() - 1;
	promoteVariation(varNo, varNo, false);
	MoveNode* node = m_currentNode->removeVariation(varNo);

	backward();
	insertUndo(New_Mainline, NewMainline, node, varNo);
	forward();

	updateSubscriber(UpdateAll);
}


bool
Game::checkConsistency(MoveNode* node, Board& board, Force flag)
{
	//M_ASSERT(node);

	while (1)
	{
		if (node->atLineEnd())
			return true;

		//M_ASSERT(node->move());

		node->move().setLegalMove(board.isValidMove(node->move(), move::DontAllowIllegalMove));

		if (!board.isValidMove(node->move(), node->constraint()))
		{
			if (flag == OnlyIfRemainsConsistent)
				return false;

			bool truncate = true;

			if (truncate)
			{
				MoveNode* last = node->getLineEnd();

				node = node->prev();
				node->deleteNext();
				node->setNext(new MoveNode);
				node->next()->setComment(last->comment(move::Post), move::Post);

				return true;
			}
		}

		for (unsigned i = 0; i < node->variationCount(); ++i)
		{
			//M_ASSERT(node->variation(i)->next());

			if (node->variation(i)->isEmptyLine())
			{
				if (i-- > 0)
					node->swapVariations(i, node->variationCount() - 1);
				node->deleteVariation(node->variationCount() - 1);
			}
			else
			{
				Board b(board);

				if (!checkConsistency(node->variation(i)->next(), b, flag))
					return false;
			}
		}

		board.doMove(node->move());
		node = node->next();
	}

	return true; // shut up the compiler
}


void
Game::moveVariation(unsigned from, unsigned to, Command command)
{
	//M_REQUIRE(from < variationCount());
	//M_REQUIRE(to < variationCount());

	if (from == to)
		return;

	insertUndo(Swap_Variations, command, to, from);

	if (from < to)
	{
		for (unsigned i = from; i < to; ++i)
			m_currentNode->swapVariations(i, i + 1);
	}
	else
	{
		for (unsigned i = from; i > to; --i)
			m_currentNode->swapVariations(i, i - 1);
	}

	updateSubscriber(UpdatePgn | UpdateBoard);
}


void
Game::firstVariation(unsigned variationNumber)
{
	//M_REQUIRE(variationNumber < variationCount());

	if (variationNumber > 0)
		moveVariation(variationNumber, 0, FirstVariation);
}


void
Game::promoteVariation(unsigned oldVariationNumber, unsigned newVariationNumber, bool update)
{
	//M_ASSERT(oldVariationNumber < variationCount());
	//M_ASSERT(newVariationNumber < variationCount());
	//M_ASSERT(!m_currentNode->variation(oldVariationNumber)->isOneBeforeLineEnd());

	MoveNode* variation	= m_currentNode->removeVariation(oldVariationNumber);
	MoveNode* parent		= m_currentNode->prev();
	MoveNode* next			= variation->removeNext();

	//M_ASSERT(parent);
	//M_ASSERT(next);

	variation->setFolded(false);
	parent->removeNext();	// = m_currentNode
	parent->setNext(next);
	variation->setNext(m_currentNode);
	next->addVariation(variation);

	Comment comment;
	variation->swapComment(comment, move::Post);
	parent->getLineStart()->swapComment(comment, move::Post);
	variation->swapComment(comment, move::Post);

	while (m_currentNode->hasVariation())
		next->addVariation(m_currentNode->removeVariation(0));

	for (unsigned i = 1; i <= newVariationNumber; ++i)
		next->swapVariations(i - 1, i);

	moveTo(m_currentKey);

	if (update)
	{
		insertUndo(Promote_Variation, PromoteVariation, newVariationNumber, oldVariationNumber);

		unsigned flags = UpdatePgn | UpdateBoard;

		if (isMainline())
			flags |= UpdateOpening;

		updateSubscriber(flags);
	}
}


void
Game::promoteVariation(unsigned oldVariationNumber, unsigned newVariationNumber)
{
	//M_REQUIRE(oldVariationNumber < variationCount());
	//M_REQUIRE(newVariationNumber < variationCount());

	promoteVariation(oldVariationNumber, newVariationNumber, true);
}


void
Game::removeVariation(unsigned variationNumber)
{
	//M_REQUIRE(variationNumber < variationCount());

	MoveNode* node = m_currentNode->removeVariation(variationNumber);
	node->setFolded(false);
	insertUndo(Insert_Variation, RemoveVariation, node, variationNumber);
	updateSubscriber(UpdatePgn | UpdateBoard | UpdateLanguageSet | UpdateIllegalMoves);
}


bool
Game::insertMoves(unsigned variationNumber, Force flag)
{
	//M_REQUIRE(variationNumber < variationCount());
	//M_ASSERT(!atLineStart());

	if (m_currentNode->variation(variationNumber)->countHalfMoves() == 0)
		return true;

	Board board(m_currentBoard);
	board.undoMove(m_currentNode->move());

	m_currentNode->variation(variationNumber)->setFolded(false);

	MoveNode* curr = m_currentNode->clone();
	MoveNode* root = curr->removeVariation(variationNumber);
	MoveNode* node = root->removeNext();

	delete root;
	node->getOneBeforeLineEnd()->setNext(curr);

	board = m_currentBoard;
	board.undoMove(m_currentNode->move());

	if (!checkConsistency(node, board, flag))
		return false;

	backward();
	insertUndo(Replace_Node, InsertMoves, m_currentNode->removeNext());
	m_currentNode->setNext(node);
	moveTo(m_currentKey);

	unsigned flags = UpdatePgn | UpdateBoard | UpdateIllegalMoves;

	if (isMainline())
		flags |= UpdateOpening;

	updateSubscriber(flags);
	return true;
}


bool
Game::exchangeMoves(unsigned variationNumber, unsigned movesToExchange, Force flag)
{
	//M_REQUIRE(variationNumber < variationCount());
	//M_ASSERT(!atLineStart());

	if (m_currentNode->variation(variationNumber)->countHalfMoves() == 0)
		return true;

	if (movesToExchange == 0)
		return insertMoves(variationNumber);

	Board board(m_currentBoard);
	board.undoMove(m_currentNode->move());

	MoveNode* curr = m_currentNode->clone();
	MoveNode* node = curr->removeVariation(variationNumber);
	MoveNode* last = node->getOneBeforeLineEnd();

	node->setFolded(false);

	MoveNode* tail = curr;

	for (unsigned i = 0, n = mstl::min(movesToExchange - 1, tail->countHalfMoves()); i < n; ++i)
		tail = tail->next();

	MoveNode* line = node->removeNext();

	last->setNext(tail->removeNext());
	tail->setNext(new MoveNode);
	node->setNext(curr);
	line->addVariation(node);

	board = m_currentBoard;
	board.undoMove(node->next()->move());

	if (!checkConsistency(line, board, flag))
		return false;

	backward();
	insertUndo(Replace_Node, ExchangeMoves, m_currentNode->removeNext());
	m_currentNode->setNext(line);
	moveTo(m_currentKey);

	unsigned flags = UpdatePgn | UpdateBoard | UpdateIllegalMoves;

	if (isMainline())
		flags |= UpdateOpening;

	updateSubscriber(flags);
	return true;
}


void
Game::truncateVariation(move::Position position)
{
	if (position == move::Ante)
		backward();

	if (atLineEnd())
		return;

	MoveNode* node = m_currentNode->removeNext();
	insertUndo(Replace_Node, TruncateVariation, node);
	m_currentNode->setNext(node->getLineEnd()->clone());

	unsigned flags = UpdatePgn | UpdateIllegalMoves | UpdateLanguageSet;

	if (isMainline())
		flags |= UpdateOpening;
	if (position == move::Ante)
		flags |= UpdateBoard;

	updateSubscriber(flags);
}


void
Game::changeVariation(MoveNodeP node, unsigned variationNumber)
{
	//M_REQUIRE(node);
	//M_REQUIRE(isBeforeLineEnd());
	//M_REQUIRE(isValidVariation(node.get()));
	//M_REQUIRE(variationNumber < subVariationCount());

	Board board(m_currentBoard);

	for (MoveNode* n = node->next(); n->isBeforeLineEnd(); n = n->next())
	{
		board.prepareUndo(n->move());
		board.prepareForPrint(n->move());
		board.doMove(n->move());
	}

	MoveNode* varNode = node.release();
	varNode->setFolded(false);
	delete m_currentNode->next()->replaceVariation(variationNumber, varNode);

	updateSubscriber(UpdatePgn | UpdateBoard | UpdateIllegalMoves | UpdateLanguageSet);
}


void
Game::replaceVariation(Move const& move)
{
	//M_REQUIRE(currentBoard().checkMove(move));

	if (atLineEnd())
	{
		addMove(move);
	}
	else
	{
		MoveNode* node = new MoveNode(m_currentBoard, move);
		node->setNext(new MoveNode);
		replaceNode(node, ReplaceVariation);
	}
}


void
Game::replaceVariation(mstl::string const& san)
{
	replaceVariation(parseMove(san));
}


bool
Game::stripMoves(move::Position position)
{
	//M_REQUIRE(isMainline());

	if (position == move::Ante)
		backward();

	if (atLineStart())
		return false;

	insertUndo(Strip_Moves, StripMoves, m_startNode->removeNext(), m_startBoard);
	m_startNode->setNext(m_currentNode->removeNext());
	m_startBoard = m_currentBoard;
	moveTo(m_currentKey);

	unsigned flags = UpdatePgn | UpdateBoard | UpdateLanguageSet | UpdateIllegalMoves;

	if (isMainline())
		flags |= UpdateOpening;

	updateSubscriber(flags);
	return true;
}


void
Game::unstripMoves(MoveNode* startNode, Board const& startBoard, edit::Key const& key)
{
	//M_ASSERT(startNode->next());

	MoveNode* last = startNode->getLineEnd();

	startNode->setFolded(false);
	last->setNext(m_startNode->removeNext());
	m_startNode->setNext(startNode);
	m_startBoard = startBoard;
	moveTo(key);
	insertUndo(Unstrip_Moves, StripMoves);

	unsigned flags = UpdatePgn | UpdateBoard | UpdateLanguageSet | UpdateIllegalMoves;

	if (key.isMainlineId())
		flags |= UpdateOpening;

	updateSubscriber(flags);
}


bool
Game::stripAnnotations()
{
	if (m_startNode->countAnnotations() == 0)
		return false;

	insertUndo(Revert_Game, StripAnnotations, m_startNode);
	m_startNode = m_startNode->clone();
	m_startNode->stripAnnotations();
	moveTo(m_currentKey);
	updateSubscriber(UpdatePgn | UpdateBoard);

	return true;
}


bool
Game::stripComments()
{
	if (m_startNode->countComments() == 0)
		return false;

	insertUndo(Revert_Game, StripComments, m_startNode);
	m_startNode = m_startNode->clone();
	m_startNode->stripComments();
	moveTo(m_currentKey);

	unsigned flags = UpdatePgn | UpdateBoard;

	if (updateLanguageSet())
		flags |= UpdateLanguageSet;

	updateSubscriber(flags);

	return true;
}


bool
Game::stripComments(mstl::string const& lang)
{
	if (m_startNode->countComments(lang) == 0)
		return false;

	insertUndo(Revert_Game, StripComments, m_startNode);
	m_startNode = m_startNode->clone();
	m_startNode->stripComments(lang);
	moveTo(m_currentKey);

	unsigned flags = UpdatePgn | UpdateBoard;

	if (updateLanguageSet())
		flags |= UpdateLanguageSet;

	updateSubscriber(flags);

	return true;
}


bool
Game::copyComments(mstl::string const& fromLang, mstl::string const& toLang, bool stripOriginal)
{
	if (m_startNode->countComments(fromLang) == 0)
		return false;

	insertUndo(Revert_Game, stripOriginal ? MoveComments : CopyComments, m_startNode);
	m_startNode = m_startNode->clone();
	m_startNode->copyComments(fromLang, toLang, stripOriginal);
	moveTo(m_currentKey);

	unsigned flags = UpdatePgn | UpdateBoard;

	if (updateLanguageSet())
		flags |= UpdateLanguageSet;

	updateSubscriber(flags);

	return true;
}


bool
Game::stripMoveInfo()
{
	if (m_startNode->countMoveInfo() == 0)
		return false;

	insertUndo(Revert_Game, StripMoveInfo, m_startNode);
	m_startNode = m_startNode->clone();
	m_startNode->stripMoveInfo();
	moveTo(m_currentKey);
	updateSubscriber(UpdatePgn);

	return true;
}


bool
Game::stripMarks()
{
	if (m_startNode->countMarks() == 0)
		return false;

	insertUndo(Revert_Game, StripMarks, m_startNode);
	m_startNode = m_startNode->clone();
	m_startNode->stripMarks();
	moveTo(m_currentKey);

	if (m_subscriber)
		m_subscriber->updateMarks(mstl::string::empty_string);

	updateSubscriber(UpdatePgn);
	return true;
}


bool
Game::stripVariations()
{
	//M_REQUIRE(isMainline());

	if (m_startNode->countVariations() == 0)
		return false;

	insertUndo(Revert_Game, StripVariations, m_startNode);
	m_startNode = m_startNode->clone();
	m_startNode->stripVariations();
	moveTo(m_currentKey);
	updateSubscriber(UpdatePgn | UpdateBoard | UpdateLanguageSet | UpdateIllegalMoves);

	return true;
}


void
Game::revertGame(MoveNode* startNode, Command command)
{
	insertUndo(Revert_Game, command, m_startNode);
	m_startNode = startNode;
	startNode->setFolded(false);

	if (command == Transpose)
	{
		m_idn = chess960::twin(m_idn);
		m_startBoard.transpose();
	}

	moveTo(m_currentKey);
	updateLanguageSet();
	updateSubscriber(UpdateAll);
}


void
Game::resetGame(MoveNode* startNode, Board const& startBoard, edit::Key const&)
{
	Board board(m_startBoard);
	MoveNode* node(m_startNode);

	moveToMainlineStart();
	startNode->setFolded(false);
	m_startNode = startNode;
	m_startBoard = startBoard;
	if (!node->isEmptyLine() && !board.isEqualPosition(m_startBoard))
		insertUndo(Set_Start_Position, Clear, node, board);
	moveToMainlineStart();
	updateSubscriber(UpdateAll);
}


void
Game::clear(Board const* startPosition)
{
	moveToMainlineStart();
	if (!m_startNode->isEmptyLine())
		insertUndo(Set_Start_Position, Clear, m_startNode, m_startBoard);
	m_startNode = m_currentNode = new MoveNode();
	m_startNode->setNext(new MoveNode);
	if (startPosition)
		m_startBoard = *startPosition;
	m_currentBoard = m_startBoard;
	updateSubscriber(UpdateAll);
}


void
Game::resetForNextLoad()
{
	delete m_startNode;
	delete m_editNode;

	m_startNode = m_currentNode = new MoveNode;
	m_startNode->setNext(new MoveNode);
	m_editNode = 0;
	m_currentKey.clear();
	m_undoList.clear();
	m_languageSet.clear();
	m_undoIndex = 0;
	m_idn = 0;
	m_isIrreversible = false;
	m_isModified = false;
	m_wasModified = false;
	m_finalBoardIsValid = false;
	m_startBoard = Board::standardBoard();
	m_currentBoard = m_startBoard;
	m_finalBoard.clear();
	m_tags.clear();
	m_flags = 0;
	m_eco = Eco();
	m_undoCommand = None;
	m_redoCommand = None;
	m_line.length = 0;
}


util::crc::checksum_t
Game::computeChecksum(util::crc::checksum_t crc) const
{
	crc = util::crc::compute(
				crc,
				reinterpret_cast<unsigned char const*>(&m_startBoard.uniquePosition()),
				sizeof(board::UniquePosition));
	crc = m_startNode->computeChecksum(m_engines, crc);

	return crc;
}


void
Game::setStartPosition(mstl::string const& fen)
{
	//M_REQUIRE(isEmpty());

	if (!m_startBoard.setup(fen)) {}
		//DB_RAISE("invalid FEN");

	m_currentBoard = m_startBoard;

	//M_DEBUG(m_startBoard.validate(variant::Unknown, castling::AllowHandicap) == Board::Valid);
}


void
Game::setStartPosition(unsigned idn)
{
	//M_REQUIRE(isEmpty());
	//M_REQUIRE(idn <= 4*960);

	m_startBoard.setup(idn);
	m_currentBoard = m_startBoard;

	//M_DEBUG(m_startBoard.validate(variant::fromIdn(idn)) == Board::Valid);
}


unsigned
Game::dumpMoves(mstl::string& result, unsigned length, unsigned flags)
{
	if (atLineEnd())
		return 0;

	unsigned n = 0;

	result.clear();

	while (length--)
	{
		if (m_currentNode->next()->atLineEnd())
			return n;

		m_currentNode = m_currentNode->next();
		printMove(result, flags | (result.empty() ? BlackNumbers : 0));
		m_currentNode = m_currentNode->prev();
		forward();
		result += ' ';
		++n;
	}

	result.rtrim();
	return n;
}


unsigned
Game::dumpMoves(mstl::string& result, unsigned flags)
{
	return dumpMoves(result, mstl::numeric_limits<unsigned>::max(), flags);
}


void
Game::getHistory(History& result) const
{
	result.clear();
	result.reserve(100);

	for (MoveNode* node = m_currentNode; node; node = node->prev())
	{
		if (!node->atLineStart() && !node->atLineEnd())
			result.push_back(node->move());
	}
}


unsigned
Game::dumpHistory(mstl::string& result) const
{
	History hist;
	getHistory(hist);

	result.reserve(result.size() + hist.size()*6);

	for (int i = hist.size() - 1; i >= 0; i--)
	{
		if (!result.empty())
			result.append(' ');
		hist[i].printAlgebraic(result);
	}

	return hist.size();
}


bool
Game::finishLoad()
{
	//M_REQUIRE(atMainlineStart());

	Board board(m_startBoard);

	bool ok = checkConsistency(m_startNode->next(), board, OnlyIfRemainsConsistent);

	if (!ok)
		checkConsistency(m_startNode->next(), board, TruncateIfNeccessary);

	m_startNode->finish(m_startBoard);
	m_currentBoard = m_startBoard;

	updateLine();
	updateLanguageSet();
	m_wantedLanguages = m_languageSet;

	return ok;
}


uint16_t
Game::currentLine(Line& result)
{
	// require: #result.moves >= opening::Max_Line_Length

	typedef mstl::vector<MoveNode const*> Path;

	Path path;
	path.reserve(100);

	if (!m_currentNode->atLineStart())
		path.push_back(m_currentNode);

	MoveNode const* prev = m_currentNode->prev();

	for (MoveNode const* next = m_currentNode; prev; next = prev, prev = prev->prev())
	{
		if (prev->next() == next && !prev->atLineStart())
			path.push_back(prev);
	}

	mstl::reverse(path.begin(), path.end());

	HomePawns	hp;
	unsigned		length = 0;

	for (Path::const_iterator i = path.begin(); i != path.end(); ++i)
	{
		MoveNode const* node = *i;

		if (length < opening::Max_Line_Length)
			const_cast<uint16_t*>(result.moves)[length++] = node->move().index();

		hp.move(node->move());
	}

	result.length = length;
	m_currentBoard.signature().setHomePawns(hp.used(), hp.data());

	return hp.signature();
}


bool
Game::updateLine()
{
	if (!isMainline())
		return false;

	m_finalBoardIsValid = false;

	unsigned idn = m_startBoard.computeIdn();

	bool update = m_idn != idn;

	if (idn)
	{
		uint16_t* lineBuf = m_line.moves == m_lineBuf[0] ? m_lineBuf[1] : m_lineBuf[0];

		unsigned	i = 0;

		for (	MoveNode const* node = m_startNode->next();
				node->isBeforeLineEnd() && !node->move().isNull() && i < opening::Max_Line_Length;
				node = node->next(), ++i)
		{
			lineBuf[i] = node->move().index();
		}

		Line line(lineBuf, i);

		if (idn == chess960::StandardIdn)
		{
			if (line != m_line || !m_eco)
			{
				Eco eco(m_eco);

				m_line = line;
				m_eco = EcoTable::specimen().getEco(m_line);

				EcoTable::specimen().lookup(m_line);

				if (m_eco != eco)
					update = true;
			}
		}
		else
		{
			m_eco = 0;
			m_line.length = 0;
		}
	}
	else
	{
		m_eco = 0;
		m_line.length = 0;
	}

	m_idn = idn;
	return update;
}


Eco
Game::computeEcoCode() const
{
	if (m_idn != chess960::StandardIdn)
		return Eco();

	uint16_t lineBuf[opening::Max_Line_Length];

	unsigned	i = 0;

	for (	MoveNode const* node = m_startNode->next();
			node->isBeforeLineEnd() && i < opening::Max_Line_Length;
			node = node->next(), ++i)
	{
		lineBuf[i] = node->move().index();
	}

	Line line(lineBuf, i);
	return EcoTable::specimen().getEco(m_line);
}


void
Game::setUndoLevel(unsigned level, unsigned combinePredecessingMoves)
{
	m_maxUndoLevel = level;
	m_combinePredecessingMoves = combinePredecessingMoves;

	if (m_undoList.size() > m_maxUndoLevel)
	{
		size_t n = m_undoList.size() - m_maxUndoLevel;
		size_t r = mstl::min(size_t(m_undoList.size() - m_undoIndex), n);
		size_t k = m_undoList.size() - r;

		// firstly erase redo's
		for (size_t i = k; i < m_undoList.size(); ++i)
			delete m_undoList[i];

		m_undoList.resize(k);
		n -= r;

		// secondly erase undo's
		for (size_t i = 0; i < n; ++i)
			delete m_undoList[i];

		m_undoList.erase(m_undoList.begin(), m_undoList.begin() + n);
		m_undoIndex = mstl::min(m_undoIndex, m_maxUndoLevel);
	}
}


bool
Game::transpose(Force flag)
{
	if (m_idn)
	{
		mstl::auto_ptr<MoveNode> root(m_startNode->clone());
		Board board(m_startBoard);

		root->transpose();
		board.transpose();

		if (!checkConsistency(root.get()->next(), board, flag))
			return false;

		insertUndo(Revert_Game, Transpose, m_startNode);

		m_startNode = root.release();
		m_idn = chess960::twin(m_idn);
		m_startBoard.transpose();
		moveToMainlineStart();
		updateSubscriber(UpdateBoard | UpdatePgn | UpdateOpening | UpdateIllegalMoves);
	}

	return true;
}


void
Game::unfold()
{
	MoveNode*	node		= m_currentNode;
	unsigned		level		= 0;
	bool			unfolded	= false;

	do
	{
		if (!node->atLineStart())
			node = node->prev();

		if (level == 0 && node->atLineStart())
		{
			node = node->prev();
			++level;
		}
		else
		{
			while (!node->atLineStart())
				node = node->prev();

			if (node->isFolded())
			{
				node->setFolded(false);
				unfolded = true;
			}

			node = node->prev();
			++level;
		}
	}
	while (node);

	if (unfolded)
		updateSubscriber(UpdatePgn);
}


bool
Game::isFolded(edit::Key const& key) const
{
	//M_REQUIRE(isValidKey(key));

	return key.findPosition(m_startNode, m_startBoard.plyNumber())->getLineStart()->isFolded();
}


void
Game::setFolded(edit::Key const& key, bool flag)
{
	//M_REQUIRE(isValidKey(key));

	if (key.level() == 0)
		return;

	MoveNode*	node		= key.findPosition(m_startNode, m_startBoard.plyNumber())->getLineStart();
	unsigned		update	= UpdatePgn;

	if (flag && node->contains(m_currentNode))
	{
		goToFirst();
		update |= UpdateBoard;
	}

	node->setFolded(flag);
	updateSubscriber(update);
}


void
Game::toggleFolded(edit::Key const& key)
{
	//M_REQUIRE(isValidKey(key));
	//M_REQUIRE(key.level() > 0);

	MoveNode*	node		= key.findPosition(m_startNode, m_startBoard.plyNumber())->getLineStart();
	bool			flag		= !node->isFolded();
	unsigned		update	= UpdatePgn;

	if (flag && node->contains(m_currentNode))
	{
		goToFirst();
		update |= UpdateBoard;
	}

	node->setFolded(flag);
	updateSubscriber(update);
}


void
Game::setFolded(bool flag)
{
	if (flag && isVariation())
		goToFirst();

	m_startNode->fold(flag);
	updateSubscriber(UpdatePgn | UpdateBoard);
}


void
Game::setSubscriber(SubscriberP subscriber)
{
	if ((m_subscriber = subscriber))
		moveToMainlineStart();
}


void
Game::setLanguages(LanguageSet const& set)
{
	if (m_wantedLanguages != set)
	{
		m_wantedLanguages = set;
		updateSubscriber(UpdatePgn | UpdateBoard | UpdateLanguageSet);
	}
}


bool
Game::containsLanguage(edit::Key const& key, move::Position position, mstl::string const& lang) const
{
	//M_REQUIRE(isValidKey(key));

	MoveNode* node = key.findPosition(m_startNode, m_startBoard.plyNumber());
	return node->comment(position).containsLanguage(lang);
}


bool
Game::commentEngFlag() const
{
	return m_startNode->containsEnglishLang();
}


bool
Game::commentOthFlag() const
{
	return m_startNode->containsOtherLang();
}


void
Game::refreshSubscriber(unsigned actions)
{
	delete m_editNode;
	m_editNode = 0;
	updateSubscriber(actions);
}


void
Game::updateSubscriber(unsigned action)
{
	if (!m_subscriber)
		return;

	if (action & UpdateIllegalMoves)
	{
		if (m_startNode->containsIllegalMoves())
			m_flags |= GameInfo::Flag_Illegal_Move;
		else
			m_flags &= ~GameInfo::Flag_Illegal_Move;

		if (m_startNode->containsIllegalCastlings())
			m_flags |= GameInfo::Flag_Illegal_Castling;
		else
			m_flags &= ~GameInfo::Flag_Illegal_Castling;
	}

	updateLine();

	if (action & (UpdatePgn | UpdateOpening | UpdateLanguageSet))
	{
		typedef mstl::auto_ptr<edit::Root> Root;

		if (m_subscriber->mainlineOnly())
		{
			Root editNode(edit::Root::makeList(	m_tags,
															m_idn,
															m_eco,
															m_startBoard,
															m_startNode,
															m_linebreakThreshold,
															m_linebreakMaxLineLengthMain,
															m_displayStyle));
			m_editNode = editNode.release();
			m_subscriber->updateEditor(m_editNode, m_moveStyle);
		}
		else
		{
			Root editNode(edit::Root::makeList(	m_tags,
															m_idn,
															m_eco,
															m_startBoard,
															m_languageSet,
															m_wantedLanguages,
															m_engines,
															m_startNode,
															m_linebreakThreshold,
															m_linebreakMaxLineLengthMain,
															m_linebreakMaxLineLengthVar,
															m_linebreakMinCommentLength,
															m_displayStyle));

			edit::Node::List diff;
			editNode->difference(m_editNode, diff);
			m_subscriber->updateEditor(diff, m_tags, m_moveStyle);
			delete m_editNode;
			m_editNode = editNode.release();
		}
	}

	if (m_isModified != m_wasModified)
		m_subscriber->stateChanged(m_wasModified = m_isModified);

	if (action & UpdateBoard)
		goToCurrentMove();
}


Board
Game::board(edit::Key const& key) const
{
	//M_REQUIRE(isValidKey(key));

	Board board = m_startBoard;
	key.setBoard(m_startNode, board);
	return board;
}


mstl::string&
Game::printFen(mstl::string& result) const
{
	return m_currentBoard.toFen(result);
}


mstl::string&
Game::printFen(mstl::string const& key, mstl::string& result) const
{
	return board(key).toFen(result);
}


void
Game::setup(unsigned linebreakThreshold,
				unsigned linebreakMaxLineLengthMain,
				unsigned linebreakMaxLineLengthVar,
				unsigned linebreakMinCommentLength,
				unsigned displayStyle,
				move::Notation moveStyle)
{
	//M_REQUIRE(displayStyle & (display::CompactStyle | display::ColumnStyle));
	//M_REQUIRE((displayStyle & (display::CompactStyle | display::ColumnStyle))
	//				!= (display::CompactStyle | display::ColumnStyle));

	m_linebreakThreshold				= linebreakThreshold;
	m_linebreakMaxLineLengthMain	= linebreakMaxLineLengthMain;
	m_linebreakMaxLineLengthVar	= linebreakMaxLineLengthVar;
	m_linebreakMinCommentLength	= linebreakMinCommentLength;
	m_displayStyle						= displayStyle;
	m_moveStyle							= moveStyle;
}


void
Game::setIsModified(bool flag)
{
	m_isModified = flag;

	if (flag)
		m_isIrreversible = false;
	else
		m_undoIndex = 0;

	if (m_subscriber && m_isModified != m_wasModified)
		m_subscriber->stateChanged(m_wasModified = m_isModified);
}

// vi:set ts=3 sw=3:
