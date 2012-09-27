// ======================================================================
// Author : $Author$
// Version: $Revision$
// Date   : $Date$
// Url    : $URL$
// ======================================================================

// ======================================================================
// Copyright: (C) 2012 Gregor Cramer
// ======================================================================

// ======================================================================
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
// ======================================================================

#include "m_exception.h"

namespace util {
namespace html {

inline mstl::string const& Hyphenate::result() const			{ return m_result; }
inline mstl::string const& BuildLigatures::result() const	{ return m_result; }

inline bool Search::tooManyMatches() const				{ return m_tooManyMatches; }
inline unsigned Search::countMatches() const				{ return m_posList.size(); }
inline mstl::string const& Search::title() const		{ return m_title; }


inline
unsigned
Search::matchPosition(unsigned i) const
{
	M_REQUIRE(i < countMatches());
	return m_posList[i];
}

} // namespace html
} // namespace util

// vi:set ts=3 sw=3:
