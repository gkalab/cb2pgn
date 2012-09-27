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
// Copyright: (C) 2009-2012 Gregor Cramer
// ======================================================================

// ======================================================================
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
// ======================================================================

#ifndef _db_eco_included
#define _db_eco_included

#include "u_base.h"

#include "m_string.h"

namespace db {

class Eco
{
public:

	enum { Bit_Size_Per_Subcode = 20 };
	enum { Sub_Code_Bits = 11 };
	enum { Num_Sub_Codes = 1 << Sub_Code_Bits };
	enum { Max_Code = 5*10*10*Num_Sub_Codes };

	typedef uint32_t Code;

	Eco();
	explicit Eco(char const* s);
	explicit Eco(Code code);

	Eco& operator=(Code code);

	bool operator< (Eco const& eco) const;
	bool operator==(Eco const& eco) const;
	bool operator!=(Eco const& eco) const;

	operator Code () const;

	bool isRoot() const;
	bool isExtendedCode() const;

	Eco basic() const;
	Code code() const;
	Code extension() const;
	uint16_t toShort() const;

	void convert(char* buf, bool shortForm = false) const;
	void setup(char const* buf);
	mstl::string asString() const;
	mstl::string asShortString() const;

	static Eco root();
	static Eco fromShort(uint16_t code);
	static uint16_t toShort(char const* s);

private:

	Code m_code;

	static Eco const m_root;
};

} // namespace db

namespace mstl {

template <typename T> struct is_pod;
template <> struct is_pod< ::db::Eco > { enum { value = 1 }; };

} // namespace mstl

#include "db_eco.ipp"

#endif // _db_eco_included

// vi:set ts=3 sw=3:
