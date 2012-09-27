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

inline bool ifstream::is_open() const			{ return bits::file::is_open(); }
inline bool ifstream::is_buffered() const		{ return bits::file::is_buffered(); }
inline bool ifstream::is_unbuffered() const	{ return bits::file::is_unbuffered(); }
inline void ifstream::close()		{ bits::file::close(); }

inline void ifstream::set_unbuffered()					{ bits::file::set_unbuffered(); }
inline void ifstream::set_binary()						{ bits::file::set_binary(); }
inline void ifstream::set_text()							{ bits::file::set_text(); }
inline void ifstream::set_bufsize(unsigned size)	{ bits::file::set_bufsize(size); }

inline int64_t ifstream::size() const		{ return bits::file::size(); }
inline unsigned ifstream::bufsize() const	{ return bits::file::bufsize(); }
inline char* ifstream::buffer() const		{ return bits::file::buffer(); }
inline uint64_t ifstream::mtime()			{ return bits::file::mtime(); }

inline mstl::string const& ifstream::filename() const { return bits::file::filename(); }

} // namespace mstl

// vi:set ts=3 sw=3:
