/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */


vertex MBlurInOut shader_main( VertexInput    vi [[ stage_in ]],
							  constant VertexUniforms *vu [[ buffer(VERTEX_UNIFORM_DATA_INDEX) ]],
							  constant SkeletalData   *sd [[ buffer(SKELETAL_DATA_INDEX) ]],
							  constant SkeletalMotionBlurData* smbd [[ buffer(SKELETAL_MOTION_BLUR_DATA_INDEX) ]])
{
    MBlurInOut r ;
  
#ifdef MBLUR_ALT
	_float4 out_curpos;
	_float4 out_prevpos;
#endif

	_float4 tmp;
	_float3 position;
	_float3 prev_position;
	_float3 normal =  vi.in_normal.xyz;
	_float3 tangent = vi.in_tangent.xyz;

	
	decodeFromByteVec3(normal);
	decodeFromByteVec3(tangent);

#ifdef SKELETAL
	_float4x4 M4;
	_float4x4 prev_M4;
	
	_float4 in_bone_weight = vi.in_bone_weight ;
    uchar4 in_bone_index = vi.in_bone_index ;

#define RESULT M4
#define BONE sd->bones
	{ 
		uchar4 I = uchar4(in_bone_index);
		_float4x4 M0 = _float4x4( BONE[I.x * 3 + 0],BONE[I.x * 3 + 1],BONE[I.x * 3 + 2],_float4( 0.0, 0.0, 0.0, 1.0));
		_float4x4 M1 = _float4x4( BONE[I.y * 3 + 0],BONE[I.y * 3 + 1],BONE[I.y * 3 + 2],_float4( 0.0, 0.0, 0.0, 1.0));
		_float4x4 M2 = _float4x4( BONE[I.z * 3 + 0],BONE[I.z * 3 + 1],BONE[I.z * 3 + 2],_float4( 0.0, 0.0, 0.0, 1.0));
		_float4x4 M3 = _float4x4( BONE[I.w * 3 + 0],BONE[I.w * 3 + 1],BONE[I.w * 3 + 2],_float4( 0.0, 0.0, 0.0, 1.0));
		RESULT = M0 * in_bone_weight.x + M1 * in_bone_weight.y + M2 * in_bone_weight.z + M3 * in_bone_weight.w;
	}

	position = (_float4( vi.in_position.xyz, 1.0) * M4).xyz;

#undef RESULT
#undef BONE

#define RESULT prev_M4
#define BONE smbd->prev_bones
	{
		uchar4 I = uchar4(in_bone_index);
		_float4x4 M0 = _float4x4( BONE[I.x * 3 + 0],BONE[I.x * 3 + 1],BONE[I.x * 3 + 2],_float4( 0.0, 0.0, 0.0, 1.0));
		_float4x4 M1 = _float4x4( BONE[I.y * 3 + 0],BONE[I.y * 3 + 1],BONE[I.y * 3 + 2],_float4( 0.0, 0.0, 0.0, 1.0));
		_float4x4 M2 = _float4x4( BONE[I.z * 3 + 0],BONE[I.z * 3 + 1],BONE[I.z * 3 + 2],_float4( 0.0, 0.0, 0.0, 1.0));
		_float4x4 M3 = _float4x4( BONE[I.w * 3 + 0],BONE[I.w * 3 + 1],BONE[I.w * 3 + 2],_float4( 0.0, 0.0, 0.0, 1.0));
		RESULT = M0 * in_bone_weight.x + M1 * in_bone_weight.y + M2 * in_bone_weight.z + M3 * in_bone_weight.w;
	}

	prev_position = (_float4( vi.in_position.xyz, 1.0) * prev_M4).xyz;

	_float4 r_out_curpos = vu->mvp * _float4( position, 1.0);
	r.out_curpos = v_float4( r_out_curpos ) ;
	r.out_prevpos = v_float4( vu->mvp2 * _float4( prev_position, 1.0) );

#else
	position = vi.in_position.xyz;

	_float4 r_out_curpos = vu->mvp * _float4( position, 1.0);
	r.out_curpos = v_float4( r_out_curpos ) ;
	r.out_prevpos = v_float4( vu->mvp2 * _float4( position, 1.0) ) ;

#endif


#ifdef MBLUR_ALT

	out_diff = (out_curpos.xy / out_curpos.w) - (out_prevpos.xy / out_prevpos.w);

#endif
	
	r.out_position = hfloat4( r_out_curpos );
    
    return r;
}

