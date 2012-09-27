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

#include "db_engine_list.h"

using namespace db;


mstl::string const&
EngineList::engine(unsigned n) const
{
	for (unsigned i = 0; i < m_map.size(); ++i)
	{
		if (m_map.container()[i].second == n)
			return m_map.container()[i].first;
	}

	return mstl::string::empty_string;
}


unsigned
EngineList::addEngine(mstl::string const& engine)
{
	if (engine.empty())
		return 0;

	return m_map.insert(Map::value_type(engine, m_map.size() + 1)).first->second;
}

// vi:set ts=3 sw=3:
