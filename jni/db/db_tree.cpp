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

#include "db_tree.h"
#include "db_database.h"
#include "db_game_info.h"
#include "db_eco_table.h"
#include "db_signature.h"
#include "db_board.h"

#include "u_byte_stream.h"
#include "u_progress.h"

#include "m_bitset.h"
#include "m_limits.h"
#include "m_ref_counted_ptr.h"

#include <string.h>

#ifdef SHOW_TREE_INFO
# include "m_stdio.h"
#endif

using namespace db;

typedef EcoTable::Successors Successors;

static TreeInfo* moveCache[1 << Move::Index_Bit_Length];


static uint16_t const Empty	= Move::Empty;
static uint16_t const Null		= sq::h8 | (sq::h8 << 6);


static Successors::Successor const*
findSuccessor(Successors const& successors, uint32_t eco)
{
	for (unsigned i = 0; i < successors.length; ++i)
	{
		//M_ASSERT(!successors.list[i].reachable.empty());

		if (successors.list[i].reachable.test(eco))
			return &successors.list[i];
	}

	return 0;
}


inline static
uint16_t
index(Move const& m)
{
	return m.isNull() ? ::Null : m.index();
}


inline static
Move
move(Board const& position, uint16_t m)
{
	if (m == ::Null)	return Move::null();
	if (m == ::Empty)	return Move::empty();

	Move move = position.makeMove(m);
	move.setLegalMove();

	return move;
}


Tree::Key::Key()
	:m_hash(0)
	,m_position(Board::emptyBoard().exactPosition())
	,m_mode(tree::Exact)
	,m_ratingType(rating::Any)
{
}


Tree::Key::Key(uint64_t hash, Position const& position, tree::Mode mode, rating::Type ratingType)
	:m_hash(hash)
	,m_position(position)
	,m_mode(mode)
	,m_ratingType(ratingType)
{
}


void
Tree::Key::clear()
{
	m_hash = 0;
	m_position = Board::emptyBoard().exactPosition();
}


void
Tree::Key::set(tree::Mode mode, rating::Type ratingType, uint64_t hash, Position const& position)
{
	m_hash = hash;
	m_position = position;
	m_mode = mode;
	m_ratingType = ratingType;
}


void
Tree::add(GameInfo const& info, Eco eco, uint16_t move, Board const& myPosition)
{
	//M_ASSERT(move < U_NUMBER_OF(::moveCache));

	m_filter.add(m_index);

	TreeInfo* tinfo = ::moveCache[move];

	if (tinfo == 0)
	{
		Move m = ::move(myPosition, move);

		m_list.reserve_exact(m_list.size() + 1);
		m_list.push_back(TreeInfo(eco, m));
		::moveCache[move] = tinfo = &m_list.back();
	}
	else
	{
		//M_ASSERT(!eco || !tinfo->eco() || eco == tinfo->eco());
		//M_ASSERT(::move(myPosition, move) == tinfo->move());
	}

	tinfo->add(info, myPosition.sideToMove(), m_key.ratingType());

#ifdef BUILD_VARIATION_LIST
	if (m_buildVariationList && eco.ecoKey())
		addToVariation(eco, info.ecoKey());
#endif
}


void
Tree::possiblyAdd(Database const& base,
						GameInfo const& info,
						Eco eco,
						Board const& myPosition)
{
	Move m = base.findExactPositionAsync(m_index, myPosition, true);

	if (!m.isInvalid())
		add(info, eco, ::index(m), myPosition);

#ifdef SHOW_TREE_INFO
	++m_numGamesParsed;
#endif
}


bool
Tree::buildTree0(	unsigned myIdn,
						Board const& startPosition,
						Board const& myPosition,
						Line const& myLine,
						uint16_t hpSig,
						Database const& base,
						tree::Mode mode,
						ReachableFunc reachableFunc,
						util::Progress& progress,
						unsigned frequency,
						unsigned numGames)
{
	typedef EcoTable::EcoSet EcoSet;

	//M_ASSERT(myIdn == 0);
//	//M_ASSERT(!myPosition.isStartPosition());

	unsigned	reportAfter = m_index + frequency;
	EcoSet	reachable;

	// TODO: we need successors
	Eco myEco = EcoTable::specimen().getEco(startPosition, myLine, &reachable);

	if (myEco)
	{
		for ( ; m_index < numGames; ++m_index)
		{
			if (progress.interrupted())
				return false;

			if (reportAfter == m_index)
			{
				progress.update(m_index);
				reportAfter += frequency;
			}

			GameInfo const& info = base.gameInfo(m_index);

			if (reachableFunc(myPosition.signature(), info.signature(), hpSig))
			{
				if (info.idn() == chess960::StandardIdn)
				{
					if (mode == tree::Exact)
					{
						Move m = base.findExactPositionAsync(m_index, myPosition, true);

						if (m.isLegal())
						{
							Board board(myPosition);
							board.doMove(m);
							add(info, EcoTable::specimen().getEco(board), ::index(m), myPosition);
						}
					}
					else
					{
						Eco otherEco = Eco(info.ecoKey());

						if (reachable.test(otherEco))
							possiblyAdd(base, info, otherEco, myPosition);
					}
				}
				else
				{
					Move m = base.findExactPositionAsync(m_index, myPosition, true);

					if (m.isLegal())
					{
						Board board(myPosition);
						board.doMove(m);
						add(info, EcoTable::specimen().getEco(board), ::index(m), myPosition);
					}
				}
			}
		}
	}
	else
	{
		for (unsigned n = base.countGames(); m_index < n; ++m_index)
		{
			if (reportAfter == m_index)
			{
				progress.update(m_index);
				reportAfter += frequency;
			}

			GameInfo const& info = base.gameInfo(m_index);

			if (reachableFunc(myPosition.signature(), info.signature(), hpSig))
				possiblyAdd(base, info, Eco(), myPosition);
		}
	}

	return true;
}


bool
Tree::buildTree518(	unsigned myIdn,
							Board const& startPosition,
							Board const& myPosition,
							Line const& myLine,
							uint16_t hpSig,
							Database const& base,
							tree::Mode mode,
							ReachableFunc reachableFunc,
							util::Progress& progress,
							unsigned frequency,
							unsigned numGames)
{
	typedef EcoTable::EcoSet EcoSet;

	//M_ASSERT(myIdn == chess960::StandardIdn);
	//M_ASSERT(!myPosition.isStandardPosition());

	unsigned				reportAfter = m_index + frequency;
	Successors			successors;
	unsigned				myLength;
	EcoSet				reachable;
	pawns::Progress	myProgress;

//	Eco myEco = EcoTable::specimen().getEco(myLine);
	Eco myKey = EcoTable::specimen().lookup(myLine, &myLength, &successors, &reachable);

	myProgress.side[color::White] = myPosition.signature().progress(color::White);
	myProgress.side[color::Black] = myPosition.signature().progress(color::Black);

	for ( ; m_index < numGames; ++m_index)
	{
		if (reportAfter == m_index)
		{
			progress.update(m_index);
			reportAfter += frequency;
		}

		if (progress.interrupted())
			return false;

		GameInfo const& info = base.gameInfo(m_index);

//		if (mode == tree::Rapid)
//		{
//			Eco	otherKey		= Eco(info.ecoKey());
//			bool	isReachable	= reachable.test(otherKey);
//
//			if (isReachable)
//			{
//				Successors::Successor const* succ = findSuccessor(successors, otherKey);
//
//				if (succ)
//					add(info, succ->eco, succ->move, myPosition);
//				else if (reachableFunc(myPosition.signature(), info.signature(), hpSig))
//					possiblyAdd(base, info, Eco(), myPosition);
//			}
//		}
//		else
		{
			if (info.idn() != chess960::StandardIdn)
			{
				if (info.idn() == 0 || mode == tree::Exact)
					possiblyAdd(base, info, Eco(), myPosition);
			}
			else if (info.plyCount() && reachableFunc(myPosition.signature(), info.signature(), hpSig))
			{
//				if (	myLength == info.plyCount()
//					&& myLine.length == myLength
//					&& info.ecoOpening() == myOpening)
//				{
//					add(info, myEco, ::Empty, myPosition);
//				}
//				else
				{
					Eco	otherKey		= Eco(info.ecoKey());
					bool	isReachable	= reachable.test(otherKey);

					Successors::Successor const* succ = 0;

					if (isReachable && otherKey != myKey)
						succ = findSuccessor(successors, otherKey);

					if (succ)
						add(info, succ->eco, succ->move, myPosition);
					else
						possiblyAdd(base, info, Eco(), myPosition);
				}
			}
		}
	}

	return true;
}


bool
Tree::buildTree960(	unsigned myIdn,
							Board const& startPosition,
							Board const& myPosition,
							Line const& myLine,
							uint16_t hpSig,
							Database const& base,
							tree::Mode mode,
							ReachableFunc reachableFunc,
							util::Progress& progress,
							unsigned frequency,
							unsigned numGames)
{
	typedef EcoTable::EcoSet EcoSet;

	//M_ASSERT(myIdn != 0);
	//M_ASSERT(myIdn != chess960::StandardIdn);
	//M_ASSERT(!myPosition.isStandardPosition());

	unsigned reportAfter = m_index + frequency;

	for ( ; m_index < numGames; ++m_index)
	{
		if (reportAfter == m_index)
		{
			progress.update(m_index);
			reportAfter += frequency;
		}

		if (progress.interrupted())
			return false;

		GameInfo const& info = base.gameInfo(m_index);

		if (reachableFunc(myPosition.signature(), info.signature(), hpSig))
		{
			if (info.idn() == myIdn)
			{
				switch (myLine.length)
				{
					case 0:
						break;

					case 1:
						switch (info.plyCount())
						{
							case 0: break;

							case 1:
								if (myLine[0] == info.ply<0>())
									add(info, Eco(), ::Empty, myPosition);
								break;

							case 2:
								if (myLine[0] == info.ply<0>())
									add(info, Eco(), info.ply<1>(), myPosition);
								break;

							default:
									if (myLine[0] == info.ply<0>())
									add(info, Eco(), info.ply<1>(), myPosition);
								else
									possiblyAdd(base, info, Eco(), myPosition);
								break;
						}
						break;

					case 2:
						switch (info.plyCount())
						{
							case 0: break;
							case 1: break;

							case 2:
								if (myLine[0] == info.ply<0>() && myLine[1] == info.ply<1>())
								{
									add(info, Eco(), ::Empty, myPosition);
									break;
								}
								// fallthru

							default:
								possiblyAdd(base, info, Eco(), myPosition);
								break;
						}
						break;

					default:
						if (info.plyCount() > 2)
							possiblyAdd(base, info, Eco(), myPosition);
						break;
				}
			}
			else if (mode == tree::Exact)
			{
				possiblyAdd(base, info, Eco(), myPosition);
			}
		}
	}

	return true;
}


bool
Tree::buildTreeStandard(unsigned myIdn,
								Board const& startPosition,
								Board const& myPosition,
								Line const& myLine,
								uint16_t hpSig,
								Database const& base,
								tree::Mode mode,
								ReachableFunc reachableFunc,
								util::Progress& progress,
								unsigned frequency,
								unsigned numGames)
{
	typedef EcoTable::EcoSet EcoSet;

	//M_ASSERT(myIdn == chess960::StandardIdn);
	//M_ASSERT(myPosition.isStandardPosition());

	unsigned		reportAfter = m_index + frequency;
	Successors	successors;
	unsigned		myLength;	// unused

	Eco myEco = EcoTable::specimen().lookup(myLine, &myLength, &successors);

	for ( ; m_index < numGames; ++m_index)
	{
		if (reportAfter == m_index)
		{
			progress.update(m_index);
			reportAfter += frequency;
		}

		if (progress.interrupted())
			return false;

		GameInfo const& info = base.gameInfo(m_index);

		if (info.idn() == chess960::StandardIdn)
		{
			if (info.plyCount() == 0)
			{
				add(info, myEco, ::Empty, myPosition);
			}
			else
			{
				Successors::Successor const* succ = findSuccessor(successors, info.ecoKey());

				if (succ)	// should always be non-null
					add(info, succ->eco, succ->move, myPosition);
				else
					possiblyAdd(base, info, Eco(), myPosition);
			}
		}
		else if (info.idn() == 0)
		{
			// NOTE: In Scid it is possible that a standard position
			// is declared as a non-standard position.
			if (reachableFunc(myPosition.signature(), info.signature(), hpSig))
				possiblyAdd(base, info, Eco(), myPosition);
		}
	}

	return true;
}


bool
Tree::buildTreeStart(unsigned myIdn,
							Board const& startPosition,
							Board const& myPosition,
							Line const& myLine,
							uint16_t hpSig,
							Database const& base,
							tree::Mode mode,
							ReachableFunc reachableFunc,
							util::Progress& progress,
							unsigned frequency,
							unsigned numGames)
{
	typedef EcoTable::EcoSet EcoSet;

	//M_ASSERT(myIdn != chess960::StandardIdn);
	//M_ASSERT(myPosition.isStartPosition());

	unsigned reportAfter = m_index + mstl::max(frequency, 1000u);

	for ( ; m_index < numGames; ++m_index)
	{
		if (reportAfter == m_index)
		{
			progress.update(m_index);
			reportAfter += frequency;
		}

		if (progress.interrupted())
			return false;

		GameInfo const& info = base.gameInfo(m_index);

		static_assert(::Empty == 0, "reimplementation required");

		if (info.idn() == myIdn)
		{
			if (info.plyCount() == 0)
				add(info, Eco(), ::Empty, myPosition);
			else if (info.ply<0>() == 0)
				possiblyAdd(base, info, Eco(), myPosition);
			else
				add(info, Eco(), info.ply<0>(), myPosition);
		}
		else if (info.idn() == 0)	// match is really possible?
		{
			possiblyAdd(base, info, Eco(), myPosition);
		}
	}

	return true;
}


Tree*
Tree::makeTree(TreeP tree,
					unsigned myIdn,
					Board startPosition,
					Board myPosition,
					Line myLine,
					uint16_t hpSig,
					Database& base,
					tree::Mode mode,
					rating::Type ratingType,
					util::Progress& progress)
{
	// M_REQUIRE(base.format() != format::ChessBase);

	typedef bool (Tree::*BuildMeth)(	unsigned,
												Board const&,
												Board const&,
												Line const&,
												uint16_t,
												Database const&,
												tree::Mode,
												ReachableFunc,
												util::Progress&,
												unsigned,
												unsigned);

	BuildMeth buildMeth;

	if (myIdn == 0)
		buildMeth = &Tree::buildTree0;
	else if (myPosition.isStandardPosition())
		buildMeth = &Tree::buildTreeStandard;
	else if (myPosition.isStartPosition())
		buildMeth = &Tree::buildTreeStart;
	else if (myIdn == chess960::StandardIdn)
		buildMeth = &Tree::buildTree518;
	else
		buildMeth = &Tree::buildTree960;

	util::ProgressWatcher watcher(progress, base.countGames());

	unsigned frequency = progress.frequency(base.countGames());

	if (frequency == 0)
		frequency = mstl::min(1000u, mstl::max(base.countGames()/1000u, 1u));

	ReachableFunc reachableFunc;

	if (mode == tree::Exact || base.format() != format::Scidb)
		reachableFunc = Signature::isReachablePosition;
	else
		reachableFunc = Signature::isReachable;

	::memset(::moveCache, 0, sizeof(::moveCache));

	if (tree)
	{
		// we have to rebuild the move cache
		for (unsigned i = 0; i < tree->m_list.size(); ++i)
		{
			TreeInfo& info = tree->m_list[i];
			::moveCache[::index(info.move())] = &info;
		}

		progress.update(tree->m_index);
	}
	else
	{
		tree.reset(new Tree);

		tree->m_key.set(mode, ratingType, myPosition.hash(), myPosition.exactPosition());
		tree->m_index = 0;
		tree->m_last = mstl::numeric_limits<unsigned>::max();
		tree->m_complete = false;
		tree->m_base = &base;

#ifdef SHOW_TREE_INFO
		tree->m_numGamesParsed = 0;
#endif
	}

	tree->m_filter.resize(base.countGames(), Filter::LeaveEmpty);

	if ((tree.get()->*buildMeth)(	myIdn,
											startPosition,
											myPosition,
											myLine,
											hpSig,
											base,
											mode,
											reachableFunc,
											progress,
											frequency,
											mstl::min(base.countGames(), tree->m_last)))
	{
		uint16_t buf[opening::Max_Line_Length];

		Line line(buf);
		line.copy(myLine);
		line.length++;

		for (unsigned i = 0; i < tree->m_list.size(); ++i)
		{
			TreeInfo& info = tree->m_list[i];

			if (info.move())
			{
				info.move().setColor(myPosition.sideToMove());
				myPosition.prepareForPrint(info.move());
			}

			if (	line.length <= opening::Max_Line_Length
				&& !info.eco()
				&& (	myIdn == chess960::StandardIdn
					|| (myIdn == 0 && !myPosition.notDerivableFromStandardChess())))
			{
				buf[line.length - 1] = ::index(info.move());
				info.setEco(EcoTable::specimen().getEco(line));
			}

			tree->m_total.add(info, tree->m_key.ratingType());
		}

		tree->m_complete = true;

#ifdef SHOW_TREE_INFO
		fprintf(stderr, "games parsed: %u\n", tree->m_numGamesParsed);
#endif
	}

	return tree.release();
}


void
Tree::setIncomplete()
{
	m_complete = false;
	m_last = mstl::numeric_limits<unsigned>::max();
}


void
Tree::setIncomplete(unsigned index)
{
	if (m_complete)
	{
		m_index = index;
		m_last = index;
		m_complete = false;
	}
	else if (index < m_index)
	{
		m_index = index;
	}
}


bool
Tree::isTreeFor(Database const& base, Key const& key) const
{
	return m_complete && m_base->id() == base.id() && m_key == key;
}


bool
Tree::isTreeFor(Database const& base, Board const& position) const
{
	return	m_complete
			&& m_base->id() == base.id()
			&& m_key.hash() == position.hash()
			&& m_key.position() == position.exactPosition();
}


bool
Tree::isTreeFor(	Database const& base,
						Board const& position,
						tree::Mode mode,
						rating::Type ratingType) const
{
	return	m_complete
			&& m_base->id() == base.id()
			&& m_key.match(mode, ratingType, position.hash(), position.exactPosition());
}


void
Tree::sort(attribute::tree::ID column)
{
	if (m_list.size() <= 1)
		return;

	for (unsigned k = 0, n = m_list.size() - 1; k < n; ++k)
	{
		unsigned index = k;

		TreeInfo* info = &m_list[index];

		for (unsigned i = k + 1; i <= n; ++i)
		{
			if (m_list[i].isLessThan(*info, m_key.ratingType(), column))
				info = &m_list[index = i];
		}

		if (index > k)
			mstl::swap(m_list[k], m_list[index]);
	}
}

// vi:set ts=3 sw=3:
