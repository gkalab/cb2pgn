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

#include "m_utility.h"
#include "m_assert.h"

#include <string.h>

namespace mstl {

inline void swap(bitset& lhs, bitset& rhs) { lhs.swap(rhs); }


inline
bitset::enumerator::enumerator(bitset const* bset)
   :m_bs(bset)
   ,m_pos(bset->find_first())
{
}


inline
bitset::enumerator::enumerator()
	:m_bs(0)
	,m_pos(bitset::npos)
{
}


inline
bool
bitset::enumerator::operator==(enumerator const& it) const
{
	return m_pos == it.m_pos;
}


inline
bool
bitset::enumerator::operator!=(enumerator const& it) const
{
	return m_pos != it.m_pos;
}


inline
bool
bitset::enumerator::operator<(enumerator const& it) const
{
	return m_pos < it.m_pos;
}


inline
bool
bitset::enumerator::operator<=(enumerator const& it) const
{
	return m_pos <= it.m_pos;
}


inline
bool
bitset::enumerator::operator>(enumerator const& it) const
{
	return m_pos > it.m_pos;
}


inline
bool
bitset::enumerator::operator>=(enumerator const& it) const
{
	return m_pos >= it.m_pos;
}


inline
bitset::enumerator&
bitset::enumerator::operator++()
{
	if (m_bs)
		m_pos = m_bs->find_next(m_pos);

	return *this;
}


inline
bitset::enumerator
bitset::enumerator::operator++(int)
{
	enumerator tmp = *this;
	++*this;
	return tmp;
}


inline
bitset::enumerator&
bitset::enumerator::operator--()
{
	if (m_bs)
		m_pos = m_bs->find_prev(m_pos);

	return *this;
}


inline
bitset::enumerator
bitset::enumerator::operator--(int)
{
	enumerator tmp = *this;
	--*this;
	return tmp;
}


inline
bitset::enumerator&
bitset::enumerator::operator+=(int n)
{
	if (m_bs)
		m_pos = m_bs->find_next(m_pos, n);

	return *this;
}


inline
bitset::enumerator&
bitset::enumerator::operator-=(int n)
{
	if (m_bs)
		m_pos = m_bs->find_prev(m_pos, n);

	return *this;
}


inline
bitset::enumerator
bitset::enumerator::operator+(int n) const
{
	return enumerator(*this) += n;
}


inline
bitset::enumerator
bitset::enumerator::operator-(int n) const
{
	return enumerator(*this) -= n;
}


inline
bitset::size_type
bitset::enumerator::operator*() const
{
	return m_pos;
}


inline
bitset::size_type const*
bitset::enumerator::operator->() const
{
	return &m_pos;
}


inline
bitset const*
bitset::enumerator::set() const
{
	return m_bs;
}


inline
bool
bitset::operator==(bitset const& bset) const
{
	return ordering(bset) == 0;
}


inline
bool
bitset::operator!=(bitset const& bset) const
{
	return ordering(bset) != 0;
}


inline
bool
bitset::operator<(bitset const& bset) const
{
	return ordering(bset) < 0;
}


inline
bool
bitset::operator<=(bitset const& bset) const
{
	return ordering(bset) <= 0;
}


inline
bool
bitset::operator>(bitset const& bset) const
{
	return ordering(bset) > 0;
}


inline
bool
bitset::operator>=(bitset const& bset) const
{
	return ordering(bset) >= 0;
}


inline
bitset::size_type
bitset::word_index(size_type n)
{
	return n >> ((bitfield::nbits + 128)>>5);	// 64 bit safe
}


inline
bool
bitset::test(size_type n) const
{
	//M_REQUIRE(!compressed());
	//M_REQUIRE(n < size());

	return static_cast<bitfield const&>(m_bits[word_index(n)])[bitfield::word_index(n)];
}


inline
bool
bitset::operator[](size_type n) const
{
	//M_REQUIRE(!compressed());
	return test(n);
}


inline
bitset::reference
bitset::operator[](size_type n)
{
	//M_REQUIRE(!compressed());
	//M_REQUIRE(n < size());

	return m_bits[word_index(n)][bitfield::word_index(n)];
}


inline
bitset
bitset::operator&(bitset const& bset) const
{
	return bitset(*this) &= bset;
}


inline
bitset
bitset::operator|(bitset const& bset) const
{
	return bitset(*this) |= bset;
}


inline
bitset
bitset::operator-(bitset const& bset) const
{
	return bitset(*this) -= bset;
}


inline
bitset
bitset::operator^(bitset const& bset) const
{
	return bitset(*this) ^= bset;
}


inline
bitset
bitset::operator<<(size_type n)
{
	return bitset(*this) <<= n;
}


inline
bitset
bitset::operator>>(size_type n)
{
	return bitset(*this) >>= n;
}


inline
bool
bitset::empty() const
{
	return m_size == 0;
}


inline
bool
bitset::compressed() const
{
	return m_words == 0 && m_bits != 0;
}


inline
bitset::size_type
bitset::count_words(size_type nbits)
{
	return nbits ? word_index(nbits - 1) + 1 : 0;
}


inline
bitset::size_type
bitset::count_words() const
{
	return count_words(m_size);
}


inline
bitset::size_type
bitset::capacity() const
{
	return count_words()*bitfield::nbits;
}


inline
bitset::size_type
bitset::size() const
{
	return m_size;
}


inline
void
bitset::fill(size_type first, size_type last, unsigned char value)
{
	//M_ASSERT(first <= last);
	::memset(m_bits + first, value, sizeof(m_bits[0])*(last - first));
}


inline
void
bitset::set(size_type n)
{
	//M_REQUIRE(!compressed());
	//M_REQUIRE(n < size());

	m_bits[word_index(n)].set(bitfield::word_index(n));
}


inline
void
bitset::set()
{
	//M_REQUIRE(!compressed());

	if (m_size > 0)
		set(0, m_size - 1);
}


inline
void
bitset::reset(size_type n)
{
	//M_REQUIRE(!compressed());
	//M_REQUIRE(n < size());

	m_bits[word_index(n)].reset(bitfield::word_index(n));
}


inline
void
bitset::reset()
{
	//M_REQUIRE(!compressed());

	if (m_size > 0)
		reset(0, m_size - 1);
}


inline
void
bitset::put(bool value)
{
	value ? set() : reset();
}


inline
void
bitset::put(size_type n, bool value)
{
	value ? set(n) : reset(n);
}


inline
void
bitset::put(size_type from, size_type to, bool value)
{
	value ? set(from) : reset(to);
}


inline
void
bitset::flip(size_type n)
{
	//M_REQUIRE(!compressed());
	//M_REQUIRE(n < size());

	m_bits[word_index(n)].flip(bitfield::word_index(n));
}


inline
void
bitset::flip()
{
	//M_REQUIRE(!compressed());

	if (m_size > 0)
		flip(0, m_size - 1);
}


inline
bitset
bitset::operator~() const
{
	//M_REQUIRE(!compressed());

	bitset bs(*this);
	bs.flip();
	return bs;
}


inline
bitset::iterator
bitset::begin()
{
	//M_REQUIRE(!compressed());
	return iterator(m_bits);
}


inline
bitset::iterator
bitset::end()
{
	//M_REQUIRE(!compressed());
	return iterator(m_bits + count_words());
}


inline
bitset::const_iterator
bitset::begin() const
{
	//M_REQUIRE(!compressed());
	return const_iterator(m_bits);
}


inline
bitset::const_iterator
bitset::end() const
{
	//M_REQUIRE(!compressed());
	return const_iterator(m_bits + count_words());
}


inline
bitset::enumerator
bitset::begin_index() const
{
	//M_REQUIRE(!compressed());
	return enumerator(this);
}


inline
bitset::enumerator
bitset::end_index() const
{
	//M_REQUIRE(!compressed());
	return enumerator();
}


inline
bitset::size_type
bitset::count_bytes(size_type nwords)
{
	return nwords*bitfield::nbytes;
}


inline
bitset::size_type
bitset::count_bytes() const
{
	return count_bytes(count_words());
}


inline
bitset::size_type
bitset::bytes_used() const
{
	return (m_size + 7) >> 3;
}


inline
unsigned char
bitset::byte(size_type n) const
{
	//M_REQUIRE(!compressed());
	//M_REQUIRE(n < count_bytes());

	return reinterpret_cast<unsigned char const*>(m_bits)[n];
}


inline
bitset::bitfield
bitset::word(size_type n) const
{
	//M_REQUIRE(!compressed());
	//M_REQUIRE(n < count_words());

	return m_bits[n];
}


inline
bitset::reference
bitset::front()
{
	//M_REQUIRE(!compressed());
	//M_REQUIRE(!empty());

	return operator[](0);
}


inline
bitset::const_reference
bitset::front() const
{
	//M_REQUIRE(!compressed());
	//M_REQUIRE(!empty());

	return operator[](0);
}


inline
bitset::reference
bitset::back()
{
	//M_REQUIRE(!compressed());
	//M_REQUIRE(!empty());

	return operator[](m_size - 1);
}


inline
bitset::const_reference
bitset::back() const
{
	//M_REQUIRE(!compressed());
	//M_REQUIRE(!empty());

	return operator[](m_size - 1);
}


inline
void
bitset::push(bool b)
{
	//M_REQUIRE(!compressed());

	resize(m_size + 1);
	operator[](m_size) = b;
}


inline
void
bitset::pop()
{
	//M_REQUIRE(!compressed());
	//M_REQUIRE(!empty());

	resize(m_size - 1);
}


inline
bool
bitset::test_and_set(size_type n)
{
	//M_REQUIRE(!compressed());
	//M_REQUIRE(n < size());

	return m_bits[word_index(n)].test_and_set(bitfield::word_index(n));
}


inline
void
bitset::clear()
{
	resize(0);
}


#if HAVE_0X_MOVE_CONSTRCUTOR_AND_ASSIGMENT_OPERATOR

inline
bitset::bitset(bitset&& bset)
	:m_size(bset.m_size)
	,m_words(bset.m_words)
	,m_bits(bset.m_bits)
{
	bset.m_bits = 0;
}


inline
bitset&
bitset::operator=(bitset&& bset)
{
	if (this != &bset)
	{
		m_size = bset.m_size;
		m_words = bset.m_words;
		mstl::swap(m_bits, bset.m_bits);
	}

	return *this;
}

#endif

} // namespace mstl

// vi:set ts=3 sw=3:
