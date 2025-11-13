/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef NG_OPTIONAL_INCLUDED
#define NG_OPTIONAL_INCLUDED

#include "none.h"
#include "ng/mpl/int_of_size.h"
#include "ng/require.h"

namespace ng
{
	template<typename T>
	class optional
	{
	private:
		struct aligment_test_type
		{
			uint8_t c;
			T t;
		};
		static const size_t c_storage_padding = sizeof(aligment_test_type) - sizeof(T) - 1;

	public:
		typedef optional<T> this_type;

		optional()
			: _bValid(false)
		{}
		optional(const T& t)
			: _bValid(false)
		{
			new (alignedStorage()) T(t);
			_bValid = true;
		}
		optional(const this_type& o)
			: _bValid(false)
		{
			if ( !!o )
			{
				new (alignedStorage()) T(*o);
				_bValid = true;
			}
		}
		
		this_type &operator=(const this_type& o)
		{
			if (this != &o)
			{
				this_type tmp(o);
				swap(tmp);
			}
			return *this;
		}
		
		void swap(this_type& y)
		{
			if ( !*this && !y )
				return;
			if ( !*this )
            {
				*this = T();
                std::swap(**this, *y);
                y = none;
            } else if ( !y )
            {
				y = T();
                std::swap(**this, *y);
                *this = none;
            } else
                std::swap(**this, *y);
		}

		optional(none_t)
			: _bValid(false)
		{}
		~optional()
		{
			if ( _bValid )
				alignedStorage()->~T();
		}
		this_type& operator=(const T& t)
		{
			*this = none;
			new (alignedStorage()) T(t);
			_bValid = true;
			return *this;
		}
		this_type& operator=(none_t)
		{
			if ( _bValid )
			{
				alignedStorage()->~T();
				_bValid = false;
			}
			return *this;
		}
		void reset() //deprecated in boost
		{
			*this = none;
		}
		const T& get() const
		{
			require(!!*this);
			return *alignedStorage();
		}
		T& get()
		{
			require(!!*this);
			return *alignedStorage();
		}
		const T& operator*() const
		{
			require(!!*this);
			return *alignedStorage();
		}
		T& operator*()
		{
			require(!!*this);
			return *alignedStorage();
		}
		const T* operator->() const
		{
			require(!!*this);
			return alignedStorage();
		}
		T* operator->()
		{
			require(!!*this);
			return alignedStorage();
		}
		bool operator!() const { return !_bValid; }
		bool operator==(const this_type& y) const
		{
			return !!*this ? (!!y && **this == *y) : !y;
		}
	private:

		typedef typename ng::mpl::uint_of_size< sizeof(void*) >::type uint_type_same_size_as_ptr;

		T* alignedStorage()
		{
			const uint8_t sp((uint8_t)c_storage_padding);
			const uint8_t s0 = (uint8_t)reinterpret_cast<uint_type_same_size_as_ptr>(_storage) & sp;
			return (T*)(_storage + (((s0 + sp) & ~sp) - s0));
		}
		const T* alignedStorage() const
		{
			const uint8_t sp((uint8_t)c_storage_padding);
			const uint8_t s0 = (uint8_t)reinterpret_cast<uint_type_same_size_as_ptr>(_storage) & sp;
			return (const T*)(_storage + (((s0 + sp) & ~sp) - s0));
		}
		uint8_t _storage[sizeof(T) + c_storage_padding];
		bool _bValid;
	};
}

#endif
