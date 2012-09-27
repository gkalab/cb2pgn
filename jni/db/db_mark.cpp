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

#include "db_mark.h"

#include "u_byte_stream.h"

#include "m_stdio.h"
#include "m_assert.h"

#include <string.h>
#include <ctype.h>

using namespace db;
using namespace util;


char const*
skipSpaces(char const* s)
{
	while (isspace(*s))
		++s;
	return s;
}


static char const*
skipWord(char const* s)
{
	while (::isalpha(*s))
		++s;
	return s;
}


static Square
parseSquare(char const* s)
{
	if (s[0] < 'a' || 'h' < s[0] || s[1] < '1' || '8' < s[1])
		return sq::Null;

	return sq::make(s[0] - 'a', s[1] - '1');
}


Mark::Mark()
	:m_command(mark::None)
	,m_type(DefaultType)
	,m_text(0)
	,m_color(DefaultColor)
	,m_square1(sq::Null)
	,m_square2(sq::Null)
{
}


Mark::Mark(mark::Type type, mark::Color color, Square square, char text)
	:m_command(mark::Draw)
	,m_type(type)
	,m_text(text)
	,m_color(color)
	,m_square1(square)
	,m_square2(sq::Null)
{
	//M_REQUIRE(type == mark::Text ? mark::isTextChar(text) : text == 0);
	//M_REQUIRE(square != sq::Null);
}


Mark::Mark(mark::Type type, mark::Color color, Square square1, Square square2, char text)
	:m_command(mark::Draw)
	,m_type(type)
	,m_text(text)
	,m_color(color)
	,m_square1(square1)
	,m_square2(square2)
{
	//M_REQUIRE(type == mark::Text ? mark::isTextChar(text) : text == 0);
	//M_REQUIRE(square1 != sq::Null);
	//M_REQUIRE(square1 != square2);
}


Mark::Mark(char const* s)
	:m_command(mark::None)
	,m_type(DefaultType)
	,m_text(0)
	,m_color(DefaultColor)
	,m_square1(sq::Null)
	,m_square2(sq::Null)
{
	//M_REQUIRE(s);
	parseScidbMark(s);
}


bool
Mark::operator==(Mark const& mark) const
{
	return	m_command == mark.m_command
			&& m_type == mark.m_type
			&& m_text == mark.m_text
			&& m_color == mark.m_color
			&& m_square1 == mark.m_square1
			&& m_square2 == mark.m_square2
			&& m_caption == mark.m_caption;
}


int
Mark::compare(Mark const& m) const
{
	if (int cmp = int(m_command) - int(m.m_command)) return cmp;
	if (int cmp = int(m_type)    - int(m.m_type)   ) return cmp;
	if (int cmp = int(m_text)    - int(m.m_text)   ) return cmp;
	if (int cmp = int(m_color)   - int(m.m_color)  ) return cmp;
	if (int cmp = int(m_square1) - int(m.m_square1)) return cmp;
	if (int cmp = int(m_square2) - int(m.m_square2)) return cmp;

	return ::strcmp(m_caption, m.m_caption);
}


bool
Mark::match(Mark const& mark) const
{
	return	m_command == mark.m_command
			&& (	(m_square1 == mark.m_square1 && m_square2 == mark.m_square2)
				|| (m_square1 == mark.m_square2 && m_square2 == mark.m_square1));
}


char const*
Mark::parseDiagramMarker(char const* s)
{
	//M_REQUIRE(s);

	if (*s != '#')
		return s;

	if (s[1] == '[')
	{
		if (char const* e = ::strchr(s + 1, ']'))
		{
			m_command = mark::Diagram;
			m_caption.assign(s, e - s);
			s = e;
		}
	}

	return s + 1;
}


char const*
Mark::parseScidbMark(char const* s)
{
	//M_REQUIRE(isEmpty());
	//M_REQUIRE(s);

	if (s[0] != '[' && s[1] != '%')
		return s;

	char const* p = s + 2;

	switch (p[0])
	{
		case 'a':	// "arrow"
			m_command = mark::Draw;
			m_type = mark::Arrow;
			p = parseScidFormat(::skipSpaces(p + 6));
			break;

		case 'm':	// "mark"
			m_command = mark::Draw;
			m_type = mark::Full;
			p = parseScidFormat(::skipSpaces(p + 5));
			break;

		case 'd':	// "draw"
			m_command = mark::Draw;
			p = parsePgnFormat(::skipSpaces(p + 5));
			break;
	}

	if (*p == ']')
	{
		++p;
	}
	else
	{
		m_square1 = sq::Null;
		p = s;
	}

	return ::skipSpaces(p);
}


// Marks in (old) Scid format:
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~
// mark		::= "[%" <key> <square> [<square>] [<color>] "]"
// key		::= "mark" | "arrow"
// square	::= [a-h] [1-8]
// color		::= "red" | "orange" | "yellow" | "green" | "blue" |
// 				 "darkBlue" | "purple" | "white" | "black"
char const*
Mark::parseScidFormat(char const* s)
{
	m_square1 = ::parseSquare(s);
	s = ::skipSpaces(s + 2);

	if (::isalpha(s[0]) && ::isdigit(s[1]))
	{
		m_square2 = ::parseSquare(s);
		s = ::skipSpaces(s + 2);
	}

	if (::isalpha(*s))
		s = parseColor(s);

	return ::skipSpaces(s);
}


// Marks in proposed PGN Format:
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// mark		::= "[%" <command> [<type> | <text> ","] <square> ["," <square>] ["," <color>] "]"
// command	::= "draw"
// type		::= "full" | "square" | "arrow" | "circle" | "disk"
// text		::= [-+=?!A-Za-z0-9]
// square	::= [a-h] [1-8]
// color		::= "red" | "orange" | "yellow" | "green" | "blue" |
// 				 "darkBlue" | "purple" | "white" | "black"
char const*
Mark::parsePgnFormat(char const* s)
{
	if (!s[0])
		return s;

	if (s[1] == ',')
		s = parseText(s);
	else if (::isalpha(s[1]))
		s = parseType(s);

	m_square1 = ::parseSquare(s);
	s += 2;

	if (s[0] == ',' && ::isalpha(s[1]) && ::isdigit(s[2]))
	{
		m_square2 = ::parseSquare(s + 1);
		s += 3;
	}

	if (*s == ',')
		s = parseColor(s + 1);

	return ::skipSpaces(s);
}


char const*
Mark::parseColor(char const* s)
{
	switch (s[0])
	{
		case 'b':
			if (s[1])
			{
				switch (s[2])
				{
					case 'a': m_color = mark::Black; break;
					case 'u': m_color = mark::Blue; break;
				}
			}
			break;

		case 'd': m_color = mark::DarkBlue; break;
		case 'g': m_color = mark::Green; break;
		case 'o': m_color = mark::Orange; break;
		case 'p': m_color = mark::Purple; break;
		case 'r': m_color = mark::Red; break;
		case 'w': m_color = mark::White; break;
		case 'y': m_color = mark::Yellow; break;
	}

	return ::skipWord(s);
}


char const*
Mark::parseText(char const* s)
{
	//M_ASSERT(s[1] == ',');

	m_type = mark::Text;
	m_text = s[0];

	if (!mark::isTextChar(m_text))
		return s + ::strlen(s);

	return s + 2;
}


char const*
Mark::parseType(char const* s)
{
	switch (*s)
	{
		case 'a': m_type = mark::Arrow; break;
		case 'c': m_type = mark::Circle; break;
		case 'd': m_type = mark::Disk; break;
		case 'f': m_type = mark::Full; break;
		case 's': m_type = mark::Square; break;
	}

	s = ::skipWord(s);

	if (*s != ',')
		return s + ::strlen(s);

	return s + 1;
}


// Marks in ChessBase format:
// ~~~~~~~~~~~~~~~~~~~~~~~~~~
// mark		::= "[%" command "]"
// command	::= full | arrow
// full		::= "csl" color square
// arrow		::= "cal" color square square
// color		::= "R" | "G" | "Y"
// square	::= [a-h] [1-8]
char const*
Mark::parseChessBaseMark(char const* s, mark::Type type)
{
	//M_REQUIRE(s);

	s = ::skipSpaces(s);

	switch (*s)
	{
		case 'R':	m_color = mark::Red; break;
		case 'G':	m_color = mark::Green; break;
		case 'Y':	m_color = mark::Yellow; break;
		default:		m_color = mark::Red; break;
	}

	if ((m_square1 = ::parseSquare(++s)) == sq::Null)
		return s;
	s += 2;

	if (type == mark::Arrow)
	{
		if ((m_square2 = ::parseSquare(s)) == sq::Null)
			return s;
		s += 2;
	}

	m_type = type;
	m_command = mark::Draw;

	return ::skipSpaces(s);
}


mstl::string&
Mark::toString(mstl::string& result) const
{
	if (isEmpty())
		return result;

	if (m_command == mark::Diagram)
	{
		result += '#';

		if (!m_caption.empty())
		{
			result += '[';
			result += m_caption;
			result += ']';
		}
	}
	else
	{
		result += "[%";
		result += mark::commandName(m_command);
		result += ' ';

		if (m_text)
			result += m_text;
		else
			result += mark::typeName(m_type);

		result += ',';
		result += sq::printAlgebraic(m_square1);

		if (m_square2 != sq::Null)
		{
			result += ',';
			result += sq::printAlgebraic(m_square2);
		}

		if (m_color != DefaultColor)
		{
			result += ',';
			result += mark::colorName(m_color);
		}

		result += "]";
	}

	return result;
}


mstl::string&
Mark::print(mstl::string& result) const
{
	if (isEmpty())
		return result;

	result += mark::commandName(m_command);
	result += ',';

	if (m_command == mark::Diagram)
	{
		result += m_caption;
	}
	else
	{
		if (m_text)
			result += m_text;
		else
			result += mark::typeName(m_type);
		result += ',';
		result += sq::printAlgebraic(m_square1);
		result += ',';
		if (m_square2 != sq::Null)
			result += sq::printAlgebraic(m_square2);
		result += ',';
		result += mark::colorName(m_color);
	}

	return result;
}


void
Mark::dump()
{
	mstl::string s;
	print(s);
	printf("%s\n", s.c_str());
}


util::crc::checksum_t
Mark::computeChecksum(util::crc::checksum_t crc) const
{
	crc = ::util::crc::compute(crc, Byte(m_command));
	crc = ::util::crc::compute(crc, Byte(m_type));
	crc = ::util::crc::compute(crc, uint16_t(m_color));
	crc = ::util::crc::compute(crc, Byte(m_square1));
	crc = ::util::crc::compute(crc, Byte(m_square2));
	crc = ::util::crc::compute(crc, m_caption.c_str(), m_caption.size());

	return crc;
}


// Binary format:
// ~~~~~~~~~~~~~~
// mark ::= <command> <square> (<type> | <text>) [<square-2>] <color>		(5 Bytes)
// mark ::= "#" <caption>																(>= 2 Bytes)
//
// command			1 Byte
// square			1 Byte	low = rank, high = fyle
// type or text	1 Byte	we assume text >= 0x20 (if type is Text)
// square-2			1 Byte	high bit is 1, subtract high bit for square value
// color				1 Byte	high bit is 0 (thus color < 128)
void
Mark::encode(ByteStream& strm) const
{
	//M_REQUIRE(!isEmpty());

	strm.put(m_command);

	if (m_command == mark::Diagram)
	{
		strm.put(m_caption, m_caption.size() + 1);
	}
	else
	{
		//M_ASSERT(m_color < 0x80);
		//M_ASSERT(m_type < 0x20);

		strm.put(m_square1);
		strm.put(m_type == mark::Text ? m_text : char(m_type));

		if (m_square2 != sq::Null)
			strm.put(m_square2 | 0x80);

		strm.put(m_color);
	}
}


void
Mark::decode(ByteStream& strm)
{
	//M_REQUIRE(isEmpty());

	m_command = mark::Command(strm.get());

	if (m_command == mark::None)
		return;

	if (m_command == mark::Diagram)
	{
		strm.get(m_caption);
	}
	else
	{
		m_square1 = Square(strm.get());
		Byte byte = strm.get();

		if (byte >= 0x20)
		{
			m_type = mark::Text;
			m_text = byte;
		}
		else
		{
			m_type = mark::Type(byte);
		}

		byte = strm.get();

		if (byte & 0x80)
		{
			m_square2 = byte & 63;
			m_color = mark::Color(strm.get());
		}
		else
		{
			m_color = mark::Color(byte);
		}
	}
}

// vi:set ts=3 sw=3:
