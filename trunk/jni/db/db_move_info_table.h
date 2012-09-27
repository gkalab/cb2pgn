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

#ifndef _db_move_info_table_included
#define _db_move_info_table_included

#include "db_move_info_set.h"
#include "db_engine_list.h"

#include "m_list.h"

namespace util { class ByteStream; }

namespace db {

class MoveInfoTable
{
public:

	bool isEmpty() const;
	bool isEmpty(unsigned n) const;

	unsigned size() const;

	MoveInfoSet const& operator[](unsigned n) const;
	MoveInfoSet& operator[](unsigned n);

	MoveInfoSet& back();

	MoveInfo& push(unsigned n);
	MoveInfo& push(unsigned n, MoveInfo const& info);

	void add(unsigned n, MoveInfoSet const& moveInfoRow);
	void set(EngineList const& engines);

	void resize(unsigned n);
	void reserve(unsigned n);
	void clear();

	bool extractFromComment(unsigned n, mstl::string& comment);
	void print(unsigned n, mstl::string& result) const;

	void decode(util::ByteStream& strm);
	void encode(util::ByteStream& strm) const;

private:

	typedef mstl::list<MoveInfoSet> Table;

	Table			m_table;
	EngineList	m_engines;
};

} // namespace db

#include "db_move_info_table.ipp"

#endif // _db_move_info_table_included

// vi:set ts=3 sw=3:
