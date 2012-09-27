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

#include "m_ios.h"
#include "m_stdio.h"
#include "m_assert.h"

using namespace mstl;

/*
ios_base::failure::failure(string const& msg) :exception(msg) {}
ios_base::failure::failure(char const* fmt, va_list args) :exception(fmt, args) {}
*/

ios_base::failure::failure(char const* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	set_message(fmt, args);
	va_end(args);
}


ios_base::~ios_base() throw() {}


void
ios_base::clear(iostate state)
{
	m_state = state;

	//if (m_state & m_except)
	//	M_THROW(failure("ios_base::clear"));
}


void
ios_base::exceptions(iostate except)
{
	m_except = except;
	clear(m_state);
}


int
ios_base::fildes()
{
	return m_fp ? fileno(m_fp) : -1;
}


#ifndef __unix__

# include <stdlib.h>

int
vasprintf(char** strp, const char* fmt, va_list ap)
{
	int bufSize = 4096;
	int size;

	*strp = 0;

	do
	{
		bufSize += bufSize;
		*strp = static_cast<char*>(::realloc(*strp, bufSize));
		size = ::vsnprintf(*strp, bufSize, fmt, ap);
	}
	while (size >= bufSize);

	return size;
}

#endif

// vi:set ts=3 sw=3:
