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

#ifndef _db_database_content_included
#define _db_database_content_included

#include "db_namebases.h"
#include "db_game_info.h"
#include "db_statistic.h"
#include "db_time.h"
#include "db_common.h"

#include "m_vector.h"
#include "m_string.h"
#include "m_chunk_allocator.h"
#include "m_utility.h"

namespace db {

class DatabaseContent : public mstl::noncopyable
{
public:

	typedef type::ID Type;
	typedef mstl::chunk_allocator<GameInfo> Allocator;

	DatabaseContent(mstl::string const& encoding, Type type = type::Unspecific);
	DatabaseContent(DatabaseContent const& content);
	virtual ~DatabaseContent() throw();

	typedef mstl::vector<GameInfo*> GameInfoList;

	unsigned size() const;

	Namebase& namebase(Namebase::Type type);
	Namebase const& namebase(Namebase::Type type) const;

	GameInfoList	m_gameInfoList;
	Namebases		m_namebases;
	Type				m_type;
	uint32_t			m_created;
	bool				m_readOnly;
	bool				m_writeable;
	bool				m_memoryOnly;
	mstl::string	m_description;
	mstl::string	m_encoding;
	Allocator 		m_allocator;
	Statistic		m_statistic;
};

} // namespace db

#include "db_database_content.ipp"

#endif // _db_database_content_includedy

// vi:set ts=3 sw=3:
