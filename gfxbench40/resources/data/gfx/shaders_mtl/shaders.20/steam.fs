/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
fragment _float4 shader_main(ParticleInOut  inFrag    [[ stage_in ]],
							texture2d<_float> texture_unit0 [[texture(0)]],
							sampler sampler0 [[sampler(0)]] )
{
	_float4 result ;

    _float3 texel = texture_unit0.sample(sampler0, hfloat2(inFrag.out_texcoord0) ).xyz;

    result.xyz = texel * inFrag.out_color.xyz;
    result.w = texel.x * inFrag.out_life;
    
    return result ;
}

