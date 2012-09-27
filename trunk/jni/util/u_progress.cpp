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

#include "u_progress.h"

#include "m_utility.h"
#include "m_limits.h"

using namespace util;


Progress::Progress() : m_freq(0) {}
Progress::~Progress() throw() {}

void Progress::start(unsigned) {}
void Progress::message(mstl::string const&) {}
void Progress::tick(unsigned) {}
void Progress::update(unsigned) {}
void Progress::finish() throw() {}
bool Progress::interrupted() { return false; }
unsigned Progress::ticks() const { return 0; }


unsigned
Progress::frequency(unsigned count, unsigned maximum) const
{
	unsigned ticks = this->ticks();

	if (ticks > 0)
		return mstl::max(1u, maximum ? mstl::min(maximum, count/ticks) : count/ticks);

	return m_freq ? mstl::min(maximum, m_freq) : maximum;
}


void
Progress::setCount(unsigned count)
{
	unsigned ticks = this->ticks();

	if (ticks > 0)
		setFrequency(mstl::max(1u, count/ticks));
}

// vi:set ts=3 sw=3:
