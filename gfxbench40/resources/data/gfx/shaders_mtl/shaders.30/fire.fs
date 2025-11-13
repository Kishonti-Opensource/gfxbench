/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */

#if defined(OSX)
#define SAMPLER_WORKAROUND
constexpr sampler osx_sampler(coord::normalized, filter::linear, address::clamp_to_edge);
#endif

fragment _float4 shader_main(ParticleVertexOutput input [[stage_in]],
							constant EmitterAdvectConsts* emitterConsts [[buffer(14)]],
							texture3d<_float> unit0 [[texture(8)]], //MUST MATCH TEXTURE_3D_SLOT in mtl_shader_constant_layout.h
// MTL_TODO
#if defined(SV_30) && defined(IOS)
							sampler sam [[sampler(0)]])
#else
							sampler sam [[sampler(8)]])
#endif
{
	_float4 frag_color;
	
#if defined(SAMPLER_WORKAROUND)
	_float texCol = unit0.sample(osx_sampler, hfloat3(input.out_UVxyz_Visibility.xyz) ).x;
#else
	_float texCol = unit0.sample(sam, hfloat3(input.out_UVxyz_Visibility.xyz) ).x;
#endif
	texCol *= input.out_UVxyz_Visibility.w;
	frag_color.xyz = texCol * _float3(emitterConsts[0].emitter_color.xyz);
	frag_color.w = texCol;
	
#ifdef SV_31
	frag_color.xyz *= 4.0 ;
	RGBA10A2_ENCODE(frag_color.xyz) ;
	frag_color = max(frag_color, _float4(0.0)) ;
#endif
    
	//return float4(1.0, 1.0, 1.0, 1.0);
	return frag_color;
}


