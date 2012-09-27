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

#ifndef _m_exception_included
#define _m_exception_included

#ifndef __OPTIMIZE__

#ifdef __clang__
class type_info; // because of a cyclic bug in gcc headers
#endif

#include <typeinfo>

namespace std { bool uncaught_exception() throw(); }

namespace mstl {

class exception;

namespace bits {

void
prepare_msg(exception& exc, char const* file, unsigned line, char const* func, char const* exc_type_id);

void
prepare_exc(exception& exc, char const* file, unsigned line, char const* func, char const* exc_type_id);

template <class Exc>
__attribute__((noreturn))
inline
static void
throw_exc(Exc const& exc, char const* file, int line, char const* func)
{
	Exc e(exc);
	prepare_exc(e, file, line, func, typeid(Exc).name());
	throw e;
}

} // namespace bits


inline bool uncaught_exception() throw() { return ::std::uncaught_exception(); }

} // namespace mstl

#endif // __OPTIMIZE__
#endif // _m_exception_included

// vi:set ts=3 sw=3:
