/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */

vertex FogVertexOutput shader_main( FogVertexInput input [[ stage_in ]],
								constant MeshConsts* meshConst [[buffer(12)]],
                                constant LightConsts* light [[buffer(13)]])
{
	FogVertexOutput out;
	
	_float4 p = _float4(input.in_position.xyz, 1.0);
	out.position = meshConst[0].mvp * p;

	out.out_pos_hom = v_float4(out.position);

    out.out_pos_hom.y *= _float(-1.0);

    out.out_pos.xyz = hfloat3(input.in_position.xyz);
	_float4 temp = meshConst[0].mv * p;
	out.out_pos.w = -temp.z;
	out.shadow_texcoord = v_float4( light[0].shadowMatrix0 * p );

	return out;
}


