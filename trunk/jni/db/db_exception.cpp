// ======================================================================
// Author : $Author$
// Version: $Revision$
// Date   : $Date$
// Url    : $URL$
// ======================================================================

// ======================================================================
//    _/|            __
//   // o\         /    )           ,        /    /
//   || ._)    ----\---------__----------__-/----/__-
//   //__\          \      /   '  /    /   /    /   )
//   )___(     _(____/____(___ __/____(___/____(___/_
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

#include "db_exception.h"

#include <stdarg.h>

using namespace db;


Exception::Exception(char const* fmt, ...)
	:util::Exception()
{
	va_list args;
	va_start(args, fmt);
	set_message(fmt, args);
	va_end(args);
}


Exception::Exception(char const* fmt, va_list args)
	:util::Exception()
{
	set_message(fmt, args);
}


Exception::Exception() :util::Exception() {}
Exception::Exception(util::Exception& exc) : util::Exception(exc) {}
Exception::~Exception() throw() {}


IOException::IOException(	FileType fileType,
									ErrorType errorType,
									char const* fmt, ...)
	:Exception()
	,m_fileType(fileType)
	,m_errorType(errorType)
{
	va_list args;
	va_start(args, fmt);
	set_message(fmt, args);
	va_end(args);
}


IOException::IOException(	FileType fileType,
									ErrorType errorType,
									mstl::backtrace const& backtrace,
									char const* fmt, ...)
	:Exception()
	,m_fileType(fileType)
	,m_errorType(errorType)
{
	va_list args;
	va_start(args, fmt);
	set_message(fmt, args);
	set_backtrace(backtrace);
	va_end(args);
}


IOException::FileType IOException::fileType() const	{ return m_fileType; }
IOException::ErrorType IOException::errorType() const	{ return m_errorType; }


DecodingFailedException::DecodingFailedException() :Exception() {}

// vi:set ts=3 sw=3:
