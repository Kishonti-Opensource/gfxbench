/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */


vertex ColorBlurVertexOutput shader_main( PPVertexInput input [[ stage_in ]],
								constant hfloat2* offset_2d [[buffer(5)]])
{
	ColorBlurVertexOutput out;
	out.position = _float4(input.in_pos, 0.0, 1.0);
	
	_float2 out_texcoord0 = input.in_uv ;
	out.texcoord0 = v_float2( out_texcoord0 );
	out.texcoord1 = v_float2( out_texcoord0 + offset_2d[0] );
	out.texcoord2 = v_float2( out_texcoord0 - offset_2d[0] );
	
	return out;
}

