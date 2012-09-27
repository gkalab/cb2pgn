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

namespace util {

inline bool BlockFile::isClosed() const			{ return m_isClosed; }
inline bool BlockFile::isOpen() const				{ return !m_isClosed; }
inline bool BlockFile::isEmpty() const				{ return m_cache.empty(); }
inline bool BlockFile::isMemoryOnly() const		{ return m_stream == 0; }
inline bool BlockFile::isReadOnly() const			{ return !isReadWrite(); }

inline BlockFile::Mode BlockFile::mode() const	{ return m_mode; }

inline unsigned BlockFile::blockSize() const		{ return m_blockSize; }
inline unsigned BlockFile::countBlocks() const	{ return m_sizeInfo.size(); }
inline unsigned BlockFile::countReads() const	{ return m_countReads; }
inline unsigned BlockFile::countWrites() const	{ return m_countWrites; }

inline unsigned BlockFile::blockNumber(unsigned fileOffset) const	{ return fileOffset >> m_shift; }
inline unsigned BlockFile::blockOffset(unsigned fileOffset) const	{ return fileOffset & m_mask; }
inline unsigned BlockFile::fileOffset(unsigned blockNumber) const	{ return blockNumber << m_shift; }


inline
unsigned
BlockFile::countSpans(unsigned size) const
{
	return (size + m_blockSize - 1) >> m_shift;
}

} // namespace db

// vi:set ts=3 sw=3:
