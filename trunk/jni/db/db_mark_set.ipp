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

inline MarkSet::MarkSet() {} // gcc-4.6 complains w/o explicit default constructor

inline bool MarkSet::isEmpty() const						{ return m_marks.empty(); }
inline unsigned MarkSet::count() const						{ return m_marks.size(); }
inline bool MarkSet::contains(Mark const& mark) const	{ return find(mark) >= 0; }


inline
bool
MarkSet::operator!=(MarkSet const& marks) const
{
	return !operator==(marks);
}


inline
Mark const&
MarkSet::operator[](unsigned index) const
{
	//M_REQUIRE(index < count());
	return m_marks[index];
}


inline
Mark&
MarkSet::operator[](unsigned index)
{
	//M_REQUIRE(index < count());
	return m_marks[index];
}


inline
void
MarkSet::add(Mark const& mark)
{
	m_marks.push_back(mark);
}


inline
void
MarkSet::add(char const* s)
{
	m_marks.push_back(Mark(s));
}


inline
Mark&
MarkSet::add()
{
	m_marks.push_back();
	return m_marks.back();
}


inline
void
MarkSet::remove(unsigned index)
{
	//M_REQUIRE(index < count());
	m_marks.erase(m_marks.begin() + index);
}


inline
void
MarkSet::clear()
{
	m_marks.clear();
}


inline
void
MarkSet::swap(MarkSet& marks)
{
	m_marks.swap(marks.m_marks);
}


#if HAVE_0X_MOVE_CONSTRCUTOR_AND_ASSIGMENT_OPERATOR

inline MarkSet::MarkSet(MarkSet&& set) : m_marks(mstl::move(set.m_marks)) {}


inline
MarkSet&
MarkSet::operator=(MarkSet&& set)
{
	m_marks = mstl::move(set.m_marks);
	return *this;
}

#endif

} // namespace db

// vi:set ts=3 sw=3:
