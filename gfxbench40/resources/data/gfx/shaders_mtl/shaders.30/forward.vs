/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */

typedef vec<uint,2> ivec2;
typedef vec<uint,3> ivec3;
typedef vec<uint,4> ivec4;


constant _float2 wave0 = _float2(  1.01, 1.08);
constant _float2 wave1 = _float2(  -1.02,   -1.02 );
constant _float2 wave2 = _float2( -1.03,  1.03 );
constant _float2 wave3 = _float2(  1.05,  -1.07 );

vertex CommonVertexOutput shader_main(VertexInput input [[ stage_in  ]],
								#ifdef INSTANCING
								constant VertexInputInstancing* instancedData [[buffer(1)]],
								#endif
								constant FrameConsts* frame [[buffer(10)]],
								constant MaterialConsts* material [[buffer(11)]],
								constant MeshConsts* mesh [[buffer(12)]],
								uint iid [[instance_id]])

{
	CommonVertexOutput out;
	_float4 tmp;
	_float3 position;
	_float3 normal = input.in_normal.xyz;
	_float3 tangent = input.in_tangent.xyz;
	
	//decodeFromByteVec3(normal);
	//decodeFromByteVec3(tangent);
	
	position = input.in_position.xyz;
	
    _float4 normal4 = _float4( normal, 0.0);
    _float4 tangent4 = _float4( tangent, 0.0);
	
    _float4 position4 = _float4( position, 1.0);
	
#ifdef INSTANCING
    //mat4 mvp2 = mvp * in_instance_mv;
	_float4x4 mvp2 = mesh[0].mvp * instancedData[iid].in_instance_mv;
	out.position = mvp2 * position4;
	
#else
	//gl_Position = mvp * vec4( position, 1.0);
	out.position = mesh[0].mvp * position4;
#endif
    //rdar://16358689
	//out.position.xy *= _float(-1.0);

	out.texcoord0 = v_float2(input.in_texcoord0.xy);
	
#if defined TRANSLATE_UV
	out.texcoord0 += material[0].translate_uv.xy;
#endif
	
#ifdef INSTANCING
	_float4 world_position = instancedData[iid].in_instance_mv * position4;
	
	out.view_dir = v_float3(frame[0].view_posXYZ_time.xyz - world_position.xyz);
	
	tmp = normal4 * instancedData[iid].in_instance_inv_mv;
	out.normal = v_float3(tmp.xyz);
	
	tmp = tangent4 * instancedData[iid].in_instance_inv_mv;
	out.tangent = v_float3(tmp.xyz);
	
#else
	//vec4 world_position = model * vec4( position, 1.0);
	_float4 world_position = mesh->model * position4;
	
	out.view_dir = v_float3(frame->view_posXYZ_time.xyz - world_position.xyz);
	
	//MTL TODO: reverse multiply here?
	
	//tmp = vec4( normal, 0.0) * inv_model;
	//tmp = mesh->inv_model * normal4;
	tmp = normal4 * mesh->inv_model;
	out.normal = v_float3(tmp.xyz);
	
	//tmp = vec4( tangent, 0.0) * inv_model;
	//tmp =  mesh->inv_model * tangent4;
	tmp = tangent4 * mesh->inv_model;
	out.tangent = v_float3(tmp.xyz);
#endif
	
#ifdef ANIMATE_NORMAL
	out.texcoord01 = out.texcoord0 * _float(1.3) + (_float(4.0) * _float(frame[0].view_posXYZ_time.w)) * wave0;
	out.texcoord02 = out.texcoord0 * _float(1.5) + (_float(4.0) * _float(frame[0].view_posXYZ_time.w)) * wave1;
	out.texcoord03 = out.texcoord0 * _float(3.0) + (_float(4.0) * _float(frame[0].view_posXYZ_time.w)) * wave2;
	out.texcoord04 = out.texcoord0 * _float(1.1) + (_float(4.0) * _float(frame[0].view_posXYZ_time.w)) * wave3;
#endif
	
	return out;
}


