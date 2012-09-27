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

#include "m_exception.h"

namespace util {

inline unsigned Progress::frequency() const { return m_freq; }
inline void Progress::setFrequency(unsigned frequency) { m_freq = frequency; }


inline
ProgressWatcher::ProgressWatcher(Progress& progress, unsigned total)
	:m_progress(&progress)
{
	m_progress->start(total);
}


inline
ProgressWatcher::ProgressWatcher(Progress* progress, unsigned total)
	:m_progress(progress)
{
	if (m_progress)
		m_progress->start(total);
}


inline ProgressWatcher::~ProgressWatcher()
{
	if (m_progress)// && !mstl::uncaught_exception() && !m_progress->interrupted())
		m_progress->finish();
}

} // namespace util

// vi:set ts=3 sw=3:
