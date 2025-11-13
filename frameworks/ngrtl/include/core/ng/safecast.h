/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef NG_SAFECAST_INCLUDED
#define NG_SAFECAST_INCLUDED

#include <limits>

#include "ng/require.h"
#include "ng/format.h"
#include "ng/macros.h"

//Usage:
//When you need to put an integer value into another of different type, like this:
//
//	thisFunctionNeedsUint8( myvector.size() );
//
//the compiler issues a warning. Now instead of directly casting to suppress warning:
//
//	thisFunctionNeedsUint8( (uint8_t)myvector.size() );
//
//write this:
//
//	thisFunctionNeedsUint8( SAFE_CAST<uint8_t>(myvector.size()) );
//
//which checks the actual value in runtime (and suppresses warning)
//
//You can also write, instead of this
//
//	int a;
//	a = myvector.size();
//
//This:
//
//	int a;
//	SAFE_ASSIGN(a, myvector.size());	


//int8_t i = -34;
//unsigned x = SAFE_CAST<unsigned>(i);

template<class R, class A>
R SAFE_CAST(A a)
{
	static_assert(std::numeric_limits<A>::is_specialized, "Argument type must be built-in numeric type");
	static_assert(std::numeric_limits<R>::is_specialized, "Return value type must be a built-in numeric type");
	bool bOk;
	if ( std::numeric_limits<R>::is_integer != std::numeric_limits<A>::is_integer )
	{
		//integer <- floatingpoint
		//floatingpoint <- integer
		const R r = (R)a;
		bOk = (A)r == a;
		if ( bOk ) return r;
	} else if ( !std::numeric_limits<A>::is_integer )
	{
		//floatingpoint <- floatingpoint
		if ( sizeof(R) < sizeof(A) )
		{
			const R r = (R)a;
			bOk = (A)r == a;
			if ( bOk ) return r;
		} else
			bOk = true;
	} else
	{
		//integer <- integer
		if ( std::numeric_limits<A>::is_signed )
		{
			if ( std::numeric_limits<R>::is_signed )
			{
				//signed <- signed
				bOk = sizeof(R) >= sizeof(A) ||
					((A)std::numeric_limits<R>::min() <= a && a <= (A)std::numeric_limits<R>::max());
			} else
			{
				//unsigned <- signed
				bOk = (a >= 0) &&
					( sizeof(R) >= sizeof(A) || a <= (A)std::numeric_limits<R>::max());
			}
		} else
		{
			if ( std::numeric_limits<R>::is_signed )
			{
				//signed <- unsigned
				bOk = sizeof(R) > sizeof(A) || a <= (A)std::numeric_limits<R>::max();
			} else
			{
				//unsigned <= unsigned
				bOk = sizeof(R) >= sizeof(A) || a <= (A)std::numeric_limits<R>::max();
			}
		}
	}

	require(bOk, ng::cstr(ng::format("Casting value of %s to a %s-byte %s")
		% (sizeof(A) == 1 ? (int)a : a)
		% sizeof(R)
		% (std::numeric_limits<R>::is_integer
			? ( std::numeric_limits<R>::is_signed
				? "signed integer"
				: "unsigned integer"
				)
			: "float"
			)
		));

	return (R)a;
}

template<class T, class U>
inline void SAFE_ASSIGN(T& t, U u)
{
	t = SAFE_CAST<T>(u);
}


#endif
