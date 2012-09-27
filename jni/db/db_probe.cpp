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

#include "db_common.h"

#include "m_utility.h"
#include "m_stdio.h"

#include <stdarg.h>

enum { Piece_Shift = 2 };

#include "db_probe.h"
#include "db_board.h"
#include "db_move_list.h"
#include "db_move.h"
#include "db_exception.h"

#include "m_string.h"

#include <string.h>

using namespace db;


static void
errorInEGTB(int n)
{

}


static void __attribute__((constructor)) initialize() { Probe::initialize(); }


Probe::Probe(unsigned cacheSize)
	:m_cache(0)
	,m_cacheSize(mstl::min(unsigned(Cache_Max_Size), mstl::max(unsigned(Cache_Min_Size), cacheSize)))
	,m_maxPieceNumber(0)
{
}


Probe::~Probe() throw()
{
	delete m_cache;
}


//! Initialises the tablebases given a directory string. All the tables
//! to be used must be in the directory; subdirectories are not
//! scanned. However, the directory string may have more than one
//! dircetory in it, separated by commas (,) or semicolons (;).
//! Returns the same value as maxPieceNumber().
unsigned
Probe::setup(mstl::string const& egtbPath)
{
    return m_maxPieceNumber;
}


int
Probe::quiesce(material::SigPart white, material::SigPart black) const
{
	return 0;
}


/// Given a material configuration, returns a boolean indicating
/// if the tablebase for that material is registered for use.
///
/// Note: there are actually TWO tablebases for any material
/// combination, one for each side to move (file suffixes .nbw.emd
/// and .nbb.emd); this function returns true if EITHER one is
/// registered (since having only one of the two is usually good
/// enough to solve the endgame).
bool
Probe::isAvailable(Board const& board) const
{
	return false;
}


/// Given a board, probes the appropriate tablebase and returns the
/// score.
///
/// The score returned is as follows, where STM is the side to move:
///
///	Not_Found	position not found
///	  3			STM mates in 3, etc.
///	  2			STM mates in 2.
///	  1			STM mates in 1.
///	  0			Draw.
///	 -1			STM is checkmated.
///	 -1			STM mated in 1.
///	 -2			STM mated in 2, etc.
int
Probe::findScore(Board const& board) const
{
	return tb::Not_Found;
}


int
Probe::findBest(Board const& board, Move& result) const
{
	return tb::Not_Found;
}


void Probe::initialize() {  }

// vi:set ts=3 sw=3:
