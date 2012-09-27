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

#ifndef _mstl_list_node_included
#define _mstl_list_node_included

namespace mstl {
namespace bits {

struct node_base
{
	node_base();

	void hook(node_base* succ);
	void unhook();
	void swap(node_base& lhs, node_base& rhs);

	node_base* m_next;
	node_base* m_prev;
};

} // namespace bits
} // namespace mstl

#include "m_list_node.ipp"

#endif // _mstl_list_node_included

// vi:set ts=3 sw=3:
