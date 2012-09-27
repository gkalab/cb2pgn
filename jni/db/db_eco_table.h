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

#ifndef _db_eco_table_included
#define _db_eco_table_included

#include "db_eco.h"
#include "db_line.h"
#include "db_home_pawns.h"
#include "db_common.h"

#include "m_map.h"
#include "m_hash.h"
#include "m_string.h"
#include "m_bitset.h"
#include "m_chunk_allocator.h"

namespace mstl { class istream; }

namespace db {

class Line;
class Board;

class EcoTable
{
public:

	enum { FileVersion = 90 };
	enum { Max_Successors = 20 };
	enum { Num_Name_Parts = 6 };

	class Node;
	class StoredLineNode;

	typedef mstl::bitset EcoSet;

	struct Successors
	{
		Successors();

		int find(uint16_t move) const;

		struct Successor
		{
			Successor();

			EcoSet	reachable;
			Eco		eco;
			uint8_t	weight;
			uint16_t	move;
		};

		Successor	list[Max_Successors];
		unsigned		length;
	};

	struct Name { mstl::string part[Num_Name_Parts]; };

	EcoTable();
	~EcoTable();

	bool isLoaded() const;
	bool isUsed(Eco code) const;

	Eco lookup(	Line const& line,
					unsigned* length = 0,
					Successors* successors = 0,
					EcoSet* reachable = 0) const;
	Name const& getName(Eco code) const;
	Line const& getLine(Eco code) const;
	uint8_t getStoredLine(Eco key, Eco opening) const;
	Eco getEco(Board const& startBoard, Line const& line, EcoSet* reachable = 0) const;
	Eco getEco(Board const& board) const;
	Eco getEco(Line const& line) const;
	void getSuccessors(uint64_t hash, Successors& successors) const;
	void getOpening(Eco code, mstl::string& opening, mstl::string& variation, mstl::string& subVar) const;
	void getOpening(	Eco code,
							mstl::string& openingLong,
							mstl::string& openingShort,
							mstl::string& variation,
							mstl::string& subVar) const;

	void print() const;
	void dump() const;

	static EcoTable const& specimen();
	static void load(mstl::istream& stream);

private:

	struct Entry
	{
		Name name;
		Line line;
	};

	typedef mstl::map<uint64_t,Node*>		Map;
	typedef mstl::hash<Eco,Entry*>			Lookup;
	typedef mstl::chunk_allocator<Entry>	Allocator;

	class Branch;
	class Loader;

	friend class Loader;
	friend class Node;

	EcoTable(EcoTable const&);
	EcoTable& operator=(EcoTable const&);

	Entry const& getEntry(Eco code) const;
	void parse(mstl::istream& strm);

	Branch*		m_branchBuffer;
	Node*			m_nodeBuffer;
	char*			m_nameBuffer;
	uint16_t*	m_moveBuffer;
	Node*			m_root;
	Lookup		m_lookup;
	Map			m_map;
	Allocator	m_allocator;

	static EcoTable m_specimen;
};

} // namespace db

namespace mstl {

template <typename T> struct is_pod;
template <typename T> struct is_movable;

template <> struct is_pod<db::EcoTable::Successors::Successor>
{
	enum { value = is_pod<db::Eco>::value };
};

template <> struct is_pod<db::EcoTable::Successors>
{
	enum { value = is_pod<db::EcoTable::Successors::Successor>::value };
};

template <> struct is_movable<db::EcoTable::Entry>
{
	enum { value = is_movable<db::Line>::value && is_movable<mstl::string>::value };
};

} // namespace mstl

#include "db_eco_table.ipp"

#endif // _db_eco_table_included

// vi:set ts=3 sw=3:
