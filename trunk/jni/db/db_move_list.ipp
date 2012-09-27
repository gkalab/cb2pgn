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

#include "m_assert.h"

namespace db {

inline MoveList::MoveList() :m_size(0) {}

inline bool MoveList::isEmpty()	const 		{ return m_size == 0; }
inline bool MoveList::isFull() const			{ return m_size == Maximum_Moves; }
inline bool MoveList::notFull() const			{ return m_size < Maximum_Moves; }
inline Move const* MoveList::begin() const	{ return m_buffer; }
inline Move const* MoveList::end() const		{ return m_buffer + m_size; }
inline Move* MoveList::begin()					{ return m_buffer; }
inline Move* MoveList::end()						{ return m_buffer + m_size; }
inline unsigned MoveList::size()	const 		{ return m_size; }
inline void MoveList::clear()						{ m_size = 0; }


inline void
MoveList::pop()
{
	//M_REQUIRE(!isEmpty());
	--m_size;
}


inline
void
MoveList::cut(unsigned size)
{
	//M_REQUIRE(size <= m_size);
	m_size = size;
}


inline
Move const&
MoveList::operator[](unsigned n) const
{
	//M_REQUIRE(n < size());
	return m_buffer[n];
}


inline
Move&
MoveList::operator[](unsigned n)
{
	//M_REQUIRE(n < size());
	return m_buffer[n];
}


inline
Move const&
MoveList::back() const
{
	//M_REQUIRE(!isEmpty());
	return m_buffer[m_size - 1];
}


inline
Move
MoveList::back()
{
	//M_REQUIRE(!isEmpty());
	return m_buffer[m_size - 1];
}


inline
void
MoveList::append(Move const& m)
{
	//M_ASSERT(m_size < U_NUMBER_OF(m_buffer));
	m_buffer[m_size++] = m;
}


inline
void
MoveList::push(Move const& m)
{
	append(m);
}

} // namespace db

// vi:set ts=3 sw=3:
