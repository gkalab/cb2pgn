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

#include "db_eco_table.h"
#include "db_board.h"
#include "db_line.h"
#include "db_move.h"
#include "db_move_list.h"
#include "db_exception.h"

#include "u_byte_stream.h"

#include "m_istream.h"
#include "m_string.h"
#include "m_vector.h"
#include "m_map.h"
#include "m_bitset.h"
#include "m_bitfield.h"
#include "m_assert.h"
#include "m_stdio.h"

#include <string.h>

#ifdef DEBUG
# undef DEBUG
# define DEBUG(stmt) stmt
#else
# define DEBUG(stmt)
#endif

using namespace db;
using namespace util;

db::EcoTable db::EcoTable::m_specimen;


namespace mstl {

template <> struct hash_key<db::Eco>
{
	static size_t hash(db::Eco key) { return key.code(); }
};

} // namespace mstl


struct EcoTable::StoredLineNode
{
	struct Branch
	{
		uint16_t				move;
		StoredLineNode*	node;
	};

	typedef mstl::hash<uint64_t,uint8_t> Map;
	typedef mstl::hash<Eco,Eco> KeyMap;

	static uint64_t const KeyMask = (1 << Eco::Bit_Size_Per_Subcode) - 1;

	Eco key() const;
	Eco opening() const;

	StoredLineNode const* find(uint16_t) const;

	static uint64_t makeValue(Eco key, Eco opening);

	uint64_t	value;
	uint8_t	index;
	Branch*	branches;
	unsigned	numBranches;

	static Map					m_storedLineMap;
	static KeyMap				m_keyMap;
	static StoredLineNode*	m_storedLineRoot;
};


EcoTable::StoredLineNode::Map		EcoTable::StoredLineNode::m_storedLineMap;
EcoTable::StoredLineNode::KeyMap	EcoTable::StoredLineNode::m_keyMap;
EcoTable::StoredLineNode*			EcoTable::StoredLineNode::m_storedLineRoot = 0;


Eco EcoTable::StoredLineNode::key() const			{ return Eco(value & KeyMask); }
Eco EcoTable::StoredLineNode::opening() const	{ return Eco(value >> Eco::Bit_Size_Per_Subcode); }


uint64_t
EcoTable::StoredLineNode::makeValue(Eco key, Eco opening)
{
	return uint64_t(key) | (uint64_t(opening) << Eco::Bit_Size_Per_Subcode);
}


EcoTable::StoredLineNode const*
EcoTable::StoredLineNode::find(uint16_t move) const
{
	return 0;
}


struct EcoTable::Branch
{
	uint16_t	move;
	uint8_t	weight;
	uint8_t	transposition;
	Node*		node;

	Branch() : move(0), weight(0), transposition(0), node(0) {}
};


struct EcoTable::Node
{
	struct Variation
	{
		typedef mstl::vector<Eco>	Codes;
		typedef mstl::vector<bool>	Transpositions;

		MoveList			moves;
		MoveList			branches;
		Codes				codes;
		Transpositions	transpositions;
	};

	typedef mstl::map<Eco,Variation*> Variations;
	typedef EcoTable::Entry Entry;
	typedef EcoTable::Branch Branch;

	Eco		eco;
	Node*		pred;
	Entry*	entry;
	Branch*	branches;
	uint8_t	numBranches;
	uint8_t	length;
	uint8_t	ply;

	mutable uint8_t flags;

	Branch* const find(uint16_t move) const;
	Node* find(Eco eco) const;

	void getSuccessors(Successors& successors, bool wantDerivedCodes) const;
	void traverse(EcoSet& reachable, bool wantTransposed) const;
	void decreasePawnMoves();
	void reset();

	void printName(Eco code, unsigned startLevel = 0) const;

	void dump(Board& board, unsigned ply) const;
	void dump();

	void print(Board& board, Variations& variations, MoveList& moves, unsigned ply);
	void print();
};


EcoTable::Branch* const
EcoTable::Node::find(uint16_t move) const
{
	return 0;
}


EcoTable::Node*
EcoTable::Node::find(Eco eco) const
{
	return 0;
}


void
EcoTable::Node::getSuccessors(Successors& successors, bool wantDerivedCodes) const
{
	//M_ASSERT(numBranches <= Max_Successors);

	typedef Successors::Successor Successor;

	unsigned n = 0;

	successors.length = n;
}


void
EcoTable::Node::traverse(EcoSet& reachable, bool wantTransposed) const
{
	reachable.set(eco);
}


void
EcoTable::Node::reset()
{
	if (!flags)
		return;

	flags = 0;

	for (unsigned i = 0; i < numBranches; ++i)
		branches[i].node->reset();
}


void
EcoTable::Node::printName(Eco code, unsigned startLevel) const
{
	Name const&		name = EcoTable::m_specimen.getName(code);
	mstl::string	s;

	for (unsigned i = startLevel; i < EcoTable::Num_Name_Parts && !name.part[i].empty(); ++i)
	{
		//codec.convertFromUtf8(name.part[i], s);
		::printf(" \"%s\"", s.c_str());
	}
}


void
EcoTable::Node::dump(Board& board, unsigned ply) const
{
	if (flags)
		return;

	flags = 1;

	for (unsigned i = 0; i < numBranches; ++i)
	{
		Branch const&	b = branches[i];
		mstl::string	s;

		Move move = board.makeMove(b.move);

		board.prepareForPrint(move);
		board.prepareUndo(move);
		board.doMove(move);

		for (unsigned k = 0; k < ply; ++k)
			::printf("| ");

		::printf("%s: %c%s%c",
					move.printSan(s).c_str(),
					b.transposition ? '[' : '(',
					b.node->eco.asString().c_str(),
					b.transposition ? ']' : ')');
		printName(b.node->eco, 1);
		::printf("\n");

		if (!b.transposition)
			b.node->dump(board, ply + 1);

		board.undoMove(move);
	}
}


void
EcoTable::Node::dump()
{
	Board board(Board::standardBoard());
	::printf("(%s)", eco.asString().c_str());
	printName(eco, 1);
	::printf("\n");
	dump(board, 0);
	reset();
}


void
EcoTable::Node::print(Board& board, Variations& variations, MoveList& moves, unsigned ply)
{
	if (flags)
		return;

	Variation* var = 0;

	flags = 1;

	if (length == ply)
	{
		//M_ASSERT(variations.find(eco) == variations.end());

		var = variations[eco] = new Variation;
		var->moves = moves;
	}

	for (unsigned i = 0; i < numBranches; ++i)
	{
		Branch const&	b		= branches[i];
		Move				move	= board.makeMove(b.move);

		board.prepareForPrint(move);
		board.prepareUndo(move);
		board.doMove(move);
		moves.push(move);

		if (length == ply)
		{
			var->branches.append(move);
			var->codes.push_back(b.node->eco);
			var->transpositions.push_back(b.transposition);
		}

		if (!b.transposition)
			b.node->print(board, variations, moves, ply + 1);

		moves.pop();
		board.undoMove(move);
	}
}


void
EcoTable::Node::print()
{
	Variations		variations;
	MoveList			moves;
	Board				board(Board::standardBoard());
	mstl::string	str;
	Eco				prev(Eco::root());

	variations.reserve(8192);
	print(board, variations, moves, 0);
	reset();

	for (Variations::const_iterator i = variations.begin(); i != variations.end(); ++i)
	{
		Variation const* v = i->second;

		if (!v->moves.isEmpty() || !v->branches.isEmpty())
		{
			Eco eco = i->first;

			if (prev != eco.basic())
			{
				prev = eco.basic();
				::printf("\n");
			}

			::printf("%s", eco.asString().c_str());
			printName(eco);

			for (unsigned k = 0; k < v->moves.size(); ++k)
			{
				str.clear();

				::printf(" ");
				if ((k & 1) == 0) ::printf("%u.", (k + 2)/2);
				::printf("%s", v->moves[k].printSan(str).c_str());
			}

			for (unsigned k = 0; k < v->branches.size(); ++k)
			{
				str.clear();
				if (v->transpositions[k])
					::printf(" [%s] ", v->codes[k].asString().c_str());
				else
					::printf(" (%s) ", v->codes[k].asString().c_str());
				if ((v->moves.size() & 1) == 0) ::printf("%u.", (v->moves.size() + 2)/2);
				::printf("%s", v->branches[k].printSan(str).c_str());
			}

			printf("\n");
			delete v;
		}
	}
}


struct EcoTable::Loader
{
	typedef mstl::vector<int>					Lengths;
	typedef mstl::vector<mstl::string>		NameRef;
	typedef EcoTable::Lookup::value_type	Pair;
	typedef EcoTable::Entry						Entry;
	typedef StoredLineNode::Branch			StoredLineBranch;

	mstl::istream&		m_strm;
	EcoTable&			m_specimen;
	EcoSet				m_ecoInfo;
	Branch*				m_branchCurr;
	Branch*				m_branchEnd;
	Node*					m_nodeCurr;
	Node*					m_nodeEnd;
	StoredLineNode*	m_storedLineNodeCurr;
	StoredLineNode*	m_storedLineNodeEnd;
	StoredLineBranch*	m_storedLineBranchCurr;
	StoredLineBranch*	m_storedLineBranchEnd;
	uint16_t*			m_moveCurr;
	uint16_t*			m_moveEnd;
	uint16_t				m_line[opening::Max_Line_Length + 1];
	NameRef				m_nameRef;
	Board					m_board;

	Loader(mstl::istream& strm, EcoTable& specimen);

	static void throwCorruptedData() { //DB_RAISE("corrupted data in ECO file"); 
}

	void readNode(unsigned ply, Node* node, Eco storedLineKey, Entry const* pred = 0);
	void readNode(StoredLineNode* node);

	void loadStoredLines();
	void load();
};


EcoTable::Loader::Loader(mstl::istream& strm, EcoTable& specimen)
	:m_strm(strm)
	,m_specimen(specimen)
	,m_ecoInfo(Eco::Max_Code + 1)
	,m_board(Board::standardBoard())
{
}


void
EcoTable::Loader::readNode(unsigned ply, Node* node, Eco storedLineKey, Entry const* pred)
{
  
}


void
EcoTable::Loader::load()
{
}


void
EcoTable::Loader::readNode(StoredLineNode* node)
{
  
}


void
EcoTable::Loader::loadStoredLines()
{
}


int
EcoTable::Successors::find(uint16_t move) const
{
	return -1;
}


EcoTable::EcoTable()
	:m_branchBuffer(0)
	,m_nodeBuffer(0)
	,m_nameBuffer(0)
	,m_moveBuffer(0)
	,m_root(0)
	,m_allocator(65536)
{
}


EcoTable::~EcoTable()
{
	delete [] m_branchBuffer;
	delete [] m_nodeBuffer;
	delete [] m_nameBuffer;
	delete [] m_moveBuffer;
}


bool
EcoTable::isLoaded() const
{
	return true;
}


void
EcoTable::load(mstl::istream& stream)
{
}


bool
EcoTable::isUsed(Eco code) const
{
    return false;
}


EcoTable::Name const&
EcoTable::getName(Eco code) const
{
    return Name();
}


void
EcoTable::getOpening(Eco code,
							mstl::string& openingLong,
							mstl::string& openingShort,
							mstl::string& variation,
							mstl::string& subVar) const
{
	openingLong.clear();
	openingShort.clear();
	variation.clear();
	subVar.clear();
}


void
EcoTable::getOpening(Eco code,
							mstl::string& opening,
							mstl::string& variation,
							mstl::string& subVar) const
{
	mstl::string dummy;
	return getOpening(code, opening, dummy, variation, subVar);
}


Eco
EcoTable::getEco(Board const& board) const
{
	return Eco();
}


Eco
EcoTable::getEco(Board const& startBoard, Line const& line, EcoSet* reachable) const
{
	return Eco();
}


Eco
EcoTable::getEco(Line const& line) const
{
	return Eco();
}


EcoTable::Entry const&
EcoTable::getEntry(Eco code) const
{
	return Entry();
}


uint8_t
EcoTable::getStoredLine(Eco key, Eco opening) const
{
	return 0;
}


void
EcoTable::getSuccessors(uint64_t hash, Successors& successors) const
{
	successors.length = 0;
}


Eco
EcoTable::lookup(	Line const& line,
						unsigned* length,
						Successors* successors,
						EcoSet* reachable) const
{
	return Eco();
}


void
EcoTable::print() const
{
	if (m_root)
	{
		m_root->print();
		fflush(stdout);
	}
}


void
EcoTable::dump() const
{
	if (m_root)
	{
		m_root->dump();
		fflush(stdout);
	}
}

// vi:set ts=3 sw=3:
