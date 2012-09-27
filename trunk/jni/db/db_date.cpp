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

#include "db_date.h"

#include "m_utility.h"
#include "m_stdio.h"

#include <string.h>

using namespace db;


inline
bool
isLeapYear(unsigned y)
{
	return !mstl::mod4(y) && (y % 100 || !(y % 400));
}


bool
Date::checkDay(unsigned y, unsigned m, unsigned d)
{
	static unsigned const MonthDays[13] = { 0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

	//M_ASSERT(m <= 12);

	return d <= MonthDays[m] || (d == 29 && m == 2 && isLeapYear(y));
}


Date::Date() : m_value(0) {}


Date::Date(mstl::string const& s)
	:m_value(0)
{
	if (!s.empty())
		fromString(s);
}


Date::Date(unsigned y)
	:m_day(0)
	,m_month(0)
	,m_year(y)
{
	//M_REQUIRE(y == 0 || mstl::is_between(uint16_t(y), MinYear, MaxYear));
}


Date::Date(unsigned y, unsigned m)
	:m_day(0)
	,m_month(m)
	,m_year(y)
{
	//M_REQUIRE(y == 0 || mstl::is_between(uint16_t(y), MinYear, MaxYear));
	//M_REQUIRE(y ? m <= 12 : m == 0);
}


Date::Date(unsigned y, unsigned m, unsigned d)
	:m_day(d)
	,m_month(m)
	,m_year(y)
{
	//M_REQUIRE(y == 0 || mstl::is_between(uint16_t(y), MinYear, MaxYear));
	//M_REQUIRE(y ? m <= 12 : m == 0);
	//M_REQUIRE(m ? checkDay(y, m, d) : d == 0);
}


bool
Date::setYMD(unsigned y, unsigned m, unsigned d)
{
	if (!mstl::is_between(uint16_t(y), MinYear, MaxYear))
	{
		m_value = 0;
		return false;
	}

	m_year = y;

	if (0 < m && m <= 12)
	{
		m_month = m;

		if (d != 0 && !checkDay(y, m, d))
		{
			m_day = 0;
			return false;
		}

		m_day = d;
	}
	else
	{
		m_minor = 0;
		return false;
	}

	return true;
}


::util::crc::checksum_t
Date::computeChecksum(util::crc::checksum_t crc) const
{
	crc = ::util::crc::compute(crc, m_year);
	crc = ::util::crc::compute(crc, m_month);
	crc = ::util::crc::compute(crc, m_day);

	return crc;
}


bool
Date::parseYear(char const* s)
{
	m_year = (s[0] - '0')*1000 + (s[1] - '0')*100 + (s[2] - '0')*10 + (s[3] - '0');

	if (mstl::is_between(m_year, MinYear, MaxYear))
		return true;

	m_value = 0;
	return false;
}


bool
Date::parseMonth(char const* s)
{
	//M_ASSERT(m_year);

	m_month = (s[0] - '0')*10 + (s[1] - '0');

	if (0 < m_month && m_month <= 12)
		return true;

	m_minor = 0;
	return false;
}


bool
Date::parseDay(char const* s)
{
	//M_ASSERT(m_year && m_month);

	m_day = (s[0] - '0')*10 + (s[1] - '0');

	if (m_day == 0)
		return false;

	if (checkDay(m_year, m_month, m_day))
		return true;

	m_day = 0;
	return false;
}


bool
Date::parseFromString(char const* s, unsigned size)
{
	switch (size)
	{
		case 4:
			m_minor = 0;

			if (s[0] == '?')
				m_year = 0;
			else if (!parseYear(s))
				return false;
			break;

		case 7:
			m_day = 0;

			if (s[4] == '.' || s[4] == '/')
			{
				if (s[0] == '?')
					m_year = m_month = 0;
				else if (!parseYear(s))
					return false;
				else if (s[5] == '?')
					m_month = 0;
				else if (!parseMonth(s + 5))
					return false;
			}
			else if (s[2] == '.' || s[2] == '/')
			{
				if (s[3] == '?')
					m_year = m_month = 0;
				else if (!parseYear(s + 3))
					return false;
				else if (*s == '?')
					m_month = 0;
				else if (!parseMonth(s))
					return false;
			}
			else
			{
				m_year = m_month = 0;
				return false;
			}
			break;

		case 10:
			if (s[4] == '.' || s[4] == '/')
			{
				if (s[0] == '?')
					m_year = m_month = m_day = 0;
				else if (!parseYear(s))
					return false;
				else if (s[5] == '?')
					m_minor = 0;
				else if (!parseMonth(s + 5))
					return false;
				else if (s[8] == '?')
					m_day = 0;
				else if (!parseDay(s + 8))
					return false;
			}
			else if (s[2] == '.' || s[2] == '/')
			{
				if (s[6] == '?')
					m_value = 0;
				else if (!parseYear(s + 6))
					return false;
				else if (s[3] == '?')
					m_minor = 0;
				else if (!parseMonth(s + 3))
					return false;
				else if (*s == '?')
					m_day = 0;
				else if (!parseDay(s))
					return false;
			}
			else
			{
				m_value = 0;
				return false;
			}
			break;

		default:
			m_value = 0;
			return false;
	}

	return true;
}


bool
Date::isValid(mstl::string const& s)
{
	return Date().parseFromString(s);
}


bool
Date::isValid(char const* s, unsigned len)
{
	return Date().parseFromString(s, len ? len : ::strlen(s));
}


bool
Date::isValid(unsigned y, unsigned m, unsigned d)
{
	if (y == 0)
		return m == 0 && d == 0;

	if (y < MinYear || MaxYear < y)
		return false;

	if (m == 0)
		return d == 0;

	if (m > 12)
		return false;

	if (d == 0)
		return true;

	return checkDay(y, m, d);
}


void
Date::fromString(mstl::string const& s)
{
	//M_REQUIRE(isValid(s));

	if (s[0] == '?')
	{
		m_value = 0;
	}
	else
	{
		m_year = (s[0] - '0')*1000 + (s[1] - '0')*100 + (s[2] - '0')*10 + (s[3] - '0');

		if (s[5] == '?')
		{
			m_minor = 0;
		}
		else
		{
			m_month = (s[5] - '0')*10 + (s[6] - '0');
			m_day = s[8] == '?' ? 0 : (s[8] - '0')*10 + (s[9] - '0');
		}
	}
}


bool
Date::parseFromString(mstl::string const& s)
{
	return parseFromString(s, s.size());
}


mstl::string
Date::asString() const
{
	if (!m_year)
		return "????.??.??";

	char buf[16];

	if (m_minor == 0)
		::snprintf(buf, sizeof(buf), "%04u.??.??", m_year);
	else if (m_day == 0)
		::snprintf(buf, sizeof(buf), "%04u.%02u.??", m_year, m_month);
	else
		::snprintf(buf, sizeof(buf), "%04u.%02u.%02u", m_year, m_month, m_day);

	return buf;
}


mstl::string
Date::asShortString() const
{
	if (!m_year)
		return "";

	char buf[16];

	if (m_minor == 0)
		::snprintf(buf, sizeof(buf), "%04u", m_year);
	else if (m_day == 0)
		::snprintf(buf, sizeof(buf), "%04u.%02u", m_year, m_month);
	else
		::snprintf(buf, sizeof(buf), "%04u.%02u.%02u", m_year, m_month, m_day);

	return buf;
}


int
Date::compare(Date const& lhs, Date const& rhs)
{
	int res;

	if ((res = int(lhs.year()) - int(rhs.year())))
		return res;

	if ((res = int(lhs.month()) - int(rhs.month())))
		return res;

	return int(lhs.day()) - int(rhs.day());
}

// vi:set ts=3 sw=3:
