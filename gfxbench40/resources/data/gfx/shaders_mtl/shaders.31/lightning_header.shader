/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */

#if defined(OSX)

#define PACK_WORKAROUND

_float2 UIntToFloat2(uint i)
{
	uint li = i & 0xFFFF;
	uint hi = i >> 16;
	
	_float x = _float(li)/65535.0;
	_float y = _float(hi)/65535.0;
	
	return _float2(x,y);
}

uint Float2ToUint(_float2 v)
{
	uint li = round(clamp(v.x, 0.0, 1.0)*65535.0);
	uint hi = round(clamp(v.y, 0.0, 1.0)*65535.0);
	
	return (hi << 16) | li ;
}

#endif // PACK_WORKAROUND


_float pack_w(_float intensity, _float end_point_distance)
{
	// scale to pack the intensity correctly
	intensity /= 2.0 ;
	
#if defined(PACK_WORKAROUND)
	return _float(Float2ToUint(_float2(intensity,end_point_distance)));
#else // PACK_WORKAROUND
	
#if FORCE_HIGHP
	return _float(pack_float_to_unorm2x16(_float2(intensity, end_point_distance)));
#else
	return _float(pack_half_to_unorm2x16(_float2(intensity, end_point_distance)));
#endif

#endif // PACK_WORKAROUND
}

_float2 unpack_w(_float w)
{
    // .x - intensity
    // .y - endpoint distance
#if defined(PACK_WORKAROUND)
	_float2 res = UIntToFloat2(uint(w));
#else // PACK_WORKAROUND
    
#if FORCE_HIGHP
    _float2 res = unpack_unorm2x16_to_float(uint(w));	
#else
	_float2 res = unpack_unorm2x16_to_half(uint(w));
#endif

#endif // PACK_WORKAROUND

	res.x *= 2.0;
	return res ;
}
