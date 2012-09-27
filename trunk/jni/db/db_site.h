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

#ifndef db_site_included
#define db_site_included

#include "db_common.h"

#include "m_string.h"
#include "m_pair.h"
#include "m_vector.h"

namespace mstl { class istream; }

namespace db {

class Namebase;

class Site
{
public:

	struct NonDiacritic
	{
		mstl::string	name;
		country::Code	country;
	};

	typedef mstl::pair<country::Code,Site const*> Match;
	typedef mstl::vector<mstl::string> StringList;
	typedef mstl::vector<Match> Matches;

	bool containsCountry(country::Code country) const;

	unsigned countCountries() const;
	mstl::string const& name() const;
	mstl::string const& nonDiacriticName(country::Code country) const;
	country::Code country(unsigned n) const;

	StringList const& aliases(country::Code country) const;

	static Site* findSite(mstl::string const& name);
	static Site* searchSite(mstl::string const& name);
	static unsigned findMatches(mstl::string const& name, Matches& result, unsigned maxMatches);
	static unsigned countSites();
	static mstl::string& normalize(mstl::string& name);
	static country::Code findCountryCode(mstl::string const& countryName);

	static void parseFile(mstl::istream& stream);
	static void loadDone();

private:

	typedef mstl::vector<NonDiacritic> NonDiacritics;

	int findCountry(country::Code country) const;

	static Site* newSite(mstl::string const& name,
								unsigned region,
								country::Code country,
								bool isCountryName);
	static bool newAlias(mstl::string const& name,
								unsigned region,
								bool isEnglish,
								country::Code country,
								bool isCountryName,
								Site* site);

	mstl::string	m_name;
	NonDiacritics	m_ascii;
};

} // namespace db

#include "db_site.ipp"

#endif // _db_site_included

// vi:set ts=3 sw=3:
