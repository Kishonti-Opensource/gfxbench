/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */

fragment _float4 shader_main(ParticleVertexOutput input [[stage_in]],
							constant EmitterAdvectConsts* emitterConsts [[buffer(14)]],
							texture2d<_float> unit0 [[texture(0)]], //must be 0
							sampler sam [[sampler(0)]])
{

    _float4 emitter_color4 = _float4( _float3(emitterConsts[0].emitter_color.xyz), 1.0);
    
    _float4 frag_color = unit0.sample(sam, hfloat2(input.out_UVxyz_Visibility.xy) ) * _float(input.out_UVxyz_Visibility.z) * emitter_color4;

#ifdef SV_31
	frag_color.xyz *= 3.0 ;
	RGBA10A2_ENCODE(frag_color.xyz) ;
	frag_color = max(frag_color, _float4(0.0)) ;
#endif
 
	return frag_color ;
}



