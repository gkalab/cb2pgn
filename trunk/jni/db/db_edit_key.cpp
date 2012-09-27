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

#include "db_edit_key.h"
#include "db_game.h"
#include "db_board.h"
#include "db_move_node.h"

#include "m_utility.h"
#include "m_stdio.h"
#include "m_assert.h"

#include <ctype.h>

using namespace db::edit;


static mstl::string const StartKey("m-0");


inline static char const* skipPrefix(char const* s) { return s + 2; }


Key::Key() :m_id(StartKey) {}
Key::Key(unsigned firstPly) :m_id(StartKey) { addPly(firstPly); }
Key::Key(mstl::string const& key) :m_id(key) { isValid(key); }
Key::Key(char const* key) :m_id(key) { isValid(key); }


Key::Key(mstl::string const& key, char prefix)
	:m_id(key)
{
	//M_REQUIRE(isValid(key));
	m_id[0] = prefix;
}


Key::Key(Key const& key, char prefix)
	:m_id(key.id())
{
	//M_REQUIRE(isValid(key.id()));
	m_id[0] = prefix;
}


bool
Key::isVariationId() const
{
	unsigned level = 0;

	for (char const *s = ::skipPrefix(m_id); *s; ++s)
	{
		if (*s == '.')
			++level;
	}

	return mstl::is_even(level);
}


bool
Key::isMainlineId() const
{
	unsigned level = 0;

	for (char const *s = ::skipPrefix(m_id); *s; ++s)
	{
		if (*s == '.' && ++level == 2)
			return false;
	}

	return true;
}


unsigned
Key::level() const
{
	unsigned level = 0;

	for (char const *s = ::skipPrefix(m_id); *s; ++s)
	{
		if (*s == '.')
			++level;
	}

	return mstl::div2(level);
}


void
Key::addPly(unsigned ply)
{
	//M_REQUIRE(isVariationId());

	char buf[32];
	m_id.append(buf, ::sprintf(buf, ".%u", ply));
}


void
Key::exchangePly(unsigned ply)
{
	//M_REQUIRE(!isVariationId());

	char buf[32];

	unsigned pos = m_id.rfind('.') + 1;
	m_id.replace(pos, m_id.size() - pos, buf, ::sprintf(buf, "%u", ply));
}


void
Key::removePly()
{
	//M_REQUIRE(!isVariationId());
	m_id.erase(m_id.rfind('.'), mstl::string::npos);
}


void
Key::incrementPly()
{
	//M_REQUIRE(!isVariationId());

	char const* s = m_id.end();
	char const* t = ::skipPrefix(m_id);

	while (s > t && s[-1] != '.')
		--s;

	unsigned number = ::strtoul(s, nullptr, 10);

	m_id.resize(s - m_id.begin());
	m_id.format("%u", number + 1);
}


void
Key::addVariation(unsigned varno)
{
	//M_REQUIRE(!isVariationId());

	char buf[32];
	m_id.append(buf, ::sprintf(buf, ".%u", varno));
}


void
Key::exchangeVariation(unsigned varno)
{
	//M_REQUIRE(isVariationId());
	//M_REQUIRE(level() > 0);

	char buf[32];

	unsigned pos = m_id.rfind('.')  + 1;
	m_id.replace(pos, m_id.size() - pos, buf, ::sprintf(buf, "%u", varno));
}


void
Key::removeVariation()
{
	//M_REQUIRE(isVariationId());
	//M_REQUIRE(level() > 0);

	m_id.erase(m_id.rfind('.'), mstl::string::npos);
}


void
Key::exchangePrefix(char prefix)
{
	//M_REQUIRE(::isalpha(prefix));
	m_id[0] = prefix;
}


void
Key::clear()
{
	m_id.assign(StartKey);
}


void
Key::reset(unsigned firstPly)
{
	m_id.assign(StartKey);
	m_id += '.';
	m_id.format("%u", firstPly);
}


bool
Key::isValid(mstl::string const& key)
{
	char const* s = key;

	if (s[0] == '\0' || s[1] != '-')
		return false;

		s += 2;

	if (*s++ != '0')
		return false;

	while (*s)
	{
		if (*s++ != '.')
			return false;

		if (*s == '0')
		{
			++s;
		}
		else
		{
			if (!::isdigit(*s++))
				return false;

			while (::isdigit(*s))
				++s;
		}
	}

	return true;
}


bool
Key::setPosition(Game& game) const
{
	game.moveToMainlineStart();

	char const* s = ::skipPrefix(m_id);

	if (s[1] == '\0')
		return true;

	s += 2;

	char* e = 0;

	int plyNumber = game.startBoard().plyNumber();

	while (*s)
	{
		unsigned num = ::strtoul(s, &e, 10) - plyNumber;
		unsigned n   = game.forward(num);

		if (n == num -1)
			return *e == '\0'; // trailing comment position

		if (n != num)
			return false;

		s = *e == '.' ? e + 1 : e;

		if (*e)
		{
			unsigned varNo = ::strtoul(s, &e, 10);

			if (varNo >= game.variationCount())
				return false;

			game.enterVariation(varNo);
			s = *e == '.' ? e + 1 : e;
		}

		plyNumber = game.currentBoard().plyNumber();
	}

	return true;
}


db::MoveNode*
Key::findPosition(MoveNode* node, unsigned plyNumber) const
{
	//M_REQUIRE(node->atLineStart());
	//M_REQUIRE(node->prev() == 0);

	char const* s = ::skipPrefix(m_id);

	if (s[1] == '\0')
		return node;

	s += 2;

	char* e = 0;

	while (*s)
	{
		unsigned nextPly = ::strtoul(s, &e, 10);

		for ( ; plyNumber < nextPly; ++plyNumber)
		{
			if (!node->next())
				return node;	// should not happen

			node = node->next();
		}

		s = *e == '.' ? e + 1 : e;

		if (*e)
		{
			unsigned varNo = ::strtoul(s, &e, 10);

			if (varNo >= node->variationCount())
				return 0;

			node = node->variation(varNo)->next();
			// M_ASSERT(node);
			s = *e == '.' ? e + 1 : e;
		}
	}

	return node;
}


bool
Key::setBoard(MoveNode const* root, Board& board) const
{
	//M_REQUIRE(root);

	MoveNode const*	node	= root;
	char const*			s		= ::skipPrefix(m_id);
	char*					e		= 0;

	if (s[1])
	{
		s += 2;

		int plyNumber = board.plyNumber();

		while (*s)
		{
			int num = ::strtoul(s, &e, 10) - plyNumber;

			for ( ; num > 0; --num)
			{
				if (!node->atLineStart())
					board.doMove(node->move());

				if ((node = node->next()) == 0)
					return false;
			}

			s = *e == '.' ? e + 1 : e;

			if (*e)
			{
				unsigned varNo = ::strtoul(s, &e, 10);

				if (varNo >= node->variationCount())
					return false;

				node = node->variation(varNo);
				s = *e == '.' ? e + 1 : e;
			}

			plyNumber = board.plyNumber();
		}
	}

	if (!node->atLineStart())
		board.doMove(node->move());

	return true;
}


Key
Key::successorKey(MoveNode const* node) const
{
	//M_REQUIRE(node);
	//M_REQUIRE(node->isBeforeLineEnd());

	if (node->next()->atLineEnd())
	{
		unsigned ply = plyNumber();

		while (!node->atLineStart())
		{
			node = node->prev();
			--ply;
		}

		if (!node->prev())
			return Key();

		unsigned i = node->prev()->variationNumber(node) + 1;
		Key key(m_id);

		node = node->prev();
		key.removePly();
		key.removeVariation();

		if (i == node->variationCount())
		{
			key.incrementPly();
		}
		else
		{
			key.addVariation(i);
			key.addPly(ply);
		}
	}

	Key key(m_id);
	key.incrementPly();
	return key;
}


unsigned
Key::plyNumber() const
{
	char const* s = m_id.end();
	char const* t = ::skipPrefix(m_id);

	while (s > t && s[-1] != '.')
		--s;
	return ::strtoul(s, nullptr, 10);
}


int
Key::computeDistance(Key const& key) const
{
	//M_REQUIRE(level() == key.level());
	//M_REQUIRE(isVariationId() == key.isVariationId());

	return int(plyNumber()) - int(key.plyNumber());
}


bool
Key::operator<(Key const& key) const
{
	unsigned lhsLevel = level();
	unsigned rhsLevel = key.level();

	if (lhsLevel < rhsLevel)
		return true;
	if (rhsLevel < lhsLevel)
		return false;
	return computeDistance(key) < 0;
}

// vi:set ts=3 sw=3:
