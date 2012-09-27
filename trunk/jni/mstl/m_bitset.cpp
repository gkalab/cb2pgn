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

#include "m_bitset.h"
#include "m_assert.h"
#include "m_uninitialized.h"
#include "m_byte_buf.h"
#include "m_utility.h"

#include <string.h>

using namespace mstl;

template <typename T> inline static int ordering(T a, T b) { return a < b ? -1 : (b < a ? 1 : 0); }


int
bitset::enumerator::operator-(enumerator it) const
{
	////M_REQUIRE(set() == it.set());

	if (m_pos > it.m_pos) return -int(m_bs->count(it.m_pos + 1, m_pos));
	if (m_pos < it.m_pos) return m_bs->count(m_pos + 1, it.m_pos);
	return 0;
}


bitset::bitset(size_type nbits, bool on)
	:m_size(0)
	,m_words(0)
	,m_bits(0)
{
	resize(nbits);

	if (on)
		set();
}


bitset::bitset(bitfield const* data, size_type n)
	:m_size(0)
	,m_words(0)
	,m_bits(0)
{
	assign(data, n);
}


bitset::bitset(value_type const* data, size_type n)
	:m_size(0)
	,m_words(0)
	,m_bits(0)
{
	assign(data, n);
}


bitset::bitset(bitset const& bset)
	:m_size(0)
	,m_words(0)
	,m_bits(0)
{
	if (bset.m_bits)
	{
		if (bset.compressed())
		{
			m_size = bset.m_size;
			m_bits = reinterpret_cast<bitfield*>(new byte_buf(*reinterpret_cast<byte_buf*>(bset.m_bits)));
		}
		else
		{
			resize(bset.m_size);
			mstl::uninitialized_copy(bset.m_bits, bset.m_bits + bset.count_words(), m_bits);
			reset_unused();
		}
	}
}


bitset::~bitset()
{
	if (compressed())
		delete reinterpret_cast<byte_buf*>(m_bits);
	else
		delete [] m_bits;
}


void
bitset::reset_unused()
{
	if (m_bits > 0 && bitfield::word_index(m_size))
		m_bits[count_words() - 1].reset(bitfield::word_index(m_size), bitfield::nbits - 1);
}


void
bitset::assign(bitfield const* data, size_type n)
{
	////M_REQUIRE(!compressed());

	resize(n*bitfield::nbits);
	mstl::uninitialized_copy(data, data + n, m_bits);
}


void
bitset::assign(value_type const* bits, size_type n)
{
	////M_REQUIRE(!compressed());

	resize(n*bitfield::nbits);

	value_type const* e = bits + n;

	for (bitfield* p = m_bits; bits < e; ++bits, ++p)
		*p = *bits;
}


bitset&
bitset::operator=(bitset const& bset)
{
	////M_REQUIRE(!compressed());
	////M_REQUIRE(!bset.compressed());

	if (this != &bset)
	{
		bitset tmp(bset);
		tmp.swap(*this);
	}

	return *this;
}


bitset&
bitset::operator&=(bitset const& bset)
{
	////M_REQUIRE(!compressed());
	////M_REQUIRE(!bset.compressed());
	////M_REQUIRE(size() == bset.size());

	size_type nwords = count_words();

	for (size_type i = 0; i < nwords; ++i)
		m_bits[i] &= bset.m_bits[i];

	return *this;
}


bitset&
bitset::operator|=(bitset const& bset)
{
	////M_REQUIRE(!compressed());
	////M_REQUIRE(!bset.compressed());
	////M_REQUIRE(size() == bset.size());

	size_type nwords = count_words();

	for (size_type i = 0; i < nwords; ++i)
		m_bits[i] |= bset.m_bits[i];

	return *this;
}


bitset&
bitset::operator^=(bitset const& bset)
{
	////M_REQUIRE(!compressed());
	////M_REQUIRE(!bset.compressed());
	////M_REQUIRE(size() == bset.size());

	size_type nwords = count_words();

	for (size_type i = 0; i < nwords; ++i)
		m_bits[i] ^= bset.m_bits[i];

	return *this;
}


bitset&
bitset::operator-=(bitset const& bset)
{
	////M_REQUIRE(!compressed());
	////M_REQUIRE(!bset.compressed());
	////M_REQUIRE(size() == bset.size());

	size_type nwords = count_words();

	for (size_type i = 0; i < nwords; ++i)
		m_bits[i] -= bset.m_bits[i];

	return *this;
}


bitset&
bitset::operator<<=(size_type n)
{
	////M_REQUIRE(!compressed());

	if (m_bits == 0)
		return *this;

	if (n >= m_size)
	{
		reset();
	}
	else if (n > 0)
	{
		int nf = count_words();
		int bi = bitfield::word_index(n);
		int wi = count_words(n);

		if (bi == 0)
		{
			::memmove(m_bits + wi, m_bits, (nf - wi)*bitfield::nbytes);
		}
		else
		{
			--wi;

			for (int i = nf - 1; i > wi; --i)
				m_bits[i] = (m_bits[i - wi] << bi) | (m_bits[i - wi - 1] >> (bitfield::nbits - bi));

			m_bits[wi] = m_bits[0] << bi;
		}

		reset(0, n - 1);
		reset_unused();
	}

	return *this;
}


bitset&
bitset::operator>>=(size_type n)
{
	////M_REQUIRE(!compressed());

	if (n >= m_size)
	{
		reset();
	}
	else if (n > 0)
	{
		int nf = count_words();
		int bi = bitfield::word_index(n);
		int wi = count_words(n);
		int li = nf - wi;

		if (bi == 0)
		{
			::memmove(m_bits, m_bits + wi, li*bitfield::nbytes);
		}
		else
		{
			--wi;

			for (int i = 0; i < li; ++i)
				m_bits[i] = (m_bits[i + wi] >> bi) | (m_bits[i + wi + 1] << bitfield::nbits - bi);

			m_bits[li] = m_bits[nf - 1] >> bi;
		}

		reset(m_size - n, m_size - 1);
	}

	return *this;
}


bool
bitset::none() const
{
	////M_REQUIRE(!compressed());

	size_type nwords = count_words();

	for (size_type i = 0; i < nwords; ++i)
	{
		if (m_bits[i].any())
			return false;
	}

	return true;
}


bool
bitset::any() const
{
	////M_REQUIRE(!compressed());

	size_type nwords = count_words();

	for (size_type i = 0; i < nwords; ++i)
	{
		if (m_bits[i].any())
			return true;
	}

	return false;
}


bool
bitset::complete() const
{
	////M_REQUIRE(!compressed());

	size_type nwords = count_words();

	if (nwords)
	{
		for (size_type i = 0; i < nwords - 1; ++i)
		{
			if (!m_bits[i].complete())
				return false;
		}

		if (m_bits[nwords - 1] != bitfield::mask(0, bitfield::word_index(m_size - 1)))
			return false;
	}

	return true;
}


bool
bitset::contains(bitset const& bset) const
{
	////M_REQUIRE(!compressed());
	////M_REQUIRE(!bset.compressed());
	////M_REQUIRE(size() == bset.size());

	size_type nwords = count_words();

	for (size_type i = 0; i < nwords; ++i)
	{
		if (!m_bits[i].contains(bset.m_bits[i]))
			return false;
	}

	return true;
}


bool
bitset::extends(bitset const& bset) const
{
	////M_REQUIRE(!compressed());
	////M_REQUIRE(!bset.compressed());
	////M_REQUIRE(size() == bset.size());

	size_type nwords = count_words();
	bool extension = false;

	for (size_type i = 0; i < nwords; ++i)
	{
		if (!m_bits[i].contains(bset.m_bits[i]))
			return false;

		if (m_bits[i] != bset.m_bits[i])
			extension = true;
	}

	return extension;
}


bool
bitset::disjunctive(bitset const& bset) const
{
	////M_REQUIRE(!compressed());
	////M_REQUIRE(!bset.compressed());
	////M_REQUIRE(size() == bset.size());

	size_type nwords = count_words();

	for (size_type i = 0; i < nwords; ++i)
	{
		if (!m_bits[i].disjunctive(bset.m_bits[i]))
			return false;
	}

	return true;
}


int
bitset::ordering(bitset const& bset) const
{
	////M_REQUIRE(!compressed());
	////M_REQUIRE(!bset.compressed());
	////M_REQUIRE(size() == bset.size());

	size_type nwords = count_words();

	for (size_type i = 0; i < nwords; ++i)
	{
		int result = ::ordering(m_bits[i], bset.m_bits[i]);

		if (result)
			return result;
	}

	return 0;
}


bitset::size_type
bitset::count() const
{
	////M_REQUIRE(!compressed());

	size_type nwords = count_words();
	size_type result = 0;

	for (size_type i = 0; i < nwords; ++i)
		result += m_bits[i].count();

	return result;
}


bitset::size_type
bitset::count(size_type start, size_type end) const
{
	////M_REQUIRE(!compressed());
	////M_REQUIRE(end < size());

	if (start > end)
		return 0;

	size_type ws = word_index(start);
	size_type we = word_index(end);

	if (ws == we)
		return m_bits[ws].count(bitfield::word_index(start), bitfield::word_index(end));

	size_type result = m_bits[ws++].count(bitfield::word_index(start), bitfield::nbits - 1);

	for ( ; ws < we; ws++)
		result += m_bits[ws].count();

	return result + m_bits[we].count(0, bitfield::word_index(end));
}


bitset::size_type
bitset::index(size_type nth) const
{
	////M_REQUIRE(!compressed());

	size_type nwords = count_words();

	for (size_type i = 0; i < nwords; ++i)
	{
		size_type count = m_bits[i].count();

		if (count > nth)
			return i*bitfield::nbits + m_bits[i].index(nth);

		nth -= count;
	}

	////M_REQUIRE(nth < count());	// delayed check (because it's expensive)
	//M_RAISE("invalid argument");

	return 0;	// satisfies the compiler
}


bitset::size_type
bitset::rindex(size_type nth) const
{
	////M_REQUIRE(!compressed());

	int nwords = count_words();

	for (int i = nwords - 1; i >= 0; --i)
	{
		size_type count = m_bits[i].count();

		if (count > nth)
			return i*bitfield::nbits + m_bits[i].rindex(nth);

		nth -= count;
	}

	////M_REQUIRE(nth < count());	// delayed check (because it's expensive)
	//M_RAISE("invalid argument");

	return 0;	// satisfies the compiler
}


bitset::size_type
bitset::find_first() const
{
	////M_REQUIRE(!compressed());

	size_type nwords = count_words();

	for (size_type i = 0; i < nwords; ++i)
	{
		if (m_bits[i].any())
			return (bitfield::nbits*i) + m_bits[i].find_first();
	}

	return npos;
}


bitset::size_type
bitset::find_last() const
{
	////M_REQUIRE(!compressed());

	size_type nwords = count_words();

	for (int i = nwords - 1; i >= 0; --i)
	{
		if (m_bits[i].any())
			return (bitfield::nbits*i) + m_bits[i].find_last();
	}

	return npos;
}


bitset::size_type
bitset::find_next(size_type prev) const
{
	////M_REQUIRE(!compressed());
	////M_REQUIRE(prev < size());

	size_type i = word_index(prev);
	size_type k = m_bits[i].find_next(bitfield::word_index(prev));

	if (k == npos)
	{
		size_type nwords = count_words();

		do
		{
			if (++i == nwords)
				return npos;
		}
		while ((k = m_bits[i].find_first()) == npos);
	}

	return k + (i << ((bitfield::nbits + 128)>>5));	// 64 bit safe
}


bitset::size_type
bitset::find_prev(size_type next) const
{
	////M_REQUIRE(!compressed());
	////M_REQUIRE(next < size());

	size_type i = word_index(next);
	size_type k = m_bits[i].find_prev(bitfield::word_index(next));

	if (k == npos)
	{
		do
		{
			if (i-- == 0)
				return npos;
		}
		while ((k = m_bits[i].find_last()) == npos);
	}

	return k + (i << ((bitfield::nbits + 128)>>5));	// 64 bit safe
}


bitset::size_type
bitset::find_next(size_type prev, size_type skip) const
{
	////M_REQUIRE(!compressed());
	////M_REQUIRE(prev < size());

	while (skip--)
		prev = find_next(prev);

	return prev;
}


bitset::size_type
bitset::find_prev(size_type next, size_type skip) const
{
	////M_REQUIRE(!compressed());
	////M_REQUIRE(next < size());

	while (skip--)
		next = find_prev(next);

	return next;
}


bitset::size_type
bitset::find_last_not() const
{
	////M_REQUIRE(!compressed());

	size_type nwords = count_words();

	for (int i = nwords - 1; i >= 0; --i)
	{
		if (!m_bits[i].complete())
			return (bitfield::nbits*i) + m_bits[i].find_last_not();
	}

	return npos;
}


bitset::size_type
bitset::find_first_not() const
{
	////M_REQUIRE(!compressed());

	size_type nwords = count_words();

	for (size_type i = 0; i < nwords; ++i)
	{
		if (!m_bits[i].complete())
			return (bitfield::nbits*i) + m_bits[i].find_first_not();
	}

	return npos;
}


bitset::size_type
bitset::find_prev_not(size_type next) const
{
	////M_REQUIRE(!compressed());
	////M_REQUIRE(next < size());

	size_type i = word_index(next);
	size_type k = m_bits[i].find_prev_not(bitfield::word_index(next));

	if (k == npos)
	{
		do
		{
			if (i-- == 0)
				return npos;
		}
		while ((k = m_bits[i].find_last_not()) == npos);
	}

	return k + (i << ((bitfield::nbits + 128)>>5));	// 64 bit safe
}


bitset::size_type
bitset::find_prev_not(size_type next, size_type skip) const
{
	////M_REQUIRE(!compressed());
	////M_REQUIRE(next < size());

	while (skip--)
		next = find_prev_not(next);

	return next;
}


bitset::size_type
bitset::find_next_not(size_type prev) const
{
	////M_REQUIRE(!compressed());
	////M_REQUIRE(prev < size());

	size_type i = word_index(prev);
	size_type k = m_bits[i].find_next_not(bitfield::word_index(prev));

	if (k == npos)
	{
		size_type nwords = count_words();

		do
		{
			if (++i == nwords)
				return npos;
		}
		while ((k = m_bits[i].find_first_not()) == npos);
	}

	return k + (i << ((bitfield::nbits + 128)>>5));	// 64 bit safe
}


void
bitset::resize(size_type nbits)
{
	////M_REQUIRE(!compressed());

	size_type new_word_count = count_words(nbits);
	size_type old_word_count = count_words();

	if (m_words < new_word_count)
	{
		bitfield* bits = new bitfield[new_word_count];
		mstl::uninitialized_copy(m_bits, m_bits + mstl::min(old_word_count, new_word_count), bits);
		mstl::swap(m_bits, bits);
		fill(m_words + (bitfield::word_index(nbits) ? 1 : 0), new_word_count, 0);
		m_words = new_word_count;
		delete [] bits;
	}
	else if (new_word_count == 0)
	{
		delete [] m_bits;
		m_bits = 0;
		m_words = 0;
	}

	m_size = nbits;
	reset_unused();
}


void
bitset::set(size_type from, size_type to)
{
	////M_REQUIRE(!compressed());
	////M_REQUIRE(from < size());
	////M_REQUIRE(to < size());
	////M_REQUIRE(from <= to);

	size_type first	= word_index(from);
	size_type last		= word_index(to);

	if (first == last)
	{
		m_bits[first].set(bitfield::word_index(from), bitfield::word_index(to));
	}
	else
	{
		m_bits[first].set(bitfield::word_index(from), bitfield::nbits - 1);

		if (last > first)
		{
			fill(first + 1, last, 0xff);
			m_bits[last].set(0, bitfield::word_index(to));
		}
	}
}


void
bitset::reset(size_type from, size_type to)
{
	////M_REQUIRE(!compressed());
	////M_REQUIRE(from < size());
	////M_REQUIRE(to < size());

	size_type first	= word_index(from);
	size_type last		= word_index(to);

	if (first == last)
	{
		m_bits[first].reset(bitfield::word_index(from), bitfield::word_index(to));
	}
	else
	{
		m_bits[first].reset(bitfield::word_index(from), bitfield::nbits - 1);

		if (last > first)
		{
			fill(first + 1, last, 0);
			m_bits[last].reset(0, bitfield::word_index(to));
		}
	}
}


void
bitset::flip(size_type from, size_type to)
{
	////M_REQUIRE(!compressed());
	////M_REQUIRE(from < size());
	////M_REQUIRE(to < size());

	size_type first	= word_index(from);
	size_type last		= word_index(to);

	if (first == last)
	{
		m_bits[first].flip(bitfield::word_index(from), bitfield::word_index(to));
	}
	else
	{
		m_bits[first].flip(bitfield::word_index(from), bitfield::nbits - 1);

		if (last > first)
		{
			for (size_type i = first + 1; i < last; ++i)
				m_bits[i].flip();

			m_bits[last].flip(0, bitfield::word_index(to));
		}
	}
}


void
bitset::swap(bitset& bset)
{
	mstl::swap(m_size, bset.m_size);
	mstl::swap(m_words, bset.m_words);
	mstl::swap(m_bits, bset.m_bits);
}


void
bitset::compress()
{
	if (m_words == 0)
		return;

	byte_buf* buf = byte_buf::compress(
							count_bytes(),
							reinterpret_cast<byte_buf::value_type const*>(m_bits));
	delete [] m_bits;
	m_bits = reinterpret_cast<bitfield*>(buf);
	m_words = 0;
}


void
bitset::uncompress()
{
	if (!compressed())
		return;

	//M_ASSERT(m_words == 0);

	byte_buf* buf = reinterpret_cast<byte_buf*>(m_bits);

	m_words = count_words(m_size);
	m_bits = new bitfield[m_words];
	buf->uncompress(reinterpret_cast<byte_buf::byte*>(m_bits), count_bytes());
	delete buf;
}

// vi:set ts=3 sw=3:
