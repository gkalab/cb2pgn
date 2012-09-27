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
// This class is adopted from chessx/src/database/partialdate.h
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

#ifndef _db_date_included
#define _db_date_included

#include "u_crc.h"
#include "u_base.h"

#include "m_string.h"

namespace db {

/** @ingroup Database
   The Date class represents a date, perhaps with missing month and day.
*/

class Date
{
public:

	// Range of year (allows 10 bit encoding).
	static uint16_t const MinYear = 1200;
	static uint16_t const MaxYear = 2222;

	static unsigned const Zero10Bits = MaxYear - MinYear + 1;

	/// Empty constructor. Date is undefined.
	Date();
	/// Partial constructor. Only year is known.
	explicit Date(unsigned y);
	/// Partial constructor. Only year and month are known.
	Date(unsigned y, unsigned m);
	/// Full constructor. All is known.
	Date(unsigned y, unsigned m, unsigned d);
	/// String constructor. Creates date from PGN date format (assumes valid input).
	Date(mstl::string const& s);

	/// Converts date to string. Uses PGN date format (e.g. "1990.01.??").
	mstl::string asString() const;
	/// Converts date to string. Uses short format (e.g. "1990", "1990.01", "1990.01.15").
	mstl::string asShortString() const;

	/// Returns @p true if year is defined.
	operator bool() const;

	/// Returns @p true if year is undefined.
	bool isEmpty() const;
	/// Returns @p true if all date parts are defined.
	bool isFull() const;

	/// Returns year, @p 0 if undefined.
	unsigned year() const;
	/// Returns month, @p 0 if undefined.
	unsigned month() const;
	/// Returns day, @p 0 if undefined.
	unsigned day() const;
	/// Return hash code
	unsigned hash() const;
	/// Returns an integer less than, equal to, or greater than zero if 'this' is found, respectively,
	/// to be less than, to match, or be greater than the argument.
	int compare(Date const& date) const;

	/// Return computed checksum.
	::util::crc::checksum_t computeChecksum(util::crc::checksum_t crc) const;

	/// Sets year, month, and day.
	bool setYMD(unsigned y, unsigned m, unsigned d);
	/// Sets date from string in PGN date format (assumes valid input).
	void fromString(mstl::string const& s);
	/// Sets date from string in PGN date format although date is illegal.
	bool parseFromString(mstl::string const& s);
	/// Sets date from string in PGN date format although date is illegal.
	bool parseFromString(char const* s, unsigned size);
	/// Set date from string in %ccsnt format.
	char const* parseCorrespondenceDate(char const* s);
	/// Reset to empty date,
	void clear();

	static bool isValid(char const* s, unsigned len = 0);
	static bool isValid(mstl::string const& s);
	static bool isValid(unsigned y, unsigned m, unsigned d);
	static bool checkDay(unsigned y, unsigned m, unsigned d);

	static int compare(Date const& lhs, Date const& rhs);

	static unsigned decodeYearFrom10Bits(unsigned year);
	static unsigned encodeYearTo10Bits(unsigned year);

	friend bool operator==(const Date& d1, const Date& d2);
	friend bool operator!=(const Date& d1, const Date& d2);
	friend bool operator>=(const Date& d1, const Date& d2);
	friend bool operator<=(const Date& d1, const Date& d2);
	friend bool operator< (const Date& d1, const Date& d2);
	friend bool operator> (const Date& d1, const Date& d2);

private:

	bool parseYear(char const* s);
	bool parseMonth(char const* s);
	bool parseDay(char const* s);

	union
	{
		struct
		{
			union
			{
				uint16_t m_minor;

				struct
				{
					uint8_t m_day;
					uint8_t m_month;
				};
			};

			uint16_t	m_year;
		};

		uint32_t m_value;
	};
};

bool operator==(const Date& d1, const Date& d2);
bool operator!=(const Date& d1, const Date& d2);
bool operator>=(const Date& d1, const Date& d2);
bool operator<=(const Date& d1, const Date& d2);
bool operator< (const Date& d1, const Date& d2);
bool operator> (const Date& d1, const Date& d2);

} // namespace db

namespace mstl {

template <typename T> struct is_pod;
template <> struct is_pod<db::Date> { enum { value = 1 }; };

} // namespace mstl

#include "db_date.ipp"

#endif // _db_date_included

// vi:set ts=3 sw=3:
