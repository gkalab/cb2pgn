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
// Copyright: (C)2011-2012 Gregor Cramer
// ======================================================================

// ======================================================================
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
// ======================================================================

#ifndef _db_clock_included
#define _db_clock_included

#include "u_crc.h"

#include "m_types.h"

namespace db {

class Clock
{
public:

	Clock();
	Clock(uint8_t hour, uint8_t minute, uint8_t second);

	bool isEmpty() const;

	uint8_t hour() const;
	uint8_t minute() const;
	uint8_t second() const;

	int compare(Clock const& clock) const;
	::util::crc::checksum_t computeChecksum(util::crc::checksum_t crc) const;

	void setHMS(uint8_t hour, uint8_t minute, uint8_t second);

	char const* parse(char const* s);

	void dump() const;

private:

	union
	{
		struct
		{
			uint8_t m_second;
			uint8_t m_minute;
			uint8_t m_hour;
		};

		uint32_t m_value;
	};
};

} // namespace db

namespace mstl {

template <typename T> struct is_pod;
template <> struct is_pod<db::Clock> { enum { value = 1 }; };

} // namespace mstl

#include "db_clock.ipp"

#endif // _db_clock_included

// vi:set ts=3 sw=3:
