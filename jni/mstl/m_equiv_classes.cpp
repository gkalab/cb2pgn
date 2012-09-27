// ======================================================================
// Author : $Author$
// Version: $Revision$
// Date   : $Date$
// Url    : $URL$
// ======================================================================

// ======================================================================
// Copyright: (C)2011-2012 Gregor Cramer
// ======================================================================

// ======================================================================
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
// ======================================================================

#include "m_equiv_classes.h"
#include "m_utility.h"

using namespace mstl;


equiv_classes::link::link() :next(0), data(0) {}
equiv_classes::link::link(unsigned a) :next(0), data(a) {}
equiv_classes::link::link(link* l, unsigned a) :next(l), data(a) {}


equiv_classes::equiv_classes(unsigned n)
	:m_list(n)
	,m_storage(n)
	,m_ngroups(n + 1)
{
	for (unsigned i = 0; i < n; ++i)
		(m_list[i] = m_storage[i] = new link)->data = i;
}


equiv_classes::link*
equiv_classes::link::last() const
{
	link* cur = next;

	while (cur->next)
		cur = cur->next;

	return cur;
}


equiv_classes::~equiv_classes() throw()
{
	for (unsigned i = 0; i < m_storage.size(); ++i)
		delete m_storage[i];
}


equiv_classes::link*
equiv_classes::new_link(unsigned a)
{
	m_storage.push_back(new link(a));
	return m_storage.back();
}



equiv_classes::link*
equiv_classes::new_link(link* next, unsigned a)
{
	m_storage.push_back(new link(next, a));
	return m_storage.back();
}


void
equiv_classes::add_relation(unsigned a, unsigned b)
{
	//M_REQUIRE(a < size());
	//M_REQUIRE(b < size());

	if (a == b)
		return;

	if (a < b)
		mstl::swap(a, b);

	if (m_list[a]->next == 0)
	{
		if (m_list[b]->next == 0)
		{
			m_list[a]->next = new_link(new_link(b), a);
			m_list[b]->next = m_list[a]->next;
		}
		else
		{
			m_list[b]->last()->next = new_link(a);
			m_list[a]->next = m_list[b]->next;
		}
	}
	else
	{
		if (m_list[b]->next == 0)
		{
			m_list[a]->last()->next = new_link(b);
			m_list[b]->next = m_list[a]->next;
		}
		else if (m_list[a]->next != m_list[b]->next)
		{
			link* nextb = m_list[b]->next;

			for (link* cur = m_list[b]->next; cur; cur = cur->next)
				m_list[cur->data]->next = m_list[a]->next;

			m_list[a]->last()->next = nextb;
		}
	}
}


void
equiv_classes::prepare_groups() const
{
	unsigned n = m_list.size();

	m_ngroups = 0;
	m_lookup.resize(n, n);
	m_dimension.reserve(n);

	for (unsigned i = 0; i < n; ++i)
	{
		if (m_lookup[m_list[i]->next->data] == n)
		{
			unsigned count = 0;

			for (link* cur = m_list[i]->next; cur; cur = cur->next, ++count)
				m_lookup[cur->data] = m_ngroups;

			m_dimension.push_back(count);
			++m_ngroups;
		}
	}
}

// vi:set ts=3 sw=3:
