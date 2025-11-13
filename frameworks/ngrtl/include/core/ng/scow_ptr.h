/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef NG_SCOW_PTR_INCLUDED
#define NG_SCOW_PTR_INCLUDED

#include <assert.h>
#include <stdexcept>
#include "ng/macros.h"
#include "ng/move.h"

#include "ng/checked_delete.h"
#include "ng/mpl/int_of_size.h"

/* ---- scow_ptr ----
   - scoped-or-weak pointer -
   
   This pointer can work like a scow_ptr or
   an plain old raw weak ptr
   
   The pointer stores a flag (bOwner) and behaves accordingly
   
   Implementation detail:
   The flag is stored in the lsb bit of the pointer. That's why
   it is assumed that all pointers are on even addresses.
   If it isn't the case the bit flag must be put into a
   separate bool field
   
*/

namespace ng
{

template<class T>
class scow_ptr // noncopyable
{
private:
    typedef scow_ptr<T> this_type;
	typedef typename mpl::uint_of_size<sizeof(T*)>::type uint_of_ptr_size_t;

	T * _px; //lsb bit is the bOwned flag
	static const uint_of_ptr_size_t ownedMask = 1;
	static const uint_of_ptr_size_t inverseOwnedMask = ~(uint_of_ptr_size_t)1;

    NG_MOVABLE_BUT_NOT_COPYABLE(scow_ptr)

public:

    typedef T element_type;

	scow_ptr()
		: _px(0)
	{}

    scow_ptr(T * p, bool bOwned)
	: _px( bOwned ? (T*)((uint_of_ptr_size_t)p | ownedMask) : p )
    {
		if ( (uint_of_ptr_size_t)p & ownedMask )
			throw std::runtime_error("odd address in scow_ptr");
	}

	scow_ptr(NG_RV_REF(this_type) y)
		: _px(0)
	{
		*this = y;
	}

	this_type& operator=(NG_RV_REF(this_type) y)
	{
		if ( y.owner() )
			reset(y.release(), true);
		else
		{
			reset(y.get(), false);
			y.reset();
		}
		return *this;
	}

	bool owner() const
	{
		return ((uint_of_ptr_size_t)_px & ownedMask) != 0;
	}
	
    ~scow_ptr() // never throws
    {
		if ( owner() )
			ng_checked_delete( get() );
    }

    void reset() // never throws
    {
		this_type tmp;
		swap(tmp);
    }

	void reset(T * p, bool bOwned) // never throws
    {
		if ( p == 0 || p != get() )
		{
			this_type tmp(p, bOwned);
			swap(tmp);
		}
    }

    T & operator*() const // never throws
    {
        return *get();
    }

    T * operator->() const // never throws
    {
        return get();
    }

    T * get() const // never throws
    {
        return (T*)((uint_of_ptr_size_t)_px & inverseOwnedMask);
    }

	bool operator!() const
	{
		return get() == 0;
	}

    void swap(scow_ptr & b) // never throws
    {
        T * tmp = b._px;
        b._px = _px;
        _px = tmp;
    }

	//should be owner (asserted)
	//if not owner returns nullptr
	//default is to clear the ptr. Specify true to keep it as weak
	T* release(bool bKeepAsWeak = false)
	{
		T* px = (T*)((uint_of_ptr_size_t)_px & inverseOwnedMask);
		if ( owner() )
		{
			_px = bKeepAsWeak ? px : nullptr;
			return px;
		} else
		{
			assert(owner());
			_px = bKeepAsWeak ? px : nullptr;
			return nullptr;
		}
	}
};

template<class T> inline void swap(scow_ptr<T> & a, scow_ptr<T> & b) // never throws
{
    a.swap(b);
}

// get_pointer(p) is a generic way to say p.get()

template<class T> inline T * get_pointer(scow_ptr<T> const & p)
{
    return p.get();
}


}
#endif



