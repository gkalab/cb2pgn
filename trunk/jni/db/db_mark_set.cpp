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

#include "db_mark_set.h"

#include "m_algorithm.h"
#include "m_assert.h"

#include <string.h>
#include <ctype.h>

using namespace db;


static int
compare(void const* lhs, void const* rhs)
{
	return static_cast<Mark const*>(lhs)->compare(*static_cast<Mark const*>(rhs));
}


bool
MarkSet::operator==(MarkSet const& marks) const
{
	if (count() != marks.count())
		return false;

	for (unsigned i = 0; i < count(); ++i)
	{
		if (!marks.contains(m_marks[i]))
			return false;
	}

	return true;
}


int
MarkSet::find(Mark const& mark) const
{
	Marks::const_iterator i = mstl::find(m_marks.begin(), m_marks.end(), mark);
	return i == m_marks.end() ? -1 : i - m_marks.begin();
}


int
MarkSet::match(Mark const& mark) const
{
	for (Marks::const_iterator i = m_marks.begin(); i != m_marks.end(); ++i)
	{
		if (mark.match(*i))
			return i - m_marks.begin();
	}

	return -1;
}


void
MarkSet::add(MarkSet const& set)
{
	m_marks.insert(m_marks.end(), set.m_marks.begin(), set.m_marks.end());
}


void
MarkSet::sort()
{
	::qsort(m_marks.begin(), m_marks.size(), sizeof(Marks::value_type), ::compare);
}


bool
MarkSet::extractFromComment(mstl::string& comment)
{
	//M_REQUIRE(comment.writeable() || comment.empty());

	mstl::string result;

	Mark mark;
	char const* p = mark.parseDiagramMarker(comment);

	if (!mark.isEmpty())
		add(mark);

	char const* q = ::strchr(p, '[');

	while (q)
	{
		if (q[1] == '%')
		{
			if (q[2] == 'c' && (q[3] == 'a' || q[3] == 's') && q[4] == 'l' && q[5] == ' ')
			{
				char const* e = 0;

				switch (q[3])
				{
					case 'a': e = parseChessBaseMark(q + 5, mark::Arrow); break;
					case 's': e = parseChessBaseMark(q + 5, mark::Full); break;
				}

				if (e && *e == ']')
				{
					result.append(p, q - p);
					p = e + 1;
					q = ::strchr(p, '[');
				}
				else
				{
					q = ::strchr(q + 1, '[');
				}
			}
			else
			{
				Mark mark;

				char const* e = mark.parseScidbMark(q);

				if (e == q)
					break;

				if (mark.isEmpty())
				{
					q = ::strchr(e == q ? e + 1: e, '[');
				}
				else
				{
					result.append(p, q - p);
					add(mark);
					p = e;
					q = ::strchr(e, '[');
				}
			}
		}
		else
		{
			q = ::strchr(q + 1, '[');
		}
	}

	bool rc = !isEmpty();

	if (rc)
	{
		result.append(p, comment.end());
		comment.swap(result);
		comment.trim();
	}

	//M_ASSERT(!isEmpty() || result.empty());

	return rc;
}


char const*
MarkSet::parseChessBaseMark(char const* s, mark::Type type)
{
	do
	{
		Mark mark;

		s = mark.parseChessBaseMark(s + 1, type);

		if (mark.isEmpty())
			return s;

		add(mark);
	}
	while (*s == ',');

	return s;
}


mstl::string&
MarkSet::toString(mstl::string& result) const
{
	for (unsigned i = 0; i < m_marks.size(); ++i)
	{
		if (i > 0)
			result += ' ';

		m_marks[i].toString(result);
	}

	return result;
}


mstl::string&
MarkSet::print(mstl::string& result) const
{
	for (unsigned i = 0; i < m_marks.size(); ++i)
	{
		if (i > 0)
			result += ' ';

		m_marks[i].print(result);
	}

	return result;
}


util::crc::checksum_t
MarkSet::computeChecksum(util::crc::checksum_t crc) const
{
	for (unsigned i = 0; i < m_marks.size(); ++i)
		crc = m_marks[i].computeChecksum(crc);

	return crc;
}

// vi:set ts=3 sw=3:
