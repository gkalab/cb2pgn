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

namespace mstl {
namespace bits {

inline
bitset_iterator::bitset_iterator()
	:m_bf(0)
	,m_offset(0)
{
}


inline
bitset_iterator::bitset_iterator(bitfield* p)
	:m_bf(p)
	,m_offset(0)
{
}


inline
void
bitset_const_iterator::bump_up()
{
	if (++m_offset == bitfield::nbits)
	{
		m_offset = 0;
		++m_bf;
	}
}


inline
void
bitset_const_iterator::bump_down()
{
	//M_REQUIRE(m_offset > 0);

	if (m_offset-- == 0)
	{
		m_offset = bitfield::nbits - 1;
		--m_bf;
	}
}


inline
bitset_iterator::reference
bitset_iterator::operator*() const
{
	return reference(m_bf->value(), value_type(1) << m_offset);
}


inline
int
bitset_iterator::operator-(iterator it) const
{
	return bitfield::nbits*(m_bf - it.m_bf) + m_offset - it.m_offset;
}


inline
bool
bitset_iterator::operator==(iterator const& it) const
{
	return m_bf == it.m_bf && m_offset == it.m_offset;
}


inline
bool
bitset_iterator::operator!=(iterator const& it) const
{
	return m_bf != it.m_bf || m_offset != it.m_offset;
}


inline
bool
bitset_iterator::operator<(iterator const& it) const
{
	return m_bf < it.m_bf || (m_bf == it.m_bf && m_offset < it.m_offset);
}


inline
void
bitset_iterator::bump_up()
{
	if (++m_offset == bitfield::nbits)
	{
		m_offset = 0;
		++m_bf;
	}
}


inline
void
bitset_iterator::bump_down()
{
	//M_REQUIRE(m_offset > 0);

	if (m_offset-- == 0)
	{
		m_offset = bitfield::nbits - 1;
		--m_bf;
	}
}


inline
bitset_iterator::iterator&
bitset_iterator::operator++()
{
	bump_up();
	return *this;
}


inline
bitset_iterator::iterator const
bitset_iterator::operator++(int)
{
	iterator tmp = *this;
	++*this;
	return tmp;
}


inline
bitset_iterator::iterator&
bitset_iterator::operator--()
{
	bump_down();
	return *this;
}


inline
bitset_iterator::iterator const
bitset_iterator::operator--(int)
{
	iterator tmp = *this;
	--*this;
	return tmp;
}


inline
bitset_iterator::iterator&
bitset_iterator::operator-=(int n)
{
	return *this += -n;
}


inline
bitset_iterator::iterator
bitset_iterator::operator+(int n) const
{
	return iterator(*this) += n;
}


inline
bitset_iterator::iterator
bitset_iterator::operator-(int n) const
{
	return iterator(*this) += -n;
}


inline
bitset_iterator::reference
bitset_iterator::operator[](int n)
{
	return *(*this + n);
}


inline
bitset_const_iterator::bitset_const_iterator()
	:m_bf(0)
	,m_offset(0)
{
}


inline
bitset_const_iterator::bitset_const_iterator(bitfield* p)
	:m_bf(p)
	,m_offset(0)
{
}


inline
bitset_const_iterator::bitset_const_iterator(bitset_iterator const& it)
	:m_bf(it.m_bf)
	,m_offset(it.m_offset)
{
}


inline
bitset_const_iterator::reference
bitset_const_iterator::operator*() const
{
	return m_bf->test(m_offset);
}


inline
bitset_const_iterator::const_iterator&
bitset_const_iterator::operator++()
{
	bump_up();
	return *this;
}


inline
bitset_const_iterator::const_iterator
bitset_const_iterator::operator++(int)
{
	const_iterator tmp = *this;
	bump_up();
	return tmp;
}


inline
bitset_const_iterator::const_iterator&
bitset_const_iterator::operator--()
{
	bump_down();
	return *this;
}


inline
bitset_const_iterator::const_iterator
bitset_const_iterator::operator--(int)
{
	const_iterator tmp = *this;
	bump_down();
	return tmp;
}


inline
bitset_const_iterator::const_iterator&
bitset_const_iterator::operator-=(int n)
{
	return *this += -n;
}


inline
bitset_const_iterator::const_iterator
bitset_const_iterator::operator+(int n) const
{
	return const_iterator(*this) += n;
}


inline
bitset_const_iterator::const_iterator
bitset_const_iterator::operator-(int n) const
{
	return const_iterator(*this) += -n;
}


inline
int
bitset_const_iterator::operator-(const_iterator it) const
{
	return bitfield::nbits*(m_bf - it.m_bf) + m_offset - it.m_offset;
}


inline
bitset_const_iterator::reference
bitset_const_iterator::operator[](int n)
{
	return *(*this + n);
}


inline
bool
bitset_const_iterator::operator==(const_iterator const& it) const
{
	return m_bf == it.m_bf && m_offset == it.m_offset;
}


inline
bool
bitset_const_iterator::operator!=(const_iterator const& it) const
{
	return m_bf != it.m_bf || m_offset != it.m_offset;
}


inline
bool
bitset_const_iterator::operator<(const_iterator const& it) const
{
	return m_bf < it.m_bf || (m_bf == it.m_bf && m_offset < it.m_offset);
}

} // namespace bits
} // namespace mstl

// vi:set ts=3 sw=3:
