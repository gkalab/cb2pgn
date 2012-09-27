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

#ifndef _mstl_hash_included
#define _mstl_hash_included

#include "m_vector.h"
#include "m_pair.h"

namespace mstl {

template <typename Key, typename Value>
class hash
{
private:

	struct dict_node;

public:

	static double constexpr Load = 70.0;

	typedef Value					value_type;
	typedef Key						key_type;
	typedef value_type&			reference;
	typedef value_type const&	const_reference;
	typedef value_type*			pointer;
	typedef value_type const*	const_pointer;
	typedef pair<Key,Value>		assoc_t;
	typedef bits::size_t			size_type;

	class const_iterator
	{
	public:

		bool operator==(const_iterator const& it) const;
		bool operator!=(const_iterator const& it) const;
		bool operator< (const_iterator const& it) const;

		const_iterator& operator++();
		const_iterator  operator++(int);

		assoc_t const& operator*() const;
		assoc_t const* operator->() const;

	private:

		friend class hash;

		const_iterator(size_t last_bucket, hash const& hash);
		const_iterator(hash const& hash, size_t last_bucket);

		hash const&			m_this;
		dict_node const*	m_node;
		size_t				m_last_bucket;
	};

	hash(size_type capacity = 8);
	hash(hash const& h);
	~hash() throw();

#if HAVE_0X_MOVE_CONSTRCUTOR_AND_ASSIGMENT_OPERATOR
	hash(hash&& h);
	hash& operator=(hash&& h);
#endif

	hash& operator=(hash const& h);

	bool empty() const;

	size_type size() const;
	size_type used() const;

	size_type count_collisions() const;
	size_type max_bucket_length() const;
	double average_bucket_length() const;
	size_type storage_size() const;

	bool has_key(key_type const& key) const;

	const_reference operator[](key_type const& i) const;
	reference operator[](key_type const& i);

	const_iterator begin() const;
	const_iterator end() const;

	const_pointer find(key_type const& key) const;
	reference find_or_insert(key_type const& key, const_reference value);

	bool insert_unique(key_type const& key, const_reference value);
	void remove(key_type const& key);

	void clear() throw();
	void rehash();
	void rebuild(size_type new_size);
	void reserve(size_type capacity);
	void swap(hash& h);

private:

	struct dict_node
	{
		dict_node*	next;
		assoc_t		assoc;
	};

	typedef vector<dict_node*> node_table;

	bool fullness() const;

	dict_node const* find_node(key_type const& key) const;
	dict_node* find_node(dict_node* node, key_type const& key) const;

	static size_type bucket_length(dict_node const* node);

	void copy_table(hash const& h);
	void update_indices(size_type index);

	dict_node* insert_new_node(key_type const& key, const_reference value, bool unique);
	void remove(dict_node* prev, dict_node *node, size_type index);

	size_type	m_size;
	size_type	m_modulo;
	size_type	m_used;
	size_type	m_first;
	size_type	m_last;
	node_table	m_buckets;
};


template <typename Key> struct hash_key			{ static size_t hash(void const* key); };

template <> struct hash_key<int16_t>				{ static size_t hash(int16_t key); };
template <> struct hash_key<int32_t>				{ static size_t hash(int32_t key); };
template <> struct hash_key<int64_t>				{ static size_t hash(int64_t key); };
template <> struct hash_key<uint16_t>				{ static size_t hash(uint16_t key); };
template <> struct hash_key<uint32_t>				{ static size_t hash(uint32_t key); };
template <> struct hash_key<uint64_t>				{ static size_t hash(uint64_t key); };
template <> struct hash_key<char*>					{ static size_t hash(char const* key); };
template <> struct hash_key<char const*>			{ static size_t hash(char const* key); };
template <> struct hash_key<mstl::string>			{ static size_t hash(mstl::string const& key); };
template <> struct hash_key<mstl::string const>	{ static size_t hash(mstl::string const& key); };

} // namespace mstl

#include "m_hash.ipp"

#endif // _mstl_hash_included

// vi:set ts=3 sw=3:
