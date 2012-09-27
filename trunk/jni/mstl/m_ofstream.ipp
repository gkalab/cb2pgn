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

inline bool ofstream::is_open() const			{ return bits::file::is_open(); }
inline bool ofstream::is_buffered() const		{ return bits::file::is_buffered(); }
inline bool ofstream::is_unbuffered() const	{ return bits::file::is_unbuffered(); }

inline void ofstream::close()	{ bits::file::close(); }

inline void ofstream::set_unbuffered()					{ bits::file::set_unbuffered(); }
inline void ofstream::set_binary()						{ bits::file::set_binary(); }
inline void ofstream::set_text()							{ bits::file::set_text(); }
inline void ofstream::set_bufsize(unsigned size)	{ bits::file::set_bufsize(size); }

inline unsigned ofstream::bufsize() const	{ return bits::file::bufsize(); }
inline char* ofstream::buffer() const		{ return bits::file::buffer(); }
inline uint64_t ofstream::mtime()			{ return bits::file::mtime(); }

inline mstl::string const& ofstream::filename() const { return bits::file::filename(); }

} // namespace mstl

// vi:set ts=3 sw=3:
