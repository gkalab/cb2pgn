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

#ifndef _mstl_iterator_included
#define _mstl_iterator_included

namespace mstl {

/// \class back_insert_iterator
/// \ingroup IteratorAdaptors
/// \brief Calls push_back on bound container for each assignment.
template <class Container>
class back_insert_iterator
{
public:

	typedef typename Container::value_type			value_type;
	typedef typename Container::difference_type	difference_type;
	typedef typename Container::pointer				pointer;
	typedef typename Container::reference			reference;

	explicit back_insert_iterator(Container& ctr);

	back_insert_iterator& operator=(typename Container::const_reference v);

	back_insert_iterator& operator*();
	back_insert_iterator& operator++();
	back_insert_iterator  operator++(int);

protected:

	Container& m_container;
};


template <class Container> back_insert_iterator<Container> back_inserter(Container& ctr);

} // namespace mstl

#include "m_iterator.ipp"

#endif // _mstl_iterator_included

// vi:set ts=3 sw=3:
