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

#ifndef _db_namebase_included
#define _db_namebase_included

#include "db_namebase_entry.h"

#include "m_vector.h"
#include "m_string.h"
#include "m_bitset.h"
#include "m_chunk_allocator.h"

namespace mstl { class istream; }

namespace db {

class Player;

class Namebase
{
public:

	typedef NamebaseEntry	Entry;
	typedef NamebaseSite		SiteEntry;
	typedef NamebaseEvent	EventEntry;
	typedef NamebasePlayer	PlayerEntry;

	typedef mstl::vector<Entry const*> Matches;
	typedef mstl::bitset IdSet;

	enum Type
	{
		Player,
		Site,
		Event,
		Annotator,
		Round,		// used in Scid codec
	};

	Namebase(Type type);
	~Namebase() throw();

	bool isEmpty() const;
	bool isPrepared() const;
	bool isConsistent() const;
	bool isModified() const;
	bool isChanged() const;
	bool isOriginal() const;
	bool isReadonly() const;

	Type type() const;

	unsigned size() const;
	unsigned previousSize() const;
	unsigned used() const;
	unsigned maxFrequency() const;
	unsigned maxUsage() const;
	unsigned nextId() const;

	PlayerEntry const* player(unsigned index) const;
	PlayerEntry* player(unsigned index);
	EventEntry const* event(unsigned index) const;
	EventEntry* event(unsigned index);
	SiteEntry const* site(unsigned index) const;
	SiteEntry* site(unsigned index);
	Entry const* entry(unsigned index) const;
	Entry* entry(unsigned index);

	PlayerEntry const* playerAt(unsigned index) const;
	PlayerEntry* playerAt(unsigned index);
	EventEntry const* eventAt(unsigned index) const;
	EventEntry* eventAt(unsigned index);
	SiteEntry const* siteAt(unsigned index) const;
	SiteEntry* siteAt(unsigned index);
	Entry const* entryAt(unsigned index) const;
	Entry* entryAt(unsigned index);

	int findPlayerIndex(	mstl::string const& name,
								uint32_t fideID,
								country::Code country,
								title::ID title,
								species::ID type,
								sex::ID sex) const;
	int findEventIndex(	mstl::string const& name,
								Date const& date,
								event::Type type,
								time::Mode timeMode,
								event::Mode eventMode,
								NamebaseSite const* site) const;
	int findSiteIndex(mstl::string const& name, country::Code country) const;
	int findAnnotatorIndex(mstl::string const& name) const;

	NamebaseSite const* findSite(mstl::string const& name, country::Code country) const;

	void setMaxFrequency(unsigned frequency);
	void setMaxUsage(unsigned usage);
	void setNextId(unsigned id);

	Entry* insert();
	Entry* insert(mstl::string const& name);
	Entry* insert(mstl::string const& name, unsigned limit);
	Entry* insert(mstl::string const& name, unsigned id, unsigned limit);
	Entry const* append(mstl::string const& name, unsigned id);

	SiteEntry* insertSite(mstl::string const& name);
	SiteEntry* insertSite(mstl::string const& name, country::Code country, unsigned limit);
	SiteEntry* insertSite(mstl::string const& name, unsigned id, country::Code country, unsigned limit);
	SiteEntry* appendSite(mstl::string const& name, unsigned id);
	SiteEntry* appendSite(mstl::string const& name, unsigned id, country::Code country);

	EventEntry* insertEvent(	mstl::string const& name,
										NamebaseSite* site);
	EventEntry* insertEvent(	mstl::string const& name,
										unsigned limit,
										NamebaseSite* site);
	EventEntry* insertEvent(	mstl::string const& name,
										unsigned id,
										unsigned limit,
										NamebaseSite* site);
	EventEntry* insertEvent(	mstl::string const& name,
										Date const& date,
										event::Type type,
										time::Mode timeMode,
										event::Mode eventMode,
										unsigned limit,
										NamebaseSite* site);
	EventEntry* insertEvent(	mstl::string const& name,
										unsigned dateYear,
										unsigned dateMonth,
										unsigned dateDay,
										event::Type type,
										time::Mode timeMode,
										event::Mode eventMode,
										unsigned limit,
										NamebaseSite* site);
	EventEntry* insertEvent(	mstl::string const& name,
										unsigned id,
										unsigned dateYear,
										unsigned dateMonth,
										unsigned dateDay,
										event::Type type,
										time::Mode timeMode,
										event::Mode eventMode,
										unsigned limit,
										NamebaseSite* site);
	EventEntry const* appendEvent(mstl::string const& name, unsigned id, NamebaseSite* site);
	EventEntry const* appendEvent(mstl::string const& name,
											unsigned id,
											unsigned dateYear,
											unsigned dateMonth,
											unsigned dateDay,
											event::Type type,
											time::Mode timeMode,
											event::Mode eventMode,
											NamebaseSite* site);

	PlayerEntry* insertPlayer(	mstl::string const& name);
	PlayerEntry* insertPlayer(	mstl::string const& name, uint32_t fideID, unsigned limit);
	PlayerEntry* insertPlayer(	mstl::string const& name,
										country::Code country,
										title::ID title,
										species::ID type,
										sex::ID sex,
										uint32_t fideID,
										unsigned limit);
	PlayerEntry* insertPlayer(	mstl::string const& name,
										unsigned id,
										country::Code country,
										title::ID title,
										species::ID type,
										sex::ID sex,
										uint32_t fideID,
										unsigned limit);
	PlayerEntry const* appendPlayer(mstl::string const& name, unsigned id);
	PlayerEntry const* appendPlayer(	mstl::string const& name,
												unsigned id,
												country::Code country,
												title::ID title,
												species::ID type,
												sex::ID sex,
												uint32_t fideID);
	void rename(NamebaseEntry* entry, mstl::string const& name);

	void update();
	void setPrepared(unsigned maxFrequency, unsigned maxId, unsigned maxUsage);
	void reserve(unsigned size, unsigned limit);
	void clear();
	void setModified(bool flag);
	void setReadonly(bool flag = true);

	unsigned findMatches(mstl::string const& name, Matches& result, unsigned maxMatches = 9) const;

	int lookupPosition(PlayerEntry const* entry) const;
	int lookupPosition(EventEntry const* entry) const;
	int lookupPosition(SiteEntry const* entry) const;
	int lookupPosition(Entry const* entry) const;

	char* alloc(unsigned length);
	void copy(mstl::string& dst, mstl::string const& src);
	void shrink(unsigned oldLength, unsigned newLength);
	void ref(Entry* entry);
	void deref(Entry* entry);
	void cleanup();

private:

	static unsigned const InvalidId = unsigned(-1);

	Namebase(Namebase const&);
	Namebase& operator=(Namebase const&);

	typedef mstl::vector<Entry*>						List;
	typedef mstl::vector<uint32_t>					Map;
	typedef mstl::chunk_allocator<Entry>			EntryAllocator;
	typedef mstl::chunk_allocator<SiteEntry>		SiteAllocator;
	typedef mstl::chunk_allocator<EventEntry>		EventAllocator;
	typedef mstl::chunk_allocator<PlayerEntry>	PlayerAllocator;
	typedef mstl::chunk_allocator<char>				StringAllocator;

	unsigned nextFreeId();

	Entry* makeEntry(mstl::string const& name);
	EventEntry* makeEventEntry(mstl::string const& name);
	SiteEntry* makeSiteEntry(mstl::string const& name, db::Site const* site);
	PlayerEntry* makePlayerEntry(mstl::string const& name, db::Player const* player);

	Type		m_type;
	unsigned	m_maxFreq;
	unsigned	m_nextId;
	unsigned	m_maxUsage;
	unsigned	m_used;
	List		m_list;
	IdSet		m_freeSet;
	IdSet		m_reuseSet;
	Map		m_map;
	bool		m_isConsistent;
	bool		m_isPrepared;
	bool		m_freeSetIsEmpty;
	bool		m_isModified;
	bool		m_isOriginal;
	bool		m_isReadonly;

	union
	{
		EntryAllocator*	m_entryAllocator;
		SiteAllocator*		m_siteAllocator;
		EventAllocator*	m_eventAllocator;
		PlayerAllocator*	m_playerAllocator;
	};

	StringAllocator m_stringAllocator;
};

} // namespace db

#include "db_namebase.ipp"

#endif // _db_namebase_included

// vi:set ts=3 sw=3:
