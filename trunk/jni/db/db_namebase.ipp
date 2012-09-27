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

inline bool Namebase::isEmpty() const					{ return m_list.empty(); }
inline Namebase::Type Namebase::type() const			{ return m_type; }
inline unsigned Namebase::size() const					{ return m_list.size(); };
inline unsigned Namebase::previousSize() const		{ return m_freeSet.size(); }
inline unsigned Namebase::maxFrequency() const		{ return m_maxFreq; }
inline unsigned Namebase::maxUsage() const			{ return m_maxUsage; }
inline unsigned Namebase::nextId() const				{ return m_nextId; }

inline bool Namebase::isConsistent() const			{ return m_isConsistent; }
inline bool Namebase::isPrepared() const				{ return m_isPrepared; }
inline bool Namebase::isModified() const				{ return m_isModified; }
inline bool Namebase::isChanged() const				{ return !m_isOriginal; }
inline bool Namebase::isOriginal() const				{ return m_isOriginal; }
inline bool Namebase::isReadonly() const				{ return m_isReadonly; }

inline void Namebase::setMaxUsage(unsigned usage)	{ m_maxUsage = usage; }
inline void Namebase::setModified(bool flag)			{ m_isModified = flag; }


inline
void
Namebase::setNextId(unsigned id)
{
	m_nextId = id;
}


inline void
Namebase::setReadonly(bool flag)
{
	// M_REQUIRE(!flag || m_isOriginal);
	m_isReadonly = flag;
}


inline
void
Namebase::setMaxFrequency(unsigned frequency)
{
	m_maxFreq = frequency;
	m_isConsistent = true;
}


inline
unsigned
Namebase::used() const
{
	//M_REQUIRE(isConsistent());
	return m_used;
}


inline
Namebase::Entry const*
Namebase::entryAt(unsigned index) const
{
	//M_REQUIRE(index < size());
	return m_list[index];
}


inline
Namebase::Entry*
Namebase::entryAt(unsigned index)
{
	//M_REQUIRE(index < size());
	return m_list[index];
}


inline
Namebase::SiteEntry const*
Namebase::siteAt(unsigned index) const
{
	//M_REQUIRE(type() == Site);
	//M_REQUIRE(index < size());

	return reinterpret_cast<SiteEntry const*>(m_list[index]);
}


inline
Namebase::SiteEntry*
Namebase::siteAt(unsigned index)
{
	//M_REQUIRE(type() == Site);
	//M_REQUIRE(index < size());

	return reinterpret_cast<SiteEntry*>(m_list[index]);
}


inline
Namebase::EventEntry const*
Namebase::eventAt(unsigned index) const
{
	//M_REQUIRE(type() == Event);
	//M_REQUIRE(index < size());

	return reinterpret_cast<EventEntry const*>(m_list[index]);
}


inline
Namebase::EventEntry*
Namebase::eventAt(unsigned index)
{
	//M_REQUIRE(type() == Event);
	//M_REQUIRE(index < size());

	return reinterpret_cast<EventEntry*>(m_list[index]);
}


inline
Namebase::PlayerEntry const*
Namebase::playerAt(unsigned index) const
{
	//M_REQUIRE(type() == Player);
	//M_REQUIRE(index < size());

	return reinterpret_cast<PlayerEntry const*>(m_list[index]);
}


inline
Namebase::PlayerEntry*
Namebase::playerAt(unsigned index)
{
	//M_REQUIRE(type() == Player);
	//M_REQUIRE(index < size());

	return reinterpret_cast<PlayerEntry*>(m_list[index]);
}


inline
Namebase::Entry const*
Namebase::entry(unsigned index) const
{
	//M_REQUIRE(isConsistent());
	//M_REQUIRE(index < used());

	return m_list[m_map[index]];
}


inline
Namebase::Entry*
Namebase::entry(unsigned index)
{
	//M_REQUIRE(isConsistent());
	//M_REQUIRE(index < used());

	return m_list[m_map[index]];
}


inline
Namebase::SiteEntry const*
Namebase::site(unsigned index) const
{
	//M_REQUIRE(type() == Site);
	//M_REQUIRE(isConsistent());
	//M_REQUIRE(index < used());

	return reinterpret_cast<SiteEntry const*>(m_list[m_map[index]]);
}


inline
Namebase::SiteEntry*
Namebase::site(unsigned index)
{
	//M_REQUIRE(type() == Site);
	//M_REQUIRE(isConsistent());
	//M_REQUIRE(index < used());

	return reinterpret_cast<SiteEntry*>(m_list[m_map[index]]);
}


inline
Namebase::EventEntry const*
Namebase::event(unsigned index) const
{
	//M_REQUIRE(type() == Event);
	//M_REQUIRE(isConsistent());
	//M_REQUIRE(index < used());

	return reinterpret_cast<EventEntry const*>(m_list[m_map[index]]);
}


inline
Namebase::EventEntry*
Namebase::event(unsigned index)
{
	//M_REQUIRE(type() == Event);
	//M_REQUIRE(isConsistent());
	//M_REQUIRE(index < used());

	return reinterpret_cast<EventEntry*>(m_list[m_map[index]]);
}


inline
Namebase::PlayerEntry const*
Namebase::player(unsigned index) const
{
	//M_REQUIRE(type() == Player);
	//M_REQUIRE(isConsistent());
	//M_REQUIRE(index < used());

	return reinterpret_cast<PlayerEntry const*>(m_list[m_map[index]]);
}


inline
Namebase::PlayerEntry*
Namebase::player(unsigned index)
{
	//M_REQUIRE(type() == Player);
	//M_REQUIRE(isConsistent());
	//M_REQUIRE(index < used());

	return reinterpret_cast<PlayerEntry*>(m_list[m_map[index]]);
}


inline
void
Namebase::ref(Entry* entry)
{
	m_isConsistent = false;	// XXX possibly superfluous, force update after changes
	entry->ref();
}


inline
Namebase::Entry*
Namebase::insert(mstl::string const& name)
{
	return insert(name, InvalidId, m_list.size() + 1);
}


inline
Namebase::Entry*
Namebase::insert(mstl::string const& name, unsigned limit)
{
	return insert(name, InvalidId, limit);
}


inline
Namebase::EventEntry*
Namebase::insertEvent(mstl::string const& name, NamebaseSite* site)
{
	return insertEvent(	name,
								InvalidId,
								0, 0, 0,
								event::Unknown,
								time::Unknown,
								event::Undetermined,
								m_list.size() + 1,
								site);
}


inline
Namebase::EventEntry*
Namebase::insertEvent(	mstl::string const& name,
								Date const& date,
								event::Type type,
								time::Mode timeMode,
								event::Mode eventMode,
								unsigned limit,
								NamebaseSite* site)
{
	return insertEvent(	name,
								InvalidId,
								date.year(), date.month(), date.day(),
								type,
								timeMode,
								eventMode,
								limit,
								site);
}


inline
Namebase::EventEntry*
Namebase::insertEvent(	mstl::string const& name,
								unsigned dateYear,
								unsigned dateMonth,
								unsigned dateDay,
								event::Type type,
								time::Mode timeMode,
								event::Mode eventMode,
								unsigned limit,
								NamebaseSite* site)
{
	return insertEvent(	name,
								InvalidId,
								dateYear, dateMonth, dateDay,
								type,
								timeMode,
								eventMode,
								limit,
								site);
}


inline
Namebase::EventEntry*
Namebase::insertEvent(	mstl::string const& name,
								unsigned id,
								unsigned limit,
								NamebaseSite* site)
{
	return insertEvent(	name,
								id,
								0, 0, 0,
								event::Unknown,
								time::Unknown,
								event::Undetermined,
								limit,
								site);
}


inline
Namebase::EventEntry*
Namebase::insertEvent(	mstl::string const& name,
								unsigned limit,
								NamebaseSite* site)
{
	return insertEvent(name, InvalidId, limit, site);
}


inline
Namebase::EventEntry const*
Namebase::appendEvent(	mstl::string const& name,
								unsigned id,
								unsigned dateYear,
								unsigned dateMonth,
								unsigned dateDay,
								event::Type type,
								time::Mode timeMode,
								event::Mode eventMode,
								NamebaseSite* site)
{
	return insertEvent(name, id, dateYear, dateMonth, dateDay, type, timeMode, eventMode, 0, site);
}


inline
Namebase::EventEntry const*
Namebase::appendEvent(mstl::string const& name, unsigned id, NamebaseSite* site)
{
	return insertEvent(name, id, 0, 0, 0, event::Unknown, time::Unknown, event::Undetermined, 0, site);
}


inline
Namebase::PlayerEntry*
Namebase::insertPlayer(mstl::string const& name)
{
	return insertPlayer(	name,
								InvalidId,
								country::Unknown,
								title::None,
								species::Unspecified,
								sex::Unspecified,
								0,
								m_list.size() + 1);
}


inline
Namebase::PlayerEntry*
Namebase::insertPlayer(	mstl::string const& name,
								country::Code country,
								title::ID title,
								species::ID type,
								sex::ID sex,
								uint32_t fideID,
								unsigned limit)
{
	return insertPlayer(name, InvalidId, country, title, type, sex, fideID, limit);
}


inline
Namebase::PlayerEntry*
Namebase::insertPlayer(mstl::string const& name, uint32_t fideID, unsigned limit)
{
	return insertPlayer(	name,
								InvalidId,
								country::Unknown,
								title::None,
								species::Unspecified,
								sex::Unspecified,
								fideID,
								limit);
}


//inline
//Namebase::PlayerEntry*
//Namebase::insertPlayer(mstl::string const& name, uint32_t fideID, unsigned id, unsigned limit)
//{
//	return insertPlayer(	name,
//								id,
//								country::Unknown,
//								title::None,
//								species::Unspecified,
//								sex::Unspecified,
//								fideID,
//								limit);
//}


inline
Namebase::PlayerEntry const*
Namebase::appendPlayer(	mstl::string const& name,
								unsigned id,
								country::Code country,
								title::ID title,
								species::ID type,
								sex::ID sex,
								uint32_t fideID)
{
	return insertPlayer(name, id, country, title, type, sex, fideID, 0);
}


inline
Namebase::PlayerEntry const*
Namebase::appendPlayer(mstl::string const& name, unsigned id)
{
	return insertPlayer(	name,
								id,
								country::Unknown,
								title::None,
								species::Unspecified,
								sex::Unspecified,
								0,
								0);
}


inline
Namebase::SiteEntry*
Namebase::insertSite(mstl::string const& name)
{
	return insertSite(name, InvalidId, country::Unknown, m_list.size() + 1);
}


inline
Namebase::SiteEntry*
Namebase::insertSite(mstl::string const& name, country::Code country, unsigned limit)
{
	return insertSite(name, InvalidId, country, limit);
}


inline
Namebase::SiteEntry*
Namebase::appendSite(mstl::string const& name, unsigned id)
{
	return insertSite(name, id, country::Unknown, 0);
}


inline
Namebase::SiteEntry*
Namebase::appendSite(mstl::string const& name, unsigned id, country::Code country)
{
	return insertSite(name, id, country, 0);
}

} // namespace db

// vi:set ts=3 sw=3:
