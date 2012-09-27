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

#include "db_namebase_entry.h"
#include "db_player.h"
#include "db_site.h"

#include <strings.h>

using namespace db;


NamebaseSite* NamebaseEvent::m_emptySite	= new NamebaseSite();
NamebaseEntry* NamebaseEntry::m_emptyEntry	= new NamebaseEntry();


NamebaseEntry::NamebaseEntry(mstl::string const& name) :m_name(name) {}
NamebaseEntry::~NamebaseEntry() throw() {}

NamebaseSite::NamebaseSite() : m_value(country::Unknown), m_site(0) {}
NamebaseEvent::NamebaseEvent() {}


country::Code
NamebaseSite::findCountry() const
{
	if (m_value != country::Unknown)
		return m_value;

	return m_site && m_site->countCountries() == 1 ? m_site->country(0) : country::Unknown;
}


NamebasePlayer::NamebasePlayer()
	:m_player(0)
	,m_species(species::Unspecified)
	,m_sex(sex::Unspecified)
	,m_federation(country::Unknown)
	,m_title(title::None)
	,m_fideIdFlag(0)
	,m_federationFlag(1)
	,m_titleFlag(1)
	,m_sexFlag(1)
	,m_speciesFlag(1)
	,m_ratingType(rating::Any)
	,m_unused_(0)
{
	::memset(m_rating, 0, sizeof(m_rating));
}


uint16_t
NamebasePlayer::findRating(rating::Type type) const
{
	if (type == rating::Any)
	{
		if (m_ratingType != rating::Any)
			return m_rating[m_ratingType];
	}
	else
	{
		if (m_rating[type])
			return m_rating[type];
	}

	if (m_player)
		return mstl::abs(m_player->highestRating(type));

	return 0;
}


rating::Type
NamebasePlayer::findRatingType() const
{
	if (m_ratingType != rating::Any)
		return rating::Type(m_ratingType);

	if (m_player)
		return m_player->ratingType();

	return rating::Last;
}


uint16_t
NamebasePlayer::playerHighestElo() const
{
	if (m_player)
	{
		int16_t elo = m_player->highestElo();

		if (elo)
			return mstl::abs(elo);
	}

	return m_rating[rating::Elo];
}


uint16_t
NamebasePlayer::playerLatestElo() const
{
	if (m_player)
	{
		int16_t elo = m_player->latestElo();

		if (elo)
			return mstl::abs(elo);
	}

	return m_rating[rating::Elo];
}


uint16_t
NamebasePlayer::playerHighestRating(rating::Type type) const
{
	if (m_player)
	{
		int16_t rating = m_player->highestRating(type);

		if (rating)
			return mstl::abs(rating);
	}

	if (type != rating::Any)
		return m_rating[type];

	if (m_ratingType != rating::Last)
		return m_rating[m_ratingType];

	return m_rating[rating::Elo];
}


uint16_t
NamebasePlayer::playerLatestRating(rating::Type type) const
{
	if (type == rating::Elo)
		return playerLatestElo();

	if (m_player)
	{
		int16_t rating = m_player->latestRating(type);

		if (rating)
			return mstl::abs(rating);
	}

	if (type != rating::Any)
		return m_rating[type];

	if (m_ratingType != rating::Last)
		return m_rating[m_ratingType];

	return m_rating[rating::Elo];
}


rating::Type
NamebasePlayer::playerRatingType() const
{
	if (m_player)
	{
		rating::Type type = m_player->ratingType();

		if (type != rating::Last)
			return type;
	}

	if (m_ratingType != rating::Last)
		return rating::Type(m_ratingType);

	return m_rating[rating::Elo] ? rating::Elo : rating::Last;
}


bool
NamebasePlayer::isPlayerRating(rating::Type type) const
{
	if (!m_player)
		return false;

	if (type == rating::Elo)
		return bool(m_player->highestElo());

	return bool(m_player->highestRating(type));
}


uint16_t
NamebasePlayer::playerHighestRating() const
{
	if (m_player)
	{
		int16_t rating = m_player->highestRating();

		if (rating)
			return mstl::abs(rating);
	}

	return m_ratingType == rating::Last ? 0 : m_rating[m_ratingType];
}


uint16_t
NamebasePlayer::playerLatestRating() const
{
	if (m_player)
	{
		int16_t rating = m_player->latestRating();

		if (rating)
			return mstl::abs(rating);
	}

	return m_ratingType == rating::Last ? 0 : m_rating[m_ratingType];
}

// vi:set ts=3 sw=3:
