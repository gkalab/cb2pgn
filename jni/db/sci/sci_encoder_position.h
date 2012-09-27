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

#ifndef _sci_encoder_position_included
#define _sci_encoder_position_included

#include "db_common.h"

#include "m_stack.h"

namespace db {

class Board;
class Move;

namespace sci {
namespace encoder {

class Position
{
public:

	class Lookup
	{
	private:

		friend class Position;

		typedef Byte Numbers[64];

		Byte operator[](unsigned i) const;
		Byte& operator[](unsigned i);

		Numbers numbers;
	};

	Position();

	void setup(Board const& board);
	void setup();

	void preparePush();
	void push();
	void pop();
	void doMove(Move const& move);
	void doMove(Lookup& lookup, Move const& move);

	Lookup& lookup();
	Lookup& previous();

	Byte operator[](int n) const;

private:

	typedef mstl::stack<Lookup> Stack;

	Stack	m_stack;
	Byte	m_rookNumbers[4];
};

} // namespace encoder
} // namespace sci
} // namespace db

namespace mstl {

template <typename> struct is_pod;
template <> struct is_pod<db::sci::encoder::Position::Lookup> { enum { value = 1 }; };

} // namespace mstl
#include "sci_encoder_position.ipp"

#endif // _sci_encoder_position_included

// vi:set ts=3 sw=3:
