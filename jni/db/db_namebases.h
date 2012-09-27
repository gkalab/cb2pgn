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

#ifndef _db_namebases_included
#define _db_namebases_included

#include "db_namebase.h"

namespace db {

class Namebases
{
public:

	typedef Namebase::Type Type;

	Namebases();

	Namebase& operator()(Type type);
	Namebase const& operator()(Type type) const;

	bool isModified() const;
	bool isChanged() const;
	bool isOriginal() const;

	void update();
	void clear();
	void setReadonly(bool flag = true);
	void setModified(bool flag);

private:

	Namebase m_player;
	Namebase m_site;
	Namebase m_event;
	Namebase m_annotator;
	Namebase m_round;
};

} // namespace db

#include "db_namebases.ipp"

#endif // _db_namebases_included

// vi:set ts=3 sw=3:
