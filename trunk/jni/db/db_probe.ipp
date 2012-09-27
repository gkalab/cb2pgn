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

namespace db {

//! Returns the largest number of pieces in any registered tablebase,
//! including kings and pawns (e.g. kpkp tablebase has 4 pieces).
inline
unsigned
Probe::maxPieceNumber() const
{
	return m_maxPieceNumber;
}


inline
unsigned
Probe::cacheSize() const
{
	return m_cacheSize;
}

} // namespace db

// vi:set ts=3 sw=3:
