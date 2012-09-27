// ======================================================================
// Author : $Author$
// Version: $Revision$
// Date   : $Date$
// Url    : $URL$
// ======================================================================

// ======================================================================
// Copyright: (C) 2012 Gregor Cramer
// ======================================================================

// ======================================================================
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
// ======================================================================

#ifndef _u_html_included
#define _u_html_included

#include "m_string.h"
#include "m_vector.h"

extern "C" { struct XML_ParserStruct; }

namespace hyphenate { class Hyphenator; }

namespace util {
namespace html {

class Search
{
public:

	Search(bool noCase, bool entireWord, bool titleOnly, unsigned maxMatches);
	~Search();

	bool tooManyMatches() const;

	unsigned countMatches() const;
	unsigned matchPosition(unsigned i) const;
	mstl::string const& title() const;

	bool parse(char const* document, unsigned length, char const* search, unsigned searchLen);

private:

	int stringFirstCmd(char const* haystack, unsigned haystackLen);
	int stringPartialMatch(char const* haystack, unsigned haystackLen);
	void addPosition(unsigned pos);

	static void htmlContent(void* cbData, char const* s, int len);
	static void htmlStartElement(void* cbData, char const* elem, char const** attr);
	static void htmlEndElement(void* cbData, char const* elem);

	typedef int (*Compare)(char const*, unsigned, char const*, unsigned);
	typedef mstl::vector<unsigned> PosList;

	XML_ParserStruct*	m_parser;
	char const*			m_needle;
	char const*			m_haystack;
	unsigned				m_length;
	Compare				m_compare;
	bool					m_entireWord;
	bool					m_titleOnly;
	bool					m_isTitle;
	bool					m_tooManyMatches;
	int					m_skip;
	unsigned				m_maxMatches;
	unsigned				m_lastBytePos;
	unsigned				m_lastCharPos;
	unsigned				m_partialMatch;
	unsigned				m_partialMatchPos;
	PosList				m_posList;
	mstl::string		m_title;
};


class Hyphenate
{
public:

	enum CacheState { KeepInCache, DontKeepInCache };

	Hyphenate(	mstl::string const& patternFilename,
					mstl::string const& dictFilenames,
					CacheState keepInCache = DontKeepInCache);
	~Hyphenate();

	bool parse(char const* document, unsigned length);
	mstl::string const& result() const;

	static void clearCache(mstl::string const& patternFilename = mstl::string::empty_string);

private:

	typedef hyphenate::Hyphenator Hyphenator;

	Hyphenator*		m_hyphenator;
	mstl::string	m_filename;
	CacheState		m_keepInCache;
	mstl::string	m_result;
};


class BuildLigatures
{
public:

	bool parse(char const* document, unsigned length);
	mstl::string const& result() const;

private:

	mstl::string m_result;
};

} // namespace html
} // namespace util

#include "u_html.ipp"

#endif // _u_html_included

// vi:set ts=3 sw=3:
