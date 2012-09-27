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

#include "db_database_codec.h"

#include "m_assert.h"

namespace db {

inline bool Database::isOpen() const							{ return m_codec; }
inline bool Database::isReadOnly() const						{ return m_readOnly; }
inline bool Database::isWriteable() const						{ return m_writeable; }
inline bool Database::shouldUpgrade() const					{ return m_codec->isExpired(); }
inline bool Database::isMemoryOnly() const					{ return m_memoryOnly; }
inline bool Database::encodingIsBroken() const				{ return !m_encodingOk; }
inline bool Database::encodingFailed() const					{ return m_encodingFailed; }
inline bool Database::usingAsyncReader() const				{ return m_usingAsyncReader; }
inline unsigned Database::id() const							{ return m_id; }
inline unsigned Database::countGames() const					{ return m_gameInfoList.size(); }
inline unsigned Database::countPlayers() const				{ return m_namebases(Namebase::Player).used(); }
inline unsigned Database::countEvents() const				{ return m_namebases(Namebase::Event).used(); }
inline unsigned Database::countSites() const					{ return m_namebases(Namebase::Site).used(); }
inline mstl::string const& Database::name() const			{ return m_name; }
inline mstl::string const& Database::description() const	{ return m_description; }
inline type::ID Database::type() const							{ return m_type; }
inline Statistic const& Database::statistic() const		{ return m_statistic; }
inline uint64_t Database::lastChange() const					{ return m_lastChange; }
inline DatabaseContent const& Database::content() const	{ return *this; }
inline TreeCache const& Database::treeCache() const		{ return m_treeCache; }
inline TreeCache& Database::treeCache()						{ return m_treeCache; }
inline Namebases& Database::namebases()						{ return m_namebases; }
inline Time Database::created() const							{ return m_created; }
inline uint32_t Database::creationTimestamp() const		{ return m_created; }


inline
DatabaseCodec const&
Database::codec() const
{
	//M_REQUIRE(isOpen());
	return *m_codec;
}


inline
DatabaseCodec&
Database::codec()
{
	//M_REQUIRE(isOpen());
	return *m_codec;
}


inline
NamebasePlayer const&
Database::player(unsigned index) const
{
	//M_REQUIRE(index < countPlayers());
	return *m_namebases(Namebase::Player).player(index);
}


inline
mstl::string const&
Database::encoding() const
{
	//M_REQUIRE(isOpen());
	return m_encoding;
}


inline
Time
Database::modified() const
{
	//M_REQUIRE(isOpen());
	return m_codec->modified(m_rootname);
}


inline
unsigned
Database::maxDescriptionLength() const
{
	//M_REQUIRE(isOpen());
	return m_codec->maxDescriptionLength();
}


inline
void
Database::setReadOnly(bool flag)
{
	//M_REQUIRE(isOpen());
	//M_REQUIRE(flag || isWriteable());

	m_readOnly = flag;
}


inline
Database::~Database() throw()
{
	delete m_codec;
}


inline
format::Type
Database::format() const
{
	return m_codec->format();
}


inline
void
Database::setEncodingFailed(bool flag)
{
	if ((m_encodingFailed = flag))
		m_encodingOk = false;
}


inline
mstl::string const&
Database::extension() const
{
	//M_REQUIRE(isOpen());
	return m_codec->extension();
}


inline
GameInfo const&
Database::gameInfo(unsigned index) const
{
	//M_REQUIRE(isOpen());
	//M_REQUIRE(index < countGames());

	return *m_gameInfoList[index];
}


inline
GameInfo&
Database::gameInfo(unsigned index)
{
	//M_REQUIRE(isOpen());
	//M_REQUIRE(index < countGames());

	return *m_gameInfoList[index];
}


inline
NamebaseEntry const*
Database::insertPlayer(mstl::string const& name)
{
	//M_REQUIRE(isOpen());
	//M_ASSERT(!isReadOnly());

	return m_namebases(Namebase::Player).insert(name, m_codec->maxPlayerCount());
}


inline
NamebaseEntry const*
Database::insertEvent(mstl::string const& name)
{
	//M_REQUIRE(isOpen());
	//M_ASSERT(!isReadOnly());

	return m_namebases(Namebase::Event).insert(name, m_codec->maxEventCount());
}


inline
NamebaseEntry const*
Database::insertSite(mstl::string const& name)
{
	//M_REQUIRE(isOpen());
	//M_ASSERT(!isReadOnly());

	return m_namebases(Namebase::Site).insert(name, m_codec->maxSiteCount());
}


inline
NamebaseEntry const*
Database::insertAnnotator(mstl::string const& name)
{
	//M_REQUIRE(isOpen());
	//M_ASSERT(!isReadOnly());

	return m_namebases(Namebase::Annotator).insert(name, m_codec->maxAnnotatorCount());
}


inline
Move
Database::findExactPositionAsync(unsigned index, Board const& position, bool skipVariations) const
{
	//M_REQUIRE(isOpen());
	//M_REQUIRE(index < countGames());
	//M_REQUIRE(usingAsyncReader());

	return m_codec->findExactPositionAsync(*m_gameInfoList[index], position, skipVariations);
}


inline
load::State
Database::loadGame(unsigned index, Game& game)
{
	return loadGame(index, game, 0);
}


inline
load::State
Database::loadGame(unsigned index, Game& game, mstl::string& encoding)
{
	return loadGame(index, game, &encoding);
}

} // namespace db

// vi:set ts=3 sw=3:
