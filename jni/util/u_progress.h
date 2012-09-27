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

#ifndef _u_progress_included
#define _u_progress_included

namespace mstl { class string; }

namespace util {

class Progress
{
public:

	Progress();
	virtual ~Progress() throw();

	virtual bool interrupted();

	unsigned frequency() const;
	unsigned frequency(unsigned count, unsigned maximum = 0) const;
	virtual unsigned ticks() const;

	virtual void start(unsigned total);
	virtual void message(mstl::string const& msg);
	virtual void tick(unsigned count);
	virtual void update(unsigned progress);
	virtual void finish() throw();

	void setFrequency(unsigned frequency);
	void setCount(unsigned count);

private:

	unsigned m_freq;
};


class ProgressWatcher
{
public:

	ProgressWatcher(Progress& progress, unsigned total);
	ProgressWatcher(Progress* progress, unsigned total);
	~ProgressWatcher();

private:

	Progress* m_progress;
};

} // namespace util

#include "u_progress.ipp"

#endif // _u_progress_included

// vi:set ts=3 sw=3:
