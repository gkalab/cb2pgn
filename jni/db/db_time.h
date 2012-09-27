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
// Copyright: (C) 2010-2012 Gregor Cramer
// ======================================================================

// ======================================================================
//   This program is free software; you can redistribute it and/or modify
//   it under the terms of the GNU General Public License as published by
//   the Free Software Foundation; either version 2 of the License, or
//   (at your option) any later version.
// ======================================================================

#ifndef _db_time_included
#define _db_time_included

#include "u_base.h"

#include "sys_time.h"

#include "m_string.h"

namespace db {

class Time
{
public:

	Time();
	Time(uint32_t utc);

	operator bool() const;

	bool isEmpty() const;

	unsigned year() const;
	unsigned month() const;
	unsigned day() const;
	unsigned hour() const;
	unsigned minute() const;
	unsigned second() const;

	mstl::string asString() const;

	void clear();

	friend bool operator==(const Time& d1, const Time& d2);
	friend bool operator!=(const Time& d1, const Time& d2);

private:

	sys::time::Time m_time;
};

bool operator==(const Time& d1, const Time& d2);
bool operator!=(const Time& d1, const Time& d2);

} // namespace db

namespace mstl {

template <typename T> struct is_pod;
template <> struct is_pod<db::Time> { enum { value = is_pod<sys::time::Time>::value }; };

} // namespace mstl

#include "db_time.ipp"

#endif // _db_time_included

// vi:set ts=3 sw=3:
