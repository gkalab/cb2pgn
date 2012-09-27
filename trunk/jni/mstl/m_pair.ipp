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

#include "m_utility.h"

namespace mstl {

template <typename T, typename U> inline pair<T,U>::pair() : first(T()), second(U()) {}

template <typename T, typename U>
inline pair<T,U>::pair(T const& a, U const& b) : first(a), second(b) {}

template <typename T, typename U>
inline pair<T,U>::pair(pair const& p) : first(p.first), second(p.second) {}


template <typename T, typename U>
inline
pair<T,U>& pair<T,U>::operator=(pair const& p)
{
	first = p.first;
	second = p.second;

	return *this;
}


template <typename T, typename U>
inline
bool
operator==(pair<T,U> const& lhs, pair<T,U> const& rhs)
{
	return lhs.first == rhs.first && lhs.second == rhs.second;
}


template <typename T, typename U>
inline
bool
operator!=(pair<T,U> const& lhs, pair<T,U> const& rhs)
{
	return !(lhs.first == rhs.first && lhs.second == rhs.second);
}


template <typename T, typename U>
inline
bool
operator<(pair<T,U> const& lhs, pair<T,U> const& rhs)
{
	return lhs.first < rhs.first || (lhs.first == rhs.first && lhs.second < rhs.second);
}


template <typename T, typename U>
inline
pair<T,U>
make_pair(T const& a, U const& b)
{
	return pair<T,U>(a, b);
}


#if HAVE_0X_MOVE_CONSTRCUTOR_AND_ASSIGMENT_OPERATOR

template <typename T, typename U>
inline
pair<T,U>::pair(pair&& p) :first(mstl::move(p.first)), second(mstl::move(p.second)) {}


template <typename T, typename U>
inline
pair<T,U>&
pair<T,U>::operator=(pair&& p)
{
	first = mstl::move(p.first);
	second = mstl::move(p.second);
	return *this;
}

#endif

} // namespace mstl

// vi:set ts=3 sw=3:
