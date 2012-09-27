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

#include "db_info_consumer.h"
#include "db_move_info_set.h"
#include "db_exception.h"

using namespace db;


InfoConsumer::InfoConsumer(format::Type srcFormat,
									mstl::string const& encoding,
									TagBits const& allowedTags,
									bool allowExtraTags)
	:Consumer(srcFormat, encoding, allowedTags, allowExtraTags)
{
}


void
InfoConsumer::sendComment(Comment const&)
{
	// M_ASSERT(!"shouldn't be called");
}


void
InfoConsumer::preparseComment(mstl::string& comment)
{
	m_moveInfoSet.extractFromComment(m_engines, comment);
}

// vi:set ts=3 sw=3:
