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

#include "db_tag_set.h"

#include "m_assert.h"
#include "m_stdio.h"

#include <ctype.h>
#include <string.h>
#include <stdlib.h>

using namespace db;


static void
trim(mstl::string& s)
{
	s.trim();
	s.unhook();
}


TagSet::TagSet()
{
	static_assert(tag::ExtraTag <= 8*sizeof(BitSet), "BitField size exceeded");
	::memset(m_significance, 0, sizeof(m_significance));
}


TagSet::TagSet(TagSet const& set)
	:m_extra(set.m_extra)
	,m_isUserSupplied(set.m_isUserSupplied)
	,m_set(set.m_set)
{
	for (unsigned i = 0; i < U_NUMBER_OF(m_values); ++i)
		m_values[i] = set.m_values[i];

	::memcpy(m_significance, set.m_significance, sizeof(m_significance));
}


TagSet&
TagSet::operator=(TagSet const& set)
{
	for (unsigned i = 0; i < U_NUMBER_OF(m_values); ++i)
		m_values[i] = set.m_values[i];

	::memcpy(m_significance, set.m_significance, sizeof(m_significance));
	m_extra = set.m_extra;
	m_isUserSupplied = set.m_isUserSupplied;
	m_set = set.m_set;

	return *this;
}


#if HAVE_0X_MOVE_CONSTRCUTOR_AND_ASSIGMENT_OPERATOR

# include "m_utility.h"

TagSet::TagSet(TagSet&& set)
	:m_values(mstl::move(set.m_values))
	,m_extra(mstl::move(set.m_extra))
	,m_isUserSupplied(set.m_isUserSupplied)
	,m_set(mstl::move(set.m_set))
{
	::memcpy(m_significance, set.m_significance, sizeof(m_significance));
}


TagSet&
TagSet::operator=(TagSet&& set)
{
	if (this != &set)
	{
		swap(m_values, set.m_values);
		::memcpy(m_significance, set.m_significance, sizeof(m_significance));
		swap(m_extra, set.m_extra);
		m_isUserSupplied = set.m_isUserSupplied;
		m_set = mstl::move(set.m_set);
	}

	return *this;
}

#endif


int
TagSet::find(mstl::string const& tag) const
{
	for (unsigned i = 0; i < m_extra.size(); ++i)
	{
		if (m_extra[i].name == tag)
			return i;
	}

	return -1;
}


void
TagSet::setExtra(mstl::string const& name, mstl::string const& value)
{
	if (!value.empty())
	{
		int index = find(name);

		if (index == -1)
		{
			index = m_extra.size();
			m_extra.push_back();
		}

		Tag& tag = m_extra[index];

		(tag.name = name).trim();
		(tag.value = value).trim();
	}
}


void
TagSet::setExtra(char const* name, unsigned nameLen, char const* value, unsigned valueLen)
{
	//M_REQUIRE(name);
	//M_REQUIRE(value);

	if (valueLen)
	{
		int index = find(name);

		if (index == -1)
		{
			index = m_extra.size();
			m_extra.push_back();
		}

		Tag& tag = m_extra[index];

		tag.name.assign(name, nameLen);
		tag.value.assign(value, valueLen);
		tag.name.trim();
		tag.value.trim();
	}
}


void
TagSet::set(tag::ID tag, mstl::string const& value, bool isUserSupplied)
{
	//M_ASSERT(tag < tag::ExtraTag);
	//M_ASSERT(!value.empty());

	m_isUserSupplied[tag] = isUserSupplied;
	::trim(m_values[tag] = value);
	m_set.set(tag);
}


void
TagSet::set(tag::ID tag, char const* value, unsigned length, bool isUserSupplied)
{
	//M_ASSERT(tag < tag::ExtraTag);
	//M_ASSERT(length);
	//M_ASSERT(value);

	m_isUserSupplied[tag] = isUserSupplied;
	m_values[tag].assign(value, length);
	::trim(m_values[tag]);
	m_set.set(tag);
}


void
TagSet::add(tag::ID tag, mstl::string const& value)
{
	//M_REQUIRE(tag < tag::ExtraTag);

	if (!value.empty() && !m_isUserSupplied[tag])
		set(tag, value, false);
}


void
TagSet::add(tag::ID tag, char const* value, unsigned valueLen)
{
	//M_REQUIRE(tag < tag::ExtraTag);
	//M_REQUIRE(value);

	if (valueLen && !m_isUserSupplied[tag])
		set(tag, value, valueLen, false);
}


void
TagSet::add(tag::ID tag, unsigned value)
{
	//M_REQUIRE(tag < tag::ExtraTag);

	if (!m_isUserSupplied[tag])
	{
		char buf[30];
		set(tag, buf, ::snprintf(buf, sizeof(buf), "%u", value), false);
	}
}


void
TagSet::overwrite(tag::ID tag, char const* value, unsigned valueLen)
{
	//M_ASSERT(tag < tag::ExtraTag);
	//M_ASSERT(value);
	//M_ASSERT(valueLen);

	m_values[tag].assign(value, valueLen);
	::trim(m_values[tag]);
	m_set.set(tag);
}


void
TagSet::replace(tag::ID tag, char const* value, unsigned valueLen)
{
	//M_REQUIRE(tag < tag::ExtraTag);
	//M_REQUIRE(value);

	if (valueLen)
		overwrite(tag, value, valueLen);
}


void
TagSet::replace(tag::ID tag, mstl::string const& value)
{
	//M_REQUIRE(tag < tag::ExtraTag);

	if (!value.empty())
		overwrite(tag, value, value.size());
}


void
TagSet::replace(tag::ID tag, unsigned value)
{
	//M_REQUIRE(tag < tag::ExtraTag);

	char buf[30];
	overwrite(tag, buf, ::snprintf(buf, sizeof(buf), "%u", value));
}


void
TagSet::set(tag::ID tag, unsigned value)
{
	//M_REQUIRE(tag < tag::ExtraTag);

	char buf[30];
	set(tag, buf, ::snprintf(buf, sizeof(buf), "%u", value), true);
}


void
TagSet::set(mstl::string const& name, mstl::string const& value)
{
	if (value.empty())
		return;

	tag::ID tagId = tag::fromName(name);

	if (tagId == tag::ExtraTag)
		setExtra(name, value);
	else
		set(tagId, value);
}


void
TagSet::set(mstl::string const& name, char const* value, unsigned valueLen)
{
	//M_REQUIRE(value);

	if (valueLen == 0)
		return;

	tag::ID tagId = tag::fromName(name);

	if (tagId == tag::ExtraTag)
		setExtra(name.c_str(), name.size(), value, valueLen);
	else
		set(tagId, value, valueLen);
}


void
TagSet::set(char const* name, unsigned nameLen, char const* value, unsigned valueLen)
{
	//M_REQUIRE(value);
	//M_REQUIRE(name);

	if (valueLen == 0)
		return;

	tag::ID tagId = tag::fromName(name, nameLen);

	if (tagId == tag::ExtraTag)
		setExtra(name, nameLen, value, valueLen);
	else
		set(tagId, value, valueLen);
}


void
TagSet::remove(tag::ID tag)
{
	m_values[tag].clear();
	m_isUserSupplied.reset(tag);
	m_set.reset(tag);
}


void
TagSet::clear()
{
	m_isUserSupplied.reset();
	m_extra.clear();
	m_set.reset();

	for (unsigned i = 0; i < tag::ExtraTag; ++i)
		m_values[i].clear();
}


int
TagSet::asInt(tag::ID tag) const
{
	mstl::string const& v = value(tag);
	return ::strtoul(v.c_str(), nullptr, 10);
}


util::crc::checksum_t
TagSet::computeChecksum(util::crc::checksum_t crc) const
{
	for (tag::ID tag = findFirst(); tag < tag::ExtraTag; tag = findNext(tag))
	{
		crc = util::crc::compute(crc, uint8_t(tag));
		crc = util::crc::compute(crc, uint8_t(m_isUserSupplied.test(tag)));
		crc = util::crc::compute(crc, uint8_t(m_significance[tag]));
		crc = util::crc::compute(crc, m_values[tag].c_str(), m_values[tag].size());
	}

	for (unsigned i = 0; i < m_extra.size(); ++i)
	{
		crc = util::crc::compute(crc, m_extra[i].name.c_str(), m_extra[i].name.size());
		crc = util::crc::compute(crc, m_extra[i].value.c_str(), m_extra[i].value.size());
	}

	return crc;
}


void
TagSet::dump() const
{
	for (tag::ID tag = findFirst(); tag < tag::ExtraTag; tag = findNext(tag))
	{
		::printf("%s: %s", tag::toName(tag).c_str(), m_values[tag].c_str());

		if (significance(tag) > 0)
			::printf(" (%d)", significance(tag));

		if (isUserSupplied(tag))
			::printf(" (user-supplied)");

		printf("\n");
	}

	for (unsigned i = 0; i < m_extra.size(); ++i)
	{
		::printf("%s: %s", m_extra[i].name.c_str(), m_extra[i].value.c_str());
		printf("\n");
	}
}

// vi:set ts=3 sw=3:
