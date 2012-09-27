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

#ifndef _mstl_equiv_classes_included
#define _mstl_equiv_classes_included

#include "m_vector.h"

namespace mstl {

class equiv_classes
{
public:

	equiv_classes(unsigned n);
	~equiv_classes() throw();

#if HAVE_0X_MOVE_CONSTRCUTOR_AND_ASSIGMENT_OPERATOR
	equiv_classes(equiv_classes&& eqcl);
	equiv_classes& operator=(equiv_classes&& eqcl);
#endif

	unsigned size() const;
	unsigned ngroups() const;
	unsigned count(unsigned group) const;
	unsigned get_group(unsigned a) const;

	void add_relation(unsigned a, unsigned b);

private:

	struct link
	{
		link();
		link(unsigned a);
		link(link* l, unsigned a);

		link* last() const;

		link*		next;
		unsigned	data;
	};

	typedef vector<link*> 		list;
	typedef vector<link*>  		storage;
	typedef vector<unsigned>	lookup;
	typedef vector<unsigned>	dimension;

	link* new_link(unsigned a);
	link* new_link(link* next, unsigned a);

	void prepare_groups() const;

	list		m_list;
	storage	m_storage;

	mutable lookup		m_lookup;
	mutable dimension	m_dimension;
	mutable unsigned	m_ngroups;
};

} // namespace mstl

#include "m_equiv_classes.ipp"

#endif // _mstl_equiv_classes_included

// vi:set ts=3 sw=3:
