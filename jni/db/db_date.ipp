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
// This implementation is adopted from chessx/src/database/partialdate.cpp
// Copyright: (C) 2005 Michal Rudolf <mrudolf@kdewebdev.org>
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

#include "m_assert.h"

namespace db {

inline Date::operator bool() const	{ return m_year > 0; }
inline bool Date::isEmpty() const	{ return m_year == 0; }
inline bool Date::isFull() const		{ return m_year && m_month && m_day; }
inline unsigned Date::year() const	{ return m_year; }
inline unsigned Date::month() const	{ return m_month; }
inline unsigned Date::day() const	{ return m_day; }
inline unsigned Date::hash() const	{ return m_value; }
inline void Date::clear()				{ m_value = 0; }


inline
unsigned
Date::decodeYearFrom10Bits(unsigned year)
{
	return year == Zero10Bits ? 0 : year + MinYear;
}


inline
unsigned
Date::encodeYearTo10Bits(unsigned year)
{
	//M_REQUIRE(year == 0 || (Date::MinYear <= year && year <= Date::MaxYear));
	return year ? (year - MinYear) & 0x3ff : Zero10Bits;
}


inline
bool
operator==(const Date& d1, const Date& d2)
{
	return d1.m_value == d2.m_value;
}


inline
bool
operator!=(const Date& d1, const Date& d2)
{
	return d1.m_value != d2.m_value;
}


inline
bool
operator<(const Date& d1, const Date& d2)
{
	return d1.m_value < d2.m_value;
}


inline
bool
operator>(const Date& d1, const Date& d2)
{
	return d1.m_value > d2.m_value;
}


inline
bool
operator<=(const Date& d1, const Date& d2)
{
	return d1.m_value <= d2.m_value;
}


inline
bool
operator>=(const Date& d1, const Date& d2)
{
	return d1.m_value >= d2.m_value;
}


inline
int
Date::compare(Date const& date) const
{
	return int(m_value) - int(date.m_value);
}

} // namespace db

// vi:set ts=3 sw=3:
