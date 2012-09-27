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

#ifndef _u_base_included
#define _u_base_included

#include "m_types.h"

#define U_NUMBER_OF_T(type)	(sizeof(type)/sizeof(((type*)0)[0]))
#define U_NUMBER_OF(var)		(sizeof(var)/sizeof((var)[0]))

#define U_BITS_OF(type) (sizeof(type)*8)

#if defined(__GNUC__) && !defined(__WIN32__)
// We want to use 'minor' and 'major' as method names.
// Unfortunately these names are used as macros,
// so we have to undefine these macros.
# include "sys/sysmacros.h"
# undef minor
# undef major
#endif

#ifdef __WIN32__
# include <windows.h>
# undef min
# undef max
#endif

#endif // _u_base_included

// vi:set ts=3 sw=3:
