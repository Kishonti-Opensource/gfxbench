/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
fragment _float4 shader_main(BikeTrackInOut  inFrag    [[ stage_in ]],
							constant FragmentUniforms *fu [[ buffer(FRAGMENT_UNIFORM_DATA_INDEX) ]],
							texture2d<_float> texture_unit0 [[texture(0)]],
							texture2d<_float> texture_unit1 [[texture(1)]],
							sampler sampler0 [[sampler(0)]] )
{    
    _float3 color = texture_unit0.sample(sampler0, hfloat2(inFrag.out_texcoord0) ).xyz;
	
	if( (_float(1.0) - inFrag.out_texcoord1.y) > _float(fu->alpha_threshold))
	{
		color = _float3( 0.5);
	}

	return _float4( color, 1.0);
}

