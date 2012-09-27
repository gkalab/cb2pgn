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

#ifndef _u_byte_stream_included
#define _u_byte_stream_included

#include "u_base.h"
#include "u_exception.h"

namespace mstl { class string; }

namespace util {

class ByteStream
{
public:

	struct UnexpectedEndOfStreamException : public Exception
	{
		UnexpectedEndOfStreamException();
	};

	typedef unsigned char Byte;

	struct uint24_t
	{
		uint24_t(uint32_t n);
		uint24_t& operator=(uint32_t n);
		operator uint32_t () const;
		uint32_t i;
	};

	struct uint48_t
	{
		uint48_t(uint64_t n);
		uint48_t& operator=(uint64_t n);
		operator uint64_t () const;
		uint64_t i;
	};

	ByteStream();
	ByteStream(Byte* buf, unsigned size);
	ByteStream(Byte* first, Byte* last);
	ByteStream(char* buf, unsigned size);
	ByteStream(char* first, char* last);
	explicit ByteStream(unsigned size);
	ByteStream(ByteStream& strm);
	virtual ~ByteStream() throw();

#if HAVE_0X_MOVE_CONSTRCUTOR_AND_ASSIGMENT_OPERATOR
	ByteStream(ByteStream&& strm);
	ByteStream& operator=(ByteStream&& strm);
#endif

	bool isEmpty() const;
	bool isFull() const;

	unsigned capacity() const;
	unsigned size() const;
	unsigned remaining() const;
	unsigned free() const;
	unsigned tellp() const;
	unsigned tellg() const;

	Byte operator[](unsigned at) const;
	Byte& operator[](unsigned at);

	ByteStream& operator>>(uint8_t& i);
	ByteStream& operator>>(uint16_t& i);
	ByteStream& operator>>(uint24_t& i);
	ByteStream& operator>>(uint32_t& i);
	ByteStream& operator>>(uint48_t& i);
	ByteStream& operator>>(uint64_t& i);

	ByteStream& operator<<(uint8_t i);
	ByteStream& operator<<(uint16_t i);
	ByteStream& operator<<(uint24_t i);
	ByteStream& operator<<(uint32_t i);
	ByteStream& operator<<(uint48_t i);
	ByteStream& operator<<(uint64_t i);

	Byte peek();

	Byte get();
	Byte unsafeGet();

	void get(Byte* buf, unsigned size);
	void get(char* buf, unsigned size);
	void get(mstl::string& buf);
	void get(mstl::string& buf, unsigned size);

	void put(Byte c);
	void put(Byte const* p, unsigned size);
	void put(char const* p, unsigned size);
	void put(mstl::string const& s);

	void fetch(unsigned size);

	virtual void flush();

	Byte* base();
	Byte const* base() const;
	Byte* buffer();
	Byte* data();
	Byte const* data() const;
	Byte* end();
	Byte const* end() const;

	uint8_t  uint8();
	uint16_t uint16();
	uint32_t uint24();
	uint32_t uint32();
	uint64_t uint48();
	uint64_t uint64();

	uint16_t uint16LE();
	uint32_t uint24LE();
	uint32_t uint32LE();

	uint32_t getUint8();
	uint32_t getUint16();
	uint32_t getUint24();
	uint32_t getUint32();

	void resetp();
	void resetg();
	void skip(unsigned n);
	void advance(unsigned n);
	void skipString();
	void seekg(unsigned offset);
	void seekp(unsigned offset);

	void provide();
	void provide(unsigned size);
	void reset(unsigned size);

	void setup(Byte* buf, Byte* end);
	void setup(Byte* buf, unsigned size);
	void setup(char* buf, char* end);
	void setup(char* buf, unsigned size);
	void reserve(unsigned size);
	void swap(ByteStream& strm);

	static uint16_t uint16(Byte const* data);
	static uint32_t uint24(Byte const* data);
	static uint32_t uint32(Byte const* data);
	static uint64_t uint48(Byte const* data);
	static uint64_t uint64(Byte const* data);

	static uint16_t uint16LE(Byte const* data);
	static uint32_t uint24LE(Byte const* data);
	static uint32_t uint32LE(Byte const* data);

	static void set(Byte* data, uint16_t i);
	static void set(Byte* data, uint24_t i);
	static void set(Byte* data, uint32_t i);
	static void set(Byte* data, uint48_t i);
	static void set(Byte* data, uint64_t i);

protected:

	virtual void underflow(unsigned size);
	virtual void overflow(unsigned size);

	Byte*	m_base;
	Byte* m_getp;
	Byte* m_putp;
	Byte* m_endp;
	bool	m_owner;

private:

	struct ByRef
	{
		ByRef(ByteStream* strm);
		ByteStream* ref;
	};

	friend class ByRef;

	ByteStream& operator=(ByteStream const&);

	Byte* searchEos();

public:

	ByteStream(ByRef ref);
	ByteStream& operator=(ByRef ref);
	operator ByRef ();
};

} // namespace util

#include "u_byte_stream.ipp"

#endif // _u_byte_stream_included

// vi:set ts=3 sw=3:
