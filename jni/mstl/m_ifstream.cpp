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

#include "m_ifstream.h"
#include "m_stdio.h"
#include "m_assert.h"

using namespace mstl;


ifstream mstl::cin(stdin, ios_base::in | ios_base::binary);


ifstream::ifstream() {}
ifstream::ifstream(char const* filename, openmode mode) { open(filename, mode); }
ifstream::ifstream(int fd, openmode mode) { open(fd, mode); }
ifstream::ifstream(FILE* fp, openmode mode) { open(fp, mode); }
ifstream::~ifstream() throw() { close(); }


void
ifstream::open(char const* filename, openmode mode)
{
	//M_REQUIRE(filename);
	//M_REQUIRE(!is_open());
	//M_REQUIRE((mode & ~binary) == (mode & in));

	setmode(mode & ~out);
	bits::file::open(filename, mode & binary ? "rb" : "r");
}


void
ifstream::open(int fd, openmode mode)
{
	//M_REQUIRE(fd >= 0);
	//M_REQUIRE(!is_open());
	//M_REQUIRE((mode & ~binary) == (mode & in));

	setmode(mode & ~out);
	bits::file::open(fd, mode & binary ? "rb" : "r");
}


void
ifstream::open(char const* filename)
{
	open(filename, in);
}


void
ifstream::open(FILE* fp, openmode mode)
{
	//M_REQUIRE(fp);
	//M_REQUIRE(!is_open());
	//M_REQUIRE((mode & ~binary) == (mode & in));

	setmode(mode & ~out);
	bits::file::open(fp);
}

// vi:set ts=3 sw=3:
