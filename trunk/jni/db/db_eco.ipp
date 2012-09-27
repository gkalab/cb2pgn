// ======================================================================
// Author : $Author$
// Version: $Revision$
// Date   : $Date$
// Url    : $URL$
// ======================================================================

// ======================================================================
//    _/|            __
//   // o\         /    )           ,        /    /
//   || ._)    ----\---------__----------__-/----/__-
//   //__\          \      /   '  /    /   /    /   )
//   )___(     _(____/____(___ __/____(___/____(___/_
// ======================================================================

// ======================================================================
// Copyright: (C) 2008-2012 Gregor Cramer
// ======================================================================

// ======================================================================
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
// ======================================================================

#include "m_assert.h"

namespace db {

inline
Eco::Eco()
	:m_code(0)
{
	static_assert(Bit_Size_Per_Subcode >= 16, "not enough room for 16bit move");
}


inline Eco::Eco(Code code) : m_code(code) {}
inline Eco::Eco(char const* s) : m_code(0) { setup(s); }

inline Eco::operator Eco::Code () const	{ return m_code; }
inline Eco::Code Eco::code() const			{ return m_code; }
inline Eco Eco::root()							{ return m_root; }

inline bool Eco::isRoot() const { return m_code == m_root; }

inline bool Eco::operator< (Eco const& eco) const { return m_code <  eco.m_code; }
inline bool Eco::operator==(Eco const& eco) const { return m_code == eco.m_code; }
inline bool Eco::operator!=(Eco const& eco) const { return m_code != eco.m_code; }


inline
Eco&
Eco::operator=(Code code)
{
	m_code = code;
	return *this;
}


inline
bool
Eco::isExtendedCode() const
{
	return m_code && ((m_code - 1) & (Num_Sub_Codes - 1));
}


inline
Eco::Code
Eco::extension() const
{
	return m_code ? (m_code - 1) & (Num_Sub_Codes - 1) : 0;
}


inline
Eco
Eco::basic() const
{
	return Eco(m_code - extension());
}


inline
uint16_t
Eco::toShort() const
{
	return m_code ? ((m_code - 1) >> Sub_Code_Bits) + 1 : 0;
}


inline
Eco
Eco::fromShort(uint16_t code)
{
	//M_REQUIRE(code <= 5*10*10);
	return Eco(code ? (uint32_t(code - 1) << Sub_Code_Bits) + 1 : 0);
}

} // namespace db

// vi:set ts=3 sw=3:
