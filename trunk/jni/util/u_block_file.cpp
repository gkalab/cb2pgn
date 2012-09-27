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

#include "u_block_file.h"
#include "u_progress.h"

#include "m_algorithm.h"
#include "m_utility.h"
#include "m_bit_functions.h"
#include "m_fstream.h"
#include "m_assert.h"

#include <string.h>

using namespace util;


inline static void
zero(void* p, unsigned size)
{
	memset(p, 0, size);	// NOTE: not really neccessary
}


inline static unsigned
modulo(unsigned a, unsigned m)
{
	unsigned result = a & m;
	return result ? result : m + 1;
}


BlockFile::Buffer::Buffer()
	:m_capacity(0)
	,m_size(0)
	,m_number(BlockFile::InvalidBlock)
	,m_span(0)
	,m_data(0)
{
}


// TODO: use member m_mtime
BlockFile::BlockFile(mstl::fstream* stream, unsigned blockSize, Mode mode)
	:m_stream(stream)
	,m_mode(mode)
	,m_blockSize(blockSize)
	,m_shift(mstl::bf::msb_index(blockSize))
	,m_mask(blockSize - 1)
	,m_mtime(0)
	,m_isDirty(false)
	,m_isClosed(false)
	,m_countWrites(0)
	,m_countReads(0)
{
	//M_REQUIRE(stream);
	//M_REQUIRE(stream->is_open());
	//M_REQUIRE(stream->good());
	//M_REQUIRE(stream->mode() & mstl::ios_base::in);
	//M_REQUIRE(stream->mode() & mstl::ios_base::binary);
	//M_REQUIRE(stream->is_unbuffered());
	//M_REQUIRE(mstl::is_pow_2(blockSize));

	m_mtime = stream->mtime();
	computeBlockCount();
}


BlockFile::BlockFile(mstl::fstream* stream, unsigned blockSize, Mode mode, mstl::string const& magic)
	:m_stream(stream)
	,m_mode(mode)
	,m_blockSize(blockSize)
	,m_shift(mstl::bf::msb_index(blockSize))
	,m_mask(blockSize - 1)
	,m_mtime(0)
	,m_isDirty(false)
	,m_isClosed(false)
	,m_countWrites(0)
{
	//M_REQUIRE(stream);
	//M_REQUIRE(stream->is_open());
	//M_REQUIRE(stream->good());
	//M_REQUIRE(stream->mode() & mstl::ios_base::in);
	//M_REQUIRE(stream->mode() & mstl::ios_base::binary);
	//M_REQUIRE(stream->is_unbuffered());
	//M_REQUIRE(mstl::is_pow_2(blockSize));
	//M_REQUIRE(magic.size() < blockSize);

	m_mtime = stream->mtime();
	computeBlockCount();
	putMagic(magic);
}


BlockFile::BlockFile(unsigned blockSize, Mode mode)
	:m_stream(0)
	,m_mode(mode)
	,m_blockSize(blockSize)
	,m_shift(mstl::bf::msb_index(blockSize))
	,m_mask(blockSize - 1)
	,m_mtime(0)
	,m_isDirty(false)
	,m_isClosed(false)
	,m_countWrites(0)
{
	//M_REQUIRE(mstl::is_pow_2(blockSize));
}


BlockFile::BlockFile(unsigned blockSize, Mode mode, mstl::string const& magic)
	:m_stream(0)
	,m_mode(mode)
	,m_blockSize(blockSize)
	,m_shift(mstl::bf::msb_index(blockSize))
	,m_mask(blockSize - 1)
	,m_mtime(0)
	,m_isDirty(false)
	,m_isClosed(false)
	,m_countWrites(0)
{
	//M_REQUIRE(mstl::is_pow_2(blockSize));
	//M_REQUIRE(magic.size() < blockSize);

	putMagic(magic);
}


BlockFile::~BlockFile() throw()
{
	deallocate();
}


void
BlockFile::computeBlockCount()
{
	size_t size = m_stream->size();

	if (size)
	{
		m_sizeInfo.insert(m_sizeInfo.end(),
								SizeInfo::size_type((size + m_blockSize - 1)/m_blockSize),
								m_blockSize);
		m_sizeInfo.back() = ::modulo(size, m_mask);
	}
}


unsigned
BlockFile::fileSize()
{
	//M_REQUIRE(!isMemoryOnly());
	return m_stream->size();
}


void
BlockFile::putMagic(mstl::string const& magic)
{
	//M_ASSERT(magic.size() < blockSize());

	if (magic.empty() || !m_sizeInfo.empty())
		return;

	resize(1);
	m_buffer.m_size = magic.size() + 1;
	m_buffer.m_number = 0;
	m_buffer.m_span = 1;

	::memcpy(m_buffer.m_data, magic.c_str(), magic.size() + 1);

	if (m_stream == 0)
		m_cache.push_back(m_buffer.m_data);

	m_sizeInfo.push_back(magic.size() + 1);
	m_isDirty = true;
}


bool
BlockFile::isReadWrite() const
{
	return !m_stream || m_stream->mode() & mstl::ios_base::out;
}


inline
unsigned
BlockFile::lastBlockSize() const
{
	//M_ASSERT(!m_sizeInfo.empty());
	return m_sizeInfo.back();
}


unsigned
BlockFile::size() const
{
	if (countBlocks() == 0)
		return 0;

	return fileOffset(countBlocks() - 1) + lastBlockSize();
}


void
BlockFile::deallocate() throw()
{
	if (m_stream)
	{
		delete [] m_buffer.m_data;
		m_buffer.m_data = 0;
	}
	else
	{
		unsigned i = 0;

		while (i < m_cache.size())
		{
			delete [] m_cache[i];
			i += mstl::max(1u, countSpans(m_sizeInfo[i]));
		}

		m_cache.clear();
	}
}


bool
BlockFile::close()
{
	if (m_isClosed)
		return true;

	bool rc = !m_isDirty || sync();

	deallocate();
	m_buffer.m_size = 0;
	m_buffer.m_capacity = 0;
	m_buffer.m_number = InvalidBlock;
	m_countReads = 0;
	m_countWrites = 0;
	m_isDirty = false;
	m_isClosed = true;
	m_sizeInfo.clear();

	if (m_stream)
	{
		m_stream->close();
		m_stream = 0;
	}

	return rc;
}


bool
BlockFile::attach(mstl::fstream* stream, Progress* progress)
{
	//M_REQUIRE(isOpen());
	//M_REQUIRE(isMemoryOnly());
	//M_REQUIRE(stream);
	//M_REQUIRE(stream->is_open());
	//M_REQUIRE(stream->good());
	//M_REQUIRE(stream->mode() & mstl::ios_base::in);
	//M_REQUIRE(stream->mode() & mstl::ios_base::out);
	//M_REQUIRE(stream->mode() & mstl::ios_base::binary);
	//M_REQUIRE(stream->is_unbuffered());

	//M_ASSERT(m_sizeInfo.size() == m_cache.size());

	m_mtime = stream->mtime();

	stream->flush();
	stream->seekp(0);

	bool rc = save(*stream, progress);
	m_countWrites = m_cache.size();

	// we keep the current block, because it may be part of a span of blocks
	Byte* data = new Byte[mstl::mul2(m_buffer.m_capacity)];
	::memcpy(data, m_buffer.m_data, m_buffer.m_size);
	m_buffer.m_data = 0;
	deallocate();
	m_buffer.m_data = data;
	m_stream = stream;
	if (size() == 0)
		computeBlockCount();

	return rc;
}


bool
BlockFile::save(mstl::ostream& stream, Progress* progress)
{
	//M_REQUIRE(isOpen());
	//M_REQUIRE(isMemoryOnly());
//	//M_REQUIRE(stream.is_open());
	//M_REQUIRE(stream.good());
	//M_REQUIRE(stream.mode() & mstl::ios_base::binary);
//	//M_REQUIRE(stream.is_unbuffered());

	//M_ASSERT(m_sizeInfo.size() == m_cache.size());

	if (!m_cache.empty())
	{
		unsigned size = m_cache.size();
		unsigned progressFrequency;
		unsigned progressReportAfter	= unsigned(-1);

		if (progress)
		{
			progressFrequency = progress->frequency(size, 1000);
			progressReportAfter = progressFrequency;
			progress->start(size);
		}

		for (unsigned i = 0, n = size - 1; i < n; ++i)
		{
			if (progressReportAfter == i)
			{
				progress->update(i);
				progressReportAfter += progressFrequency;
			}

			if (__builtin_expect(!stream.write(m_cache[i], m_blockSize), 0))
			{
				if (progress)
					progress->finish();

				return false;
			}
		}

		if (progress)
			progress->finish();

		if (__builtin_expect(!stream.write(m_cache.back(), m_sizeInfo.back()), 0))
			return false;
	}

	m_isDirty = false;
	return true;
}


bool
BlockFile::sync()
{
	//M_REQUIRE(isOpen());

	if (!m_stream || !m_isDirty || m_buffer.m_number == InvalidBlock)
		return true;

	//M_ASSERT(m_buffer.m_data);

	m_stream->seekp(fileOffset(m_buffer.m_number), mstl::ios_base::beg);
	//M_ASSERT(m_stream->tellp() == m_buffer.m_number*m_blockSize);

	if (__builtin_expect(!m_stream->write(m_buffer.m_data, m_buffer.m_size), 0))
		return false;

	m_countWrites += countSpans(m_buffer.m_size);
	m_isDirty = false;
	m_mtime = m_stream->mtime();

	return true;
}


bool
BlockFile::resize(unsigned span)
{
	//M_ASSERT(span >= 1);

	if (m_buffer.m_capacity >= fileOffset(span))
		return false;

	m_buffer.m_capacity = fileOffset(span);
	delete [] m_buffer.m_data;

	// IMPORTANT NOTE:
	// If the block file is on disk we have to use a double sized buffer. This allows
	// some optimizations (on user side).

	unsigned capacity = m_stream ? mstl::mul2(m_buffer.m_capacity) : m_buffer.m_capacity;
	m_buffer.m_data = new Byte[capacity];

	return true;
}


unsigned
BlockFile::fetch(unsigned blockNumber, unsigned span)
{
	//M_ASSERT(isOpen());
	//M_ASSERT(countBlocks() > 0);
	//M_ASSERT(blockNumber < m_sizeInfo.size());
	//M_ASSERT(blockNumber + span <= countBlocks());

	if (m_buffer.m_number != blockNumber || m_buffer.m_span < span)
	{
		if (m_stream)
		{
			if (m_isDirty && !sync())	 // XXX not mt-safe
				return SyncFailed;

			//M_ASSERT(span > 0);

			if (span > 1)
			{
				//M_ASSERT(m_sizeInfo[blockNumber] <= span*m_blockSize);
				m_sizeInfo[blockNumber] = span*m_blockSize; // XXX not mt-safe
			}

			m_stream->seekg(blockNumber*m_blockSize, mstl::ios_base::beg);

			resize(span);
			m_buffer.m_size = m_sizeInfo[blockNumber];
			m_buffer.m_span = span;

			if (__builtin_expect(!m_stream->read(m_buffer.m_data, m_buffer.m_size), 0))
				return ReadError;

			m_countReads += span;
		}
		else
		{
			//M_ASSERT(blockNumber < m_cache.size());

			m_buffer.m_size = m_sizeInfo[blockNumber];
			m_buffer.m_data = m_cache[blockNumber];
		}

		m_buffer.m_number = blockNumber;
	}

	return 0;
}


unsigned
BlockFile::retrieve(unsigned blockNumber, unsigned offset)
{
	//M_ASSERT(isOpen());
	//M_ASSERT(countBlocks() > 0);
	//M_ASSERT(blockNumber < m_sizeInfo.size());
	//M_ASSERT(blockNumber < countBlocks());
	//M_ASSERT(offset < m_blockSize);
	//M_ASSERT(m_mode == ReadWriteLength);

	if (unsigned rc = fetch(blockNumber, 1))
		return rc;

	return ByteStream::uint24(m_buffer.m_data + offset);
}


void
BlockFile::copy(ByteStream const& buf, unsigned offset, unsigned nbytes)
{
	//M_ASSERT(offset + nbytes <= m_buffer.m_capacity);

	unsigned char* data = m_buffer.m_data + offset;

	if (m_mode == ReadWriteLength)
	{
		ByteStream::set(data, ByteStream::uint24_t(nbytes));
		::memcpy(data + 3, buf.data(), nbytes - 3);
	}
	else
	{
		::memcpy(data, buf.data(), nbytes);
	}
}


unsigned
BlockFile::put(ByteStream const& buf, unsigned offset, unsigned minSize)
{
	//M_REQUIRE(isOpen());
	//M_REQUIRE(isReadWrite());
	//M_REQUIRE(minSize <= MaxSpanSize);
	//M_REQUIRE(buf.size() <= MaxSpanSize);
	//M_REQUIRE(offset/blockSize() < countBlocks());
	//M_REQUIRE(	(offset + minSize - 1)/blockSize() == offset/blockSize()	// fits into a single block
	//			|| offset % blockSize() == 0);										// or starts at block offset 0

	unsigned nbytes = buf.size();

	if (nbytes == 0)
		return 0;

	unsigned blockNo		= blockNumber(offset);
	unsigned blockOffset	= this->blockOffset(offset);

	if (m_mode == ReadWriteLength)
	{
		minSize = retrieve(blockNo, blockOffset);

		if (minSize > MaxFileSize)
			return minSize;	// it's an error code

		nbytes += 3;
	}

	unsigned newSpan = countSpans(nbytes);
	unsigned oldSpan = countSpans(minSize);

	if (nbytes <= minSize)
	{
		if (m_buffer.m_number != blockNo)
		{
			if (unsigned rc = fetch(blockNo, oldSpan))
				return rc;
		}

		resize(newSpan);
		copy(buf, blockOffset, nbytes);
		::zero(m_buffer.m_data + blockOffset + minSize, minSize - nbytes);
		m_isDirty = true;

		if (newSpan < oldSpan)
		{
			m_buffer.m_capacity = fileOffset(newSpan);
			m_buffer.m_size = nbytes;
			m_buffer.m_span = newSpan;

			unsigned span = oldSpan - newSpan - 1;

			if (span == 0)
			{
				if (m_buffer.m_number + newSpan == countBlocks())
				{
					if (m_sizeInfo[m_buffer.m_number] == minSize)
						m_sizeInfo[m_buffer.m_number + newSpan] = 0;

					m_sizeInfo[m_buffer.m_number] = nbytes;
				}
			}
			else
			{
				if (unsigned rc = fetch(blockNo + newSpan))
					return rc;

				m_buffer.m_size = 0;
				m_buffer.m_capacity = fileOffset(span);
				m_sizeInfo[m_buffer.m_number] = 0;
			}
		}
		else if (m_buffer.m_size == blockOffset + minSize)
		{
			m_sizeInfo[m_buffer.m_number] = (m_buffer.m_size -= minSize - nbytes);
		}
	}
	else if (	blockNo == countBlocks() - 1
				&& m_sizeInfo[blockNo] == blockOffset + minSize
				&& ::modulo(offset, m_mask) + nbytes <= m_blockSize)
	{
		resize(newSpan);
		copy(buf, blockOffset, nbytes);
		m_isDirty = true;
		m_sizeInfo[blockNo] = (m_buffer.m_size += nbytes - minSize);
	}
	// TODO:
	// else if (	newSpan > 1
	//				&& (newSpan == oldSpan || blockNo + oldSpan == countBlocks())
	//				&& m_sizeInfo[blockNo + oldSpan - 1] == ::modulo(offset + minSize, m_mask))
	// {
	// }
	else
	{
		offset = put(buf);
	}

	return offset;
}


unsigned
BlockFile::put(ByteStream const& buf)
{
	//M_REQUIRE(isOpen());
	//M_REQUIRE(isReadWrite());
	//M_REQUIRE(buf.size() <= MaxSpanSize);

	size_t nbytes = buf.size();

	if (nbytes == 0)
		return 0;

	if (m_mode == ReadWriteLength)
		nbytes += 3;

	size_t span = countSpans(nbytes);

	if (m_buffer.m_capacity == 0)
	{
		//M_ASSERT(m_buffer.m_number == InvalidBlock);
		//M_ASSERT(m_buffer.m_size == 0);
		//M_ASSERT(m_isDirty == false);

		// create first block
		resize(span);

		if (m_sizeInfo.empty())
		{
			if (m_stream == 0)
				m_cache.push_back(m_buffer.m_data);

			m_sizeInfo.push_back(0);
			m_buffer.m_number = 0;
		}
		else if ((m_buffer.m_size = m_sizeInfo.back()) + nbytes <= m_blockSize)
		{
			if (unsigned rc = fetch(m_sizeInfo.size() - 1, span))
				return rc;
		}
	}

	//M_ASSERT(m_buffer.m_data);
	//M_ASSERT(m_buffer.m_number != InvalidBlock);
	//M_ASSERT(!m_sizeInfo.empty());

	if (m_buffer.m_size + nbytes <= m_blockSize)
	{
		// use current block
	}
	else if (lastBlockSize() + nbytes <= m_blockSize)
	{
		// use last block
		if (unsigned rc = fetch(countBlocks() - 1))
			return rc;
	}
	else
	{
		// need new block

		if (fileOffset(countBlocks() + 1) > MaxFileSize)
			return MaxFileSizeExceeded;

		if (m_stream)
		{
			if (!sync())
				return SyncFailed;

			resize(span);
			::zero(m_buffer.m_data, m_buffer.m_capacity);
		}
		else
		{
			m_buffer.m_size = fileOffset(span);
			m_buffer.m_data = new Byte[m_buffer.m_size];
			m_buffer.m_capacity = mstl::max(m_buffer.m_capacity, m_buffer.m_size);

			for (unsigned i = 0; i < span; ++i)
				m_cache.push_back(m_buffer.m_data + fileOffset(i));
		}

		m_buffer.m_number = m_sizeInfo.size();
		m_buffer.m_size = 0;

		if (span >= 2)
		{
			m_sizeInfo.push_back(fileOffset(span));
			m_sizeInfo.insert(m_sizeInfo.end(), SizeInfo::size_type(span - 2), m_blockSize);
		}

		m_sizeInfo.push_back(0);
	}

	//M_ASSERT(m_buffer.m_size + nbytes <= m_buffer.m_capacity);
	//M_ASSERT(m_buffer.m_size + nbytes <= m_blockSize || nbytes > m_blockSize);

	copy(buf, m_buffer.m_size, nbytes);
	m_isDirty = true;

	unsigned offset = fileOffset(m_buffer.m_number) + m_buffer.m_size;

	//M_ASSERT(offset <= MaxFileSize);

	m_buffer.m_size += nbytes;
	m_sizeInfo[m_buffer.m_number + span - 1] = ::modulo(m_buffer.m_size, m_mask);
	return offset;
}


unsigned
BlockFile::get(ByteStream& result, unsigned offset, unsigned size)
{
	//M_REQUIRE(isOpen());
	//M_REQUIRE(size <= MaxSpanSize);
	//M_REQUIRE(offset + size <= this->size());
	//M_REQUIRE(	(offset + size - 1)/blockSize() == offset/blockSize()	// fits into a single block
	//			|| offset % blockSize() == 0);									// or starts at block offset 0

	if (m_mode == ReadWriteLength)
	{
		size = retrieve(blockNumber(offset), blockOffset(offset));

		if (size <= MaxFileSize)	// otherwise it's an error code
		{
			if (unsigned span	= countSpans(size) > 1)
			{
				if (unsigned rc = fetch(blockNumber(offset), span))
					return rc;
			}

			result.setup(m_buffer.m_data + blockOffset(offset) + 3, size - 3);
		}
	}
	else if (size > 0)
	{
		unsigned rc = fetch(blockNumber(offset), countSpans(size));

		if (rc <= MaxFileSize)	// otherwise it's an error code
			result.setup(m_buffer.m_data + blockOffset(offset), size);
		else
			size = rc;
	}

	return size;
}

// vi:set ts=3 sw=3:
