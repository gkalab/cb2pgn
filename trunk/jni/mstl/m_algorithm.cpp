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

#include "m_algorithm.h"
#include "m_utility.h"

#include <alloca.h>
#include <string.h>


/// Exchanges ranges [first, middle) and [middle, last)
void
mstl::bits::rotate_fast(void* first, void* middle, void* last)
{
	// borrowed from ustl/ualgobase.cc

	size_t half1	= distance(first, middle);
	size_t half2	= distance(middle, last);
	size_t hmin		= min(half1, half2);

	if (!hmin)
		return;

	void* buf = alloca(hmin);

	if (buf)
	{
		if (half2 < half1)
		{
			size_t nBytes = distance(first, middle);

			memcpy(buf, middle, half2);
			memmove(advance(last, -nBytes), first, nBytes);
			memcpy(first, buf, half2);
		}
		else
		{
			memcpy(buf, first, half1);
			memcpy(first, middle, half2);
			memcpy(advance(first, half2), buf, half1);
		}
	}
	else
	{
		char* f = static_cast<char*>(first);
		char* m = static_cast<char*>(middle);
		char* l = static_cast<char*>(last);

		reverse(f, m);
		reverse(m, l);

		while (f != m && m != l)
			swap(*f++, *--l);

		reverse(f, f == m ? l : m);
	}
}

// vi:set ts=3 sw=3:
