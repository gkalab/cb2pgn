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

#ifndef _mstl_member_list_included
#define _mstl_member_list_included

namespace mstl {
namespace tl {
namespace bits {

template <typename TList> struct member_list;


template <typename Head, typename Tail>
struct member_list< type_list<Head, Tail> >
{
	typedef Head head;
	typedef member_list<Tail> tail;
	typedef type_list<Head, Tail> type_list_t;

	template <typename T0, typename T1, typename T2>
	member_list(T0 const& t0, T1 const& t1, T2 const& t2);

#if HAVE_OX_EXPLICITLY_DEFAULTED_AND_DELETED_SPECIAL_MEMBER_FUNCTIONS
	member_list(member_list const&) = default;
	member_list& operator=(member_list const&) = default;
#endif

#if HAVE_0X_MOVE_CONSTRCUTOR_AND_ASSIGMENT_OPERATOR
	member_list(member_list&& ml);
	member_list& operator=(member_list&& ml);
#endif

	head m_head;
	tail m_tail;
};


template <typename Head>
struct member_list< type_list<Head, null_type> >
{
	typedef Head head;
	typedef type_list<Head, null_type> type_list_t;

	template <typename T0, typename T1, typename T2>
	member_list(T0 const& t0, T1 const&, T2 const&);

#if HAVE_OX_EXPLICITLY_DEFAULTED_AND_DELETED_SPECIAL_MEMBER_FUNCTIONS
	member_list(member_list const&) = default;
	member_list& operator=(member_list const&) = default;
#endif

#if HAVE_0X_MOVE_CONSTRCUTOR_AND_ASSIGMENT_OPERATOR
	member_list(member_list&& ml);
	member_list& operator=(member_list&& ml);
#endif

	head m_head;
};


template <typename MemberList, int N>
struct accessor
{
	static typename tl::type_at<typename MemberList::type_list_t,N>::result const&
	get(MemberList const& list);

	static typename tl::type_at<typename MemberList::type_list_t,N>::result&
	get(MemberList& list);
};


template <typename MemberList>
struct accessor<MemberList, 0>
{
	static typename MemberList::head const& get(MemberList const& list);
	static typename MemberList::head& get(MemberList& list);
};

} // namespace bits
} // namespace tl
} // namespace mstl

#include "m_member_list.ipp"

#endif // _mstl_member_list_included

// vi:set ts=3 sw=3:
