// ======================================================================
// $RCSfile: m_bit_functions.h,v $
// $Revision$
// $Date$
// $Author: gregor $
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

#ifndef _mstl_byte_order_included
#define _mstl_byte_order_included

namespace mstl {
namespace bo {

int16_t  swap(int16_t  v);
int32_t  swap(int32_t  v);
int64_t  swap(int64_t  v);
uint16_t swap(uint16_t v);
uint32_t swap(uint32_t v);
uint64_t swap(uint64_t v);

int16_t  swapLE(int16_t  v);
int32_t  swapLE(int32_t  v);
int64_t  swapLE(int64_t  v);
uint16_t swapLE(uint16_t v);
uint32_t swapLE(uint32_t v);
uint64_t swapLE(uint64_t v);

int16_t  swapBE(int16_t  v);
int32_t  swapBE(int32_t  v);
int64_t  swapBE(int64_t  v);
uint16_t swapBE(uint16_t v);
uint32_t swapBE(uint32_t v);
uint64_t swapBE(uint64_t v);

} // namespace bo
} // namespace mstl

#include "m_byte_order.ipp"

#endif // _mstl_byte_order_included

// vi:set ts=3 sw=3:
