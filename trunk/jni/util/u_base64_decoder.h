// ======================================================================
// Author : $Author$
// Version: $Revision$
// Date   : $Date$
// Url    : $URL$
// ======================================================================

// ======================================================================
// Copyright: (C)2011-2012 Gregor Cramer
// ======================================================================

// ======================================================================
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
// ======================================================================

#ifndef u_base64_decoder_included
#define u_base64_decoder_included

#include "m_types.h"

namespace util {

class Base64Decoder
{
public:

	Base64Decoder(unsigned char const* first, unsigned char const* last);

	size_t read(unsigned char* buf, size_t len);
	bool skip(size_t nbytes);

	void reset();

private:

	unsigned char const*	m_base;
	unsigned char const*	m_current;
	unsigned char const*	m_end;
	unsigned char			m_last;
	unsigned					m_state;
};

} // namespace util

#endif // u_base64_decoder_included

// vi:set ts=3 sw=3:
