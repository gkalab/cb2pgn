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

#include "m_fstream.h"
#include "m_assert.h"

#include <string.h>

using namespace mstl;


fstream::fstream() {}
fstream::fstream(char const* filename, openmode mode) { open(filename, mode); }


void
fstream::open(char const* filename, openmode mode)
{
	//M_REQUIRE(filename);
	//M_REQUIRE(!is_open());
	/*M_REQUIRE(	(mode & ~binary) == (mode & out)
				|| (mode & ~binary) == (mode & (out | app))
				|| (mode & ~binary) == (mode & (out | trunc))
				|| (mode & ~binary) == (mode & in)
				|| (mode & ~binary) == (mode & (in | out))
				|| (mode & ~binary) == (mode & (in | out | app))
				|| (mode & ~binary) == (mode & (in | out | trunc)));
*/
	char modestr[4] = { 'w', 0, 0, 0 };

	switch (mode & ~binary)
	{
		case out | app:			::strcpy(modestr, "a"); break;
		case in:						::strcpy(modestr, "r"); break;
		case in | out:				::strcpy(modestr, "r+"); break;
		case in | out | app:		::strcpy(modestr, "a+"); break;
		case in | out | trunc:	::strcpy(modestr, "w+"); break;
	}

	if (mode & binary)
	{
		modestr[2] = modestr[1];
		modestr[1] = 'b';
	}

	setmode(mode);
	bits::file::open(filename, modestr);
}


void
fstream::open(char const* filename)
{
	open(filename, in | out);
}

// vi:set ts=3 sw=3:
