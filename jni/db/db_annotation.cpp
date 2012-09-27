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

#include "db_annotation.h"

#include "m_utility.h"
#include "m_string.h"
#include "m_stdio.h"

#include <string.h>
#include <stdlib.h>
#include <ctype.h>

using namespace db;
using namespace db::nag;


static void
addNumber(mstl::string& str, unsigned n)
{
	char buf[20];
	snprintf(buf, sizeof(buf), "%u", n);
	buf[sizeof(buf) - 1] = '\0';
	str += buf;
}


static void
addSpace(mstl::string& str)
{
	if (!str.empty())
		str += ' ';
}


static void
append(mstl::string& result, mstl::string const& s)
{
	if (!s.empty() && !result.empty() && result.back() != ' ')
		result += ' ';
	result += s;
}


class Annotation::Default
{
public:

	Default()
	{
		for (unsigned i = 0; i < U_NUMBER_OF(m_sets); ++i)
		{
			if (nag::ID(i) != nag::Null)
				m_sets[i].m_annotation[m_sets[i].m_count++] = nag::ID(i);
		}
	}

	bool contains(Annotation const* p) const
	{
		return &m_sets[0] <= p && p <= &m_sets[U_NUMBER_OF(m_sets) - 1];
	}

	Annotation* get(nag::ID nag) { return &m_sets[nag]; }

	Annotation m_sets[nag::Scidb_Last];
};


static Annotation::Default defaultSets;


Annotation::Annotation(nag::ID nag)
{
	if (nag == nag::Null)
	{
		m_count = 0;
	}
	else
	{
		m_annotation[0] = nag;
		m_count = 1;
	}
}


Annotation::Annotation(mstl::string const& str)
	:m_count(0)
{
	add(str.c_str());
}


bool
Annotation::operator==(Annotation const& annotation) const
{
	if (m_count != annotation.m_count)
		return false;

	for (unsigned i = 0; i < m_count; ++i)
	{
		if (!annotation.contains(nag::ID(m_annotation[i])))
			return false;
	}

	return true;
}


bool Annotation::isDefaultSet() const { return ::defaultSets.contains(this); }
Annotation const* Annotation::defaultSet(nag::ID nag) { return ::defaultSets.get(nag); }


bool
Annotation::hasTrailingAnnotation() const
{
	for (unsigned i = 0; i < m_count; ++i)
	{
		if (!nag::isPrefix(nag::ID(m_annotation[i])))
			return true;
	}

	return false;
}


bool
Annotation::contains(nag::ID nag) const
{
	for (unsigned i = 0; i < m_count; ++i)
	{
		if (m_annotation[i] == nag)
			return true;
	}

	return false;
}


bool
Annotation::add(nag::ID nag)
{
	//M_REQUIRE(!isDefaultSet());
	//M_REQUIRE(!contains(nag));

	if (nag == Null)
		return true;

	if (m_count == Max_Nags || nag >= nag::Scidb_Last)
		return false;

	m_annotation[m_count++] = nag;
	return true;
}


void
Annotation::set(nag::ID nag)
{
	//M_REQUIRE(!isDefaultSet());

	clear();

	if (nag != nag::Null)
		m_annotation[m_count++] = nag;
}


void
Annotation::remove(nag::ID nag)
{
	//M_REQUIRE(!isDefaultSet());

	if (nag == Null)
		return;

	for (unsigned i = 0; i < m_count; ++i)
	{
		if (nag == m_annotation[i])
		{
			::memmove(&m_annotation[i], &m_annotation[i + 1], (Max_Nags - i - 1)*sizeof(m_annotation[0]));
			--m_count;
			return;
		}
	}
}


mstl::string&
Annotation::prefix(mstl::string& result) const
{
	for (unsigned i = 0; i < m_count; ++i)
	{
		if (nag::isPrefix(nag::ID(m_annotation[i])))
		{
			if (!result.empty())
				::addSpace(result);
			result += "$";
			::addNumber(result, m_annotation[i]);
		}
	}

	return result;
}


mstl::string&
Annotation::infix(mstl::string& result) const
{
	for (unsigned i = 0; i < m_count; ++i)
	{
		if (nag::isInfix(nag::ID(m_annotation[i])))
		{
			if (!result.empty())
				::addSpace(result);
			result += "$";
			::addNumber(result, m_annotation[i]);
		}
	}

	return result;
}


mstl::string&
Annotation::suffix(mstl::string& result) const
{
	for (unsigned i = 0; i < m_count; ++i)
	{
		if (nag::isSuffix(nag::ID(m_annotation[i])))
		{
			if (!result.empty())
				::addSpace(result);
			result += "$";
			::addNumber(result, m_annotation[i]);
		}
	}

	return result;
}


mstl::string&
Annotation::print(mstl::string& result, unsigned flags) const
{
	mstl::string symbolic1;
	mstl::string symbolic2;
	mstl::string evaluation;
	mstl::string other;

	uint8_t firstNag = Null;

	for (unsigned i = 0; i < m_count; ++i)
	{
		switch (m_annotation[i])
		{
			case GoodMove:
			case PoorMove:
			case VeryGoodMove:
			case VeryPoorMove:
			case SpeculativeMove:
			case QuestionableMove:
				if ((flags & Flag_Symbolic_Annotation_Style) && firstNag == Null)
				{
					switch (unsigned(m_annotation[i]))
					{
						case GoodMove:				result += "!"; break;
						case PoorMove:				result += "?"; break;
						case VeryGoodMove:		result += "!!"; break;
						case VeryPoorMove:		result += "??"; break;
						case SpeculativeMove:	result += "!?"; break;
						case QuestionableMove:	result += "?!"; break;
					}
					firstNag = m_annotation[i];
				}
				else
				{
					::addSpace(result);
					result += "$";
					::addNumber(result, m_annotation[i]);
				}
				break;

			case ForcedMove:
			case SingularMove:
			case WorstMove:
				::addSpace(other);
				other += "$";
				::addNumber(other, m_annotation[i]);
				break;

			case DrawishPosition:
			case UnclearPosition:
			case WhiteHasASlightAdvantage:
			case BlackHasASlightAdvantage:
			case WhiteHasAModerateAdvantage:
			case BlackHasAModerateAdvantage:
			case WhiteHasADecisiveAdvantage:
			case BlackHasADecisiveAdvantage:
			case WhiteHasSufficientCompensationForMaterialDeficit:
			case BlackHasSufficientCompensationForMaterialDeficit:
				::addSpace(symbolic2);
				if (flags & Flag_Extended_Symbolic_Annotation_Style)
				{
					switch (unsigned(m_annotation[i]))
					{
						case DrawishPosition:					symbolic2 += "="; break;
						case UnclearPosition:					symbolic2 += "~"; break;
						case WhiteHasASlightAdvantage:		symbolic2 += "+="; break;
						case BlackHasASlightAdvantage:		symbolic2 += "=+"; break;
						case WhiteHasAModerateAdvantage:		symbolic2 += "+/-"; break;
						case BlackHasAModerateAdvantage:		symbolic2 += "-/+"; break;
						case WhiteHasACrushingAdvantage:		// fallthru
						case WhiteHasADecisiveAdvantage:		symbolic2 += "+-"; break;
						case BlackHasACrushingAdvantage:		// fallthru
						case BlackHasADecisiveAdvantage:		symbolic2 += "-+"; break;

						case WhiteHasSufficientCompensationForMaterialDeficit: symbolic2 += "~="; break;
						case BlackHasSufficientCompensationForMaterialDeficit: symbolic2 += "=~"; break;
					}
				}
				else
				{
					symbolic2 += "$";
					::addNumber(symbolic2, m_annotation[i]);
				}
				break;

			case EqualChancesQuietPosition:
			case EqualChancesActivePosition:
				::addSpace(evaluation);
				evaluation += "$";
				::addNumber(evaluation, m_annotation[i]);
				break;

			case Novelty:
			case Diagram:
				if (flags & Flag_Extended_Symbolic_Annotation_Style)
				{
					::addSpace(symbolic1);
					symbolic1 += m_annotation[i] == Novelty ? 'N' : 'D';
					break;
				}
				// fallthru

			default:
				::addSpace(other);
				other += "$";
				::addNumber(other, nag::ID(m_annotation[i]));
				break;
		}
	}

	::append(result, evaluation);
	::append(result, symbolic1);
	::append(result, symbolic2);
	::append(result, other);

	return result;
}


mstl::string
Annotation::dump() const
{
	mstl::string s;
	print(s);
	return s;
}


bool
Annotation::add(char const* str)
{
	//M_REQUIRE(!isDefaultSet());

	bool rc = true;

	while (::isspace(*str))
		++str;

	while (*str == '$')
	{
		if (m_count == Max_Nags)
			return false;

		nag::ID nag = nag::ID(::strtoul(str + 1, const_cast<char**>(&str), 10));

		if (nag >= nag::Scidb_Last)
			rc = false;
		else if (nag != nag::Null && !contains(nag))
			m_annotation[m_count++] = nag;

		while (::isspace(*str))
			++str;
	}

	return rc;
}


unsigned
Annotation::add(Annotation const& set)
{
	//M_REQUIRE(!isDefaultSet());

	uint8_t n = mstl::min(set.m_count, uint8_t(Max_Nags - m_count));
	;;memcpy(m_annotation + m_count, set.m_annotation, n);
	m_count += n;
	return n;
}


void
Annotation::sort()
{
	for (unsigned n = m_count; n > 1; --n)
	{
		bool done = true;

		for (unsigned i = 1; i < n; ++i)
		{
			if (m_annotation[i - 1] > m_annotation[i])
			{
				mstl::swap(m_annotation[i - 1], m_annotation[i]);
				done = false;
			}
		}

		if (done)
			return;
	}
}


util::crc::checksum_t
Annotation::computeChecksum(util::crc::checksum_t crc) const
{
	return ::util::crc::compute(crc, m_annotation, m_count);
}

// vi:set ts=3 sw=3:
