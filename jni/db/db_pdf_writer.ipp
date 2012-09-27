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

inline
void
PdfWriter::loadPiece(Part part, mstl::string const& id)
{
	loadImage(part, m_boardStyle, m_pieceSet, id);
}


inline
void
PdfWriter::loadSquare(Part part, mstl::string const& id)
{
	loadImage(part, m_boardStyle, mstl::string::empty_string, id);
}


inline
void
PdfWriter::loadBorder(Part part, mstl::string const& id)
{
	loadImage(part, m_boardStyle, mstl::string::empty_string, id);
}

} // namespace db

// vi:set ts=3 sw=3:
