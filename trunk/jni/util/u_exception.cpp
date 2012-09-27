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

#include "u_exception.h"

#include "m_stdio.h"

#include <stdarg.h>

using namespace util;


BasicException::BasicException() {}


BasicException::BasicException(char const* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	set_message(fmt, args);
	va_end(args);
}


BasicException::BasicException(char const* fmt, va_list args)
	:mstl::basic_exception(fmt, args)
{
}


Exception::Exception() {}


Exception::Exception(char const* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	set_message(fmt, args);
	va_end(args);
}


Exception::Exception(char const* fmt, va_list args)
	:mstl::exception(fmt, args)
{
}

// vi:set ts=3 sw=3:
