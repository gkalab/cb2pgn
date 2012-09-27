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

#ifndef _mstl_bitset_iterator_included
#define _mstl_bitset_iterator_included

#include "m_bitfield.h"

namespace mstl {
namespace bits {

struct bitset_iterator
{
	// types
	typedef mstl::bitfield<unsigned long>	bitfield;
	typedef bitfield::reference				reference;
	typedef bitfield::value_type				value_type;
	typedef bitset_iterator						iterator;
	typedef unsigned								size_type;

	// structors
	bitset_iterator();
	bitset_iterator(bitfield* p);

	// comparators
	bool operator==(iterator const& it) const;
	bool operator!=(iterator const& it) const;
	bool operator< (iterator const& it) const;

	// accessors
	reference operator*() const;
	reference operator[](int n);

	// modifiers
	iterator& operator++();
	iterator  const operator++(int);
	iterator& operator--();
	iterator  const operator--(int);
	iterator& operator+=(int n);
	iterator& operator-=(int n);
	iterator  operator+ (int n) const;
	iterator  operator- (int n) const;

	void bump_up();
	void bump_down();

	// computation
	int operator-(iterator it) const;

	// attributes
	bitfield*	m_bf;
	size_type	m_offset;
};


struct bitset_const_iterator
{
	// types
	typedef mstl::bitfield<unsigned long>	bitfield;
	typedef bool									reference;
	typedef bitset_const_iterator				const_iterator;
	typedef unsigned								size_type;

	// structors
	bitset_const_iterator();
	bitset_const_iterator(bitfield* p);
	bitset_const_iterator(bitset_iterator const& it);

	// comparators
	bool operator==(const_iterator const& it) const;
	bool operator!=(const_iterator const& it) const;
	bool operator< (const_iterator const& it) const;

	// accessors
	reference operator*() const;
	reference operator[](int n);

	// modifiers
	const_iterator& operator++();
	const_iterator  operator++(int);
	const_iterator& operator--();
	const_iterator  operator--(int);
	const_iterator& operator+=(int n);
	const_iterator& operator-=(int n);
	const_iterator  operator+ (int n) const;
	const_iterator  operator- (int n) const;

	void bump_up();
	void bump_down();

	// computation
	int operator-(const_iterator it) const;

	// attributes
	bitfield*	m_bf;
	size_type	m_offset;
};

} // namespace bits
} // namespace mstl

#include "m_bitset_iterator.ipp"

#endif // _mstl_bitset_iterator_included

// vi:set ts=3 sw=3:
