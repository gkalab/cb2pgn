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

#ifndef _mstl_stack_included
#define _mstl_stack_included

#include "m_memblock.h"

namespace mstl {

template <typename T>
class stack : protected memblock<T>
{
public:

	typedef T						value_type;
	typedef value_type*			pointer;
	typedef value_type const*	const_pointer;
	typedef pointer				iterator;
	typedef const_pointer		const_iterator;
	typedef value_type&			reference;
	typedef value_type const&	const_reference;
	typedef bits::size_t			size_type;

	stack();
	explicit stack(size_type n);
	stack(size_type n, const_reference v);
	stack(stack const& v);
	~stack() throw();

#if HAVE_0X_MOVE_CONSTRCUTOR_AND_ASSIGMENT_OPERATOR
	stack(stack&& v);
	stack& operator=(stack&& v);
#endif

	stack& operator=(stack const& v);

	bool empty() const;

	size_type size() const;
	size_type capacity() const;

	reference operator[](size_type n);
	const_reference operator[](size_type n) const;

	reference bottom();
	const_reference bottom() const;
	reference top();
	const_reference top() const;

	iterator begin();
	const_iterator begin() const;
	iterator end();
	const_iterator end() const;

	void push(const_reference value);
	void dup();
	void push();
	void pop();

	void reserve(size_type n);
	void clear();
	void swap(stack& v);
};

template <typename T> void swap(stack<T>& lhs, stack<T>& rhs);

} // namespace mstl

#include "m_stack.ipp"

#endif // _mstl_stack_included

// vi:set ts=3 sw=3:
