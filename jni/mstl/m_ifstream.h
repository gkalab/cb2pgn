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

#ifndef _mstl_ifstream_included
#define _mstl_ifstream_included

#include "m_istream.h"
#include "m_file.h"

namespace mstl {

class ifstream : public istream, virtual protected bits::file
{
public:

	ifstream();
	explicit ifstream(char const* filename, openmode mode = in);
	explicit ifstream(int fd, openmode mode = in);
	explicit ifstream(FILE* fp, openmode mode = in);
	~ifstream() throw();

	bool is_open() const;
	bool is_buffered() const;
	bool is_unbuffered() const;

	int64_t size() const override;
	unsigned bufsize() const;
	char* buffer() const;
	uint64_t mtime();
	mstl::string const& filename() const;

	virtual void open(char const* filename);
	virtual void open(char const* filename, openmode mode);
	void open(int fd, openmode mode = in);
	void open(FILE* fp, openmode mode = in);
	void close();

	void set_unbuffered();
	void set_binary();
	void set_text();
	void set_bufsize(unsigned size);
};

extern ifstream cin;

} // namespace mstl

#include "m_ifstream.ipp"

#endif // _mstl_ifstream_included

// vi:set ts=3 sw=3:
