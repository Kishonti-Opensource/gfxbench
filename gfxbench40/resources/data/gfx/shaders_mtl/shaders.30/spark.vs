/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */

vertex ParticleVertexOutput shader_main(constant hfloat4* in_pos_uv [[buffer(0)]],
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
	ParticleVertexOutput out;
	_float3 up;
	_float3 right;
	_float3 fwd;
	
	//axis aligned, camera facing quad
	up = instances[iid].in_Velocity.xyz;
	_float3 camFwd = _float3(meshConst[0].mv[0][2], meshConst[0].mv[1][2], meshConst[0].mv[2][2]);
	
	right = cross(up, camFwd);
	
	_float size = mix(_float(emitterConst[0].emitter_maxlifeX_sizeYZ_pad.y), _float(emitterConst[0].emitter_maxlifeX_sizeYZ_pad.z), clamp(instances[iid].in_Age01_Speed_Accel.x,_float(0.0),_float(1.0)));
	
	_float width = 0.3;
	_float height = 0.3;
	_float3 my_position = instances[iid].in_Pos + size * in_pos_uv[vid].x * width * right + size * in_pos_uv[vid].y * height * up;
	
    _float4 my_position4 = _float4( my_position, 1.0);
	
	out.position = meshConst[0].mvp * my_position4;
	
	out.out_UVxyz_Visibility.xy = v_float2(in_pos_uv[vid].zw) ;
	out.out_UVxyz_Visibility.z = _float(1.0) - _float(instances[iid].in_Age01_Speed_Accel.x);
	
	return out;
}


