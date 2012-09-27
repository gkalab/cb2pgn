// ======================================================================
// Author : $Author$
// Version: $Revision$
// Date   : $Date$
// Url    : $URL$
// ======================================================================

// ======================================================================
// Copyright: (C) 2012 Gregor Cramer
// ======================================================================

// ======================================================================
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
// ======================================================================

namespace util {

inline bool ZlibOStream::isOpen() const					{ return m_dst != 0; }
inline uint32_t ZlibOStream::crc() const					{ return m_crc; }
inline unsigned ZlibOStream::size() const					{ return m_size; }
inline unsigned ZlibOStream::compressedSize() const	{ return m_compressedSize; }

} // namespace util

// vi:set ts=3 sw=3:
