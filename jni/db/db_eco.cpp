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

#include "db_eco.h"
#include "db_exception.h"

#include <ctype.h>

using namespace db;


Eco const Eco::m_root("A00");


uint16_t
Eco::toShort(char const* s)
{
	if (__builtin_expect(*s < 'A' || 'E' < *s, 0))
		return Eco();

	uint16_t code = *s++ - 'A';

	if (__builtin_expect(!::isdigit(*s), 0))
		return Eco();

	code = 10*code + (*s++ - '0');

	if (__builtin_expect(!::isdigit(*s), 0))
		return Eco();

	return 10*code + (*s++ - '/');
}


void
Eco::setup(char const* s)
{
	m_code = 0;

	if (__builtin_expect(*s < 'A' || 'E' < *s, 0))
		return;

	Code code = *s++ - 'A';

	if (__builtin_expect(!::isdigit(*s), 0))
		return;

	code = 10*code + (*s++ - '0');

	if (__builtin_expect(!::isdigit(*s), 0))
		return;

	code = (10*code + (*s++ - '0')) << Sub_Code_Bits;

	if (*s++ == '.')
	{
		if (__builtin_expect(	!::isdigit(s[0])
									|| !::isdigit(s[1])
									|| !::isdigit(s[2])
									|| !::isdigit(s[3]),
									0))
		{
			return;
		}

		Code extended = (s[0] - '0')*1000 + (s[1] - '0')*100 + (s[2] - '0')*10 + (s[3] - '0');

		if (__builtin_expect(extended >= Num_Sub_Codes, 0))
			return;

		code += extended;

		if (code > Max_Code)
			return;
	}

	m_code = code + 1;
}


void
Eco::convert(char* buf, bool shortForm) const
{
	// M_REQUIRE(buf);	// require: #buf >= 9

	if (m_code == 0)
	{
		buf[0] = '\0';
	}
	else
	{
		Code code = m_code - 1;

		if (shortForm)
		{
			buf[3] = '\0';
		}
		else
		{
			Code deci = code & (Num_Sub_Codes - 1);

			buf[3] = '.';
			buf[4] = '0' + deci/1000;	deci %= 1000;
			buf[5] = '0' + deci/100;	deci %= 100;
			buf[6] = '0' + deci/10;		deci %= 10;
			buf[7] = '0' + deci;
			buf[8] = 0;
		}

		code >>= Sub_Code_Bits;

		buf[2] = '0' + code % 10; code /= 10;
		buf[1] = '0' + code % 10; code /= 10;
		buf[0] = code + 'A';
	}
}


mstl::string
Eco::asString() const
{
	char buf[9];
	convert(buf);
	return mstl::string(buf);
}


mstl::string
Eco::asShortString() const
{
	char buf[9];
	convert(buf, true);
	return mstl::string(buf);
}

// vi:set ts=3 sw=3:
