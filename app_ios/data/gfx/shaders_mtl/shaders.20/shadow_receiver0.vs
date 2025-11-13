/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */

vertex ShadowReceiver0InOut shader_main(         VertexInput    vi  [[ stage_in ]],
										constant VertexUniforms *vu [[ buffer(VERTEX_UNIFORM_DATA_INDEX) ]],
										constant SkeletalData   *sd [[ buffer(SKELETAL_DATA_INDEX) ]])
{
    ShadowReceiver0InOut r ;
    
    _float3 position;
	_float4 tmp;
	
#ifdef SKELETAL
	_float4x4 M4;
	
#define RESULT M4
#define BONE sd->bones

	_float4 in_bone_weight = vi.in_bone_weight ;
    uchar4 in_bone_index = vi.in_bone_index ;

	{ 
		uchar4 I = uchar4(in_bone_index);
		_float4x4 M0 = _float4x4( BONE[I.x * 3 + 0],BONE[I.x * 3 + 1],BONE[I.x * 3 + 2],_float4( 0.0, 0.0, 0.0, 1.0));
		_float4x4 M1 = _float4x4( BONE[I.y * 3 + 0],BONE[I.y * 3 + 1],BONE[I.y * 3 + 2],_float4( 0.0, 0.0, 0.0, 1.0));
		_float4x4 M2 = _float4x4( BONE[I.z * 3 + 0],BONE[I.z * 3 + 1],BONE[I.z * 3 + 2],_float4( 0.0, 0.0, 0.0, 1.0));
		_float4x4 M3 = _float4x4( BONE[I.w * 3 + 0],BONE[I.w * 3 + 1],BONE[I.w * 3 + 2],_float4( 0.0, 0.0, 0.0, 1.0)); 
		RESULT = M0 * in_bone_weight.x + M1 * in_bone_weight.y + M2 * in_bone_weight.z + M3 * in_bone_weight.w;
	}

	position = (_float4( vi.in_position.xyz, 1.0) * M4).xyz;
#else
	position = vi.in_position.xyz;
#endif
	
	r.out_position = vu->mvp * _float4( position, 1.0);
		
	_float4 world_position = vu->model * _float4( position, 1.0);
	
	_float4 shadow_texcoord = vu->shadow_matrix0 * world_position;
	
	r.out_texcoord0 = v_float3( shadow_texcoord.xyz / shadow_texcoord.w );
    
    return r;
}



