/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */

#include "gauss_blur_fs.h"

struct GaussBlurInOut
{
	hfloat4 position [[position]];
	v_float2 out_texcoord0;
};


#ifdef TYPE_fragment

fragment _float4 shader_main(GaussBlurInOut in [[stage_in]],
							 constant hfloat* gauss_offsets   [[ buffer (FB_GAUSS_OFFSETS_BFR_SLOT)   ]],
							 constant hfloat* gauss_weights   [[ buffer (FB_GAUSS_WEIGHTS_BFR_SLOT)   ]],
							 constant hfloat2* inv_resolution [[ buffer (FB_INV_RESOLUTION_BFR_SLOT)  ]],
							 constant int* gauss_lod_level    [[ buffer (FB_GAUSS_LOD_LEVEL_BFR_SLOT) ]],
							 texture2d<_float> unit0          [[ texture(FB_IN_TEXTURE_SLOT)          ]],
							 sampler sampler0                 [[ sampler(FB_SAMPLER_SLOT)             ]])
{			
	_float4 s = _float4(0.0);
	
	for (int i = 0; i < KS; i++)
	{
#if defined HORIZONTAL	
		hfloat2 offset = hfloat2(inv_resolution[ gauss_lod_level[0] ].x * gauss_offsets[i],0.0);
#elif defined VERTICAL
		hfloat2 offset = hfloat2(0.0,inv_resolution[ gauss_lod_level[0] ].y * gauss_offsets[i]);
#endif
		_float w = gauss_weights[i];
		
		hfloat2 si = hfloat2(in.out_texcoord0) + offset;
		
		s += w * unit0.sample(sampler0, si, level(_float( gauss_lod_level[0] )) );
	}
	
    return s;
}

#endif

#ifdef TYPE_vertex

vertex GaussBlurInOut shader_main(	PPVertexInput input [[ stage_in ]] )
{
	GaussBlurInOut out;

	out.position = _float4(input.in_pos, 0.0, 1.0);
	out.out_texcoord0 = v_float2( input.in_pos.xy * 0.5 + _float(0.5) );
	
	return out;
}

#endif
