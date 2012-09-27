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

#ifndef _db_log_included
#define _db_log_included

#include "db_common.h"

namespace db {

struct Log
{
	enum Warning { InvalidRoundTag, MaximalWarningCountExceeded };

	virtual ~Log() throw() = 0;

	virtual bool error(save::State code, unsigned gameNumber) = 0;
	virtual void warning(Warning code, unsigned gameNumber) = 0;
};

} // namespace db

#endif // _db_log_included

// vi:set ts=3 sw=3:
