// ======================================================================
// Author : $Author$
// Version: $Revision$
// Date   : $Date$
// Url    : $URL$
// ======================================================================

// ======================================================================
// Copyright: (C)2011-2012 Gregor Cramer
// ======================================================================

// ======================================================================
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
// ======================================================================

#include "u_base64_decoder.h"

#include "u_exception.h"

using namespace util;


enum
{
	B64_Special	= 0x80,
	B64_Space	= 0x80,
	B64_Pad		= 0x81,
	B64_Done		= 0x82,
	B64_Bad		= 0x83,
};


static unsigned char Base64Tbl[256] =
{
//  NUL   SOH   STX   ETX   EOT   ENQ   ACK   BEL    BS    HT    LF    VT    FF    CR    SO    SI
	0x82, 0x83, 0x83, 0x83, 0x83, 0x83, 0x83, 0x83, 0x83, 0x80, 0x80, 0x83, 0x80, 0x80, 0x83, 0x83,
//  DLE   DC1   DC2   DC3   DC4   NAK   SYN   ETB   CAN    EM   SUB   ESC    FS    GS    RS    US
	0x83, 0x83, 0x83, 0x83, 0x83, 0x83, 0x83, 0x83, 0x83, 0x83, 0x83, 0x83, 0x83, 0x83, 0x83, 0x83,
//         !     "     #     $     %     &     '     (     )     *     +     ,     -     .     /
	0x80, 0x83, 0x83, 0x83, 0x83, 0x83, 0x83, 0x83, 0x83, 0x83, 0x83, 0x3e, 0x83, 0x83, 0x83, 0x3f,
//   0     1     2     3     4     5     6     7     8     9     :     ;     <     =     >     ?
	0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x83, 0x83, 0x83, 0x81, 0x83, 0x83,
//   @     A     B     C     D     E     F     G     H     I     J     K     L     M     N     O
	0x83, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e,
//   P     Q     R     S     T     U     V     W     X     Y     Z     [     \     ]     ^     _
	0x0f, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x83, 0x83, 0x83, 0x83, 0x83,
//   `     a     b     c     d     e     f     g     h     i     j     k     l     m     n     o
	0x83, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28,
//   p     q     r     s     t     u     v     w     x     y     z     {     |     }     ~    DEL
	0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f, 0x30, 0x31, 0x32, 0x33, 0x83, 0x83, 0x83, 0x83, 0x83,
// 128 .. 255
	0x83, 0x83, 0x83, 0x83, 0x83, 0x83, 0x83, 0x83, 0x83, 0x83, 0x83, 0x83, 0x83, 0x83, 0x83, 0x83,
	0x83, 0x83, 0x83, 0x83, 0x83, 0x83, 0x83, 0x83, 0x83, 0x83, 0x83, 0x83, 0x83, 0x83, 0x83, 0x83,
	0x83, 0x83, 0x83, 0x83, 0x83, 0x83, 0x83, 0x83, 0x83, 0x83, 0x83, 0x83, 0x83, 0x83, 0x83, 0x83,
	0x83, 0x83, 0x83, 0x83, 0x83, 0x83, 0x83, 0x83, 0x83, 0x83, 0x83, 0x83, 0x83, 0x83, 0x83, 0x83,
	0x83, 0x83, 0x83, 0x83, 0x83, 0x83, 0x83, 0x83, 0x83, 0x83, 0x83, 0x83, 0x83, 0x83, 0x83, 0x83,
	0x83, 0x83, 0x83, 0x83, 0x83, 0x83, 0x83, 0x83, 0x83, 0x83, 0x83, 0x83, 0x83, 0x83, 0x83, 0x83,
	0x83, 0x83, 0x83, 0x83, 0x83, 0x83, 0x83, 0x83, 0x83, 0x83, 0x83, 0x83, 0x83, 0x83, 0x83, 0x83,
	0x83, 0x83, 0x83, 0x83, 0x83, 0x83, 0x83, 0x83, 0x83, 0x83, 0x83, 0x83, 0x83, 0x83, 0x83, 0x83,
};


Base64Decoder::Base64Decoder(unsigned char const* first, unsigned char const* last)
	:m_base(first)
	,m_current(first)
	,m_end(last)
	,m_last(0)
	,m_state(0)
{
}


void
Base64Decoder::reset()
{
	m_current = m_base;
	m_last = 0;
	m_state = 0;
}


size_t
Base64Decoder::read(unsigned char* buf, size_t len)
{
	unsigned char* p = buf;
	unsigned char* e = buf + len;

	for ( ; p < e && m_current < m_end; ++m_current)
	{
		unsigned char c64 = Base64Tbl[*m_current];

		if (__builtin_expect(c64 & B64_Special, 0))
		{
#ifdef NDBUG
			// ignore character
#else
			/*switch (c64)
			{
				case B64_Pad:
					if (m_last)
						throw BasicException("couldn't recognize image data");
					break;

				case B64_Space:	break;	// ignore spaces
				case B64_Done:		throw BasicException("unexpected nul byte in Base-64 stream");
				default:				throw BasicException("illegal character in Base-64 stream");
			}*/
#endif
		}
		else
		{
			switch (m_state++)
			{
				case 0:
					m_last = c64 << 2;
					break;

				case 1:
					*p++ = m_last | (c64 >> 4);
					m_last = (c64 & 0xF) << 4;
					break;

				case 2:
					*p++ = m_last | (c64 >> 2);
					m_last = (c64 & 0x3) << 6;
					break;

				case 3:
					*p++ = m_last | c64;
					m_last = m_state = 0;
					break;
			}
		}
	}

	return p - buf;
}


bool
Base64Decoder::skip(size_t nbytes)
{
	for ( ; nbytes && m_current < m_end; ++m_current)
	{
		unsigned char c64 = Base64Tbl[*m_current];

		if (__builtin_expect(c64 & B64_Special, 0))
		{
#ifdef NDBUG
			// ignore character
#else
			/*switch (c64)
			{
				case B64_Pad:
					if (m_last == 0)
						throw BasicException("incomplete Base-64 stream");
					break;

				case B64_Space:	break; // ignore spaces
				case B64_Done:		throw BasicException("unexpected nul byte in Base-64 stream");
				default:				throw BasicException("illegal character in Base-64 stream");
			}*/
#endif
		}
		else
		{
			switch (m_state++)
			{
				case 0:
					m_last = c64 << 2;
					break;

				case 1:
					m_last = (c64 & 0xF) << 4;
					--nbytes;
					break;

				case 2:
					m_last = (c64 & 0x3) << 6;
					--nbytes;
					break;

				case 3:
					m_last = m_state = 0;
					--nbytes;
					break;
			}
		}
	}

	return nbytes == 0;
}

// vi:set ts=3 sw=3:
