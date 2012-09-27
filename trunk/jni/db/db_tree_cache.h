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
// Copyright: (C) 2010-2012 Gregor Cramer
// ======================================================================

// ======================================================================
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
// ======================================================================

#ifndef _db_tree_cache_included
#define _db_tree_cache_included

#include "db_board.h"

namespace db {

class Tree;
class Board;

class TreeCache
{
public:

	typedef board::ExactPosition Position;

	enum { CacheSize = 256 };

	TreeCache();

	bool isCached(Board const& position, tree::Mode mode, rating::Type ratingType) const;
	bool isCached(	uint64_t hash,
						Position const& position,
						tree::Mode mode,
						rating::Type ratingType) const;

	static unsigned size();
	unsigned used() const;

	Tree* lookup(Board const& position, tree::Mode mode, rating::Type ratingType) const;
	Tree* lookup(uint64_t hash, Position const& position, tree::Mode mode, rating::Type ratingType) const;

	void add(Tree* tree);
	void clear();
	void setIncomplete();
	void setIncomplete(unsigned index);

private:

	Tree*		m_cache[CacheSize];
	unsigned	m_inUse;
	unsigned	m_mostRecentIndex;

	mutable unsigned m_lastIndex;
};

} // namespace db

#include "db_tree_cache.ipp"

#endif // _db_tree_cache_included

// vi:set ts=3 sw=3:
