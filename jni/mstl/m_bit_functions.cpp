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

#include "m_bit_functions.h"

#ifdef BF_USE_FLIP_ARR
uint8_t mstl::bf::bits::Flip[256];
#endif

namespace mstl {
namespace bf {
namespace bits {

#if !__GNUC_PREREQ(3,4)
uint8_t Msb[65536];
uint8_t Lsb[65536];
#endif

#if !__GNUC_PREREQ(3,4) || defined(BF_USE_FLIP_ARR)

namespace {

struct initializer { initializer(); };
static initializer initializer;

initializer::initializer()
{
#if !__GNUC_PREREQ(3,4)

	Lsb[0] = 0;
	Msb[0] = 0;

	for (unsigned i = 1; i < 65536; i++)
	{
		Lsb[i] = 16;

		for (unsigned j = 0; j < 16; j++)
		{
			if (i & (1 << j))
			{
				Msb[i] = j;

				if (Lsb[i] == 16)
					Lsb[i] = j;
			}
		}
	}

#endif

#ifdef BF_USE_FLIP_ARR

	for (unsigned i = 0; i < 256; ++i)
	{
		uint8_t x = i;

		x =			((x >> 1) & 0x55) | ((x << 1) & 0xAA);
		x =			((x >> 2) & 0x33) | ((x << 2) & 0xCC);
		Flip[i] =	((x >> 4) & 0x0F) | ((x << 4) & 0xF0);
	}

#endif
}

} // namespace
} // namespace bits
} // namespace bf
} // namespace mstl

#endif // !__GNUC_PREREQ(3,4) || defined(BF_USE_FLIP_ARR)

#if !__GNUC_PREREQ(3,4)

# if __WORDSIZE == 64

unsigned
mstl::bf::bits::msb(uint64_t x)
{
	if (x & 0xffff)
		return Msb[x & 0xffff];

	if (x & 0xffff0000)
		return Msb[(x >> 16) & 0xffff] + 16;

	if (x & 0xffff00000000)
		return Msb[(x >> 32) & 0xffff] + 32;

	return Msb[x >> 48] + 48;
}


unsigned
mstl::bf::bits::lsb(uint64_t x)
{
	if (x & 0xffff000000000000)
		return Lsb[x >> 48] + 48;

	if (x & 0xffff00000000)
		return Lsb[(x >> 32) & 0xffff] + 32;

	if (x & 0xffff0000)
		return Lsb[(x >> 16) & 0xffff] + 16;

	return Lsb[x & 0xffff];
}

# else // if __WORDSIZE == 32

unsigned
mstl::bf::bits::msb(uint64_t x)
{
	if (unsigned(x) & 0xffff)
		return Msb[unsigned(x) & 0xffff];

	if (unsigned(x) & 0xffff0000)
		return Msb[(unsigned(x) >> 16) & 0xffff] + 16;

	if (x & UINT64_C(0xffff00000000))
		return Msb[unsigned(x >> 32) & 0xffff] + 32;

	return Msb[unsigned(x >> 48)] + 48;
}


unsigned
mstl::bf::bits::lsb(uint64_t x)
{
	if (x & UINT64_C(0xffff000000000000))
		return Lsb[unsigned(x >> 48)] + 48;

	if (x & UINT64_C(0xffff00000000))
		return Lsb[unsigned(x >> 32) & 0xffff] + 32;

	if (x & 0xffff0000)
		return Lsb[(unsigned(x) >> 16) & 0xffff] + 16;

	return Lsb[unsigned(x) & 0xffff];
}

# endif // __WORDSIZE == 32

unsigned
mstl::bf::bits::popcount(uint8_t x)
{
	x |= (x >> 1);
	x |= (x >> 2);
	x |= (x >> 4);

	return x;
}


unsigned
mstl::bf::bits::popcount(uint16_t x)
{
	x |= (x >> 1);
	x |= (x >> 2);
	x |= (x >> 4);
	x |= (x >> 8);

	return x;
}


unsigned
mstl::bf::bits::popcount(uint32_t x)
{
#if 1

	// algorithm from http://aggregate.ee.engr.uky.edu/MAGIC/

	x -= ((x >> 1) & 0x55555555);
	x = (x & 0x33333333) + ((x >> 2) & 0x33333333);
	x = ((x >> 4) + x) & 0x0f0f0f0f;
	x += x >> 8;
	x += x >> 16;

	return x & 0x0000003f;

#else

	x =		(x & 0x55555555) + ((x & 0xaaaaaaaa) >>  1);
	x =		(x & 0x33333333) + ((x & 0xcccccccc) >>  2);
	x =		(x & 0x0f0f0f0f) + ((x & 0xf0f0f0f0) >>  4);
	x =		(x & 0X00ff00ff) + ((x & 0xff00ff00) >>  8);
	return	(x & 0X0000ffff) + ((x & 0xffff0000) >> 16);

#endif
}


unsigned
mstl::bf::bits::popcount(uint64_t x)
{
#if 1

	// algorithm from Wikipedia

	x -= (x >> 1) & UINT64_C(0x5555555555555555);
	x  = (x & UINT64_C(0x3333333333333333)) + ((x >> 2) & UINT64_C(0x3333333333333333));
	x  = (x + (x >> 4)) & UINT64_C(0x0f0f0f0f0f0f0f0f);

	return ((x*UINT64_C(0x0101010101010101)) >> 56);

#else

   x =		(x & UINT64_C(0x5555555555555555)) + ((x & UINT64_C(0xaaaaaaaaaaaaaaaa)) >>  1);
   x =		(x & UINT64_C(0x3333333333333333)) + ((x & UINT64_C(0xcccccccccccccccc)) >>  2);
   x =		(x & UINT64_C(0x0f0f0f0f0f0f0f0f)) + ((x & UINT64_C(0xf0f0f0f0f0f0f0f0)) >>  4);
   x =		(x & UINT64_C(0x00ff00ff00ff00ff)) + ((x & UINT64_C(0xff00ff00ff00ff00)) >>  8);
   x =		(x & UINT64_C(0x0000ffff0000ffff)) + ((x & UINT64_C(0xffff0000ffff0000)) >> 16);
   return	(x & UINT64_C(0x00000000ffffffff)) + ((x & UINT64_C(0xffffffff00000000)) >> 32);

#endif
}

#endif	// !__GNUC_PREREQ(3,4)


uint32_t
mstl::bf::bits::reverse(uint32_t a)
{
	a =		((a >>  1) & 0x55555555) | ((a <<  1) & 0xAAAAAAAA);
	a =		((a >>  2) & 0x33333333) | ((a <<  2) & 0xCCCCCCCC);
	a =		((a >>  4) & 0x0F0F0F0F) | ((a <<  4) & 0xF0F0F0F0);
	a =		((a >>  8) & 0x00FF00FF) | ((a <<  8) & 0xFF00FF00);
	return	((a >> 16) & 0x0000FFFF) | ((a << 16) & 0xFFFF0000);
}


uint64_t
mstl::bf::bits::reverse(uint64_t a)
{
   a =		((a >>  1) & 0x5555555555555555LL) | ((a <<  1) & 0xAAAAAAAAAAAAAAAALL);
   a =		((a >>  2) & 0x3333333333333333LL) | ((a <<  2) & 0xCCCCCCCCCCCCCCCCLL);
   a =		((a >>  4) & 0x0F0F0F0F0F0F0F0FLL) | ((a <<  4) & 0xF0F0F0F0F0F0F0F0LL);
   a =		((a >>  8) & 0x00FF00FF00FF00FFLL) | ((a <<  8) & 0xFF00FF00FF00FF00LL);
   a =		((a >> 16) & 0x0000FFFF0000FFFFLL) | ((a << 16) & 0xFFFF0000FFFF0000LL);
   return	((a >> 32) & 0x00000000FFFFFFFFLL) | ((a << 32) & 0xFFFFFFFF00000000LL);
}

// vi:set ts=3 sw=3:
