/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */

fragment _float4 shader_main(SkyInOut inFrag    [[ stage_in ]],
							texture2d<_float> texture_unit0 [[texture(0)]],
							sampler sampler0 [[sampler(0)]] )
{
    _float4 result ;
    
    result = texture_unit0.sample(sampler0, hfloat2(inFrag.out_texcoord0) );
	result.w = 0.0;
    
    return result ;
}


