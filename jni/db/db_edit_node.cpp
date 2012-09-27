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

#include "db_edit_node.h"
#include "db_move_node.h"
#include "db_board.h"
#include "db_annotation.h"
#include "db_mark_set.h"
#include "db_move_info_set.h"
#include "db_tag_set.h"
#include "db_engine_list.h"

#include "m_stack.h"
#include "m_limits.h"
#include "m_utility.h"
#include "m_assert.h"

using namespace db::edit;


static unsigned const DontSetBreaks = mstl::numeric_limits<unsigned>::max();


template <class List>
static void
deleteList(List& list)
{
	for (unsigned i = 0; i < list.size(); ++i)
		delete list[i];
}


struct Node::Spacing
{
	enum Type		{ Zero, Space, Open, Close, Break, Para };
	enum Context	{ None, Comment, PreComment, Diagram, StartVariation, EndVariation };

	struct Token
	{
		Token(unsigned lvl, Type t)
			:level(mstl::min(lvl, 255u))
			,number(0)
			,space(t)
			,context(None)
			,isFirstOrLast(0)
		{
		}

		Token(unsigned lvl, Type t, bool flag)
			:level(mstl::min(lvl, 255u))
			,number(0)
			,space(t)
			,context(None)
			,isFirstOrLast(flag)
		{
		}

		Token(unsigned lvl, Type t, Context c)
			:level(mstl::min(lvl, 255u))
			,number(0)
			,space(t)
			,context(c)
			,isFirstOrLast(0)
		{
		}

		Token(unsigned lvl, Type t, unsigned no, bool flag)
			:level(mstl::min(lvl, 255u))
			,number(mstl::min(no, 255u))
			,space(t)
			,context(None)
			,isFirstOrLast(flag)
		{
		}

		bool operator==(Type t) const { return space == t; }
		bool operator!=(Type t) const { return space != t; }

		uint8_t level;
		uint8_t number;
		uint8_t space:4;
		uint8_t context:3;
		uint8_t isFirstOrLast:1;
	};

	typedef mstl::stack<Token> TokenList;

	Spacing();

	void incrPlyCount();

	void pushSpace();
	void pushOpen(bool istFirstVar, unsigned number = 0);
	void pushClose(bool isLastVar);
	void pushBreak();
	void pushBreak(unsigned level);
	void pushParagraph(Context context);
	void pushSpaceOrParagraph(Context context);

	void pop(List& list);

	bool m_isVirgin;

	TokenList	m_tokenList;
	unsigned		m_level;
	unsigned		m_plyCount;
	unsigned		m_commentCount;
	unsigned		m_linebreakMaxLineLength;
	unsigned		m_displayStyle;
};


Node::Spacing::Spacing()
	:m_isVirgin(true)
	,m_tokenList(1, Token(0, Zero))
	,m_level(0)
	,m_plyCount(0)
	,m_linebreakMaxLineLength(::DontSetBreaks)
	,m_displayStyle(display::CompactStyle)
{
}


void
Node::Spacing::incrPlyCount()
{
	if (++m_plyCount == m_linebreakMaxLineLength)
	{
		m_tokenList.push(Token(0, Break));
		m_tokenList.push(Token(0, Break));
		m_plyCount = 0;
	}

	m_isVirgin = false;
}


void
Node::Spacing::pushOpen(bool istFirstVar, unsigned number)
{
	Type type;

	while (m_tokenList.top() == Space || m_tokenList.top() == Break || m_tokenList.top() == Para)
		m_tokenList.pop();

	if (m_level > 1 || m_tokenList.top() == Close || !(m_displayStyle & display::ParagraphSpacing))
		type = Break;
	else
		type = Para;

	m_tokenList.push(Token(m_level, type));
	m_tokenList.push(Token(m_level, Open, number, istFirstVar));
	m_plyCount = 0;
}


void
Node::Spacing::pushClose(bool isLastVar)
{
   m_tokenList.clear();
   m_tokenList.push(Token(0, Zero));
   m_tokenList.push(Token(m_level, Close, isLastVar));
}


void
Node::Spacing::pushSpace()
{
   if (!m_isVirgin && m_tokenList.size() == 1)
   	m_tokenList.push(Token(m_level, Space));
}


void
Node::Spacing::pushBreak(unsigned level)
{
   if (	!m_isVirgin
   	&& m_tokenList.top() != Break
   	&& m_tokenList.top() != Para
   	&& m_tokenList.top() != Open)
   {
   	if (m_tokenList.top() == Space)
   		m_tokenList.pop();

		m_tokenList.push(Token(level, Break));
		m_plyCount = 0;
   }
}


void
Node::Spacing::pushBreak()
{
	pushBreak(m_level);
}


void
Node::Spacing::pushParagraph(Context context)
{
	if (!m_isVirgin)
	{
		if (context != None && m_tokenList.top() == Para && m_tokenList.top().context == context)
		{
			m_tokenList.top() = Token(m_level, Break);
		}
		else if (m_tokenList.top() != Para && m_tokenList.top() != Open)
		{
			if ((m_level == 0 || context == Diagram) && (m_displayStyle & display::ParagraphSpacing))
			{
				while (m_tokenList.top() == Break || m_tokenList.top() == Space)
					m_tokenList.pop();

				if (m_tokenList.top() != Para)
					m_tokenList.push(Token(m_level, Para, context));
				else if (context != None && m_tokenList.top().context == context)
					m_tokenList.top() = Token(m_level, Break);
			}
			else if (context == EndVariation)
			{
				pushBreak(m_level + 1);
			}
			else
			{
				pushBreak();
			}
		}

		m_plyCount = 0;
	}
}


void
Node::Spacing::pushSpaceOrParagraph(Context context)
{
	if (m_level == 0)
		pushParagraph(context);
	else
		pushSpace();
}


void
Node::Spacing::pop(List& list)
{
	for (unsigned i = 1; i < m_tokenList.size(); ++i)
	{
		Token const& token = m_tokenList[i];

		switch (token.space)
		{
			case None:	/* skip */ break;
			case Space:	list.push_back(new edit::Space); break;
			case Close:	/* skip */ break;
			case Para:	list.push_back(new edit::Space(token.level)); // fallthru
			case Break:	list.push_back(new edit::Space(token.level)); break;

			case Open:
				if (token.number)
					list.push_back(new edit::Space(token.level, token.number, token.isFirstOrLast));
				else
					list.push_back(new edit::Space(Node::Open, token.isFirstOrLast));
				break;
		}
	}

	m_tokenList.clear();
	m_tokenList.push(Token(0, Zero));
}


struct Node::Work : public Node::Spacing
{
	Work();

	LanguageSet const* wantedLanguages;

	db::Board			board;
	Languages*			languages;
	EngineList const*	engineList;
	Key					key;
	bool					needMoveNo;
	bool					isFolded;
	bool					isEmpty;
	unsigned				linebreakMaxLineLengthVar;
	unsigned				linebreakMinCommentLength;
};


Node::Work::Work()
	:wantedLanguages(0)
	,languages(0)
	,engineList(0)
	,needMoveNo(true)
	,isFolded(false)
	,isEmpty(true)
	,linebreakMaxLineLengthVar(0)
	,linebreakMinCommentLength(0)
{
}


Node::~Node() throw()				{}
Visitor::~Visitor() throw()		{}
Variation::~Variation() throw()	{ ::deleteList(m_list); }
Move::~Move() throw()				{ ::deleteList(m_list); }


Node::Type Action::type() const		{ return TAction; }
Node::Type Root::type() const			{ return TRoot; }
Node::Type Opening::type() const		{ return TOpening; }
Node::Type Languages::type() const	{ return TLanguages; }
Node::Type Variation::type() const	{ return TVariation; }
Node::Type Move::type() const			{ return TMove; }
Node::Type Diagram::type() const		{ return TDiagram; }
Node::Type Ply::type() const			{ return TPly; }
Node::Type Comment::type() const		{ return TComment; }
Node::Type Annotation::type() const	{ return TAnnotation; }
Node::Type Marks::type() const		{ return TMarks; }
Node::Type Space::type() const		{ return TSpace; }


void Opening::visit(Visitor& visitor) const		{ visitor.opening(m_board, m_idn, m_eco); }
void Languages::visit(Visitor& visitor) const	{ visitor.languages(m_langSet); }
void Ply::visit(Visitor& visitor) const			{ visitor.move(m_moveNo, m_move); }
void Comment::visit(Visitor& visitor) const		{ visitor.comment(m_position, m_varPos, m_comment); }
void Annotation::visit(Visitor& visitor) const	{ visitor.annotation(m_annotation); }
void Marks::visit(Visitor& visitor) const			{ visitor.marks(m_hasMarks); }


inline bool KeyNode::operator<(KeyNode const* node) const { return m_key < node->m_key; }
inline bool KeyNode::operator>(KeyNode const* node) const { return node->m_key < m_key; }

Key const& KeyNode::startKey() const	{ return m_key; }
Key const& KeyNode::endKey() const		{ return m_key; }


bool
KeyNode::operator==(KeyNode const* node) const
{
	return m_key == node->key() && Node::operator==(node);
}


bool
KeyNode::operator!=(KeyNode const* node) const
{
	return m_key != node->key() || Node::operator!=(node);
}


bool
Diagram::operator==(Node const* node) const
{
	//M_ASSERT(node);
	//M_ASSERT(dynamic_cast<Diagram const*>(node));
	//M_ASSERT(m_key == static_cast<Diagram const*>(node)->m_key);

	return	m_fromColor == static_cast<Diagram const*>(node)->m_fromColor
			&& m_board.isEqualPosition(static_cast<Diagram const*>(node)->m_board);
}


inline
Diagram::Diagram(Work& work, color::ID fromColor)
	:KeyNode(Key(work.key, PrefixDiagram))
	,m_board(work.board)
	,m_fromColor(fromColor)
{
}


void
Diagram::visit(Visitor& visitor) const
{
	visitor.startDiagram(m_key);
	visitor.position(m_board, m_fromColor);
	visitor.endDiagram(m_key);
}


Ply::Ply(MoveNode const* move, unsigned moveno)
	:m_moveNo(moveno)
	,m_move(move->move())
{
}


bool
Ply::operator==(Node const* node) const
{
	//M_ASSERT(node);
	//M_ASSERT(dynamic_cast<Ply const*>(node));

	return	m_moveNo == static_cast<Ply const*>(node)->m_moveNo
			&& m_move == static_cast<Ply const*>(node)->m_move;
}


bool
Comment::operator==(Node const* node) const
{
	//M_ASSERT(node);
	//M_ASSERT(dynamic_cast<Comment const*>(node));

	return	m_position == static_cast<Comment const*>(node)->m_position
			&& m_comment == static_cast<Comment const*>(node)->m_comment;
}


Annotation::Annotation(db::Annotation const& annotation, bool deleteDiagram)
	:m_annotation(annotation)
{
	if (deleteDiagram)
	{
		m_annotation.remove(nag::Diagram);
		m_annotation.remove(nag::DiagramFromBlack);
	}
}


bool
Annotation::operator==(Node const* node) const
{
	//M_ASSERT(node);
	//M_ASSERT(dynamic_cast<Annotation const*>(node));

	return m_annotation == static_cast<Annotation const*>(node)->m_annotation;
}


Marks::Marks(MarkSet const& marks)
	:m_hasMarks(!marks.isEmpty())
{
}


bool
Marks::operator==(Node const* node) const
{
	//M_ASSERT(node);
	//M_ASSERT(dynamic_cast<Marks const*>(node));

	return m_hasMarks == static_cast<Marks const*>(node)->m_hasMarks;
}


void
Space::visit(Visitor& visitor) const
{
	if (m_level >= 0 && m_number == 0)
	{
		visitor.linebreak(m_level);
	}
	else if (m_number == 0 || m_level > 3)
	{
		visitor.space(m_bracket, m_isFirstOrLast);
	}
	else
	{
		mstl::string s;

		switch (m_level)
		{
			case 1: s.format("%u", m_number); break;
			case 2: s.append(char(((m_number - 1) % 26) + 'a')); break;
			case 3: s.appendSmallRomanNumber(m_number); break;
		}

		visitor.number(s, m_isFirstOrLast);
	}
}


bool
Space::operator==(Node const* node) const
{
	//M_ASSERT(node);
	//M_ASSERT(dynamic_cast<Space const*>(node));

	return	m_level == static_cast<Space const*>(node)->m_level
			&& m_bracket == static_cast<Space const*>(node)->m_bracket
			&& m_number == static_cast<Space const*>(node)->m_number;
}


bool
Opening::operator==(Node const* node) const
{
	//M_ASSERT(node);
	//M_ASSERT(dynamic_cast<Opening const*>(node));

	return	m_idn == static_cast<Opening const*>(node)->m_idn
			&& m_eco == static_cast<Opening const*>(node)->m_eco
			&& m_board.isEqualPosition(static_cast<Opening const*>(node)->m_board);
}


bool
Node::operator==(Node const* node) const
{
	//M_ASSERT(!"unexpected call");
	return false;
}


void
Node::visit(Visitor& visitor, List const& nodes, TagSet const& tags)
{
	result::ID result = result::fromString(tags.value(tag::Result));

	visitor.start(result);

	for (unsigned i = 0; i < nodes.size(); ++i)
		nodes[i]->visit(visitor);

	visitor.finish(result);
}


Action::Action(Command command, unsigned level)
	:m_command(command)
	,m_level(level)
{
	//M_ASSERT(command == Finish);
}


Action::Action(Command command)
	:m_command(command)
{
	//M_ASSERT(command == Clear);
}


Action::Action(Command command, unsigned level, Key const& beforeKey)
	:m_command(command)
	,m_key1(beforeKey)
	,m_level(level)
{
	//M_ASSERT(command == Insert);
}


Action::Action(Command command, unsigned level, Key const& startKey, Key const& endKey)
	:m_command(command)
	,m_key1(startKey)
	,m_key2(endKey)
	,m_level(level)
{
	//M_ASSERT(command == Remove || command == Replace);
}


void
Action::visit(Visitor& visitor) const
{
	switch (m_command)
	{
		case Clear:		visitor.clear(); break;
		case Insert:	visitor.insert(m_level, m_key1); break;
		case Replace:	visitor.replace(m_level, m_key1, m_key2); break;
		case Remove:	visitor.remove(m_level, m_key1, m_key2); break;
		case Finish:	visitor.finish(m_level); break;
	}
}


Languages::Languages(MoveNode const* root)
{
	if (root)
		root->collectLanguages(m_langSet);
}


bool
Languages::operator==(Node const* node) const
{
	//M_ASSERT(node);
	//M_ASSERT(dynamic_cast<Languages const*>(node));

	return m_langSet == dynamic_cast<Languages const*>(node)->m_langSet;
}


Key const&
Variation::startKey() const
{
	return m_list.empty() ? m_key : m_list.front()->startKey();
}


Key const&
Variation::endKey() const
{
	return m_list.empty() ? m_key : m_list.back()->endKey();
}


bool
Variation::operator==(Node const* node) const
{
	//M_ASSERT(node);
	//M_ASSERT(dynamic_cast<Variation const*>(node));

	if (m_key != static_cast<Variation const*>(node)->m_key)
		return false;

	if (m_list.size() != static_cast<Variation const*>(node)->m_list.size())
		return false;

	for (unsigned i = 0; i < m_list.size(); ++i)
	{
		if (*m_list[i] != static_cast<Variation const*>(node)->m_list[i])
			return false;
	}

	return true;
}


void
Variation::visit(Visitor& visitor) const
{
	Key key(startKey());
	key.removePly();

	visitor.startVariation(key, startKey(), endKey());

	for (unsigned i = 0; i < m_list.size(); ++i)
		m_list[i]->visit(visitor);

	visitor.endVariation(key, startKey(), endKey());
}


void
Variation::difference(Root const* root, Variation const* var, unsigned level, Node::List& nodes) const
{
	//M_ASSERT(root);
	//M_ASSERT(var);

	Key const& endKey = var->successor();

	unsigned	i = 0;
	unsigned k = 0;
	unsigned m = m_list.size();
	unsigned n = var->m_list.size();

	if (m > 0 && n > 0)
	{
		KeyNode const* lhs = m_list[0];			// node from current game
		KeyNode const* rhs = var->m_list[0];	// node from previous game

		if (lhs->key() < rhs->key())
		{
			do
				++i;
			while (i < m && m_list[i]->key() < rhs->key());

			nodes.push_back(root->newAction(Action::Insert, level, rhs->startKey()));
			nodes.insert(nodes.end(), m_list.begin(), m_list.begin() + i);
			nodes.push_back(root->newAction(Action::Finish, level));
		}
		else if (rhs->key() < lhs->key())
		{
			do
				++k;
			while (k < n && var->m_list[k]->key() < lhs->key());

			Key const& after = k == n ? endKey : var->m_list[k]->startKey();
			nodes.push_back(root->newAction(Action::Remove, level, rhs->startKey(), after));
		}
	}

	while (i < m && k < n)
	{
		KeyNode const* lhs = m_list[i];			// node from current game
		KeyNode const* rhs = var->m_list[k];	// node from previous game

		Type lhsType = lhs->type();
		Type rhsType = rhs->type();

		if (lhsType == rhsType)
		{
			if (lhsType == TVariation)
			{
				Variation const* lhsVar = static_cast<Variation const*>(lhs);
				Variation const* rhsVar = static_cast<Variation const*>(rhs);

				lhsVar->difference(root, rhsVar, level + 1, nodes);
			}
			else if (*lhs != rhs)
			{
				if (	lhsType == TMove
					&& (	static_cast<Move const*>(lhs)->ply() == 0
						|| static_cast<Move const*>(rhs)->ply() == 0
						|| *static_cast<Move const*>(lhs)->ply() != static_cast<Move const*>(rhs)->ply()))
				{
					KeyNode const* const* lhsLast	= m_list.begin() + i + 1;
					KeyNode const* const* lhsEnd	= m_list.end();
					KeyNode const* const* rhsLast	= var->m_list.begin() + k + 1;
					KeyNode const* const* rhsEnd	= var->m_list.end();

					while	(	lhsLast < lhsEnd
							&& rhsLast < rhsEnd
							&& (*lhsLast)->type() == (*rhsLast)->type()
							&& (*lhsLast)->key() == (*rhsLast)->key()
							&& *static_cast<Node const*>(*lhsLast) == static_cast<Node const*>(*rhsLast))
					{
						++lhsLast;
						++rhsLast;
					}

					Key const& before	= rhs->endKey();
					Key const& after	= rhsLast == var->m_list.end() ? endKey : (*rhsLast)->startKey();

					nodes.push_back(root->newAction(Action::Replace, level, before, after));
					nodes.insert(nodes.end(), m_list.begin() + i, lhsLast);
					nodes.push_back(root->newAction(Action::Finish, level));
					i = lhsLast - m_list.begin() - 1;
					k = rhsLast - var->m_list.begin() - 1;
				}
				else
				{
					Key const& before	= rhs->endKey();
					Key const& after	= (k == n - 1) ? endKey : var->m_list[k + 1]->startKey();

					nodes.push_back(root->newAction(Action::Replace, level, before, after));
					nodes.push_back(lhs);
					nodes.push_back(root->newAction(Action::Finish, level));
				}
			}

			++i;
			++k;
		}
		else // if (lhsType != rhsType)
		{
			enum { Insert, Remove } action;

			switch (rhsType)
			{
				case TDiagram:		action = Remove; break;
				case TMove:			action = Insert; break;
				case TVariation:	action = (lhsType == TMove) ? Remove : Insert; break;
				default:				//M_ASSERT(!"should not happen");
					return;
			}

			switch (action)
			{
				case Insert:
					nodes.push_back(root->newAction(Action::Insert, level, rhs->startKey()));
					nodes.push_back(lhs);
					nodes.push_back(root->newAction(Action::Finish, level));
					++i;
					break;

				case Remove:
					Key const& after = (k == n - 1) ? endKey : var->m_list[k + 1]->startKey();
					nodes.push_back(root->newAction(Action::Remove, level, rhs->startKey(), after));
					++k;
					break;
			}
		}
	}

	if (i < m)
	{
		nodes.push_back(root->newAction(Action::Insert, level, successor()));
		nodes.insert(nodes.end(), m_list.begin() + i, m_list.end());
		nodes.push_back(root->newAction(Action::Finish, level));
	}

	if (k < n)
		nodes.push_back(root->newAction(Action::Remove, level, var->m_list[k]->startKey(), endKey));
}


bool
Move::operator==(Node const* node) const
{
	//M_ASSERT(node);
	//M_ASSERT(dynamic_cast<Move const*>(node));
	//M_ASSERT(m_key == static_cast<Move const*>(node)->m_key);

	if (m_list.size() != static_cast<Move const*>(node)->m_list.size())
		return false;

	for (unsigned i = 0; i < m_list.size(); ++i)
	{
		if (m_list[i]->type() != static_cast<Move const*>(node)->m_list[i]->type())
			return false;
		if (*m_list[i] != static_cast<Move const*>(node)->m_list[i])
			return false;
	}

	return true;
}


Move::Move(Work& work, MoveNode const* move, bool isEmptyGame, unsigned varNo, unsigned varCount)
	:KeyNode(work.key)
	,m_ply(0)
{
	if (work.m_level == 0)
	{
		if (work.isEmpty && isEmptyGame)
			m_list.push_back(new Space(Empty));
		else
			m_list.push_back(new Space(Start));
	}
	else
	{
		if (/*varCount > 1 && */(work.m_displayStyle & display::ShowVariationNumbers))
			work.pushOpen(varNo == 1, varNo);
		else
			work.pushOpen(varNo == 1);

		work.pushSpace();
		work.needMoveNo = true;
	}

	if (work.isFolded)
		return;

	if (move->hasMark())
	{
		work.pop(m_list);
		m_list.push_back(new Marks(move->marks()));
		work.m_isVirgin = false;
		work.pushSpace();
	}

	bool needSpace = false;

	mstl::string info;
	getMoveInfo(work, move, info);

	if (move->hasComment(move::Post))
	{
		db::Comment comm(move->comment(move::Post));
		comm.strip(*work.wantedLanguages);

		if (!comm.isEmpty())
		{
			work.pop(m_list);
			m_list.push_back(new Comment(comm, move::Post, Comment::AtStart));
			needSpace = true;
		}
	}

	if (!info.empty())
	{
		if (needSpace)
			work.pushSpace();
		work.pop(m_list);
		m_list.push_back(new Comment(db::Comment(info, false, false), move::Post, Comment::Finally));
		needSpace = true;
	}

	if (needSpace)
	{
		work.m_isVirgin = false;
//		work.pushParagraph(Spacing::PreComment);
		if (!move->hasMark())
			work.pushSpaceOrParagraph(Spacing::Comment);
		work.needMoveNo = true;
	}
}


Move::Move(Work& work, MoveNode const* move)
	:KeyNode(work.key)
{
	//M_ASSERT(!move->atLineStart());

	if (work.board.whiteToMove())
	{
		if (work.m_level == 0 && (work.m_displayStyle & display::ColumnStyle))
			work.pushBreak();
		work.needMoveNo = true;
	}

	if (!work.isFolded && move->hasComment(move::Ante))
	{
		db::Comment comment(move->comment(move::Ante));
		comment.strip(*work.wantedLanguages);

		if (!comment.isEmpty())
		{
			if (!move->prev()->hasMark() || move->prev()->hasComment(move::Post))
				work.pushSpaceOrParagraph(Spacing::Comment);
			work.pop(m_list);
			m_list.push_back(new Comment(comment, move::Ante));
			work.m_isVirgin = false;
			work.pushSpaceOrParagraph(Spacing::Comment);
			work.needMoveNo = true;
		}
	}

	work.pop(m_list);

	if (!work.isFolded && move->hasAnnotation())
	{
		m_list.push_back(new Annotation(
			move->annotation(),
			bool(work.m_displayStyle & display::ShowDiagrams)));
	}

	m_ply = work.needMoveNo ? new Ply(move, work.board.moveNumber()) : new Ply(move);
	m_list.push_back(m_ply);
	work.incrPlyCount();
	work.needMoveNo = false;
	work.pushSpace();

	if (work.isFolded)
		return;

	if (move->hasMark())
	{
		m_list.push_back(new Marks(move->marks()));
		work.pushSpace();
	}

	mstl::string info;
	getMoveInfo(work, move, info);

	bool needSpace = false;

	if (move->hasComment(move::Post))
	{
		db::Comment comment(move->comment(move::Post));
		comment.strip(*work.wantedLanguages);

		if (!comment.isEmpty())
		{
			bool isShort =		info.empty()
								&& comment.length() <= work.linebreakMinCommentLength
								&& bool(work.m_displayStyle & display::CompactStyle);

			if (isShort)
				work.pushSpace();
			else
				work.pushSpaceOrParagraph(Spacing::Comment);

			work.pop(m_list);
			m_list.push_back(new Comment(comment, move::Post));

			if (isShort)
				work.pushSpace();
			else
				needSpace = true;
		}
	}

	if (!info.empty())
	{
		if (needSpace)
			work.pushSpace();
		else
			work.pushSpaceOrParagraph(Spacing::Comment);

		work.pop(m_list);
		m_list.push_back(new Comment(db::Comment(info, false, false), move::Post, Comment::Finally));
		needSpace = true;
	}

	if (needSpace)
	{
		work.pushSpaceOrParagraph(Spacing::Comment);
		work.needMoveNo = true;
	}
}


Move::Move(Work& work, db::Comment const& comment, unsigned varNo, unsigned varCount)
	:KeyNode(work.key)
	,m_ply(0)
{
	if (!work.isFolded && !comment.isEmpty())
	{
		db::Comment comm(comment);
		comm.strip(*work.wantedLanguages);

		if (!comm.isEmpty())
		{
			work.pushSpaceOrParagraph(Spacing::Comment);
			work.pop(m_list);
			m_list.push_back(new Comment(comm, move::Post, Comment::AtEnd));
		}
	}

	if (work.m_level > 0)
	{
		Node::Bracket bracket;

		if (work.isFolded)
			bracket = Node::Fold;
		else if (/*varCount > 1 && */(work.m_displayStyle & display::ShowVariationNumbers))
			bracket = Node::End;
		else
			bracket = Node::Close;

		work.pushClose(varNo == varCount);
	   m_list.push_back(new Space(bracket, varNo == varCount));
	}
	else
	{
		m_list.push_back(new Space(0));
	}
}


Move::Move(Spacing& spacing, Key const& key, unsigned moveNumber, MoveNode const* move)
	:KeyNode(key)
{
	//M_ASSERT(!move->atLineStart());

	if (color::isWhite(move->move().color()) && (spacing.m_displayStyle & display::ColumnStyle))
		spacing.pushBreak();
	else
		spacing.pushSpace();

	spacing.pop(m_list);

	if (move->hasAnnotation())
		m_list.push_back(new Annotation(move->annotation()));

	m_ply = color::isWhite(move->move().color()) ? new Ply(move, moveNumber) : new Ply(move);
	m_list.push_back(m_ply);
	spacing.incrPlyCount();

	if (move->hasMark())
		m_list.push_back(new Marks(move->marks()));
}


void
Move::getMoveInfo(Work& work, MoveNode const* move, mstl::string& result)
{
	if (bool(work.m_displayStyle & display::ShowMoveInfo) && move->hasMoveInfo())
	{
		//M_ASSERT(work.engineList);

		move->moveInfo().print(*work.engineList, result, MoveInfo::Text);

		if (!result.empty())
		{
			result.insert(result.begin(), "<xml><b>", 8);
			result.append("</b></xml>", 10);
		}
	}
}


void
Move::visit(Visitor& visitor) const
{
	visitor.startMove(m_key);

	for (unsigned i = 0; i < m_list.size(); ++i)
		m_list[i]->visit(visitor);

	visitor.endMove(m_key);
}


Root::~Root() throw()
{
	delete m_opening;
	delete m_languages;
	delete m_variation;

	for (unsigned i = 0; i < m_nodes.size(); ++i)
		delete m_nodes[i];
}


void
Root::visit(Visitor& visitor) const
{
	visitor.start(m_result);
	m_opening->visit(visitor);
	m_languages->visit(visitor);
	m_variation->visit(visitor);
	visitor.finish(m_result);
}


Node*
Root::newAction(Action::Command command) const
{
	Node* node = new Action(command);
	m_nodes.push_back(node);
	return node;
}


Node*
Root::newAction(Action::Command command, unsigned level) const
{
	Node* node = new Action(command, level);
	m_nodes.push_back(node);
	return node;
}


Node*
Root::newAction(Action::Command command, unsigned level, Key const& beforeKey) const
{
	Node* node = new Action(command, level, beforeKey);
	m_nodes.push_back(node);
	return node;
}


Node*
Root::newAction(Action::Command command, unsigned level, Key const& startKey, Key const& endKey) const
{
	Node* node = new Action(command, level, startKey, endKey);
	m_nodes.push_back(node);
	return node;
}


void
Root::difference(Root const* root, List& nodes) const
{
	for (unsigned i = 0; i < m_nodes.size(); ++i)
		delete m_nodes[i];

	if (root == 0)
	{
		nodes.push_back(m_opening);
		nodes.push_back(m_languages);
		nodes.push_back(newAction(Action::Clear));

		if (!m_variation->empty())
		{
			nodes.push_back(newAction(Action::Insert, 0, Key()));
			nodes.push_back(m_variation);
		}
	}
	else
	{
		if (*m_opening != root->m_opening)
			nodes.push_back(m_opening);

		if (*m_languages != root->m_languages)
			nodes.push_back(m_languages);

		m_variation->difference(this, root->m_variation, 0, nodes);
	}
}


Root*
Root::makeList(TagSet const& tags,
					uint16_t idn,
					Eco eco,
					db::Board const& startBoard,
					MoveNode const* node,
					unsigned linebreakThreshold,
					unsigned linebreakMaxLineLength,
					unsigned displayStyle)
{
	//M_REQUIRE(node);
	//M_REQUIRE(node->atLineStart());
	/*M_REQUIRE(	(displayStyle & display::CompactStyle) == display::CompactStyle
				|| (displayStyle & display::ColumnStyle) == display::ColumnStyle);
*/
	if (node->countHalfMoves() <= linebreakThreshold)
		linebreakMaxLineLength = ::DontSetBreaks;

	unsigned	moveNumber	= startBoard.moveNumber();
	unsigned	plyNumber	= startBoard.plyNumber();
	Spacing	spacing;
	Key		key;

	Root* root = new Root;

	root->m_opening = new Opening(startBoard, idn, eco);
	root->m_languages = new Languages;
	root->m_variation = new Variation(key);
	root->m_result = result::fromString(tags.value(tag::Result));

	KeyNode::List& result = root->m_variation->m_list;

	result.reserve(2*node->countHalfMoves() + 10);
	key.addPly(plyNumber);
	result.push_back(new Move(key));
	spacing.m_linebreakMaxLineLength = linebreakMaxLineLength;
	spacing.m_displayStyle = displayStyle;

	for (node = node->next(); node->isBeforeLineEnd(); node = node->next())
	{
		key.exchangePly(++plyNumber);
		result.push_back(new Move(spacing, key, moveNumber, node));
		if (color::isBlack(node->move().color()))
			++moveNumber;
	}

	return root;
}


Root*
Root::makeList(TagSet const& tags,
					uint16_t idn,
					Eco eco,
					db::Board const& startBoard,
					LanguageSet const& langSet, // unused
					LanguageSet const& wantedLanguages,
					EngineList const& engines,
					MoveNode const* node,
					unsigned linebreakThreshold,
					unsigned linebreakMaxLineLength,
					unsigned linebreakMaxLineLengthVar,
					unsigned linebreakMinCommentLength,
					unsigned displayStyle)
{
	//M_REQUIRE(node);
	//M_REQUIRE(node->atLineStart());
	/*M_REQUIRE(	(displayStyle & display::CompactStyle) == display::CompactStyle
				|| (displayStyle & display::ColumnStyle) == display::ColumnStyle);
*/
	Work work;
	work.board = startBoard;
	work.languages = new Languages(node);
	work.engineList = &engines;
	work.wantedLanguages = &wantedLanguages;
	work.linebreakMaxLineLengthVar = linebreakMaxLineLengthVar;
	work.linebreakMinCommentLength = linebreakMinCommentLength;
	work.isEmpty = node->isEmptyLine();
	work.m_displayStyle = displayStyle;

	if ((displayStyle & display::CompactStyle) && node->countHalfMoves() > linebreakThreshold)
		work.m_linebreakMaxLineLength = linebreakMaxLineLength;

	Root*			root	= new Root;
	Variation*	var	= new Variation(work.key);

	root->m_opening = new Opening(startBoard, idn, eco);
	root->m_languages = work.languages;
	root->m_variation = var;
	root->m_result = result::fromString(tags.value(tag::Result));

	makeList(work, var->m_list, node, 1, 1);

	return root;
}


void
Root::makeList(Work& work,
					KeyNode::List& result,
					MoveNode const* node,
					unsigned varNo,
					unsigned varCount)
{
	//M_ASSERT(node);
	//M_ASSERT(node->atLineStart());

	bool isFolded = node->isFolded();

	work.isFolded = isFolded;
	result.reserve(2*node->countHalfMoves() + 10);
	work.key.addPly(work.board.plyNumber());

	if (node->prev())
		++work.m_level;

	result.push_back(new Move(work, node, node->isEmptyLine(), varNo, varCount));

	if (	!work.isFolded
		&& (work.m_displayStyle & display::ShowDiagrams)
		&& (	node->annotation().contains(nag::Diagram)
			|| node->annotation().contains(nag::DiagramFromBlack)))
	{
		work.pushParagraph(Spacing::Diagram);
		work.pop(const_cast<Move*>(static_cast<Move const*>(result.back()))->m_list);
		result.push_back(new Diagram(
			work, node->annotation().contains(nag::Diagram) ? color::White : color::Black));
		work.m_isVirgin = false;
		work.pushParagraph(Spacing::Diagram);
		work.needMoveNo = true;
	}

	work.key.removePly();
	node = node->next();
	//M_ASSERT(node);

	if (work.isFolded)
	{
//		too confusing!
//		if (node->next()->atLineEnd() && !node->hasNote() && !node->prev()->hasNote())
//			work.isFolded = false;
		work.key.addPly(work.board.plyNumber() + 1);
		result.push_back(new Move(work, node));
		work.board.doMove(node->move());
		work.key.removePly();
	}
	else
	{
		for ( ; !node->atLineEnd(); node = node->next())
		{
			work.key.addPly(work.board.plyNumber() + 1);
			result.push_back(new Move(work, node));

			if (	(work.m_displayStyle & display::ShowDiagrams)
				&& (	node->annotation().contains(nag::Diagram)
					|| node->annotation().contains(nag::DiagramFromBlack)))
			{
				work.pushParagraph(Spacing::Diagram);
				work.pop(const_cast<Move*>(static_cast<Move const*>(result.back()))->m_list);
				work.board.doMove(node->move());
				result.push_back(new Diagram(
					work,
					node->annotation().contains(nag::Diagram) ? color::White : color::Black));
				work.board.undoMove(node->move());
				work.pushParagraph(Spacing::Diagram);
				work.needMoveNo = true;
			}

			if (node->hasVariation())
			{
				work.pushParagraph(Spacing::StartVariation);

				for (unsigned i = 0; i < node->variationCount(); ++i)
				{
					Board	board(work.board);
					Key	succKey(work.key);

					if (i + 1 < node->variationCount())
					{
						succKey.addVariation(i + 1);
						succKey.addPly(work.board.plyNumber());
					}
					else
					{
						succKey.exchangePly(work.board.plyNumber() + 2);
					}

					work.key.addVariation(i);
					work.pushBreak();
					work.needMoveNo = true;

					Variation* var = new Variation(work.key, succKey);
					result.push_back(var);

					unsigned linebreakMaxLineLength = work.m_linebreakMaxLineLength;

					if (	work.m_linebreakMaxLineLength != ::DontSetBreaks
						&& work.linebreakMaxLineLengthVar > 0
						&& node->variation(i)->countNodes() > work.linebreakMaxLineLengthVar)
					{
						work.m_linebreakMaxLineLength = work.linebreakMaxLineLengthVar;
					}
					else
					{
						work.m_linebreakMaxLineLength = ::DontSetBreaks;
					}

					makeList(work, var->m_list, node->variation(i), i + 1, node->variationCount());
					work.m_linebreakMaxLineLength = linebreakMaxLineLength;
					//M_ASSERT(work.m_level > 0);
					--work.m_level;
					work.key.removeVariation();
					work.board = board;
				}

				work.pushParagraph(Spacing::EndVariation);
				work.needMoveNo = true;
			}

			work.isFolded = isFolded;
			work.board.doMove(node->move());
			work.key.removePly();
		}
	}

	work.key.addPly(work.board.plyNumber() + 1);
	result.push_back(new Move(work, node->comment(move::Post), varNo, varCount));
	work.key.removePly();
}

// vi:set ts=3 sw=3:
