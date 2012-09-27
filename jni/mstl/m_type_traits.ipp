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

#include "m_types.h"

namespace mstl {
namespace bits {

struct any_conversion
{
	template <typename T> any_conversion(volatile const T&);
	template <typename T> any_conversion(T&);
};

template <typename T>
struct is_convertible_checker
{
	static int16_t m_check(any_conversion ...);
	static int32_t m_check(T, int);
};

template <typename From, typename To>
struct is_convertible
{
	static From m_from;
	enum { value = sizeof(is_convertible_checker<To>::m_check(m_from, 0)) == sizeof(int16_t) };
};

template <class U> int16_t is_class_tester(void(U::*)(void));
template <class U> int32_t is_class_tester(...);

template <typename T>
struct is_class
{
	enum { value = sizeof(is_class_tester<T>(0)) == sizeof(int16_t) };
};

template <typename T> struct is_member_pointer { enum { value = 0 }; };

} // namespace bits

template <typename From, typename To>
struct is_convertible
{
	typedef From& ref_type;
	enum { value = bits::is_convertible<ref_type, To>::value & !is_array<To>::value };
};

template <typename T> struct is_integral				{ enum { value = 0 }; };
template <> struct is_integral<bool>					{ enum { value = 1 }; };
template <> struct is_integral<signed char>			{ enum { value = 1 }; };
template <> struct is_integral<signed short>			{ enum { value = 1 }; };
template <> struct is_integral<signed int>			{ enum { value = 1 }; };
template <> struct is_integral<signed long>			{ enum { value = 1 }; };
template <> struct is_integral<signed long long>	{ enum { value = 1 }; };
template <> struct is_integral<unsigned char>		{ enum { value = 1 }; };
template <> struct is_integral<unsigned short>		{ enum { value = 1 }; };
template <> struct is_integral<unsigned int>			{ enum { value = 1 }; };
template <> struct is_integral<unsigned long>		{ enum { value = 1 }; };
template <> struct is_integral<unsigned long long>	{ enum { value = 1 }; };

template <typename T> struct is_float					{ enum { value = 0 }; };
template <> struct is_float<float>						{ enum { value = 1 }; };
template <> struct is_float<double>						{ enum { value = 1 }; };
template <> struct is_float<long double>				{ enum { value = 1 }; };

template <typename T> struct is_void					{ enum { value = 0 }; };
template <> struct is_void<void>							{ enum { value = 1 }; };
template <> struct is_void<void const>					{ enum { value = 1 }; };
template <> struct is_void<void volatile>				{ enum { value = 1 }; };
template <> struct is_void<void const volatile>		{ enum { value = 1 }; };

template <typename T> struct is_reference				{ enum { value = 0 }; };
template <typename T> struct is_reference<T&>		{ enum { value = 1 }; };

template <typename T> struct is_arithmetic		{ enum { value = is_integral<T>::value
																					| is_float<T>::value }; };

template <typename T> struct is_enum				{ enum { value = !is_arithmetic<T>::value
																					& !is_reference<T>::value
																					& !is_function<T>::value
																					& !is_class<T>::value }; };

template <typename T> struct is_scalar			{ enum { value = is_arithmetic<T>::value
																				| is_enum<T>::value
																				| is_pointer<T>::value
																				| is_member_pointer<T>::value }; };

template <typename T> struct is_pod				{ enum { value = is_scalar<T>::value
																				| is_void<T>::value }; };

template <typename T, size_t N> struct is_pod<T[N]> : is_pod<T> {};

template <typename T> struct is_pointer							{ enum { value = 0 }; };
template <typename T> struct is_pointer<T*>						{ enum { value = 1 }; };
template <typename T> struct is_pointer<T const*>				{ enum { value = 1 }; };
template <typename T> struct is_pointer<T volatile*>			{ enum { value = 1 }; };
template <typename T> struct is_pointer<T volatile const*>	{ enum { value = 1 }; };

template <typename T> struct is_member_pointer							{ enum { value = 0 }; };
template <typename T, typename U> struct is_member_pointer<U T::*>	{ enum { value = 1 }; };

template <typename T> struct is_array											{ enum { value = 0 }; };
template <typename T, size_t N> struct is_array<T[N]>						{ enum { value = 1 }; };
template <typename T, size_t N> struct is_array<T const[N]>				{ enum { value = 1 }; };
template <typename T, size_t N> struct is_array<T volatile[N]>			{ enum { value = 1 }; };
template <typename T, size_t N> struct is_array<T volatile const[N]>	{ enum { value = 1 }; };

template <typename T>
struct is_function { enum { value = !is_convertible<T*, volatile void const*>::value }; };

template <typename T> struct is_class { enum { value = bits::is_class<T>::value }; };

template <typename T> struct has_trivial_destructor	{ enum { value = is_pod<T>::value }; };
template <typename T> struct is_movable					{ enum { value = is_pod<T>::value }; };

template <typename T> struct remove_reference		{ typedef T type; };
template <typename T> struct remove_reference<T&>	{ typedef T type; };
#if USE_0X_STANDARD
template <typename T> struct remove_reference<T&&>	{ typedef T type; };
#endif

} // namespace mstl

// vi:set ts=3 sw=3:
