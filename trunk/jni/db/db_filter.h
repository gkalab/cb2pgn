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

#ifndef _db_filter_included
#define _db_filter_included

#include "m_bitset.h"

namespace db {

class Query;
class DatabaseContent;

class Filter
{
public:

	enum { Invalid = -1 };
	enum ResizeMode { AddNewIndices, LeaveEmpty };

	Filter();
	Filter(unsigned size);

#if HAVE_OX_EXPLICITLY_DEFAULTED_AND_DELETED_SPECIAL_MEMBER_FUNCTIONS
	Filter(Filter const&) = default;
	Filter& operator=(Filter const&) = default;
#endif

#if HAVE_0X_MOVE_CONSTRCUTOR_AND_ASSIGMENT_OPERATOR
	Filter(Filter&& filter);
	Filter& operator=(Filter&& filter);
#endif

	/// return true if filter is empty
	bool isEmpty() const;
	/// return true if the game is in the filter
	bool contains(unsigned index) const;
	/// return true if all games are selected
	bool isComplete() const;
	/// return true if filter is compresed
	bool isCompressed() const;

	/// return number of games in the filter
	unsigned count() const;
	/// return the size of the filter
	unsigned size() const;

	/// return next game in the filter or @p Invalid if there is none
	/// use Invalid to retrieve first game in filter
	int next(int current = Invalid) const;
	/// return previous game in the filter or @p Invalid if there is none
	/// use Invalid to retrieve last game in filter
	int prev(int current = Invalid) const;

//	/// return @p number in filter of game with database index @p index
//	int toNumber(unsigned index) const;
	/// return database index of @p number from filter
	unsigned toIndex(unsigned number) const;

	/// set all games in filter
	void set();
	/// unset all games in filter
	void reset();
	/// negate filter (complement set)
	void negate();
	/// do a search
	void search(Query const& query, DatabaseContent const& content);
	/// add given game number
	void add(unsigned index);

	/// resize the filter to the specified size (keeps the current filter content)
	void resize(unsigned newSize, ResizeMode mode);
	/// swap internal data
	void swap(Filter& filter);

	/// compress filter
	void compress();
	/// uncompress filter
	void uncompress();

	/// dump filter
	void dump() const;

private:

	bool checkClassInvariance() const;

	mstl::bitset	m_set;
	unsigned			m_count;
};

} // namespace db

#include "db_filter.ipp"

#endif // _db_filter_included

// vi:set ts=3 sw=3:
