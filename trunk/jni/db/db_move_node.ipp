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

#include "m_assert.h"

namespace db {

inline bool MoveNode::atLineStart() const					{ return m_prev == 0 || m_prev->m_next != this; }
inline bool MoveNode::atLineEnd() const					{ return m_next == 0; }
inline bool MoveNode::isBeforeLineEnd() const			{ return m_next != 0; }
inline bool MoveNode::isOneBeforeLineEnd() const		{ return m_next != 0 && m_next->m_next == 0; }
inline bool MoveNode::isAfterLineStart() const			{ return m_prev != 0 && m_prev->m_next == this; }
inline bool MoveNode::hasAnyComment() const				{ return m_flags & (HasComment | HasPreComment); }
inline bool MoveNode::hasMark() const						{ return m_flags & HasMark; }
inline bool MoveNode::hasAnnotation() const				{ return m_flags & HasAnnotation; }
inline bool MoveNode::hasVariation() const				{ return m_flags & HasVariation; }
inline bool MoveNode::hasNote() const						{ return m_flags & HasNote; }
inline bool MoveNode::hasSupplement() const				{ return m_flags & HasSupplement; }
inline bool MoveNode::hasMoveInfo() const					{ return m_flags & HasMoveInfo; }
inline bool MoveNode::isFolded() const						{ return m_flags & IsFolded; }

inline bool MoveNode::hasComment(move::Position position) const	 { return m_flags & (1 << position); }

inline MoveNode* MoveNode::prev() const					{ return m_prev; }
inline Move& MoveNode::move()									{ return m_move; }
inline Move const& MoveNode::move() const					{ return m_move; }
inline MoveNode* MoveNode::next() const					{ return m_next; }

inline unsigned MoveNode::variationCount() const		{ return m_variations.size(); }
inline MoveNode* MoveNode::clone() const					{ return clone(0); }
inline Annotation const& MoveNode::annotation() const	{ return *m_annotation; }
inline MarkSet const& MoveNode::marks() const			{ return *m_marks; }
inline Byte MoveNode::commentFlag() const					{ return m_commentFlag; }

inline Comment const& MoveNode::comment(move::Position position) const { return m_comment[position]; }

inline void MoveNode::setComment(move::Position position)		{ m_flags |= (1 << position); }
inline void MoveNode::unsetComment(move::Position position)	{ m_flags &= ~(1 << position); }


inline
void
MoveNode::setCommentFlag(Byte flag)
{
	m_commentFlag = flag;
	m_flags |= IsPrepared;
}


inline
MoveInfoSet const& MoveNode::moveInfo() const
{
	//M_REQUIRE(hasMoveInfo());
	return *m_moveInfo;
}


inline
void
MoveNode::setInfoFlag(bool flag)
{
	if (flag)
		m_flags |= HasMoveInfo;
	else
		m_flags &= ~HasMoveInfo;
}


inline
void
MoveNode::setFolded(bool flag)
{
	if (flag)
		m_flags |= IsFolded;
	else
		m_flags &= ~IsFolded;
}


inline
MoveNode*
MoveNode::variation(unsigned i) const
{
	//M_REQUIRE(i < variationCount());
	return m_variations[i];
}


inline
MoveNode::Nodes const&
MoveNode::variations() const
{
	return m_variations;
}


inline
void
MoveNode::updateCommentFlags(move::Position position)
{
	m_flags &= ~IsPrepared;

	if (m_comment[position].isEmpty())
		m_flags &= ~(1 << position);
	else
		m_flags |= (1 << position);
}


inline
void
MoveNode::swapComment(Comment& comment, move::Position position)
{
	m_comment[position].swap(comment);
	updateCommentFlags(position);
}


inline
void
MoveNode::setComment(Comment const& comment, move::Position position)
{
	m_comment[position] = comment;
	updateCommentFlags(position);
}


inline
move::Constraint
MoveNode::constraint() const
{
	return m_move.isEmpty() || m_move.isLegal() ? move::DontAllowIllegalMove : move::AllowIllegalMove;
}

} // namespace db

// vi:set ts=3 sw=3:
