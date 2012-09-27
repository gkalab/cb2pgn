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

#ifndef _mstl_pair_included
#define _mstl_pair_included

namespace mstl {

template <typename T, typename U>
class pair
{
public:

	typedef T first_type;
	typedef U second_type;

	pair();
	pair(T const& a, U const& b);
	pair(pair const& p);

#if HAVE_0X_MOVE_CONSTRCUTOR_AND_ASSIGMENT_OPERATOR
	pair(pair&& p);
	pair& operator=(pair&& p);
#endif

	pair&	operator=(pair const& p);

public:

	first_type	first;
	second_type	second;
};


template <typename T, typename U> bool operator==(pair<T,U> const& lhs, pair<T,U> const& rhs);
template <typename T, typename U> bool operator!=(pair<T,U> const& lhs, pair<T,U> const& rhs);
template <typename T, typename U> bool operator< (pair<T,U> const& lhs, pair<T,U> const& rhs);

template <typename T, typename U> pair<T,U> make_pair(T const& a, U const& b);


template <typename T> struct is_pod;
template <typename T> struct is_movable;

template <typename T, typename U>
struct is_pod<pair<T,U> > { enum { value = is_pod<T>::value & is_pod<U>::value }; };

template <typename T, typename U>
struct is_movable<pair<T,U> > { enum { value = is_movable<T>::value & is_movable<U>::value }; };

} // namespace mstl

#include "m_pair.ipp"

#endif // _mstl_pair_included

// vi:set ts=3 sw=3:
