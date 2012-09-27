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

#include "m_hash.h"
#include "m_string.h"
#include "m_bit_functions.h"
#include "m_utility.h"


mstl::bits::size_t
mstl::bits::hash_expansion(size_t size)
{
	return size_t(1) << (bf::msb_index(size) + 1);
}


mstl::bits::size_t
mstl::bits::hash_size(size_t min_size)
{
	//M_ASSERT(min_size > 0);
	return is_not_pow_2(min_size) ? hash_expansion(min_size) : min_size;
}


mstl::bits::size_t
mstl::bits::hash_str(mstl::string const& key)
{
	char const* s = key.begin();
	char const* e = key.end();

	// FNV1 hash algorithm
	// source: <http://www.isthe.com/chongo/tech/comp/fnv/index.html>
	// NOTE: due to Bob Jenkin FNV is a good choice for short keys

	size_t value = 2166136261u;

	for ( ; s < e; ++s)
		value = (value ^ *s)*16777619u;

	return value;
}


mstl::bits::size_t
mstl::bits::hash_str(char const* key)
{
	// FNV1 hash algorithm
	// source: <http://www.isthe.com/chongo/tech/comp/fnv/index.html>
	// NOTE: due to Bob Jenkin FNV is a good choice for short keys

	size_t value = 2166136261u;

	for ( ; *key; ++key)
		value = (value ^ *key)*16777619u;

	return value;
}

// vi:set ts=3 sw=3:
