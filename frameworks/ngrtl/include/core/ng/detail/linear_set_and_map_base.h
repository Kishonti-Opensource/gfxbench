/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef INCLUDE_GUARD_linear_set_and_map_base_1361441388
#define INCLUDE_GUARD_linear_set_and_map_base_1361441388

#include <assert.h>
#include <algorithm>

#include "ng/macros.h"
#include "ng/require.h"

namespace ng
{

	template<typename Tr>
	class linear_set_and_map_base
	{
		typedef linear_set_and_map_base<Tr> this_type;
		typedef Tr traits_type;
	public:
		typedef typename traits_type::container_type container_type;
		typedef typename traits_type::key_type key_type;
		typedef typename container_type::value_type value_type;
		typedef typename traits_type::container_type::iterator iterator;
		typedef typename traits_type::container_type::const_iterator const_iterator;
		typedef typename container_type::size_type size_type;
		typedef typename container_type::reference reference;
		typedef typename container_type::const_reference const_reference;

		linear_set_and_map_base()
			: _b_keep_sorted_on_write(false)
			, _b_sort_before_read(true)
			, _b_sorted(true)
		{
		}
		
		iterator insert(const value_type& val)
		{
			iterator it;

			if ( _b_sorted && _b_keep_sorted_on_write )
			{
				it = insert_sorted(val);
			} else
			{
				_v.push_back(val);
				it = _v.end() - 1;
				_b_sorted = _b_sorted &&
					(_v.size() == 1 || !_less(*it, it[-1]));
			}

			if ( !_b_sorted && _b_keep_sorted_on_write )
				sort_not_sorted();
			return it;
		}

		iterator insert(iterator hint, const value_type& val)
		{
			if ( !_b_sorted )
				return insert(val);
			bool b1 = hint == begin() || !_less(val, hint[-1]);
			bool b2 = hint == end() || !_less(*hint, val);
			if ( b1 && b2 )
				return _v.insert(hint, val);
			else
				return insert(val);
		}
		
		template<typename It>
		void insert(It b, It e)
		{
			iterator it;
			if ( _b_sorted && _b_keep_sorted_on_write )
			{
				for(It it = b; it != e; ++it)
					insert_sorted(*it);
			} else
			{
				_v.insert(_v.end(), b, e);
				_b_sorted = false;
			}

			if ( !_b_sorted && _b_keep_sorted_on_write )
				sort_not_sorted();
		}
		
		void sort() const
		{
			if ( !_b_sorted )
				sort_not_sorted();
		}
		
		iterator lower_bound(const key_type& k)
		{
			prepare_read();
			require(_b_sorted, "linear_set_and_map_base: lower_bound called on unsorted vector");
			return lower_bound_sorted(k);
		}
		
		const_iterator lower_bound(const key_type& k) const
		{
			prepare_read();
			require(_b_sorted, "linear_set_and_map_base: lower_bound called on unsorted vector");
			return lower_bound_sorted(k);
		}
		
		iterator find(const key_type& k)
		{
			const_iterator it = ((const this_type*)this)->find(k);
			return *(iterator*)&it;
		}
		
		const_iterator find(const key_type& k) const
		{
			prepare_read();

			if ( size() > 4 && _b_sorted )
			{
				const_iterator it = lower_bound_sorted(k);
				return it != _v.end() && traits_type::value_to_key(*it) == k
					? it : _v.end();
			}
			else
			{
				for(const_iterator it = _v.begin(); it != _v.end(); ++it)
					if ( traits_type::value_to_key(*it) == k )
						return it;
				return _v.end();
			}
		}
				
		iterator must_find(const key_type& k)
		{
			const_iterator it = ((const this_type*)this)->must_find(k);
			return *(iterator*)&it;
		}
		
		const_iterator must_find(const key_type& k) const
		{
			const_iterator it = find(k);
			require(it != end());
			return it;
		}

		iterator begin()
		{
			return _v.begin();
		}
		const_iterator begin() const
		{
			return _v.begin();
		}
		const_iterator cbegin()
		{
			return _v.begin();
		}
		iterator end()
		{
			return _v.end();
		}
		const_iterator end() const
		{
			return _v.end();
		}
		const_iterator cend()
		{
			return _v.end();
		}

		size_type size() const
		{
			return _v.size();
		}
		
		bool empty() const
		{
			return _v.empty();
		}

		reference front()
		{
			return _v.front();
		}
		const_reference front() const
		{
			return _v.front();
		}
		reference back()
		{
			return _v.back();
		}
		const_reference back() const
		{
			return _v.back();
		}
		
		void clear()
		{
			_v.clear();
			_b_sorted = true;
		}
		
		bool contains(const key_type& k) const
		{
			prepare_read();
			
			const_iterator it;
			if ( _b_sorted )
			{
				it = lower_bound_sorted(k);
				return it != _v.end() && traits_type::value_to_key(*it) == k;
			} else
			{
				for(const_iterator it = _v.begin(); it != _v.end(); ++it)
					if ( _eq(*it, k) )
						return true;
				return false;
			}
		}

		container_type& v()
		{
			return _v;
		}

		const container_type& v() const
		{
			return _v;
		}

		void swap(INOUT this_type& y)
		{
			std::swap(_less, y._less);
			std::swap(_eq, y._eq);
			std::swap(_b_keep_sorted_on_write, y._b_keep_sorted_on_write);
			std::swap(_b_sort_before_read, y._b_sort_before_read);
			std::swap(_b_sorted, y._b_sorted);
			_v.swap(y._v);
		}
		iterator erase(iterator it)
		{
			return _v.erase(it);
		}

	private:
		typename traits_type::less _less;
		typename traits_type::eq _eq;
		
		bool _b_keep_sorted_on_write;
		bool _b_sort_before_read;
		bool _b_sorted;
		
		container_type _v;
		
		void sort_not_sorted() const
		{
			this_type* mutable_this = (this_type*)this;
			std::sort(mutable_this->_v.begin(), mutable_this->_v.end(), _less);
			mutable_this->_b_sorted = true;
		}
		iterator lower_bound_sorted(const key_type& k)
		{
			assert(_b_sorted);
			return std::lower_bound(_v.begin(), _v.end(), k, _less);
		}
		const_iterator lower_bound_sorted(const key_type& k) const
		{
			assert(_b_sorted);
			return std::lower_bound(_v.begin(), _v.end(), k, _less);
		}
		iterator insert_sorted(const value_type& val)
		{
			iterator it = lower_bound_sorted(traits_type::value_to_key(val));
			_v.insert(it, val);
			return it;
		}
		void prepare_read() const
		{
			if ( !_b_sorted && _b_sort_before_read )
				sort_not_sorted();
		}
	};


}

#endif

