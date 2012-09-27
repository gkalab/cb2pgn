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

template <typename Signature>
function<Signature>::function()
{
}


template <typename Signature>
function<Signature>::function(Signature& func)
	:base(func)
{
}


template <typename Signature>
template <class Obj>
function<Signature>::function(typename base::template meth_t<Obj>::method_ptr meth, Obj* obj)
	:base(meth, obj)
{
}


template <typename Signature>
template <class Obj>
function<Signature>::function(typename base::template meth_t<Obj>::const_method_ptr meth, Obj const*obj)
	:base(meth, obj)
{
}


template <typename Signature>
template <class Functor>
function<Signature>::function(Functor* functor)
	:base(functor)
{
}


template <typename Signature>
template <class Functor>
function<Signature>::function(Functor const* functor)
	:base(const_cast<Functor*>(functor))
{
}


template <typename Signature>
inline
bool
function<Signature>::empty() const
{
	return base::empty();
}


template <typename Signature>
inline
function<Signature>::operator bool () const
{
	return !base::empty();
}


template <typename Signature>
bool
function<Signature>::operator==(function const& f) const
{
	return equal(f);
}

} // namespace mstl

// vi:set ts=3 sw=3:
