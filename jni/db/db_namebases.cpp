// ======================================================================
// Author : $Author$
// Version: $Revision$
// Date   : $Date$
// Url    : $URL$
// ======================================================================

// ======================================================================
//    _/|            __
//   // o\         /    )           ,        /    /
//   || ._)    ----\---------__----------__-/----/__-
//   //__\          \      /   '  /    /   /    /   )
//   )___(     _(____/____(___ __/____(___/____(___/_
// ======================================================================

// ======================================================================
// Copyright: (C) 2010-2012 Gregor Cramer
// ======================================================================

// ======================================================================
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
// ======================================================================

#include "db_namebases.h"

#include "m_assert.h"

using namespace db;


Namebases::Namebases()
	:m_player(Namebase::Player)
	,m_site(Namebase::Site)
	,m_event(Namebase::Event)
	,m_annotator(Namebase::Annotator)
	,m_round(Namebase::Round)
{
	// M_ASSERT(&m_player + Namebase::Player    == &m_player   );
	// M_ASSERT(&m_player + Namebase::Site      == &m_site     );
	// M_ASSERT(&m_player + Namebase::Event     == &m_event    );
	// M_ASSERT(&m_player + Namebase::Annotator == &m_annotator);
	// M_ASSERT(&m_player + Namebase::Round     == &m_round);
}


bool
Namebases::isModified() const
{
	return	m_player.isModified()
			|| m_site.isModified()
			|| m_event.isModified()
			|| m_annotator.isModified()
			|| m_round.isModified();
}


bool
Namebases::isChanged() const
{
	return	m_player.isChanged()
			|| m_site.isChanged()
			|| m_event.isChanged()
			|| m_annotator.isChanged()
			|| m_round.isChanged();
}


bool
Namebases::isOriginal() const
{
	return	m_player.isOriginal()
			&& m_site.isOriginal()
			&& m_event.isOriginal()
			&& m_annotator.isOriginal()
			&& m_round.isOriginal();
}


void
Namebases::setReadonly(bool flag)
{
	m_player.setReadonly(flag);
	m_site.setReadonly(flag);
	m_event.setReadonly(flag);
	m_annotator.setReadonly(flag);
	m_round.setReadonly(flag);
}


void
Namebases::setModified(bool flag)
{
	m_player.setModified(flag);
	m_site.setModified(flag);
	m_event.setModified(flag);
	m_annotator.setModified(flag);
	m_round.setModified(flag);
}


void
Namebases::clear()
{
	m_player.clear();
	m_site.clear();
	m_event.clear();
	m_annotator.clear();
	m_round.clear();
}


void
Namebases::update()
{
	if (!m_player.isConsistent())
		m_player.update();
	if (!m_site.isConsistent())
		m_site.update();
	if (!m_event.isConsistent())
		m_event.update();
	if (!m_annotator.isConsistent())
		m_annotator.update();
	if (!m_round.isConsistent())
		m_round.update();
}

// vi:set ts=3 sw=3:
