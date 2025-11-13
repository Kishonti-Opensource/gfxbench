/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef NG_SCOPED_PTR_INCLUDED
#define NG_SCOPED_PTR_INCLUDED

#include <assert.h>

#include "checked_delete.h"
#include "move.h"


namespace ng
{

template<class T>
class scoped_ptr // noncopyable
{
private:
    T * px;

    typedef scoped_ptr<T> this_type;
	
    NG_MOVABLE_BUT_NOT_COPYABLE(scoped_ptr)

public:

    typedef T element_type;

    explicit scoped_ptr( T * p = 0 ): px( p ) // never throws
    {}

	scoped_ptr(NG_RV_REF(this_type) y)
		: px(y.release())
	{
	}

	this_type& operator=(NG_RV_REF(this_type) y)
	{
		reset(y.release());
		return *this;
	}

	~scoped_ptr() // never throws
    {
        ng_checked_delete( px );
    }

    void reset(T * p = 0) // never throws
    {
        assert( p == 0 || p != px ); // catch self-reset errors
        this_type(p).swap(*this);
    }

    T & operator*() const // never throws
    {
        assert( px != 0 );
        return *px;
    }

    T * operator->() const // never throws
    {
        assert( px != 0 );
        return px;
    }

    T * get() const // never throws
    {
        return px;
    }

	bool operator!() const
	{
		return px == 0;
	}

    void swap(scoped_ptr & b) // never throws
    {
        T * tmp = b.px;
        b.px = px;
        px = tmp;
    }

	T* release()
	{
		T* p(px);
		px = 0;
		return p;
	}
};

template<class T> inline void swap(scoped_ptr<T> & a, scoped_ptr<T> & b) // never throws
{
    a.swap(b);
}

// get_pointer(p) is a generic way to say p.get()

template<class T> inline T * get_pointer(scoped_ptr<T> const & p)
{
    return p.get();
}


}
#endif



