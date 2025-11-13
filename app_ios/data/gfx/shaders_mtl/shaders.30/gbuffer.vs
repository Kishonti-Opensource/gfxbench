/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */

constant _float2 wave0 = _float2(  1.01, 1.08);
constant _float2 wave1 = _float2(  -1.02,   -1.02 );
constant _float2 wave2 = _float2( -1.03,  1.03 );
constant _float2 wave3 = _float2(  1.05,  -1.07 );


#ifdef UBYTE_NORMAL_TANGENT
_float3 decodeFromByteVec3(_float3 myVec)
{

	myVec = _float(2.0) * myVec - _float(1.0);
	return myVec;
}
#endif

vertex CommonVertexOutput shader_main(	VertexInput    input [[ stage_in  ]],
									#ifdef INSTANCING
									constant VertexInputInstancing* instancedData [[buffer(1)]],
									#endif
								 	constant FrameConsts* frameConst [[buffer(10)]],
								 	constant MaterialConsts* matConst [[buffer(11)]],
								 	constant MeshConsts* meshConst [[buffer(12)]],
                                    uint iid [[instance_id]])
{
	CommonVertexOutput out;
	_float4 tmp;
	_float3 position;
	_float3 normal = input.in_normal.xyz;
	_float3 tangent = input.in_tangent.xyz;
	
	_float4 view_posXYZ_time = frameConst[0].view_posXYZ_time;
	
	//always floats, so this isn't needed
	//normal = decodeFromByteVec3(normal);
	//tangent = decodeFromByteVec3(tangent);

	position = input.in_position.xyz;
	
	_float4 normal4 = _float4( normal, 0.0);
    _float4 tangent4 = _float4( tangent, 0.0);
	
#ifdef INSTANCING
    {
        _float4 t = _float4(position.x, position.y, position.z, 1.0f);
        _float4 t2 = instancedData[iid].in_instance_mv * t;
        out.position = meshConst->mvp * t2;
    }
#else
	{
		_float4 t = _float4(position.x, position.y, position.z, 1.0f);
		out.position = meshConst->mvp * t;
	}
#endif

	out.texcoord0 = v_float2( input.in_texcoord0.xy ) ;

#if defined TRANSLATE_UV
	out.texcoord0 += v_float2( matConst[0].translate_uv.xy );
#endif


#ifdef INSTANCING
	_float4 world_position = instancedData[iid].in_instance_mv * _float4(position.xyz, 1.0);

	out.view_dir = v_float3( view_posXYZ_time.xyz - world_position.xyz );

	tmp = normal4 * instancedData[iid].in_instance_inv_mv;
	out.normal = v_float3( tmp.xyz );

	tmp = tangent4 * instancedData[iid].in_instance_inv_mv;
	out.tangent = v_float3( tmp.xyz );

#else
	//vec4 world_position = model * vec4( position, 1.0);
	_float4 world_position = meshConst->model * _float4(position.xyz, 1.0);

	out.view_dir = v_float3( view_posXYZ_time.xyz - world_position.xyz );

	//tmp = vec4( normal, 0.0) * inv_model;
	tmp = normal4 * meshConst->inv_model;
	out.normal = v_float3( tmp.xyz );

	//tmp = vec4( tangent, 0.0) * inv_model;
	tmp = tangent4 * meshConst->inv_model;
	out.tangent = v_float3( tmp.xyz );
#endif


#if defined(ANIMATE_NORMAL) && 0
	
	out.texcoord01 = out.texcoord0 * _float(1.3) + (_float(4.0) * _float(view_posXYZ_time.w)) * wave0;
	out.texcoord02 = out.texcoord0 * _float(1.5) + (_float(4.0) * _float(view_posXYZ_time.w)) * wave1;
	out.texcoord03 = out.texcoord0 * _float(3.0) + (_float(4.0) * _float(view_posXYZ_time.w)) * wave2;
	out.texcoord04 = out.texcoord0 * _float(1.1) + (_float(4.0) * _float(view_posXYZ_time.w)) * wave3;

#endif

#ifdef TRANSITION_EFFECT
	//out_eye_space_normal = (vec4( normal, _float(0.0)) * inv_modelview).xyz;
	_float4 t = _float4(normal.xyz,0.0) * meshConst->inv_modelview;
	out.eye_space_normal = v_float3(t.xyz);
	
	out.world_pos = v_float3( position.xyz * _float(0.3) );

#endif
	
	return out;
}
