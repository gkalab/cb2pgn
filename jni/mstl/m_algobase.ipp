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

/// Copy copies elements from the range [first, last) to the range
/// [result, result + (last - first)). That is, it performs the assignments
/// *result = *first, *(result + 1) = *(first + 1), and so on. Generally,
/// for every integer n from 0 to last - first, copy performs the assignment
/// *(result + n) = *(first + n). Assignments are performed in forward order,
/// i.e. in order of increasing n.
/// \ingroup MutatingAlgorithms
template <typename InputIterator, typename OutputIterator>
inline
OutputIterator
copy(InputIterator first, InputIterator last, OutputIterator result)
{
	for ( ; first != last; ++result, ++first)
		*result = *first;

	return result;
}


/// \brief Copy copies elements from the range (last, first] to result.
/// \ingroup MutatingAlgorithms
/// Copies elements starting at last, decrementing both last and result.
template <typename InputIterator, typename OutputIterator>
inline
OutputIterator
copy_backward(InputIterator first, InputIterator last, OutputIterator result)
{
   while (first != last)
      *--result = *--last;

   return result;
}


/// Fill assigns the value value to every element in the range [first, last).
/// That is, for every iterator i in [first, last),
/// it performs the assignment *i = value.
/// \ingroup GeneratorAlgorithms
template <typename ForwardIterator, typename T>
inline
void
fill(ForwardIterator first, ForwardIterator last, T const& value)
{
   for (; first != last; ++first)
      *first = value;
}


/// Fill_n assigns the value value to every element in the range
/// [first, first+count). That is, for every iterator i in [first, first+count),
/// it performs the assignment *i = value. The return value is first + count.
/// \ingroup GeneratorAlgorithms
template <typename OutputIterator, typename T>
inline
OutputIterator
fill_n(OutputIterator first, unsigned count, T const& value)
{
	for ( ; count; --count, ++first)
		*first = value;

	return first;
}


/// \brief Reverse reverses a range.
/// That is: for every i such that 0 <= i <= (last - first) / 2),
/// it exchanges *(first + i) and *(last - (i + 1)).
/// \ingroup MutatingAlgorithms
///
template <typename BidirectionalIterator>
inline
void
reverse(BidirectionalIterator first, BidirectionalIterator last)
{
	for (; distance(first, --last) > 0; ++first)
		swap(*first, *last);
}

} // namespace mstl

// vi:set ts=3 sw=3:
