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

#ifndef _db_probe_included
#define _db_probe_included

#include "db_common.h"

namespace mstl { class string; }

namespace db {

class Board;
class Move;

class Probe
{
public:

	enum
	{
		Cache_Min_Size			= 512*1024,			// 0.5 MB
		Cache_Default_Size	= 2*1024*1024,		// 2 MB
		Cache_Max_Size			= 256*1024*1024,	// 256 MB
	};

	Probe(unsigned cacheSize = Cache_Default_Size);
	~Probe() throw();

	unsigned maxPieceNumber() const;
	unsigned cacheSize() const;

	unsigned setup(mstl::string const& egtbPath);

	bool isAvailable(Board const& board) const;

	int findScore(Board const& board) const;
	int findBest(Board const& board, Move& result) const;

	static void initialize();

private:

	int quiesce(material::SigPart white, material::SigPart black) const;

	Probe(Probe const&);
	Probe& operator=(Probe const&);

	typedef unsigned char Byte;

	Byte*		m_cache;
	unsigned	m_cacheSize;
	unsigned	m_maxPieceNumber;
};

} // namespace db

#include "db_probe.ipp"

#endif // _db_probe_included

// vi:set ts=3 sw=3:
