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

namespace util {

inline ByteStream::ByRef::ByRef(ByteStream* strm) :ref(strm) {}

inline ByteStream::operator ByteStream::ByRef () { return ByRef(this); }

inline ByteStream::uint24_t::uint24_t(uint32_t n) : i(n) {}
inline ByteStream::uint24_t::operator uint32_t () const { return i; }
inline ByteStream::uint24_t& ByteStream::uint24_t::operator=(uint32_t n) { i = n; return *this; }
inline ByteStream::uint48_t::uint48_t(uint64_t n) : i(n) {}
inline ByteStream::uint48_t::operator uint64_t () const { return i; }
inline ByteStream::uint48_t& ByteStream::uint48_t::operator=(uint64_t n) { i = n; return *this; }

inline bool ByteStream::isEmpty() const			{ return m_getp == m_endp; }
inline bool ByteStream::isFull() const				{ return m_putp == m_endp; }
inline unsigned ByteStream::capacity() const		{ return m_endp - m_base; }
inline unsigned ByteStream::size() const			{ return m_endp - m_getp; }
inline unsigned ByteStream::remaining() const	{ return m_endp - m_getp; }
inline unsigned ByteStream::free() const			{ return m_endp - m_putp; }
inline unsigned ByteStream::tellp() const			{ return m_putp - m_base; }
inline unsigned ByteStream::tellg() const			{ return m_getp - m_base; }

inline ByteStream::Byte* ByteStream::base()					{ return m_base; }
inline ByteStream::Byte const* ByteStream::base() const	{ return m_base; }
inline ByteStream::Byte* ByteStream::data()					{ return m_getp; }
inline ByteStream::Byte const* ByteStream::data() const	{ return m_getp; }
inline ByteStream::Byte* ByteStream::buffer()				{ return m_putp; }
inline ByteStream::Byte* ByteStream::end()					{ return m_endp; }
inline ByteStream::Byte const* ByteStream::end() const	{ return m_endp; }

inline void ByteStream::reset(unsigned size)					{ m_endp = m_base + size; }

inline ByteStream& ByteStream::operator>>(uint16_t& i)	{ i = uint16(); return *this; }
inline ByteStream& ByteStream::operator>>(uint24_t& i)	{ i = uint24(); return *this; }
inline ByteStream& ByteStream::operator>>(uint48_t& i)	{ i = uint48(); return *this; }
inline ByteStream& ByteStream::operator>>(uint32_t& i)	{ i = uint32(); return *this; }
inline ByteStream& ByteStream::operator>>(uint64_t& i)	{ i = uint64(); return *this; }

inline void ByteStream::resetp() { m_putp = m_base; }
inline void ByteStream::resetg() { m_getp = m_base; }

inline uint32_t ByteStream::getUint8()		{ return uint8();  }
inline uint32_t ByteStream::getUint16()	{ return uint16(); }
inline uint32_t ByteStream::getUint24()	{ return uint24(); }
inline uint32_t ByteStream::getUint32()	{ return uint32(); }


inline
ByteStream::Byte
ByteStream::operator[](unsigned at) const
{
	//M_REQUIRE(at < capacity());
	return m_base[at];
}


inline
ByteStream::Byte&
ByteStream::operator[](unsigned at)
{
	//M_REQUIRE(at < capacity());
	return m_base[at];
}


inline
ByteStream::Byte
ByteStream::peek()
{
	if (__builtin_expect(m_getp == m_endp, 0))
		underflow(1);

	return *m_getp;
}


inline
ByteStream::Byte
ByteStream::get()
{
	if (__builtin_expect(m_getp == m_endp, 0))
		underflow(1);

	return *m_getp++;
}


inline
ByteStream::Byte
ByteStream::unsafeGet()
{
	//M_REQUIRE(data() < end());
	return *m_getp++;
}


inline
void
ByteStream::get(char* buf, unsigned size)
{
	get(reinterpret_cast<Byte*>(buf), size);
}


inline
void
ByteStream::put(char const* p, unsigned size)
{
	put(reinterpret_cast<Byte const*>(p), size);
}


inline
ByteStream& ByteStream::operator>>(uint8_t& i)
{
	i = get();
	return *this;
}


inline
uint8_t
ByteStream::uint8()
{
	return get();
}


inline
void
ByteStream::put(Byte c)
{
	if (__builtin_expect(m_putp == m_endp, 0))
		overflow(1);

	*m_putp++ = c;
}


inline
ByteStream&
ByteStream::operator<<(uint8_t i)
{
	put(i);
	return *this;
}


inline
void
ByteStream::advance(unsigned n)
{
	if (__builtin_expect(free() < n, 0))
		overflow(n);

	//M_REQUIRE(free() >= n);
	m_putp += n;
}


inline
void
ByteStream::skip(unsigned n)
{
	//M_REQUIRE(n <= capacity());

	if (__builtin_expect(remaining() < n, 0))
		underflow(n);

	//M_REQUIRE(remaining() >= n);
	m_getp += n;
}


inline
void
ByteStream::seekg(unsigned offset)
{
	//M_REQUIRE(offset <= capacity());
	m_getp = m_base + offset;
}


inline
void
ByteStream::seekp(unsigned offset)
{
	//M_REQUIRE(offset <= tellp());
	m_putp = m_base + offset;
}

#if HAVE_0X_MOVE_CONSTRCUTOR_AND_ASSIGMENT_OPERATOR

inline
ByteStream::ByteStream(ByteStream&& strm)
	:m_base(strm.m_base)
	,m_getp(strm.m_getp)
	,m_putp(strm.m_putp)
	,m_endp(strm.m_endp)
	,m_owner(strm.m_owner)
{
	strm.m_owner = false;
}


inline
ByteStream&
ByteStream::operator=(ByteStream&& strm)
{
	if (this != &strm)
	{
		mstl::swap(m_base, strm.m_base);
		m_getp = strm.m_getp;
		m_putp = strm.m_putp;
		m_endp = strm.m_endp;
		mstl::swap(m_owner, strm.m_owner);
	}

	return *this;
}

#endif

inline
uint16_t
ByteStream::uint16(Byte const* data)
{
	return (uint16_t(data[0]) << 8) | uint16_t(data[1]);
}


inline
uint16_t
ByteStream::uint16LE(Byte const* data)
{
	return (uint16_t(data[1]) << 8) | uint16_t(data[0]);
}


inline
uint32_t
ByteStream::uint24(Byte const* data)
{
	return (uint32_t(data[0]) << 16) | (uint32_t(data[1]) << 8) | uint32_t(data[2]);
}


inline
uint32_t
ByteStream::uint24LE(Byte const* data)
{
	return (uint32_t(data[2]) << 16) | (uint32_t(data[1]) << 8) | uint32_t(data[0]);
}


inline
uint32_t
ByteStream::uint32(Byte const* data)
{
	return	(uint32_t(data[0]) << 24)
			 | (uint32_t(data[1]) << 16)
			 | (uint32_t(data[2]) << 8)
			 | (uint32_t(data[3]));
}


inline
uint32_t
ByteStream::uint32LE(Byte const* data)
{
	return	(uint32_t(data[3]) << 24)
			 | (uint32_t(data[2]) << 16)
			 | (uint32_t(data[1]) << 8)
			 | (uint32_t(data[0]));
}


inline
uint64_t
ByteStream::uint48(Byte const* data)
{
	return  (uint64_t(data[0]) << 40)
			| (uint64_t(data[1]) << 32)
			| (uint64_t(data[2]) << 24)
			| (uint64_t(data[3]) << 16)
			| (uint64_t(data[4]) << 8)
			| (uint64_t(data[5]));
}


inline
uint64_t
ByteStream::uint64(Byte const* data)
{
	return  (uint64_t(data[0]) << 56)
			| (uint64_t(data[1]) << 48)
			| (uint64_t(data[2]) << 40)
			| (uint64_t(data[3]) << 32)
			| (uint64_t(data[4]) << 24)
			| (uint64_t(data[5]) << 16)
			| (uint64_t(data[6]) << 8)
			| (uint64_t(data[7]));
}


inline
void
ByteStream::set(Byte* data, uint16_t i)
{
	data[0] = i >> 8;
	data[1] = i;
}


inline
void
ByteStream::set(Byte* data, uint24_t i)
{
	data[0] = i >> 16;
	data[1] = i >> 8;
	data[2] = i;
}


inline
void
ByteStream::set(Byte* data, uint32_t i)
{
	data[0] = i >> 24;
	data[1] = i >> 16;
	data[2] = i >> 8;
	data[3] = i;
}


inline
void
ByteStream::set(Byte* data, uint48_t i)
{
	data[0] = i >> 40;
	data[1] = i >> 32;
	data[2] = i >> 24;
	data[3] = i >> 16;
	data[4] = i >> 8;
	data[5] = i;
}


inline
void
ByteStream::set(Byte* data, uint64_t i)
{
	data[0] = i >> 56;
	data[1] = i >> 48;
	data[2] = i >> 40;
	data[3] = i >> 32;
	data[4] = i >> 24;
	data[5] = i >> 16;
	data[6] = i >> 8;
	data[7] = i;
}


inline
void
ByteStream::setup(char* buf, char* end)
{
	return setup(reinterpret_cast<Byte*>(buf), reinterpret_cast<Byte*>(end));
}


inline
void
ByteStream::setup(char* buf, unsigned size)
{
	return setup(reinterpret_cast<Byte*>(buf), size);
}

} // namespace db

// vi:set ts=3 sw=3:
