/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifdef INSTANCING

vertex LightingVertexOutput shader_main( LightingVertexInput input [[ stage_in  ]],
								constant FrameConsts* frameConst [[buffer(10)]],
								constant CameraConsts* cameraConst [[buffer(12)]],
								constant InstancedLightConsts* instancedLightConst [[buffer(13)]],
                                uint iid [[instance_id]])
{
	LightingVertexOutput out;

	int instanceID = iid ;
	out.instanceID = instanceID ;

	_float4x4 mvp = cameraConst->vp * instancedLightConst[instanceID].model;
	_float4 pos = mvp * _float4( input.in_position, 1.0);

	out.position = pos;
	out.out_pos = v_float4( pos );

	pos = instancedLightConst[instanceID].model * _float4( input.in_position, 1.0);
	
	_float3 out_view_dir_xyz = pos.xyz - _float3(frameConst[0].view_posXYZ_time.xyz) ;
	out.view_dir.xyz = v_float3( out_view_dir_xyz) ;
	out.view_dir.w = dot( _float3(frameConst[0].view_dirXYZ_pad.xyz), out_view_dir_xyz);
	
	return out ;
}


#else //INSTANCING

vertex LightingVertexOutput shader_main( LightingVertexInput input [[ stage_in  ]],
								constant FrameConsts* frameConst [[buffer(10)]],
								constant MeshConsts* meshConst [[buffer(12)]])
{
	LightingVertexOutput out;
	//vec4 pos = mvp * vec4( in_position, 1.0);
	
	_float4 p = _float4(input.in_position.xyz, 1.0);
	
	_float4 pos = meshConst->mvp * p;

	out.position = pos;
	
	out.out_pos = v_float4( pos );
	
	//pos = model * vec4( in_position, 1.0);
	pos = meshConst->model * p;

	_float3 out_view_dir_xyz = pos.xyz - _float3(frameConst[0].view_posXYZ_time.xyz) ;
	out.view_dir.xyz = v_float3( out_view_dir_xyz) ;
	out.view_dir.w = dot( _float3(frameConst[0].view_dirXYZ_pad.xyz), out_view_dir_xyz);
	
	return out;
}

#endif //INSTANCING

