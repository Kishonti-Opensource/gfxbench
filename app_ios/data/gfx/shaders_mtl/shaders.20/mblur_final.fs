/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */

// MTL_TODO
constexpr sampler motionblur_sampler(coord::normalized, filter::linear, address::clamp_to_edge);

fragment _float4 shader_main(PostProcessInOut  inFrag    [[ stage_in ]],
							texture2d<_float> texture_unit0 [[texture(0)]],
							texture2d<_float> texture_unit1 [[texture(1)]] )
{
	_float4 result ;
	
	const int n = 4;
	_float3 motion = texture_unit1.sample(motionblur_sampler, hfloat2(inFrag.out_texcoord0) ).xyz;
	_float4 texel = texture_unit0.sample(motionblur_sampler, hfloat2(inFrag.out_texcoord0) );
	_float3 color = texel.xyz / _float( n); 

	motion.xy = (motion.xy - _float(0.5)) * _float2( 0.0666667,.125);
	motion.xy *= texel.a;
	
	for( int i=1; i<n; i++)
	{
		_float2 tc = inFrag.out_texcoord0 - motion.xy * _float2(_float(i),_float( -i)) ;
		color += texture_unit0.sample(motionblur_sampler, hfloat2(tc) ).xyz / _float( n); 
	}

	result.xyz = color;
    
    return result ;
}

