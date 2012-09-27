// ======================================================================
// Author : $Author$
// Version: $Revision$
// Date   : $Date$
// Url    : $URL$
// ======================================================================

// ======================================================================
//    _/|            __
//   // o\         /    )           ,        /    /
//   || ._)    ----\---------__----------__-/----/__-
//   //__\          \      /   '  /    /   /    /   )
//   )___(     _(____/____(___ __/____(___/____(___/_
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

#include "db_move_list.h"

#include "m_utility.h"
#include "m_stdio.h"

#include <string.h>

using namespace db;


MoveList&
MoveList::operator=(MoveList const& list)
{
	static_assert(mstl::is_pod<Move>::value, "POD required");

	if (&list != this)
		::memcpy(m_buffer, list.m_buffer, (m_size = list.m_size)*sizeof(m_buffer[0]));

	return *this;
}


int
MoveList::find(uint16_t move) const
{
	for (unsigned i = 0; i < m_size; ++i)
	{
		if (m_buffer[i].index() == move)
			return i;
	}

	return -1;
}


void
MoveList::sort(int scores[])
{
	// require: #scores >= size()

	if (size() <= 1)
		return;

	for (unsigned k = 0, n = m_size - 1; k < n; ++k)
	{
		unsigned index = k;

		int score = scores[index];

		for (unsigned i = k + 1; i < m_size; ++i)
		{
			if (score < scores[i])
				score = scores[index = i];
		}

		if (index > k)
		{
			mstl::swap(scores[k], scores[index]);
			mstl::swap(m_buffer[k], m_buffer[index]);
		}
	}
}


void
MoveList::sort(unsigned startIndex, int scores[])
{
	// M_REQUIRE(startIndex < size());
	// require: #scores >= size()

	int score = scores[startIndex];

	unsigned index = startIndex;

	for (unsigned i = startIndex + 1; i < m_size; ++i)
	{
		if (score < scores[i])
			score = scores[index = i];
	}

	if (index > startIndex)
	{
		mstl::swap(scores[startIndex], scores[index]);
		mstl::swap(m_buffer[startIndex], m_buffer[index]);
	}
}


void
MoveList::dump()
{
	::printf("Moves(%u)\n", m_size);
	for (unsigned i = 0; i < m_size; ++i)
	{
		::printf("  ");
		m_buffer[i].dump();
	}
	::fflush(stdout);
}

// vi:set ts=3 sw=3:
