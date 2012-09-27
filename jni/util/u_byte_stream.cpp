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

#include "u_byte_stream.h"
#include "u_exception.h"

#include "m_utility.h"
#include "m_string.h"
#include "m_assert.h"

#include <string.h>

using namespace util;


ByteStream::UnexpectedEndOfStreamException::UnexpectedEndOfStreamException()
	:Exception("unexpected end of stream")
{
}


ByteStream::ByteStream(ByteStream& strm)
	:m_base(strm.m_base)
	,m_getp(strm.m_getp)
	,m_putp(strm.m_putp)
	,m_endp(strm.m_endp)
	,m_owner(strm.m_owner)
{
	strm.m_owner = false;
}


ByteStream::ByteStream(ByRef ref)
	:m_base(ref.ref->m_base)
	,m_getp(ref.ref->m_getp)
	,m_putp(ref.ref->m_putp)
	,m_endp(ref.ref->m_endp)
	,m_owner(ref.ref->m_owner)
{
	if (m_owner)
		ref.ref->m_base = ref.ref->m_getp = ref.ref->m_putp = ref.ref->m_endp = 0;

	ref.ref->m_owner = false;
}


ByteStream::ByteStream(Byte* buf, unsigned size)
	:m_base(buf)
	,m_getp(buf)
	,m_putp(buf)
	,m_endp(buf + size)
	,m_owner(false)
{
}


ByteStream::ByteStream(Byte* first, Byte* last)
	:m_base(first)
	,m_getp(first)
	,m_putp(first)
	,m_endp(last)
	,m_owner(false)
{
}


ByteStream::ByteStream(char* buf, unsigned size)
	:m_base(reinterpret_cast<Byte*>(buf))
	,m_getp(reinterpret_cast<Byte*>(buf))
	,m_putp(reinterpret_cast<Byte*>(buf))
	,m_endp(reinterpret_cast<Byte*>(buf) + size)
	,m_owner(false)
{
}


ByteStream::ByteStream(char* first, char* last)
	:m_base(reinterpret_cast<Byte*>(first))
	,m_getp(reinterpret_cast<Byte*>(first))
	,m_putp(reinterpret_cast<Byte*>(first))
	,m_endp(reinterpret_cast<Byte*>(last))
	,m_owner(false)
{
}


ByteStream::ByteStream(unsigned size)
	:m_base(new Byte[mstl::max(1u, size)])
	,m_getp(m_base)
	,m_putp(m_base)
	,m_endp(m_base + size)
	,m_owner(true)
{
	//M_REQUIRE(size > 0);
}


ByteStream::ByteStream()
	:m_base(0)
	,m_getp(0)
	,m_putp(0)
	,m_endp(0)
	,m_owner(true)
{
}


ByteStream::~ByteStream() throw()
{
	if (m_owner)
		delete [] m_base;
}


ByteStream&
ByteStream::operator=(ByRef ref)
{
	if (this != ref.ref)
	{
		m_base = ref.ref->m_base;
		m_getp = ref.ref->m_getp;
		m_putp = ref.ref->m_putp;
		m_endp = ref.ref->m_endp;
		m_owner = ref.ref->m_owner;

		if (m_owner)
			ref.ref->m_base = ref.ref->m_putp = ref.ref->m_getp = ref.ref->m_endp = 0;

		ref.ref->m_owner = false;
	}

	return *this;
}


void
ByteStream::overflow(unsigned size)
{
	//M_ASSERT(size > free());
	reserve(mstl::max(512u, this->capacity() + mstl::max(size - free(), this->capacity())));
	//M_ASSERT(size <= free());
}


void
ByteStream::underflow(unsigned)
{
	//M_THROW(UnexpectedEndOfStreamException());
}


void
ByteStream::reserve(unsigned size)
{
	if (size <= capacity())
		return;

	unsigned ppos = tellp();
	unsigned gpos = tellg();

	Byte* buf = new Byte[size];
	::memcpy(buf, m_base, mstl::max(ppos, gpos));

	if (m_owner)
		delete [] m_base;

	m_owner = true;
	m_base = buf;
	m_putp = m_base + ppos;
	m_getp = m_base + gpos;
	m_endp = m_base + size;
}


void
ByteStream::provide()
{
	m_getp = m_base;
	m_endp = m_putp;
}


void
ByteStream::provide(unsigned size)
{
	//M_REQUIRE(size <= free());

	m_getp = m_putp;
	m_endp = m_getp + size;
}


void
ByteStream::swap(ByteStream& strm)
{
	mstl::swap(m_base, strm.m_base);
	mstl::swap(m_putp, strm.m_putp);
	mstl::swap(m_getp, strm.m_getp);
	mstl::swap(m_endp, strm.m_endp);
	mstl::swap(m_owner, strm.m_owner);
}


void
ByteStream::fetch(unsigned size)
{
	//M_REQUIRE(size <= capacity());

	if (remaining() < size)
		underflow(size);
}


void
ByteStream::get(Byte* buf, unsigned size)
{
	//M_REQUIRE(size <= capacity());

	if (remaining() < size)
		underflow(size);

	//M_ASSERT(remaining() >= size);
	::memcpy(buf, m_getp, size);
	m_getp += size;
}


ByteStream::Byte*
ByteStream::searchEos()
{
	Byte* p = m_getp;

	while (true)
	{
		for ( ; p < m_endp ; ++p)
		{
			if (*p == 0)
				return p;
		}

		underflow(1);
	}

	return 0; // satisifies the compiler
}


void
ByteStream::get(mstl::string& buf)
{
	Byte* eos = searchEos();
	buf.append(reinterpret_cast<char const*>(m_getp), reinterpret_cast<char const*>(eos));
	m_getp = eos + 1;
}


void
ByteStream::get(mstl::string& buf, unsigned size)
{
	//M_REQUIRE(size <= capacity());

	size_t oldSize = buf.size();
	buf.resize(oldSize + size);
	get(buf.data() + oldSize, size);
}


void
ByteStream::skipString()
{
	while (true)
	{
		if (m_getp == m_endp)
			underflow(1);

		if (*m_getp++ == '\0')
			return;
	}
}


void
ByteStream::flush()
{
	// no action
}


uint16_t
ByteStream::uint16()
{
	skip(2);
	return (uint16_t(m_getp[-2]) << 8) | uint16_t(m_getp[-1]);
}


uint16_t
ByteStream::uint16LE()
{
	skip(2);
	return (uint16_t(m_getp[-1]) << 8) | uint16_t(m_getp[-2]);
}


uint32_t
ByteStream::uint24()
{
	skip(3);
	return (uint32_t(m_getp[-3]) << 16) | (uint32_t(m_getp[-2]) << 8) | uint32_t(m_getp[-1]);
}


uint32_t
ByteStream::uint24LE()
{
	skip(3);
	return (uint32_t(m_getp[-1]) << 16) | (uint32_t(m_getp[-2]) << 8) | uint32_t(m_getp[-3]);
}


uint32_t
ByteStream::uint32()
{
	skip(4);
	return	(uint32_t(m_getp[-4]) << 24)
			 | (uint32_t(m_getp[-3]) << 16)
			 | (uint32_t(m_getp[-2]) << 8)
			 | (uint32_t(m_getp[-1]));
}


uint32_t
ByteStream::uint32LE()
{
	skip(4);
	return	(uint32_t(m_getp[-1]) << 24)
			 | (uint32_t(m_getp[-2]) << 16)
			 | (uint32_t(m_getp[-3]) << 8)
			 | (uint32_t(m_getp[-4]));
}


uint64_t
ByteStream::uint48()
{
	skip(6);
	return  (uint64_t(m_getp[-6]) << 40)
			| (uint64_t(m_getp[-5]) << 32)
			| (uint64_t(m_getp[-4]) << 24)
			| (uint64_t(m_getp[-3]) << 16)
			| (uint64_t(m_getp[-2]) << 8)
			| (uint64_t(m_getp[-1]));
}


uint64_t
ByteStream::uint64()
{
	skip(8);
	return  (uint64_t(m_getp[-8]) << 56)
			| (uint64_t(m_getp[-7]) << 48)
			| (uint64_t(m_getp[-6]) << 40)
			| (uint64_t(m_getp[-5]) << 32)
			| (uint64_t(m_getp[-4]) << 24)
			| (uint64_t(m_getp[-3]) << 16)
			| (uint64_t(m_getp[-2]) << 8)
			| (uint64_t(m_getp[-1]));
}


void
ByteStream::put(Byte const* p, unsigned size)
{
	if (free() < size)
		overflow(size);

	//M_ASSERT(free() >= size);
	::memcpy(m_putp, p, size);
	m_putp += size;
}


void
ByteStream::put(mstl::string const& s)
{
	put(s, s.size() + 1);
}


ByteStream&
ByteStream::operator<<(uint16_t i)
{
	advance(2);
	m_putp[-2] = i >> 8;
	m_putp[-1] = i;

	return *this;
}


ByteStream&
ByteStream::operator<<(uint24_t i)
{
	advance(3);
	m_putp[-3] = i >> 16;
	m_putp[-2] = i >> 8;
	m_putp[-1] = i;

	return *this;
}


ByteStream&
ByteStream::operator<<(uint32_t i)
{
	advance(4);
	m_putp[-4] = i >> 24;
	m_putp[-3] = i >> 16;
	m_putp[-2] = i >> 8;
	m_putp[-1] = i;

	return *this;
}


ByteStream&
ByteStream::operator<<(uint48_t i)
{
	advance(6);
	m_putp[-6] = i >> 40;
	m_putp[-5] = i >> 32;
	m_putp[-4] = i >> 24;
	m_putp[-3] = i >> 16;
	m_putp[-2] = i >> 8;
	m_putp[-1] = i;

	return *this;
}


ByteStream&
ByteStream::operator<<(uint64_t i)
{
	advance(8);
	m_putp[-8] = i >> 56;
	m_putp[-7] = i >> 48;
	m_putp[-6] = i >> 40;
	m_putp[-5] = i >> 32;
	m_putp[-4] = i >> 24;
	m_putp[-3] = i >> 16;
	m_putp[-2] = i >> 8;
	m_putp[-1] = i;

	return *this;
}


void
ByteStream::setup(Byte* buf, Byte* end)
{
	if (m_owner)
		delete [] m_base;

	m_base = m_getp = m_putp = buf;
	m_endp = end;
	m_owner = false;
}


void
ByteStream::setup(Byte* buf, unsigned size)
{
	if (m_owner)
		delete [] m_base;

	m_base = m_getp = m_putp = buf;
	m_endp = buf + size;
	m_owner = false;
}

// vi:set ts=3 sw=3:
