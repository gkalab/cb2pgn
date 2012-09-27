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
// Copyright: (C)2011-2012 Gregor Cramer
// ======================================================================

// ======================================================================
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
// ======================================================================

#include "db_move_info_table.h"

#include "u_byte_stream.h"

using namespace db;
using namespace util;


MoveInfo&
MoveInfoTable::push(unsigned n)
{
	if (n >= m_table.size())
		m_table.resize(n + 1);

	return m_table[n].add();
}


MoveInfo&
MoveInfoTable::push(unsigned n, MoveInfo const& info)
{
	if (n >= m_table.size())
		m_table.resize(n + 1);

	return m_table[n].add(info);
}


void
MoveInfoTable::add(unsigned n, MoveInfoSet const& moveInfoRow)
{
	if (n >= m_table.size())
		m_table.resize(n + 1);

	m_table[n] = moveInfoRow;
}


void
MoveInfoTable::set(EngineList const& engines)
{
	m_engines = engines;
}


bool
MoveInfoTable::extractFromComment(unsigned n, mstl::string& comment)
{
	MoveInfoSet moveInfo;

	if (!moveInfo.extractFromComment(m_engines, comment))
		return false;

	if (n >= m_table.size())
		m_table.resize(n + 1);

	m_table.back().swap(moveInfo);

	return true;
}


void
MoveInfoTable::print(unsigned n, mstl::string& result) const
{
	if (n < m_table.size() && !m_table[n].isEmpty())
		m_table[n].print(m_engines, result);
}


void
MoveInfoTable::decode(ByteStream& strm)
{
	MoveInfo	m;
	unsigned	index	= 0;

	m_table.clear();

	while (strm.remaining())
	{
		unsigned skip = m.decode(strm);
		m_table.resize((index += skip) + 1);
		m_table[index].add(m);
	}
}


void
MoveInfoTable::encode(ByteStream& strm) const
{
	unsigned skip = 0;

	for (unsigned i = 0; i < m_table.size(); ++i)
	{
		if (m_table[i].isEmpty())
		{
			++skip;
		}
		else
		{
			MoveInfoSet const& row = m_table[i];
			unsigned count = 0;

			for (unsigned k = 0; k < row.size(); ++k)
			{
				MoveInfo const& m = row[k];

				if (!m.isEmpty())
				{
					m.encode(strm, skip);
					skip = 0;
					++count;
				}
			}

			if (count)
				skip = 1;
		}
	}
}

// vi:set ts=3 sw=3:
