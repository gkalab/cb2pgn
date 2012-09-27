// ======================================================================
// Author : $Author$
// Version: $Revision$
// Date   : $Date$
// Url    : $URL$
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

namespace mstl {

inline ios_base::ios_base() : m_fp(0), m_state(0), m_except(0), m_openmode(0) {}

inline bool ios_base::good() const					{ return m_state == 0; }
inline bool ios_base::eof() const					{ return m_state & eofbit; }
inline bool ios_base::fail() const					{ return m_state & (badbit | failbit); }
inline bool ios_base::bad() const					{ return m_state & badbit; }
inline bool ios_base::operator!() const			{ return fail(); }
inline void ios_base::setstate(iostate state)	{ clear(m_state | state); }
inline void ios_base::setmode(openmode mode)		{ m_openmode = mode; }
inline int  ios_base::fdir(seekdir dir) const	{ return int(dir); }

inline ios_base::iostate ios_base::rdstate() const		{ return m_state; }
inline ios_base::iostate ios_base::exceptions() const	{ return m_except; }
inline ios_base::openmode ios_base::mode() const		{ return m_openmode; }

inline ios_base::operator void*() const { return fail() ? 0 : const_cast<ios_base*>(this); }

} // namespace mstl

// vi:set ts=3 sw=3:
