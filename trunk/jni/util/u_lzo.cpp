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

#include "u_lzo.h"
#include "u_byte_stream.h"
#include "u_exception.h"

#include "m_assert.h"

#include "lzo/minilzo.h"

#include <string.h>
#include <assert.h>

using namespace util;


static void
__attribute__((constructor))
initialize()
{
	__attribute__((unused)) int rc = lzo_init();
	assert(rc == LZO_E_OK && "lzo_init() failed");
}


static lzo_align_t workingMem[(LZO1X_1_MEM_COMPRESS + sizeof(lzo_align_t) - 1)/sizeof(lzo_align_t)];


unsigned
Lzo::maxSizeAfterCompression(unsigned sizeBeforeCompression)
{
	return U_LZO_OVERHEAD_COMPRESS(sizeBeforeCompression);
}


bool
Lzo::compress(ByteStream const& src, ByteStream& dst)
{
	M_REQUIRE(dst.tellp() == 0);
	M_REQUIRE(src.base() != dst.base());

	lzo_uint outLen;

	dst.reserve(maxSizeAfterCompression(src.size()));

	if (lzo1x_1_compress(src.data(), src.size(), dst.buffer(), &outLen, ::workingMem) != LZO_E_OK)
		U_RAISE("LZO compression failed");

	if (outLen >= src.size())
		return false;

	dst.provide(outLen);
	return true;
}


void
Lzo::decompress(ByteStream const& src, ByteStream& dst)
{
	// The user has to know the maximal output size, thus the
	// following check is insufficient, but more than nothing.
	M_REQUIRE(dst.free() >= src.size());
	M_REQUIRE(src.base() != dst.base());

	if (src.size() == dst.free())
	{
		::memcpy(dst.buffer(), src.data(), src.size());
	}
	else
	{
		lzo_uint outLen = dst.free();

		if (lzo1x_decompress_safe(src.data(), src.size(), dst.buffer(), &outLen, 0) != LZO_E_OK)
			U_RAISE("LZO decompression: corrupt data");

		dst.provide(outLen);
	}
}

// vi:set ts=3 sw=3:
