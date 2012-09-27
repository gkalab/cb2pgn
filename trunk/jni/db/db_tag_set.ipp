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

#include "m_assert.h"

namespace db {

inline bool TagSet::contains(tag::ID tag) const					{ return m_set.test(tag); }
inline bool TagSet::contains(mstl::string const& tag) const	{ return find(tag) >= 0; }
inline bool TagSet::isUserSupplied(tag::ID tag) const			{ return m_isUserSupplied[tag]; }
inline Byte TagSet::significance(tag::ID tag) const			{ return m_significance[tag]; }
inline mstl::string const& TagSet::value(tag::ID tag) const	{ return m_values[tag]; }
inline unsigned TagSet::countExtra() const						{ return m_extra.size(); }


inline
tag::ID
TagSet::findFirst() const
{
	static_assert(tag::ExtraTag < 0xff, "reimplementation required");
	return tag::ID(m_set.find_first() & 0xff);
}


inline
tag::ID
TagSet::findNext(tag::ID prev) const
{
	static_assert(tag::ExtraTag < 0xff, "reimplementation required");
	return tag::ID(m_set.find_next(prev) & 0xff);
}


inline
TagSet::Tag const&
TagSet::extra(unsigned index) const
{
	//M_REQUIRE(index < countExtra());
	return m_extra[index];
}


inline
void
TagSet::set(tag::ID tag, mstl::string const& value)
{
	//M_REQUIRE(tag < tag::ExtraTag);

	if (!value.empty())
		set(tag, value, true);
}


inline
void
TagSet::set(tag::ID tag, char const* value, unsigned length)
{
	//M_REQUIRE(tag < tag::ExtraTag);
	//M_REQUIRE(value);

	if (length)
		set(tag, value, length, true);
}


inline
void
TagSet::setSignificance(tag::ID tag, Byte value)
{
	//M_REQUIRE(tag < tag::ExtraTag);
	m_significance[tag] = value;
}

} // namespace db

// vi:set ts=3 sw=3:
