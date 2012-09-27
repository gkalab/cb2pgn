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

#include "db_time.h"

using namespace db;


Time::Time(uint32_t utc)
{
	if (utc)
		sys::time::localtime(utc, m_time);
}


mstl::string
Time::asString() const
{
	mstl::string str;

	if (!isEmpty())
	{
		str.format(	"%04u.%02u.%02u %02u:%02u:%02u",
						m_time.year, m_time.month, m_time.day, m_time.hour, m_time.minute, m_time.second);
	}

	return str;
}

// vi:set ts=3 sw=3:
