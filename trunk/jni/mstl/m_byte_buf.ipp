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

#include "m_utility.h"
#include "m_assert.h"

#include <string.h>

namespace mstl {

inline bool byte_buf::empty() const									{ return m_size == 0; }
inline byte_buf::size_type byte_buf::size() const				{ return m_size; }
inline byte_buf::value_type const* byte_buf::data() const	{ return m_data; }
inline byte_buf::value_type* byte_buf::data()					{ return m_data; }
inline void byte_buf::clear()											{ ::memset(m_data, 0, m_size); }

inline byte_buf::byte_buf(size_type size) : m_data(new value_type[size]), m_size(size) {}
inline byte_buf::~byte_buf() { delete [] m_data; }


inline
byte_buf::byte_buf(size_type size, value_type const* data)
	:m_data(new value_type[size])
	,m_size(size)
{
	//M_REQUIRE(data || size == 0);
	::memcpy(m_data, data, size);
}


inline
byte_buf::byte_buf(byte_buf const& buf)
	:m_data(new value_type[buf.m_size])
	,m_size(buf.m_size)
{
	::memcpy(m_data, buf.m_data, m_size);
}


#if HAVE_0X_MOVE_CONSTRCUTOR_AND_ASSIGMENT_OPERATOR

inline
byte_buf::byte_buf(byte_buf&& buf)
	:m_data(buf.m_data)
	,m_size(buf.m_size)
{
	buf.m_data = 0;
}


inline
byte_buf&
byte_buf::operator=(byte_buf&& buf)
{
	if (this != &buf)
	{
		swap(m_data, buf.m_data);
		m_size = buf.m_size;
	}

	return *this;
}

#endif

} // namespace mstl

// vi:set ts=3 sw=3:
