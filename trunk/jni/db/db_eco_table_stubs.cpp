// ======================================================================
// $RCSfile: db_eco_table.cpp,v $
// $Revision$
// $Date$
// $Author: gregor $
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

#include "db_eco_table.h"

db::EcoTable db::EcoTable::m_specimen;

using namespace db;


EcoTable::EcoTable()
	:m_branchBuffer(0)
	,m_nodeBuffer(0)
	,m_nameBuffer(0)
	,m_moveBuffer(0)
	,m_root(0)
{
}


EcoTable::~EcoTable() {}


void
EcoTable::getSuccessors(uint64_t, Successors&) const
{
	M_ASSERT(!"cannot execute stubs");
}


int
EcoTable::Successors::find(uint16_t) const
{
	M_ASSERT(!"cannot execute stubs");
	return -1;
}



EcoTable::Entry const&
EcoTable::getEntry(Eco code) const
{
	M_ASSERT(!"cannot execute stubs");
	static Entry dummy;
	return dummy;
}


uint8_t EcoTable::getStoredLine(Eco, Eco) const { return 0; }
Eco EcoTable::lookup(Line const&, unsigned*, Successors*, EcoSet*) const { return Eco(); }

// vi:set ts=3 sw=3:
