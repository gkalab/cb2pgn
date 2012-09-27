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

#ifndef _mstl_sstream_included
#define _mstl_sstream_included

#include "m_ostream.h"
#include "m_istream.h"
#include "m_string.h"

namespace mstl {

class ostringstream : public ostream
{
public:

	ostringstream();
	~ostringstream() throw();

	string str() const;

	void str(mstl::string const& s);

private:

	ostringstream(ostringstream const&);
	ostringstream& operator=(ostringstream const&);

	char*		m_buf;
	size_t	m_size;
};


class istringstream : public istream
{
public:

	istringstream(string const& str = string::empty_string);
	~istringstream() throw();

	string str() const;
	void str(string const& str);

private:

	string m_buf;
};

} // namespace mstl

#endif // _mstl_sstream_included

// vi:set ts=3 sw=3:
