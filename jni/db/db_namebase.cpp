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

#include "db_namebase.h"
#include "db_player.h"
#include "db_site.h"

#include "m_algorithm.h"
#include "m_utility.h"
#include "m_assert.h"

#ifdef DEBUG_SI4
# include "m_stdio.h"
#endif

#include <string.h>
#include <ctype.h>

using namespace db;


static bool
isPrefix(mstl::string const& s, mstl::string const& t)
{
	if (s.size() > t.size())
		return false;

	for (unsigned i = 0; i < s.size(); ++i)
	{
		if (s[i] != t[i])
			return false;
	}

	return true;
}


namespace db {

inline bool operator< (Namebase::Entry* lhs, mstl::string const& rhs) { return *lhs <  rhs; }
inline bool operator> (Namebase::Entry* lhs, mstl::string const& rhs) { return *lhs >  rhs; }
inline bool operator==(Namebase::Entry* lhs, mstl::string const& rhs) { return *lhs == rhs; }


inline
bool
operator<(Namebase::Entry* lhs, NamebaseSite::Key const& rhs)
{
	//M_ASSERT(dynamic_cast<NamebaseSite const*>(lhs));
	return static_cast<NamebaseSite const&>(*lhs) < rhs;
}


inline
bool
operator<(Namebase::Entry* lhs, NamebaseEvent::Key const& rhs)
{
	//M_ASSERT(dynamic_cast<NamebaseEvent const*>(lhs));
	return static_cast<NamebaseEvent const&>(*lhs) < rhs;
}


inline
bool
operator<(Namebase::Entry* lhs, NamebasePlayer::Key const& rhs)
{
	//M_ASSERT(dynamic_cast<NamebasePlayer const*>(lhs));
	return static_cast<NamebasePlayer const&>(*lhs) < rhs;
}

} // namespace db


Namebase::Namebase(Type type)
	:m_type(type)
	,m_maxFreq(0)
	,m_nextId(0)
	,m_maxUsage(0)
	,m_used(0)
	,m_isConsistent(true)
	,m_isPrepared(false)
	,m_freeSetIsEmpty(true)
	,m_isModified(false)
	,m_isOriginal(true)
	,m_isReadonly(false)
	,m_stringAllocator(32768)
{
	switch (type)
	{
		case Site:		m_siteAllocator = new SiteAllocator(1000*sizeof(SiteEntry)); break;
		case Event:		m_eventAllocator = new EventAllocator(1000*sizeof(EventEntry)); break;
		case Player:	m_playerAllocator = new PlayerAllocator(2000*sizeof(PlayerEntry)); break;
		default:			m_entryAllocator = new EntryAllocator(1000*sizeof(NamebaseEntry));  break;
	}
}


Namebase::~Namebase() throw()
{
	switch (m_type)
	{
		case Site:		delete m_siteAllocator; break;
		case Event:		delete m_eventAllocator; break;
		case Player:	delete m_playerAllocator; break;
		default:			delete m_entryAllocator; break;
	}
}


void
Namebase::reserve(unsigned size, unsigned limit)
{
	if (size > m_list.capacity())
		m_list.reserve(mstl::min(limit, size + (size + 9)/10));
}


unsigned
Namebase::findMatches(mstl::string const& name, Matches& result, unsigned maxMatches) const
{
	if (maxMatches == 0)
		return 0;

	unsigned n = result.size();

	maxMatches += n;

	List::const_iterator i = mstl::lower_bound(m_list.begin(), m_list.end(), name);

	if (i == m_list.end() || *i < name || !::isPrefix(name, (*i)->name()))
		return 0;

	if ((*i)->m_frequency)
		result.push_back(*(i++));

	for ( ; result.size() < maxMatches && i != m_list.end() && ::isPrefix(name, (*i)->name()); ++i)
	{
		if ((*i)->m_frequency)
			result.push_back(*i);
	}

	return result.size() - n;
}


Namebase::Entry*
Namebase::makeEntry(mstl::string const& name)
{
	//M_ASSERT(type() != Event);
	//M_ASSERT(type() != Player);
	//M_ASSERT(name.size() <= NamebaseEntry::MaxNameLength);

	Entry* entry;

	entry = m_entryAllocator->alloc();

	if (name.readonly())
	{
		entry->m_name = name;
	}
	else
	{
		char* p = alloc(name.size());
		::memcpy(p, name.c_str(), name.size() + 1);
		entry->m_name.hook(p, name.size());
	}

	if (entry->m_name.size() > NamebaseEntry::MaxNameLength)
		entry->m_name.set_size(NamebaseEntry::MaxNameLength);

	return entry;
}


Namebase::EventEntry*
Namebase::makeEventEntry(mstl::string const& name)
{
	//M_ASSERT(type() == Event);
	//M_ASSERT(name.size() <= NamebaseEntry::MaxNameLength);

	EventEntry* entry;

	entry = m_eventAllocator->alloc();

	if (name.readonly())
	{
		entry->m_name = name;
	}
	else
	{
		char* p = alloc(name.size());
		::memcpy(p, name.c_str(), name.size() + 1);
		entry->m_name.hook(p, name.size());
	}

	if (entry->m_name.size() > NamebaseEntry::MaxNameLength)
		entry->m_name.set_size(NamebaseEntry::MaxNameLength);

	return entry;
}


Namebase::SiteEntry*
Namebase::makeSiteEntry(mstl::string const& name, db::Site const* site)
{
	//M_ASSERT(type() == Site);
	//M_ASSERT(name.size() <= NamebaseEntry::MaxNameLength);

	SiteEntry* entry;

	entry = m_siteAllocator->alloc();
	entry->m_site = site;

	if (name.readonly())
	{
		entry->m_name = name;
	}
	else
	{
		char* p = alloc(name.size());
		::memcpy(p, name.c_str(), name.size() + 1);
		entry->m_name.hook(p, name.size());
	}

	if (entry->m_name.size() > NamebaseEntry::MaxNameLength)
		entry->m_name.set_size(NamebaseEntry::MaxNameLength);

	return entry;
}


Namebase::PlayerEntry*
Namebase::makePlayerEntry(mstl::string const& name, db::Player const* player)
{
	//M_ASSERT(type() == Player);
	//M_ASSERT(name.size() <= NamebaseEntry::MaxNameLength);

	PlayerEntry* entry = m_playerAllocator->alloc();

	entry->m_player = player;

	if (name.readonly())
	{
		entry->m_name = name;
	}
	else
	{
		char* p = alloc(name.size());
		::memcpy(p, name.c_str(), name.size() + 1);
		entry->m_name.hook(p, name.size());
	}

	if (entry->m_name.size() > NamebaseEntry::MaxNameLength)
		entry->m_name.set_size(NamebaseEntry::MaxNameLength);

	return entry;
}


Namebase::SiteEntry*
Namebase::insertSite(mstl::string const& name,
							unsigned id,
							country::Code country,
							unsigned limit)
{
	// //M_REQUIRE(!isReadonly());
	// //M_REQUIRE(this->type() == Site);
	////M_REQUIRE(::sys::utf8::validate(name));
	// //M_REQUIRE(name.size() <= NamebaseEntry::MaxNameLength);
	// //M_REQUIRE(limit > 0 || isEmpty() || *entryAt(size() - 1) <= name);
//	//M_REQUIRE(limit == 0 || id == InvalidId || id < limit);

	// TODO: probably we do not need the site if country is known
	db::Site const* p = Site::findSite(name);

	if (country != country::Unknown && p && !p->containsCountry(country))
		p = 0;

	SiteEntry* entry;

	if (limit)
	{
		NamebaseSite::Key key(name, country);

		List::iterator i = mstl::lower_bound(m_list.begin(), m_list.end(), key);

		if (i != m_list.end() && *static_cast<SiteEntry const*>(*i) == key)
			return static_cast<SiteEntry*>(*i);

		if (m_list.size() >= limit)
			return 0;

		entry = makeSiteEntry(name, p);
		entry->m_value = key.value;
		unsigned index = i - m_list.begin();
		reserve(index + 1, limit);
		m_list.insert(m_list.begin() + index, entry);

		//M_ASSERT(index == 0 || *siteAt(index - 1) < *siteAt(index));
	}
	else
	{
		m_list.push_back(entry = makeSiteEntry(name, p));
		entry->m_value = country;

		//M_ASSERT(size() == 1 || *siteAt(size() - 2) < *siteAt(size() - 1));
	}

	entry->m_id = id == InvalidId ? nextFreeId() : id;
	m_isConsistent = false;
	m_isModified = true;
	return entry;
}


Namebase::EventEntry*
Namebase::insertEvent(	mstl::string const& name,
								unsigned id,
								unsigned dateYear,
								unsigned dateMonth,
								unsigned dateDay,
								event::Type type,
								time::Mode timeMode,
								event::Mode eventMode,
								unsigned limit,
								NamebaseSite* site)
{
	// //M_REQUIRE(!isReadonly());
	// //M_REQUIRE(this->type() == Event);
	// //M_REQUIRE(site);
	////M_REQUIRE(::sys::utf8::validate(name));
	// //M_REQUIRE(name.size() <= NamebaseEntry::MaxNameLength);
	// //M_REQUIRE(limit > 0 || isEmpty() || *entryAt(size() - 1) <= name);
//	//M_REQUIRE(limit == 0 || id == InvalidId || id < limit);
	// //M_REQUIRE(Date::isValid(dateYear, dateMonth, dateDay));

	EventEntry* entry;

	if (limit)
	{
		NamebaseEvent::Key key(name, type, dateYear, dateMonth, dateDay, timeMode, eventMode, site);

		List::iterator i = mstl::lower_bound(m_list.begin(), m_list.end(), key);

		if (i != m_list.end() && *static_cast<EventEntry const*>(*i) == key)
			return static_cast<EventEntry*>(*i);

		if (m_list.size() >= limit)
			return 0;

		entry = makeEventEntry(name);
		entry->m_value = key.value;
		unsigned index = i - m_list.begin();
		reserve(index + 1, limit);
		m_list.insert(m_list.begin() + index, entry);

		//M_ASSERT(index == 0 || *eventAt(index - 1) < *eventAt(index));
	}
	else
	{
		m_list.push_back(entry = makeEventEntry(name));

		entry->m_value.m_dateYear = Date::encodeYearTo10Bits(dateYear);
		entry->m_value.m_dateMonth = dateMonth;
		entry->m_value.m_dateDay = dateDay;
		entry->m_value.m_type = type;
		entry->m_value.m_timeMode = timeMode;
		entry->m_value.m_eventMode = eventMode;
		entry->m_value.m_site = site;

#ifdef SCI_NAMEBASE_FIX
		{
			unsigned i = size() - 1;

			while (i > 0 && *eventAt(i) < *eventAt(i - 1))
			{
				mstl::swap(m_list[i], m_list[i - 1]);
				--i;
			}
		}
#endif

		//M_ASSERT(size() == 1 || *eventAt(size() - 2) < *eventAt(size() - 1));
	}

	entry->m_id = id == InvalidId ? nextFreeId() : id;
	m_isConsistent = false;
	m_isModified = true;
	return entry;
}


Namebase::PlayerEntry*
Namebase::insertPlayer(	mstl::string const& name,
								unsigned id,
								country::Code federation,
								title::ID title,
								species::ID type,
								sex::ID sex,
								uint32_t fideID,
								unsigned limit)
{
	//M_REQUIRE(!isReadonly());
	//M_REQUIRE(this->type() == Player);
	////M_REQUIRE(::sys::utf8::validate(name));
	//M_REQUIRE(name.size() <= NamebaseEntry::MaxNameLength);
	//M_REQUIRE(limit > 0 || isEmpty() || *entryAt(size() - 1) <= name);
//	//M_REQUIRE(limit == 0 || id == InvalidId || id < limit);

	bool federationFlag	= true;
	bool titleFlag			= true;
	bool typeFlag			= true;
	bool sexFlag			= true;
	bool fideIdFlag		= fideID != 0;

	species::ID myType = type;

	db::Player const* p;

	if (fideIdFlag)
	{
		p = Player::findPlayer(fideID);

		if (p == 0)
			p = Player::insertPlayer(fideID, name);
	}
	else
	{
		// TODO: strip prefix "Comp " for player search (used by ChessBase)
		p = Player::findPlayer(name, federation, sex);

		if (p == 0)
		{
			if (myType == species::Program)
			{
				mstl::string shortName;
				shortName = name; //sys::utf8::Codec::makeShortName(name, shortName);
				p = Player::findEngine(shortName);
			}
			else if (name.find(',') == mstl::string::npos)
			{
				mstl::string shortName;

				shortName = name; //sys::utf8::Codec::makeShortName(name, shortName);

				if ((p = Player::findEngine(shortName)) && myType == species::Unspecified && !p->isUnique())
				{
					char const *p = name.c_str() + shortName.size();

					while (*p == '-' || *p == '_' || *p == '.' || ::isspace(*p))
						++p;

					while (::isdigit(*p) || islower(*p))
						++p;

					if (*p == '\0')
						myType = species::Program;
				}
			}
			else if (myType == species::Unspecified)
			{
				myType = species::Human;
				typeFlag = false;
			}
		}
		else if (myType == species::Unspecified && name.find(',') != mstl::string::npos)
		{
			myType = species::Human;
			typeFlag = false;
		}
	}

	if (p)
	{
		if (myType == species::Unspecified ? p->isUnique() : myType == p->type())
		{
			if (federation == country::Unknown)
				federationFlag = false;

			if (title == title::None)
				titleFlag = false;

			if (myType == species::Unspecified)
				typeFlag = false;

			if (sex == sex::Unspecified)
				sexFlag = false;
		}
		else
		{
			p = 0;
		}
	}

	PlayerEntry* entry;

	if (limit)
	{
		NamebasePlayer::Key key(name, fideID, federation, title, type, sex,
										fideIdFlag, federationFlag, titleFlag, typeFlag, sexFlag);

		List::iterator i = mstl::lower_bound(m_list.begin(), m_list.end(), key);

		if (i != m_list.end() && *static_cast<PlayerEntry const*>(*i) == key)
			return static_cast<PlayerEntry*>(*i);

		if (m_list.size() >= limit)
			return 0;

		entry = makePlayerEntry(name, p);
		unsigned index = i - m_list.begin();
		reserve(index + 1, limit);
		m_list.insert(m_list.begin() + index, entry);
	}
	else
	{
		m_list.push_back(entry = makePlayerEntry(name, p));
	}

	entry->m_id = id == InvalidId ? nextFreeId() : id;
	entry->m_federation = federation;
	entry->m_title = title;
	entry->m_species = type;
	entry->m_sex = sex;
	entry->m_federationFlag = federationFlag;
	entry->m_titleFlag = titleFlag;
	entry->m_speciesFlag = typeFlag;
	entry->m_sexFlag = sexFlag;
	entry->m_fideIdFlag = fideIdFlag;

#ifdef SCI_NAMEBASE_FIX
	{
		unsigned i = size() - 1;

		while (i > 0 && *playerAt(i) < *playerAt(i - 1))
		{
			mstl::swap(m_list[i], m_list[i - 1]);
			--i;
		}
	}
#endif

	//M_ASSERT(size() == 1 || *playerAt(size() - 2) < *playerAt(size() - 1));

	m_isConsistent = false;
	m_isModified = true;
	return entry;
}


Namebase::Entry*
Namebase::insert(mstl::string const& name, unsigned id, unsigned limit)
{
	//M_REQUIRE(!isReadonly());
	//M_REQUIRE(this->type() != Site);
	//M_REQUIRE(this->type() != Event);
	//M_REQUIRE(this->type() != Player);
	////M_REQUIRE(::sys::utf8::validate(name));
	//M_REQUIRE(name.size() <= NamebaseEntry::MaxNameLength);
//	//M_REQUIRE(limit == 0 || id == InvalidId || id < limit);

	List::iterator i = mstl::lower_bound(m_list.begin(), m_list.end(), name);

	if (i != m_list.end() && *i == name)
		return *i;

	if (limit <= m_list.size())
		return 0;

	Entry* entry = makeEntry(name);

	unsigned index = i - m_list.begin();
	reserve(index + 1, limit);
	m_list.insert(m_list.begin() + index, entry);

	//M_ASSERT(size() == 1 || *this->entryAt(size() - 2) < *this->entryAt(size() - 1));

	entry->m_id = id == InvalidId ? nextFreeId() : id;
	m_isConsistent = false;
	m_isModified = true;
	return entry;
}


Namebase::Entry*
Namebase::insert()
{
	//M_REQUIRE(!isReadonly());
	//M_REQUIRE(this->type() != Site);
	//M_REQUIRE(this->type() != Event);
	//M_REQUIRE(this->type() != Player);
	//M_REQUIRE(m_list.empty() || (m_list.size() == 1 && m_list.front()->name().empty()));

	if (m_list.empty())
	{
		m_list.push_back(m_entryAllocator->alloc());
		m_list.back()->m_id = 0;
		m_isModified = true;
	}

	return m_list.front();
}


Namebase::Entry const*
Namebase::append(mstl::string const& name, unsigned id)
{
	//M_REQUIRE(!isReadonly());
	//M_REQUIRE(this->type() != Site);
	//M_REQUIRE(this->type() != Event);
	//M_REQUIRE(this->type() != Player);
	////M_REQUIRE(::sys::utf8::validate(name));
	//M_REQUIRE(name.size() <= NamebaseEntry::MaxNameLength);
	//M_REQUIRE(isEmpty() || *entryAt(size() - 1) < name);
	//M_ASSERT(id != InvalidId);

	Entry* entry = makeEntry(name);

	m_list.push_back(entry);
	m_isConsistent = false;
	m_isModified = true;
	entry->m_id = id;
	return entry;
}


int
Namebase::findPlayerIndex(	mstl::string const& name,
									uint32_t fideID,
									country::Code country,
									title::ID title,
									species::ID type,
									sex::ID sex) const
{
	//M_REQUIRE(this->type() == Player);

	NamebasePlayer::Key key(name,
									fideID,
									country,
									title,
									type,
									sex,
									fideID != 0,
									country != country::Unknown,
									title != title::None,
									type != species::Unspecified,
									sex != sex::Unspecified);

	List::const_iterator i = mstl::lower_bound(m_list.begin(), m_list.end(), key);

	if (i != m_list.end() && *static_cast<PlayerEntry const*>(*i) == key)
		return i - m_list.begin();

	return -1;
}


int
Namebase::findEventIndex(	mstl::string const& name,
									Date const& date,
									event::Type type,
									time::Mode timeMode,
									event::Mode eventMode,
									NamebaseSite const* site) const
{
	//M_REQUIRE(this->type() == Event);
	//M_REQUIRE(site);

	NamebaseEvent::Key key(name, type, date, timeMode, eventMode, const_cast<NamebaseSite*>(site));

	List::const_iterator i = mstl::lower_bound(m_list.begin(), m_list.end(), key);

	if (i != m_list.end() && *static_cast<EventEntry const*>(*i) == key)
		return i - m_list.begin();

	return -1;
}


int
Namebase::findSiteIndex(mstl::string const& name, country::Code country) const
{
	//M_REQUIRE(this->type() == Site);

	NamebaseSite::Key key(name, country);

	List::const_iterator i = mstl::lower_bound(m_list.begin(), m_list.end(), key);

	if (i != m_list.end() && *static_cast<SiteEntry const*>(*i) == key)
		return i - m_list.begin();

	return -1;
}


int
Namebase::findAnnotatorIndex(mstl::string const& name) const
{
	//M_REQUIRE(this->type() == Annotator);

	List::const_iterator i = mstl::lower_bound(m_list.begin(), m_list.end(), name);

	if (i != m_list.end() && *static_cast<Entry const*>(*i) == name)
		return i - m_list.begin();

	return -1;
}


NamebaseSite const*
Namebase::findSite(mstl::string const& name, country::Code country) const
{
	//M_REQUIRE(this->type() == Site);

	NamebaseSite::Key key(name, country);

	List::const_iterator i = mstl::lower_bound(m_list.begin(), m_list.end(), key);

	if (i != m_list.end() && *static_cast<SiteEntry const*>(*i) == key)
		return static_cast<NamebaseSite*>(*i);

	return 0;
}


void
Namebase::clear()
{
	//M_REQUIRE(!isReadonly());

	m_list.clear();
	m_freeSet.clear();
	m_reuseSet.clear();
	m_map.clear();

	m_maxFreq = 0;
	m_maxUsage = 0;
	m_used = 0;
	m_nextId = 0;

	m_isConsistent = true;
	m_isPrepared = true;
	m_isModified = true;
	m_freeSetIsEmpty = true;

	m_stringAllocator.clear();

	switch (m_type)
	{
		case Player:	m_playerAllocator->clear(); break;
		case Event:		m_eventAllocator->clear(); break;
		case Site:		m_siteAllocator->clear(); break;
		default:			m_entryAllocator->clear(); break;
	}
}


char*
Namebase::alloc(unsigned length)
{
	//M_REQUIRE(length <= NamebaseEntry::MaxNameLength);

	char* s = m_stringAllocator.alloc(length + 1);
	s[length] = '\0';
	return s;
}


void
Namebase::shrink(unsigned oldLength, unsigned newLength)
{
	//M_REQUIRE(newLength <= NamebaseEntry::MaxNameLength);
	m_stringAllocator.shrink(oldLength + 1, newLength ? newLength + 1 : 0u);
}


void
Namebase::copy(mstl::string& dst, mstl::string const& src)
{
	//M_REQUIRE(src.size() <= NamebaseEntry::MaxNameLength);

	unsigned length = src.size();
	char* s = m_stringAllocator.alloc(length + 1);
	::memcpy(s, src, length + 1);
	dst.hook(s, length);
}


void
Namebase::cleanup()
{
	//M_REQUIRE(isReadonly());

	for (List::reverse_iterator i = m_list.rbegin(); i != m_list.rend(); ++i)
	{
		if ((*i)->m_frequency == 0 && (*i)->m_id >= size_t(i.base() - m_list.begin()))
			i = m_list.erase(i.base());
	}

	for (unsigned i = 0, n = m_list.size(); i < n; ++i)
		m_list[i]->m_id = i;
}


void
Namebase::deref(Entry* entry)
{
	//M_REQUIRE(entry->frequency() > 0);
	//M_REQUIRE(entry->id() < previousSize());

	m_isConsistent = false;
	entry->deref();

	if (entry->frequency() == 0)
	{
		m_freeSet.set(entry->m_id);
		m_reuseSet.set(entry->m_id);
		m_freeSetIsEmpty = false;
	}
}


void
Namebase::update()
{
	//M_REQUIRE(!isReadonly());

	IdSet	usedSet(mstl::max(List::size_type(m_nextId), m_list.size()));
	List	prepareSet;

	if (m_isModified)
		m_isOriginal = false;

	unsigned index		= 0;
	unsigned used		= m_used;
	unsigned maxUsage	= m_maxUsage;
	unsigned maxFreq	= m_maxFreq;

	m_maxFreq = 0;
	m_maxUsage = 0;
	m_nextId = 0;
	m_used = 0;

	m_freeSet.resize(m_list.size());
	m_freeSet.reset();
	m_reuseSet.resize(m_list.size());
	m_freeSetIsEmpty = true;
	m_map.resize(m_list.size());

	for (List::iterator i = m_list.begin(); i != m_list.end(); ++i, ++index)
	{
		unsigned freq	= (*i)->m_frequency;
		unsigned id		= (*i)->m_id;

		if (freq)
		{
			if (!(*i)->m_name.empty())
				m_maxUsage = mstl::max(m_maxUsage, freq);

			m_maxFreq = mstl::max(m_maxFreq, freq);
			m_nextId = mstl::max(m_nextId, id + 1);
			m_map[m_used++] = index;

			if (usedSet.test_and_set(id) && m_reuseSet.test(id))
				prepareSet.push_back(*i);
		}
		else
		{
			m_freeSet.set(id);
			m_freeSetIsEmpty = false;
		}

#ifdef DEBUG_SI4
		if (m_type != Site && (*i)->m_orig_freq >= 0)
		{
			if (m_type == Event)
			{
				for (List::iterator j = i + 1; j != m_list.end() && (*j)->name() == (*i)->name(); ++j)
					freq += (*j)->frequency();
				for (List::iterator j = i - 1; j >= m_list.begin() && (*j)->name() == (*i)->name(); --j)
					freq += (*j)->frequency();
			}

			if (freq != unsigned((*i)->m_orig_freq))
			{
				char const* type = "";	// shut up the compiler

				switch (m_type)
				{
					case Player:		type = "player"; break;
					case Site:			type = "site"; break;
					case Event:			type = "event"; break;
					case Annotator:	type = "annotator"; break;
					case Round:			type = "round"; break;
				}

				::fprintf(	stderr,
								"WARNING(%u): invalid frequency value %u in %s namebase "
								"(item '%s') (%u is expected)\n",
								unsigned(i - m_list.begin()),
								(*i)->m_orig_freq,
								type,
								(*i)->name().c_str(),
								freq);
			}

			(*i)->m_orig_freq = -1;
		}
#endif
	}

	if (!prepareSet.empty())
	{
		unsigned id = usedSet.find_first_not();

		for (unsigned i = 0; i < prepareSet.size(); ++i)
		{
			//M_ASSERT(id != IdSet::npos);
			prepareSet[i]->m_id = id;
			usedSet.set(id);
			m_freeSet.reset(id);
			m_nextId = mstl::max(m_nextId, id + 1);
			id = usedSet.find_next_not(id);
		}

		m_freeSetIsEmpty = m_freeSet.count() == 0;
	}

	m_reuseSet.reset();
	m_map.resize(m_used);
	m_isConsistent = true;
	m_isPrepared = true;

	if (used != m_used || maxUsage != m_maxUsage || maxFreq != m_maxFreq)
		m_isModified = true;

}


void
Namebase::setPrepared(unsigned maxFrequency, unsigned maxId, unsigned maxUsage)
{
	m_used = m_list.size();
	m_isConsistent = true;
	m_isPrepared = true;
	m_isModified = false;
	m_maxFreq = maxFrequency;
	m_maxUsage = maxUsage;
	m_freeSet.resize(m_used);
	m_freeSet.reset();
	m_reuseSet.resize(m_used);
	m_freeSetIsEmpty = true;
	m_map.resize(m_used);
	m_nextId = maxId + 1;

	for (unsigned i = 0; i < m_used; ++i)
		m_map[i] = i;
}


unsigned
Namebase::nextFreeId()
{
	//M_ASSERT(!m_list.empty());

	if (m_freeSetIsEmpty)
	{
		//M_ASSERT(m_nextId + 1 == m_list.size());
		return m_nextId++;
	}

	unsigned id = m_freeSet.find_first();

	if (id == IdSet::npos)
	{
		m_freeSetIsEmpty = true;
		//M_ASSERT(m_nextId + 1 == m_list.size());
		return m_nextId++;
	}
	else
	{
		m_freeSet.reset(id);
		m_reuseSet.set(id);
	}

	return id;
}


int
Namebase::lookupPosition(PlayerEntry const* entry) const
{
	List::const_iterator i = mstl::lower_bound(m_list.begin(), m_list.end(), NamebasePlayer::Key(*entry));
	return i == m_list.end() || *i != entry ? -1 : int(i - m_list.begin());
}


int
Namebase::lookupPosition(EventEntry const* entry) const
{
	List::const_iterator i = mstl::lower_bound(m_list.begin(), m_list.end(), NamebaseEvent::Key(*entry));
	return i == m_list.end() || *i != entry ? -1 : int(i - m_list.begin());
}


int
Namebase::lookupPosition(SiteEntry const* entry) const
{
	List::const_iterator i = mstl::lower_bound(m_list.begin(), m_list.end(), NamebaseSite::Key(*entry));
	return i == m_list.end() || *i != entry ? -1 : int(i - m_list.begin());
}


int
Namebase::lookupPosition(Entry const* entry) const
{
	List::const_iterator i = mstl::lower_bound(m_list.begin(), m_list.end(), entry->name());
	return i == m_list.end() || *i != entry ? -1 : int(i - m_list.begin());
}


void
Namebase::rename(NamebaseEntry* entry, mstl::string const& name)
{
	//M_REQUIRE(!isReadonly());
	//M_REQUIRE(entry);
	////M_REQUIRE(sys::utf8::validate(name));

	if (name == entry->name())
		return;

	List::iterator oldPos;

	switch (type())
	{
		case Player:
			//M_REQUIRE(dynamic_cast<NamebasePlayer*>(entry));
			{
				PlayerEntry* e = static_cast<PlayerEntry*>(entry);
				oldPos = mstl::lower_bound(m_list.begin(), m_list.end(), NamebasePlayer::Key(*e));
			}
			break;

		case Event:
			//M_REQUIRE(dynamic_cast<NamebaseEvent*>(entry));
			{
				EventEntry* e = static_cast<EventEntry*>(entry);
				oldPos = mstl::lower_bound(m_list.begin(), m_list.end(), NamebaseEvent::Key(*e));
			}
			break;

		case Site:
			//M_REQUIRE(dynamic_cast<NamebaseSite*>(entry));
			{
				SiteEntry* e = static_cast<SiteEntry*>(entry);
				oldPos = mstl::lower_bound(m_list.begin(), m_list.end(), NamebaseSite::Key(*e));
			}
			break;

		case Round:
		case Annotator:
			oldPos = mstl::lower_bound(m_list.begin(), m_list.end(), entry->m_name);
			break;

		default:
			oldPos = m_list.end(); // shut up the compiler
			break;
	}

	//M_ASSERT(oldPos != m_list.end());
	//M_ASSERT(*oldPos == entry);

	mstl::string oldName;

	oldName.swap(entry->m_name);

	if (name.readonly())
	{
		entry->m_name = name;
	}
	else
	{
		char* s = alloc(name.size());
		::memcpy(s, name.c_str(), name.size() + 1);
		entry->m_name.hook(s);
	}

	if (entry->m_name.size() > NamebaseEntry::MaxNameLength)
		entry->m_name.set_size(NamebaseEntry::MaxNameLength);

	switch (type())
	{
		case Player:
			{
				NamebasePlayer::Key key(*static_cast<PlayerEntry*>(entry));
				List::iterator newPos(mstl::lower_bound(m_list.begin(), m_list.end(), key));

				if (oldPos != newPos)
				{
					m_list.erase(oldPos);
					m_list.insert(mstl::lower_bound(m_list.begin(), m_list.end(), key), entry);
				}
			}
			break;

		case Event:
			{
				NamebaseEvent::Key key(*static_cast<EventEntry*>(entry));
				List::iterator newPos = mstl::lower_bound(m_list.begin(), m_list.end(), key);

				if (oldPos != newPos)
				{
					m_list.erase(oldPos);
					m_list.insert(mstl::lower_bound(m_list.begin(), m_list.end(), key), entry);
				}
			}
			break;

		case Site:
			{
				NamebaseSite::Key key(*static_cast<SiteEntry*>(entry));
				List::iterator newPos = mstl::lower_bound(m_list.begin(), m_list.end(), key);

				if (oldPos != newPos)
				{
					m_list.erase(oldPos);
					m_list.insert(mstl::lower_bound(m_list.begin(), m_list.end(), key), entry);
				}
			}
			break;

		case Round:
		case Annotator:
			{
				List::iterator newPos = mstl::lower_bound(m_list.begin(), m_list.end(), entry->m_name);

				if (oldPos != newPos)
				{
					m_list.erase(oldPos);
					m_list.insert(mstl::lower_bound(m_list.begin(), m_list.end(), entry->m_name), entry);
				}
			}
			break;
	}
}

// vi:set ts=3 sw=3:
