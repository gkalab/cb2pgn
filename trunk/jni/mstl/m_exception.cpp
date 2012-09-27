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

#include "m_exception.h"
#include "m_sstream.h"
#include "m_string.h"

#include <stdlib.h>
#include <stdarg.h>

using namespace mstl;


basic_exception::basic_exception() throw() : m_msg(new string) {}
basic_exception::basic_exception(string const& msg) : m_msg(new string(msg)) {}
basic_exception::basic_exception(basic_exception const& exc) : m_msg(new mstl::string(*exc.m_msg)) {}
basic_exception::~basic_exception() throw() { delete m_msg; }

char const* basic_exception::what() const throw() { return *m_msg; }


basic_exception::basic_exception(char const* fmt, va_list args)
	:m_msg(new string)
{
	m_msg->vformat(fmt, args);
}


basic_exception::basic_exception(char const* fmt, ...)
	:m_msg(new string)
{
	//M_REQUIRE(fmt);

	va_list args;
	va_start(args, fmt);
	m_msg->vformat(fmt, args);
	va_end(args);
}


void
basic_exception::set_message(char const* fmt, va_list args)
{
	m_msg->vformat(fmt, args);
}


void
basic_exception::assign(mstl::string const& s)
{
	m_msg->assign(s);
}


exception::exception() throw() {}
exception::exception(string const& msg) : basic_exception(msg) {}
exception::exception(exception const& exc) : basic_exception(exc) {}

backtrace const& exception::backtrace() const { return m_backtrace; }


exception::exception(char const* fmt, va_list args)
	:basic_exception(fmt, args)
{
}


exception::exception(char const* fmt, ...)
{
	//M_REQUIRE(fmt);

	va_list args;
	va_start(args, fmt);
	set_message(fmt, args);
	va_end(args);
}


void
exception::set_backtrace(::mstl::backtrace const& backtrace)
{
	m_backtrace = backtrace;
}


#ifndef __OPTIMIZE__

#ifdef __GNUC__
# include <cxxabi.h>
#endif


static int
excbreak()
{
	return 0;
}


void
mstl::bits::prepare_msg(mstl::exception& exc,
								char const* file,
								unsigned line,
								char const* func,
								char const* exc_type)
{
//	try
//	{
		ostringstream strm;
		strm.format(	"(func) %s\n(file) %s:%u\n(what) %s\n(type) %s\n",
							func, file, line, exc.what(), exc_type);

#ifndef NDEBUG
		if (!exc.backtrace().empty())
		{
			strm.write("\n", 1);
			strm.format("=== Backtrace ============================================\n");
			exc.backtrace().text_write(strm, 3);
			strm.format("==========================================================\n");
		}
#endif

		excbreak();
		exc.assign(strm.str());
//	}
//	catch (...)
//	{
//	}
}


void
mstl::bits::prepare_exc(mstl::exception& exc,
								char const* file,
								unsigned line,
								char const* func,
								char const* exc_type_id)
{
#ifdef __GNUC__

	int	status;
	char*	exc_type = ::abi::__cxa_demangle(exc_type_id, 0, 0, &status);

	prepare_msg(exc, file, line, func, status == -1 ? 0 : exc_type);

	if (status != -1)
		free(exc_type);

#else

	prepare_msg(exc, file, line, func, exc_type_id);

#endif
}

#endif // __OPTIMIZE__

// vi:set ts=3 sw=3:
