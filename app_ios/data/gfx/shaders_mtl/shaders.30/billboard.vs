/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */


vertex BillboardVertexOutput shader_main(VertexInput input [[ stage_in  ]],
		  constant VertexInputInstancing* instance_data [[buffer(1)]],
		  constant FrameConsts* frameConst [[buffer(10)]],
		  constant MeshConsts* meshConst [[buffer(12)]],
		  uint iid [[instance_id]])
{
	BillboardVertexOutput out;
	_float3 up;
	_float3 right;
	
#ifdef AALIGNED
	up = _float3( 0.0, 1.0, 0.0);
#else
	//up = float3( mv[0][1], mv[1][1], mv[2][1]);
	up = _float3( meshConst[0].mv[0][1], meshConst[0].mv[1][1], meshConst[0].mv[2][1]);
#endif
	
	//right = vec3( mv[0][0], mv[1][0], mv[2][0]);
	right = _float3( meshConst[0].mv[0][0], meshConst[0].mv[1][0], meshConst[0].mv[2][0]);
	
	//vec3 delta = vec3( model[3][0], model[3][1], model[3][2]) - view_pos;
	_float3 delta = _float3( meshConst[0].model[3][0], meshConst[0].model[3][1], meshConst[0].model[3][2]) - _float3( frameConst[0].view_posXYZ_time.xyz );
	delta = normalize(delta);
	
	_float3 position = input.in_position.x * right + input.in_position.y * up + input.in_position.z * delta;
	
    _float4 p = _float4(position, 1.0);
	
#ifdef INSTANCING
    //mat4 mvp2 = matMul(meshConst[0].mvp, in_instance_mv);
	
	//not sure if mul * mul works correctly, so split it out here
	p = instance_data[iid].in_instance_mv * p;
	out.position = hfloat4( _float4x4(meshConst->mvp) * p );
#else
	out.position = hfloat4( _float4x4(meshConst->mvp) * p );
#endif
	
	out.texcoord0 = v_float2( input.in_texcoord0.xy );
	
	return out;
}

