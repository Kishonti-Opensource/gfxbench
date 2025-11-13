/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */

#include "pp_hdr.h"

#include "hdr_luminance.h"
#ifdef SV_40
#include "hdr_common40.h"
#else
#include "hdr_common.h"
#endif

struct PPHDRInOut
{
	hfloat4 position [[position]];
#ifdef SV_40
	hfloat2 out_texcoord0;
#else
	v_float2 out_texcoord0;
#endif
};

#ifdef TYPE_fragment


#ifdef SV_40
#include "rgbm_helper.h"
#endif

constexpr sampler sampler0(coord::normalized, filter::linear, address::clamp_to_edge);
#ifdef SV_40
constexpr sampler sampler3(coord::normalized, filter::linear, address::clamp_to_edge);
#endif

fragment _float4 shader_main(         PPHDRInOut               in            [[ stage_in ]],
		                              texture2d<_float>        texture_unit0 [[ texture( PP_HDR_HDR_COLOR_TEXTURE_SLOT ) ]],
					                //sampler                  sampler0      [[ sampler( PP_HDR_HDR_COLOR_SAMPLER_SLOT ) ]],
					                  texture2d<_float>        texture_unit2 [[ texture( PP_HDR_BLOOM_TEXTURE_SLOT )     ]],
#ifdef SV_40
					                  texture2d<_float>        texture_unit3 [[ texture( 3 )     ]],
#endif
					                  sampler                  sampler2      [[ sampler( PP_HDR_BLOOM_SAMPLER_SLOT )     ]],
					         constant HDRConsts*               hdr_consts    [[ buffer ( PP_HDR_HDRCONTS_BFR_SLOT )      ]],
					         constant hfloat4*                 adaptive_avg_pad2 [[ buffer ( PP_HDR_LUMINANCE_BFR_SLOT ) ]])
{
	_float4 final_color = _float4(0.0);

    // Color texture
	_float4 c0 = texture_unit0.sample( sampler0, hfloat2(in.out_texcoord0) );

    // Bloom textures
#ifdef SV_40
	_float3 b1 = texture_unit2.sample( sampler2, hfloat2(in.out_texcoord0), bias(0.0) ).xyz;
	_float3 b2 = texture_unit2.sample( sampler2, hfloat2(in.out_texcoord0), bias(1.0) ).xyz;
	_float3 b3 = texture_unit2.sample( sampler2, hfloat2(in.out_texcoord0), bias(2.0) ).xyz;
	_float3 b4 = texture_unit2.sample( sampler2, hfloat2(in.out_texcoord0), bias(3.0) ).xyz;
#else
	_float3 b1 = texture_unit2.sample( sampler2, hfloat2(in.out_texcoord0), level(0.0) ).xyz;
	_float3 b2 = texture_unit2.sample( sampler2, hfloat2(in.out_texcoord0), level(1.0) ).xyz;
	_float3 b3 = texture_unit2.sample( sampler2, hfloat2(in.out_texcoord0), level(2.0) ).xyz;
	_float3 b4 = texture_unit2.sample( sampler2, hfloat2(in.out_texcoord0), level(3.0) ).xyz;
#endif

    //////////////////
	// Bloom debug
	bool a1 = (in.out_texcoord0.x < 0.5) && (in.out_texcoord0.y < 0.5);
	bool a2 = (in.out_texcoord0.x > 0.5) && (in.out_texcoord0.y < 0.5);
	bool a3 = (in.out_texcoord0.x < 0.5) && (in.out_texcoord0.y > 0.5);
	bool a4 = (in.out_texcoord0.x > 0.5) && (in.out_texcoord0.y > 0.5);
#ifdef SV_40
	if (a1) {
		hfloat2 tc = 2.0*in.out_texcoord0;
		final_color = texture_unit2.sample( sampler2, tc, level(0.0) );
	} else if(a2) {
		hfloat2 tc = 2.0*(in.out_texcoord0+hfloat2(-0.5,0));
		final_color = texture_unit2.sample( sampler2, tc, level(1.0) );
	} else if (a3) {
		hfloat2 tc = 2.0*(in.out_texcoord0+hfloat2(0,-0.5));
		final_color = texture_unit2.sample( sampler2, tc, level(2.0) );
	} else if (a4) {
		hfloat2 tc = 2.0*(in.out_texcoord0+hfloat2(-0.5,-0.5));
		final_color = texture_unit2.sample( sampler2, tc, level(3.0) );
	}
#else
	if (a1) {
		_float2 tc = _float2(2.0)*in.out_texcoord0;
		final_color = texture_unit2.sample( sampler2, hfloat2(tc), level(0.0) );
	} else if(a2) {
		_float2 tc = _float2(2.0)*(in.out_texcoord0+_float2(-0.5,0));
		final_color = texture_unit2.sample( sampler2, hfloat2(tc), level(1.0) );
	} else if (a3) {
		_float2 tc = _float2(2.0)*(in.out_texcoord0+_float2(0,-0.5));
		final_color = texture_unit2.sample( sampler2, hfloat2(tc), level(2.0) );
	} else if (a4) {
		_float2 tc = _float2(2.0)*(in.out_texcoord0+_float2(-0.5,-0.5));
		final_color = texture_unit2.sample( sampler2, hfloat2(tc), level(3.0) );
	}
#endif
    //////////////////

#ifdef SV_40
	_float3 bloom = (b1 + b2 + b3 + b4) * 0.25; //0.05 * b1 + 0.1 * b2 + 0.2 * b3 + 0.4 * b4; //TODO: expose bloom magnitude setting
	bloom = powr(bloom, _float3(2.0)) * 50.0;
#endif
	
#if defined(SV_40) && !defined(LIGHTCOMBINE_FP_RENDERTARGET_ENABLED)
	hfloat3 solid_color = hfloat3(RGBDtoRGB_lightcombine( c0 ));
	_float4 transparent_accum = texture_unit3.sample(sampler3, in.out_texcoord0);
	transparent_accum.xyz = powr(transparent_accum.xyz, _float3(2.2)); //convert back to linear values

	//tonemap and add bloom
	hfloat3 ldr_solid_color = compose(solid_color, hfloat3(bloom), hdr_consts, adaptive_avg_pad2);
	hfloat3 transp_color = compose(hfloat3(transparent_accum.xyz), hfloat3(bloom), hdr_consts, adaptive_avg_pad2);

	//programmable bleding of offscreen transparents with premultiplied alpha :)
	hfloat3 merged = transp_color * 1.0 + ldr_solid_color * (1.0 - transparent_accum.w);

	hfloat3 fc = merged.xyz;
#elif defined(SV_40)
	_float3 fc = compose(RGBDtoRGB_lightcombine( c0 ), bloom, hdr_consts, adaptive_avg_pad2);
#else  // SV_31
	_float3 fc = compose(c0.xyz,b1,b2,b3,b4,hdr_consts, adaptive_avg_pad2);
#endif

#ifdef SV_40
#ifdef DRAW_GS_WIREFRAME
	fc = RGBDtoRGB_lightcombine(c0);
#endif
#else
	//fc = c0.xyz;	
#endif

    // NOTE: Comment this out to debug bloom textures
	final_color = _float4( _float3(fc), 1.0);

    // NOTE: Remove comment to disable bloom
	//final_color = _float4(tone_map(c0.xyz * 4.0), 1.0); // uncompress GL_RGB10_A2

#ifdef USE_SHADER_GAMMA_CORRECTION
	final_color.x = powr(final_color.x, _float(0.45));
	final_color.y = powr(final_color.y, _float(0.45));
	final_color.z = powr(final_color.z, _float(0.45));
#endif

	return final_color;
}
#endif

#ifdef TYPE_vertex

vertex PPHDRInOut shader_main(	PPVertexInput input [[ stage_in ]] )
{
	PPHDRInOut out;

	out.position = _float4(input.in_pos, 0.0, 1.0);
#ifdef SV_40
	out.out_texcoord0 = hfloat2( input.in_pos.xy * 0.5 + hfloat(0.5) );
#else
	out.out_texcoord0 = v_float2( input.in_pos.xy * 0.5 + _float(0.5) );
#endif
	out.out_texcoord0.y = 1.0 - out.out_texcoord0.y ;
	
	return out;
}

#endif
