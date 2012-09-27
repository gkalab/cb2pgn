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

#include "db_search.h"

#include "m_utility.h"
#include "m_assert.h"

namespace db {

inline Query::Operator Query::op() const { return m_op; }
inline bool Query::empty() const { return m_search == 0; }


inline
bool
Query::match(GameInfo const& info) const
{
	//M_REQUIRE(!empty());
	return m_search->match(info);
}


#if HAVE_0X_MOVE_CONSTRCUTOR_AND_ASSIGMENT_OPERATOR

inline
Query::Query(Query&& query)
	:m_op(query.m_op)
	,m_search(mstl::move(query.m_search))
{
}


inline
Query
Query::operator=(Query&& query)
{
	m_op = query.m_op;
	m_search = mstl::move(query.m_search);

	return *this;
}

#endif

} // namespace db

// vi:set ts=3 sw=3:
