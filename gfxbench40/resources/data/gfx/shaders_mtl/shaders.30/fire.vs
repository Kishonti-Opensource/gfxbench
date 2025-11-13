/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */

vertex ParticleVertexOutput shader_main(constant _float4* in_pos_uv [[buffer(0)]],
#ifdef IOS
								constant ParticleVertexInput* instances [[buffer(1)]],
#else
								device   ParticleVertexInput* instances [[buffer(1)]],
#endif
								constant MeshConsts* meshConst [[buffer(12)]],
								constant EmitterAdvectConsts* emitterConst [[buffer(14)]],
								uint vid [[vertex_id]],
								uint iid [[instance_id]])
{
	_float3 up;
	_float3 right;
	//unused?
	_float3 fwd;
	ParticleVertexOutput out;

	up = _float3(meshConst[0].mv[0][1], meshConst[0].mv[1][1], meshConst[0].mv[2][1]);
	right = _float3(meshConst[0].mv[0][0], meshConst[0].mv[1][0], meshConst[0].mv[2][0]);
	
	//billboard
//	up = vec3( mv[0][1], mv[1][1], mv[2][1]);
//	right = vec3( mv[0][0], mv[1][0], mv[2][0]);
	
	_float size = mix(emitterConst[0].emitter_maxlifeX_sizeYZ_pad.y, emitterConst[0].emitter_maxlifeX_sizeYZ_pad.z, clamp(instances[iid].in_Age01_Speed_Accel.x,0.0,1.0));
	
	_float3 my_position = instances[iid].in_Pos.xyz + size * in_pos_uv[vid].x * right + size * in_pos_uv[vid].y * up;

	_float4 p = _float4(my_position, 1.0);
	out.position = meshConst[0].mvp * p;

	out.out_UVxyz_Visibility.xyz = v_float3( v_float2(in_pos_uv[vid].zw), instances[iid].in_Age01_Speed_Accel.x);
	out.out_UVxyz_Visibility.w = _float(1.0) - instances[iid].in_Age01_Speed_Accel.x;
	
	return out;
}

