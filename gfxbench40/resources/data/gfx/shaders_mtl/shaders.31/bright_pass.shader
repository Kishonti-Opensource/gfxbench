/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifdef TYPE_compute

#include "hdr_common.h"
#include "bright_pass.h"


kernel void shader_main(         texture2d<mfloat>                 in_tex               [[ texture( BRIGHT_PASS_INPUT_TEXTURE_SLOT  ) ]],
			                     sampler                           input_sampler        [[ sampler( BRIGHT_PASS_INPUT_SAMPLER_SLOT  ) ]],
				                 texture2d<mfloat, access::write>  out_level0_tex       [[ texture( BRIGHT_PASS_OUTPUT_TEXTURE_SLOT ) ]],
				        constant HDRConsts*                        hdr_consts           [[ buffer ( BRIGHT_PASS_HDRCONTS_BFR_SLOT   ) ]],
				        constant hfloat4*                          adaptive_avg_pad2    [[ buffer ( BRIGHT_PASS_LUMINANCE_BFR_SLOT  ) ]],
                                 uint2                             global_invocation_id [[ thread_position_in_grid                    ]])
{
	uint2 g_i = global_invocation_id.xy;	
	
    // first +0.5 pixelwise fetch
    // second +0.5 use texture sampler to calc the average
    _float2 uv = (_float2(2 * g_i) + 1.0) * STEP_UV;
#ifdef SV_40
	_float3 res = RGBDtoRGB_lightcombine ( texture( in_tex, uv) ) ;
#else // SV_31
	_float3 res = RGBA10A2_DECODE( _float3( in_tex.sample( input_sampler, uv).xyz ) ); //decompress from GL_RGB10_A2
#endif
	
	res = bright_pass(res, hdr_consts, adaptive_avg_pad2);

	out_level0_tex.write(mfloat4( mfloat3(res), 1.0),g_i) ;
}

#endif
