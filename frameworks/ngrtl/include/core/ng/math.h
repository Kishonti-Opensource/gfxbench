/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef NG_BASE_MATH_INCLUDED
#define NG_BASE_MATH_INCLUDED

#include <cmath>
#include <limits>
#include <stdint.h>
#include <string.h>
#include <float.h>
#include <assert.h>

#ifndef INFINITY
#define INFINITY (DBL_MAX+DBL_MAX)
#endif
#ifndef NAN
#define NAN (INFINITY+INFINITY)
#endif

#if defined _MSC_VER && _MSC_VER < 1800
inline bool isnan(double x) { return _isnan(x) != 0; }
#endif

namespace ng
{

template<typename T>
inline T round(T x) { return floor(x + (T)0.5); }

template<typename T>
inline T frac(T x) { return x - floor(x); }

template<typename T>
T square(const T& x) { return x * x; }

template<typename T>
int sign(const T& x) { return x < (T)0 ? -1 : (x > (T)0 ? 1 : 0); }

template<typename T>
bool isodd(const T& t) { return (t & 1) != 0; }

template<typename T>
bool iseven(const T& t) { return (t & 1) == 0; }

const double c_pi = 3.141592653589793238462643383279502884197;

template<typename T>
inline T rad2deg(T r) { return r * (T)(180.0 / c_pi); }

template<typename T>
inline T deg2rad(T d) { return d * (T)(c_pi / 180.0); }

template<typename T>
inline const T& clamp(const T& x, const T& lo, const T& hi)
{
	assert(lo <= hi);
	if ( x <= lo ) return lo;
	else if ( x >= hi ) return hi;
	return x;
}

//return:
//0 -> 1
//1 -> 1
//2 -> 2
//3- > 4
//4 -> 4
//v must be <= 0x80000000
uint32_t NextPowerOf2(uint32_t v);

//0 -> 0
//1 -> 1
//2 -> 2
//3 - > 2
//0xffffffff -> 32
uint8_t BitWidth0(uint32_t number);
uint8_t BitWidth0(uint64_t number);

//0 -> 1
//1 -> 1
//2 -> 2
//3 - > 2
inline uint8_t BitWidth1(uint32_t number)
{
	return number == 0 ? 1 : BitWidth0(number);
}
inline uint8_t BitWidth1(uint64_t number)
{
	return number == 0 ? 1 : BitWidth0(number);
}


uint8_t BitWidth0(uint32_t number);
uint8_t BitWidth0(uint64_t number);

inline uint8_t BitWidth0(int32_t number) { return BitWidth0((uint32_t)number); }
inline uint8_t BitWidth1(int32_t number) { return BitWidth1((uint32_t)number); }
inline uint8_t BitWidth0(int64_t number) { return BitWidth0((uint64_t)number); }
inline uint8_t BitWidth1(int64_t number) { return BitWidth1((uint64_t)number); }

}

#endif
