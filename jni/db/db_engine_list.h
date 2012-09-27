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

#include "m_string.h"
#include "m_map.h"

#ifndef _db_engline_list_included
#define _db_engline_list_included

namespace db {

class EngineList
{
public:

	bool isEmpty() const;

	unsigned count() const;

	mstl::string const& operator[](unsigned n) const;

	mstl::string const& engine(unsigned n) const;

	unsigned addEngine(mstl::string const& engine);

	void swap(EngineList& engines);
	void reserve(unsigned size);
	void clear();

private:

	typedef mstl::map<mstl::string,unsigned> Map;

	Map m_map;
};

} // namespace db

#include "db_engine_list.ipp"

#endif // _db_engline_list_included

// vi:set ts=3 sw=3:
