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

#ifndef _mstl_fstream_included
#define _mstl_fstream_included

#include "m_ifstream.h"
#include "m_ofstream.h"

namespace mstl {

class fstream : public ifstream, public ofstream
{
public:

	fstream();
	fstream(char const* filename, openmode mode = in | out);

	using ifstream::is_open;
	using ifstream::is_buffered;
	using ifstream::is_unbuffered;

	using ifstream::size;
	using ifstream::bufsize;
	using ifstream::buffer;
	using ifstream::mtime;
	using ifstream::filename;

	void open(char const* filename) override;
	void open(char const* filename, openmode mode) override;
	using ifstream::close;

	using ifstream::set_unbuffered;
	using ifstream::set_binary;
	using ifstream::set_text;
	using ifstream::set_bufsize;
};

} // namespace mstl

#endif // _mstl_fstream_included

// vi:set ts=3 sw=3:
