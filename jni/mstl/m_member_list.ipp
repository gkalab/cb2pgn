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

#include "m_utility.h"

namespace mstl {
namespace tl {
namespace bits {

template <typename Head, typename Tail>
template <typename T0, typename T1, typename T2>
inline
member_list< type_list<Head, Tail> >::member_list(T0 const& t0, T1 const& t1, T2 const& t2)
	:m_head(t0)
	,m_tail(t1, t2, null_type())
{
}

template <typename Head>
template <typename T0, typename T1, typename T2>
inline
member_list< type_list<Head, null_type> >::member_list(T0 const& t0, T1 const&, T2 const&)
	:m_head(t0)
{
}

#if HAVE_0X_MOVE_CONSTRCUTOR_AND_ASSIGMENT_OPERATOR

template <typename Head, typename Tail>
inline
member_list< type_list<Head, Tail> >::member_list(member_list&& ml)
	:m_head(mstl::move(ml.m_head))
	,m_tail(mstl::move(ml.m_tail))
{
}


template <typename Head, typename Tail>
inline
member_list< type_list<Head, Tail> >&
member_list< type_list<Head, Tail> >::operator=(member_list&& ml)
{
	m_head = mstl::move(ml.m_head);
	m_tail = mstl::move(ml.m_tail);
	return *this;
}


template <typename Head>
inline
member_list< type_list<Head, null_type> >::member_list(member_list&& ml)
	:m_head(mstl::move(ml.m_head))
{
}


template <typename Head>
inline
member_list< type_list<Head, null_type> >&
member_list< type_list<Head, null_type> >::operator=(member_list&& ml)
{
	m_head = mstl::move(ml.m_head);
	return *this;
}

#endif

template <typename MemberList, int N>
inline
typename tl::type_at<typename MemberList::type_list_t,N>::result const&
accessor<MemberList,N>::get(MemberList const& list)
{
	return accessor<typename MemberList::tail,N - 1>::get(list.m_tail);
}


template <typename MemberList, int N>
inline
typename tl::type_at<typename MemberList::type_list_t,N>::result&
accessor<MemberList,N>::get(MemberList& list)
{
	return accessor<typename MemberList::tail,N - 1>::get(list.m_tail);
}


template <typename MemberList>
inline
typename MemberList::head const&
accessor<MemberList, 0>::get(MemberList const& list)
{
	return list.m_head;
}


template <typename MemberList>
inline
typename MemberList::head&
accessor<MemberList, 0>::get(MemberList& list)
{
	return list.m_head;
}

} // namespace bits
} // namespace tl
} // namespace mstl

// vi:set ts=3 sw=3:
