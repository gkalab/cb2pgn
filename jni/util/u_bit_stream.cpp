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

#include "u_bit_stream.h"

#include "m_assert.h"

using namespace util;


BitStream::BitStream(Byte const* buf, unsigned size)
	:m_buffer(buf)
	,m_size(size)
	,m_bits(0)
	,m_bitsLeft(0)
{
}


void
BitStream::skip(unsigned n)
{
	//M_REQUIRE(n <= bitsLeft());

	if (n <= m_bitsLeft)
	{
		m_bitsLeft -= n;
	}
	else
	{
		n -= m_bitsLeft;

		unsigned nbytes = n >> 3;

		m_bitsLeft = 0;
		m_size -= nbytes;
		m_buffer += nbytes;
		fetchBits(n -= (nbytes << 3));
		m_bitsLeft -= n;
	}
}


void
BitStream::getBits()
{
	//M_ASSERT(m_size);

	do
	{
		--m_size;
		m_bits = (m_bits << 8) | *m_buffer++;
		m_bitsLeft += 8;
	}
	while (m_size && m_bitsLeft <= U_BITS_OF(m_bits) - 8);
}

// vi:set ts=3 sw=3:
