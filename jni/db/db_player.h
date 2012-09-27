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

#ifndef db_player_included
#define db_player_included

#include "db_common.h"
#include "db_date.h"

#include "m_string.h"
#include "m_list.h"
#include "m_vector.h"
#include "m_pair.h"

namespace mstl { class istream; }
namespace TeXt { class Receptacle; }

namespace db {

class Namebase;
class NamebasePlayer;
class PlayerStats;

class Player
{
public:

	class PlayerCallback
	{
	public:

		virtual ~PlayerCallback();
		virtual void entry(Player const& player) = 0;
	};

	typedef mstl::pair<mstl::string,mstl::string> Assoc;
	typedef mstl::list<Assoc> AssocList;
	typedef mstl::vector<mstl::string> StringList;
	typedef mstl::vector<Player const*> Matches;

	/// Construct an empty player.
	Player();

	/// Returns whether player is an engine.
	bool isEngine() const;
	/// Returns whether player name is an unique name (not an ordinary last name).
	bool isUnique() const;
	/// Returns whether player name is an ordinary last name.
	bool isNotUnique() const;
	/// Returns whether UCI protocol is supported.
	bool supportsUciProtocol() const;
	/// Returns whether Winboard protocol is supported.
	bool supportsWinboardProtocol() const;
	/// Return whether chess 960 is supported.
	bool supportsChess960() const;
	/// Return whether shuffle chess is supported.
	bool supportsShuffleChess() const;

	/// Returns players name.
	mstl::string const& name() const;
	/// Return sex of player.
	sex::ID sex() const;
	/// Return type of player.
	species::ID type() const;
	/// Returns players date of birth, if known.
	Date dateOfBirth() const;
	/// Return players date of death, if known.
	Date dateOfDeath() const;
	/// Return players year of birth, if known.
	uint16_t birthYear() const;
	/// Returns players highest overall Elo rating achieved (< 0 if estimated).
	int16_t highestElo() const;
	/// Returns players latest Elo rating achieved  (< 0 if estimated).
	int16_t latestElo() const;
	/// Returns players highest overall rating achieved (< 0 if estimated).
	int16_t highestRating() const;
	/// Returns players latest rating achieved  (< 0 if estimated).
	int16_t latestRating() const;
	/// Returns players highest overall rating achieved (< 0 if estimated).
	int16_t highestRating(rating::Type type) const;
	/// Returns players latest rating achieved  (< 0 if estimated).
	int16_t latestRating(rating::Type type) const;
	/// Returns (best) rating type available (rating::Last if none available)
	rating::Type ratingType() const;
	/// Returns titles of player.
	unsigned titles() const;
	/// Returns federation of player.
	country::Code federation() const;
	/// Returns native country of player.
	country::Code nativeCountry() const;
	/// Return FIDE player ID.
	unsigned fideID() const;
	/// Return DSB player ID.
	mstl::string dsbID() const;
	/// Return ECF player ID.
	mstl::string ecfID() const;
	/// Return ICCF player ID.
	unsigned iccfID() const;
	/// Return VIAF (Virtual International Authority File) ID.
	unsigned viafID() const;
	/// Return PND (Personennamendatei) ID.
	mstl::string pndID() const;
	/// Return chessgames.com identifier
	mstl::string const& chessgamesID() const;
	/// Return wikipedia URL (if existing)
	unsigned wikipediaLinks(AssocList& result) const;
	/// Return list of aliases.
	StringList const& aliases() const;
	/// Returns asciified players name.
	mstl::string const& asciiName() const;
	/// Return region code of player name.
	unsigned region() const;

	void setSex(sex::ID id);
	void setType(species::ID id);
	void setDateOfBirth(Date const& date);
	void setDateOfDeath(Date const& date);
	void setHighestElo(int16_t rating);
	void setLatestElo(int16_t rating);
	void setHighestRating(rating::Type type, int16_t rating);
	void setLatestRating(rating::Type type, int16_t rating);
	void setTitles(unsigned titles);
	void addTitle(title::ID title);
	void setFederation(country::Code federation);
	void setNativeCountry(country::Code country);
	void setFideID(unsigned id);
	void setEcfID(char* id);
	void setDsbID(char const* zps, char const* nr);
	void setIccfID(unsigned id);
	void setViafID(unsigned id);
	void setPndID(char const* id);
	void setChess960Flag(bool flag);
	void setShuffleChessFlag(bool flag);
	void setWinboardProtocol(bool flag);
	void setUciProtocol(bool flag);
	void setUnique(bool flag);

	static bool isNormalized(mstl::string const& name);
	static bool containsPlayer(mstl::string const& name, country::Code country, sex::ID sex);

	static Player* findPlayer(uint32_t fideID);
	static Player* findPlayer(	mstl::string const& name,
										country::Code federation = country::Unknown,
										sex::ID sex = sex::Unspecified);
	static Player* findEngine(mstl::string const& name);
	static mstl::string& normalize(mstl::string& name);
	static mstl::string& normalize(mstl::string const& name, mstl::string& result);
	static void standardizeNames(mstl::string& name);
	static unsigned findMatches(mstl::string const& name, Matches& result, unsigned maxMatches);
	static unsigned countPlayers();
	static Player* insertPlayer(uint32_t fideID, mstl::string const& name);
	static void enumerate(PlayerCallback& cb);

	static void parseSpellcheckFile(mstl::istream& stream);
	static void parseFideRating(mstl::istream& stream);
	static void parseEcfRating(mstl::istream& stream);
	static void parseDwzRating(mstl::istream& stream);
	static void parseIpsRatingList(mstl::istream& stream);
	static void parseIccfRating(mstl::istream& stream);
	static void parseWikipediaLinks(mstl::istream& stream);
	static void parseChessgamesDotComLinks(mstl::istream& stream);
	static void parseComputerList(mstl::istream& stream);
	static void loadDone();

	static void emitPlayerCard(TeXt::Receptacle& receptacle,
										NamebasePlayer const& player,
										PlayerStats const& stats);

	static void dump();

private:

	Player(Player const&);
	Player& operator=(Player const&);

	friend class Namebase;

	static Player* newPlayer(	mstl::string const& name,
										unsigned region,
										mstl::string const& ascii,
										country::Code federation = country::Unknown,
										sex::ID sex = sex::Unspecified);
	static bool newAlias(mstl::string const& name, mstl::string const& ascii, Player* player);
	static bool replaceName(mstl::string const& name, mstl::string const& ascii, Player* player);

	static Player* insertPlayer(	mstl::string& name,
											country::Code federation = country::Unknown,
											sex::ID sex = sex::Unspecified);
	static Player* insertPlayer(	mstl::string& name,
											unsigned region,
											country::Code federation = country::Unknown,
											sex::ID sex = sex::Unspecified);
	static bool insertAlias(mstl::string& name, Player* player);
	static bool insertAlias(mstl::string& name, unsigned region, Player* player);
	static bool replaceName(mstl::string& name, unsigned region, Player* player);

	typedef int16_t Ratings[rating::Last];

	mstl::string m_name;

	Ratings m_highestRating;
	Ratings m_latestRating;

	uint32_t m_fideID			:26;	// 100.000..40.000.000
	uint32_t m_ratingType	:4;
	uint32_t m_sex				:2;

	uint32_t m_iccfID			:20;	// 10.000..1.000.000
	uint32_t m_dsbMglNr		:11;	// 0..2000
	uint32_t m_chess960		:1;

	uint32_t m_uscfID			:25;	// 10.000.000...30.000.000
	uint32_t m_birthDay		:5;
	uint32_t m_winboard		:1;
	uint32_t m_uci				:1;

	uint32_t m_ecfPrefix		:19;	// 6 digits
	uint32_t m_birthYear		:11;
	uint32_t m_species		:2;

	uint32_t m_titles			:16;
	uint32_t m_deathYear		:11;
	uint32_t m_deathDay		:5;

	uint32_t m_zpsSuffix		:14;	// 4 digits
	uint32_t m_federation	:9;
	uint32_t m_birthMonth	:4;
	uint32_t m_deathMonth	:4;
	uint32_t m_notUnique		:1;

	uint32_t m_nativeCountry:9;
	uint32_t m_zpsPrefix		:5;	// 1. digit (0-9, A-L)
	uint32_t m_ecfSuffix		:4;	// A-L
	uint32_t m_region			:3;
	uint32_t m_shuffle		:1;
	uint32_t m_unused			:10;

	static unsigned m_minELO;
	static unsigned m_minDWZ;
	static unsigned m_minECF;
	static unsigned m_minICCF;
};

} // namespace db

#include "db_player.ipp"

#endif // _db_player_included

// vi:set ts=3 sw=3:
