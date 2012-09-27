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

#include "db_move_node.h"
#include "db_annotation.h"
#include "db_mark_set.h"
#include "db_move_info_set.h"
#include "db_board.h"

#include "u_crc.h"

#include "m_utility.h"
#include "m_assert.h"

#include <string.h>
#include <ctype.h>

using namespace db;


static MarkSet const NoMarks;
static MoveInfoSet const NoMoveInfo;


MoveNode::MoveNode(Move const& move)
	:m_flags(0)
	,m_next(0)
	,m_prev(0)
	,m_annotation(const_cast<Annotation*>(Annotation::defaultSet(nag::Null)))
	,m_marks(const_cast<MarkSet*>(&::NoMarks))
	,m_moveInfo(const_cast<MoveInfoSet*>(&::NoMoveInfo))
	,m_move(move)
	,m_commentFlag(0)
{
}


MoveNode::MoveNode(Board const& board, Move const& move)
	:m_flags(0)
	,m_next(0)
	,m_prev(0)
	,m_annotation(const_cast<Annotation*>(Annotation::defaultSet(nag::Null)))
	,m_marks(const_cast<MarkSet*>(&::NoMarks))
	,m_moveInfo(const_cast<MoveInfoSet*>(&::NoMoveInfo))
	,m_move(move)
	,m_commentFlag(0)
{
	//M_REQUIRE(board.isValidMove(move));

	board.prepareUndo(m_move);
	board.prepareForPrint(m_move);
}


MoveNode::MoveNode(MoveNode* node)
	:m_flags(0)
	,m_next(node)
	,m_prev(0)
	,m_annotation(const_cast<Annotation*>(Annotation::defaultSet(nag::Null)))
	,m_marks(const_cast<MarkSet*>(&::NoMarks))
	,m_moveInfo(const_cast<MoveInfoSet*>(&::NoMoveInfo))
	,m_commentFlag(0)
{
	//M_REQUIRE(node);
	m_next->m_prev = this;
}


MoveNode::MoveNode(Annotation* set)
	:m_flags(0)
	,m_next(0)
	,m_prev(0)
	,m_annotation(set ? set : const_cast<Annotation*>(Annotation::defaultSet(nag::Null)))
	,m_marks(const_cast<MarkSet*>(&::NoMarks))
	,m_moveInfo(const_cast<MoveInfoSet*>(&::NoMoveInfo))
	,m_commentFlag(0)
{
}


MoveNode::~MoveNode()
{
	if (!m_annotation->isDefaultSet())
		delete m_annotation;

	if (m_marks != &::NoMarks)
		delete m_marks;

	if (m_moveInfo != &::NoMoveInfo)
		delete m_moveInfo;

	for (unsigned i = 0; i < m_variations.size(); ++i)
		delete m_variations[i];

	delete m_next;
}


bool
MoveNode::isEmptyLine() const
{
	if (hasNote())
		return false;

	if (atLineEnd())
		return true;

	if (!atLineStart())
		return false;

	return m_next->atLineEnd() && !m_next->hasNote();
}


void
MoveNode::setMove(Board const& board, Move const& move)
{
	//M_REQUIRE(board.isValidMove(move));

	m_move = move;
	board.prepareUndo(m_move);
	board.prepareForPrint(m_move);
}


void
MoveNode::addAnnotation(nag::ID nag)
{
	if (nag == nag::Null)
		return;

	switch (m_annotation->count())
	{
		case 0:
			m_annotation = const_cast<Annotation*>(Annotation::defaultSet(nag));
			break;

		case 1:
			m_annotation = new Annotation(*m_annotation);
			// fallthru

		default:
			if (!m_annotation->contains(nag))
				m_annotation->add(nag);
			break;
	}

	m_flags |= HasAnnotation;
}


void
MoveNode::clearAnnotation()
{
	if (m_annotation->count() > 1)
		delete m_annotation;

	m_annotation = const_cast<Annotation*>(Annotation::defaultSet(nag::Null));
	m_flags &= ~HasAnnotation;
}


void
MoveNode::clearMarks()
{
	if (m_marks != &::NoMarks)
	{
		delete m_marks;
		m_marks = const_cast<MarkSet*>(&::NoMarks);
		m_flags &= ~HasMark;
	}
}


void
MoveNode::addMark(Mark const& mark)
{
	if (m_marks == &::NoMarks)
		m_marks = new MarkSet;

	m_marks->add(mark);
	m_flags |= HasMark;
}


void
MoveNode::addMoveInfo(MoveInfo const& moveInfo)
{
	if (m_moveInfo == &::NoMoveInfo)
		m_moveInfo = new MoveInfoSet;

	m_moveInfo->add(moveInfo);
	m_flags |= HasMoveInfo;
}


void
MoveNode::setupAnnotation(Annotation const& annotation)
{
	if (annotation.isEmpty())
	{
		if (!m_annotation->isDefaultSet())
			delete m_annotation;

		 m_annotation = const_cast<Annotation*>(Annotation::defaultSet(nag::Null));
		 m_flags &= ~HasAnnotation;
	}
	else
	{
		if (annotation.count() == 1)
		{
			if (!m_annotation->isDefaultSet())
				delete m_annotation;

			m_annotation = const_cast<Annotation*>(Annotation::defaultSet(annotation[0]));
		}
		else if (m_annotation->count() > 1)
		{
			*m_annotation = annotation;
		}
		else
		{
			m_annotation = new Annotation(annotation);
		}

		m_flags |= HasAnnotation;
	}
}


void
MoveNode::replaceAnnotation(Annotation const& annotation)
{
	setupAnnotation(annotation);
	m_annotation->sort();
}


void
MoveNode::swapMoveInfo(MoveInfoSet& moveInfo)
{
	if (m_moveInfo != &::NoMoveInfo)
	{
		m_moveInfo->swap(moveInfo);

		if (m_moveInfo->isEmpty())
			m_flags &= ~HasMoveInfo;
		else
			m_flags |= HasMoveInfo;
	}
	else if (!moveInfo.isEmpty())
	{
		m_moveInfo = new MoveInfoSet;
		m_moveInfo->swap(moveInfo);
		m_flags |= HasMoveInfo;
	}
}


void
MoveNode::replaceMoveInfo(MoveInfoSet const& moveInfo)
{
	//M_REQUIRE(!hasMoveInfo());

	if (moveInfo.isEmpty())
	{
		if (m_moveInfo != &::NoMoveInfo)
			delete m_moveInfo;

		m_moveInfo = const_cast<MoveInfoSet*>(&::NoMoveInfo);
		m_flags &= HasMoveInfo;
	}
	else
	{
		m_moveInfo = new MoveInfoSet(moveInfo);
		m_moveInfo->sort();
		m_flags |= HasMoveInfo;
	}
}


void
MoveNode::swapMarks(MarkSet& marks)
{
	if (m_marks != &::NoMarks)
	{
		m_marks->swap(marks);

		if (m_marks->isEmpty())
			m_flags &= ~HasMark;
		else
			m_flags |= HasMark;
	}
	else if (!marks.isEmpty())
	{
		m_marks = new MarkSet;
		m_marks->swap(marks);
		m_flags |= HasMark;
	}
}


void
MoveNode::replaceMarks(MarkSet const& marks)
{
	if (marks.isEmpty())
	{
		if (m_marks != &::NoMarks)
			delete m_marks;

		m_marks = const_cast<MarkSet*>(&::NoMarks);
		m_flags &= ~HasMark;
	}
	else
	{
		m_marks = new MarkSet(marks);
		m_marks->sort();
		m_flags |= HasMark;
	}
}


void
MoveNode::setNext(MoveNode* next)
{
	//M_REQUIRE(next);
	//M_REQUIRE(!next->prev());

	delete m_next;
	m_next = next;
	next->m_prev = this;
}


void
MoveNode::deleteNext()
{
	delete m_next;
	m_next = 0;
}


MoveNode*
MoveNode::removeNext()
{
	MoveNode* next = m_next;

	if (next)
	{
		next->m_prev = 0;
		m_next = 0;
	}

	return next;
}


void
MoveNode::addVariation(MoveNode* variation)
{
	//M_REQUIRE(variation);
	//M_REQUIRE(!variation->prev());
	//M_REQUIRE(!variation->move());

	m_variations.push_back(variation);
	variation->m_prev = this;
	m_flags |= HasVariation;
}


void
MoveNode::deleteVariation(unsigned varNo)
{
	//M_REQUIRE(varNo < variationCount());

	delete m_variations[varNo];
	m_variations.erase(m_variations.begin() + varNo);

	if (m_variations.empty())
		m_flags &= ~HasVariation;
}


void
MoveNode::swapVariations(unsigned varNo1, unsigned varNo2)
{
	//M_REQUIRE(varNo1 < variationCount());
	//M_REQUIRE(varNo2 < variationCount());

	mstl::swap(m_variations[varNo1], m_variations[varNo2]);
}


void
MoveNode::swapData(MoveNode* node)
{
	//M_REQUIRE(node);

	mstl::swap(m_flags,			node->m_flags);
	mstl::swap(m_annotation,	node->m_annotation);
	mstl::swap(m_marks,			node->m_marks);
	mstl::swap(m_moveInfo,		node->m_moveInfo);
	mstl::swap(m_comment[0],	node->m_comment[0]);
	mstl::swap(m_comment[1],	node->m_comment[1]);
}


MoveNode*
MoveNode::removeVariation(unsigned varNo)
{
	//M_REQUIRE(varNo < variationCount());

	MoveNode* variation = m_variations[varNo];

	variation->m_prev = 0;
	m_variations.erase(m_variations.begin() + varNo);

	if (m_variations.empty())
		m_flags &= ~HasVariation;

	return variation;
}


MoveNode*
MoveNode::replaceVariation(unsigned varNo, MoveNode* node)
{
	//M_REQUIRE(varNo < variationCount());
	//M_REQUIRE(node);
	//M_REQUIRE(!node->prev());
	//M_REQUIRE(!node->move());

	MoveNode* variation = m_variations[varNo];
	node->m_prev = variation->m_prev;
	m_variations[varNo] = node;
	return variation;
}


MoveNode*
MoveNode::clone(MoveNode* prev) const
{
	MoveNode* root = 0;

	for (MoveNode const* n = this; n; n = n->m_next)
	{
		MoveNode* node = new MoveNode;

		if (root == 0)
			root = node;

		if (prev)
		{
			prev->m_next = node;
			node->m_prev = prev;
		}

		prev = node;

		if (!n->m_annotation->isEmpty())
			node->setupAnnotation(*n->m_annotation);

		if (!n->m_marks->isEmpty())
			node->m_marks = new MarkSet(*n->m_marks);

		if (!n->m_moveInfo->isEmpty())
			node->m_moveInfo = new MoveInfoSet(*n->m_moveInfo);

		node->m_comment[0] = n->m_comment[0];
		node->m_comment[1] = n->m_comment[1];
		node->m_move = n->m_move;
		node->m_flags = n->m_flags;

		for (unsigned i = 0; i < n->m_variations.size(); ++i)
			node->addVariation(n->m_variations[i]->clone());
	}

	return root;
}


unsigned
MoveNode::variationNumber(MoveNode const* node) const
{
	//M_REQUIRE(node->prev() == this);

	for (unsigned i = 0; i < m_variations.size(); ++i)
	{
		if (node == m_variations[i])
			return i;
	}

	return 0;	// not reached
}


void
MoveNode::prepareForPrint(Board const& board)
{
	board.prepareForPrint(m_move);
}


unsigned
MoveNode::countHalfMoves() const
{
	MoveNode const* p = atLineStart() ? m_next : this;
	unsigned count = 0;

	for ( ; p->isBeforeLineEnd(); p = p->m_next)
		++count;

	return count;
}


unsigned
MoveNode::countNodes() const
{
	MoveNode const* p = atLineStart() ? m_next : this;
	unsigned count = 0;

	for ( ; p->isBeforeLineEnd(); p = p->m_next)
	{
		++count;

		if (p->hasVariation())
		{
			for (unsigned i = 0; i < p->variationCount(); ++i)
				count += p->variation(i)->countNodes();
		}
	}

	return count;
}


unsigned
MoveNode::countAnnotations() const
{
	unsigned result = 0;

	for (MoveNode const* n = this; n; n = n->m_next)
	{
		result += n->m_annotation->count();

		for (unsigned i = 0; i < n->m_variations.size(); ++i)
			result += n->m_variations[i]->countAnnotations();
	}

	return result;
}


unsigned
MoveNode::countMoveInfo() const
{
	unsigned result = 0;

	for (MoveNode const* n = this; n; n = n->m_next)
	{
		result += n->m_moveInfo->count();

		for (unsigned i = 0; i < n->m_variations.size(); ++i)
			result += n->m_variations[i]->countMoveInfo();
	}

	return result;
}


unsigned
MoveNode::countMarks() const
{
	unsigned result = 0;

	for (MoveNode const* n = this; n; n = n->m_next)
	{
		result += n->m_marks->count();

		for (unsigned i = 0; i < n->m_variations.size(); ++i)
			result += n->m_variations[i]->countMarks();
	}

	return result;
}


unsigned
MoveNode::countComments() const
{
	unsigned result = 0;

	for (MoveNode const* n = this; n; n = n->m_next)
	{
		result += (n->m_comment[0].isEmpty() ? 0 : 1) + (n->m_comment[1].isEmpty() ? 0 : 1);

		for (unsigned i = 0; i < n->m_variations.size(); ++i)
			result += n->m_variations[i]->countComments();
	}

	return result;
}


unsigned
MoveNode::countComments(mstl::string const& lang) const
{
	unsigned result = (m_comment[0].countLength(lang) ? 1 : 0) + (m_comment[1].countLength(lang) ? 1 : 0);

	for (MoveNode const* n = this; n; n = n->m_next)
	{
		result += (n->m_comment[0].countLength(lang) ? 1 : 0);
		result += (n->m_comment[1].countLength(lang) ? 1 : 0);

		for (unsigned i = 0; i < n->m_variations.size(); ++i)
			result += n->m_variations[i]->countComments(lang);
	}

	return result;
}


unsigned
MoveNode::countVariations() const
{
	unsigned result = 0;

	for (MoveNode const* n = this; n; n = n->m_next)
		result += n->m_variations.size();

	return result;
}


unsigned
MoveNode::unfoldedVariationCount() const
{
	unsigned n = 0;

	for (unsigned i = 0; i < variationCount(); ++i)
	{
		if (!variation(i)->isFolded())
			++n;
	}

	return n;
}


void
MoveNode::stripAnnotations()
{
	for (MoveNode* n = this; n; n = n->m_next)
	{
		if (!n->m_annotation->isDefaultSet())
			delete n->m_annotation;

		n->m_annotation = const_cast<Annotation*>(Annotation::defaultSet(nag::Null));
		n->m_flags &= ~HasAnnotation;

		for (unsigned i = 0; i < n->m_variations.size(); ++i)
			n->m_variations[i]->stripAnnotations();
	}
}


void
MoveNode::stripMoveInfo()
{
	for (MoveNode* n = this; n; n = n->m_next)
	{
		n->m_flags &= ~HasMoveInfo;

		if (n->m_moveInfo != &::NoMoveInfo)
		{
			delete n->m_moveInfo;
			n->m_moveInfo = const_cast<MoveInfoSet*>(&::NoMoveInfo);
		}

		for (unsigned i = 0; i < n->m_variations.size(); ++i)
			n->m_variations[i]->stripMoveInfo();
	}
}


void
MoveNode::stripMarks()
{
	for (MoveNode* n = this; n; n = n->m_next)
	{
		n->m_flags &= ~HasMark;

		if (n->m_marks != &::NoMarks)
		{
			delete n->m_marks;
			n->m_marks = const_cast<MarkSet*>(&::NoMarks);
		}

		for (unsigned i = 0; i < n->m_variations.size(); ++i)
			n->m_variations[i]->stripMarks();
	}
}


void
MoveNode::stripComments()
{
	for (MoveNode* n = this; n; n = n->m_next)
	{
		n->m_comment[0].clear();
		n->m_comment[1].clear();

		n->m_flags &= ~(HasComment | HasPreComment | IsPrepared);

		for (unsigned i = 0; i < n->m_variations.size(); ++i)
			n->m_variations[i]->stripComments();
	}
}


void
MoveNode::stripComments(mstl::string const& lang)
{
	for (MoveNode* n = this; n; n = n->m_next)
	{
		n->m_comment[move::Ante].remove(lang);
		n->m_comment[move::Post].remove(lang);

		n->m_flags &= ~IsPrepared;

		if (n->m_comment[move::Ante].isEmpty())
			n->m_flags &= ~HasPreComment;
		if (n->m_comment[move::Post].isEmpty())
			n->m_flags &= ~HasComment;

		for (unsigned i = 0; i < n->m_variations.size(); ++i)
			n->m_variations[i]->stripComments(lang);
	}
}


void
MoveNode::copyComments(mstl::string const& fromLang, mstl::string const& toLang, bool stripOriginal)
{
	for (MoveNode* n = this; n; n = n->m_next)
	{
		if (!n->m_comment[move::Ante].isEmpty())
			n->m_comment[move::Ante].copy(fromLang, toLang, stripOriginal);
		if (!n->m_comment[move::Post].isEmpty())
			n->m_comment[move::Post].copy(fromLang, toLang, stripOriginal);

		for (unsigned i = 0; i < n->m_variations.size(); ++i)
			n->m_variations[i]->copyComments(fromLang, toLang, stripOriginal);
	}
}


void
MoveNode::stripVariations()
{
	for (MoveNode* n = this; n; n = n->m_next)
	{
		for (unsigned i = 0; i < n->m_variations.size(); ++i)
			delete n->m_variations[i];

		n->m_variations.clear();
		n->m_flags &= ~HasVariation;
	}
}


void
MoveNode::fold(bool flag)
{
	for (MoveNode* n = this; n; n = n->m_next)
	{
		if (n->hasVariation())
		{
			for (unsigned i = 0; i < n->m_variations.size(); ++i)
			{
				n->m_variations[i]->setFolded(flag);
				n->m_variations[i]->fold(flag);
			}
		}
	}
}


void
MoveNode::transpose()
{
	for (MoveNode* n = this; n; n = n->m_next)
	{
		if (n->m_move)
			n->m_move.transpose();

		for (unsigned i = 0; i < n->m_variations.size(); ++i)
			n->m_variations[i]->transpose();
	}
}


void
MoveNode::finish(Board const& board)
{
	//M_REQUIRE(atLineStart());

	Board myBoard(board);

	for (MoveNode* node = m_next; node->isBeforeLineEnd(); node = node->m_next)
	{
		node->m_annotation->sort();
		node->m_marks->sort();
		node->m_moveInfo->sort();

		for (unsigned i = 0; i < node->m_variations.size(); ++i)
			node->m_variations[i]->finish(myBoard);

		myBoard.prepareUndo(node->m_move);
		myBoard.prepareForPrint(node->m_move);
		myBoard.doMove(node->m_move);
	}
}


void
MoveNode::setMark()
{
	m_flags |= HasMark;
	addMark(Mark());
}


util::crc::checksum_t
MoveNode::computeChecksum(EngineList const& engines, util::crc::checksum_t crc) const
{
	if (m_move)
		crc = m_move.computeChecksum(crc);

	crc = m_annotation->computeChecksum(crc);
	crc = m_marks->computeChecksum(crc);
	crc = m_moveInfo->computeChecksum(engines, crc);

	if (!m_comment[move::Ante].isEmpty())
		crc = m_comment[move::Ante].computeChecksum(crc);
	if (!m_comment[move::Post].isEmpty())
		crc = m_comment[move::Post].computeChecksum(crc);

	if (m_next)
		crc = m_next->computeChecksum(engines, crc);

	for (unsigned i = 0; i < variationCount(); ++i)
		crc = variation(i)->computeChecksum(engines, crc);

	return crc;
}


void
MoveNode::collectLanguages(LanguageSet& langSet) const
{
	for (MoveNode const* n = this; n; n = n->m_next)
	{
		n->m_comment[move::Ante].collectLanguages(langSet);
		n->m_comment[move::Post].collectLanguages(langSet);

		for (unsigned i = 0; i < n->variationCount(); ++i)
			n->variation(i)->collectLanguages(langSet);
	}
}


bool
MoveNode::containsIllegalMoves() const
{
	for (MoveNode const* p = atLineStart() ? m_next : this; p->isBeforeLineEnd(); p = p->m_next)
	{
		if (!p->m_move.isLegal() && !p->m_move.isCastling())
			return true;

#ifdef ILLEGAL_MOVES_IN_VARIATIONS
		if (p->hasVariation())
		{
			for (unsigned i = 0; i < p->variationCount(); ++i)
			{
				if (p->variation(i)->containsIllegalMoves())
					return true;
			}
		}
#endif
	}

	return false;
}


bool
MoveNode::containsIllegalCastlings() const
{
	for (MoveNode const* p = atLineStart() ? m_next : this; p->isBeforeLineEnd(); p = p->m_next)
	{
		if (!p->m_move.isLegal() && p->m_move.isCastling())
			return true;

#ifdef ILLEGAL_MOVES_IN_VARIATIONS
		if (p->hasVariation())
		{
			for (unsigned i = 0; i < p->variationCount(); ++i)
			{
				if (p->variation(i)->containsIllegalCastlings())
					return true;
			}
		}
#endif
	}

	return false;
}


bool
MoveNode::contains(MoveNode const* node) const
{
	for (MoveNode const* p = this; p; p = p->m_next)
	{
		if (p == node)
			return true;

		if (p->hasVariation())
		{
			for (unsigned i = 0; i < p->variationCount(); ++i)
			{
				if (p->variation(i)->contains(node))
					return true;
			}
		}
	}

	return false;
}


bool
MoveNode::containsEnglishLang() const
{
	for (MoveNode const* p = this; p; p = p->m_next)
	{
		if (p->m_comment[0].engFlag() || p->m_comment[1].engFlag())
			return true;
	}

	return false;
}


bool
MoveNode::containsOtherLang() const
{
	for (MoveNode const* p = this; p; p = p->m_next)
	{
		if (p->m_comment[0].othFlag() || p->m_comment[1].othFlag())
			return true;
	}

	return false;
}


void
MoveNode::unfold()
{
	getLineStart()->m_flags &= ~IsFolded;
}


MoveNode*
MoveNode::getLineStart()
{
	MoveNode* node = this;

	while (node->isAfterLineStart())
		node = node->m_prev;

	return node;
}


MoveNode*
MoveNode::getLineEnd()
{
	MoveNode* node = this;

	while (node->isBeforeLineEnd())
		node = node->m_next;

	return node;
}


MoveNode*
MoveNode::getOneBeforeLineEnd()
{
	MoveNode* node = this;

	while (node->isBeforeLineEnd())
		node = node->m_next;

	return node->m_prev;
}


bool MoveNode::checkHasMark() const			{ return !m_marks->isEmpty(); }
bool MoveNode::checkHasAnnotation() const	{ return !m_annotation->isEmpty(); }

// vi:set ts=3 sw=3:
