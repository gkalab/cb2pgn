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
#include "db_move.h"

#include "m_utility.h"

namespace db {
namespace bits {

template <int N>
struct Accessor
{
	static uint16_t ply(GameInfo const& info) { return info.m_ply[N]; }
	static void set(GameInfo& info, uint16_t move) { info.m_ply[N] = move; }
};

} // namespace bits

inline bool GameInfo::isDeleted() const						{ return m_gameFlags & Flag_Deleted; }
inline bool GameInfo::isDirty() const							{ return m_gameFlags & Flag_Dirty; }
inline bool GameInfo::isChanged() const						{ return m_gameFlags & Flag_Changed; }
inline bool GameInfo::hasPromotion() const					{ return m_signature.hasPromotion(); }
inline bool GameInfo::hasUnderPromotion() const				{ return m_signature.hasUnderPromotion(); }
inline bool GameInfo::containsIllegalCastlings() const	{ return m_gameFlags & Flag_Illegal_Castling; }
inline bool GameInfo::containsIllegalMoves() const			{ return m_gameFlags & Flag_Illegal_Move; }
inline bool GameInfo::hasGameRecordLength() const			{ return m_recordLength & 1; }
inline bool GameInfo::hasShuffleChessPosition() const		{ return m_positionId; }
inline bool GameInfo::hasChess960Position() const			{ return m_positionId <= 960; }
inline bool GameInfo::hasStandardPosition() const			{ return m_positionId == chess960::StandardIdn;}
inline bool GameInfo::containsEnglishLanguage() const		{ return m_pd[0].langFlag; }
inline bool GameInfo::containsOtherLanguage() const		{ return m_pd[1].langFlag; }

inline uint16_t GameInfo::idn() const							{ return m_positionId; }
inline result::ID GameInfo::result() const					{ return result::ID(m_result); }
inline uint16_t GameInfo::plyCount() const					{ return m_plyCount; }
inline event::Mode GameInfo::eventMode() const				{ return m_event->eventMode(); }
inline time::Mode GameInfo::timeMode() const					{ return m_event->timeMode(); }
inline event::Type GameInfo::eventType() const				{ return m_event->type(); }
inline country::Code GameInfo::eventCountry() const		{ return m_event->country(); }
inline Date GameInfo::eventDate() const						{ return m_event->date(); }
inline uint32_t GameInfo::flags() const						{ return m_gameFlags & (Flag_Dirty - 1); }
inline mstl::string const& GameInfo::event() const			{ return m_event->name(); }
inline uint32_t GameInfo::gameOffset() const					{ return m_gameOffset; }
inline unsigned GameInfo::round() const						{ return m_round; }
inline unsigned GameInfo::subround() const					{ return m_subround; }
inline mstl::string const& GameInfo::site() const			{ return m_event->site()->name(); }
inline uint8_t GameInfo::countVariations() const			{ return m_variationCount; }
inline uint8_t GameInfo::countComments() const				{ return m_commentCount; }
inline uint8_t GameInfo::countAnnotations() const			{ return m_annotationCount; }
inline Signature GameInfo::signature() const					{ return m_signature; }
inline unsigned GameInfo::moveCount() const 					{ return mstl::div2(m_plyCount + 1); }

inline void GameInfo::setFlags(unsigned flags)				{ m_gameFlags = flags; }

inline species::ID GameInfo::playerType(color::ID color) const	{ return m_player[color]->type(); }
inline sex::ID GameInfo::sex(color::ID color) const				{ return m_player[color]->sex(); }
inline Player const* GameInfo::player(color::ID color) const	{ return m_player[color]->player(); }
inline uint32_t GameInfo::fideID(color::ID color) const			{ return m_player[color]->fideID(); }

inline NamebasePlayer const* GameInfo::playerEntry(color::ID color) const { return m_player[color]; }
inline NamebaseEvent const* GameInfo::eventEntry() const { return m_event; }


inline
uint16_t
GameInfo::elo(color::ID color) const
{
	return m_pd[color].elo;
}


inline
uint16_t
GameInfo::rating(color::ID color) const
{
	return m_pd[color].rating;
}


inline
void
GameInfo::setIllegalMove(bool flag)
{
	if (flag)
		m_gameFlags |= Flag_Illegal_Move;
	else
		m_gameFlags &= ~Flag_Illegal_Move;
}


inline
void
GameInfo::setIllegalCastling(bool flag)
{
	if (flag)
		m_gameFlags |= Flag_Illegal_Castling;
	else
		m_gameFlags &= ~Flag_Illegal_Castling;
}


inline
Eco
GameInfo::eco() const
{
	return m_positionId == chess960::StandardIdn ? Eco::fromShort(m_eco) : Eco();
}


inline
mstl::string const&
GameInfo::annotator() const
{
	return hasGameRecordLength() ? mstl::string::empty_string : m_annotator->name();
}


inline
Eco
GameInfo::ecoKey() const
{
	//M_REQUIRE(m_positionId == chess960::StandardIdn);
	return Eco(m_ecoKey);
}


inline
uint32_t
GameInfo::gameRecordLength() const
{
	return hasGameRecordLength() ? m_recordLength >> 1 : 0;
}


inline
void
GameInfo::setGameRecordLength(unsigned length)
{
	m_recordLength = (length << 1) | 1u;
}


inline
uint16_t
GameInfo::playerElo(color::ID color) const
{
	uint16_t elo = m_player[color]->playerHighestElo();
	return elo ? elo : m_pd[color].elo;
}


inline
uint16_t
GameInfo::findElo(color::ID color) const
{
	uint16_t elo = m_pd[color].elo;
	return elo ? elo : m_player[color]->findElo();
}


inline
country::Code
GameInfo::findFederation(color::ID color) const
{
	return m_player[color]->findFederation();
}


inline
title::ID
GameInfo::findTitle(color::ID color) const
{
	return m_player[color]->findTitle();
}


inline
species::ID
GameInfo::findPlayerType(color::ID color) const
{
	return m_player[color]->findType();
}


inline
sex::ID
GameInfo::findSex(color::ID color) const
{
	return m_player[color]->findSex();
}


inline
country::Code
GameInfo::findEventCountry() const
{
	return m_event->site()->findCountry();
}


inline
int32_t
GameInfo::findFideID(color::ID color) const
{
	return m_player[color]->findFideID();
}


//inline
//uint16_t
//GameInfo::playerHighestElo(color::ID color) const
//{
//	uint16_t elo = m_player[color]->playerHighestElo();
//	return elo ? elo : uint16_t(m_pd[color].elo);
//}
//
//
//inline
//uint16_t
//GameInfo::playerLatestElo(color::ID color) const
//{
//	uint16_t elo = m_player[color]->playerLatestElo();
//	return elo ? elo : uint16_t(m_pd[color].elo);
//}


inline
bool
GameInfo::isEngine(color::ID color) const
{
	return m_player[color]->isEngine();
}


inline
rating::Type
GameInfo::ratingType(color::ID color) const
{
	rating::Type rt = rating::Type(m_pd[color].ratingType);
	return rt == rating::Elo ? rating::Any : rt;
}


inline
Date
GameInfo::date() const
{
	return Date(Date::decodeYearFrom10Bits(m_dateYear), m_dateMonth, m_dateDay);
}


inline
unsigned
GameInfo::dateYear() const
{
	return Date::decodeYearFrom10Bits(m_dateYear);
}


inline
mstl::string const&
GameInfo::playerName(color::ID color) const
{
	return m_player[color]->name();
}


inline
termination::Reason
GameInfo::terminationReason() const
{
	return termination::Reason(m_termination);
}


inline
country::Code
GameInfo::federation(color::ID color) const
{
	return m_player[color]->federation();
}


inline
title::ID
GameInfo::title(color::ID color) const
{
	return m_player[color]->title();
}


inline
mstl::string
GameInfo::position() const
{
	return idn() ? shuffle::position(idn()) : mstl::string::empty_string;
}


template <int N>
inline
uint16_t
GameInfo::ply() const
{
	return bits::Accessor<N>::ply(*this);
}


template <int N>
inline
void
GameInfo::setPly(uint16_t move)
{
	bits::Accessor<N>::set(*this, move);
}


inline
void
GameInfo::setRating(color::ID color, rating::Type ratingType, uint16_t value)
{
	//M_ASSERT(value > 0);
	//M_ASSERT(value < rating::Max_Value);

	m_pd[color].rating = value;
	m_pd[color].ratingType = ratingType;
}

} // namespace db

#undef m_pd

// vi:set ts=3 sw=3:
