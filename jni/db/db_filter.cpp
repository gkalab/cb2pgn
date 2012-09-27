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

#include "db_filter.h"
#include "db_selector.h"
#include "db_query.h"
#include "db_database_content.h"

#include "m_assert.h"
#include "m_utility.h"
#include "m_stdio.h"

using namespace db;


Filter::Filter() : m_count(0) {}


Filter::Filter(unsigned size)
	:m_set(size)
	,m_count(size)
{
	m_set.set();
	//M_ASSERT(checkClassInvariance());
}


void
Filter::set()
{
	m_set.set();
	m_count = m_set.size();
	//M_ASSERT(checkClassInvariance());
}


void
Filter::reset()
{
	m_set.reset();
	m_count = 0;
	//M_ASSERT(checkClassInvariance());
}


void
Filter::negate()
{
	m_set.flip();
	m_count = m_set.size() - m_count;
	//M_ASSERT(checkClassInvariance());
}


//int
//Filter::toNumber(unsigned index) const
//{
//	//M_REQUIRE(index < size());
//
//	if (m_count == m_set.size())
//		return index;
//
//	if (!m_set.test(index))
//		return Invalid;
//
//	if (index <= mstl::div2(m_set.size()))
//		return m_set.count(0, index) - 1;
//
//	return m_count - m_set.count(index, m_set.size() - 1);
//}


unsigned
Filter::toIndex(unsigned number) const
{
	//M_REQUIRE(number < count());

	if (m_count == m_set.size())
		return number;

	if (number <= mstl::div2(m_count))
		return m_set.index(number);

	return m_set.rindex(m_count - number - 1);
}


void
Filter::resize(unsigned newSize, ResizeMode mode)
{
	unsigned size = m_set.size();

	if (newSize > size)
	{
		if (mode == AddNewIndices)
			m_count += newSize - size;
	}
	else if (m_count && newSize < size)
	{
		m_count -= m_set.count(newSize, size - 1);
	}

	m_set.resize(newSize);

	if (mode == AddNewIndices && newSize > size)
		m_set.set(size, newSize - 1);

	//M_ASSERT(checkClassInvariance());
}


void
Filter::search(Query const& query, DatabaseContent const& content)
{
	//M_REQUIRE(content.size() == size());

	switch (query.op())
	{
		case Query::Null:
			m_set.reset();
			m_count = 0;
			if (query.empty())
				return;
			// fallthru

		case Query::Or:
			if (m_count)
			{
				for (int i = m_set.find_first_not(); i != Invalid; i = m_set.find_next_not(i))
				{
					if (query.match(*content.m_gameInfoList[i]))
					{
						m_set.set(i);
						++m_count;
					}
				}
			}
			else
			{
				for (unsigned i = 0, n = m_set.size(); i < n; ++i)
				{
					if (query.match(*content.m_gameInfoList[i]))
					{
						m_set.set(i);
						++m_count;
					}
				}
			}
			break;

		case Query::And:
			if (m_count < content.m_gameInfoList.size())
			{
				for (int i = m_set.find_first(); i != Invalid; i = m_set.find_next(i))
				{
					if (!query.match(*content.m_gameInfoList[i]))
					{
						m_set.reset(i);
						--m_count;
					}
				}
			}
			else
			{
				for (unsigned i = 0, n = m_set.size(); i < n; ++i)
				{
					if (!query.match(*content.m_gameInfoList[i]))
					{
						m_set.reset(i);
						--m_count;
					}
				}
			}
			break;

		case Query::Not:
			if (m_count)
			{
				for (int i = m_set.find_first_not(); i != Invalid; i = m_set.find_next_not(i))
				{
					if (!query.match(*content.m_gameInfoList[i]))
					{
						m_set.set(i);
						++m_count;
					}
				}
			}
			else
			{
				for (unsigned i = 0, n = m_set.size(); i < n; ++i)
				{
					if (!query.match(*content.m_gameInfoList[i]))
					{
						m_set.set(i);
						++m_count;
					}
				}
			}
			break;

		case Query::Reset:
			m_set.set();
			m_count = m_set.size();
			if (query.empty())
				return;
			// fallthru

		case Query::Remove:
			if (m_count < content.m_gameInfoList.size())
			{
				for (int i = m_set.find_first(); i != Invalid; i = m_set.find_next(i))
				{
					if (query.match(*content.m_gameInfoList[i]))
					{
						m_set.reset(i);
						--m_count;
					}
				}
			}
			else
			{
				for (unsigned i = 0, n = m_set.size(); i < n; ++i)
				{
					if (query.match(*content.m_gameInfoList[i]))
					{
						m_set.reset(i);
						--m_count;
					}
				}
			}
			break;
	}

	//M_ASSERT(checkClassInvariance());
}


void
Filter::swap(Filter& filter)
{
	m_set.swap(filter.m_set);
	mstl::swap(m_count, filter.m_count);
}


void
Filter::dump() const
{
	for (int i = m_set.find_first(); i != Invalid; i = m_set.find_next(i))
		::printf("%d,", i);
	::printf("\n");
}

// vi:set ts=3 sw=3:
