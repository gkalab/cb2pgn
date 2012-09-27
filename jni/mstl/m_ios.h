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

#ifndef _mstl_ios_included
#define _mstl_ios_included

#include <stdio.h>
#include "m_exception.h"
#include "m_types.h"

#ifdef __WIN32__

extern "C" { struct _iobuf; }
# define _IO_FILE _iobuf

#else

extern "C" { struct _IO_FILE; }

#endif

namespace mstl {

class string;

class ios_base
{
public:

	struct failure : public exception
	{
		failure(string const& msg);
		explicit failure(char const* fmt, ...) __attribute__((__format__(__printf__, 2, 3)));
		failure(char const* fmt, va_list args);
	};

	typedef unsigned iostate;
	typedef unsigned openmode;

	static iostate const goodbit	= 0;
	static iostate const badbit	= 1 << 0;
	static iostate const eofbit	= 1 << 1;
	static iostate const failbit	= 1 << 2;

	static openmode const in		= 1 << 0;
	static openmode const out		= 1 << 1;
	static openmode const app		= 1 << 2;
	static openmode const ate		= 1 << 3;
	static openmode const binary	= 1 << 4;
	static openmode const trunc	= 1 << 5;

	ios_base();
	virtual ~ios_base() throw() = 0;

#if HAVE_OX_EXPLICITLY_DEFAULTED_AND_DELETED_SPECIAL_MEMBER_FUNCTIONS
	ios_base(ios_base const&) = delete;
	ios_base& operator=(ios_base const&) = delete;
#endif

	// coincides with SEEK_SET, SEEK_CUR, SEEK_END
	enum seekdir { beg, cur, end };

	bool operator!() const;

	bool good() const;
	bool eof() const;
	bool fail() const;
	bool bad() const;

	operator void*() const;

	int fildes();

	iostate rdstate() const;
	openmode mode() const;

	void clear(iostate state = goodbit);
	void setstate(iostate state);

	iostate exceptions() const;
	void exceptions(iostate except);

protected:

	void setmode(openmode mode);
	int fdir(seekdir dir) const;

	FILE* m_fp;

private:

#if !HAVE_OX_EXPLICITLY_DEFAULTED_AND_DELETED_SPECIAL_MEMBER_FUNCTIONS
	ios_base(ios_base const&);
	ios_base& operator=(ios_base const&);
#endif

	iostate	m_state;
	iostate	m_except;
	openmode	m_openmode;
};

} // namespace mstl

#include "m_ios.ipp"

#ifdef __WIN32__
# include <stdarg.h>

extern "C"
{
	int vasprintf(char** strp, const char* fmt, va_list ap);
}
#endif

#endif // _mstl_ios_included

// vi:set ts=3 sw=3:
