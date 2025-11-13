/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef INCLUDE_GUARD_byteorder_1362822158
#define INCLUDE_GUARD_byteorder_1362822158

#include "ng/macros.h"
#include <stdint.h>

namespace ng
{
	inline void hton(uint32_t x, void* p)
	{
#ifdef NG_LITTLE_ENDIAN
		uint8_t* v = (uint8_t*)p;
		v[0] = x >> 24;
		v[1] = (x >> 16) & 0xff;
		v[2] = (x >> 8) & 0xff;
		v[3] = x & 0xff;
#else
		*(uint32_t*)p = x;
#endif		
	}
	inline void hton(int32_t x, void* p)
	{
		return hton((uint32_t)x, p);
	}
	inline uint32_t ntoh_u32(const void* p)
	{
#ifdef NG_LITTLE_ENDIAN
		const uint8_t* v = (const uint8_t*)p;
		return ((uint32_t)v[0] << 24) |
			((uint32_t)v[1] << 16) |
			((uint32_t)v[2] << 8) |
			v[3];
#else
		return *(const uint32_t*)p;
#endif
	}
	inline int32_t ntoh_i32(const void* p)
	{
		return (int32_t)ntoh_u32(p);
	}


	inline void hton(uint64_t x, void* p)
	{
#ifdef NG_LITTLE_ENDIAN
		uint8_t* v = (uint8_t*)p;
		v[0] = x >> 56;
		v[1] = (x >> 48) & 0xff;
		v[2] = (x >> 40) & 0xff;
		v[3] = (x >> 32) & 0xff;
		v[4] = (x >> 24) & 0xff;
		v[5] = (x >> 16) & 0xff;
		v[6] = (x >> 8) & 0xff;
		v[7] = x & 0xff;
#else
		*(uint64_t*)p = x;
#endif		
	}
	inline void hton(int64_t x, void* p)
	{
		return hton((uint64_t)x, p);
	}

	inline uint64_t ntoh_u64(const void* p)
	{
#ifdef NG_LITTLE_ENDIAN
		const uint8_t* v = (const uint8_t*)p;
		return ((uint64_t)v[0] << 56) |
			((uint64_t)v[1] << 48) |
			((uint64_t)v[2] << 40) |
			((uint64_t)v[3] << 32) |
			((uint64_t)v[4] << 24) |
			((uint64_t)v[5] << 16) |
			((uint64_t)v[6] << 8) |
			v[7];
#else
		return *(const uint64_t*)p;
#endif
	}
	inline int64_t ntoh_i64(const void* p)
	{
		return (int64_t)ntoh_u64(p);
	}
}


#endif

