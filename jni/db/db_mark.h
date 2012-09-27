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

#ifndef _db_mark_included
#define _db_mark_included

#include "db_common.h"

#include "u_crc.h"

#include "m_string.h"

namespace util { class ByteStream; }

namespace db {

class Mark
{
public:

	static mark::Type const DefaultType		= mark::Full;
	static mark::Color const DefaultColor	= mark::Red;

	Mark();
	Mark(mark::Type type, mark::Color color, Square square, char text = '\0');
	Mark(mark::Type type, mark::Color color, Square square1, Square square2, char text = '\0');
	explicit Mark(char const* s);

#if HAVE_OX_EXPLICITLY_DEFAULTED_AND_DELETED_SPECIAL_MEMBER_FUNCTIONS
	Mark(Mark const&) = default;
	Mark& operator=(Mark const&) = default;
#endif

#if HAVE_0X_MOVE_CONSTRCUTOR_AND_ASSIGMENT_OPERATOR
	Mark(Mark&& mark);
	Mark& operator=(Mark&& mark);
#endif

	bool operator==(Mark const& mark) const;

	bool isEmpty() const;
	bool match(Mark const& mark) const;

	mark::Type type() const;
	char text() const;
	mark::Color color() const;
	Square square(unsigned index = 0) const;

	::util::crc::checksum_t computeChecksum(util::crc::checksum_t crc) const;
	int compare(Mark const& mark) const;

	void clear();

	char const* parseDiagramMarker(char const* s);
	char const* parseScidbMark(char const* s);
	char const* parseChessBaseMark(char const* s, mark::Type type);
	mstl::string& toString(mstl::string& result) const;
	mstl::string& print(mstl::string& result) const;

	void decode(util::ByteStream& strm);
	void encode(util::ByteStream& strm) const;

	void dump();

private:

	char const* parsePgnFormat(char const* s);
	char const* parseScidFormat(char const* s);

	char const* parseColor(char const* s);
	char const* parseText(char const* s);
	char const* parseType(char const* s);

	mark::Command	m_command;
	mark::Type		m_type;
	char				m_text;
	mark::Color		m_color;
	Square			m_square1;
	Square			m_square2;
	mstl::string	m_caption;
};

} // namespace db

#include "db_mark.ipp"

#endif // _db_mark_included

// vi:set ts=3 sw=3:
