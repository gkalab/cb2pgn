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

#ifndef _mstl_ofstream_included
#define _mstl_ofstream_included

#include "m_ostream.h"
#include "m_file.h"

namespace mstl {

class ofstream : public ostream, virtual protected bits::file
{
public:

	ofstream();
	explicit ofstream(char const* filename, openmode mode = out);
	explicit ofstream(int fd, openmode mode = out);
	explicit ofstream(FILE* fp, openmode mode = out);

	bool is_open() const;
	bool is_buffered() const;
	bool is_unbuffered() const;

	unsigned bufsize() const;
	char* buffer() const;
	uint64_t mtime();
	mstl::string const& filename() const;

	virtual void open(char const* filename);
	virtual void open(char const* filename, openmode mode);
	void open(int fd, openmode mode = out);
	void open(FILE* fp, openmode mode = out);
	void close();

	void set_unbuffered();
	void set_binary();
	void set_text();
	void set_bufsize(unsigned size);
};

extern ofstream cout, cerr;

} // namespace mstl

#include "m_ofstream.ipp"

#endif // _mstl_ofstream_included

// vi:set ts=3 sw=3:
