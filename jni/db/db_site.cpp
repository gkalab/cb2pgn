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

#include "db_site.h"

#include "m_hash.h"
#include "m_vector.h"
#include "m_map.h"
#include "m_string.h"
#include "m_chunk_allocator.h"
#include "m_algorithm.h"
#include "m_istream.h"
#include "m_stdio.h"

#include <ctype.h>

//#define DEBUG

#ifdef DEBUG
# undef DEBUG
# define DEBUG(stmt) stmt
#else
# define DEBUG(stmt)
#endif

using namespace db;


namespace {

struct City
{
	Site const*	site;
	unsigned		country;

	City() {}
	City(Site const* site, country::Code country) : site(site), country(country) {}

	bool operator==(City const& city) const
	{
		return country == city.country && site->name() == city.site->name();
	}

	bool operator<(City const& city) const
	{
		if (country < city.country)
			return true;
		if (country > city.country)
			return false;

		return site->name() < city.site->name();
	}
};

} // namespace


namespace mstl {

template <> struct hash_key< ::City>
{
	static size_t
	hash(::City key)
	{
		return hash_key<mstl::string>::hash(key.site->name()) ^ size_t(key.country);
	}
};

} // namespace mstl


typedef Site::StringList StringList;
typedef mstl::hash<mstl::string,Site*> SiteLookup;
typedef mstl::hash<mstl::string,country::Code> CountryLookup;
typedef mstl::vector<SiteLookup::assoc_t> SiteList;
typedef mstl::chunk_allocator<Site> SiteAllocator;
typedef mstl::chunk_allocator<char> CharAllocator;
typedef mstl::hash<City,StringList> AliasDict;

static SiteLookup siteLookup(unsigned(20000*(100/SiteLookup::Load)));
static CountryLookup countryLookup(2000);
static AliasDict aliasDict(unsigned(3000*(100/AliasDict::Load)));
static SiteList siteList;
static CharAllocator charAllocator(1024);
static SiteAllocator siteAllocator(4096);
static StringList removeSites;
static StringList emptyList;


namespace db {

inline
bool
operator<(SiteList::value_type const& lhs, mstl::string const& rhs)
{
	return lhs.first < rhs;
}

} // namespace db


static int
cmpAssoc(SiteList::value_type const* lhs, SiteList::value_type const* rhs)
{
	return strcmp(lhs->first, rhs->first);
}


static int
cmpMatch(Site::Matches::value_type const* lhs, Site::Matches::value_type const* rhs)
{
	return strcmp(lhs->second->name(), rhs->second->name());
}


static bool
match(Site const* lhs, Site const* rhs)
{
	return		lhs == rhs
				|| (	lhs->countCountries() == 1
					&& rhs->countCountries() == 1
					&& lhs->country(0) == rhs->country(0));
}


static void
insert(Site::Matches& matches, Site const* site)
{
	//M_ASSERT(site);

	unsigned i = 0;
	while (i < matches.size() && matches[i].second->name() != site->name())
		++i;

	if (i == matches.size())
	{
		for (unsigned k = 0; k < site->countCountries(); ++k)
			matches.push_back(Site::Match(site->country(k), site));
	}
}


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


static char*
skipSpace(char* s)
{
	while (::isspace(*s))
		++s;

	return s;
}


mstl::string&
Site::normalize(mstl::string& name)
{
	static mstl::string const Exclude(" .-'´`/,()");

	name.make_writable();

	mstl::string::size_type n = name.find_first_of(Exclude);

	while (n != mstl::string::npos)
	{
		name.erase(name.begin() + n);
		n = name.find_first_of(Exclude, n);
	}

	return name;
}


bool
Site::containsCountry(country::Code country) const
{
	for (unsigned i = 0; i < m_ascii.size(); ++i)
	{
		if (m_ascii[i].country == country)
			return true;
	}

	return false;
}


Site*
Site::findSite(mstl::string const& name)
{
	mstl::string s(name);
	normalize(s);
	::SiteLookup::const_pointer i = ::siteLookup.find(s);
	return i ? *i : 0;
}


Site*
Site::searchSite(mstl::string const& name)
{
	// TODO: search word-wise
	return findSite(name);
}


int
Site::findCountry(country::Code country) const
{
	for (int index = 0; index < int(m_ascii.size()); ++index)
	{
		if (m_ascii[index].country == country)
			return index;
	}

	return -1;
}


Site*
Site::newSite(mstl::string const& name, unsigned region, country::Code country, bool isCountryName)
{
	//M_ASSERT(!name.empty());
	////M_ASSERT(sys::utf8::validate(name));
	//M_ASSERT(region <= 6);

	mstl::string buf;
	mstl::string const& ascii = name; //sys::utf8::Codec::convertToNonDiacritics(region, name, buf);

	mstl::string key(ascii);
	normalize(key);

	char* s = ::charAllocator.alloc(key.size() + 1);
	::memcpy(s, key, key.size() + 1);

	mstl::string key2;
	key2.hook(s, key.size());

	Site*& site = ::siteLookup.find_or_insert(key2, 0);
	bool hasCountry = false;

	if (site == 0 || site->m_name != name)
	{
		if (site != 0)
		{
			DEBUG(::printf("clash of key %s: %s - %s\n", key2.c_str(), name.c_str(), site->m_name.c_str()));
			::removeSites.push_back(key2);
		}

		site = ::siteAllocator.alloc();
		s = ::charAllocator.alloc(name.size() + 1);
		::memcpy(s, name, name.size());
		s[name.size()] = '\0';
		site->m_name.hook(s, name.size());
		::siteList.push_back(SiteList::value_type(site->m_name, site));

		if (isCountryName)
			::countryLookup.insert_unique(site->m_name, country);
	}
	else
	{
		::charAllocator.shrink(key.size() + 1, 0);
		hasCountry = site->containsCountry(country);
	}

	mstl::string normalized(name);
	normalize(normalized);

	if (normalized != key)
	{
		Site const* p = ::siteLookup.find_or_insert(normalized, site);

		if (!match(p, site) || p->name() != site->name())
		{
			DEBUG(::printf("clash of normalized %s: %s - %s\n",
								normalized.c_str(),
								p->m_name.c_str(),
								p->m_name.c_str()));
			::removeSites.push_back(normalized);
		}
	}

	if (hasCountry)
	{
		DEBUG(::printf("City '%s' in country %s already exists\n",
							name.c_str(),
							country::toString(country)));
	}
	else
	{
		site->m_ascii.push_back();
		site->m_ascii.back().country = country;

		if (ascii == name)
		{
			site->m_ascii.back().name = site->m_name;
		}
		else
		{
			s = ::charAllocator.alloc(ascii.size() + 1);
			::memcpy(s, ascii, ascii.size());
			s[ascii.size()] = '\0';
			site->m_ascii.back().name.hook(s, ascii.size());
			::siteList.push_back(SiteList::value_type(site->m_ascii.back().name, site));

			if (isCountryName)
				::countryLookup.insert_unique(site->m_ascii.back().name, country);
		}
	}

	return site;
}


bool
Site::newAlias(mstl::string const& name,
					unsigned region,
					bool isEnglish,
					country::Code country,
					bool isCountryName,
					Site* site)
{
	//M_ASSERT(name);
	//M_ASSERT(site);
	////M_ASSERT(sys::utf8::validate(name));
	//M_ASSERT(region <= 6);

	mstl::string buf;
	mstl::string const& ascii = name;
		//sys::utf8::Codec::convertToNonDiacritics(region, name, buf);

	mstl::string key(ascii);
	key.make_writable();
	normalize(key);

	if (key.empty())
		return true;

	char* s = ::charAllocator.alloc(key.size() + 1);
	::memcpy(s, key, key.size() + 1);

	mstl::string key2;
	key2.hook(s, key.size());

	s = ::charAllocator.alloc(name.size() + 1);
	::memcpy(s, name, name.size());
	s[name.size()] = '\0';
	mstl::string name2;
	name2.hook(s, name.size());

	::siteList.push_back(SiteList::value_type(name2, site));
	::aliasDict.find_or_insert(City(site, country), ::StringList()).push_back(name2);

	Site const* p = ::siteLookup.find_or_insert(key2, site);

	if (!match(p, site))
	{
		DEBUG(::printf("clash of key %s: %s - %s\n",
							key2.c_str(),
							p->m_name.c_str(),
							site->m_name.c_str()));
		::removeSites.push_back(key2);
	}

	mstl::string normalized(name);
	normalize(normalized);

	if (normalized != key)
	{
		p = ::siteLookup.find_or_insert(normalized, site);

		if (!match(p, site))
		{
			DEBUG(::printf("clash of normalized %s: %s - %s\n",
								normalized.c_str(),
								p->m_name.c_str(),
								site->m_name.c_str()));
			::removeSites.push_back(normalized);
		}
	}

	int index = site->findCountry(country);

	if (index == -1)
		return false;

	if (isCountryName)
		::countryLookup.insert_unique(name2, country);

	if (ascii == name2)
	{
		if (isEnglish)
			site->m_ascii[index].name = name2;
	}
	else
	{
		mstl::string ascii2;

		s = ::charAllocator.alloc(ascii.size() + 1);
		::memcpy(s, ascii, ascii.size());
		s[ascii.size()] = '\0';
		ascii2.hook(s, ascii.size());

		if (isEnglish)
			site->m_ascii[index].name = ascii2;

		p = ::siteLookup.find_or_insert(ascii2, site);

		if (!match(p, site))
			::removeSites.push_back(ascii2);

		::siteList.push_back(SiteList::value_type(ascii2, site));

		if (isCountryName)
			::countryLookup.insert_unique(ascii2, country);
	}

	return p != site;
}


unsigned
Site::findMatches(mstl::string const& name, Matches& result, unsigned maxMatches)
{
	typedef int (*Compare)(const void *, const void *);

	mstl::string::size_type n = result.size();

	if (maxMatches <= n)
		return 0;

 	SiteList::const_iterator i = mstl::lower_bound(::siteList.begin(), ::siteList.end(), name);

	if (i == ::siteList.end() || i->first < name || !::isPrefix(name, i->first))
		return 0;

	::insert(result, i->second);
	++i;

	mstl::string::size_type maxSize = maxMatches + n;

	while (result.size() < maxSize && i != ::siteList.end() && ::isPrefix(name, i->first))
	{
		::insert(result, i->second);
		++i;
	}

	result.resize(mstl::min(maxMatches + n, result.size()));

	::qsort(	result.begin(),
				result.size(),
				sizeof(Matches::value_type),
				reinterpret_cast<Compare>(cmpMatch));

	return result.size() - n;
}


Site::StringList const&
Site::aliases(country::Code country) const
{
	::AliasDict::const_pointer i = ::aliasDict.find(City(this, country));
	return i ? *i : ::emptyList;
}


void
Site::loadDone()
{
	typedef int (*Compare)(const void *, const void *);

	for (unsigned i = 0; i < ::removeSites.size(); ++i)
		::siteLookup.remove(::removeSites[i]);
	::removeSites.clear();

	::qsort(	::siteList.begin(),
				::siteList.size(),
				sizeof(SiteList::value_type),
				reinterpret_cast<Compare>(cmpAssoc));

	DEBUG(::printf("Sites total:   %u\n", ::siteList.size()));
	DEBUG(::printf("Sites hashed:  %u\n", ::siteLookup.used()));
	DEBUG(::printf("Aliases total: %u\n", ::aliasDict.used()));
}


country::Code
Site::findCountryCode(mstl::string const& countryName)
{
	::CountryLookup::const_pointer i = ::countryLookup.find(countryName);
	return i ? *i : country::Unknown;
}


void
Site::parseFile(mstl::istream& stream)
{
	DEBUG(unsigned lineno = 0);

	mstl::string	line;
	mstl::string	name;
	mstl::string	str;
	country::Code	country	= country::Unknown;
	Site*				site		= 0;
	bool				first		= false;

	siteList.reserve(12000);

	while (stream.getline(line))
	{
		char* s = line.data();

		DEBUG(++lineno);

		if (line.size() >= 3)
		{
			if (::isalpha(*s))
			{
				line.set_size(3);
				country = country::fromString(line);
				first = true;
			}
			else if (::isspace(*s))
			{
				unsigned region = 0;

				s = ::skipSpace(s + 1);

				char* t = line.end() - 1;

				while (t > s && ::isspace(*t))
					--t;

				if (t - s > 3 && t[0] == ']' && t[-2] == '[')
				{
					region = t[-1] - '0';

					t -= 2;
					while (t > s && ::isspace(t[-1]))
						--t;
					*t = '\0';
				}
				else
				{
					++t;
				}

				if (region <= 6)
				{
					str.hook(s, t - s);

					//if (sys::utf8::Codec::fitsRegion(str, region))
					{
						if (*s == '=' || *s == '!')
						{
							if (site)
							{
								char* p = ::skipSpace(s + 1);
								str.hook(p, t - p);
								newAlias(str, region, *s == '!', country, first, site);
							}
						}
						else if (*s)
						{
							site = newSite(str, region, country, first);
						}
					}
					/*else
					{
						DEBUG(::printf("line %u: name does not fit region %u\n", lineno, region));
					}*/
				}
				else
				{
					DEBUG(::printf("line %u: invalid region\n", lineno));
				}
			}
		}
	}
}

// vi:set ts=3 sw=3:
