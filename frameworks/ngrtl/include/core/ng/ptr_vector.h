/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef NG_PTR_VECTOR_INCLUDED
#define NG_PTR_VECTOR_INCLUDED

#include <vector>
#include <cstdlib> // size_t

namespace ng {

//minimal implementation
//please extend as needed

template<typename T>
class ptr_vector_const_iterator;

template<typename T>
class ptr_vector;

template<typename T>
class ptr_vector_iterator
{
public:
	typedef typename std::vector<T*>::iterator base_iterator;
	typedef ptr_vector_iterator<T> this_type;

	typedef typename base_iterator::difference_type difference_type;
	typedef T value_type;
	typedef T* pointer;
	typedef T& reference;
	typedef typename base_iterator::iterator_category iterator_category;

	ptr_vector_iterator(const base_iterator& b)
		: _base(b)
	{}
	bool operator==(const this_type& y) const
	{
		return _base == y._base;
	}
	bool operator!=(const this_type& y) const
	{
		return _base != y._base;
	}
	this_type& operator++() { ++_base; return *this; }
	this_type& operator++(int) { this_type x(*this); ++_base; return x; }
	this_type& operator--() { --_base; return *this; }
	this_type& operator--(int) { this_type x(*this); --_base; return x; }
	pointer operator->() const { return *_base; }
	reference operator*() const { return **_base; }
	bool operator<(const this_type& y) const { return _base < y._base; }
	difference_type operator-(const this_type& y) const
	{
		return _base - y._base;
	}
private:
	friend class ptr_vector_const_iterator<T>;
	friend class ptr_vector<T>;

	base_iterator _base;

	base_iterator base() const { return _base; }
};

template<typename T>
class ptr_vector_const_iterator
{
public:
	typedef typename std::vector<T*>::const_iterator base_iterator;
	typedef ptr_vector_const_iterator<T> this_type;

	typedef typename base_iterator::difference_type difference_type;
	typedef T value_type;
	typedef const T* pointer;
	typedef const T& reference;
	typedef typename base_iterator::iterator_category iterator_category;

	ptr_vector_const_iterator(const base_iterator& b)
		: _base(b)
	{}
	ptr_vector_const_iterator(const ptr_vector_iterator<T>& b)
		: _base(b._base)
	{}
	bool operator==(const this_type& y) const
	{
		return _base == y._base;
	}
	bool operator!=(const this_type& y) const
	{
		return _base != y._base;
	}
	this_type& operator++() { ++_base; return *this; }
	this_type& operator++(int) { this_type x(*this); ++_base; return x; }
	this_type& operator--() { --_base; return *this; }
	this_type& operator--(int) { this_type x(*this); --_base; return x; }
	pointer operator->() const { return *_base; }
	reference operator*() const { return **_base; }
	bool operator<(const this_type& y) const { return _base < y._base; }
	difference_type operator-(const this_type& y) const
	{
		return _base - y._base;
	}
private:
	friend class ptr_vector<T>;

	base_iterator _base;

	base_iterator base() const { return _base; }
};

template<typename T>
class ptr_vector
{
public:
	typedef ptr_vector<T> this_type;
	typedef size_t size_type;
	typedef T value_type;
	typedef ptr_vector_iterator<T> iterator;
	typedef ptr_vector_const_iterator<T> const_iterator;

	~ptr_vector()
	{
		deleteItems();
	}

	size_type size() const { return _v.size(); }
	bool empty() const { return _v.empty(); }

	T& operator[](size_type i) { return *_v[i]; }
	const T& operator[](size_type i) const { return *_v[i]; }

	T& front() { return *_v.front(); }
	const T& front() const { return *_v.front(); }

	T& back() { return *_v.back(); }
	const T& back() const { return *_v.back(); }

	void push_back(T* p) //takes ownership!!!
	{
		_v.push_back(p);
	}

	iterator begin() { return iterator(_v.begin()); }
	const_iterator begin() const { return const_iterator(_v.begin()); }
	iterator end() { return iterator(_v.end()); }
	const_iterator end() const { return const_iterator(_v.end()); }
	iterator erase (iterator position)
	{
		delete &*position;
		return iterator(_v.erase(position.base()));
	}
	
	void clear()
	{
		deleteItems();
		_v.clear();
	}

	void swap(this_type& y)
	{
		_v.swap(y._v);
	}

	//moves all elements from 'y' before 'before'
	void transfer(iterator before, ptr_vector<T>& y)
	{
		_v.insert(before.base(), y._v.begin(), y._v.end());
		y._v.erase(y._v.begin(), y._v.end());
	}
private:
	typedef std::vector<T*> Cont;
	Cont _v;

	void deleteItems()
	{
		for(typename Cont::iterator it = _v.begin(); it != _v.end(); ++it)
			delete *it;
	}
};

}

#endif
