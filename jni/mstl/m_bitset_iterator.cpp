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

#include "m_bitset_iterator.h"

using namespace mstl;
using namespace mstl::bits;


bitset_iterator::iterator&
bitset_iterator::operator+=(int n)
{
	int k = n + m_offset;

	m_bf += k/bitfield::nbits;
	k %= bitfield::nbits;

	if (k < 0)
	{
		m_offset = k + bitfield::nbits;
		--m_bf;
	}
	else
	{
		m_offset = k;
	}

	return *this;
}


bitset_const_iterator::const_iterator&
bitset_const_iterator::operator+=(int n)
{
	int k = n + m_offset;

	m_bf += k/bitfield::nbits;
	k %= bitfield::nbits;

	if (k < 0)
	{
		m_offset = k + bitfield::nbits;
		--m_bf;
	}
	else
	{
		m_offset = k;
	}

	return *this;
}

// vi:set ts=3 sw=3:
