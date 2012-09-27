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

#include "m_utility.h"
#include "m_assert.h"

namespace db {

inline bool Filter::isComplete() const					{ return m_count == m_set.size(); }
inline bool Filter::isCompressed() const				{ return m_set.compressed(); }
inline bool Filter::checkClassInvariance() const	{ return m_count == m_set.count(); }

inline unsigned Filter::count() const	{ return m_count; }
inline unsigned Filter::size() const	{ return m_set.size(); }

inline bool Filter::isEmpty() const		{ return m_count == 0; }


inline
bool
Filter::contains(unsigned index) const
{
	// M_REQUIRE(index < size());
	return m_set.test(index);
}


inline
int
Filter::next(int current) const
{
	// M_REQUIRE(current < int(size()));
	return current == Invalid ? m_set.find_first() : int(m_set.find_next(current));
}


inline
int
Filter::prev(int current) const
{
	// M_REQUIRE(current < int(size()));
	return current == Invalid ? m_set.find_last() : int(m_set.find_prev(current));
}


inline
void
Filter::add(unsigned index)
{
	// M_REQUIRE(index < size());

	if (!m_set.test_and_set(index))
		++m_count;

// too slow!
//	M_ASSERT(checkClassInvariance());
}


inline
void
Filter::compress()
{
	// M_ASSERT(m_set.compressed() || m_count == m_set.count());
	m_set.compress();
}


inline
void
Filter::uncompress()
{
	m_set.uncompress();
	// M_ASSERT(checkClassInvariance());
}


#if HAVE_0X_MOVE_CONSTRCUTOR_AND_ASSIGMENT_OPERATOR

inline
Filter::Filter(Filter&& filter)
	:m_set(mstl::move(filter.m_set))
	,m_count(filter.m_count)
{
}


inline
Filter&
Filter::operator=(Filter&& filter)
{
	m_set = mstl::move(filter.m_set);
	m_count = filter.m_count;
	return *this;
}

#endif

} // namespace db

// vi:set ts=3 sw=3:
