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

#ifndef _mstl_algobase_included
#define _mstl_algobase_included

#include "m_types.h"

namespace mstl {

template <typename InputIterator, typename OutputIterator>
OutputIterator copy(InputIterator first, InputIterator last, OutputIterator result);

template <typename InputIterator, typename OutputIterator>
OutputIterator copy_backward(InputIterator first, InputIterator last, OutputIterator result);

template <typename ForwardIterator, typename T>
void fill(ForwardIterator first, ForwardIterator last, T const& value);

template <typename OutputIterator, typename T>
OutputIterator fill_n(OutputIterator first, unsigned count, T const& value);

template <typename BidirectionalIterator>
void reverse(BidirectionalIterator first, BidirectionalIterator last);

} // namespace mstl

#include "m_algobase.ipp"

#endif // _mstl_algobase_included

// vi:set ts=3 sw=3:
