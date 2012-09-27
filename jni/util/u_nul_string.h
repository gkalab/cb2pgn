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

#ifndef _util_nul_string_included
#define _util_nul_string_included

#include "m_string.h"

namespace util {

class NulString : public mstl::string
{
public:

	NulString(char* s, size_t len);
	~NulString();

	operator const_pointer() const;

private:

	char*	m_pos;
	char	m_nulChar;
};

} // namespace util

#include "u_nul_string.ipp"

#endif // _util_nul_string_included

// vi:set ts=3 sw=3:
