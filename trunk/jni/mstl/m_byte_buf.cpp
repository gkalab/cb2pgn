// ======================================================================
// Author : $Author$
// Version: $Revision$
// Date   : $Date$
// Url    : $URL$
// ======================================================================

// ======================================================================
// Copyright: (C) 2010-2012 Gregor Cramer
// ======================================================================

// ======================================================================
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
// ======================================================================

#include "m_byte_buf.h"
#include "m_utility.h"
#include "m_assert.h"

#include <string.h>

using namespace mstl;

enum
{
	Min_RLE_Length		= 9,

	Code_Zero_Literal	= 0,
	Code_Prev_Literal	= 1,
	Code_Run_Length	= 2,
	Code_New_Literal	= 3,
};

static unsigned char const Flag_Packed = 0;
static unsigned char const Flag_Copied = 1;


namespace {

// Adopted from scid/src/filter.cpp: packBytemap(), unpackBytemap()
// (with slight modifications)
// -----------------------------------------------------------------
// Compresses the contents a tailored run-length encoding and
// byte packing algorithm.
//
// The compression algorithm assumes:
//   -  that the byte value 0 is very common;
//   -  that the input buffer usually contains just two values,
// which is typically the case for tree filters.
//
// At each step through the algorithm, one of the following is coded:
//   - a run length of 9 or more of the same value is coded in 18
//       bits (or 42 bits if the length is >= 255); or
//   - a byte with value zero is coded in two bits; or
//   - a byte with nonzero value the same as the last nonzero byte
//       (excluding run lengths) is coded in two bits; or
//   - a byte with nonzero value different to the last nonzero byte
//       is coded in 10 bits.
// -----------------------------------------------------------------

class Codec
{
public:

	typedef byte_buf::byte byte;

	byte_buf* compress(unsigned src_bytes, byte const* src);
	static void uncompress(byte_buf const& data, byte* dst, unsigned dst_bytes);

private:

	void encode_control_bits(unsigned bits);

	byte		m_ctrl_data;
	byte		m_ctrl_bits;
	byte*		m_ctrl;
	byte*		m_dst;
	unsigned	m_dst_bytes;
};


void
Codec::encode_control_bits(unsigned bits)
{
	//M_ASSERT(bits >= 0 && bits <= 3);

	m_ctrl_data >>= 2;
	m_ctrl_data |= (bits << 6);

	//M_ASSERT(m_ctrl_bits >= 2);

	if ((m_ctrl_bits -= 2) == 0)
	{
		*m_ctrl = m_ctrl_data;
		m_ctrl = m_dst++;
		m_dst_bytes++;
		m_ctrl_data = 0;
		m_ctrl_bits = 8;
	}
}


byte_buf*
Codec::compress(unsigned src_bytes, byte const* data)
{
	enum { Min_Size_Reduction = 7 };

	typedef byte_buf::byte byte;

	if (src_bytes >= Min_Size_Reduction)
	{
		byte buffer[src_bytes + 8];
		byte prev_literal = 0;

		byte const* src = data;
		byte const* end = data + src_bytes;

		m_dst = buffer + 2;
		m_ctrl = buffer + 1;
		m_ctrl_data = 0;
		m_ctrl_bits = 8;
		m_dst_bytes = 2;

		buffer[0] = Flag_Packed;

		while (src < end && m_dst_bytes <= src_bytes)
		{
			// Find the run length value:

			byte value = *src;
			byte const* p = src + 1;

			while (p < end && *p == value)
				++p;

			unsigned rle = mstl::min(size_t(16777215), size_t(p - src));

			if (rle >= Min_RLE_Length)
			{
				// Run length is long enough to be worth encoding as a run:
				encode_control_bits(Code_Run_Length);
				src += rle;
				*m_dst++ = value;

				if (rle > 255)
				{
					// Longer run length:
					*m_dst++ = 0;
					*m_dst++ = (rle >> 16) & 255;
					*m_dst++ = (rle >>  8) & 255;
					*m_dst++ = rle & 255;
					m_dst_bytes += 5;
				} else {
					*m_dst++ = rle;
					m_dst_bytes += 2;
				}
			}
			else if (value == 0)
			{
				// Zero-valued literal: coded in two bits.
				encode_control_bits(Code_Zero_Literal);
				src++;
			}
			else if (value == prev_literal)
			{
				// Nonzero literal, same as previous: coded in two bits.
				encode_control_bits(Code_Prev_Literal);
				src++;
			}
			else
			{
				// Nonzero literal, different to previous one: coded in 10 bits.
				encode_control_bits(Code_New_Literal);
				src++;
				prev_literal = value;
				*m_dst++ = value;
				m_dst_bytes++;
			}
		}

		// Flush the control bits:
		*m_ctrl = m_ctrl_data >> m_ctrl_bits;

		if (m_dst_bytes + 10 < src_bytes)
			return new byte_buf(m_dst_bytes, buffer);
	}

	// Switch to regular copying:
	byte_buf* result = new byte_buf(src_bytes + 1);
	*result->data() = Flag_Copied;
	result->copy(src_bytes, reinterpret_cast<byte const*>(data), 1);
	return result;
}


void
Codec::uncompress(byte_buf const& buf, byte* dst, unsigned dst_bytes)
{
	//M_ASSERT(buf.size() > 0);

	// Check if the buffer was copied without compression:

	if (*buf.data() == Flag_Copied)
	{
		//M_ASSERT(dst_bytes + 1 == buf.size());
		::memcpy(dst, buf.data() + 1, mstl::min(buf.size(), byte_buf::size_type(dst_bytes)));
		return;
	}

	//M_ASSERT(*buf.data() == Flag_Packed);
	//M_ASSERT(buf.size() > 2);

	byte const* src = buf.data() + 1;

	unsigned src_bytes_left	= buf.size() - 2;
	unsigned ctrl_data		= *src++;
	unsigned ctrl_bits		= 8;

	byte prev_literal = 0;

	while (dst_bytes > 0)
	{
		byte		value;
		unsigned	length;

		// Read the two control bits for this literal or run length:
		unsigned code = ctrl_data & 3;

		ctrl_data >>= 2;
		ctrl_bits -= 2;

		if (ctrl_bits == 0)
		{
			//M_ASSERT(src_bytes_left > 0);
			--src_bytes_left;
			ctrl_data = *src++;
			ctrl_bits = 8;
		}

		switch (code)
		{
			case Code_Zero_Literal:		// Literal value zero:
				*dst++ = 0;
				--dst_bytes;
				break;

			case Code_Prev_Literal:		// Nonzero literal same as previous:
				*dst++ = prev_literal;
				--dst_bytes;
				break;

			case Code_Run_Length:		// Run length encoding:
				//M_ASSERT(src_bytes_left >= 2);
				src_bytes_left -= 2;
				value = *src++;
				length = *src++;

				if (length == 0)
				{
					// Longer run length, coded in next 3 bytes:
					//M_ASSERT(src_bytes_left >= 3);
					src_bytes_left -= 3;
					length = *src++;
					length <<= 8; length |= *src++;
					length <<= 8; length |= *src++;
				}

				//M_ASSERT(dst_bytes >= length);
				dst_bytes -= length;
				while (length--)
					*dst++ = value;
				break;

			case Code_New_Literal:		// Nonzero literal with different value:
				//M_ASSERT(src_bytes_left >= 1);
				prev_literal = *src++;
				--src_bytes_left;
				*dst++ = prev_literal;
				--dst_bytes;
				break;

			default:
				//M_ASSERT(!"broken data");
				return;
		}
	}

	//M_ASSERT(src_bytes_left == 0);
	//M_ASSERT(dst_bytes == 0);
}

} // namespace


byte_buf&
byte_buf::operator=(byte_buf const& buf)
{
	if (this != &buf)
	{
		if (m_size < buf.m_size)
		{
			delete [] m_data;
			m_data = new value_type[buf.m_size];
		}

		::memcpy(m_data, buf.m_data, m_size = buf.m_size);
	}

	return *this;
}


void
byte_buf::copy(size_type size, value_type const* data, size_type offset)
{
	//M_REQUIRE(data || size == 0);
	//M_REQUIRE(size + offset <= this->size());

	::memcpy(m_data + offset, data, size);
}


byte_buf*
byte_buf::compress(size_type size, value_type const* data)
{
	//M_REQUIRE(data || size == 0);

	return Codec().compress(size, data);
}


void
byte_buf::uncompress(byte* dst, unsigned dst_bytes) const
{
	return Codec::uncompress(*this, dst, dst_bytes);
}

// vi:set ts=3 sw=3:
