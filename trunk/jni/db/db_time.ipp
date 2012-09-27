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
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
// ======================================================================

namespace db {

inline Time::operator bool() const		{ return m_time.year > 0; }
inline bool Time::isEmpty() const		{ return m_time.year == 0; }
inline unsigned Time::year() const		{ return m_time.year; }
inline unsigned Time::month() const		{ return m_time.month; }
inline unsigned Time::day() const		{ return m_time.day; }
inline unsigned Time::hour() const		{ return m_time.hour; }
inline unsigned Time::minute() const	{ return m_time.minute; }
inline unsigned Time::second() const	{ return m_time.second; }


inline
void
Time::clear()
{
	m_time.year = m_time.month = m_time.day = m_time.hour = m_time.minute = m_time.second = 0;
}


inline
bool
operator==(const Time& lhs, const Time& rhs)
{
	return	lhs.m_time.year	== rhs.m_time.year
			&& lhs.m_time.month	== rhs.m_time.month
			&& lhs.m_time.day		== rhs.m_time.day
			&& lhs.m_time.hour	== rhs.m_time.hour
			&& lhs.m_time.minute	== rhs.m_time.minute
			&& lhs.m_time.second	== rhs.m_time.second;
}


inline
bool
operator!=(const Time& lhs, const Time& rhs)
{
	return	lhs.m_time.year	!= rhs.m_time.year
			|| lhs.m_time.month	!= rhs.m_time.month
			|| lhs.m_time.day		!= rhs.m_time.day
			|| lhs.m_time.hour	!= rhs.m_time.hour
			|| lhs.m_time.minute	!= rhs.m_time.minute
			|| lhs.m_time.second	!= rhs.m_time.second;
}

} // namespace db

// vi:set ts=3 sw=3:
