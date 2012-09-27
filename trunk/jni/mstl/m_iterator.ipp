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

template <class Container>
inline
back_insert_iterator<Container>::back_insert_iterator(Container& ctr) : m_container(ctr) {}


template <class Container>
inline
back_insert_iterator<Container>&
back_insert_iterator<Container>::operator=(typename Container::const_reference v)
{
	m_container.push_back(v);
	return *this;
}


template <class Container>
inline
back_insert_iterator<Container>&
back_insert_iterator<Container>::operator*()
{
	return *this;
}


template <class Container>
inline
back_insert_iterator<Container>&
back_insert_iterator<Container>::operator++()
{
	return *this;
}


template <class Container>
inline
back_insert_iterator<Container>
back_insert_iterator<Container>::operator++(int)
{
	return *this;
}


/// Returns the back_insert_iterator for \p ctr.
template <class Container>
inline
back_insert_iterator<Container>
back_inserter(Container& ctr)
{
	return back_insert_iterator<Container>(ctr);
}

} // namespace mstl

// vi:set ts=3 sw=3:
