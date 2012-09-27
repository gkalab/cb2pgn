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

#ifndef _util_exception_included
#define _util_exception_included

#include "m_exception.h"

#define U_RAISE(fmt,args...) M_THROW(::util::Exception(fmt,##args))

namespace util {

class BasicException : public mstl::basic_exception
{
public:

	// structors
	BasicException();
	explicit BasicException(char const* fmt, ...) __attribute__((__format__(__printf__, 2, 3)));
	BasicException(char const* fmt, va_list args);
};

class Exception : public mstl::exception
{
public:

	// structors
	Exception();
	explicit Exception(char const* fmt, ...) __attribute__((__format__(__printf__, 2, 3)));
	Exception(char const* fmt, va_list args);
};

} // namespace util

#endif // _util_exception_included

// vi:set ts=3 sw=3:
