// ======================================================================
// Author : $Author$
// Version: $Revision$
// Date   : $Date$
// Url    : $URL$
// ======================================================================

// ======================================================================
// Copyright: (C) 2012 Gregor Cramer
// ======================================================================

// ======================================================================
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
// ======================================================================

#ifndef u_zlib_ostream_included
#define u_zlib_ostream_included

#include "m_ostream.h"

#include "m_stdio.h"

extern "C" { struct z_stream_s; }

namespace util {

class ZlibOStream : public mstl::ostream
{
public:

	ZlibOStream(FILE* destination = 0);
	~ZlibOStream() throw();

	bool isOpen() const;

	uint32_t crc() const;
	unsigned size() const;
	unsigned compressedSize() const;

	void open(FILE* destination);
	void flush();
	void close() throw();

private:

	class Cookie;
	friend class Cookie;

	FILE*	m_dst;
	char	m_buf[16384];

	uint32_t m_crc;
	unsigned m_size;
	unsigned m_compressedSize;

	struct z_stream_s* m_zstrm;
};

} // namespace util

#include "u_zlib_ostream.ipp"

#endif //u_zlib_ostream_included

// vi:set ts=3 sw=3:
