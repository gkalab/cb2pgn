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
#include "db_namebase_entry.h"
#include "db_player_stats.h"
#include "db_eco_table.h"

#include "m_hash.h"
#include "m_map.h"
#include "m_vector.h"
#include "m_chunk_allocator.h"
#include "m_algorithm.h"
#include "m_istream.h"
#include "m_utility.h"
#include "m_bit_functions.h"
#include "m_assert.h"
#include "m_stdio.h"

#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include <string.h>

using namespace db;

//#define DEBUG

#ifdef DEBUG
# undef DEBUG
# define DEBUG(stmt) stmt
#else
# define DEBUG(stmt)
#endif

//#define USE_CONFLICT_MAP

unsigned Player::m_minELO	= 1800;
unsigned Player::m_minDWZ	= 1800;
unsigned Player::m_minECF	=  110;
unsigned Player::m_minICCF	= 1800;


namespace {

struct PndID
{
	uint32_t prefix:27;	// 8 digits
	uint32_t suffix:4;	// 9. digit (possibly an 'X', decoded as number 10)

	mstl::string asString() const;
	void set(char const* id);
};

typedef uint32_t ViafID;


mstl::string
PndID::asString() const
{
	mstl::string result;
	result.reserve(9);

	result.format("%u", prefix);
	result += (suffix == 10 ? 'X' : '0' + suffix);

	return result;
}


void
PndID::set(char const* id)
{
	// require: #id >= 9
	// require: variable 'id' is writable

	char c = id[8];

	const_cast<char*>(id)[8] = '\0';
	prefix = ::strtoul(id, nullptr, 10);
	suffix = c == 'X' ? 10 : c - '0';
	const_cast<char*>(id)[8] = c;
}

} // namespace


typedef Player::StringList StringList;
typedef mstl::pair<mstl::string,Player const*> Entry;
typedef mstl::vector<Player*> Players;

typedef mstl::hash<mstl::string,Players> PlayerLookup;
typedef mstl::hash<mstl::string,Player*> ConflictMap;
typedef mstl::hash<Player*,StringList> AliasDict;
typedef mstl::map<Player*,PndID> PndDict;
typedef mstl::map<Player*,ViafID> ViafDict;
typedef mstl::map<Player const*,mstl::string> Lookup;
typedef mstl::vector<Entry> PlayerList;
typedef mstl::hash<unsigned,Player*> PlayerDict;
typedef mstl::hash<mstl::string,Lookup*> LangMap;
typedef mstl::chunk_allocator<char> CAllocator;
typedef mstl::chunk_allocator<Player> PAllocator;

static Player InvalidEntry;
static PlayerLookup playerLookup(unsigned(200000*(100/PlayerLookup::Load)));
static AliasDict aliasDict(unsigned(150000*(100/AliasDict::Load)));
static PlayerDict playerDict(unsigned(120000*(100/PlayerDict::Load)));
static Lookup asciiDict(8192);
static Lookup chessgamesDict(8192);
static CAllocator charAllocator(1024);
static PAllocator playerAllocator(32768);
static PlayerList playerList;
static mstl::string exclude;
static LangMap langMap;
static PndDict pndMap(250);
static ViafDict viafMap(250);
static StringList emptyList;
#ifdef USE_CONFLICT_MAP
static ConflictMap conflictMap;
#endif


namespace db {

inline
bool
operator<(PlayerList::value_type const& lhs, mstl::string const& rhs)
{
	return lhs.first < rhs;
}

} // namespace db


static void
alloc(mstl::string& dst, mstl::string const& src)
{
	char* s = charAllocator.alloc(src.size() + 1);
	::memcpy(s, src, src.size());
	s[src.size()] = '\0';
	dst.hook(s, src.size());
}


static bool
equal(Player const* lhs, Player const* rhs)
{
	return	lhs->name() == rhs->name()
			&& lhs->sex() == rhs->sex()
			&& lhs->federation() == rhs->federation();
}


static void
insert(Player::Matches& matches, Player const* player)
{
	// //M_ASSERT(player);

	unsigned i = 0;
	while (i < matches.size() && !equal(matches[i], player))
		++i;

	if (i == matches.size())
		matches.push_back(player);
}


static int
cmpAssoc(PlayerList::value_type const* lhs, PlayerList::value_type const* rhs)
{
	return 0; //string::compare(lhs->first, rhs->first);
}


static int
cmpMatch(Player::Matches::value_type const* lhs, Player::Matches::value_type const* rhs)
{
	return 0; //ys::utf8::compare((*lhs)->name(), (*rhs)->name());
}


static char const*
findSpace(char const* s)
{
	while (*s && !isspace(*s))
		++s;

	return s;
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


static char const*
skipSpaces(char const* s)
{
	while (isspace(*s))
		++s;

	return s;
}


static mstl::string&
toupper(mstl::string& s)
{
	for (mstl::string::iterator i = s.begin() + 1; i != s.end(); ++i)
	{
		if (::islower(*i))
			*i = ::toupper(*i);
	}

	return s;
}


static void
extractPlayerData(mstl::string& str,
						sex::ID& sex,
						mstl::string& titles,
						mstl::string& countries,
						mstl::string& elo,
						Date& birthDate,
						Date& deathDate)
{
	titles.clear();
	countries.clear();
	elo.clear();
	birthDate.clear();
	deathDate.clear();

	char const* s = str.c_str();
	char const* t = ::strchr(s, '#');

	if (t)
	{
		char const* p = t;
		char const* e = ++t;

		if (*t == 'w')
			sex = sex::Female;

		while (*e && !::isspace(*e))
			++e;
		titles.assign(t, e);
		t = e;
		while (::isspace(*t))
			++t;

		if (*t)
		{
			e = t;
			while (*e && !::isspace(*e))
				++e;
			countries.assign(t, e);
			t = e;
			while (::isspace(*t))
				++t;

			if (*t)
			{
				e = t;
				while (*e && !::isspace(*e))
					++e;
				elo.assign(t, e);
				t = e;
				while (::isspace(*t))
					++t;

				if (*t)
				{
					e = t;
					while (*e && *e != '-' && !::isspace(*e))
						++e;
					birthDate.parseFromString(t, e - t);
					t = e;

					if (*t == '-')
					{
						while (*t == '-')
							++t;
						e = t;
						while (*e && *e != '-' && !::isspace(*e))
							++e;
						deathDate.parseFromString(t, e - t);
					}
				}
			}
		}

		str.resize(p - s);
	}

	str.trim();
}


static unsigned
getTitles(mstl::string const& str)
{
	char const*	s = str.c_str();
	unsigned		titles = 0;

	while (*s)
	{
		char const* t = s;

		while (*t && *t != '+')
			++t;

		unsigned len = t - s;

		switch (len)
		{
			case 2:
				if (tolower(s[1]) == 'm')
				{
					switch (tolower(s[0]))
					{
						case 'g': titles |= title::Mask_GM; break;
						case 'i': titles |= title::Mask_IM; break;
						case 'f': titles |= title::Mask_FM; break;
					}
				}
				break;

			case 3:
				if (tolower(s[2]) == 'm')
				{
					switch (tolower(s[0]))
					{
						case 'c':
							switch (tolower(s[1]))
							{
								case 'g': titles |= title::Mask_CGM; break;
								case 'i': titles |= title::Mask_CIM; break;
							}
							break;

						case 'h':
							if (tolower(s[1]) == 'g')
								titles |= title::Mask_HGM;
							break;

						case 'w':
							switch (tolower(s[1]))
							{
								case 'g': titles |= title::Mask_WGM; break;
								case 'i': titles |= title::Mask_WIM; break;
								case 'f': titles |= title::Mask_WFM; break;
							}
							break;
					}
				}
				break;
		}

		s += len;
		if (*s == '+')
			++s;
	}

	return titles;
}


static country::Code
getFederation(mstl::string const& federations)
{
	if (federations.size() < 3)
		return country::Unknown;

	return country::fromString(federations.end() - 3);
}


static country::Code
getNativeCountry(mstl::string const& federations)
{
	if (federations.size() < 3)
		return country::Unknown;

	return country::fromString(federations);
}


static bool
isTitle(char const* s, char const* e)
{
	return (e - s == 2 || (e - s == 3 && ::isupper(s[2]))) && title::fromString(s) != title::None;
}


static int
getElo(mstl::string const& elo)
{
	if (	elo.size() >= 6
		&& elo[0] == '['
		&& isdigit(elo[1])
		&& isdigit(elo[2])
		&& isdigit(elo[3])
		&& isdigit(elo[4])
		&& (	elo[5] == ']'
			|| (elo.size() >= 7 && elo[5] == '*' && elo[6] == ']')))
	{
		int value = strtoul(elo.c_str() + 1, nullptr, 10);

		if (value <= rating::Max_Value)
		{
			if (elo[5] == '*')
				value = -value;

			return value;
		}
	}

	return 0;
}


static bool
hasTrailingCountryCode(mstl::string const& s)
{
	return	s.size() > 6
			&& *(s.end() - 5) != ','
			&& *(s.end() - 4) == ' '
			&& ::isupper(*(s.end() - 3))
			&& ::isupper(*(s.end() - 2))
			&& ::isupper(*(s.end() - 1));
}


static mstl::string&
stripCountryCode(mstl::string& s)
{
	//M_ASSERT(hasTrailingCountryCode(s));

	// strip country following name; e.g. "Mueller, Hans AUT"
	unsigned size = s.size() - 4;

	while (size > 0 && s[size - 1] == ' ')
		--size;

	s.set_size(size);
	return s;
}


static void
standardizeNames(char const* name, char* result)
{
	switch (name[0])
	{
		case 'D':
			if (::strncmp(name + 1, "e La ", 5) == 0)
			{
				// "De La " -> "De la "
				result[3] = 'l';
			}
			break;

		case 'V':
			if (	(name[1] == 'a' || name[1] == 'o')
				&& ::strncmp(name + 2, "n De", 4) == 0
				&& (name[6] == ' ' || ((name[6] == 'r' || name[6] == 'n') && name[7] == ' ')))
			{
				// "Van De "  -> "Van de "
				// "Van Den " -> "Van den "
				// "Van Der " -> "Van der "
				// "Von De "  -> "Von de "		(not really wanted, but a harmless side effect)
				// "Von Den " -> "Von den "
				// "Von Der " -> "Von der "
				result[4] = 'd';
			}
	}
}


static mstl::string const&
insertAlias(Player::StringList& sl, mstl::string const& alias)
{
	for (unsigned i = 0; i < sl.size(); ++i)
	{
		if (sl[i] == alias)
			sl[i];
	}

	sl.push_back();
	::alloc(sl.back(), alias);
	return sl.back();
}


static bool
containsPlayer(Players const& players, country::Code federation, sex::ID sex)
{
	//M_ASSERT(players.size() >= 1);

	if (federation != country::Unknown)
	{
		Player* possibleMatch = 0;

		for (unsigned i = 0; i < players.size(); ++i)
		{
			Player* player = players[i];

			if (sex == sex::Unspecified || player->sex() == sex::Unspecified || sex == player->sex())
			{
				if (player->federation() == country::Unknown)
					return true;

				if (federation == player->federation() || federation == player->nativeCountry())
					return true;

				if (	country::match(federation, player->federation())
					|| country::match(federation, player->nativeCountry()))
				{
					possibleMatch = player;
				}
			}
		}

		return possibleMatch != 0;
	}

	if (sex != sex::Unspecified)
	{
		for (unsigned i = 0; i < players.size(); ++i)
		{
			Player* player = players[i];

			if (player->sex() == sex::Unspecified)
				return true;

			if (player->sex() == sex)
				return true;
		}

		return false;
	}

	return true;
}


static Player*
findPlayer(mstl::string const& name, Players const& players)
{
	if (players.size() == 1)
		return players.front();

	for (unsigned i = 0; i < players.size(); ++i)
	{
		::StringList const* sl = ::aliasDict.find(players[i]);

		if (sl)
		{
			for (unsigned k = 0; k < sl->size(); ++k)
			{
				if ((*sl)[k] == name)
					return players[i];
			}
		}
	}

	return 0;
}


static Player*
findPlayer(mstl::string const& name, Players const& players, country::Code federation, sex::ID sex)
{
	//M_ASSERT(players.size() >= 1);

	if (federation != country::Unknown)
	{
		Player* possibleMatch = 0;

		for (unsigned i = 0; i < players.size(); ++i)
		{
			Player* player = players[i];

			if (sex == sex::Unspecified || player->sex() == sex::Unspecified || sex == player->sex())
			{
				if (player->federation() == country::Unknown)
					return findPlayer(name, players);

				if (federation == player->federation() || federation == player->nativeCountry())
					return player;

				if (	country::match(federation, player->federation())
					|| country::match(federation, player->nativeCountry()))
				{
					possibleMatch = player;
				}
			}
		}

		return possibleMatch;
	}

	if (sex != sex::Unspecified)
	{
		for (unsigned i = 0; i < players.size(); ++i)
		{
			Player* player = players[i];

			if (player->sex() == sex::Unspecified)
				return findPlayer(name, players);

			if (player->sex() == sex)
				return player;
		}

		return 0;
	}

	return findPlayer(name, players);
}


Player::PlayerCallback::~PlayerCallback() {}


Player::Player()
	:m_fideID(0)
	,m_ratingType(rating::Last)
	,m_sex(sex::Unspecified)
	,m_iccfID(0)
	,m_dsbMglNr(0)
	,m_chess960(0)
	,m_uscfID(0)
	,m_birthDay(0)
	,m_winboard(0)
	,m_uci(0)
	,m_ecfPrefix(0)
	,m_birthYear(0)
	,m_species(species::Human)
	,m_titles(0)
	,m_deathYear(0)
	,m_deathDay(0)
	,m_zpsSuffix(0)
	,m_federation(country::Unknown)
	,m_birthMonth(0)
	,m_deathMonth(0)
	,m_notUnique(0)
	,m_nativeCountry(country::Unknown)
	,m_zpsPrefix(0)
	,m_ecfSuffix(0)
	,m_region(0)
{
	::memset(m_latestRating, 0, sizeof(m_latestRating));
	::memset(m_highestRating, 0, sizeof(m_highestRating));
}


unsigned
Player::countPlayers()
{
	return ::playerLookup.used();
}


bool
Player::isNormalized(mstl::string const& name)
{
	// TODO: possibly not working with Utf8-strings
	return name.find_first_of(::exclude) == mstl::string::npos;
}


mstl::string&
Player::normalize(mstl::string& name)
{
	if (!name.empty())
	{
		name.make_writable();

		//sys::utf8::Codec::firstCharToUpper(name);
		::standardizeNames(name, name.data());

		// TODO: possibly not working with Utf8-strings
		mstl::string::size_type n = name.find_first_of(::exclude);

		while (n != mstl::string::npos)
		{
			name.erase(name.begin() + n);
			n = name.find_first_of(::exclude, n);
		}
	}

	return name;
}


mstl::string&
Player::normalize(mstl::string const& name, mstl::string& result)
{
	//M_REQUIRE(name.c_str() != result.c_str());

	result.clear();

	if (!name.empty())
	{
		if (::exclude.find(name[0]) == mstl::string::npos) {}
			//sys::utf8::Codec::firstCharToUpper(name, result);

		// TODO: possibly not working with Utf8-strings
		for (unsigned i = 1; i < name.size(); ++i)
		{
			char c = name[i];

			if (::exclude.find(c) == mstl::string::npos)
				result += c;
		}

		::standardizeNames(name, result.data() - 1);
	}

	return result;
}


void
Player::standardizeNames(mstl::string& name)
{
	switch (name[0])
	{
		case 'd':
			if (name[1] && name[2] == ' ' && ::strchr("aeiou", name[1]))
			{
				// "da " -> "Da "
				// "de " -> "De "
				// "di " -> "Di "
				// "do " -> "Do "
				// "du " -> "Du "
				name[0] = 'D';
			}
			break;

		case 't':
			if (name[1] == 'e' && (name[2] == ' ' || (name[2] == 'r' && name[3] == ' ')))
			{
				// "te "  -> "Te "
				// "ter " -> "Ter "
				name[0] = 'T';
			}
			break;

		case 'v':
			if ((name[1] == 'a' || name[1] == 'o') && name[2] == 'n' && name[3] == ' ')
			{
				// "van" -> "Van"
				// "von" -> "Von"
				name[0] = 'V';
			}
			break;

		case 'z':
			if (name[1] == 'u' && (name[2] == 'm' || name[2] == 'r'))
			{
				// "zum " -> "Zum "
				// "zur " -> "Zur "
				name[0] = 'Z';
			}
			break;
	}

	::standardizeNames(name, name.data());
}


bool
Player::containsPlayer(mstl::string const& name, country::Code federation, sex::ID sex)
{
	mstl::string key(name);
	normalize(key);

	::Players const* p = ::playerLookup.find(key);

	if (p == 0)
		return false;

	return ::containsPlayer(*p, federation, sex);
}


Player*
Player::findPlayer(uint32_t fideID)
{
	Player* const* playerEntry = ::playerDict.find(fideID);
	return playerEntry ? *playerEntry : 0;
}


Player*
Player::findPlayer(mstl::string const& name, country::Code federation, sex::ID sex)
{
	mstl::string key(name);
	normalize(key);

	::Players const* p = ::playerLookup.find(key);
	return p ? ::findPlayer(name, *p, federation, sex) : 0;
}


Player*
Player::insertPlayer(uint32_t fideID, mstl::string const& name)
{
	//M_REQUIRE(findPlayer(fideID) == 0);

	Player* player = new Player;

	::playerDict.insert_unique(fideID, player);

	player->m_name = name;
	player->setFideID(fideID);

	return player;
}


Player*
Player::newPlayer(mstl::string const& name,
						unsigned region,
						mstl::string const& ascii,
						country::Code federation,
						sex::ID sex)
{
	//M_ASSERT(!name.empty());
	////M_ASSERT(sys::utf8::validate(name));
	////M_ASSERT(sys::utf8::Codec::is7BitAscii(ascii));

	mstl::string key, key2;
	normalize(ascii, key);
	::alloc(key2, key);

	::Players&	players	= ::playerLookup.find_or_insert(key2, Players());
	Player*		player	= 0;

	if (players.empty())
	{
		player = ::playerAllocator.alloc();
	}
	else
	{
		::charAllocator.shrink(key.size() + 1, 0);

		if (federation == country::Unknown)
		{
			if (sex == sex::Unspecified)
			{
				if (players.size() >= 2)
				{
					DEBUG(::printf("cannot distinguish between federation: %s ignored\n", name.c_str()));
					return 0;
				}

				player = players.front();
			}
			else
			{
				for (unsigned i = 0; i < players.size(); ++i)
				{
					Player* p = players[i];

					if (p->sex() == sex || p->sex() == sex::Unspecified)
					{
						player = p;
						break;
					}
				}
			}
		}
		else
		{
			if (sex == sex::Unspecified)
			{
				for (unsigned i = 0; i < players.size(); ++i)
				{
					Player* p = players[i];

					if (federation == p->federation())
					{
						player = p;
						break;
					}
				}
			}
			else
			{
				for (unsigned i = 0; i < players.size(); ++i)
				{
					Player* p = players[i];

					if (p->sex() == sex::Unspecified || p->sex() == sex)
					{
						if (federation == p->federation())
						{
							player = p;
							break;
						}
					}
				}
			}
		}

		if (player == 0)
			player = ::playerAllocator.alloc();
	}

	if (player->federation() == country::Unknown)
		player->m_federation = federation;

	if (player->sex() == sex::Unspecified)
	{
		if (sex != sex::Unspecified)
		{
			player->m_sex = sex;
			player->m_species = species::Human;
		}
	}

	if (player->m_name.empty())
	{
		players.push_back(player);
		::alloc(player->m_name, name);
		player->m_region = region;
		player->m_nativeCountry = federation;
		::playerList.push_back(PlayerList::value_type(player->m_name, player));

		if (ascii.c_str() != name.c_str())
		{
			//M_ASSERT(ascii != name);

			mstl::string ascii2;
			::alloc(ascii2, ascii);

			::playerList.push_back(PlayerList::value_type(ascii2, player));
			::asciiDict[player] = ascii2;
		}
	}

	if (name != player->m_name)
	{
		::StringList& sl = ::aliasDict.find_or_insert(player, ::StringList());
		::insertAlias(sl, name);
	}

	if (ascii != name)
	{
		::StringList& sl = ::aliasDict.find_or_insert(player, ::StringList());
		::insertAlias(sl, name);

		normalize(name, key);
		::alloc(key2, key);
		::playerLookup.find_or_insert(key2, players);
	}

	//M_ASSERT(!players.empty());

	return player;
}


bool
Player::newAlias(mstl::string const& name, mstl::string const& ascii, Player* player)
{
	//M_ASSERT(player);
	//M_ASSERT(!name.empty());
	////M_ASSERT(sys::utf8::validate(name));
	////M_ASSERT(sys::utf8::Codec::is7BitAscii(ascii));

	mstl::string key, key2;
	normalize(ascii, key);
	::alloc(key2, key);

	::Players& players = ::playerLookup.find_or_insert(key2, Players());

	if (!players.empty())
	{
		country::Code	federation	= player->federation();
		sex::ID			sex			= player->sex();

		::charAllocator.shrink(key.size() + 1, 0);

		for (unsigned i = 0; i < players.size(); ++i)
		{
			if (players[i] == player)
			{
				if (players[i]->name() != name)
				{
					::StringList& sl = ::aliasDict.find_or_insert(player, ::StringList());
					::insertAlias(sl, name);
					::insertAlias(sl, ascii);
				}

				return true;
			}
		}

		if (federation == country::Unknown)
		{
			if (sex == sex::Unspecified)
			{
				if (players.size() >= 2)
				{
					DEBUG(::printf("cannot distinguish between federation: alias %s ignored\n",
										name.c_str()));
					return false;
				}

				player = players.front();
			}
			else
			{
				for (unsigned i = 0; i < players.size(); ++i)
				{
					Player* p = players[i];

					if (p->sex() == sex || p->sex() == sex::Unspecified)
					{
						DEBUG(::printf("alias %s already exists\n", name.c_str()));
						return false;
					}
				}
			}
		}
		else
		{
			if (sex == sex::Unspecified)
			{
				for (unsigned i = 0; i < players.size(); ++i)
				{
					Player* p = players[i];

					if (country::match(federation, p->federation()))
					{
						DEBUG(::printf("alias %s already exists\n", name.c_str()));
						return false;
					}
				}
			}
			else
			{
				for (unsigned i = 0; i < players.size(); ++i)
				{
					Player* p = players[i];

					if (	(p->sex() == sex::Unspecified || p->sex() == sex)
						&& country::match(federation, p->federation()))
					{
						DEBUG(::printf("alias %s already exists\n", name.c_str()));
						return false;
					}
				}
			}
		}
	}

	players.push_back(player);

	::StringList& sl = ::aliasDict.find_or_insert(player, ::StringList());
	::insertAlias(sl, name);
	::insertAlias(sl, ascii);

	if (	asciiDict.find(player) == asciiDict.end()
		/*&& (	sys::utf8::Codec::matchAscii(player->name(), ascii)
			|| (	country::isGermanSpeakingCountry(player->federation())
				&& sys::utf8::Codec::matchGerman(player->name(), ascii)))*/)
	{
		::alloc(::asciiDict[player], ascii);
	}

	if (ascii.c_str() != name.c_str())
	{
		//M_ASSERT(ascii != name);
		sl.push_back();
		::alloc(sl.back(), ascii);
		::playerList.push_back(PlayerList::value_type(sl.back(), player));

		normalize(name, key);
		::alloc(key2, key);
		::playerLookup.find_or_insert(key2, players);
	}

	return true;
}


bool
Player::replaceName(mstl::string const& name, mstl::string const& ascii, Player* player)
{
	//M_ASSERT(player);
	//M_ASSERT(!name.empty());
	////M_ASSERT(sys::utf8::validate(name));
	////M_ASSERT(sys::utf8::Codec::is7BitAscii(ascii));

	bool found = false;

	mstl::string key, key2;
	normalize(ascii, key);
	::alloc(key2, key);

	::Players& players = ::playerLookup.find_or_insert(key2, Players());

	if (!players.empty())
	{
		::charAllocator.shrink(key.size() + 1, 0);

		for (unsigned i = 0; i < players.size(); ++i)
		{
			if (players[i] == player)
				found = true;
		}

		if (!found)
		{
			country::Code	federation	= player->federation();
			sex::ID			sex			= player->sex();

			if (federation == country::Unknown)
			{
				if (sex == sex::Unspecified)
				{
					if (players.size() >= 2)
					{
						DEBUG(::printf("cannot distinguish between federation: name %s ignored\n",
											name.c_str()));
						return false;
					}

					player = players.front();
				}
				else
				{
					for (unsigned i = 0; i < players.size(); ++i)
					{
						Player* p = players[i];

						if (p->sex() == sex || p->sex() == sex::Unspecified)
						{
							DEBUG(::printf("name %s already exists\n", name.c_str()));
							return false;
						}
					}
				}
			}
			else
			{
				if (sex == sex::Unspecified)
				{
					for (unsigned i = 0; i < players.size(); ++i)
					{
						Player* p = players[i];

						if (country::match(federation, p->federation()))
						{
							DEBUG(::printf("name %s already exists\n", name.c_str()));
							return false;
						}
					}
				}
				else
				{
					for (unsigned i = 0; i < players.size(); ++i)
					{
						Player* p = players[i];

						if (	(p->sex() == sex::Unspecified || p->sex() == sex)
							&& country::match(federation, p->federation()))
						{
							DEBUG(::printf("name %s already exists\n", name.c_str()));
							return 0;
						}
					}
				}
			}
		}
	}

	if (!found)
	{
		if (mstl::find(players.begin(), players.end(), player) == players.end())
			players.push_back(player);
	}

	::StringList& sl = ::aliasDict.find_or_insert(player, ::StringList());
	::playerList.push_back(PlayerList::value_type(::insertAlias(sl, name), player));

	if (ascii.c_str() != name.c_str())
	{
		//M_ASSERT(ascii != name);

		mstl::string const& s = ::insertAlias(sl, ascii);
		::alloc(::asciiDict[player], s);
		::playerList.push_back(PlayerList::value_type(s, player));

		normalize(name, key);
		::alloc(key2, key);
		::playerLookup.find_or_insert(key2, players);
	}

	return true;
}


Player*
Player::insertPlayer(mstl::string& name, country::Code federation, sex::ID sex)
{
	////M_ASSERT(sys::utf8::Codec::is7BitAscii(name));

	if (name.empty())
		return 0;

	standardizeNames(name);

	return newPlayer(name, 0, name, federation, sex);
}


Player*
Player::insertPlayer(mstl::string& name, unsigned region, country::Code federation, sex::ID sex)
{
	if (name.empty())
		return 0;

	standardizeNames(name);

	/*if (sys::utf8::Codec::is7BitAscii(name))
		return newPlayer(name, 0, name, federation, sex);*/

	if (region == 0)
	{
		if (federation == country::Unknown)
		{
			/*if (sys::utf8::Codec::fitsRegion(name, 1))
			{
				region = 1;
			}
			else if (sys::utf8::Codec::fitsRegion(name, 2))
			{
				region = 2;
			}
			else if (sys::utf8::Codec::fitsRegion(name, 4))
			{
				region = 4;
			}
			else*/
			{
				DEBUG(::printf("'%s' does not fit region 1, 2, and 4\n", name.c_str()));
				return 0;;
			}
		}
		else
		{
			region = country::toRegion(federation);
		}
	}
	return 0;/*
	if (!sys::utf8::Codec::fitsRegion(name, region))
	{
		DEBUG(::printf("'%s' does not fit region %u\n", name.c_str(), region));
		return 0;;
	}

	mstl::string ascii;
	sys::utf8::Codec::convertToNonDiacritics(region, name, ascii);

	return newPlayer(name, region, ascii, federation, sex);*/
}


bool
Player::insertAlias(mstl::string& name, Player* player)
{
	////M_ASSERT(sys::utf8::Codec::is7BitAscii(name));

	if (name.empty())
		return 0;

	standardizeNames(name);

	return newAlias(name, name, player);
}


bool
Player::insertAlias(mstl::string& name, unsigned region, Player* player)
{
	if (name.empty())
		return 0;

	standardizeNames(name);

	//if (sys::utf8::Codec::is7BitAscii(name))
	//	return newAlias(name, name, player);

	if (region == 0)
	{
		if (player->federation() == country::Unknown)
			region = 1;
		else
			region = country::toRegion(player->federation());
	}

	//DEBUG(if (!sys::utf8::Codec::fitsRegion(name, region))
	//			return ::printf("'%s' does not fit region %u\n", name.c_str(), region) == 0);

	mstl::string ascii;
	//sys::utf8::Codec::convertToNonDiacritics(region, name, ascii);

	return newAlias(name, ascii, player);
}


bool
Player::replaceName(mstl::string& name, unsigned region, Player* player)
{
	if (name.empty())
		return 0;

	standardizeNames(name);

	//if (sys::utf8::Codec::is7BitAscii(name))
		return replaceName(name, name, player);
/*
	if (region == 0)
	{
		if (player->federation() == country::Unknown)
			region = 1;
		else
			region = country::toRegion(player->federation());
	}

	mstl::string ascii;

	if (country::isGermanSpeakingCountry(player->federation()))
	{
		DEBUG(if (!sys::utf8::Codec::fitsRegion(name, 1))
					return ::printf("'%s' does not fit region %u\n", name.c_str(), 1) == 0);

		sys::utf8::Codec::mapFromGerman(name, ascii);
	}
	else
	{
		DEBUG(if (!sys::utf8::Codec::fitsRegion(name, region))
					return ::printf("'%s' does not fit region %u\n", name.c_str(), region) == 0);

		sys::utf8::Codec::convertToNonDiacritics(region, name, ascii);
	}

	return replaceName(name, ascii, player);*/
}


mstl::string const&
Player::asciiName() const
{
	//if (sys::utf8::Codec::is7BitAscii(m_name))
		return m_name;
/*
	::Lookup::const_iterator i = ::asciiDict.find(this);

	if (i != ::asciiDict.end())
		return i->second;

	return mstl::string::empty_string;*/
}


mstl::string const&
Player::chessgamesID() const
{
	::Lookup::const_iterator i = ::chessgamesDict.find(this);
	return i == ::chessgamesDict.end() ? mstl::string::empty_string : i->second;
}


unsigned
Player::wikipediaLinks(AssocList& result) const
{
	result.clear();

	for (LangMap::const_iterator i = ::langMap.begin(); i != ::langMap.end(); ++i)
	{
		Lookup::const_iterator k = i->second->find(this);

		if (k != i->second->end())
			result.push_back(mstl::make_pair(i->first, k->second));
	}

	return result.size();
}


Player::StringList const&
Player::aliases() const
{
	::AliasDict::const_pointer i = ::aliasDict.find(const_cast<Player*>(this));
	return i ? *i : ::emptyList;
}


mstl::string
Player::pndID() const
{
	::PndDict::const_iterator i = ::pndMap.find(const_cast<Player*>(this));
	return i == ::pndMap.end() ? mstl::string::empty_string : i->second.asString();
}


unsigned
Player::viafID() const
{
	::ViafDict::const_iterator i = ::viafMap.find(const_cast<Player*>(this));
	return i == ::viafMap.end() ? 0 : i->second;
}


void
Player::setViafID(unsigned id)
{
	::viafMap[this] = id;
}


void
Player::setPndID(char const* id)
{
	//M_REQUIRE(id);
	::pndMap[this].set(id);
}


void
Player::setEcfID(char* id)
{
	//M_REQUIRE(id);
	// require: #id >= 7
	//M_REQUIRE('A' <= id[6] && id[6] <= 'L');

	m_ecfSuffix = id[6] - 'A';

	char c = id[6];
	id[6] = '\0';
	m_ecfPrefix = ::strtoul(id, nullptr, 10);
	id[6] = c;
}


mstl::string
Player::ecfID() const
{
	mstl::string id;

	if (m_ecfPrefix)
		id.format("%u%c", unsigned(m_ecfPrefix), char(m_ecfSuffix + 'A'));

	return id;
}


void
Player::setDsbID(char const* zps, char const* nr)
{
	//M_REQUIRE(zps);
	//M_REQUIRE(nr);
	// require: #zps >= 5
	//M_REQUIRE(('0' <= *zps && *zps <= '9') || ('A' <= *zps && *zps <= 'L'));

	m_zpsPrefix = ::isdigit(*zps) ? *zps - '0' : *zps - 'A' + 10;
	m_zpsSuffix = ::strtoul(zps + 1, nullptr, 10);
	m_dsbMglNr = ::strtoul(nr, nullptr, 10);
}


mstl::string
Player::dsbID() const
{
	mstl::string id;

	if (m_dsbMglNr)
	{
		id.format(	"%c%04u-%u",
						char(m_zpsPrefix <= 9 ? m_zpsPrefix + '0' :  m_zpsPrefix - 10 + 'A'),
						unsigned(m_zpsSuffix),
						unsigned(m_dsbMglNr));
	}

	return id;
}


unsigned
Player::findMatches(mstl::string const& name, Matches& result, unsigned maxMatches)
{
	typedef int (*Compare)(const void *, const void *);

	mstl::string::size_type n = result.size();

	if (maxMatches <= n)
		return 0;

	mstl::string name2(name);
	standardizeNames(name2);

	PlayerList::const_iterator i = mstl::lower_bound(::playerList.begin(), ::playerList.end(), name2);

	if (i == ::playerList.end() || i->first < name2 || !::isPrefix(name2, i->first))
		return 0;

	::insert(result, i->second);
	++i;

	mstl::string::size_type maxSize = maxMatches + n;

	while (result.size() < maxSize && i != ::playerList.end() && ::isPrefix(name2, i->first))
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


void
Player::loadDone()
{
	typedef int (*Compare)(const void *, const void *);

	::qsort(	playerList.begin(),
				playerList.size(),
				sizeof(PlayerList::value_type),
				reinterpret_cast<Compare>(cmpAssoc));
}


// Syntax:
// ---------------------------------------------------------------------
// start reading at token "@PLAYER"
// remove := string after "@PLAYER"
// skip to next line
//
// for each line do
//    if line starts with '@' -> break
//    if line starts with letter (and contains '#')
//       name := start of line - first occurence of '#'
//       right trim name
//       skip '#'
//       sex := next token (w, m)
//       title := next token (wgm(+im)*, '-' -> None)
//       country := next token YYY{/YYY}* (get the final one)
//       elo := extract next token from [dddd] or [dddd*]; "[....]" -> 0
//       birthDate := next token (YY.MM.DDDD; '?' is allowed)
//       if '--' follows
//          deathDate := next token (YY.MM.DDDD; '?' is allowed)
//       fi
//    elif first character is "="
//       name := first char after '=' until eol or '('
//    elif line starts with "%Elo"
//    elif line starts with "%Bio"
//    fi
// end
void
Player::parseSpellcheckFile(mstl::istream& stream)
{
	enum Section { Invalid, Player, Site, Event, Round };

	DEBUG(unsigned countPlayers	= 0);
	DEBUG(unsigned countViafIds	= 0);
	DEBUG(unsigned countPndIds		= 0);

	DEBUG(::printf("### Parse Spellcheck File #####################\n"));

	::playerList.reserve(200000);

	Section			section = Invalid;
	db::Player*		player = 0;
	mstl::string	line;
	mstl::string	titles;
	mstl::string	federations;
	mstl::string	elo;
	Date				birthDate;
	Date				deathDate;

	::exclude.clear();

	while (stream.getline(line))
	{
		char const* s = line.c_str();

		while (::isspace(*s))
			++s;

		switch (char c = *s)
		{
			case '\0':	// skip empty line
			case '#':	// skip comments
			case '>':	// skip old biography line
				break;

			case '@':
				if (line.begin() == s)
				{
					player = 0;

					switch (s[1])
					{
						case 'P': section = Player; break;	// %Player
						case 'S': section = Site; break;		// %Site
						case 'E': section = Event; break;	// %Event
						case 'R': section = Round; break;	// %Round
					}

					while (*s && !::isspace(*s))
						++s;
					while (::isspace(*s))
						++s;

					if (section == Player && *s == '"')
					{
						char const* t = ++s;

						while (*t && *t != '"')
							++t;

						::exclude.assign(s, t);

						if (exclude.find('\'') == mstl::string::npos)
							::exclude += '\'';	// forgotten character
					}
				}
				break;

			case '%':
				if (player)
				{
					switch (s[1])
					{
						case 'B':
							if (::strncmp(s, "%Bio ", 5) == 0)
							{
								char const* t = ::skipSpaces(s + 5);

								switch (t[0])
								{
									case 'F':
										if (::strncmp(t, "FIDEID ", 7) == 0)
										{
											player->setFideID(::strtoul(t + 7, nullptr, 10));
											::playerDict.insert_unique(player->fideID(), player);
										}
										break;

									case 'V':
										if (::strncmp(t, "VIAF ", 5) == 0)
										{
											player->setViafID(::strtoul(t + 5, nullptr, 10));
											DEBUG(++countViafIds);
										}
										break;

									case 'P':
										if (::strncmp(t, "PND ", 4) == 0 && ::findSpace(t + 4) - t == 13)
										{
											player->setPndID(t + 4);
											DEBUG(++countPndIds);
										}
										break;
								}
							}
//							else if (::strncmp(s, "%BornCity ", 10) == 0)
//							{
//							}
							break;

						case 'F':
							if (::strncmp(s, "%Federation ", 12) == 0)
								player->setFederation(country::fromString(::skipSpaces(s + 12)));
							break;

//						case 'P':
//							skip %Prefix
//							break;
//
//						case 'I':
//							skip %Infix
//							break;
//
//						case 'S':
//							// skip %Suffix
//							break;
//						case 'D':
//							// skip %DiedCity
//							break;
//
//						case 'E':
//							// skip %Elo
//							break;
//
//						case 'T':
//							// skip %Title
//							break;
					}
			}
			break;

			case '=':
			case '+':
				if (player)
				{
					s += 1;
					while (::isspace(*s))
						++s;
					char const* e = s;
					while (*e && *e != '#')
						++e;
					while (e > s && ::isspace(e[-1]))
						--e;
					// NOTE: we don't want entries like "A.B.,Bappi"
					if (e > s && s[1] != '.')
					{
						// NOTE: we don't want entries like
						// "Polgar, Zsuzsa GM" or "Polgar, Zsuzsa (GM)"
						if (e[-1] != ')' && !isTitle(s, e))
						{
							mstl::string name(s, e);

#ifndef USE_CONFLICT_MAP
							if (::hasTrailingCountryCode(name))
							{
								mstl::string str(name.c_str(), name.size());
								insertAlias(str, 0, player);
								::stripCountryCode(name);
							}
#endif

							if (c == '+')
								replaceName(name, 0, player);
							else
								insertAlias(name, 0, player);
						}
					}
				}
				break;

			default:
				if (section == Player)
				{
					sex::ID sex = sex::Male;
					::extractPlayerData(line, sex, titles, federations, elo, birthDate, deathDate);
					line.trim();

					if (!line.empty())
					{
						country::Code federation = ::getFederation(federations);
						country::Code nativeCountry	= ::getNativeCountry(federations);
						unsigned titleMask = ::getTitles(titles);
						unsigned region = 0;

						if (title::containsFemaleTtile(titleMask))
							sex = sex::Female;

						if (nativeCountry != country::Unknown)
							region = country::toRegion(nativeCountry);

#ifndef USE_CONFLICT_MAP
						mstl::string alias;

						if (::hasTrailingCountryCode(line))
						{
							alias.assign(line.c_str(), line.size());
							::stripCountryCode(line);
						}
#endif

						if ((player = insertPlayer(line, region, federation, sex)))
						{
							DEBUG(++countPlayers);

							int score = ::getElo(elo);

							player->setType(species::Human);

							if (nativeCountry != country::Unknown)
								player->setNativeCountry(nativeCountry);

							if (score)
							{
								player->setLatestElo(score);
								player->setHighestElo(score);
							}

							if (titleMask)
								player->setTitles(titleMask | player->titles());

							if (birthDate)
								player->setDateOfBirth(birthDate);

							if (deathDate)
								player->setDateOfDeath(deathDate);

#ifdef USE_CONFLICT_MAP
							if (::hasTrailingCountryCode(line))
							{
								mstl::string shortName(line);
								::stripCountryCode(shortName);

								ConflictMap::reference ref = ::conflictMap.find_or_insert(shortName, 0);

								if (ref == 0)
									ref = player;
								else
									ref = &::InvalidEntry;
							}
#else
							if (!alias.empty())
								insertAlias(alias, 0, player);
#endif
						}
					}
				}
				break;
		}
	}

#ifdef USE_CONFLICT_MAP

	for (::ConflictMap::const_iterator i = ::conflictMap.begin(); i != ::conflictMap.end(); ++i)
	{
		mstl::string const& shortName = i->first;

		db::Player* p = findPlayer(shortName);

		if (p != 0)
		{
			mstl::string str(shortName);
			replaceName(str, 0u, i->second);
		}
	}

	::conflictMap.clear();

#endif

	DEBUG(::printf("-----------------------------------------------------\n"));
	DEBUG(if (countViafIds) ::printf("VIAF-ID entries:     %u\n", countViafIds));
	DEBUG(if (countPndIds)  ::printf("PND-ID entries:      %u\n", countPndIds));
	DEBUG(::printf("Players total:       %u\n", ::playerLookup.used()));
	DEBUG(::printf("Aliases total:       %u\n", ::aliasDict.used()));
	DEBUG(::printf("ASCII total:         %u\n", ::asciiDict.size()));
	DEBUG(::printf("Player map entries:  %u\n", ::playerDict.used()));
	DEBUG(::printf("Player list entries: %u\n", ::playerList.size()));
	DEBUG(::printf("-----------------------------------------------------\n"));
}


// Syntax:
// --------------------------------
// ID:			  0 -  7 (  8 chars)
// Name:			 10 - 42 ( 33 chars)
// Title:		 44 - 47 (  4 chars)
// Federation:	 48 - 50 (  3 chars)
// Rating:		 53 - 57 (  4 chars)
// Games:		 59 - 62 (  3 chars)
// Born:			 64 - 57 (  4 chars)
// Flag:			 70 - 71 (0-2 chars)
void
Player::parseFideRating(mstl::istream& stream)
{
	mstl::string line;
	mstl::string name;
	mstl::string key;
	mstl::string federation;

	DEBUG(unsigned count = 0);
	DEBUG(unsigned total = 0);

	if (!stream.getline(line))
		return;

	DEBUG(::printf("### Parse Fide Rating #########################\n"));

	while (stream.getline(line))
	{
		if (line.size() >= 70)
		{
			char const* s = line.c_str();

			unsigned rating = ::isdigit(line[53]) ? ::strtoul(s + 53, nullptr, 10) : 0;

			char const* t = s + 10;
			char const* e = s + 42;

			while (*t == ',') ++t;
			while (*t == ' ') ++t;

			while (e > t && *e == ' ')
				--e;

			name.assign(t, e - t + 1);

			if (name.size() > 4 && ::islower(name[1]))
			{
				federation.assign(s + 48, 3);
				::toupper(federation);

				unsigned			fideID	= ::strtoul(s, nullptr, 10);
				country::Code	country	= country::fromString(federation);
				sex::ID			sex		= s[70] == 'w' ? sex::Female : sex::Male;
				unsigned			titles	= 0;

				for (t = s + 44; *t && *t != ' '; ++t)
				{
					if (*t == 'w')
					{
						switch (*++t)
						{
							case 'f': titles |= title::Mask_WFM; break;
							case 'g': titles |= title::Mask_WGM; break;
							case 'm': titles |= title::Mask_WIM; break;
							case 'c': titles |= title::Mask_WCM; break;
						}

						// In some cases the gender flag is missing.
						sex = sex::Female;
					}
					else
					{
						switch (*t)
						{
							case 'f': titles |= title::Mask_FM; break;
							case 'g': titles |= title::Mask_GM; break;
							case 'm': titles |= title::Mask_IM; break;
							case 'c': titles |= title::Mask_CM; break;
						}
					}
				}

				Player* const* playerEntry = ::playerDict.find(fideID);
				Player* player = 0;

				if (playerEntry)
				{
					player = *playerEntry;
/*
					if (!(	sys::utf8::Codec::matchAscii(player->name(), name)
							|| (	country::isGermanSpeakingCountry(country)
								&& sys::utf8::Codec::matchGerman(player->name(), name))))
					{
						if (!containsPlayer(name, country::Unknown, sex::Unspecified))
						{
							if (rating < m_minELO)
								continue;
							insertAlias(name, player);
						}
					}*/
				}

				if (player == 0)
				{
					if (rating < m_minELO)
						continue;

					//if (!sys::utf8::Codec::is7BitAscii(name))
					//	continue;

					standardizeNames(name);

					if (!(player = insertPlayer(name, country, sex)))
						continue;

					::playerDict.insert_unique(fideID, player);
					player->setFideID(fideID);
					player->setType(species::Human);
					DEBUG(++count);
				}
				else
				{
					if (sex != sex::Unspecified)
					{
						if (player->m_sex == sex::Unspecified)
							player->m_sex = sex;
						else if (sex != sex::ID(player->m_sex))
							DEBUG(::printf("mismatch of sex: %s (%u)\n", player->name().c_str(), fideID));
					}

					if (country != country::Unknown)
					{
						if (player->m_federation == country::Unknown)
						{
							player->m_federation = country;
							player->m_nativeCountry = country;
						}
						else if (country != player->federation() && country != player->nativeCountry())
						{
							DEBUG(::printf("mismatch of country: %s (%u)\n", player->name().c_str(), fideID));
						}
					}
				}

				DEBUG(++total);

				unsigned	year = ::isdigit(s[64]) ? ::strtoul(s + 64, nullptr, 10) : 0;

				DEBUG(if (year && player->dateOfBirth() && player->dateOfBirth().year() != year)
							::printf("birth date mismatch: %s (%u)\n", name.c_str(), fideID));

				player->setTitles(player->titles() | titles);

				if (year && (!player->dateOfBirth() || player->dateOfBirth().month() == 0))
					player->setDateOfBirth(Date(year));

				if (0 < rating && rating <= rating::Max_Value)
				{
					player->setLatestElo(rating);

					if (rating > unsigned(mstl::abs(player->highestElo())))
						player->setHighestElo(rating);
				}
			}
		}
	}

	DEBUG(::printf("-----------------------------------------------------\n"));
	DEBUG(::printf("FIDE entries:        %u (%u)\n", count, total));
	DEBUG(::printf("Players total:       %u\n", ::playerLookup.used()));
	DEBUG(::printf("Player map entries:  %u\n", ::playerDict.used()));
	DEBUG(::printf("Player list entries: %u\n", ::playerList.size()));
	DEBUG(::printf("Aliases total:       %u\n", ::aliasDict.used()));
	DEBUG(::printf("ASCII total:         %u\n", ::asciiDict.size()));
	DEBUG(::printf("-----------------------------------------------------\n"));
}


// Syntax:
// --------------------------------
// ECF ID:		  0 -  6 (7 chars)
// Fide ID:      8 - 15 (8 chars)
// Gender:      17 - 17 (1 char )
// Rating:      19 - 21 (3 chars)
// Name:        23 - end
void
Player::parseEcfRating(mstl::istream& stream)
{
	mstl::string line;
	mstl::string name;

	DEBUG(unsigned count = 0);
	DEBUG(unsigned total = 0);

	DEBUG(::printf("### Parse ECF Rating ##########################\n"));

	while (stream.getline(line))
	{
		if (line.size() >= 23 && ::isdigit(line[19]) && 'A' <= line[6] && line[6] <= 'L')
		{
			unsigned		fideID	= ::strtoul(line.c_str() + 8, nullptr, 10);
			unsigned		rating	= ::strtoul(line.c_str() + 19, nullptr, 10);
			sex::ID		sex		= sex::Unspecified;
			db::Player*	player	= 0;

			switch (line[17])
			{
				case 'M': sex = sex::Male; break;
				case 'F': sex = sex::Female; break;
			}

			if (fideID)
			{
				if (Player* const* playerEntry = ::playerDict.find(fideID))
				{
					player = *playerEntry;

					if (player->sex() == sex::Unspecified)
					{
						player->setSex(sex);
						player->setType(species::Human);
					}
				}
			}

			if (player == 0)
			{
				name.hook(line.data() + 23, line.size() - 23);
				standardizeNames(name);
				player = findPlayer(name, country::England, sex);

				if (player && fideID && player->fideID() != fideID)
					continue;
			}

			if (player == 0)
			{
				if (rating < m_minECF)
					continue;

				if (!(player = insertPlayer(name, country::England, sex)))
					continue;

				DEBUG(++count);
			}

			player->setLatestRating(rating::ECF, rating);
			player->setHighestRating(rating::ECF, rating);
			player->setEcfID(line.data());
			DEBUG(++total);
		}
	}

	DEBUG(::printf("-----------------------------------------------------\n"));
	DEBUG(::printf("ECF entried:         %u (%u)\n", count, total));
	DEBUG(::printf("Players total:       %u\n", ::playerLookup.used()));
	DEBUG(::printf("Player map entries:  %u\n", ::playerDict.used()));
	DEBUG(::printf("Player list entries: %u\n", ::playerList.size()));
	DEBUG(::printf("-----------------------------------------------------\n"));
}


// Syntax:
// --------------------------------
// ZPS:		     0 -  4 (5 chars)
// MglNr:        6 -  9 (4 chars)
// Fide ID:     11 - 18 (8 chars)
// Sex:         20 - 20 (1 char )
// Birth Year:  22 - 25 (4 chars)
// Rating:      27 - 30 (4 chars)
// Name:        32 - end
void
Player::parseDwzRating(mstl::istream& stream)
{
	mstl::string line;
	mstl::string name;
	mstl::string ascii;
	mstl::string latin1;

	//sys::utf8::Codec codec(sys::utf8::Codec::latin1());

	DEBUG(unsigned count = 0);
	DEBUG(unsigned total = 0);
	DEBUG(::printf("### Parse DWZ Rating ##########################\n"));

	while (stream.getline(line))
	{
		if (	line.size() >= 32
			&& ::isdigit(line[27])
			&& (('0' <= *line && *line <= '9') || ('A' <= *line && *line <= 'L')))
		{
			unsigned		fideID	= ::strtoul(line.c_str() + 11, nullptr, 10);
			sex::ID		sex		= sex::Unspecified;
			db::Player*	player	= 0;

			if (fideID)
			{
				Player* const* playerEntry = ::playerDict.find(fideID);

				if (playerEntry)
					player = *playerEntry;
			}

			unsigned rating = ::strtoul(line.c_str() + 27, nullptr, 10);

			if (player == 0)
			{
				if (rating < m_minDWZ)
					continue;

				latin1.hook(line.data() + 32, line.size() - 32);
				//codec.toUtf8(latin1, name);

				switch (line[20])
				{
					case 'M': sex = sex::Male; break;
					case 'W': sex = sex::Female; break;
				}

				if (!(player = insertPlayer(name, 1, country::Germany, sex)))
					continue;

				if (fideID && player->fideID() != fideID)
				{
					if (player->fideID() == 0)
					{
						player->setFideID(fideID);
					}
					else if (fideID)
					{
//						DEBUG(::printf("player '%s': mismatch of FIDE Id (%u - %u)\n",
//											name.c_str(),
//											player->fideID(),
//											fideID));
						continue;
					}
				}

				if (player->federation() == country::Unknown)
					player->setFederation(country::Germany);

				/*if (!sys::utf8::Codec::is7BitAscii(name))
				{
					sys::utf8::Codec::mapFromGerman(name, ascii);
					::alloc(::asciiDict[player], ascii);
				}*/

				DEBUG(++count);
			}

			unsigned yearOfBirth = ::isdigit(line[22]) ? ::strtoul(line.c_str() + 22, nullptr, 10) : 0;

			if (yearOfBirth && player->dateOfBirth().year() == 0)
				player->setDateOfBirth(Date(yearOfBirth));

			if (player->sex() == sex::Unspecified)
			{
				player->setSex(sex);
				player->setType(species::Human);
			}

			player->setLatestRating(rating::DWZ, rating);
			player->setHighestRating(rating::DWZ, rating);
			player->setDsbID(line, line.c_str() + 6);
			DEBUG(++total);
		}
	}

	DEBUG(::printf("-----------------------------------------------------\n"));
	DEBUG(::printf("DWZ entries:         %u (%u)\n", count, total));
	DEBUG(::printf("Players total:       %u\n", ::playerLookup.used()));
	DEBUG(::printf("Player map entries:  %u\n", ::playerDict.used()));
	DEBUG(::printf("Player list entries: %u\n", ::playerList.size()));
	DEBUG(::printf("ASCII total:         %u\n", ::asciiDict.size()));
	DEBUG(::printf("-----------------------------------------------------\n"));
}


// Syntax:
// --------------------------------
// ICCF ID:	     0 -  5 (6 chars)
// Federation:   7 -  9 (3 chars)
// Title:       11 - 13 (3 chars) +1
// Rating:      15 - 18 (4 chars)
// Sex:         20 - 20 (1 char )
// Name:        22 - end
void
Player::parseIccfRating(mstl::istream& stream)
{
	mstl::string line;
	mstl::string name;

	//sys::utf8::Codec codec(sys::utf8::Codec::latin1());

	DEBUG(unsigned count = 0);
	DEBUG(unsigned total = 0);
	DEBUG(::printf("### Parse ICCF Rating #########################\n"));

	while (stream.getline(line))
	{
		if (line.size() >= 22 && ::isdigit(line[15]))
		{
			unsigned rating = ::strtoul(line.c_str() + 15, nullptr, 10);

			if (rating >= m_minICCF)
			{
				country::Code	federation	= country::fromString(line.c_str() + 7);
				sex::ID			sex			= sex::Unspecified;

				switch (line[20])
				{
					case 'M': sex = sex::Male; break;
					case 'F': sex = sex::Female; break;
				}

				name.hook(line.data() + 22, line.size() - 22);

				if (db::Player* player = insertPlayer(name, 0, federation, sex))
				{
					unsigned id = ::strtoul(line.c_str(), nullptr, 10);

					DEBUG(if (player->latestRating() == 0) ++count);
					DEBUG(++total);

					switch (line[11])
					{
						case 'I': player->addTitle(line[12] == 'M' ? title::CIM : title::CILM); break;
						case 'L': player->addTitle(title::CLGM); break;
						case 'S': player->addTitle(title::CSIM); break;
						case 'G': player->addTitle(title::CGM); break;
					}

					if (player->sex() == sex::Unspecified)
					{
						player->setSex(sex);
						player->setType(species::Human);
					}

					player->setLatestRating(rating::ICCF, rating);
					player->setHighestRating(rating::ICCF, rating);
					player->setIccfID(id);

					if (player->federation() == country::Unknown)
						player->setFederation(federation);
				}
			}
		}
	}

	DEBUG(::printf("-----------------------------------------------------\n"));
	DEBUG(::printf("ICCF entries:        %u (%u)\n", count, total));
	DEBUG(::printf("Players total:       %u\n", ::playerLookup.used()));
	DEBUG(::printf("Player map entries:  %u\n", ::playerDict.used()));
	DEBUG(::printf("Player list entries: %u\n", ::playerList.size()));
	DEBUG(::printf("-----------------------------------------------------\n"));
}


void
Player::parseIpsRatingList(mstl::istream& stream)
{
	//sys::utf8::Codec codec(sys::utf8::Codec::latin1());

	mstl::string line;
	mstl::string name;
	mstl::string latin1;

	DEBUG(unsigned count = 0);
	DEBUG(unsigned total = 0);
	DEBUG(::printf("### Parse IPS Rating ##########################\n"));

	while (stream.getline(line))
	{
		if (!line.empty() && line.front() != '#')
		{
			mstl::string::size_type n = line.find('\t');

			if (n != mstl::string::npos)
			{
				line[n] = '\0';
				//M_ASSERT(n < line.size());
				latin1.hook(line.data(), n);
				//codec.toUtf8(latin1, name);

				Player* player = findPlayer(name);

				if (player == 0)
				{
					if (!containsPlayer(name, country::Unknown, sex::Unspecified))
					{
						player = insertPlayer(name, 1);
						DEBUG(++count);
					}
				}

				DEBUG(++total);

				if (player)
				{
					uint16_t	value = ::strtoul(::skipSpaces(line.c_str() + n + 1), nullptr, 10);

					player->setLatestRating(rating::IPS, value);
					player->setHighestRating(rating::IPS, value);
					player->setType(species::Human);
				}

				name = "";
			}
		}
	}

	DEBUG(::printf("-----------------------------------------------------\n"));
	DEBUG(::printf("IPS rating entries:  %u (%u)\n", count, total));
	DEBUG(::printf("Players total:       %u\n", ::playerLookup.used()));
	DEBUG(::printf("Player map entries:  %u\n", ::playerDict.used()));
	DEBUG(::printf("Player list entries: %u\n", ::playerList.size()));
	DEBUG(::printf("-----------------------------------------------------\n"));
}


void
Player::parseChessgamesDotComLinks(mstl::istream& stream)
{
	DEBUG(unsigned countLinks = 0);
	DEBUG(::printf("### Parse chessgames.com Links ################\n"));

	mstl::string line;

	while (stream.getline(line))
	{
		mstl::string::size_type n = line.find('|');

		if (n != mstl::string::npos)
		{
			char* t = line.data() + n;
			*t++ = '\0';

			mstl::string name;
			name.hook(line.data(), t - line.c_str() - 1);

			Player const* player = findPlayer(name);

			if (player)
			{
				unsigned len = line.size() - (t - line.c_str());
				char* s = ::charAllocator.alloc(len + 1);
				::memcpy(s, t, len + 1);
				::chessgamesDict[player] = s;
				DEBUG(++countLinks);
			}
			else
			{
				DEBUG(::printf("chessgames-links: cannot find '%s'\n", name.c_str()));
			}
		}
	}

	DEBUG(::printf("-----------------------------------------------------\n"));
	DEBUG(::printf("Chessgames.com links %u\n", countLinks));
	DEBUG(::printf("-----------------------------------------------------\n"));
}


void
Player::parseWikipediaLinks(mstl::istream& stream)
{
	DEBUG(unsigned countLinks = 0);
	DEBUG(char const* lang = "??");
	DEBUG(::printf("### Parse Wikipedia Links #####################\n"));

	mstl::string line;

	Lookup* lookup = 0;

	while (lookup == 0 && stream.getline(line))
	{
		if (line.front() == '@')
		{
			if (::strncmp(line, "@lang=", 6) == 0)
			{
				line.trim();

				if (line.size() == 8)
				{
					char* s = ::charAllocator.alloc(3);
					::memcpy(s, line.c_str() + 6, 3);
					DEBUG(lang = s);

					LangMap::const_pointer i = ::langMap.find(s);

					if (i == 0)
						::langMap.insert_unique(s, lookup = new Lookup(750));
					else
						lookup = *i;
				}
				else
				{
					fprintf(stderr, "warning: invalid language specifier '%s'\n", line.c_str() + 6);
					return;
				}
			}
		}
	}

	if (lookup == 0)
	{
		fprintf(stderr, "warning: cannot find valid language specification\n");
		return;
	}

	while (stream.getline(line))
	{
		mstl::string::size_type n = line.find('|');

		if (n != mstl::string::npos)
		{
			char* t = line.data() + n;
			*t++ = '\0';

			mstl::string name;
			name.hook(line.data(), t - line.c_str() - 1);

			Player const* player = findPlayer(name);

			if (player)
			{
				unsigned len = line.size() - (t - line.c_str());
				char* s = ::charAllocator.alloc(len + 1);
				::memcpy(s, t, len + 1);
				(*lookup)[player].hook(s, len);
				DEBUG(++countLinks);
			}
			else
			{
				DEBUG(::printf("wiki-links(%s): cannot find '%s'\n", lang, name.c_str()));
			}
		}
	}

	DEBUG(::printf("-----------------------------------------------------\n"));
	DEBUG(::printf("Wikipedia links:     %u (%s)\n", countLinks, lang));
	DEBUG(::printf("-----------------------------------------------------\n"));
}


void
Player::parseComputerList(mstl::istream& stream)
{
	DEBUG(unsigned countEngines = 0);
	DEBUG(::printf("### Parse Computer List #######################\n"));

	mstl::string		line;
	mstl::string		name;
	mstl::string		str;
	Player*				player	= 0;
	//sys::utf8::Codec	codec(sys::utf8::Codec::latin1());

	while (stream.getline(line))
	{
		if (!line.empty() && line[0] != '#')
		{
			char const* s = line.c_str();

			bool isAlias	= *s == '=';
			bool isUnique	= *s == '*';

			++s;
			while (::isspace(*s))
				++s;

			if (*s)
			{
				if (isAlias)
				{
					if (player)
					{
						unsigned skip = s - line.c_str();

						line.rtrim();
						str.hook(const_cast<char*>(s), line.size() - skip);
						//codec.toUtf8(str, name);
						insertAlias(name, 0, player);
					}
				}
				else
				{
					char const* t = s + 1;

					while (*t && *t != '\t')
						++t;

					country::Code federation = country::Unknown;

					bool winboard		= false;
					bool uci				= false;
					bool chess960		= false;
					bool shuffleChess	= false;

					*const_cast<char*>(t) = '\0';
					str.hook(const_cast<char*>(s), t++ - s);
					s = t;
					while (::isspace(*s))
						++s;

					unsigned elo = ::strtoul(s, const_cast<char**>(&s), 10);

					while (*s == '-')
						++s;
					while (::isspace(*s))
						++s;

					unsigned ccrl = ::strtoul(s, const_cast<char**>(&s), 10);

					while (*s == '-')
						++s;
					while (::isspace(*s))
						++s;

					if (*s)
					{
						t = s;

						while (::isalpha(*t))
							++t;

						if (t - s == 3)
							federation = country::fromString(s);

						s = t;

						while (*s == '-')
							++s;
						while (::isspace(*s))
							++s;

						while (*s != '\0')
						{
							switch (::toupper(*s))
							{
								case 'W': winboard = true; break;
								case 'U': uci = true; break;
								case 'F': chess960 = true; break;
								case 'S': shuffleChess = chess960 = true; break;
							}

							while (::isalpha(*s))
								++s;
							if (*s != '\0')
								++s;
						}
					}

					//codec.toUtf8(str, name);

					if (Player* player = insertPlayer(name, 1, federation))
					{
						DEBUG(++countEngines);

						player->setChess960Flag(chess960);
						player->setShuffleChessFlag(shuffleChess);
						player->setWinboardProtocol(winboard);
						player->setUciProtocol(uci);
						player->setType(species::Program);
						player->setLatestElo(elo);
						player->setHighestElo(elo);
						player->setLatestRating(rating::Rating, ccrl);
						player->setHighestRating(rating::Rating, ccrl);
						player->setFederation(federation);
						player->setUnique(isUnique);
					}
				}
			}
		}
	}

	DEBUG(::printf("-----------------------------------------------------\n"));
	DEBUG(::printf("Engines:             %u\n", countEngines));
	DEBUG(::printf("Players total:       %u\n", ::playerLookup.used()));
	DEBUG(::printf("Player list entries: %u\n", ::playerList.size()));
	DEBUG(::printf("Aliases total:       %u\n", ::aliasDict.used()));
	DEBUG(::printf("ASCII total:         %u\n", ::asciiDict.size()));
	DEBUG(::printf("-----------------------------------------------------\n"));
}



void
Player::enumerate(PlayerCallback& cb)
{
	for (PlayerLookup::const_iterator i = playerLookup.begin(); i != playerLookup.end(); ++i)
	{
		for (unsigned k = 0; k < i->second.size(); ++k)
			cb.entry(*i->second[k]);
	}
}


void
Player::dump()
{
	for (PlayerLookup::const_iterator i = playerLookup.begin(); i != playerLookup.end(); ++i)
	{
		for (unsigned k = 0; k < i->second.size(); ++k)
		{
			printf(	"%s (%s - %s)\n",
						i->second[k]->name().c_str(),
						country::toString(i->second[k]->federation()),
						sex::toString(i->second[k]->sex()).c_str());
		}
	}
}

// vi:set ts=3 sw=3:
