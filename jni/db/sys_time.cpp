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

#include "sys_time.h"

#include <time.h>

#ifdef __WIN32__

# include <windows.h>
# include <sys/timeb.h>

uint32_t
sys::time::time()
{
	FILETIME ft;
	GetSystemTimeAsFileTime(&ft);
	uint64_t time = (uint64_t(ft.dwHighDateTime) << 32) | uint64_t(ft.dwLowDateTime);

	// 116444736000000000 = 10000000 * 60 * 60 * 24 * 365 * 369 + 89 leap days
	return (time - UINT64_C(116444736000000000))/10000000;
}


uint64_t
sys::time::timestamp()
{
	struct ::timeb tb;
	return (uint64_t(tb.time)*1000 + tb.millitm)*1000;
}

#else

# include <time.h>
# include <sys/time.h>

uint32_t sys::time::time() { return ::time(0); }


uint64_t
sys::time::timestamp()
{
	struct ::timeval tv;
	::gettimeofday(&tv, 0);
	return uint64_t(tv.tv_sec)*(1000*1000) + tv.tv_usec;
}

#endif


sys::time::Time::Time() : year(0), month(0), day(0), hour(0), minute(0), second(0) {}


void
sys::time::localtime(uint32_t time, Time& tm)
{
	time_t ctime = time;
	struct tm const* t = ::localtime(&ctime);

	tm.year		= t->tm_year + 1900;
	tm.month		= t->tm_mon + 1;
	tm.day		= t->tm_mday;
	tm.hour		= t->tm_hour;
	tm.minute	= t->tm_min;
	tm.second	= t->tm_sec;
}

// vi:set ts=3 sw=3:
