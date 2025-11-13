/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef NG_STDAUX_INCLUDED
#define NG_STDAUX_INCLUDED

#include <string.h>
#include <algorithm>
#include <functional>
#include <utility>

#include "ng/macros.h"
#include "ng/require.h"

namespace ng { namespace stdaux {

template<typename T>
void sort(INOUT T& c)
{
	::std::sort(c.begin(), c.end());
}

template<typename T, typename Pr>
void sort(INOUT T& c, const Pr& pr)
{
	::std::sort(c.begin(), c.end(), pr);
}

template<typename T>
typename T::iterator unique(INOUT T& c)
{
	return ::std::unique(c.begin(), c.end());
}

template<typename T, typename BinaryPredicate>
typename T::iterator unique(INOUT T& c, BinaryPredicate pred)
{
	return ::std::unique(c.begin(), c.end(), pred);
}

template<typename T>
void unique_trunc(INOUT T& c)
{
	c.erase(unique(c), c.end());
}

template<typename T, typename BinaryPredicate>
void unique_trunc(INOUT T& c, BinaryPredicate pred)
{
	c.erase(unique(c, pred), c.end());
}

template<typename C, typename T>
typename C::const_iterator lower_bound(const C& c, const T& value)
{
	return ::std::lower_bound(c.begin(), c.end(), value);
}

template<typename C, typename T>
typename C::iterator lower_bound(C& c, const T& value)
{
	return ::std::lower_bound(c.begin(), c.end(), value);
}

template<typename C, typename T, typename Pred>
typename C::const_iterator lower_bound(const C& c, const T& value, Pred pred)
{
	return ::std::lower_bound(c.begin(), c.end(), value, pred);
}

template<typename C, typename T, typename Pred>
typename C::iterator lower_bound(C& c, const T& value, Pred pred)
{
	return ::std::lower_bound(c.begin(), c.end(), value, pred);
}

template<typename C, typename T>
bool find_yesno(const C& c, const T& t)
{
	return std::find(c.begin(), c.end(), t) != c.end();
}

template<typename C, typename T>
bool binary_search(const C& c, const T& t)
{
	return std::binary_search(c.begin(), c.end(), t);
}

template<typename C, typename T, typename Pr>
bool binary_search(const C& c, const T& t, Pr pr)
{
	return std::binary_search(c.begin(), c.end(), t, pr);
}

template<typename C, typename T>
typename C::iterator remove(C& c, const T& t)
{
	return std::remove(c.begin(), c.end(), t);
}

template<typename C, typename T>
typename C::iterator must_remove(C& c, const T& t)
{
	typename C::iterator it = remove(c, t);
	require(it != c.end());
	return it;
}

template<typename C, typename T>
void remove_truncate(C& c, const T& t)
{
	c.erase(
		remove(c, t),
		c.end());
}

template<typename C, typename T>
void must_remove_truncate(C& c, const T& t)
{
	c.erase(
		must_remove(c, t),
		c.end());
}

template<typename C>
C set_difference(const C& x, const C& y)
{
	C z(x.size());
	z.erase(
		std::set_difference(x.begin(), x.end(), y.begin(), y.end(), z.begin()),
		z.end());
	return NG_MOVE_IF_SUPPORTED(z);
}

template<typename C, typename Iter>
void set_difference(const C& x, const C& y, Iter res)
{
	std::set_difference(x.begin(), x.end(), y.begin(), y.end(), res);
}

template<typename C>
C set_union(const C& x, const C& y)
{
	C z(x.size() + y.size());
	z.erase(
		std::set_union(x.begin(), x.end(), y.begin(), y.end(), z.begin()),
		z.end());
	return NG_MOVE_IF_SUPPORTED(z);
}

template<typename C>
typename C::const_iterator max_element(const C& c)
{
	return std::max_element(c.begin(), c.end());
}

template<typename C, typename T>
void fill(C& c, const T& x)
{
	std::fill(c.begin(), c.end(), x);
}

template<typename C>
typename C::value_type& push_back_default(C& c)
{
	c.push_back(typename C::value_type());
	return c.back();
}

template<typename C>
void tolower(C& c)
{
	std::transform(c.begin(), c.end(), c.begin(), ::tolower);
}

template<typename C>
void toupper(C& c)
{
	std::transform(c.begin(), c.end(), c.begin(), ::toupper);
}

template<typename It>
void tolower(const It& begin, const It& end)
{
	std::transform(begin, end, begin, ::tolower);
}

template<typename C, typename Pred>
typename C::difference_type
	count_if(const C& c, Pred pred)
{
	return std::count_if(c.begin(), c.end(), pred);
}

template<class PairType>
struct pair_less_by_first : std::binary_function<PairType, PairType, bool>
{
    pair_less_by_first() {}
	bool operator ()(const PairType& lhs, const PairType& rhs) const
	{
		return lhs.first < rhs.first;
	}
};

template<class PairType>
struct pair_less_by_second : std::binary_function<PairType, PairType, bool>
{
    pair_less_by_second() {}
	bool operator ()(const PairType& lhs, const PairType& rhs) const
	{
		return lhs.second < rhs.second;
	}
};

template<class PairType>
struct pair_equal_by_first : std::binary_function<PairType, PairType, bool>
{
	bool operator ()(const PairType& lhs, const PairType& rhs) const
	{
		return lhs.first == rhs.first;
	}
};

template<class PairType>
struct pair_equal_by_second : std::binary_function<PairType, PairType, bool>
{
	bool operator ()(const PairType& lhs, const PairType& rhs) const
	{
		return lhs.second == rhs.second;
	}
};

template<class PairType>
struct pair_select_first : std::unary_function<PairType, typename PairType::first_type>
{
	typename PairType::first_type operator ()(const PairType& pair) const
	{
		return pair.first;
	}
};

template<class PairType>
struct pair_select_second : std::unary_function<PairType, typename PairType::second_type>
{
	typename PairType::second_type operator ()(const PairType& pair) const
	{
		return pair.second;
	}
};

template< class T >
std::pair<const T&,const T&> minmax( const T& a, const T& b )
{
	return a < b ? std::pair<const T&,const T&>(a,b) : std::pair<const T&,const T&>(b,a);
}

template<class Type, class MemberType>
struct member_selector_ : std::unary_function<const Type, MemberType>
{
	typedef MemberType Type::*Selector;
	Selector m_selector;

	member_selector_(Selector selector) : m_selector(selector) {}

	const MemberType operator ()(const Type& item) const { return item.*m_selector; }
};

template<class Type, class MemberType>
member_selector_<Type, MemberType> member_selector(MemberType Type::*selector)
{
	return member_selector_<Type, MemberType>(selector);
}

template<class Type>
struct address_of : std::unary_function<Type, Type*>
{
	Type* operator ()(Type& item) const { return &item; }
};

template<typename C, typename K>
typename C::const_iterator must_find(const C& c, const K& k)
{
	typename C::const_iterator it = c.find(k);
	require(it != c.end());
	return it;
}

template<typename C, typename Pr>
typename C::const_iterator find_if(const C& c, Pr& pr)
{
	return std::find_if(c.begin(), c.end(), pr);
}

template<typename C, typename Pr>
typename C::iterator find_if(C& c, Pr& pr)
{
	return std::find_if(c.begin(), c.end(), pr);
}

template<typename C, typename K>
typename C::iterator must_find(C& c, const K& k)
{
	typename C::iterator it = c.find(k);
	require(it != c.end());
	return it;
}


template<typename T, typename Pr>
bool any_of(T& c, Pr pr)
{
#ifdef NG_HAS_CPP11
	return ::std::any_of(c.begin(), c.end(), pr);
#else
	typename T::const_iterator itEnd = c.end();
	for(typename T::const_iterator it = c.begin(); it != itEnd; ++it)
		if(pr(*it))
			return true;
	return false;
#endif
}

template<typename T, typename Pr>
bool exists(T& c, Pr pr)
{
	return any_of(c, pr);
}


template<typename C>
C set_intersection(const C& x, const C& y)
{
	C z(std::min(x.size(), y.size()));
	z.erase(
		std::set_intersection(x.begin(), x.end(), y.begin(), y.end(), z.begin()),
		z.end());
	return NG_MOVE_IF_SUPPORTED(z);
}

template<typename C>
bool set_intersects(C& x, C& y)
{
	typename C::const_iterator itx = x.begin();
	typename C::const_iterator itxend = x.end();
	typename C::const_iterator ity = y.begin();
	typename C::const_iterator ityend = y.end();
	for(; itx != itxend && ity != ityend; )
	{
		if(*itx < *ity)
		{
			++itx;
			continue;
		}
		if(*ity < *itx)
		{
			++ity;
			continue;
		}
		return true;
	}
	return false;
}

template<typename C1, typename C2>
void insert_at_end(C1& c1, const C2& c2)
{
	c1.insert(c1.end(), c2.begin(), c2.end());
}

template<typename C1, typename C2>
void insert(C1& x1, C2& x2)
{
	x1.insert(x2.begin(), x2.end());
}

template<typename C, typename Pr>
Pr for_each(C& c, Pr pr)
{
	return std::for_each(c.begin(), c.end(), pr);
}

template<typename C1, typename C2>
void copy_back_insert(C1& c1, C2& c2)
{
	std::copy(c1.begin(), c1.end(), std::back_inserter(c2));
}

template<typename C1, typename C2>
bool lexicographical_compare(C1& c1, C2& c2)
{
	return std::lexicographical_compare(c1.begin(), c1.end(), c2.begin(), c2.end());
}

template<typename C1, typename C2, typename Pr>
bool lexicographical_compare(C1& c1, C2& c2, Pr pr)
{
	return std::lexicographical_compare(c1.begin(), c1.end(), c2.begin(), c2.end(), pr);
}

template<typename C1, typename C2>
size_t mismatch_idx(C1& c1, C2& c2)
{
	return (std::mismatch(c1.begin(), c1.begin() + std::min(c1.size(), c2.size()), c2.begin())).first -
		c1.begin();
}

template<typename C1, typename C2>
bool equal(C1& c1, C2& c2)
{
	return c1.size() == c2.size() && std::equal(c1.begin(), c1.end(), c2.begin());
}

template<typename C1, typename C2, typename Eq>
bool equal(C1& c1, C2& c2, Eq eq)
{
	return c1.size() == c2.size() && std::equal(c1.begin(), c1.end(), c2.begin(), eq);
}

template<typename C>
bool is_sorted(const C& c)
{
	typename C::const_iterator first = c.begin();
	typename C::const_iterator last = c.end();
	if (first == last) return true;
	typename C::const_iterator next = first;
	while (++next != last) {
		if (*next<*first)
			return false;
		++first;
	}
	return true;
}

template<typename C, typename Cmp>
bool is_sorted(const C& c, Cmp cmp)
{
	typename C::const_iterator first = c.begin();
	typename C::const_iterator last = c.end();
	if (first == last) return true;
	typename C::const_iterator next = first;
	while (++next != last) {
		if (cmp(*next, *first))
			return false;
		++first;
	}
	return true;
}

#if _MSC_VER >= 1600 || (defined(__cplusplus) && __cplusplus >= 201103)

#include <functional>

template<typename Pair>
class hash_of_pair
{
public:
	std::hash<typename Pair::first_type> h1;
	std::hash<typename Pair::second_type> h2;
    std::size_t operator()(const Pair& x) const 
    {
		size_t seed = h1(x.first);
		seed ^= h2(x.second) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
		return seed;
    }
};

template<typename S>
typename S::iterator
	set_insert_new(S& s, const typename S::value_type& v)
{
	std::pair<typename S::iterator, bool> itb = s.insert(v);
	require(itb.second);
	return itb.first;
}

template<typename M>
std::pair<typename M::iterator, bool>
	map_insert(M& m, const typename M::key_type& k, const typename M::mapped_type& v)
{
	return m.insert(std::make_pair(k, v));
}

template<typename M>
std::pair<typename M::iterator, bool>
	map_insert_default(M& m, const typename M::key_type& k)
{
	return map_insert(m, k, typename M::mapped_type());
}


//implements less functions (as operator()) between std::pair<F, S> and F operands
//using pair::first for pair operands
//can be used for key-value pairs
template<typename F, typename S>
struct less_pair_first_and_first
{
	bool operator()(const std::pair<F, S>& x, const std::pair<F, S>& y) const
	{
		return x.first < y.first;
	}
	bool operator()(const std::pair<F, S>& x, const F& y) const
	{
		return x.first < y;
	}
	bool operator()(const F& x, const std::pair<F, S>& y) const
	{
		return x < y.first;
	}
	bool operator()(const F& x, const F& y) const
	{
		return x < y;
	}
};

//c is a container of pairs, sorted by pair::first
template<typename C>
typename C::iterator binary_find_by_pair_first(C& c, const typename C::value_type::first_type& v)
{
	typename C::iterator it =
		std::lower_bound(c.begin(), c.end(), v,
			less_pair_first_and_first<typename C::value_type::first_type, typename C::value_type::second_type>());
	return it != c.end() && it->first == v //this could be tested with <
		? it : c.end();
}

template<typename C>
typename C::const_iterator binary_find_by_pair_first(const C& c, const typename C::value_type::first_type& v)
{
	typename C::const_iterator it =
		std::lower_bound(c.begin(), c.end(), v,
			less_pair_first_and_first<typename C::value_type::first_type, typename C::value_type::second_type>());
	return it != c.end() && it->first == v //this could be tested with <
		? it : c.end();
}

//c is a container of pairs, sorted by pair::first
template<typename C>
bool binary_search_by_pair_first(const C& c, const typename C::value_type::first_type& v)
{
	typename C::const_iterator it =
	std::lower_bound(c.begin(), c.end(), v,
					 less_pair_first_and_first<typename C::value_type::first_type, typename C::value_type::second_type>());
	return it != c.end() && it->first == v; //this could be tested with <
}

template<typename C>
typename C::iterator binary_must_find_by_pair_first(C& c, const typename C::value_type::first_type& v)
{
	typename C::iterator it = binary_find_by_pair_first(c, v);
	require(it != c.end());
	return it;
}

template<typename C>
typename C::const_iterator binary_must_find_by_pair_first(const C& c, const typename C::value_type::first_type& v)
{
	typename C::const_iterator it = binary_find_by_pair_first(c, v);
	require(it != c.end());
	return it;
}

#endif

}}

#define BEGINEND(x) (x).begin(), (x).end()


#endif
