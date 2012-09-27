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

#include "u_lzo_byte_stream.h"
#include "u_exception.h"

#include "m_iostream.h"
#include "m_assert.h"

#include <string.h>

using namespace util;


LzoByteStream::~LzoByteStream() throw() {}


LzoByteStream::LzoByteStream(mstl::iostream& strm)
	:ByteStream(m_in, Chunk_Size)
	,m_strm(strm)
{
}


void
LzoByteStream::flush()
{
	overflow(0);
}


void
LzoByteStream::underflow(unsigned)
{
	unsigned remaining = this->remaining();

	if (remaining > Additional_Size)
		U_RAISE("LZO stream: additional buffer size is too small");

	::memmove(m_out, m_getp, remaining);

	char buf[2];

	if (!m_strm.read(buf, 2))
		U_RAISE("LZO stream: unexpected end of stream");

	unsigned sizeAfterCompression = unsigned(buf[0]) << 8 | buf[1];

	if (sizeAfterCompression > Chunk_Size)
		U_RAISE("LZO stream: corrupt data");

	m_base = m_out;
	m_putp = m_out + remaining;
	m_endp = m_putp + sizeAfterCompression;

	if (sizeAfterCompression < Chunk_Size)
	{
		if (m_strm.readsome(m_in, Chunk_Size) < sizeAfterCompression)
			U_RAISE("LZO stream: unexpected end of stream");

		ByteStream src(m_in, sizeAfterCompression);
		Lzo::decompress(src, *this);

		if (size() != sizeAfterCompression)
			U_RAISE("LZO stream: corrupt data");
	}
	else if (m_strm.readsome(m_putp, Chunk_Size) < Chunk_Size)
	{
		U_RAISE("LZO stream: unexpected end of stream");
	}

	m_getp = m_putp - remaining;
}


void
LzoByteStream::overflow(unsigned)
{
	M_ASSERT(!m_owner);

	if (isEmpty())
		return;

	ByteStream dst(m_out, m_out + Buf_Size);

	if (!Lzo::compress(*this, dst))
	{
		ByteStream tmp(m_in, size());
		tmp.swap(dst);
	}

	char buf[2] = { dst.size() >> 8, dst.size() & 0x0f };

	if (!m_strm.write(buf, 2) || !m_strm.write(dst.data(), dst.size()))
		U_RAISE("LZO stream: write failed");

	m_base = m_putp = m_in;
}

// vi:set ts=3 sw=3:
