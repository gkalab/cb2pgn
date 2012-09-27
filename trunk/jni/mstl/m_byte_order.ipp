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

#include <byteswap.h>
#include <endian.h>

namespace mstl {
namespace bo {

inline int16_t  swap(int16_t  v) { return bswap_16(v); }
inline int32_t  swap(int32_t  v) { return bswap_32(v); }
inline int64_t  swap(int64_t  v) { return bswap_64(v); }
inline uint16_t swap(uint16_t v) { return bswap_16(v); }
inline uint32_t swap(uint32_t v) { return bswap_32(v); }
inline uint64_t swap(uint64_t v) { return bswap_64(v); }

#if __BYTE_ORDER == __LITTLE_ENDIAN

inline int16_t  swapLE(int16_t  v) { return v; }
inline int32_t  swapLE(int32_t  v) { return v; }
inline int64_t  swapLE(int64_t  v) { return v; }
inline uint16_t swapLE(uint16_t v) { return v; }
inline uint32_t swapLE(uint32_t v) { return v; }
inline uint64_t swapLE(uint64_t v) { return v; }

inline int16_t  swapBE(int16_t  v) { return bswap_16(v); }
inline int32_t  swapBE(int32_t  v) { return bswap_32(v); }
inline int64_t  swapBE(int64_t  v) { return bswap_64(v); }
inline uint16_t swapBE(uint16_t v) { return bswap_16(v); }
inline uint32_t swapBE(uint32_t v) { return bswap_32(v); }
inline uint64_t swapBE(uint64_t v) { return bswap_64(v); }

#elif __BYTE_ORDER == __BIG_ENDIAN

inline int16_t  swapBE(int16_t  v) { return v; }
inline int32_t  swapBE(int32_t  v) { return v; }
inline int64_t  swapBE(int64_t  v) { return v; }
inline uint16_t swapBE(uint16_t v) { return v; }
inline uint32_t swapBE(uint32_t v) { return v; }
inline uint64_t swapBE(uint64_t v) { return v; }

inline int16_t  swapLE(int16_t  v) { return bswap_16(v); }
inline int32_t  swapLE(int32_t  v) { return bswap_32(v); }
inline int64_t  swapLE(int64_t  v) { return bswap_64(v); }
inline uint16_t swapLE(uint16_t v) { return bswap_16(v); }
inline uint32_t swapLE(uint32_t v) { return bswap_32(v); }
inline uint64_t swapLE(uint64_t v) { return bswap_64(v); }

#else

error "Unsupported byte order (neither Big Endian nor Little Endian)"

#endif

} // namespace bo
} // namespace mstl

// vi:set ts=3 sw=3:
