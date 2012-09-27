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

extern "C" {
#include "extrastdio.h"
}
#include "u_zlib_ostream.h"

#include "m_assert.h"

#include <zlib.h>

using namespace util;


struct ZlibOStream::Cookie
{
	static ssize_t
	read(void* cookie, char* buf, size_t len)
	{
		//M_RAISE("should not be called");
	}

	static int
	seek(void* cookie, off_t* pos, int whence)
	{
		//if (*pos != 0)
		//	M_RAISE("should not be called");

		return 0;
	}

	static int
	close(void* cookie)
	{
		ZlibOStream* that = static_cast<ZlibOStream*>(cookie);

		if (that->m_dst == 0)
			return -1;

		z_stream*	zstrm = that->m_zstrm;
		size_t		have;

		zstrm->total_out = 0;
		zstrm->avail_in = 0;

		do
		{
			zstrm->avail_out = sizeof(that->m_buf);
			zstrm->next_out = reinterpret_cast<Bytef*>(that->m_buf);

			if (::deflate(zstrm, Z_FINISH) == Z_STREAM_ERROR)
			{}
			//	M_RAISE("zlib: deflate() failed");

			have = sizeof(that->m_buf) - zstrm->avail_out;

			if (have > 0)
			{
				that->m_crc = crc32(that->m_crc, reinterpret_cast<Bytef*>(that->m_buf), have);

				int rc = ::fwrite(that->m_buf, 1, have, that->m_dst);
				if (rc != int(have))
				{
					::deflateEnd(that->m_zstrm);
					that->m_dst = 0;
					return -1;
				}
			}
		}
		while (have > 0);

		that->m_compressedSize += zstrm->total_out;

		::deflateEnd(that->m_zstrm);
		that->m_dst = 0;

		return 0;
	}

	static ssize_t
	write(void* cookie, char const* buf, size_t len)
	{
		ZlibOStream*	that	= static_cast<ZlibOStream*>(cookie);
		z_stream*		zstrm	= that->m_zstrm;

		if (that->m_dst == 0)
			return -1;

		that->m_size += len;

		zstrm->avail_in = len;
		zstrm->next_in = reinterpret_cast<Bytef*>(const_cast<char*>(buf));
		zstrm->total_out = 0;

		while (zstrm->avail_in > 0)
		{
			zstrm->avail_out = sizeof(that->m_buf);
			zstrm->next_out = reinterpret_cast<Bytef*>(that->m_buf);

			if (::deflate(zstrm, Z_NO_FLUSH) == Z_STREAM_ERROR) {}
				//M_RAISE("zlib: deflate() failed");

			size_t have = sizeof(that->m_buf) - zstrm->avail_out;

			if (have > 0)
			{
				that->m_crc = crc32(that->m_crc, reinterpret_cast<Bytef*>(that->m_buf), have);

				if (::fwrite(that->m_buf, 1, have, that->m_dst) != have)
					return -1;
			}
		}

		that->m_compressedSize += zstrm->total_out;

		// if we return zstrm->total_out then fwrite will return a stream error
		return len;
	}
};


ZlibOStream::ZlibOStream(FILE* destination)
	:m_dst(0)
	,m_crc(0)
	,m_size(0)
	,m_compressedSize(0)
	,m_zstrm(new z_stream)
{
	//M_REQUIRE(destination);

	setmode(mstl::ios_base::out | mstl::ios_base::binary);

	m_zstrm->zalloc = 0;
	m_zstrm->zfree = 0;
	m_zstrm->opaque = 0;

	if (destination)
		open(destination);
}


ZlibOStream::~ZlibOStream() throw()
{
	close();
	delete m_zstrm;
}


void
ZlibOStream::open(FILE* destination)
{
	static cookie_io_functions_t Cookie	=
	{
		Cookie::read,
		Cookie::write,
		Cookie::seek,
		Cookie::close,
	};

	//M_REQUIRE(destination);
	//M_REQUIRE(!isOpen());

	if (::deflateInit(m_zstrm, Z_DEFAULT_COMPRESSION) != Z_OK) {}
		//M_RAISE("inflateInit() failed");

	m_dst = destination;
	m_fp = ::fopencookie(this, "wb", Cookie);
	::setvbuf(m_fp, 0, _IONBF, 0);
	m_size = 0;
	m_compressedSize = 0;
}


void
ZlibOStream::flush()
{
	//M_REQUIRE(isOpen());
	::fflush(m_fp);
}


void
ZlibOStream::close() throw()
{
	if (m_fp)
	{
		::fclose(m_fp);
		m_fp = 0;
	}
}

// vi:set ts=3 sw=3:
