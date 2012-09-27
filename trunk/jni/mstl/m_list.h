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

#ifndef _mstl_list_included
#define _mstl_list_included

#include "m_vector.h"
#include "m_list_node.h"

namespace mstl {

template <typename T>
class list
{
public:

	typedef T						value_type;
	typedef vector<T*>			vector_type;
	typedef list<T>				list_type;
	typedef value_type*			pointer;
	typedef value_type const*	const_pointer;
	typedef value_type&			reference;
	typedef value_type const&	const_reference;
	typedef ptrdiff_t				difference_type;
	typedef bits::size_t			size_type;

	class iterator
	{
	public:

		typedef ptrdiff_t	difference_type;
		typedef T			value_type;
		typedef T*			pointer;
		typedef T&			reference;

		iterator();
		explicit iterator(bits::node_base* n);

		bool operator==(iterator const& i) const;
		bool operator!=(iterator const& i) const;

		reference operator*() const;
		pointer operator->() const;

		iterator& operator++();
		iterator  operator++(int);
		iterator& operator--();
		iterator  operator--(int);

		iterator& operator+=(int n);
		iterator& operator-=(int n);
		iterator  operator+ (int n) const;
		iterator  operator- (int n) const;

		bits::node_base* base() const;

	private:

		void advance(int n);

		bits::node_base* m_node;
	};

	class const_iterator
	{
	public:

		typedef ptrdiff_t	difference_type;
		typedef T			value_type;
		typedef T const*	pointer;
		typedef T const&	reference;

		const_iterator();
		explicit const_iterator(bits::node_base const* n);
		const_iterator(iterator const& i);

		const_iterator& operator=(const_iterator const& i);
		const_iterator& operator=(iterator const& i);

		bool operator==(const_iterator const& i) const;
		bool operator!=(const_iterator const& i) const;

		reference operator*() const;
		pointer operator->() const;

		const_iterator& operator++();
		const_iterator  operator++(int);
		const_iterator& operator--();
		const_iterator  operator--(int);

		const_iterator& operator+=(int n);
		const_iterator& operator-=(int n);
		const_iterator  operator+ (int n) const;
		const_iterator  operator- (int n) const;

	private:

		void advance(int n);

		bits::node_base const* m_node;
	};

	list();
	list(size_type n);
	list(size_type n, const_reference v);
	list(list const& v);
	~list();

#if HAVE_0X_MOVE_CONSTRCUTOR_AND_ASSIGMENT_OPERATOR
	list(list&& v);
	list& operator=(list&& v);
#endif

	list& operator=(list const& v);

#if 0
	reference operator[](size_type n);
	const_reference operator[](size_type n) const;
#endif

	reference front();
	const_reference front() const;
	reference back();
	const_reference back() const;

	bool empty() const;

	size_type size() const;
	size_type capacity() const;

	iterator begin();
	const_iterator begin() const;
	iterator end();
	const_iterator end() const;

	void push_back(const_reference v);
	void push_back();
	void pop_back();

	iterator insert(iterator i, const_reference value);
	void insert(iterator i, size_type n, const_reference value);
	void insert(iterator pos, const_iterator first, const_iterator last);
	iterator erase(iterator i);
	iterator erase(iterator first, iterator last);

	void resize(size_type n);
	void resize(size_type n, const_reference v);
	void clear();
	void swap(list& v);

private:

	struct node : public bits::node_base
	{
		T m_data;
	};

	void init();

	node* create_node(T const& x);
	void erase(node* n);

	node			m_node;
	size_type	m_size;
};

template <typename T> void swap(list<T>& lhs, list<T>& rhs);

} // namespace mstl

#include "m_list.ipp"

#endif // _mstl_list_included

// vi:set ts=3 sw=3:
