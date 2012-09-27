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

#include "m_ostream.h"
#include "m_string.h"
#include "m_stdio.h"
#include "m_assert.h"
#include <stdlib.h>
#include <string.h>

using namespace mstl;


ostream::~ostream() throw()
{
// NOTE: too late!
//	if (m_fp)
//		fflush(m_fp);
}


ostream&
ostream::operator<<(int16_t n)
{
	char buf[20];
	return write(buf, ::sprintf(buf, "%hd", static_cast<short>(n)));
}


ostream&
ostream::operator<<(int32_t n)
{
	char buf[20];
	return write(buf, ::sprintf(buf, "%d", static_cast<int>(n)));
}


ostream&
ostream::operator<<(int64_t n)
{
	char buf[20];
	return write(buf, ::sprintf(buf, "%lld", static_cast<long long>(n)));
}


ostream&
ostream::operator<<(uint16_t n)
{
	char buf[20];
	return write(buf, ::sprintf(buf, "%hu", static_cast<unsigned short>(n)));
}


ostream&
ostream::operator<<(uint32_t n)
{
	char buf[20];
	return write(buf, ::sprintf(buf, "%u", static_cast<unsigned>(n)));
}


ostream&
ostream::operator<<(uint64_t n)
{
	char buf[20];
	return write(buf, ::sprintf(buf, "%llu", static_cast<unsigned long long>(n)));
}


ostream&
ostream::operator<<(void const* p)
{
	char buf[20];
	return write(buf, ::sprintf(buf, "%p", p));
}


ostream&
ostream::write(char const* buffer, size_t size)
{
	if (fwrite(buffer, 1, size, m_fp) == 0)
	{
		if (ferror(m_fp))
			setstate(badbit);
		else if (feof(m_fp))
			setstate(eofbit | failbit);
	}

	return *this;
}


ostream&
ostream::write(string const& str)
{
	return write(str.c_str(), str.size());
}


ostream&
ostream::writenl(string const& str)
{
	if (write(str))
		put('\n');
	return *this;
}


int
ostream::vformat(char const* fmt, va_list args)
{
	//M_REQUIRE(fmt);

	char buffer[2048];

	int size = ::vsnprintf(buffer, sizeof(buffer), fmt, args);

	if (size >= (int)sizeof(buffer))
	{
		char* buf = nullptr;
		size = ::vasprintf(&buf, fmt, args);
		if (size)
			write(buf, size);
		::free(buf);
	}
	else if (size)
	{
		write(buffer, size);
	}

	return size;
}


int
ostream::format(char const* fmt, ...)
{
	//M_REQUIRE(fmt);

	va_list args;
	va_start(args, fmt);
	int rc = vformat(fmt, args);
	va_end(args);

	return rc;
}


ostream&
ostream::flush() throw()
{
	if (fflush(m_fp) == EOF)
		setstate(feof(m_fp) ? eofbit | failbit : badbit);

	return *this;
}


unsigned long
ostream::tellp()
{
	long pos = ftell(m_fp);

	if (pos >= 0)
		return pos;

	setstate(badbit);
	return 0;
}


ostream&
ostream::seekp(long pos, seekdir dir)
{
	if (fseek(m_fp, pos, fdir(dir)) != 0)
		setstate(failbit);

	return *this;
}


ostream&
ostream::seekp(size_t pos)
{
	return seekp(pos, cur);
}

// vi:set ts=3 sw=3:
