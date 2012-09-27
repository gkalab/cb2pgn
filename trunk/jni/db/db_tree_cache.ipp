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

namespace db {

inline unsigned TreeCache::size()			{ return CacheSize; }
inline unsigned TreeCache::used() const	{ return m_inUse; }


inline
Tree*
TreeCache::lookup(Board const& position, tree::Mode mode, rating::Type ratingType) const
{
	return lookup(position.hash(), position.exactPosition(), mode, ratingType);
}


inline
bool
TreeCache::isCached(	uint64_t hash,
							Position const& position,
							tree::Mode mode,
							rating::Type ratingType) const
{
	return lookup(hash, position, mode, ratingType) != 0;
}


inline
bool
TreeCache::isCached(Board const& position, tree::Mode mode, rating::Type ratingType) const
{
	return isCached(position.hash(), position.exactPosition(), mode, ratingType);
}

} // namespace db

// vi:set ts=3 sw=3:
