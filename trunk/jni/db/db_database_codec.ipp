//// ======================================================================
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

#include "m_assert.h"

namespace db {

inline bool DatabaseCodec::isOpen() const								{ return m_db; }
inline bool DatabaseCodec::isReadOnly() const						{ return m_db->m_readOnly; }
inline void DatabaseCodec::setType(DatabaseContent::Type type) { m_db->m_type = type; }
inline void DatabaseCodec::setCreated(uint32_t time)				{ m_db->m_created = time; }
inline mstl::string const& DatabaseCodec::description() const	{ return m_db->m_description; }
inline DatabaseContent::Type DatabaseCodec::type() const			{ return m_db->m_type; }
inline uint32_t DatabaseCodec::created() const						{ return m_db->m_created; }
inline Namebases& DatabaseCodec::namebases()							{ return m_db->m_namebases; }

inline void DatabaseCodec::setDescription(char const* description) { m_db->m_description = description; }


inline
DatabaseCodec::GameInfoList&
DatabaseCodec::gameInfoList()
{
	return m_db->m_gameInfoList;
}


inline
GameInfo&
DatabaseCodec::gameInfo(unsigned index)
{
	//M_REQUIRE(index < gameInfoList().size());
	return *m_db->m_gameInfoList[index];
}


inline
Namebase&
DatabaseCodec::namebase(Namebase::Type type)
{
	return m_db->namebase(type);
}


inline
DatabaseCodec::CustomFlags const&
DatabaseCodec::customFlags() const
{
	//M_REQUIRE(format() == format::Scid4);
	//M_ASSERT(m_customFlags);

	return *m_customFlags;
}


inline
DatabaseCodec::CustomFlags&
DatabaseCodec::customFlags()
{
	//M_REQUIRE(format() == format::Scid4);
	//M_ASSERT(m_customFlags);

	return *m_customFlags;
}


inline
char const*
DatabaseCodec::CustomFlags::get(unsigned n) const
{
	//M_REQUIRE(n < 6);
	return m_text[n];
}


inline
void
DatabaseCodec::encodeGame(util::ByteStream& strm, GameData const& data, Signature const& signature)
{
	encodeGame(strm, data, signature, TagBits(true), true);
}

} // namespace db

// vi:set ts=3 sw=3:
