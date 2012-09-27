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

#ifndef _db_query_included
#define _db_query_included

#include "m_ref_counted_ptr.h"

namespace db {

class Search;
class GameInfo;

class Query
{
public:

	typedef mstl::ref_counted_ptr<Search> SearchP;

	enum Operator { Null, Not, And, Or, Reset, Remove };

	Query(SearchP const& search, Operator op = Null);
#if HAVE_OX_EXPLICITLY_DEFAULTED_AND_DELETED_SPECIAL_MEMBER_FUNCTIONS
	Query(Query const&) = default;
#endif

#if HAVE_0X_MOVE_CONSTRCUTOR_AND_ASSIGMENT_OPERATOR
	Query(Query&& query);
	Query operator=(Query&& query);
#endif

	bool empty() const;

	Operator op() const;

	bool match(GameInfo const& info) const;

private:

	Operator	m_op;
	SearchP	m_search;
};

} // namespace db

#include "db_query.ipp"

#endif // _db_query_included

// vi:set ts=3 sw=3:
