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

#ifndef _u_lzo_byte_stream_included
#define _u_lzo_byte_stream_included

#include "u_byte_stream.h"
#include "u_lzo.h"

namespace mstl { class iostream; }

namespace util {

class LzoByteStream : public ByteStream
{
public:

	enum { Chunk_Size = 32768 };

	LzoByteStream(mstl::iostream& strm);
	~LzoByteStream() throw();

	void flush() override;

protected:

	void underflow(unsigned size) override;
	void overflow(unsigned size) override;

private:

	enum { Additional_Size	= 8192 };
	enum { Overhead_Size		= U_LZO_OVERHEAD_COMPRESS(Chunk_Size) };
	enum { Buf_Size			= Chunk_Size + Overhead_Size + Additional_Size };

	mstl::iostream&	m_strm;
	unsigned char		m_in[Chunk_Size];
	unsigned char		m_out[Buf_Size];
};

} // namespace util

#endif // _u_lzo_byte_stream_included

// vi:set ts=3 sw=3:
