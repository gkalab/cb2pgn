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

#ifndef _db_tag_set_included
#define _db_tag_set_included

#include "db_common.h"

#include "u_crc.h"

#include "m_string.h"
#include "m_vector.h"
#include "m_bitfield.h"

namespace db {

class TagSet
{
public:

	struct Tag
	{
		mstl::string name;
		mstl::string value;
	};

	TagSet();
	TagSet(TagSet const& set);

#if HAVE_0X_MOVE_CONSTRCUTOR_AND_ASSIGMENT_OPERATOR
	TagSet(TagSet&& set);
	TagSet& operator=(TagSet&& set);
#endif

	TagSet& operator=(TagSet const& set);

	bool contains(tag::ID tag) const;
	bool contains(mstl::string const& tag) const;
	bool isUserSupplied(tag::ID tag) const;

	unsigned countExtra() const;
	util::crc::checksum_t computeChecksum(util::crc::checksum_t crc = 0) const;

	mstl::string const& value(tag::ID tag) const;
	int asInt(tag::ID tag) const;
	Tag const& extra(unsigned index) const;
	Byte significance(tag::ID tag) const;

	void set(mstl::string const& name, mstl::string const& value);
	void set(char const* name, unsigned nameLen, char const* value, unsigned valueLen);
	void set(mstl::string const& name, char const* value, unsigned valueLen);
	void setExtra(mstl::string const& name, mstl::string const& value);
	void setExtra(char const* name, unsigned nameLen, char const* value, unsigned valueLen);
	void set(tag::ID tag, char const* value, unsigned length);
	void set(tag::ID tag, mstl::string const& value);
	void set(tag::ID tag, unsigned value);

	void add(tag::ID tag, char const* value, unsigned length);
	void add(tag::ID tag, mstl::string const& value);
	void add(tag::ID tag, unsigned value);

	void replace(tag::ID tag, char const* value, unsigned length);
	void replace(tag::ID tag, mstl::string const& value);
	void replace(tag::ID tag, unsigned value);

	void setSignificance(tag::ID tag, Byte value);

	void remove(tag::ID tag);
	void clear();

	tag::ID findFirst() const;
	tag::ID findNext(tag::ID prev) const;

	void dump() const;

private:

	typedef mstl::vector<Tag> ExtraTags;
	typedef mstl::bitfield<uint64_t> BitSet;

	int find(mstl::string const& tag) const;

	void set(tag::ID tag, mstl::string const& value, bool isUserSupplied);
	void set(tag::ID tag, char const* value, unsigned length, bool isUserSupplied);
	void overwrite(tag::ID tag, char const* value, unsigned valueLen);

	mstl::string	m_values[tag::ExtraTag];
	Byte				m_significance[tag::ExtraTag];
	ExtraTags		m_extra;
	BitSet			m_isUserSupplied;
	BitSet			m_set;
};

} // namespace db

namespace mstl {

template <typename T> struct is_movable;

template <>
struct is_movable<db::TagSet::Tag> { enum { value = is_movable<mstl::string>::value }; };

} // namespace mstl

#include "db_tag_set.ipp"

#endif // _db_tag_set_included

// vi:set ts=3 sw=3:
