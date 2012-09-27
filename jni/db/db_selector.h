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

#ifndef _db_selector_included
#define _db_selector_included

#include "db_common.h"

#include "m_vector.h"

namespace db {

class Database;
class GameInfo;
class Filter;

class Selector
{
public:

#if HAVE_OX_EXPLICITLY_DEFAULTED_AND_DELETED_SPECIAL_MEMBER_FUNCTIONS
	Selector() = default;
	Selector(Selector const&) = default;
	Selector& operator=(Selector const&) = default;
#endif

#if HAVE_0X_MOVE_CONSTRCUTOR_AND_ASSIGMENT_OPERATOR
	Selector(Selector&& sel);
	Selector& operator=(Selector&& sel);
#endif

	unsigned size() const;

	unsigned map(unsigned index) const;
	unsigned lookup(unsigned index) const;
	unsigned find(unsigned number) const;

	int findPlayer(Database const& db, mstl::string const& name) const;
	int findEvent(Database const& db, mstl::string const& name) const;
	int findSite(Database const& db, mstl::string const& name) const;
	int findAnnotator(Database const& db, mstl::string const& name) const;
	int searchPlayer(Database const& db, mstl::string const& name) const;
	int searchEvent(Database const& db, mstl::string const& name) const;
	int searchSite(Database const& db, mstl::string const& name) const;
	int searchAnnotator(Database const& db, mstl::string const& name) const;

	void sort(	Database const& db,
					attribute::game::ID attr,
					order::ID order = order::Ascending,
					rating::Type ratingType = rating::Any);
	void sort(	Database const& db,
					attribute::player::ID attr,
					order::ID order = order::Ascending,
					rating::Type ratingType = rating::Any);
	void sort(	Database const& db,
					attribute::event::ID attr,
					order::ID order = order::Ascending);
	void sort(	Database const& db,
					attribute::site::ID attr,
					order::ID order = order::Ascending);
	void sort(	Database const& db,
					attribute::annotator::ID attr,
					order::ID order = order::Ascending);

	void reverse(Database const& db);
	void swap(Selector& selector);
	void update(Filter const& filter);
	void update(unsigned newSize);
	void update();

private:

	typedef mstl::vector<unsigned> Map;
	typedef int(*Compar)(void const*, void const*);

	void finish(Database const& db, unsigned numEntries, order::ID order, Compar compFunc);

	Map m_map;
	Map m_list;
};

} // namespace db

#include "db_selector.ipp"

#endif // _db_selector_included

// vi:set ts=3 sw=3:
