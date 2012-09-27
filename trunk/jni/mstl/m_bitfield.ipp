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

#include "m_assert.h"
#include "m_bit_functions.h"
#include "m_limits.h"

namespace mstl {

template <class Bits>
inline
bitfield<Bits>::reference::reference(value_type& bits, value_type mask)
	:m_bits(bits)
	,m_mask(mask)
{
}


template <class Bits>
inline
typename bitfield<Bits>::reference&
bitfield<Bits>::reference::operator=(reference const& ref)
{
	return *this = bool(*this);
}


template <class Bits>
inline
bool
bitfield<Bits>::reference::operator==(reference const& ref) const
{
	return bool(*this) == bool(ref);
}


template <class Bits>
inline
bool
bitfield<Bits>::reference::operator<(reference const& ref) const
{
	return !bool(*this) && bool(ref);
}


template <class Bits>
inline
bitfield<Bits>::reference::operator bool () const
{
	return !!(m_bits & m_mask);
}


template <class Bits>
inline
bool
bitfield<Bits>::reference::operator!() const
{
	return !(m_bits & m_mask);
}


template <class Bits>
inline
bool
bitfield<Bits>::reference::operator~() const
{
	return !(m_bits & m_mask);
}


template <class Bits>
inline
typename bitfield<Bits>::reference&
bitfield<Bits>::reference::operator=(bool x)
{
	if (x)
		m_bits |= m_mask;
	else
		m_bits &= ~m_mask;

	return *this;
}


template <class Bits>
inline
typename bitfield<Bits>::reference&
bitfield<Bits>::reference::operator&=(bool x)
{
	if (!x)
		m_bits &= ~m_mask;

	return *this;
}


template <class Bits>
inline
typename bitfield<Bits>::reference&
bitfield<Bits>::reference::operator|=(bool x)
{
	if (x)
		m_bits |= m_mask;

	return *this;
}


template <class Bits>
inline
typename bitfield<Bits>::reference&
bitfield<Bits>::reference::operator^=(bool x)
{
	if (x)
		m_bits ^= m_mask;

	return *this;
}


template <class Bits>
inline
bitfield<Bits>::bitfield()
	:m_bits(0)
{
	static_assert(numeric_limits<Bits>::is_integer, "template parameter is not integer");
	static_assert(numeric_limits<Bits>::is_unsigned, "template parameter is not unsigned integer");
}


template <class Bits>
inline
bitfield<Bits>::bitfield(bool flag)
	:m_bits(flag ? ~value_type(0) : value_type(0))
{
}


template <class Bits>
inline
bitfield<Bits>::bitfield(value_type n)
	:m_bits(n)
{
	static_assert(numeric_limits<Bits>::is_integer, "template parameter is not integer");
	static_assert(numeric_limits<Bits>::is_unsigned, "template parameter is not unsigned integer");
}


template <class Bits>
inline
bitfield<Bits>::bitfield(unsigned from, unsigned to)
{
	static_assert(numeric_limits<Bits>::is_integer, "template parameter is not integer");
	static_assert(numeric_limits<Bits>::is_unsigned, "template parameter is not unsigned integer");

	set(from, to);
}


template <class Bits>
inline
bitfield<Bits>&
bitfield<Bits>::operator=(value_type value)
{
	m_bits = value;
	return *this;
}


template <class Bits>
inline
bitfield<Bits>
bitfield<Bits>::operator<<(unsigned n) const
{
	////M_REQUIRE(n < nbits);
	return bitfield(m_bits << n);
}


template <class Bits>
inline
bitfield<Bits>
bitfield<Bits>::operator>>(unsigned n) const
{
	////M_REQUIRE(n < nbits);
	return bitfield(Bits(m_bits >> n));
}


template <class Bits>
inline
bitfield<Bits>&
bitfield<Bits>::operator<<=(unsigned n)
{
	////M_REQUIRE(n < nbits);

	m_bits <<= n;
	return *this;
}


template <class Bits>
inline
bitfield<Bits>&
bitfield<Bits>::operator>>=(unsigned n)
{
	////M_REQUIRE(n < nbits);

	m_bits >>= n;
	return *this;
}


template <class Bits>
inline
bitfield<Bits>
bitfield<Bits>::operator&(bitfield const& bf) const
{
	return bitfield(m_bits & bf.m_bits);
}


template <class Bits>
inline
bitfield<Bits>
bitfield<Bits>::operator|(bitfield const& bf) const
{
	return bitfield(m_bits | bf.m_bits);
}


template <class Bits>
inline
bitfield<Bits>
bitfield<Bits>::operator^(bitfield const& bf) const
{
	return bitfield(m_bits ^ bf.m_bits);
}


template <class Bits>
inline
bitfield<Bits>
bitfield<Bits>::operator-(bitfield const& bf) const
{
	return bitfield(m_bits & ~bf.m_bits);
}


template <class Bits>
inline
bitfield<Bits>
bitfield<Bits>::operator~() const
{
	return bitfield(~m_bits);
}


template <class Bits>
inline
bitfield<Bits>&
bitfield<Bits>::operator&=(bitfield const& bf)
{
	m_bits &= bf.m_bits;
	return *this;
}


template <class Bits>
inline
bitfield<Bits>&
bitfield<Bits>::operator|=(bitfield const& bf)
{
	m_bits |= bf.m_bits;
	return *this;
}


template <class Bits>
inline
bitfield<Bits>&
bitfield<Bits>::operator^=(bitfield const& bf)
{
	m_bits ^= bf.m_bits;
	return *this;
}


template <class Bits>
inline
bitfield<Bits>&
bitfield<Bits>::operator-=(bitfield const& bf)
{
	m_bits &= ~bf.m_bits;
	return *this;
}


template <class Bits>
inline
Bits
bitfield<Bits>::mask(unsigned n)
{
	return value_type(1) << n;
}


template <class Bits>
inline
Bits
bitfield<Bits>::mask(unsigned from, unsigned to)
{
	return value_type((value_type(~0) << from) & (value_type(~0) >> ((nbits - 1) - to)));
}


template <class Bits>
inline
Bits
bitfield<Bits>::byte_mask(unsigned n)
{
	return value_type(0xff) << (n << 3);
}


template <class Bits>
inline
Bits
bitfield<Bits>::byte_mask(unsigned from, unsigned to)
{
	return mask(from ? (from << 3) - 1 : from, ((to + 1) << 3) - 1);
}


template <class Bits>
inline
typename bitfield<Bits>::reference
bitfield<Bits>::operator[](unsigned n)
{
	return reference(m_bits, mask(n));
}


template <class Bits>
inline
bool
bitfield<Bits>::operator[](unsigned n) const
{
	return m_bits & mask(n);
}


template <class Bits>
inline
Bits
bitfield<Bits>::value() const
{
	return m_bits;
}


template <class Bits>
inline
Bits&
bitfield<Bits>::value()
{
	return m_bits;
}


template <class Bits>
inline
void
bitfield<Bits>::set()
{
	m_bits = value_type(~0);
}


template <class Bits>
inline
void
bitfield<Bits>::set(unsigned n)
{
	m_bits |= mask(n);
}


template <class Bits>
inline
void
bitfield<Bits>::set(unsigned from, unsigned to)
{
	m_bits |= mask(from, to);
}


template <class Bits>
inline
bool
bitfield<Bits>::test_and_set(unsigned n)
{
	value_type m = mask(n);

	if ((m_bits & m) != 0)
		return true;

	m_bits |= m;
	return false;
}


template <class Bits>
inline
void
bitfield<Bits>::reset()
{
	m_bits = 0;
}


template <class Bits>
inline
void
bitfield<Bits>::reset(unsigned n)
{
	m_bits &= ~mask(n);
}


template <class Bits>
inline
void
bitfield<Bits>::reset(unsigned from, unsigned to)
{
	m_bits &= ~mask(from, to);
}


template <class Bits>
inline
void
bitfield<Bits>::put(bool value)
{
	value ? set() : reset();
}


template <class Bits>
inline
void
bitfield<Bits>::put(unsigned n, bool value)
{
	value ? set(n) : reset(n);
}


template <class Bits>
inline
void
bitfield<Bits>::put(unsigned from, unsigned to, bool value)
{
	value ? set(from, to) : reset(from, to);
}


template <class Bits>
inline
void
bitfield<Bits>::flip()
{
	m_bits ^= value_type(~0);
}


template <class Bits>
inline
void
bitfield<Bits>::flip(unsigned n)
{
	m_bits ^= mask(n);
}


template <class Bits>
inline
void
bitfield<Bits>::flip(unsigned from, unsigned to)
{
	m_bits ^= mask(from, to);
}


template <class Bits>
inline
unsigned
bitfield<Bits>::word_index(unsigned n)
{
	return n & (nbits - 1);
}


template <class Bits>
inline
bool
bitfield<Bits>::test(unsigned n) const
{
	return operator[](n);
}


template <class Bits>
inline
bool
bitfield<Bits>::none() const
{
	return !m_bits;
}


template <class Bits>
inline
bool
bitfield<Bits>::any() const
{
	return m_bits != 0;
}


template <class Bits>
inline
bool
bitfield<Bits>::complete() const
{
	return m_bits == value_type(~0);
}


template <class Bits>
inline
bool
bitfield<Bits>::contains(bitfield const& bf) const
{
	return bf.m_bits == (m_bits & bf.m_bits);
}


template <class Bits>
inline
bool
bitfield<Bits>::disjunctive(bitfield const& bf) const
{
	return !(bf.m_bits & m_bits);
}


template <class Bits>
inline
unsigned
bitfield<Bits>::count() const
{
	return bf::count_bits(m_bits);
}


template <class Bits>
inline
unsigned
bitfield<Bits>::count(unsigned start, unsigned end) const
{
	////M_REQUIRE(end < nbits);
	////M_REQUIRE(start <= end);

	return bf::count_bits(m_bits & mask(start, end));
}


template <class Bits>
inline
unsigned
bitfield<Bits>::find_last() const
{
	return m_bits == 0 ? npos : bf::msb_index(m_bits);
}


template <class Bits>
inline
unsigned
bitfield<Bits>::find_first() const
{
	return m_bits == 0 ? npos : bf::lsb_index(m_bits);
}


template <class Bits>
inline
unsigned
bitfield<Bits>::find_next(unsigned prev) const
{
	////M_REQUIRE(prev < nbits);
	return bitfield(value_type(m_bits & ~mask(0, prev))).find_first();
}


template <class Bits>
inline
unsigned
bitfield<Bits>::find_prev(unsigned next) const
{
	////M_REQUIRE(next < nbits);
	return bitfield(value_type(m_bits & ~mask(next, nbits - 1))).find_last();
}


template <class Bits>
inline
unsigned
bitfield<Bits>::find_last_not() const
{
	return m_bits == value_type(~0) ? npos : bf::msb_index(~m_bits);
}


template <class Bits>
inline
unsigned
bitfield<Bits>::find_first_not() const
{
	return m_bits == value_type(~0) ? npos : bf::lsb_index(~m_bits);
}


template <class Bits>
inline
unsigned
bitfield<Bits>::find_next_not(unsigned prev) const
{
	////M_REQUIRE(prev < nbits);
	return bitfield(value_type(m_bits | mask(0, prev))).find_first_not();
}


template <class Bits>
inline
unsigned
bitfield<Bits>::find_prev_not(unsigned next) const
{
	////M_REQUIRE(next < nbits);
	return bitfield(value_type(m_bits | mask(next, nbits - 1))).find_last_not();
}


template <class Bits>
inline
unsigned
bitfield<Bits>::index(unsigned nth) const
{
	////M_REQUIRE(nth <= count());

	unsigned n = find_first();

	while (nth--)
		n = find_next(n);

	return n;
}


template <class Bits>
inline
unsigned
bitfield<Bits>::rindex(unsigned nth) const
{
	////M_REQUIRE(nth <= count());

	unsigned n = find_last();

	while (nth--)
		n = find_prev(n);

	return n;
}


template <class Bits>
inline
void
bitfield<Bits>::increase(unsigned from, unsigned to)
{
	value_type m = mask(from, to);
	value_type v = m_bits & m;

	if (v == 0)
		m_bits |= 1 << from;
	else
		m_bits |= (v << 1) & m;
}


template <class Bits>
inline
bool
operator==(bitfield<Bits> const& lhs, bitfield<Bits> const& rhs)
{
	return lhs.value() == rhs.value();
}


template <class Bits>
inline
bool
operator==(Bits lhs, bitfield<Bits> const& rhs)
{
	return lhs == rhs.value();
}


template <class Bits>
inline
bool
operator==(bitfield<Bits> const& lhs, Bits rhs)
{
	return lhs.value() == rhs;
}


template <class Bits>
inline
bool
operator!=(bitfield<Bits> const& lhs, bitfield<Bits> const& rhs)
{
	return lhs.value() != rhs.value();
}


template <class Bits>
inline
bool
operator!=(Bits lhs, bitfield<Bits> const& rhs)
{
	return lhs != rhs.value();
}


template <class Bits>
inline
bool
operator!=(bitfield<Bits> const& lhs, Bits rhs)
{
	return lhs.value() != rhs;
}


template <class Bits>
inline
bool
operator<(bitfield<Bits> const& lhs, bitfield<Bits> const& rhs)
{
	return lhs.value() < rhs.value();
}


template <class Bits>
inline
bool
operator<(Bits lhs, bitfield<Bits> const& rhs)
{
	return lhs < rhs.value();
}


template <class Bits>
inline
bool
operator<(bitfield<Bits> const& lhs, Bits rhs)
{
	return lhs.value() < rhs;
}

} // namespace mstl

// vi:set ts=3 sw=3:
