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

#ifndef _db_namebase_entry
#define _db_namebase_entry

#include "db_date.h"
#include "db_common.h"

#include "m_string.h"

namespace db {

class Namebase;
class Player;
class Site;


class NamebaseEntry
{
public:

	enum { MaxNameLength = 255 };

	NamebaseEntry();

	NamebaseEntry(mstl::string const& name);
	virtual ~NamebaseEntry() throw();

	bool operator==(mstl::string const& name) const;
	bool operator< (mstl::string const& name) const;
	bool operator<=(mstl::string const& name) const;
	bool operator> (mstl::string const& name) const;
	bool operator>=(mstl::string const& name) const;

	bool operator<(NamebaseEntry const& entry) const;

	bool used() const;

	mstl::string const& name() const;
	unsigned frequency() const;
	unsigned id() const;

	void ref();
	void deref();

	void setId(unsigned id);
	void setFrequency(unsigned frequency);
	void setName(mstl::string const& name);

	static int compare(char const* lhs, char const* rhs);

	// XXX not working; one instance per database is needed
	static NamebaseEntry* emptyEntry();

private:

	friend class Namebase;

	mstl::string	m_name;
	uint32_t			m_frequency;
	uint32_t			m_id;

	static NamebaseEntry* m_emptyEntry;

#ifdef DEBUG_SI4
public: int m_orig_freq;
#endif
};


class NamebaseSite : public NamebaseEntry
{
public:

	typedef country::Code Value;

	struct Key
	{
		Key(NamebaseSite const& event);
		Key(mstl::string const& name, country::Code country);

		mstl::string const& name;
		Value value;
	};

	NamebaseSite();

	bool operator==(Key const& key) const;
	bool operator< (Key const& key) const;

	bool operator<(NamebaseSite const& event) const;

	country::Code country() const;
	country::Code findCountry() const;

	Site const* site() const;

	void setCountry(country::Code country);

private:

	friend class Namebase;

	Value			m_value;
	Site const*	m_site;
};


class NamebaseEvent : public NamebaseEntry
{
public:

	struct Value
	{
		Value();
		Value(NamebaseEvent const& event);
		Value(event::Type type,
				unsigned dateYear,
				unsigned dateMonth,
				unsigned dateDay,
				time::Mode timeMode,
				event::Mode eventMode,
				NamebaseSite* site);
		Value(event::Type type,
				Date const& date,
				time::Mode timeMode,
				event::Mode eventMode,
				NamebaseSite* site);

		bool operator< (Value value) const;
		bool operator==(Value value) const;
		bool operator!=(Value value) const;

		union
		{
			// XXX this is not platform independent!
			struct
			{
				uint32_t m_type		:4;
				uint32_t m_eventMode	:3;
				uint32_t m_timeMode	:3;
				uint32_t m_dateYear	:10;
				uint32_t m_dateMonth	:4;
				uint32_t m_dateDay	:5;
				uint32_t m_unused_	:3;
			};

			uint32_t m_key;
		};

		NamebaseSite* m_site;
	};

	struct Key
	{
		Key(NamebaseEvent const& event);
		Key(	mstl::string const& name,
				event::Type type,
				Date const& date,
				time::Mode timeMode,
				event::Mode eventMode,
				NamebaseSite* site);
		Key(	mstl::string const& name,
				event::Type type,
				unsigned dateYear,
				unsigned dateMonth,
				unsigned dateDay,
				time::Mode timeMode,
				event::Mode eventMode,
				NamebaseSite* site);

		mstl::string const& name;
		Value value;
	};

	NamebaseEvent();

	bool operator==(Key const& key) const;
	bool operator< (Key const& key) const;

	bool operator<(NamebaseEvent const& event) const;

	bool hasDate() const;

	Date date() const;
	event::Type type() const;
	time::Mode timeMode() const;
	event::Mode eventMode() const;
	uint16_t dateYear() const;
	uint16_t dateMonth() const;
	uint16_t dateDay() const;
	country::Code country() const;
	NamebaseSite* site() const;

	// NOTE: use with care because the Namebase is dending on the key!
	void setDate_(Date const& date);
	void setDate_(unsigned year, unsigned month, unsigned day);
	void setType_(event::Type type);
	void setTimeMode_(time::Mode timeMode);
	void setEventMode_(event::Mode mode);
	void setCountry_(country::Code country);
	void setSite_(NamebaseSite* site);

	// XXX not working; one instance per database is needed
	static NamebaseSite* emptySite();

private:

	friend class Namebase;

	Value m_value;

	static NamebaseSite* m_emptySite;
};


class NamebasePlayer : public NamebaseEntry
{
public:

	static uint32_t const KeyMask		= 0x01ffffff;
	static uint32_t const SortMask	= 0x001fffff;

	union Value
	{
		Value();
		Value(uint32_t key);
		Value(NamebasePlayer const& player);
		Value(country::Code federation,
				title::ID title,
				species::ID type,
				sex::ID sex,
				bool fideIdFlag,
				bool federationFlag,
				bool typeFlag,
				bool sexFlag,
				bool titleFlag);

		bool operator<(Value value) const;

		// XXX this is not platform independent!
		struct
		{
			uint32_t	m_species:3;
			uint32_t m_sex:3;
			uint32_t	m_federation:9;
			uint32_t	m_title:5;
			uint32_t m_fideIdFlag:1;
			uint32_t	m_federationFlag:1;
			uint32_t	m_titleFlag:1;
			uint32_t m_sexFlag:1;
			uint32_t m_speciesFlag:1;
			uint32_t m_ignored_:4;
			uint32_t m_unused_:3;
		};

		uint32_t m_key;
	};

	struct Key
	{
		Key(NamebasePlayer const& player);
		Key(	mstl::string const& name,
				uint32_t fideID,
				country::Code federation,
				title::ID title,
				species::ID type,
				sex::ID sex,
				bool fideIdFlag,
				bool federationFlag,
				bool typeFlag,
				bool sexFlag,
				bool titleFlag);

		static uint32_t makeValue(NamebasePlayer const& player);
		static uint32_t makeValue(	country::Code federation,
											title::ID title,
											species::ID type,
											sex::ID sex);

		mstl::string const& name;
		uint32_t fideID;
		Value value;
	};

	typedef uint16_t Ratings[rating::Last];

	NamebasePlayer();

	bool operator==(Key const& key) const;
	bool operator< (Key const& key) const;

	bool operator<(NamebasePlayer const& player) const;

	bool isEngine() const;

	bool haveFideId() const;
	bool havePlayerInfo() const;

	sex::ID sex() const;
	species::ID type() const;
	uint16_t elo() const;
	uint16_t rating() const;
	uint16_t rating(rating::Type type) const;
	rating::Type ratingType() const;
	country::Code federation() const;
	title::ID title() const;
	uint32_t fideID() const;

	country::Code findFederation() const;
	title::ID findTitle() const;
	species::ID findType() const;
	sex::ID findSex() const;
	uint16_t findElo() const;
	uint16_t findRating(rating::Type type) const;
	rating::Type findRatingType() const;
	int32_t findFideID() const;

	uint16_t playerHighestElo() const;
	uint16_t playerLatestElo() const;
	uint16_t playerHighestRating() const;
	uint16_t playerLatestRating() const;
	uint16_t playerHighestRating(rating::Type type) const;
	uint16_t playerLatestRating(rating::Type type) const;
	rating::Type playerRatingType() const;
	bool isPlayerRating(rating::Type type) const;

	Player const* player() const;

	void setElo(uint16_t value);
	void setRating(rating::Type type, uint16_t value);

private:

	friend class Namebase;

	Player const*	m_player;
	Ratings			m_rating;

	union
	{
		// XXX this is not platform independent!
		struct
		{
			uint32_t	m_species:3;
			uint32_t m_sex:3;
			uint32_t	m_federation:9;
			uint32_t	m_title:5;
			uint32_t m_fideIdFlag:1;
			uint32_t	m_federationFlag:1;
			uint32_t	m_titleFlag:1;
			uint32_t m_sexFlag:1;
			uint32_t m_speciesFlag:1;
			uint32_t	m_ratingType:4;
			uint32_t m_unused_:3;
		};

		uint32_t m_value;
	};
};

} // namespace db

#include "db_namebase_entry.ipp"

#endif // _db_namebase_entry

// vi:set ts=3 sw=3:
