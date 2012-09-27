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

#ifndef _db_move_info_included
#define _db_move_info_included

#include "db_date.h"
#include "db_clock.h"

#include "u_crc.h"

#include "m_string.h"

namespace util { class ByteStream; }

namespace db {

class EngineList;

class MoveInfo
{
public:

	enum Type
	{
		None,
		Evaluation,
		PlayersClock,
		ElapsedGameTime,
		ElapsedMoveTime,
		MechanicalClockTime,
		DigitalClockTime,
		CorrespondenceChessSent,
	};

	enum Format
	{
		Pgn,
		Text,
	};

	MoveInfo();

	bool isEmpty() const;
	bool hasTimeInfo() const;
	bool hasEvaluationInfo() const;

	Type content() const;

	Clock const& clock() const;
	Date const& date() const;

	unsigned engine() const;
	unsigned depth() const;
	unsigned pawns() const;
	unsigned centipawns() const;
	float value() const;

	::util::crc::checksum_t computeChecksum(EngineList const& engines, util::crc::checksum_t crc) const;
	int compare(MoveInfo const& info) const;

	void setAnalysisEngine(unsigned engine);
	void setClockEngine(unsigned engine);

	void print(EngineList const& engines, mstl::string& result, Format format = Pgn) const;
	void clear();

	void decode(util::ByteStream& strm);
	void encode(util::ByteStream& strm) const;

	char const* parseCorrespondenceChessSent(char const* s);
	char const* parsePlayersClock(char const* s);
	char const* parseDigitalClockTime(char const* s);
	char const* parseMechanicalClockTime(char const* s);
	char const* parseElapsedGameTime(char const* s);
	char const* parseElapsedMoveTime(char const* s);
	char const* parseEvaluation(char const* s);

private:

	char const* parseTime(Type type, char const* s);

	struct TimeInfo
	{
		Clock	m_clock;
		Date	m_date;
	};

	struct AnalysisInfo
	{
		AnalysisInfo();

		uint8_t m_depth;
		uint8_t m_sign;
		uint8_t m_pawns;
		uint8_t m_centipawns;
	};

	Type		m_content;
	uint8_t	m_engine;

#if HAVE_0X_UNRESTRICTED_UNIONS
	union
	{
#endif

		TimeInfo			m_time;
		AnalysisInfo	m_analysis;

#if HAVE_0X_UNRESTRICTED_UNIONS
	};
#endif
};

} // namespace db

namespace mstl {

template <typename T> struct is_pod;

template <>
struct is_pod<db::MoveInfo>
{
	enum { value = is_pod<db::Clock>::value && is_pod<db::Date>::value };
};

} // namespace mstl

#include "db_move_info.ipp"

#endif // _db_move_info_included

// vi:set ts=3 sw=3:
