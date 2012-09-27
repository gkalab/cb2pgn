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

#ifndef _db_edit_key_included
#define _db_edit_key_included

#include "m_string.h"

namespace db {

class Board;
class Game;
class MoveNode;

namespace edit {

class Key
{
public:

	Key();
	explicit Key(unsigned firstPly);
	Key(mstl::string const& key);
	Key(mstl::string const& key, char prefix);
	Key(Key const& key, char prefix);
	explicit Key(char const* key);

#if HAVE_OX_EXPLICITLY_DEFAULTED_AND_DELETED_SPECIAL_MEMBER_FUNCTIONS
	Key(Key const&) = default;
	Key& operator=(Key const&) = default;
#endif

#if HAVE_0X_MOVE_CONSTRCUTOR_AND_ASSIGMENT_OPERATOR
	Key(Key&& key);
	Key& operator=(Key&& key);
#endif

	bool operator==(Key const& key) const;
	bool operator!=(Key const& key) const;
	bool operator< (Key const& key) const;
	bool operator> (Key const& key) const;

	bool isVariationId() const;
	bool isMainlineId() const;

	mstl::string const& id() const;
	char prefix() const;
	unsigned level() const;
	unsigned plyNumber() const;

	int computeDistance(Key const& key) const;

	void addPly(unsigned ply);
	void exchangePly(unsigned ply);
	void removePly();
	void incrementPly();

	void addVariation(unsigned varno);
	void exchangeVariation(unsigned varno);
	void exchangePrefix(char prefix);
	void removeVariation();

	void clear();
	void reset(unsigned firstPly);

	bool setPosition(Game& game) const;
	bool setBoard(MoveNode const* root, Board& board) const;
	Key successorKey(MoveNode const* current) const;
	MoveNode* findPosition(MoveNode* root, unsigned startPly) const;

	static bool isValid(mstl::string const& key);

private:

	mstl::string m_id;
};


bool operator==(mstl::string const& lhs, Key const& rhs);
bool operator!=(mstl::string const& lhs, Key const& rhs);
bool operator==(Key const& lhs, mstl::string const& rhs);
bool operator!=(Key const& lhs, mstl::string const& rhs);

} // namespace edit
} // namespace db

#include "db_edit_key.ipp"

#endif // _db_edit_key_included

// vi:set ts=3 sw=3:
