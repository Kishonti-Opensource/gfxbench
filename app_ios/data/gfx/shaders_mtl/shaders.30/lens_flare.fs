/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */

fragment _float4 shader_main(LensflareVertexOutput input [[stage_in]],
					 constant hfloat4* light_color [[buffer(3)]],
					 texture2d<_float> unit0 [[texture(0)]],
					 sampler sam0 [[sampler(0)]])
{
	_float4 frag_color = unit0.sample(sam0, hfloat2(input.texcoord0) ) * _float4(light_color[0].x,light_color[0].y,light_color[0].z, 1.0) * _float(0.1);
	
#ifdef SV_31
	RGBA10A2_ENCODE(frag_color.xyz) ;
#endif
	
	return frag_color ;
}


