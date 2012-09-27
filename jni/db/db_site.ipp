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
// Copyright: (C) 2010-2012 Gregor Cramer
// ======================================================================

// ======================================================================
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
// ======================================================================

#include <m_assert.h>

namespace db {

inline mstl::string const& Site::name() const	{ return m_name; }
inline unsigned Site::countCountries() const		{ return m_ascii.size(); }


inline
mstl::string const&
Site::nonDiacriticName(country::Code country) const
{
	// M_REQUIRE(containsCountry(country));
	return m_ascii[findCountry(country)].name;
}


inline
country::Code
Site::country(unsigned n) const
{
	// M_REQUIRE(n < countCountries());
	return m_ascii[n].country;

}

} // namespace db

// vi:set ts=3 sw=3:
