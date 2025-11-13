/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */

//#define HALF_SIZED_LIGHTING

#ifdef HALF_SIZED_LIGHTING
struct FragOut
{
	_float4 frag_color0 [[color(0)]];
	_float4 frag_color1 [[color(1)]];
	_float4 frag_color2 [[color(2)]];
	_float4 frag_color3 [[color(3)]];
};

#else
	typedef _float4 FragOut;
#endif

fragment FragOut shader_main(SkyVertexOutput input [[stage_in]],
							 texture2d<_float> unit0 [[texture(0)]],
							 sampler sam [[sampler(0)]])
{
	_float4 out_color = unit0.sample(sam, hfloat2(input.texCoord) );
	out_color.w = 0.0;
    
#ifdef SV_31
	out_color = powr( out_color + _float4(0.1,0.1,0.1,0.0), _float4( 3.0));
#endif
    
	FragOut out;
#ifdef HALF_SIZED_LIGHTING
	out.frag_color0 = out_color;
	out.frag_color1 = _float4(0,0,0,0.6f); //needs emissive due to being unlit
	out.frag_color2 = _float4(0,0,0,0);
	out.frag_color3 = _float4(0,0,0,0);
#else
	out = out_color;
#endif

#ifdef SV_31
	RGBA10A2_ENCODE(out.xyz) ;
#endif

	return out;
}

