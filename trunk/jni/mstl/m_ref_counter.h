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

#ifndef _mstl_ref_counter_included
#define _mstl_ref_counter_included

#include "m_utility.h"

namespace mstl {

struct ref_counter : private mstl::noncopyable
{
public:

	// structors
	ref_counter();

	// queries
	bool unique() const;
	bool expired() const;

	// accessors
	unsigned use_count() const;

	// modifiers
	void ref();
	bool release();

protected:

	// structors
	ref_counter(ref_counter const& counter);

private:

	// attributes
	unsigned m_count;
};

} // namespace mstl

#include "m_ref_counter.ipp"

#endif // _mstl_ref_counter_included

// vi:set ts=3 sw=3:
