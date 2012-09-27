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

#ifndef _db_move_node_included
#define _db_move_node_included

#include "db_comment.h"
#include "db_move.h"
#include "db_common.h"

#include "u_crc.h"

#include "m_set.h"
#include "m_vector.h"
#include "m_string.h"

// +---+----+---+   +---+----+---+   +---+----+---+   +---+----+---+   +---+---+---+
// |   |    |   |<--|-+ |e2e4|   |<--|-+ |e7e5|   |<--|-+ |d2d4|   |<--|-+ |   |   |
// +---+----+---+   +---+----+---+   +---+----+---+   +---+----+---+   +---+---+---+
// |   |    | +-|-->|   |    | +-|-->|   | V  | +-|-->|   |    | +-|-->|   |   |   |
// +---+----+---+   +---+----+---+   +---+----+---+   +---+----+---+   +---+---+---+
//                                         |    ^
//                        +----------------+    |
//                        |                     |
//                        |    +----------------+
//                        V    |
//                  +---+----+---+   +---+----+---+   +---+----+---+   +---+---+---+
//                  |   |    | + |<--|-+ |e7e6|   |<--|-+ |d2d4|   |<--|-+ |   |   |
//                  +---+----+---+   +---+----+---+   +---+----+---+   +---+---+---+
//                  |   | 0  | +-|-->|   |    | +-|-->|   |    | +-|-->|   |   |   |
//                  +---+----+---+   +---+----+---+   +---+----+---+   +---+---+---+
//                        |    |
//                        |    |
//                        V    |
//                  +- -+----+---+   +---+----+---+   +---+----+---+   +---+---+---+
//                  |   |    | + |<--|-+ |c7c5|   |<--|-+ |d2d4| + |<--|-+ |   |   |
//                  +---+----+---+   +---+----+---+   +---+----+---+   +---+---+---+
//                  |   | 1  | +-|-->|   |    | +-|-->|   | V  | +-|-->|   |   |   |
//                  +---+----+---+   +---+----+---+   +---+----+---+   +---+---+---+
//                                                          |    ^
//                                         +----------------+    |
//                                         |                     |
//                                         |    +----------------+
//                                         V    |
//                                   +---+----+---+   +---+----+---+   +---+---+---+
//                                   |   |    | + |<--|-+ |f2f4|   |<--|-+ |   |   |
//                                   +---+----+---+   +---+----+---+   +---+---+---+
//                                   |   | 0  | +-|-->|   |    | +-|-->|   |   |   |
//                                   +---+----+---+   +---+----+---+   +---+---+---+

namespace db {

class Board;
class Annotation;
class MarkSet;
class Mark;
class MoveInfo;
class MoveInfoSet;
class EngineList;

class MoveNode
{
public:

	typedef mstl::vector<MoveNode*> Nodes;
	typedef Comment::LanguageSet LanguageSet;

	MoveNode(Move const& move);
	MoveNode(Board const& board, Move const& move);
	explicit MoveNode(MoveNode* node);
	explicit MoveNode(Annotation* set = 0);
	~MoveNode();

	bool atLineStart() const;
	bool atLineEnd() const;
	bool isBeforeLineEnd() const;
	bool isOneBeforeLineEnd() const;
	bool isAfterLineStart() const;

	bool hasComment(move::Position position) const;
	bool hasAnyComment() const;
	bool hasVariation() const;
	bool hasAnnotation() const;
	bool hasMark() const;
	bool hasNote() const;
	bool hasSupplement() const;
	bool hasMoveInfo() const;
	bool containsIllegalCastlings() const;
	bool containsIllegalMoves() const;
	bool containsEnglishLang() const;
	bool containsOtherLang() const;
	bool contains(MoveNode const* node) const;
	bool isFolded() const;
	bool isEmptyLine() const;

	unsigned variationCount() const;
	unsigned unfoldedVariationCount() const;
	unsigned variationNumber(MoveNode const* node) const;
	unsigned countHalfMoves() const;
	unsigned countNodes() const;
	unsigned countAnnotations() const;
	unsigned countMoveInfo() const;
	unsigned countMarks() const;
	unsigned countComments() const;
	unsigned countComments(mstl::string const& lang) const;
	unsigned countVariations() const;
	unsigned count() const;

	MoveNode* getLineStart();
	MoveNode* getLineEnd();
	MoveNode* getOneBeforeLineEnd();

	Move& move();
	Move const& move() const;
	MoveNode* next() const;
	MoveNode* prev() const;
	MoveNode* variation(unsigned i) const;
	Nodes const& variations() const;
	Annotation const& annotation() const;
	MarkSet const& marks() const;
	MoveInfoSet const& moveInfo() const;
	Comment const& comment(move::Position position) const;
	move::Constraint constraint() const;
	Byte commentFlag() const;

	void setComment(move::Position position);
	void unsetComment(move::Position position);
	void setCommentFlag(Byte flag);
	void setMark();

	void setFolded(bool flag);
	void fold(bool flag);
	void setMove(Board const& board, Move const& move);
	void setNext(MoveNode* next);
	void addVariation(MoveNode* variation);
	void addAnnotation(nag::ID nag);
	void addMark(Mark const& mark);
	void addMoveInfo(MoveInfo const& moveInfo);
	void swapMoveInfo(MoveInfoSet& moveInfo);
	void replaceAnnotation(Annotation const& annotation);
	void swapMarks(MarkSet& marks);
	void replaceMoveInfo(MoveInfoSet const& moveInfo);
	void replaceMarks(MarkSet const& marks);
	void swapComment(Comment& comment, move::Position position);
	void setComment(Comment const& comment, move::Position position);
	void setInfoFlag(bool flag = true);
	void swapVariations(unsigned varNo1, unsigned varNo2);
	void prepareForPrint(Board const& board);
	void transpose();
	void finish(Board const& board);
	void unfold();

	void deleteNext();
	void deleteVariation(unsigned varNo);
	void clearAnnotation();
	void clearMarks();
	void swapData(MoveNode* node);

	void stripAnnotations();
	void stripMoveInfo();
	void stripMarks();
	void stripComments();
	void stripComments(mstl::string const& lang);
	void stripVariations();
	void copyComments(mstl::string const& fromLang, mstl::string const& toLang, bool stripOriginal);

	util::crc::checksum_t computeChecksum(EngineList const& engines, util::crc::checksum_t crc = 0) const;
	void collectLanguages(LanguageSet& langSet) const;

	MoveNode* removeNext();
	MoveNode* removeVariation(unsigned varNo);
	MoveNode* replaceVariation(unsigned varNo, MoveNode* node);

	MoveNode* clone() const;

private:

	enum
	{
		HasPreComment	= 1 << move::Ante,
		HasComment		= 1 << move::Post,
		HasMark			= 1 << 2,
		HasAnnotation	= 1 << 3,
		HasVariation	= 1 << 4,
		HasMoveInfo		= 1 << 5,
		IsPrepared		= 1 << 6,
		HasNote			= HasComment | HasPreComment | HasMark | HasAnnotation | HasMoveInfo,
		HasSupplement	= HasNote | HasVariation | IsPrepared,
		IsFolded			= 1 << 7,
	};

	MoveNode(MoveNode const&);
	MoveNode& operator=(MoveNode const&);

	MoveNode* clone(MoveNode* prev) const;

	void setupAnnotation(Annotation const& annotation);
	void updateCommentFlags(move::Position position);

	bool checkHasMark() const;
	bool checkHasAnnotation() const;

	unsigned			m_flags;
	MoveNode*		m_next;
	MoveNode*		m_prev;
	Nodes				m_variations;
	Annotation*		m_annotation;
	MarkSet*			m_marks;
	MoveInfoSet*	m_moveInfo;
	Move				m_move;
	Comment			m_comment[2];
	Byte				m_commentFlag;
};

} // namespace db

#include "db_move_node.ipp"

#endif // _db_move_node_included

// vi:set ts=3 sw=3:
