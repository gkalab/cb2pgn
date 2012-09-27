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

namespace db {
namespace cbh {
namespace decoder {

inline unsigned Position::variationLevel() const	{ return m_stack.size() - 1; }
inline Board const& Position::board() const			{ return m_stack.top().board; }

inline void Position::push()	{ m_stack.dup(); }
inline void Position::pop()	{ m_stack.pop(); }


inline
Move
Position::doNullMove()
{
	return doMove(Move::null());
}


inline
Move
Position::doCaptureRight(Byte number)
{
	return doCapture(number, m_stack.top().board.whiteToMove() ? +9 : -9);
}


inline
Move
Position::doCaptureLeft(Byte number)
{
	return doCapture(number, m_stack.top().board.whiteToMove() ? +7 : -7);
}

} // namespace decoder
} // namespace cbh
} // namespace db

// vi:set ts=3 sw=3:
