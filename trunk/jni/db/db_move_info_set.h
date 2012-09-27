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

#ifndef _db_move_info_set_included
#define _db_move_info_set_included

#include "db_move_info.h"

#include "u_crc.h"

#include "m_vector.h"

namespace mstl { class string; }

namespace db {

class EngineList;

class MoveInfoSet
{
public:

	MoveInfoSet();

	bool isEmpty() const;

	unsigned count() const;
	::util::crc::checksum_t computeChecksum(EngineList const& engines, util::crc::checksum_t crc) const;

	MoveInfo const& operator[](unsigned n) const;
	MoveInfo& operator[](unsigned n);

	MoveInfo& add();
	MoveInfo& add(MoveInfo const& info);

	void resize(unsigned n);
	void reserve(unsigned n);
	void swap(MoveInfoSet& row);
	void sort();
	void clear();

	bool extractFromComment(EngineList& engineList, mstl::string& comment);
	void print(	EngineList const& engines,
					mstl::string& result,
					MoveInfo::Format format = MoveInfo::Pgn) const;

private:

	typedef mstl::vector<MoveInfo> Row;

	Row m_row;
};

} // namespace db

#include "db_move_info_set.ipp"

#endif // _db_move_info_row_included

// vi:set ts=3 sw=3:
