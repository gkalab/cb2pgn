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

#include "m_utility.h"

#include <string.h>

namespace db {

inline bool Comment::isXml() const							{ return ::strncmp(m_content, "<xml>", 5) == 0; }
inline bool Comment::isEmpty() const						{ return m_content.empty(); }
inline bool Comment::engFlag() const						{ return m_engFlag; }
inline bool Comment::othFlag() const						{ return m_othFlag; }
inline unsigned Comment::size() const						{ return m_content.size(); }
inline mstl::string const& Comment::content() const	{ return m_content; }
inline Comment::operator mstl::string const& () const	{ return m_content; }

inline bool Comment::operator==(Comment const& comment) const { return m_content == comment.m_content; }
inline bool Comment::operator!=(Comment const& comment) const { return m_content != comment.m_content; }

#if HAVE_0X_MOVE_CONSTRCUTOR_AND_ASSIGMENT_OPERATOR

inline
Comment::Comment(Comment&& comment)
	:m_content(mstl::move(comment.m_content))
	,m_engFlag(comment.m_engFlag)
	,m_othFlag(comment.m_othFlag)
{
}


inline
Comment&
Comment::operator=(Comment&& comment)
{
	m_content = mstl::move(comment.m_content);
	m_engFlag = comment.m_engFlag;
	m_othFlag = comment.m_othFlag;

	return *this;
}

#endif

} // naespace db

// vi:set ts=3 sw=3:
