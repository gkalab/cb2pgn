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

#ifndef _mstl_limits_included
#define _mstl_limits_included

#include "m_types.h"

#include <limits.h>
#include <float.h>

#ifndef UINTPTR_MAX
# if __WORDSIZE == 64
#  define UINTPTR_MAX UINT64_C(18446744073709551615)
# else
#  define UINTPTR_MAX (4294967295U)
# endif
#endif

#ifndef LLONG_MAX
# define ULLONG_MAX	UINT64_C(0xFFFFFFFFFFFFFFFF)
# define LLONG_MAX	INT64_C(0x7FFFFFFFFFFFFFFF)
# define LLONG_MIN	ULLONG_MAX
#endif

namespace mstl {

template <typename T>
struct numeric_limits
{
	/// Returns the minimum value for type T.
	inline static T min() { return T(0); }
	/// Returns the minimum value for type T.
	inline static T max() { return T(0); }

	static bool const is_signed	= false;	///< True if the type is signed.
	static bool const is_unsigned	= false;	///< True if the type is unsigned.
	static bool const is_integer	= false;	///< True if stores an exact value.
	static bool const is_integral	= false;	///< True if fixed size and cast-copyable.
};

#ifndef DOXYGEN_SHOULD_SKIP_THIS

template <typename T>
struct numeric_limits<T*>
{
	inline static T* min() { return 0; }
	inline static T* max() { return reinterpret_cast<T*>(UINTPTR_MAX); }

	static bool const is_signed	= false;
	static bool const is_unsigned	= true;
	static bool const is_integer	= true;
	static bool const is_integral	= true;
};

#define _NUMERIC_LIMITS(type, min_val, max_val, b_signed, b_integer, b_integral)	\
template <>																								\
struct numeric_limits<type>																		\
{																											\
    inline static type min() { return (min_val); }											\
    inline static type max() { return (max_val); }											\
																											\
    static bool const is_signed		= b_signed;												\
	 static bool const is_unsigned	= !b_signed;											\
    static bool const is_integer		= b_integer;											\
    static bool const is_integral	= b_integral;											\
}

//--------------------------------------------------------------------------------------
//		 			type						min			max			signed	integer	integral
//--------------------------------------------------------------------------------------
_NUMERIC_LIMITS(bool,					false,		true,			false,	true,		true);
_NUMERIC_LIMITS(char,					CHAR_MIN,	CHAR_MAX,	true,		true,		true);
_NUMERIC_LIMITS(int,						INT_MIN,		INT_MAX,		true,		true,		true);
_NUMERIC_LIMITS(short,					SHRT_MIN,	SHRT_MAX,	true,		true,		true);
_NUMERIC_LIMITS(long,					LONG_MIN,	LONG_MAX,	true,		true,		true);
_NUMERIC_LIMITS(signed char,			SCHAR_MIN,	SCHAR_MAX,	true,		true,		true);
_NUMERIC_LIMITS(unsigned char,		0,				UCHAR_MAX,	false,	true,		true);
_NUMERIC_LIMITS(unsigned int,			0,				UINT_MAX,	false,	true,		true);
_NUMERIC_LIMITS(unsigned short,		0,				USHRT_MAX,	false,	true,		true);
_NUMERIC_LIMITS(unsigned long,		0,				ULONG_MAX,	false,	true,		true);
_NUMERIC_LIMITS(float,					FLT_MIN,		FLT_MAX,		true,		false,	true);
_NUMERIC_LIMITS(double,					DBL_MIN,		DBL_MAX,		true,		false,	true);
_NUMERIC_LIMITS(long double,			LDBL_MIN,	LDBL_MAX,	true,		false,	true);
_NUMERIC_LIMITS(long long,				LLONG_MIN,	LLONG_MAX,	true,		true,		true);
_NUMERIC_LIMITS(unsigned long long,	0,				ULLONG_MAX,	false,	true,		true);
//--------------------------------------------------------------------------------------
#undef _NUMERIC_LIMITS

#endif // DOXYGEN_SHOULD_SKIP_THIS

} // namespace mstl

#endif // _mstl_limits_included

// vi:set ts=3 sw=3:
