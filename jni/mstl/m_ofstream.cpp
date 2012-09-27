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

#include "m_ofstream.h"
#include "m_stdio.h"
#include "m_assert.h"

using namespace mstl;

ofstream mstl::cout(stdout, ios_base::out | ios_base::binary);
ofstream mstl::cerr(stderr, ios_base::out | ios_base::binary);


ofstream::ofstream() {}
ofstream::ofstream(char const* filename, openmode mode) { open(filename, mode); }
ofstream::ofstream(int fd, openmode mode) { open(fd, mode); }
ofstream::ofstream(FILE* fp, openmode mode) { open(fp); }


void
ofstream::open(char const* filename, openmode mode)
{
	//M_REQUIRE(filename);
	//M_REQUIRE(!is_open());
	/*M_REQUIRE(	(mode & ~binary) == (mode & out)
				|| (mode & ~binary) == (mode & (out | app))
				|| (mode & ~binary) == (mode & (out | trunc)));*/

	char modestr[3] = { mode & app ? 'a' : 'w', mode & binary ? 'b' : '\0', 0 };

	setmode(mode & ~in);
	bits::file::open(filename, modestr);
}


void
ofstream::open(int fd, openmode mode)
{
	//M_REQUIRE(fd >= 0);
	//M_REQUIRE(!is_open());
	/*M_REQUIRE(	(mode & ~binary) == (mode & out)
				|| (mode & ~binary) == (mode & (out | app))
				|| (mode & ~binary) == (mode & (out | trunc)));*/

	char modestr[3] = { mode & app ? 'a' : 'w', mode & binary ? 'b' : '\0', 0 };

	setmode(mode & ~in);
	bits::file::open(fd, modestr);
}


void
ofstream::open(char const* filename)
{
	open(filename, out);
}


void
ofstream::open(FILE* fp, openmode mode)
{
	//M_REQUIRE(fp);
	//M_REQUIRE(!is_open());
	/*M_REQUIRE(	(mode & ~binary) == (mode & out)
				|| (mode & ~binary) == (mode & (out | app))
				|| (mode & ~binary) == (mode & (out | trunc)));*/

	setmode(mode & ~in);
	bits::file::open(fp);
}

// vi:set ts=3 sw=3:
