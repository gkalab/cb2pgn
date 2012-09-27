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

inline bool Mark::isEmpty() const		{ return m_command == mark::None; }

inline mark::Type Mark::type() const	{ return m_type; }
inline char Mark::text() const			{ return m_text; }
inline mark::Color Mark::color() const	{ return m_color; }


inline
Square
Mark::square(unsigned index) const
{
	//M_REQUIRE(index <= 1);
	return index ? m_square2 : m_square1;
}


inline
void
Mark::clear()
{
	*this = Mark();
}


#if HAVE_0X_MOVE_CONSTRCUTOR_AND_ASSIGMENT_OPERATOR

inline
Mark::Mark(Mark&& mark)
	:m_command(mark.m_command)
	,m_type(mark.m_type)
	,m_text(mark.m_text)
	,m_color(mark.m_color)
	,m_square1(mark.m_square1)
	,m_square2(mark.m_square2)
	,m_caption(mstl::move(mark.m_caption))
{
}


inline
Mark&
Mark::operator=(Mark&& mark)
{
	if (this != &mark)
	{
		m_command = mark.m_command;
		m_type = mark.m_type;
		m_text = mark.m_text;
		m_color = mark.m_color;
		m_square1 = mark.m_square1;
		m_square2 = mark.m_square2;
		m_caption = mstl::move(mark.m_caption);
	}

	return *this;
}

#endif

} // namespace db

// vi:set ts=3 sw=3:
