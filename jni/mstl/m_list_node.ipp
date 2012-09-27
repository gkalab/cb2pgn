// ======================================================================
// Author : $Author$
// Version: $Revision$
// Date   : $Date$
// Url    : $URL$
// ======================================================================

// ======================================================================
// Copyright: (C) 2012 Gregor Cramer
// ======================================================================

// ======================================================================
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
// ======================================================================

#include "m_assert.h"

namespace mstl {
namespace bits {

//   +-------------+
//   |             V
// +-+-+  +---+  +---+
// | | |  |   |  |   |
// | * |<-+-* |<-+-* |
// |   |  |   |  |   |
// | *-+->| *-+->| * |
// |   |  |   |  | | |
// +---+  +---+  +-+-+
//   ^             |
//   +-------------+


inline
node_base::node_base()
	:m_next(this)
	,m_prev(this)
{
}


inline
void
node_base::swap(node_base& lhs, node_base& rhs)
{
	mstl::swap(lhs.m_prev, rhs.m_prev);
	mstl::swap(lhs.m_next, rhs.m_next);
}


inline
void
node_base::hook(node_base* succ)
{
	//M_ASSERT(succ);

	m_next = succ;
	m_prev = succ->m_prev;

	m_prev->m_next = this;
	m_next->m_prev = this;
}


inline
void
node_base::unhook()
{
	//M_ASSERT(m_next);
	//M_ASSERT(m_prev);

	m_next->m_prev = m_prev;
	m_prev->m_next = m_next;
}

} // namespace bits
} // namespace mstl

// vi:set ts=3 sw=3:
