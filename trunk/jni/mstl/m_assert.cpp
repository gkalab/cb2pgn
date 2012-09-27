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

#include "m_assert.h"

using namespace mstl;


precondition_violation_exception::precondition_violation_exception(char const* what)
	:exception("precondition violation: %s", what)
{
}


assertion_failure_exception::assertion_failure_exception(char const* what)
	:exception("assertion failed: %s", what)
{
}

// vi:set ts=3 sw=3:
