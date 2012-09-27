// ======================================================================
// Author : $Author$
// Version: $Revision$
// Date   : $Date$
// Url    : $URL$
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

#ifndef _mstl_set_included
#define _mstl_set_included

#include "m_vector.h"

namespace mstl {

template <typename T>
class set
{
public:

	typedef vector<T> container_type;

	typedef typename container_type::value_type			key_type;
	typedef typename container_type::value_type			data_type;
	typedef typename container_type::value_type			value_type;
	typedef typename container_type::reference			reference;
	typedef typename container_type::const_reference	const_reference;
	typedef typename container_type::const_iterator		const_iterator;
	typedef typename container_type::iterator				iterator;
	typedef typename container_type::size_type			size_type;

	set();
	explicit set(size_type n);
	set(set const& v);
	set(const_iterator first, const_iterator last);

#if HAVE_0X_MOVE_CONSTRCUTOR_AND_ASSIGMENT_OPERATOR
	set(set&& v);
	set& operator=(set&& v);
#endif

	set& operator=(set const& v);

	bool operator==(set const& v) const;
	bool operator!=(set const& v) const;

	bool empty() const;
	bool contains(const_reference v) const;

	size_type size() const;
	container_type const& container() const;

	iterator begin();
	const_iterator begin() const;
	iterator end();
	const_iterator end() const;

	void assign(const_iterator first, const_iterator last);
	void push_back(const_reference v);
	iterator insert(const_reference v);
	void insert(const_iterator first, const_iterator last);
	bool insert_unique(const_reference v);
	void erase(const_reference v);
	iterator erase(iterator i);
	iterator erase(iterator first, iterator last);
	void reserve(size_type n);
	void clear();
	void swap(set& v);

	const_iterator find(const_reference v) const;
	iterator find(const_reference v);

private:

	container_type m_v;
};

template <typename T> void swap(set<T>& lhs, set<T>& rhs);

} // namespace mstl

#include "m_set.ipp"

#endif // _mstl_set_included

// vi:set ts=3 sw=3:
