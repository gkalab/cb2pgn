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

#ifndef _db_mark_set_defined
#define _db_mark_set_defined

#include "db_mark.h"

#include "u_crc.h"

#include "m_vector.h"

namespace mstl { class string; }

namespace db {

class MarkSet
{
public:

	MarkSet();

#if HAVE_OX_EXPLICITLY_DEFAULTED_AND_DELETED_SPECIAL_MEMBER_FUNCTIONS
	MarkSet(MarkSet const&) = default;
	MarkSet& operator=(MarkSet const&) = default;
#endif

#if HAVE_0X_MOVE_CONSTRCUTOR_AND_ASSIGMENT_OPERATOR
	MarkSet(MarkSet&& set);
	MarkSet& operator=(MarkSet&& set);
#endif

	bool operator==(MarkSet const& marks) const;
	bool operator!=(MarkSet const& marks) const;

	bool isEmpty() const;
	bool contains(Mark const& mark) const;

	unsigned count() const;
	int find(Mark const& mark)const;
	int match(Mark const& mark)const;
	::util::crc::checksum_t computeChecksum(util::crc::checksum_t crc) const;

	Mark const& operator[](unsigned index) const;
	Mark& operator[](unsigned index);

	void add(MarkSet const& set);
	void add(Mark const& mark);
	void add(char const* s);
	Mark& add();

	void remove(unsigned index);
	void swap(MarkSet& marks);
	void sort();
	void clear();

	bool extractFromComment(mstl::string& comment);
	mstl::string& toString(mstl::string& result) const;
	mstl::string& print(mstl::string& result) const;

private:

	typedef mstl::vector<Mark> Marks;

	char const* parseChessBaseMark(char const* s, mark::Type type);

	Marks m_marks;
};

} // namespace db

#include "db_mark_set.ipp"

#endif // _db_mark_set_defined

// vi:set ts=3 sw=3:
