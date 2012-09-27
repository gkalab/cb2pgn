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

#ifndef _mstl_bitset_included
#define _mstl_bitset_included

#include "m_bitset_iterator.h"
#include "m_types.h"

namespace mstl {

class byte_buf;

class bitset
{
public:

	// types
	typedef bits::bitset_iterator			iterator;
	typedef bits::bitset_const_iterator	const_iterator;
	typedef iterator::reference			reference;
	typedef const_iterator::reference	const_reference;
	typedef iterator::bitfield				bitfield;
	typedef iterator::value_type			value_type;
	typedef iterator::size_type			size_type;

	// nested classes
	class enumerator
	{
	public:

		// structors
		enumerator();

		// comparators
		bool operator==(enumerator const& it) const;
		bool operator!=(enumerator const& it) const;
		bool operator< (enumerator const& it) const;
		bool operator<=(enumerator const& it) const;
		bool operator> (enumerator const& it) const;
		bool operator>=(enumerator const& it) const;

		// modifiers
		enumerator& operator++();
		enumerator  operator++(int);
		enumerator& operator--();
		enumerator  operator--(int);
		enumerator& operator+=(int n);
		enumerator& operator-=(int n);
		enumerator  operator+ (int n) const;
		enumerator  operator- (int n) const;

		// computation
		int operator-(enumerator it) const;

		// accessors
		size_type operator*() const;
		size_type const* operator->() const;

		bitset const* set() const;

	private:

		// structors
		enumerator(bitset const* bset);

		// attributes
		bitset const*	m_bs;
		size_type		m_pos;

		// friends
		friend class bitset;
	};

	// constants
	static size_type const npos = size_type(-1);

	// structors
	explicit bitset(size_type nbits = 0, bool on = false);
	bitset(bitset const& bset);
	bitset(bitfield const* data, size_type n);
	bitset(value_type const* bits, size_type n);
	~bitset();

#if HAVE_0X_MOVE_CONSTRCUTOR_AND_ASSIGMENT_OPERATOR
	bitset(bitset&& bset);
	bitset& operator=(bitset&& bset);
#endif

	// assignment
	bitset& operator=(bitset const& bset);

	// comparators
	bool operator==(bitset const& bset) const;
	bool operator!=(bitset const& bset) const;
	bool operator< (bitset const& bset) const;
	bool operator<=(bitset const& bset) const;
	bool operator> (bitset const& bset) const;
	bool operator>=(bitset const& bset) const;

	// accessors
	reference operator[](size_type n);
	const_reference operator[](size_type n) const;

	reference front();
	const_reference front() const;
	reference back();
	const_reference back() const;

	// operators
	bitset operator&(bitset const& bset) const;
	bitset operator|(bitset const& bset) const;
	bitset operator-(bitset const& bset) const;
	bitset operator^(bitset const& bset) const;
	bitset operator<<(size_type n);
	bitset operator>>(size_type n);
	bitset operator~() const;

	bitset& operator&=(bitset const& bset);
	bitset& operator|=(bitset const& bset);
	bitset& operator-=(bitset const& bset);
	bitset& operator^=(bitset const& bset);
	bitset& operator<<=(size_type n);
	bitset& operator>>=(size_type n);

	// queries
	bool empty() const;
	bool none() const;
	bool any() const;
	bool complete() const;
	bool test(size_type n) const;
	bool contains(bitset const& bset) const;
	bool extends(bitset const& bset) const;
	bool disjunctive(bitset const& bset) const;
	bool compressed() const;

	// accessors
	size_type capacity() const;
	size_type size() const;

	// find
	size_type find_last() const;
	size_type find_prev(size_type next) const;
	size_type find_prev(size_type next, size_type skip) const;
	size_type find_first() const;
	size_type find_next(size_type prev) const;
	size_type find_next(size_type prev, size_type skip) const;
	size_type find_last_not() const;
	size_type find_prev_not(size_type next) const;
	size_type find_prev_not(size_type next, size_type skip) const;
	size_type find_first_not() const;
	size_type find_next_not(size_type prev) const;
	size_type find_next_not(size_type prev, size_type skip) const;

	// ordering
	int ordering(bitset const& bset) const;

	// modifiers
	bool test_and_set(size_type n);

	void set();
	void set(size_type n);
	void set(size_type from, size_type to);

	void reset();
	void reset(size_type n);
	void reset(size_type from, size_type to);

	void put(bool value);
	void put(size_type n, bool value);
	void put(size_type from, size_type to, bool value);

	void flip();
	void flip(size_type n);
	void flip(size_type from, size_type to);

	void push(bool b);
	void pop();
	void resize(size_type nbits);
	void swap(bitset& bset);
	void clear();

	void assign(bitfield const* data, size_type n);
	void assign(value_type const* bits, size_type n);

	// accessors
	unsigned char byte(size_type n) const;
	bitfield word(size_type n) const;

	iterator begin();
	const_iterator begin() const;
	iterator end();
	const_iterator end() const;
	enumerator begin_index() const;
	enumerator end_index() const;

	// computation
	size_type count_bytes() const;
	size_type count_words() const;
	size_type bytes_used() const;
	size_type count() const;
	size_type count(size_type start, size_type end) const;
	size_type index(size_type nth) const;
	size_type rindex(size_type nth) const;

	// compression
	void compress();
	void uncompress();

	// helpers
	static size_type count_words(size_type nbits);
	static size_type word_index(size_type n);

private:

	// modifiers
	void fill(size_type first, size_type last, unsigned char value);
	void reset_unused();

	//  helpers
	static size_type count_bytes(size_type nwords);

	// attributes
	size_type	m_size;
	size_type	m_words;
	bitfield*	m_bits;
};

void swap(bitset& lhs, bitset& rhs);

} // namespace mstl

#include "m_bitset.ipp"

#endif // _mstl_bitset_included

// vi:set ts=3 sw=3:
