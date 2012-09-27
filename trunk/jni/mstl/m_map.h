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

#ifndef _mstl_map_included
#define _mstl_map_included

#include "m_vector.h"
#include "m_pair.h"

namespace mstl {

template <typename K, typename V>
class map
{
public:

	typedef K						key_type;
	typedef V						mapped_type;
	typedef K const&				const_key_ref;
	typedef V const&				const_data_ref;
	typedef pair<K,V>				value_type;
	typedef vector<value_type>	container_type;

	typedef typename container_type::size_type					size_type;
	typedef typename container_type::difference_type			difference_type;
	typedef typename container_type::pointer						pointer;
	typedef typename container_type::const_pointer				const_pointer;
	typedef typename container_type::reference					reference;
	typedef typename container_type::const_reference			const_reference;
	typedef typename container_type::const_iterator				const_iterator;
	typedef typename container_type::iterator						iterator;
	typedef typename container_type::reverse_iterator			reverse_iterator;
	typedef typename container_type::const_reverse_iterator	const_reverse_iterator;

	typedef pair<iterator,bool> result_t;

	map();
	explicit map(size_type n);
	map(map const& v);
	map(const_iterator first, const_iterator last);
	map const& operator=(map const& v);

#if HAVE_0X_MOVE_CONSTRCUTOR_AND_ASSIGMENT_OPERATOR
	map(map&& m);
	map& operator=(map&& m);
#endif

	bool operator==(map const& v) const;
	bool operator!=(map const& v) const;

	const_data_ref operator[](const_key_ref i) const;
	mapped_type& operator[](const_key_ref i);

	bool empty() const;

	size_type size() const;
	size_type capacity() const;
	container_type const& container() const;

	iterator begin();
	const_iterator	begin() const;
	iterator end();
	const_iterator	end() const;
	reverse_iterator rbegin();
	reverse_iterator rend();
	const_reverse_iterator rbegin() const;
	const_reverse_iterator rend() const;

	void assign(const_iterator first, const_iterator last);
	result_t insert(const_reference v);
	void insert(const_iterator first, const_iterator last);
	iterator replace(const_reference v);
	void erase(const_key_ref k);
	iterator erase(iterator ep);
	iterator erase(iterator first, iterator last);
	void reserve(size_type n);
	void clear();
	void swap(map& m);

	const_iterator find(const_key_ref k) const;
	iterator find(const_key_ref k);
	const_iterator find_data(const_data_ref v, const_iterator first = 0, const_iterator last = 0) const;
	iterator find_data(const_data_ref v, iterator first = 0, iterator last = 0);

private:

	iterator lower_bound(const_key_ref k);

	container_type m_v;
};

template <typename K, typename V> void swap(map<K,V>& lhs, map<K,V>& rhs);

} // namespace mstl

#include "m_map.ipp"

#endif // _mstl_map_included

// vi:set ts=3 sw=3:
