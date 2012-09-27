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

#include "db_move_info.h"
#include "db_engine_list.h"
#include "db_exception.h"
#include "db_common.h"

#include "u_byte_stream.h"

#include "m_utility.h"
#include "m_assert.h"

#include <ctype.h>
#include <stdlib.h>

using namespace db;
using namespace util;

typedef ByteStream::uint24_t uint24_t;


inline
static char const*
skipSpaces(char const* s)
{
	while (*s == ' ')
		++s;

	return s;
}


MoveInfo::AnalysisInfo::AnalysisInfo()
	:m_depth(0)
	,m_pawns(0)
	,m_centipawns(0)
{
}


MoveInfo::MoveInfo()
	:m_content(None)
	,m_engine(0)
{
}


int
MoveInfo::compare(MoveInfo const& mi) const
{
	if (int cmp = int(m_content) - int(mi.m_content))
		return cmp;

	switch (m_content)
	{
		case None:
			return 0;

		case CorrespondenceChessSent:
			if (int cmp = m_time.m_date.compare(mi.m_time.m_date))
				return cmp;
			// fallthru

		case PlayersClock:
		case ElapsedGameTime:
		case ElapsedMoveTime:
		case MechanicalClockTime:
		case DigitalClockTime:
			if (int cmp = m_time.m_clock.compare(mi.m_time.m_clock))
				return cmp;
			break;

		case Evaluation:
			if (int cmp = int(m_analysis.m_depth)      - int(mi.m_analysis.m_depth)     ) return cmp;
			if (int cmp = int(m_analysis.m_sign)       - int(mi.m_analysis.m_sign)      ) return cmp;
			if (int cmp = int(m_analysis.m_pawns)      - int(mi.m_analysis.m_pawns)     ) return cmp;
			if (int cmp = int(m_analysis.m_centipawns) - int(mi.m_analysis.m_centipawns)) return cmp;
			break;
	}

	return 0; // never reached
}


void
MoveInfo::setAnalysisEngine(unsigned engine)
{
	//M_REQUIRE(content() == Evaluation || content() == ElapsedMoveTime || content() == PlayersClock);

	if (m_content == PlayersClock)
		m_content = ElapsedMoveTime;

	m_engine = engine;
}


void
MoveInfo::setClockEngine(unsigned engine)
{
	//M_REQUIRE(content() == PlayersClock);

	if (engine)
	{
		m_engine = engine;
		m_content = ElapsedMoveTime;
	}
}


char const*
MoveInfo::parseTime(Type type, char const* s)
{
	//M_REQUIRE(s);

	char const* e = m_time.m_clock.parse(s);

	if (e)
		m_content = type;

	return e;
}


char const*
MoveInfo::parseCorrespondenceChessSent(char const* s)
{
	//M_REQUIRE(s);

	char const* e = (s = ::skipSpaces(s));

	while (::isdigit(*e) || *e == '.')
		++e;

	if (!m_time.m_date.parseFromString(s, e - s))
		return 0;

	if (*(s = ::skipSpaces(e)) != ',')
		return s;

	s = m_time.m_clock.parse(::skipSpaces(s + 1));

	if (s == 0)
		return 0;

	m_content = CorrespondenceChessSent;

	return s ? ::skipSpaces(s) : 0;
}


char const*
MoveInfo::parseEvaluation(char const* s)
{
	//M_REQUIRE(s);

	char* e = 0;

	s = ::skipSpaces(s);

	if (::isdigit(*s))
	{
		m_analysis.m_depth = ::strtoul(s, &e, 10);

		if (*e != ':')
			return 0;

		s = e + 1;
	}
	else
	{
		m_analysis.m_depth = 0;
	}

	switch (*s)
	{
		case '+':	m_analysis.m_sign = 0; break;
		case '-':	m_analysis.m_sign = 1; break;
		default:		return 0;
	}

	unsigned pawns = ::strtoul(++s, &e, 10);

	if (s == e || *e != '.')
		return 0;

	s = e + 1;

	unsigned centipawns = ::strtoul(s, &e, 10);

	if (s == e)
		return 0;

	m_analysis.m_pawns = mstl::min(pawns, (1u << 11) - 1);
	m_analysis.m_centipawns = mstl::min(centipawns, 99u);

	switch (*e)
	{
		case '|':
			if (::isdigit(e[1]))
				m_analysis.m_depth = ::strtoul(e + 1, &e, 10);
			else if (e[1] == 'd' && ::isdigit(e[2]))
				m_analysis.m_depth = ::strtoul(e + 2, &e, 10);
			break;

		case '/':
			if (::isdigit(e[1]))
				m_analysis.m_depth = ::strtoul(e + 1, &e, 10);
			break;
	}

	m_content = Evaluation;

	return ::skipSpaces(e);
}


util::crc::checksum_t
MoveInfo::computeChecksum(EngineList const& engines, util::crc::checksum_t crc) const
{
	crc = ::util::crc::compute(crc, Byte(m_content));

	switch (m_content)
	{
		case None:
			break;

		case CorrespondenceChessSent:
			crc = m_time.m_date.computeChecksum(crc);
			// fallthru

		case PlayersClock:
		case ElapsedGameTime:
		case ElapsedMoveTime:
		case MechanicalClockTime:
		case DigitalClockTime:
			crc = m_time.m_clock.computeChecksum(crc);
			break;

		case Evaluation:
			crc = ::util::crc::compute(crc, Byte(m_analysis.m_depth));
			crc = ::util::crc::compute(crc, Byte(m_analysis.m_sign));
			crc = ::util::crc::compute(crc, uint16_t(m_analysis.m_pawns));
			crc = ::util::crc::compute(crc, Byte(m_analysis.m_centipawns));
			break;
	}

	if (m_engine)
	{
		mstl::string const& engine = engines.engine(m_engine);
		crc = ::util::crc::compute(crc, engine, engine.size());
	}

	return crc;
}


void
MoveInfo::print(EngineList const& engines, mstl::string& result, Format format) const
{
	static char const* Identifiers[8] = { "", "clk", "egt", "emt", "mct", "ct", "", "", };

	if (m_engine)
	{
		switch (int(m_content))
		{
			case ElapsedMoveTime:
			case Evaluation:
				result.append(engines[m_engine - 1]);
				result.append(' ');
				format = Text;	// cannot use PGN format
				break;
		}
	}

	switch (m_content)
	{
		case None:
			return;

		case PlayersClock:
		case ElapsedGameTime:
		case ElapsedMoveTime:
		case MechanicalClockTime:
		case DigitalClockTime:
			switch (format)
			{
				case Pgn:	result.format("[%%%s ", Identifiers[m_content]); break;
				case Text:	result.append('(');
			}
			result.format(	"%u:%02u:%02u",
								m_time.m_clock.hour(),
								m_time.m_clock.minute(),
								m_time.m_clock.second());
			break;

		case CorrespondenceChessSent:
			switch (format)
			{
				case Pgn:	result.append("[%ccsnt ", 8); break;
				case Text:	result.append('('); break;
			}
			result.format(	"%04u.%02u:%02u",
								m_time.m_date.year(),
								m_time.m_date.month(),
								m_time.m_date.day());
			if (!m_time.m_clock.isEmpty())
			{
				result.format(",%u", m_time.m_clock.hour());
				if (m_time.m_clock.minute())
					result.format(":%02u", m_time.m_clock.minute());
				if (m_time.m_clock.second())
					result.format(":%02u", m_time.m_clock.second());
			}
			break;

		case Evaluation:
			if (format == Pgn)
				result.append("[%eval ", 7);
			if (m_analysis.m_depth)
				result.format("%u:", m_analysis.m_depth);
			result.format(	"%c%u.%02u",
								m_analysis.m_sign ? '-' : '+',
								unsigned(m_analysis.m_pawns),
								m_analysis.m_centipawns);
			if (format == Text)
				return;
			break;
	}

	switch (format)
	{
		case Pgn:	result.append(']'); break;
		case Text:	result.append(')'); break;
	}
}


void
MoveInfo::decode(ByteStream& strm)
{
	//M_REQUIRE(strm.peek() & 0x80);

	uint8_t	u = strm.get();
	uint32_t	v;

	switch (m_content = Type((u >> 4) & 0x07))
	{
		case None:	// should not happen
			break;

		case CorrespondenceChessSent:
			v = strm.uint32();
			m_time.m_date.setYMD(	((v >>  4) & 0x0fff),
											((v      ) & 0x000f),
											((u      ) & 0x000f));

			m_time.m_clock.setHMS(	((v >> 16) & 0x000f),
											((v >> 20) & 0x003f),
											((v >> 26) & 0x003f));
			break;

		case PlayersClock:
		case ElapsedGameTime:
		case ElapsedMoveTime:
		case MechanicalClockTime:
		case DigitalClockTime:
			v = strm.uint16();
			m_engine = u & 0x0f;
			m_time.m_clock.setHMS(	((v      ) & 0x000f),
											((v >>  4) & 0x003f),
											((v >> 10) & 0x003f));
			break;

		case Evaluation:
			v = strm.uint24();
			m_engine = u & 0x0f;
			m_analysis.m_depth      = ((v      ) & 0x001f);
			m_analysis.m_sign       = ((v >>  5) & 0x0001);
			m_analysis.m_pawns      = ((v >>  6) & 0x07ff);
			m_analysis.m_centipawns = ((v >> 17) & 0x007f);
			break;

		default:
			return;
			//IO_RAISE(Game, Corrupted, "corrupted game data");
	}
}


void
MoveInfo::encode(ByteStream& strm) const
{
	switch (m_content)
	{
		case None: // should not happen
			break;

		case CorrespondenceChessSent:
			strm << uint8_t(	0x80
								 | (uint8_t(m_content) << 4)									//  4 bit
								 | (uint8_t(m_time.m_date.day() & 0x0f))					//  4 bit
			);
			strm << uint32_t(	(uint32_t(m_time.m_date.month() & 0x0fff))			//  4 bit
								 | (uint32_t(m_time.m_date.year() & 0x0fff) << 4)		// 12 bit
								 | (uint32_t(m_time.m_clock.hour() & 0x000f) << 16)	//  4 bit
								 | (uint32_t(m_time.m_clock.minute() & 0x003f) << 20)	//  6 bit
								 | (uint32_t(m_time.m_clock.second() & 0x003f) << 26)	//  6 bit
			);
			break;

		case PlayersClock:
		case ElapsedGameTime:
		case ElapsedMoveTime:
		case MechanicalClockTime:
		case DigitalClockTime:
			strm << uint8_t(	0x80
								 | (uint8_t(m_content) << 4)									//  4 bit
								 | (uint8_t(m_engine & 0x0f))									//  4 bit
			);
			strm << uint16_t(	(uint32_t(m_time.m_clock.hour() & 0x000f))			//  4 bit
								 | (uint32_t(m_time.m_clock.minute() & 0x003f) << 4)	//  6 bit
								 | (uint32_t(m_time.m_clock.second() & 0x003f) << 10)	//  6 bit
			);
			break;

		case Evaluation:
			strm << uint8_t(	0x80
								 | (uint8_t(m_content) << 4)									//  4 bit
								 | (uint8_t(m_engine & 0x0f))									//  4 bit
			);
			strm << uint24_t(	(uint32_t(m_analysis.m_depth & 0x001f))				//  5 bit
								 | (uint32_t(m_analysis.m_sign & 0x0001) << 5)			//  1 bit
								 | (uint32_t(m_analysis.m_pawns & 0x07ff) << 6)			// 11 bit
								 | (uint32_t(m_analysis.m_centipawns & 0x007f) << 17)	//  7 bit
			);
			break;
	}
}

// vi:set ts=3 sw=3:
