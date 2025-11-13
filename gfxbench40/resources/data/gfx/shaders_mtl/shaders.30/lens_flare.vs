/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */

vertex LensflareVertexOutput shader_main(VertexInput input [[ stage_in  ]],
								constant _float4* light_pos [[buffer(6)]],
								constant FrameConsts* frameConst [[buffer(10)]],
								constant MeshConsts* meshConst [[buffer(12)]])
{
	LensflareVertexOutput out;
	_float2 size;
	
	_float4 lp = _float4(light_pos[0].xyz, 1.0f);
	
	//vec4 pos0 = mvp * vec4( light_pos, 1);
	_float4 pos0 = meshConst[0].mvp * lp;
	
	pos0.xy /= pos0.w;
	
	size.x = input.in_position.x;
	size.y = input.in_position.y * (_float(1.0) / _float(frameConst[0].inv_resolutionXY_pad.x)) / (_float(1.0) / _float(frameConst[0].inv_resolutionXY_pad.y));
	
	_float2 pos1 = -pos0.xy;
	
	_float2 pos = mix( pos0.xy, pos1, input.in_position.z);
	
	out.position.x = size.x + pos.x;
	out.position.y = size.y + pos.y;
	out.position.z = 0.0;
	out.position.w = 1.0;
	
	out.texcoord0 = v_float2( input.in_texcoord0.xy );
	
	return out;
}


