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

#ifndef _mstl_bit_functions_included
#define _mstl_bit_functions_included

namespace mstl {
namespace bf {

template <class T> unsigned count_bits(T x);
template <class T> unsigned msb_index(T x);
template <class T> unsigned lsb_index(T x);
template <class T> T reverse(T x);

} // namespace bf
} // namespace mstl

#include "m_bit_functions.ipp"

#endif // _mstl_bit_functions_included

// vi:set ts=3 sw=3:
