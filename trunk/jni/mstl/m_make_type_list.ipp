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

namespace mstl {

template <>
struct make_type_list<
	null_type,null_type,null_type,null_type,null_type,null_type,null_type,null_type,null_type,null_type,
	null_type,null_type,null_type,null_type,null_type,null_type,null_type,null_type,null_type,null_type,
	null_type,null_type,null_type,null_type>
{
	typedef null_type result;
};

template <typename T0>
struct make_type_list<
	T0,null_type,null_type,null_type,null_type,null_type,null_type,null_type,null_type,null_type,
	null_type,null_type,null_type,null_type,null_type,null_type,null_type,null_type,null_type,null_type,
	null_type,null_type,null_type,null_type>
{
	typedef type_list<T0,null_type> result;
};

template <typename T0, typename T1>
struct make_type_list<
	T0,T1,null_type,null_type,null_type,null_type,null_type,null_type,null_type,null_type,
	null_type,null_type,null_type,null_type,null_type,null_type,null_type,null_type,null_type,null_type,
	null_type,null_type,null_type,null_type>
{
	typedef type_list<T0,typename make_type_list<T1>::result> result;
};

template <typename T0, typename T1, typename T2>
struct make_type_list<
	T0,T1,T2,null_type,null_type,null_type,null_type,null_type,null_type,null_type,
	null_type,null_type,null_type,null_type,null_type,null_type,null_type,null_type,null_type,null_type,
	null_type,null_type,null_type,null_type>
{
	typedef type_list<T0,typename make_type_list<T1,T2>::result> result;
};

template <typename T0, typename T1, typename T2, typename T3>
struct make_type_list<
	T0,T1,T2,T3,null_type,null_type,null_type,null_type,null_type,null_type,
	null_type,null_type,null_type,null_type,null_type,null_type,null_type,null_type,null_type,null_type,
	null_type,null_type,null_type,null_type>
{
	typedef type_list<T0,typename make_type_list<T1,T2,T3>::result> result;
};

template <typename T0, typename T1, typename T2, typename T3, typename T4>
struct make_type_list<
	T0,T1,T2,T3,T4,null_type,null_type,null_type,null_type,null_type,
	null_type,null_type,null_type,null_type,null_type,null_type,null_type,null_type,null_type,null_type,
	null_type,null_type,null_type,null_type>
{
	typedef type_list<T0,typename make_type_list<T1,T2,T3,T4>::result> result;
};

template <typename T0, typename T1, typename T2, typename T3, typename T4, typename T5>
struct make_type_list<
	T0,T1,T2,T3,T4,T5,null_type,null_type,null_type,null_type,
	null_type,null_type,null_type,null_type,null_type,null_type,null_type,null_type,null_type,null_type,
	null_type,null_type,null_type,null_type>
{
	typedef type_list<T0,typename make_type_list<T1,T2,T3,T4,T5>::result> result;
};

template <typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
struct make_type_list<
	T0,T1,T2,T3,T4,T5,T6,null_type,null_type,null_type,
	null_type,null_type,null_type,null_type,null_type,null_type,null_type,null_type,null_type,null_type,
	null_type,null_type,null_type,null_type>
{
	typedef type_list<T0,typename make_type_list<T1,T2,T3,T4,T5,T6>::result> result;
};

template <
	typename T0, typename T1, typename T2, typename T3, typename T4,
	typename T5, typename T6, typename T7>
struct make_type_list<
	T0,T1,T2,T3,T4,T5,T6,T7,null_type,null_type,
	null_type,null_type,null_type,null_type,null_type,null_type,null_type,null_type,null_type,null_type,
	null_type,null_type,null_type,null_type>
{
	typedef type_list<T0,typename make_type_list<T1,T2,T3,T4,T5,T6,T7>::result> result;
};

template <
	typename T0, typename T1, typename T2, typename T3, typename T4,
	typename T5, typename T6, typename T7, typename T8>
struct make_type_list<
	T0,T1,T2,T3,T4,T5,T6,T7,T8,null_type,
	null_type,null_type,null_type,null_type,null_type,null_type,null_type,null_type,null_type,null_type,
	null_type,null_type,null_type,null_type>
{
	typedef type_list<T0,typename make_type_list<T1,T2,T3,T4,T5,T6,T7,T8>::result> result;
};

template <
	typename T0, typename T1, typename T2, typename T3, typename T4,
	typename T5, typename T6, typename T7, typename T8, typename T9>
struct make_type_list<
	T0,T1,T2,T3,T4,T5,T6,T7,T8,T9,
	null_type,null_type,null_type,null_type,null_type,null_type,null_type,null_type,null_type,null_type,
	null_type,null_type,null_type,null_type>
{
	typedef type_list<T0,typename make_type_list<T1,T2,T3,T4,T5,T6,T7,T8,T9>::result> result;
};

template <
	typename T0, typename T1, typename T2, typename T3, typename T4,
	typename T5, typename T6, typename T7, typename T8, typename T9,
	typename T10>
struct make_type_list<
	T0,T1,T2,T3,T4,T5,T6,T7,T8,T9,
	T10,null_type,null_type,null_type,null_type,null_type,null_type,null_type,null_type,null_type,
	null_type,null_type,null_type,null_type>
{
	typedef type_list<T0,typename make_type_list<T1,T2,T3,T4,T5,T6,T7,T8,T9,T10>::result> result;
};

template <
	typename T0, typename T1, typename T2, typename T3, typename T4,
	typename T5, typename T6, typename T7, typename T8, typename T9,
	typename T10, typename T11>
struct make_type_list<
	T0,T1,T2,T3,T4,T5,T6,T7,T8,T9,
	T10,T11,null_type,null_type,null_type,null_type,null_type,null_type,null_type,null_type,
	null_type,null_type,null_type,null_type>
{
	typedef type_list<T0,typename make_type_list<T1,T2,T3,T4,T5,T6,T7,T8,T9,T10,T11>::result> result;
};

template <
	typename T0, typename T1, typename T2, typename T3, typename T4,
	typename T5, typename T6, typename T7, typename T8, typename T9,
	typename T10, typename T11, typename T12>
struct make_type_list<
	T0,T1,T2,T3,T4,T5,T6,T7,T8,T9,
	T10,T11,T12,null_type,null_type,null_type,null_type,null_type,null_type,null_type,
	null_type,null_type,null_type,null_type>
{
	typedef type_list<T0,typename make_type_list<T1,T2,T3,T4,T5,T6,T7,T8,T9,T10,T11,T12>::result> result;
};

template <
	typename T0, typename T1, typename T2, typename T3, typename T4,
	typename T5, typename T6, typename T7, typename T8, typename T9,
	typename T10, typename T11, typename T12, typename T13>
struct make_type_list<
	T0,T1,T2,T3,T4,T5,T6,T7,T8,T9,
	T10,T11,T12,T13,null_type,null_type,null_type,null_type,null_type,null_type,
	null_type,null_type,null_type,null_type>
{
	typedef type_list<
			T0,
			typename make_type_list<T1,T2,T3,T4,T5,T6,T7,T8,T9,T10,T11,T12,T13>::result> result;
};

template <
	typename T0, typename T1, typename T2, typename T3, typename T4,
	typename T5, typename T6, typename T7, typename T8, typename T9,
	typename T10, typename T11, typename T12, typename T13, typename T14>
struct make_type_list<
	T0,T1,T2,T3,T4,T5,T6,T7,T8,T9,
	T10,T11,T12,T13,T14,null_type,null_type,null_type,null_type,null_type,
	null_type,null_type,null_type,null_type>
{
	typedef type_list<
			T0,
			typename make_type_list<T1,T2,T3,T4,T5,T6,T7,T8,T9,T10,T11,T12,T13,T14>::result> result;
};

template <
	typename T0, typename T1, typename T2, typename T3, typename T4,
	typename T5, typename T6, typename T7, typename T8, typename T9,
	typename T10, typename T11, typename T12, typename T13, typename T14,
	typename T15>
struct make_type_list<
	T0,T1,T2,T3,T4,T5,T6,T7,T8,T9,
	T10,T11,T12,T13,T14,T15,null_type,null_type,null_type,null_type,
	null_type,null_type,null_type,null_type>
{
	typedef type_list<
			T0,
			typename make_type_list<T1,T2,T3,T4,T5,T6,T7,T8,T9,T10,T11,T12,T13,T14,T15>::result> result;
};

template <
	typename T0, typename T1, typename T2, typename T3, typename T4,
	typename T5, typename T6, typename T7, typename T8, typename T9,
	typename T10, typename T11, typename T12, typename T13, typename T14,
	typename T15, typename T16>
struct make_type_list<
	T0,T1,T2,T3,T4,T5,T6,T7,T8,T9,
	T10,T11,T12,T13,T14,T15,T16,null_type,null_type,null_type,
	null_type,null_type,null_type,null_type>
{
	typedef type_list<
			T0,
			typename make_type_list<T1,T2,T3,T4,T5,T6,T7,T8,T9,T10,T11,T12,T13,T14,T15,T16>::result> result;
};

template <
	typename T0, typename T1, typename T2, typename T3, typename T4,
	typename T5, typename T6, typename T7, typename T8, typename T9,
	typename T10, typename T11, typename T12, typename T13, typename T14,
	typename T15, typename T16, typename T17>
struct make_type_list<T0,T1,T2,T3,T4,T5,T6,T7,T8,T9,T10,T11,T12,T13,T14,T15,T16,T17,null_type,null_type,
	null_type,null_type,null_type,null_type>
{
	typedef type_list<
			T0,
			typename make_type_list<T1,T2,T3,T4,T5,T6,T7,T8,T9,T10,T11,T12,T13,T14,T15,T16,T17>
		::result> result;
};

template <
	typename T0, typename T1, typename T2, typename T3, typename T4,
	typename T5, typename T6, typename T7, typename T8, typename T9,
	typename T10, typename T11, typename T12, typename T13, typename T14,
	typename T15, typename T16, typename T17, typename T18>
struct make_type_list<T0,T1,T2,T3,T4,T5,T6,T7,T8,T9,T10,T11,T12,T13,T14,T15,T16,T17,T18,null_type,
	null_type,null_type,null_type,null_type>
{
	typedef type_list<
			T0,
			typename make_type_list<T1,T2,T3,T4,T5,T6,T7,T8,T9,T10,T11,T12,T13,T14,T15,T16,T17,T18>
		::result> result;
};

template <
	typename T0, typename T1, typename T2, typename T3, typename T4,
	typename T5, typename T6, typename T7, typename T8, typename T9,
	typename T10, typename T11, typename T12, typename T13, typename T14,
	typename T15, typename T16, typename T17, typename T18, typename T19>
struct make_type_list<T0,T1,T2,T3,T4,T5,T6,T7,T8,T9,T10,T11,T12,T13,T14,T15,T16,T17,T18,T19,null_type,
	null_type,null_type,null_type>
{
	typedef type_list<
			T0,
			typename make_type_list<T1,T2,T3,T4,T5,T6,T7,T8,T9,T10,T11,T12,T13,T14,T15,T16,T17,T18,T19>
		::result> result;
};

template <
	typename T0, typename T1, typename T2, typename T3, typename T4,
	typename T5, typename T6, typename T7, typename T8, typename T9,
	typename T10, typename T11, typename T12, typename T13, typename T14,
	typename T15, typename T16, typename T17, typename T18, typename T19,
	typename T20>
struct make_type_list
	<T0,T1,T2,T3,T4,T5,T6,T7,T8,T9,T10,T11,T12,T13,T14,T15,T16,T17,T18,T19,T20,
		null_type,null_type,null_type>
{
	typedef type_list<
			T0,
			typename make_type_list<T1,T2,T3,T4,T5,T6,T7,T8,T9,T10,T11,T12,T13,T14,T15,T16,T17,T18,T19,T20>
		::result> result;
};

template <
	typename T0, typename T1, typename T2, typename T3, typename T4,
	typename T5, typename T6, typename T7, typename T8, typename T9,
	typename T10, typename T11, typename T12, typename T13, typename T14,
	typename T15, typename T16, typename T17, typename T18, typename T19,
	typename T20, typename T21>
struct make_type_list
	<T0,T1,T2,T3,T4,T5,T6,T7,T8,T9,T10,T11,T12,T13,T14,T15,T16,T17,T18,T19,T20,T21,
	null_type,null_type>
{
	typedef type_list<
			T0,
			typename make_type_list
				<T1,T2,T3,T4,T5,T6,T7,T8,T9,T10,T11,T12,T13,T14,T15,T16,T17,T18,T19,T20,T21>
		::result> result;
};

template <
	typename T0, typename T1, typename T2, typename T3, typename T4,
	typename T5, typename T6, typename T7, typename T8, typename T9,
	typename T10, typename T11, typename T12, typename T13, typename T14,
	typename T15, typename T16, typename T17, typename T18, typename T19,
	typename T20, typename T21, typename T22>
struct make_type_list
	<T0,T1,T2,T3,T4,T5,T6,T7,T8,T9,T10,T11,T12,T13,T14,T15,T16,T17,T18,T19,T20,T21,T22,
	null_type>
{
	typedef type_list<
			T0,
			typename make_type_list
				<T1,T2,T3,T4,T5,T6,T7,T8,T9,T10,T11,T12,T13,T14,T15,T16,T17,T18,T19,T20,T21,T22>
		::result> result;
};

template <
	typename T0, typename T1, typename T2, typename T3, typename T4,
	typename T5, typename T6, typename T7, typename T8, typename T9,
	typename T10, typename T11, typename T12, typename T13, typename T14,
	typename T15, typename T16, typename T17, typename T18, typename T19,
	typename T20, typename T21, typename T22, typename T23>
struct make_type_list
{
	typedef type_list<
			T0,
			typename make_type_list
				<T1,T2,T3,T4,T5,T6,T7,T8,T9,T10,T11,T12,T13,T14,T15,T16,T17,T18,T19,T20,T21,T22,T23>
		::result> result;
};

} // namespace mstl

// vi:set ts=3 sw=3:
