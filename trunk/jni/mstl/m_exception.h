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

#ifndef _mstl_exception_included
#define _mstl_exception_included

#include "m_backtrace.h"

#include <stdarg.h>

#define M_RAISE(fmt,args...) M_THROW(::mstl::exception(fmt,##args))

#ifdef __OPTIMIZE__
# define M_THROW(exc) ({ throw exc; })
#else
# define M_THROW(exc) ({ ::mstl::bits::throw_exc(exc, __FILE__, __LINE__, __func__); })
#endif

#include "m_exception.ipp"

namespace mstl {

class string;

class basic_exception
{
public:

	basic_exception() throw();
	basic_exception(string const& msg);
	explicit basic_exception(char const* fmt, ...) __attribute__((__format__(__printf__, 2, 3)));
	basic_exception(char const* fmt, va_list args);
	basic_exception(basic_exception const& exc);
	virtual ~basic_exception () throw();

	virtual char const* what() const throw();

protected:

	void set_message(char const* fmt, va_list args);
	void assign(mstl::string const& s);

private:

#if !HAVE_OX_EXPLICITLY_DEFAULTED_AND_DELETED_SPECIAL_MEMBER_FUNCTIONS
	basic_exception& operator=(basic_exception const&);
#endif

	string* m_msg;
};

class exception : public basic_exception
{
public:

	exception() throw();
	exception(string const& msg);
	explicit exception(char const* fmt, ...) __attribute__((__format__(__printf__, 2, 3)));
	exception(char const* fmt, va_list args);
	exception(exception const& exc);

#if HAVE_OX_EXPLICITLY_DEFAULTED_AND_DELETED_SPECIAL_MEMBER_FUNCTIONS
	exception& operator=(exception const&) = delete;
#endif

	::mstl::backtrace const& backtrace() const;

protected:

	void set_backtrace(::mstl::backtrace const& backtrace);

private:

#ifndef __OPTIMIZE__
	friend void bits::prepare_msg(exception& exc, char const*, unsigned, char const*, char const*);
#endif

	::mstl::backtrace	m_backtrace;
};


bool uncaught_exception() throw();

} // namespace mstl

#endif // _mstl_exception_included

// vi:set ts=3 sw=3:
