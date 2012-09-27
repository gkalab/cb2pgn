// ======================================================================
// Author : $Author$
// Version: $Revision$
// Date   : $Date$
// Url    : $URL$
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

#include "u_misc.h"

#include <string.h>

#ifdef __WIN32__

# include <windows.h>
static char Separator = '\\';

#else

# include <sys/time.h>
# include <time.h>

static char Separator = '/';

#endif


using namespace util;


char util::misc::file::pathSeparator() { return Separator; }


bool
util::misc::file::hasSuffix(mstl::string const& path)
{
	char const* p = ::strrchr(path.c_str(), '.');

	if (!p || p == path.c_str())
		return false;

	return !::strchr(p + 1, Separator);
}


mstl::string
util::misc::file::dirname(mstl::string const& path)
{
	char const* p = ::strrchr(path.c_str(), Separator);

	if (!p)
		return mstl::string::empty_string;

	return mstl::string(path.begin(), p + 1);
}


mstl::string
util::misc::file::basename(mstl::string const& path)
{
	char const* p = ::strrchr(path.c_str(), Separator);

	if (!p)
		return path;

	return mstl::string(p + 1, path.end());
}


mstl::string
util::misc::file::rootname(mstl::string const& path)
{
	char const* p = ::strrchr(path.c_str(), '.');

	if (!p || p == path.c_str())
		return path;

	char const* q = ::strchr(p + 1, Separator);

	if (q)
		return path;

	return mstl::string(path.c_str(), p - path.c_str());
}


mstl::string
util::misc::file::suffix(mstl::string const& path)
{
	char const* p = ::strrchr(path.c_str(), '.');

	if (!p || p == path.c_str())
		return mstl::string::empty_string;

	return ::strchr(p + 1, Separator) ? mstl::string::empty_string : mstl::string(p + 1);
}


bool
util::misc::time::getCurrentTime(struct tm& result)
{
#ifdef WI__WIN32__N32

	SYSTEMTIME t;
	::GetLocalTime(&t);

	dst.year = t.wYear;	// between 1601 and 30827
	dst.mon  = t.wMonth;
	dst.mfay = t.wDay;
	dst.hour = t.wHour;
	dst.min  = t.mMinute;
	dst.sec  = t.wMilliseconds*1000;

#else

	::time_t sec = ::time(0);

	struct ::tm t;
	::localtime_r(&sec, &t);

	result.sec  = t.tm_sec;
	result.min  = t.tm_min;
	result.hour = t.tm_hour;
	result.mday = t.tm_mday;
	result.mon  = t.tm_mon;
	result.year = t.tm_year;

#endif

	return true;
}

// vi:set ts=3 sw=3:
