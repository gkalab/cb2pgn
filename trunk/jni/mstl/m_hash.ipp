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
#include "m_assert.h"

#include <string.h>

namespace mstl {
namespace bits {

size_t hash_str(mstl::string const& key);
size_t hash_str(char const* key);
size_t hash_size(size_t min_size);
size_t hash_expansion(size_t size);

template <typename T>
inline int equal(T const& lhs, T const& rhs) { return lhs == rhs; }

inline int equal(char const* lhs, char const* rhs) { return ::strcmp(lhs, rhs) == 0; }

} // namespace bits


template <typename Key>
inline
size_t
hash_key<Key>::hash(void const* key)
{
	return size_t(key);
}


inline size_t hash_key<int16_t>::hash(int16_t key)							{ return key; }
inline size_t hash_key<int32_t>::hash(int32_t key)							{ return key; }
inline size_t hash_key<int64_t>::hash(int64_t key)							{ return key; }
inline size_t hash_key<uint16_t>::hash(uint16_t key)						{ return key; }
inline size_t hash_key<uint32_t>::hash(uint32_t key)						{ return key; }
inline size_t hash_key<uint64_t>::hash(uint64_t key)						{ return key; }
inline size_t hash_key<char*>::hash(char const* key)						{ return bits::hash_str(key); }
inline size_t hash_key<char const*>::hash(char const* key)				{ return bits::hash_str(key); }
inline size_t hash_key<mstl::string>::hash(mstl::string const& key)	{ return bits::hash_str(key); }

inline size_t hash_key<mstl::string const>::hash(mstl::string const& key) { return bits::hash_str(key); }


template <typename Value, typename Key>
inline
hash<Value,Key>::const_iterator::const_iterator(hash const& hash, size_t last_bucket)
	:m_this(hash)
	,m_node(0)
	,m_last_bucket(last_bucket)
{
	++(*this);
}


template <typename Value, typename Key>
inline
hash<Value,Key>::const_iterator::const_iterator(size_t last_bucket, hash const& hash)
	:m_this(hash)
	,m_node(0)
	,m_last_bucket(last_bucket)
{
}


template <typename Value, typename Key>
inline
bool
hash<Value,Key>::const_iterator::operator==(const_iterator const& it) const
{
	return m_last_bucket == it.m_last_bucket;
}



template <typename Value, typename Key>
inline
bool
hash<Value,Key>::const_iterator::operator!=(const_iterator const& it) const
{
	return m_last_bucket != it.m_last_bucket;
}



template <typename Value, typename Key>
inline
bool
hash<Value,Key>::const_iterator::operator<(const_iterator const& it) const
{
	return m_last_bucket < it.m_last_bucket;
}


template <typename Value, typename Key>
inline
typename hash<Value,Key>::const_iterator&
hash<Value,Key>::const_iterator::operator++()
{
	if (m_node)
	{
		if (m_node->next)
		{
			m_node = m_node->next;
			return *this;
		}

		++m_last_bucket;
	}

	for ( ; m_last_bucket < m_this.m_last; ++m_last_bucket)
	{
		if (m_this.m_buckets[m_last_bucket])
		{
			m_node = m_this.m_buckets[m_last_bucket];
			return *this;
		}
	}

	return *this;
}


template <typename Value, typename Key>
inline
typename hash<Value,Key>::const_iterator
hash<Value,Key>::const_iterator::operator++(int)
{
	const_iterator it(*this);
	++(*this);
	return it;
}


template <typename Value, typename Key>
inline
typename hash<Value,Key>::assoc_t const&
hash<Value,Key>::const_iterator::operator*() const
{
	return m_node->assoc;
}


template <typename Value, typename Key>
inline
typename hash<Value,Key>::assoc_t const*
hash<Value,Key>::const_iterator::operator->() const
{
	return &m_node->assoc;
}


template <typename Value, typename Key>
inline
bool
hash<Value,Key>::empty() const
{
	return m_used == 0;
}


template <typename Value, typename Key>
inline
typename hash<Value,Key>::size_type
hash<Value,Key>::size() const
{
	return m_size;
}


template <typename Value, typename Key>
inline
typename hash<Value,Key>::size_type
hash<Value,Key>::used() const
{
	return m_used;
}


template <typename Value, typename Key>
inline
bool
hash<Value,Key>::has_key(key_type const& key) const
{
	return find_node(key);
}


template <typename Value, typename Key>
inline
typename hash<Value,Key>::const_pointer
hash<Value,Key>::find(key_type const& key) const
{
	dict_node const* node = find_node(key);
	return node ? &node->assoc.second : 0;
}


template <typename Value, typename Key>
inline
bool
hash<Value,Key>::fullness() const
{
	return size_type(m_size*Load)/100.0 <= m_used;
}


template <typename Value, typename Key>
inline
hash<Value,Key>::hash(size_type capacity)
	:m_size(bits::hash_size(mstl::max(size_type(8), capacity)))
	,m_modulo(m_size - 1)
	,m_used(0)
	,m_first(m_size)
	,m_last(0)
	,m_buckets(m_size)
{
}


template <typename Value, typename Key>
inline
hash<Value,Key>::hash(hash const& h)
	:m_size(h.m_size)
	,m_modulo(m_size - 1)
	,m_used(0)
	,m_first(h.m_first)
	,m_last(h.m_last)
	,m_buckets(m_size)
{
	copy_table(h);
}


template <typename Value, typename Key>
inline
hash<Value,Key>::~hash() throw()
{
	clear();
}


template <typename Value, typename Key>
hash<Value,Key>&
hash<Value,Key>::operator=(hash const& h)
{
	if (this != &h)
	{
		hash newh(h);
		swap(newh);
	}

	return *this;
}


template <typename Value, typename Key>
inline
void
hash<Value,Key>::copy_table(hash const& h)
{
	//M_ASSERT(h.m_size == m_size);

	for (size_type bucket = h.m_first; bucket < h.m_last; ++bucket)
	{
		for (dict_node* node = m_buckets[bucket]; node; node = node->next)
		{
			dict_node* n = new dict_node;
			n->assoc = node->assoc;
			n->next = m_buckets[bucket];
			m_buckets[bucket] = n;
		}
	}
}


template <typename Value, typename Key>
inline
void
hash<Value,Key>::clear() throw()
{
	for (size_type i = m_first; i < m_last; ++i)
	{
		dict_node* node = m_buckets[i];

		while (node)
		{
			dict_node* next = node->next;
			delete node;
			node = next;
		}

		m_buckets[i] = 0;
	}

	m_first = m_size;
	m_last = 0;
	m_used = 0;
}


template <typename Value, typename Key>
inline
void
hash<Value,Key>::swap(hash& h)
{
	mstl::swap(m_size,		h.m_size);
	mstl::swap(m_modulo,		h.m_modulo);
	mstl::swap(m_used,		h.m_used);
	mstl::swap(m_first,		h.m_first);
	mstl::swap(m_last,		h.m_last);
	mstl::swap(m_buckets,	h.m_buckets);
}


template <typename Value, typename Key>
inline
void
hash<Value,Key>::reserve(size_type capacity)
{
	rebuild(bits::hash_size(mstl::max(size_type(8), capacity)));
}


template <typename Value, typename Key>
inline
void
hash<Value,Key>::rehash()
{
	rebuild(m_size);
}


template <typename Value, typename Key>
inline
void
hash<Value,Key>::update_indices(size_type index)
{
	if (m_first > index) m_first = index;
	if (m_last <= index) m_last = index + 1;
}


template <typename Value, typename Key>
inline
void
hash<Value,Key>::rebuild(size_type new_size)
{
	hash new_hash(new_size);

	for (size_type bucket = m_first; bucket < m_last; ++bucket)
	{
		dict_node* node = m_buckets[bucket];

		while (node)
		{
			dict_node*	next	= node->next;
			size_type	index	= hash_key<key_type>::hash(node->assoc.first) & new_hash.m_modulo;

			if ((node->next = new_hash.m_buckets[index]) == 0)
				new_hash.update_indices(index);

			new_hash.m_buckets[index] = node;
			++new_hash.m_used;
			node = next;
		}

		m_buckets[bucket] = 0;
	}

	m_first = m_size;
	m_last = 0;
	swap(new_hash);
}


template <typename Value, typename Key>
inline
typename hash<Value,Key>::dict_node*
hash<Value,Key>::find_node(dict_node* node, key_type const& key) const
{
	for ( ; node; node = node->next)
	{
		if (bits::equal(node->assoc.first, key))
			return node;
	}

	return 0;
}


template <typename Value, typename Key>
inline
typename hash<Value,Key>::dict_node const*
hash<Value,Key>::find_node(key_type const& key) const
{
	size_type index = hash_key<key_type>::hash(key) & m_modulo;
	return find_node(m_buckets[index], key);
}


template <typename Value, typename Key>
inline
bool
hash<Value,Key>::insert_unique(key_type const& key, const_reference value)
{
	dict_node* node = insert_new_node(key, value, true);
	return node != 0;
}


template <typename Value, typename Key>
inline
typename hash<Value,Key>::reference
hash<Value,Key>::find_or_insert(key_type const& key, const_reference value)
{
	dict_node* node = insert_new_node(key, value, false);
	return node->assoc.second;
}


template <typename Value, typename Key>
typename hash<Value,Key>::dict_node*
hash<Value,Key>::insert_new_node(key_type const& key, const_reference value, bool unique)
{
	size_type	index	= hash_key<key_type>::hash(key) & m_modulo;
	dict_node*	node	= find_node(m_buckets[index], key);

	if (node)
		return unique ? 0 : node;

	if (fullness())
		rebuild(bits::hash_expansion(m_size));

	node = new dict_node;

	if ((node->next = m_buckets[index]) == 0)
		update_indices(index);

	m_buckets[index] = node;
	++m_used;

	node->assoc.first = key;
	node->assoc.second = value;

	return node;
}


template <typename Value, typename Key>
void
hash<Value,Key>::remove(dict_node* prev, dict_node *node, size_type index)
{
	//M_ASSERT(!empty());

	m_used--;

	if (prev)
	{
		prev->next = node->next;
	}
	else if ((m_buckets[index] = node->next) == 0)
	{
		if (m_used == 0)
		{
			m_first = m_size;
			m_last = 0;
		}
		else
		{
			if (m_first == index)
			{
				++m_first;
				//M_ASSERT(m_first < m_size);
			}

			if (m_last == index)
			{
				//M_ASSERT(m_last > 0);
				--m_last;
			}

			//M_ASSERT(m_first < m_last);
		}
	}

	delete node;
}


template <typename Value, typename Key>
void
hash<Value,Key>::remove(key_type const& key)
{
	size_type	index	= hash_key<key_type>::hash(key) & m_modulo;
	dict_node*	node	= m_buckets[index];

	if (node)
	{
		dict_node* prev = 0;

		for ( ; node; node = node->next)
		{
			if (bits::equal(node->assoc.first, key))
				return remove(prev, node, index);

			prev = node;
		}
	}
}


template <typename Value, typename Key>
inline
typename hash<Value,Key>::const_iterator
hash<Value,Key>::begin() const
{
	return const_iterator(*this, mstl::min(m_first, m_last));
}


template <typename Value, typename Key>
inline
typename hash<Value,Key>::const_iterator
hash<Value,Key>::end() const
{
	return const_iterator(m_last, *this);
}


template <typename Value, typename Key>
inline
typename hash<Value,Key>::size_type
hash<Value,Key>::bucket_length(dict_node const* node)
{
	size_type n = 0;

	for ( ; node; node = node->next)
		++n;

	return n;
}


template <typename Value, typename Key>
inline
typename hash<Value,Key>::size_type
hash<Value,Key>::count_collisions() const
{
	size_type count = 0;

	for (size_type bucket = m_first; bucket < m_last; ++bucket)
		count += mstl::max(1u, bucket_length(m_buckets[bucket])) - 1;

	return count;
}


template <typename Value, typename Key>
inline
typename hash<Value,Key>::size_type
hash<Value,Key>::max_bucket_length() const
{
	size_type length = 0;

	for (size_type bucket = m_first; bucket < m_last; ++bucket)
		length = mstl::max(length, bucket_length(m_buckets[bucket]));

	return length;
}


template <typename Value, typename Key>
inline
double
hash<Value,Key>::average_bucket_length() const
{
	size_type used = 0;

	for (size_type bucket = m_first; bucket < m_last; ++bucket)
	{
		if (m_buckets[bucket])
			++used;
	}

	return double(count_collisions() + used)/double(used);
}


template <typename Value, typename Key>
inline
typename hash<Value,Key>::size_type
hash<Value,Key>::storage_size() const
{
	return	sizeof(*this)
			 + sizeof(dict_node)*m_used
			 + m_buckets.capacity()*sizeof(typename node_table::value_type);
}


template <typename K, typename V>
inline
typename hash<K,V>::const_reference
hash<K,V>::operator[](key_type const& i) const
{
	M_REQUIRE(has_key(i));
	return *find(i);
}


template <typename K, typename V>
inline
typename hash<K,V>::reference
hash<K,V>::operator[](key_type const& i)
{
	return find_or_insert(i, value_type());
}


#if HAVE_0X_MOVE_CONSTRCUTOR_AND_ASSIGMENT_OPERATOR

template <typename K, typename V>
inline
hash<K,V>::hash(hash&& h)
	:m_size(h.m_size)
	,m_modulo(h.m_modulo)
	,m_used(h.m_used)
	,m_first(h.m_first)
	,m_last(h.m_last)
	,m_buckets(mstl::move(h.m_buckets))
{
}


template <typename K, typename V>
inline
hash<K,V>&
hash<K,V>::operator=(hash&& h)
{
	if (this != &h)
	{
		m_size = h.m_size;
		m_modulo = h.m_modulo;
		m_used = h.m_used;
		m_first = h.m_first;
		m_last = h.m_last;
		m_buckets = mstl::move(h.m_buckets);
	}

	return *this;
}

#endif

} // namespace mstl

// vi:set ts=3 sw=3:
