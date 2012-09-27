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

#ifndef _db_comment_included
#define _db_comment_included

#include "db_common.h"

#include "u_crc.h"

#include "m_string.h"
#include "m_map.h"

namespace db {

class Comment
{
public:

	struct Callback
	{
		enum Attribute
		{
			Bold			= 'b',
			Italic		= 'i',
			Underline	= 'u',
		};

		virtual ~Callback() throw();

		virtual void start() = 0;
		virtual void finish() = 0;

		virtual void startLanguage(mstl::string const& lang) = 0;
		virtual void endLanguage(mstl::string const& lang) = 0;

		virtual void startAttribute(Attribute attr) = 0;
		virtual void endAttribute(Attribute attr) = 0;

		virtual void content(mstl::string const& s) = 0;
		virtual void nag(mstl::string const& s) = 0;
		virtual void symbol(char s) = 0;

		virtual void invalidXmlContent(mstl::string const& content) = 0;
	};

	typedef mstl::map<mstl::string,unsigned> LanguageSet;

	Comment();
	Comment(mstl::string const& content, bool engFlag, bool othFlag);

#if HAVE_OX_EXPLICITLY_DEFAULTED_AND_DELETED_SPECIAL_MEMBER_FUNCTIONS
	Comment(Comment const&) = default;
	Comment& operator=(Comment const&) = default;
#endif

#if HAVE_0X_MOVE_CONSTRCUTOR_AND_ASSIGMENT_OPERATOR
	Comment(Comment&& comment);
	Comment& operator=(Comment&& comment);
#endif

	operator mstl::string const& () const;

	bool operator==(Comment const& comment) const;
	bool operator!=(Comment const& comment) const;

	bool isEmpty() const;
	bool isXml() const;
	bool engFlag() const;
	bool othFlag() const;
	bool containsLanguage(mstl::string const& lang) const;

	unsigned size() const;
	unsigned length() const;
	mstl::string const& content() const;
	util::crc::checksum_t computeChecksum(util::crc::checksum_t crc) const;

	void append(Comment const& comment, char delim = '\0');
	void remove(mstl::string const& lang);
	void strip(LanguageSet const& set);
	bool fromHtml(mstl::string const& s);
	void swap(Comment& comment);
	void swap(mstl::string& content, bool engFlag, bool othFlag);
	void copy(mstl::string const& fromLang, mstl::string const& toLang, bool stripOriginal = false);
	void normalize(char delim = '\n');
	void clear();

	void parse(Callback& cb) const;
	void collectLanguages(LanguageSet& result) const;
	void flatten(mstl::string& result, encoding::CharSet encoding) const;
	void toHtml(mstl::string& result) const;

	unsigned countLength(mstl::string const& lang) const;
	unsigned countLength(LanguageSet const& set) const;

	static bool convertCommentToXml(	mstl::string const& comment,
												Comment& result,
												encoding::CharSet encoding);

private:

	bool operator==(mstl::string const& comment) const; // avoid this usage
	bool operator!=(mstl::string const& comment) const; // avoid this usage

	void collect() const;

	mstl::string m_content;

	mutable bool m_engFlag;
	mutable bool m_othFlag; // other language than en (but not ALL)
	mutable LanguageSet m_languageSet;
};

} // namespace db

#include "db_comment.ipp"

#endif // _db_comment_included

// vi:set ts=3 sw=3:
