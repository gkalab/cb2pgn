// ======================================================================
// Author : $Author$
// Version: $Revision$
// Date   : $Date$
// Url    : $URL$
// ======================================================================

// ======================================================================
// Copyright: (C) 2010-2012 Gregor Cramer
// ======================================================================

// ======================================================================
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
// ======================================================================

#ifndef _mstl_byte_buf_included
#define _mstl_byte_buf_included

#include "m_types.h"

namespace mstl {

class byte_buf
{
public:

	typedef unsigned char byte;
	typedef bits::size_t size_type;

	typedef byte value_type;

	byte_buf(size_type size);
	byte_buf(size_type size, value_type const* data);
	byte_buf(byte_buf const& buf);
	~byte_buf();

#if HAVE_0X_MOVE_CONSTRCUTOR_AND_ASSIGMENT_OPERATOR
	byte_buf(byte_buf&& buf);
	byte_buf& operator=(byte_buf&& buf);
#endif

	byte_buf& operator=(byte_buf const& buf);

	bool empty() const;

	size_type size() const;

	value_type const* data() const;
	value_type* data();

	void copy(size_type size, value_type const* data, size_type offset = 0);
	void clear();

	void uncompress(byte* dst, unsigned dst_bytes) const;
	static byte_buf* compress(size_type size, value_type const* data);

private:

	value_type*	m_data;
	size_type	m_size;
};

} // namespace mstl

#include "m_byte_buf.ipp"

#endif // _mstl_byte_buf_included

// vi:set ts=3 sw=3:
