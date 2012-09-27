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

#include "db_player.h"

#include "m_assert.h"
#include "m_utility.h"

#include <string.h>

namespace db {

inline NamebaseEntry::NamebaseEntry()
	:m_frequency(0)
	,m_id(0)
#ifdef DEBUG_SI4
	,m_orig_freq(-1)
#endif
{}

inline bool NamebaseEntry::used() const						{ return m_frequency > 0; }
inline mstl::string const& NamebaseEntry::name() const		{ return m_name; }
inline unsigned NamebaseEntry::frequency() const			{ return m_frequency; }
inline unsigned NamebaseEntry::id() const						{ return m_id; }
inline NamebaseEntry* NamebaseEntry::emptyEntry()			{ return m_emptyEntry; }

inline void NamebaseEntry::ref()									{ ++m_frequency; }
inline void NamebaseEntry::deref()								{ if(frequency() > 0){}; --m_frequency; }
inline void NamebaseEntry::setId(unsigned id)				{ m_id = id; }

inline void NamebaseEntry::setFrequency(unsigned frequency)	{ m_frequency = frequency; }

// TODO: should we use case compare?
inline int NamebaseEntry::compare(char const* lhs, char const* rhs)	{ return ::strcmp(lhs, rhs); }


inline
bool
NamebaseEntry::operator==(mstl::string const& name) const
{
	return compare(m_name, name) == 0;
}


inline bool
NamebaseEntry::operator<(mstl::string const& name) const
{
	return compare(m_name, name) < 0;
}


inline
bool
NamebaseEntry::operator>(mstl::string const& name) const
{
	return compare(m_name, name) > 0;
}


inline bool
NamebaseEntry::operator<=(mstl::string const& name) const
{
	return compare(m_name, name) <= 0;
}


inline
bool
NamebaseEntry::operator>=(mstl::string const& name) const
{
	return compare(m_name, name) >= 0;
}


inline
bool
NamebaseEntry::operator<(NamebaseEntry const& entry) const
{
	return *this < entry.name();
}


inline
void
NamebaseEntry::setName(mstl::string const& name)
{
	// M_REQUIRE(name.size() <= MaxNameLength);
	m_name = name;
}


inline
NamebaseSite::Key::Key(NamebaseSite const& site)
	:name(site.name())
	,value(site.m_value)
{
}


inline
NamebaseSite::Key::Key(mstl::string const& name, country::Code country)
	:name(name)
	,value(country)
{
}


inline
bool
NamebaseSite::operator<(NamebaseSite const& site) const
{
	int cmp = compare(name(), site.name());

	if (cmp < 0) return true;
	if (cmp > 0) return false;

	return m_value < site.m_value;
}


inline
bool
NamebaseSite::operator==(Key const& key) const
{
	return NamebaseEntry::compare(name(), key.name) == 0 && m_value == key.value;
}


inline
bool
NamebaseSite::operator<(Key const& key) const
{
	int cmp = NamebaseEntry::compare(name(), key.name);

	if (cmp < 0) return true;
	if (cmp > 0) return false;

	return m_value < key.value;
}


inline void NamebaseSite::setCountry(country::Code country)	{ m_value = country; }
inline country::Code NamebaseSite::country() const				{ return m_value; }
inline Site const* NamebaseSite::site() const					{ return m_site; }


inline
NamebaseEvent::Value::Value()
	:m_type(event::Unknown)
	,m_eventMode(event::Undetermined)
	,m_timeMode(time::Unknown)
	,m_dateYear(Date::Zero10Bits)
	,m_dateMonth(0)
	,m_dateDay(0)
	,m_unused_(0)
	,m_site(m_emptySite)
{
}


inline
NamebaseEvent::Value::Value(NamebaseEvent const& event)
	:m_key(event.m_value.m_key)
	,m_site(event.m_value.m_site)
{
}


inline
NamebaseEvent::Value::Value(	event::Type type,
										unsigned dateYear,
										unsigned dateMonth,
										unsigned dateDay,
										time::Mode timeMode,
										event::Mode eventMode,
										NamebaseSite* site)
	:m_type(type)
	,m_eventMode(eventMode)
	,m_timeMode(timeMode)
	,m_dateYear(Date::encodeYearTo10Bits(dateYear))
	,m_dateMonth(dateMonth)
	,m_dateDay(dateDay)
	,m_unused_(0)
	,m_site(site)
{
	// M_REQUIRE(site);
}


inline
NamebaseEvent::Value::Value(	event::Type type,
										Date const& date,
										time::Mode timeMode,
										event::Mode eventMode,
										NamebaseSite* site)
	:m_type(type)
	,m_eventMode(eventMode)
	,m_timeMode(timeMode)
	,m_dateYear(Date::encodeYearTo10Bits(date.year()))
	,m_dateMonth(date.month())
	,m_dateDay(date.day())
	,m_unused_(0)
	,m_site(site)
{
	// M_REQUIRE(site);
}


inline
bool
NamebaseEvent::Value::operator<(Value value) const
{
	return m_key < value.m_key || (m_key == value.m_key && *m_site < *value.m_site);
}


inline
bool
NamebaseEvent::Value::operator==(Value value) const
{
	return m_key == value.m_key && m_site == value.m_site;
}


inline
NamebaseEvent::Key::Key(NamebaseEvent const& event)
	:name(event.name())
	,value(event.m_value)
{
}


inline
NamebaseEvent::Key::Key(mstl::string const& name,
								event::Type type,
								Date const& date,
								time::Mode timeMode,
								event::Mode eventMode,
								NamebaseSite* site)
	:name(name)
	,value(type, date, timeMode, eventMode, site)
{
}


inline
NamebaseEvent::Key::Key(mstl::string const& name,
								event::Type type,
								unsigned dateYear,
								unsigned dateMonth,
								unsigned dateDay,
								time::Mode timeMode,
								event::Mode eventMode,
								NamebaseSite* site)
	:name(name)
	,value(type, dateYear, dateMonth, dateDay, timeMode, eventMode, site)
{
}


inline event::Type NamebaseEvent::type() const			{ return event::Type(m_value.m_type); }
inline time::Mode NamebaseEvent::timeMode() const		{ return time::Mode(m_value.m_timeMode); }
inline event::Mode NamebaseEvent::eventMode() const	{ return event::Mode(m_value.m_eventMode); }
inline uint16_t NamebaseEvent::dateMonth() const		{ return m_value.m_dateMonth; }
inline uint16_t NamebaseEvent::dateDay() const			{ return m_value.m_dateDay; }
inline country::Code NamebaseEvent::country() const	{ return m_value.m_site->country(); }
inline NamebaseSite* NamebaseEvent::site() const		{ return m_value.m_site; }
inline NamebaseSite* NamebaseEvent::emptySite()			{ return m_emptySite; }

inline void NamebaseEvent::setType_(event::Type type)				{ m_value.m_type = type; }
inline void NamebaseEvent::setTimeMode_(time::Mode timeMode)	{ m_value.m_timeMode = timeMode; }
inline void NamebaseEvent::setEventMode_(event::Mode mode)		{ m_value.m_eventMode = mode; }
inline void NamebaseEvent::setCountry_(country::Code country)	{ m_value.m_site->setCountry(country); }
inline void NamebaseEvent::setSite_(NamebaseSite* site)			{ m_value.m_site = site; }


inline
bool
NamebaseEvent::hasDate() const
{
	return m_value.m_dateYear != Date::Zero10Bits;
}


inline
uint16_t
NamebaseEvent::dateYear() const
{
	return Date::decodeYearFrom10Bits(m_value.m_dateYear);
}


inline
Date
NamebaseEvent::date() const
{
	return Date(dateYear(), dateMonth(), dateDay());
}


inline
void
NamebaseEvent::setDate_(unsigned year, unsigned month, unsigned day)
{
	// M_REQUIRE(year == 0 || (Date::MinYear <= year && year <= Date::MaxYear));

	m_value.m_dateYear = Date::encodeYearTo10Bits(year);
	m_value.m_dateMonth = month;
	m_value.m_dateDay = day;
}


inline
void
NamebaseEvent::setDate_(Date const& date)
{
	setDate_(date.year(), date.month(), date.day());
}


inline
bool
NamebaseEvent::operator<(NamebaseEvent const& event) const
{
	int cmp = compare(name(), event.name());

	if (cmp < 0) return true;
	if (cmp > 0) return false;

	return m_value < event.m_value;
}


inline
bool
NamebaseEvent::operator==(Key const& key) const
{
	return NamebaseEntry::compare(name(), key.name) == 0 && m_value == key.value;
}


inline
bool
NamebaseEvent::operator<(Key const& key) const
{
	int cmp = NamebaseEntry::compare(name(), key.name);

	if (cmp < 0) return true;
	if (cmp > 0) return false;

	return m_value < key.value;
}


inline NamebasePlayer::Value::Value(uint32_t key) : m_key(key & KeyMask) {}


inline
NamebasePlayer::Value::Value()
	:m_species(species::Unspecified)
	,m_sex(sex::Unspecified)
	,m_federation(country::Unknown)
	,m_title(title::None)
	,m_fideIdFlag(0)
	,m_federationFlag(0)
	,m_titleFlag(0)
	,m_sexFlag(0)
	,m_speciesFlag(0)
	,m_ignored_(0)
	,m_unused_(0)
{
}


inline
NamebasePlayer::Value::Value(NamebasePlayer const& player)
	:m_key(player.m_value & KeyMask)
{
}


inline
NamebasePlayer::Value::Value(	country::Code federation,
										title::ID title,
										species::ID type,
										sex::ID sex,
										bool fideIdFlag,
										bool federationFlag,
										bool titleFlag,
										bool typeFlag,
										bool sexFlag)
	:m_species(type)
	,m_sex(sex)
	,m_federation(federation)
	,m_title(title)
	,m_fideIdFlag(fideIdFlag)
	,m_federationFlag(federationFlag)
	,m_titleFlag(titleFlag)
	,m_sexFlag(sexFlag)
	,m_speciesFlag(typeFlag)
	,m_ignored_(0)
	,m_unused_(0)
{
}


inline bool NamebasePlayer::Value::operator<(Value value) const { return m_key < value.m_key; }


inline
NamebasePlayer::Key::Key(NamebasePlayer const& player)
	:name(player.name())
	,value(player.m_value)
{
}


inline
NamebasePlayer::Key::Key(	mstl::string const& name,
									uint32_t fideID,
									country::Code country,
									title::ID title,
									species::ID type,
									sex::ID sex,
									bool fideIdFlag,
									bool federationFlag,
									bool titleFlag,
									bool typeFlag,
									bool sexFlag)
	:name(name)
	,fideID(fideID)
	,value(country, title, type, sex, fideIdFlag, federationFlag, titleFlag, typeFlag, sexFlag)
{
}


inline bool NamebasePlayer::haveFideId() const				{ return m_fideIdFlag; }
inline bool NamebasePlayer::havePlayerInfo() const			{ return m_player; }

inline uint16_t NamebasePlayer::elo() const					{ return m_rating[rating::Elo]; }
inline rating::Type NamebasePlayer::ratingType() const	{ return rating::Type(m_ratingType); }
inline Player const* NamebasePlayer::player() const		{ return m_player; }


inline
uint16_t
NamebasePlayer::rating(rating::Type type) const
{
	// M_REQUIRE(type != rating::Any);
	return m_rating[type];
}


inline
uint16_t
NamebasePlayer::rating() const
{
	return m_ratingType == rating::Any ? 0 : m_rating[m_ratingType];
}


inline
bool
NamebasePlayer::operator<(NamebasePlayer const& player) const
{
	int cmp = compare(name(), player.name());

	if (cmp < 0) return true;
	if (cmp > 0) return false;

	if (m_fideIdFlag && player.m_fideIdFlag)
	{
		// M_ASSERT(m_player);
		// M_ASSERT(player.m_player);

		return m_player->fideID() < player.m_player->fideID();
	}

	return (m_value & SortMask) < (player.m_value & SortMask);
}


inline
bool
NamebasePlayer::operator==(Key const& key) const
{
	// M_ASSERT(!m_fideIdFlag || m_player);

	return	NamebaseEntry::compare(name(), key.name) == 0
			&& (m_value & SortMask) == (key.value.m_key & SortMask)
			&& (!m_fideIdFlag || m_player->fideID() == key.fideID);
}


inline
bool
NamebasePlayer::operator<(Key const& key) const
{
	int cmp = NamebaseEntry::compare(name(), key.name);

	if (cmp < 0) return true;
	if (cmp > 0) return false;

	if (m_fideIdFlag && key.fideID)
	{
		// M_ASSERT(m_player);
		return m_player->fideID() < key.fideID;
	}

	return (m_value & SortMask) < (key.value.m_key & SortMask);
}


inline
uint32_t
NamebasePlayer::fideID() const
{
	// M_ASSERT(m_player != 0 || m_fideIdFlag == 0);
	return m_fideIdFlag ? m_player->fideID() : 0;
}


inline
country::Code
NamebasePlayer::federation() const
{
	return m_federationFlag ? country::Code(m_federation) : country::Unknown;
}


inline
title::ID
NamebasePlayer::title() const
{
	return m_titleFlag ? title::ID(m_title) : title::None;
}


inline
species::ID
NamebasePlayer::type() const
{
	return m_speciesFlag ? species::ID(m_species) : species::Unspecified;
}


inline
sex::ID
NamebasePlayer::sex() const
{
	return m_sexFlag ? sex::ID(m_sex) : sex::Unspecified;
}


inline
bool
NamebasePlayer::isEngine() const
{
	if (m_species == species::Program)
		return true;

	return m_player && m_player->isEngine();
}


inline
uint16_t
NamebasePlayer::findElo() const
{
	if (m_rating[rating::Elo])
		return m_rating[rating::Elo];

	return m_player ? mstl::abs(m_player->highestElo()) : 0;
}


inline
title::ID
NamebasePlayer::findTitle() const
{
	if (m_title != title::None)
		return title::ID(m_title);

	return m_player ? title::best(m_player->titles()) : title::None;
}


inline
int32_t
NamebasePlayer::findFideID() const
{
	if (m_player == 0)
		return 0;

	return m_fideIdFlag ? int32_t(m_player->fideID()) : -int32_t(m_player->fideID());
}


inline
country::Code
NamebasePlayer::findFederation() const
{
	if (m_federation != country::Unknown)
		return country::Code(m_federation);

	return m_player ? m_player->federation() : country::Unknown;
}


inline
species::ID
NamebasePlayer::findType() const
{
	if (m_species != species::Unspecified)
		return species::ID(m_species);

	return m_player ? m_player->type() : species::Unspecified;
}


inline
sex::ID
NamebasePlayer::findSex() const
{
	if (m_sex != sex::Unspecified)
		return sex::ID(m_sex);

	return m_player ? m_player->sex() : sex::Unspecified;
}


inline
void
NamebasePlayer::setElo(uint16_t value)
{
	// M_REQUIRE(value <= rating::Max_Value);
	m_rating[rating::Elo] = mstl::max(m_rating[rating::Elo], value);
}


inline
void
NamebasePlayer::setRating(rating::Type type, uint16_t value)
{
	// M_REQUIRE(type != rating::Any);
	// M_REQUIRE(value <= rating::Max_Value);

	if (value)
	{
		m_rating[type] = mstl::max(m_rating[type], value);

		if (type != rating::Elo)
			m_ratingType = mstl::min(rating::Type(m_ratingType), type);
	}
}

} // namespace db

// vi:set ts=3 sw=3:
