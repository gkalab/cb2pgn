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

namespace mstl {

inline
istream&
istream::read(unsigned char* buf, size_t n)
{
	return read(reinterpret_cast<char*>(buf), n);
}


inline
size_t
istream::readsome(unsigned char* buf, size_t n)
{
	return readsome(reinterpret_cast<char*>(buf), n);
}

} // namespace mstl

// vi:set ts=3 sw=3:
