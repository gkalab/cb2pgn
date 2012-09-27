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

extern "C" {
#include "extrastdio.h"
}
#include "m_sstream.h"
#include "m_stdio.h"
#include "m_assert.h"

#include <stdio.h>
#include <stdlib.h>

using namespace mstl;


ostringstream::ostringstream()
	:m_buf(0)
	,m_size(0)
{
	if ((m_fp = open_memstream(&m_buf, &m_size)) == 0)
		{};
		//M_RAISE("open_memstream() failed");

	setmode(out | binary);
}


ostringstream::~ostringstream() throw()
{
	::fclose(m_fp);
	::free(m_buf);
	m_fp = 0;
}


string
ostringstream::str() const
{
	::fflush(m_fp);
	return m_buf ? string(m_buf, m_size) : string::empty_string;
}


void
ostringstream::str(mstl::string const& s)
{
	if (m_fp)
	{
		::fclose(m_fp);
		::free(m_buf);

		m_fp = 0;
		m_buf = 0;
		m_size = 0;
	}

	if ((m_fp = open_memstream(&m_buf, &m_size)) == 0)
		{};
		//M_RAISE("open_memstream() failed");

	setmode(out | binary);
	write(s);
}


istringstream::istringstream(string const& str)
	:m_buf(str)
{
	if ((m_fp = fmemopen(m_buf.data(), m_buf.size(), "r")) == 0)
		{};
		//M_RAISE("fmemopen() failed");

	setmode(in | binary);
}


istringstream::~istringstream() throw()
{
	::fclose(m_fp);
	m_fp = 0;
}


void
istringstream::str(string const& str)
{
	m_buf = str;
	::fclose(m_fp);

	if ((m_fp = fmemopen(m_buf.data(), m_buf.size(), "r")) == 0)
		{};
		//M_RAISE("fmemopen() failed");
}

// vi:set ts=3 sw=3:
