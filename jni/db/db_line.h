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

#ifndef _db_line_included
#define _db_line_included

#include "db_common.h"

#include "u_base.h"

namespace mstl { class string; }

namespace db {

struct Line
{
	Line();
	Line(uint16_t const* moves, unsigned length = 0);

#if HAVE_OX_EXPLICITLY_DEFAULTED_AND_DELETED_SPECIAL_MEMBER_FUNCTIONS
	Line(Line const&) = default;
	Line& operator=(Line const&) = default;
#endif

#if HAVE_0X_MOVE_CONSTRCUTOR_AND_ASSIGMENT_OPERATOR
	Line(Line&& line);
	Line& operator=(Line&& line);
#endif

	bool operator==(Line const& line) const;
	bool operator!=(Line const& line) const;
	bool operator<=(Line const& line) const;
	bool partialMatch(Line const& line) const;

	uint16_t operator[](unsigned n) const;

	mstl::string& print(mstl::string& result, encoding::CharSet charSet = encoding::Latin1) const;
	void dump() const;

	void copy(Line const& line);
	void copy(Line const& line, unsigned maxLength);
	Line& transpose(Line& dst) const;
	Line& transpose();

	uint16_t const*	moves;	// null terminated
	unsigned 			length;	// w/o null terminator
};

} // namespace db

namespace mstl {

template <typename T> struct is_pod;
template <> struct is_pod<db::Line> { enum { value = 1 }; };

} // namespace mstl

#include "db_line.ipp"

#endif // _db_line_included

// vi:set ts=3 sw=3:
