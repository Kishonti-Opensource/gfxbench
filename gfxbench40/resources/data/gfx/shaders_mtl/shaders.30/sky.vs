/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */


vertex SkyVertexOutput shader_main(VertexInput input [[ stage_in  ]],
								constant MeshConsts* meshConst [[buffer(12)]],
								uint vid [[vertex_id]])
{
	//gl_Position = mvp * vec4( in_position, 1.0);
	SkyVertexOutput out;
	
	_float4 p = _float4(input.in_position.xyz, 1.0);
	out.position = meshConst[0].mvp * p;
	
	out.position.z = out.position.w;
	out.texCoord = v_float2(input.in_texcoord0.xy);
	
	return out;
}

