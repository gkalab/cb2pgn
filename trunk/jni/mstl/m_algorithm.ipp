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
namespace bits {

void rotate_fast(void* first, void* middle, void* last);

}


/// Returns the first iterator i in the range [first, last) such that
/// *i == value. Returns last if no such iterator exists.
/// \ingroup SearchingAlgorithms
template <typename ForwardIterator, typename EqualityComparable>
inline
ForwardIterator
find(ForwardIterator first, ForwardIterator last, const EqualityComparable& value)
{
	while (first != last && !(*first == value))
		++first;
	return (first);
}


/// Returns the furthermost iterator i in [first, last) such that,
/// for every iterator j in [first, i), *j < value.
/// Assumes the range is sorted.
/// \ingroup SearchingAlgorithms
template <typename ForwardIterator, typename LessThanComparable>
inline
ForwardIterator
lower_bound(ForwardIterator first, ForwardIterator last, LessThanComparable const& value)
{
	while (first != last)
	{
		ForwardIterator mid = advance(first, div2(distance(first, last)));

		if (*mid < value)
			first = advance(mid, 1);
		else
			last = mid;
	}

	return first;
}


/// Returns the furthermost iterator i in [first,last) such that for
/// every iterator j in [first,i), value < *j is false.
/// Assumes the range is sorted.
/// \ingroup SearchingAlgorithms
template <typename ForwardIterator, typename LessThanComparable>
inline
ForwardIterator
upper_bound(ForwardIterator first, ForwardIterator last, LessThanComparable const& value)
{
	while (first != last)
	{
		ForwardIterator mid = advance(first, div2(distance(first, last)));

		if (value < *mid)
			last = mid;
		else
			first = advance(mid, 1);
	}

	return last;
}


/// Performs a binary search inside the sorted range.
/// \ingroup SearchingAlgorithms
template <typename ForwardIterator, typename LessThanComparable>
inline
ForwardIterator
binary_search(ForwardIterator first, ForwardIterator last, LessThanComparable const& value)
{
    ForwardIterator found = lower_bound(first, last, value);
    return (found == last || value < *found) ? last : found;
}


/// \brief Exchanges ranges [first, middle) and [middle, last)
/// \ingroup MutatingAlgorithms
template <typename ForwardIterator>
inline
ForwardIterator
rotate(ForwardIterator first, ForwardIterator middle, ForwardIterator last)
{
	if (first == middle || middle == last)
		return (first);

	reverse (first, middle);
	reverse (middle, last);

	for ( ; first != middle && middle != last; ++first)
		swap(*first, *--last);

	reverse (first, (first == middle ? last : middle));
	return first;
}


/// Specialization for pointers, which can be treated identically.
template <typename T>
inline
T*
rotate(T* first, T* middle, T* last)
{
    bits::rotate_fast(first, middle, last);
    return first;
}

} // namespace mstl

// vi:set ts=3 sw=3:
