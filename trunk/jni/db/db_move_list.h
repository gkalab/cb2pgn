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

#ifndef _db_move_list_included
#define _db_move_list_included

#include "db_move.h"

namespace db {

class MoveList
{
public:

	// According to
	// http://en.wikipedia.org/wiki/World_records_in_chess
	// the maximum number of moves is possibly 218.
	// The following (calculated) constant is safer and can
	// never be exceeded.
	// TODO: Make a sharper calculation.
	enum { Maximum_Moves = 290 };

	MoveList();

	MoveList& operator=(MoveList const& list);

	Move const& operator[](unsigned n) const;
	Move& operator[](unsigned n);

	Move const& back() const;
	Move back();

	bool isEmpty() const;
	bool isFull() const;
	bool notFull() const;

	unsigned size() const;
	int find(uint16_t move) const;

	Move const* begin() const;
	Move const* end() const;

	Move* begin();
	Move* end();

	void append(Move const& m);
	void push(Move const& m);
	void cut(unsigned size);
	void pop();
	void clear();

	void sort(int scores[]);
	void sort(unsigned startIndex, int scores[]);

	void dump();

private:

	unsigned	m_size;
	Move		m_buffer[Maximum_Moves];
};

} // namespace db

#include "db_move_list.ipp"

#endif // _db_move_list_included

// vi:set ts=3 sw=3:
