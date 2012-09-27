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

#ifndef _u_bit_stream_included
#define _u_bit_stream_included

#include "u_base.h"

namespace util {

class BitStream
{
public:

	typedef unsigned char Byte;

	BitStream(Byte const* buf, unsigned size);

#if HAVE_0X_MOVE_CONSTRCUTOR_AND_ASSIGMENT_OPERATOR
	BitStream(BitStream&& strm);
	BitStream& operator=(BitStream&& strm);
#endif

	unsigned bitsLeft() const;

	Byte peek(unsigned n);
	Byte next(unsigned n);

	void skip(unsigned n);

private:

	void fetchBits(unsigned n);
	void getBits();

	Byte const*	m_buffer;
	unsigned 	m_size;
	uint32_t		m_bits;
	unsigned		m_bitsLeft;
};

}; // namespace util

#include "u_bit_stream.ipp"

#endif // _u_bit_stream_included

// vi:set ts=3 sw=3:
