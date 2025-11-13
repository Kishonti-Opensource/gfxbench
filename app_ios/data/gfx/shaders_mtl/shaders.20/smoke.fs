/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
fragment _float4 shader_main(ParticleInOut  inFrag    [[ stage_in ]],
							texture2d<_float> texture_unit0 [[texture(0)]],
							texture2d<_float> texture_unit1 [[texture(1)]],
							sampler sampler0 [[sampler(0)]],
							sampler sampler1 [[sampler(1)]] )
{
	_float4 result ;
	
    _float3 texel = texture_unit0.sample(sampler0, hfloat2(inFrag.out_texcoord0) ).xyz;
	_float3 texel2 = texture_unit1.sample(sampler1, hfloat2(inFrag.world_texcoord) ).xyz;

    result.xyz = clamp(texel.xyz * _float(4.0) + _float(0.25), _float(0.0), _float(1.0) ) * texel2.xyz;
    result.w = texel.x * inFrag.out_life * _float(3.5);
    
    return result ;
}

