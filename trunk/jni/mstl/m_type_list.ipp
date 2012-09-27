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
namespace tl {

// length					/////////////////////////////////////////////////////////////////////////////////

template <>
struct length<null_type>
{
	static int const Value = 0;
};

template <typename T, typename U>
struct length< type_list<T, U> >
{
	static int const Value = 1 + length<U>::Value;
};

// type_at					/////////////////////////////////////////////////////////////////////////////////

template <typename Head, typename Tail>
struct type_at<type_list<Head, Tail>, 0>
{
	typedef Head result;
};

template <typename Head, typename Tail, int I>
struct type_at<type_list<Head, Tail>, I>
{
	typedef typename type_at<Tail, I - 1>::result result;
};

// type_at_non_strict	/////////////////////////////////////////////////////////////////////////////////

template <typename TList, int Index, typename DefaultType = null_type>
struct type_at_non_strict
{
	typedef DefaultType result;
};

template <typename Head, typename Tail, typename DefaultType>
struct type_at_non_strict<type_list<Head, Tail>, 0, DefaultType>
{
	typedef Head result;
};

template <typename Head, typename Tail, int I, typename DefaultType>
struct type_at_non_strict<type_list<Head, Tail>, I, DefaultType>
{
	typedef typename type_at_non_strict<Tail, I - 1, DefaultType>::result result;
};

// index_of					/////////////////////////////////////////////////////////////////////////////////

template <typename T>
struct index_of<null_type, T>
{
	static int const Value = -1;
};

template <typename T, typename Tail>
struct index_of<type_list<T, Tail>, T>
{
	static int const Value = 0;
};

template <typename Head, typename Tail, typename T>
struct index_of<type_list<Head, Tail>, T>
{
	static int const Value = (index_of<Tail, T>::Value == -1) ? -1 : 1 + index_of<Tail, T>::Value;
};

// append					/////////////////////////////////////////////////////////////////////////////////

template <> struct append<null_type, null_type>
{
	typedef null_type result;
};

template <typename T> struct append<null_type, T>
{
	typedef type_list<T, null_type> result;
};

template <typename Head, typename Tail>
struct append<null_type, type_list<Head, Tail> >
{
	typedef type_list<Head, Tail> result;
};

template <typename Head, typename Tail, typename T>
struct append<type_list<Head, Tail>, T>
{
	typedef type_list<Head, typename append<Tail, T>::result> result;
};

// erase						/////////////////////////////////////////////////////////////////////////////////

template <typename T>
struct erase<null_type, T>
{
	typedef null_type result;
};

template <typename T, typename Tail>
struct erase<type_list<T, Tail>, T>
{
	typedef Tail result;
};

template <typename Head, typename Tail, typename T>
struct erase<type_list<Head, Tail>, T>
{
	typedef type_list<Head, typename erase<Tail, T>::result> result;
};

// erase_all				/////////////////////////////////////////////////////////////////////////////////

template <typename T>
struct erase_all<null_type, T>
{
	typedef null_type result;
};

template <typename T, typename Tail>
struct erase_all<type_list<T, Tail>, T>
{
	typedef typename erase_all<Tail, T>::result result;
};

template <typename Head, typename Tail, typename T>
struct erase_all<type_list<Head, Tail>, T>
{
	typedef type_list<Head, typename erase_all<Tail, T>::result> result;
};

// no_duplicates			/////////////////////////////////////////////////////////////////////////////////

template <> struct no_duplicates<null_type>
{
	typedef null_type result;
};

template <typename Head, typename Tail>
struct no_duplicates< type_list<Head, Tail> >
{
	typedef type_list<Head, typename erase<typename no_duplicates<Tail>::result, Head>::result> result;
};

// replace					/////////////////////////////////////////////////////////////////////////////////

template <typename T, typename U>
struct replace<null_type, T, U>
{
	typedef null_type result;
};

template <typename T, typename Tail, typename U>
struct replace<type_list<T, Tail>, T, U>
{
	typedef type_list<U, Tail> result;
};

template <typename Head, typename Tail, typename T, typename U>
struct replace<type_list<Head, Tail>, T, U>
{
	typedef type_list<Head, typename replace<Tail, T, U>::result> result;
};

// replace_all				/////////////////////////////////////////////////////////////////////////////////

template <typename T, typename U>
struct replace_all<null_type, T, U>
{
	typedef null_type result;
};

template <typename T, typename Tail, typename U>
struct replace_all<type_list<T, Tail>, T, U>
{
	typedef type_list<U, typename replace_all<Tail, T, U>::result> result;
};

template <typename Head, typename Tail, typename T, typename U>
struct replace_all<type_list<Head, Tail>, T, U>
{
	typedef type_list<Head, typename replace_all<Tail, T, U>::result> result;
};

// reverse					/////////////////////////////////////////////////////////////////////////////////

template <typename T>
struct reverse< type_list<T, null_type> >
{
	typedef type_list<T, null_type> result;
};

template <typename Head, typename Tail>
struct reverse< type_list<Head, Tail> >
{
	typedef typename append<typename reverse<Tail>::result, Head>::result result;
};

// size_of				/////////////////////////////////////////////////////////////////////////////////

template <typename T>
struct size_of< type_list<T, null_type> >
{
	enum { value = sizeof(T) };
};

template <typename Head, typename Tail>
struct size_of< type_list<Head, Tail> >
{
	enum { value = sizeof(Head) < size_of<Tail>::value ? size_of<Tail>::value : sizeof(Head) };
};

#if 0
// most_derived			/////////////////////////////////////////////////////////////////////////////////

template <typename T>
struct most_derived<null_type, T>
{
	typedef T result;
};

template <typename Head, typename Tail, typename T>
struct most_derived<type_list<Head, Tail>, T>
{
private:

	typedef typename most_derived<Tail, T>::result candidate;

public:

	typedef typename
		boost::mpl::if_c<super_subtypename<candidate, Head>::Value, Head, candidate>::type result;
};

// derived_to_front		/////////////////////////////////////////////////////////////////////////////////

template <>
struct derived_to_front<null_type>
{
	typedef null_type result;
};

template <typename Head, typename Tail>
struct derived_to_front< type_list<Head, Tail> >
{
private:

	typedef typename most_derived<Tail, Head>::result the_most_derived;

public:

	typedef type_list<the_most_derived, typename replace<Tail, the_most_derived, Head>::result> result;
};
#endif

} // namespace tl
} // namespace mstl

// vi:set ts=3 sw=3:
