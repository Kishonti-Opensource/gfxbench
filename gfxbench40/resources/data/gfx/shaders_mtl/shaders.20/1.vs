/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */

constant _float2 wave0 = _float2(  1.01, 1.08);
constant _float2 wave1 = _float2(  -1.02,   -1.02 );
constant _float2 wave2 = _float2( -1.03,  1.03 );
constant _float2 wave3 = _float2(  1.05,  -1.07 );


#ifndef ZPREPASS


vertex _1_InOut shader_main(		 VertexInput     vi      [[ stage_in   ]],
							constant VertexUniforms *vu [[ buffer(VERTEX_UNIFORM_DATA_INDEX) ]],
							constant SkeletalData   *sd [[ buffer(SKELETAL_DATA_INDEX) ]])
{
    _1_InOut r ; // result variable
    
    _float2 in_texcoord0 = vi.in_texcoord0 ;
    _float2 in_texcoord1 = vi.in_texcoord1 ;
    
    
	_float4 tmp;
	_float3 position;
	_float3 normal = vi.in_normal.xyz;
	_float3 tangent = vi.in_tangent.xyz;

	
	decodeFromByteVec3(normal);
	decodeFromByteVec3(tangent);

#ifdef SKELETAL
	_float4x4 M4;

#define RESULT M4
#define BONE sd->bones

	_float4 in_bone_weight = vi.in_bone_weight ;
    uchar4 in_bone_index = vi.in_bone_index ;
	
	{ 
		uchar4 I = uchar4(in_bone_index); 
		_float4x4 M0 = _float4x4( BONE[I.x * 3 + 0], BONE[I.x * 3 + 1], BONE[I.x * 3 + 2], _float4( 0.0, 0.0, 0.0, 1.0));
		_float4x4 M1 = _float4x4( BONE[I.y * 3 + 0], BONE[I.y * 3 + 1], BONE[I.y * 3 + 2], _float4( 0.0, 0.0, 0.0, 1.0));
		_float4x4 M2 = _float4x4( BONE[I.z * 3 + 0], BONE[I.z * 3 + 1], BONE[I.z * 3 + 2], _float4( 0.0, 0.0, 0.0, 1.0));
		_float4x4 M3 = _float4x4( BONE[I.w * 3 + 0], BONE[I.w * 3 + 1], BONE[I.w * 3 + 2], _float4( 0.0, 0.0, 0.0, 1.0));
		
		RESULT = M0 * in_bone_weight.x + M1 * in_bone_weight.y + M2 * in_bone_weight.z + M3 * in_bone_weight.w;
	}

	position = (_float4( vi.in_position.xyz, 1.0) * M4).xyz;
	normal = (_float4( normal, 0.0) * M4).xyz;
	tangent = (_float4( tangent, 0.0) * M4).xyz;
#else
	position = vi.in_position.xyz;
#endif


	r.out_position = vu->mvp * _float4( position, 1.0);

	_float2 r_out_texcoord0 = in_texcoord0;

#if defined TRANSLATE_UV
	r_out_texcoord0 += vu->translate_uv.xy;
#endif
	r.out_texcoord0 = v_float2(r_out_texcoord0) ;


#ifdef LIGHTMAP
	r.out_texcoord1 = v_float2( in_texcoord1 );
#endif

#if defined LIGHTING || defined REFLECTION || defined RELIEF

	_float4 world_position = vu->model * _float4( position, 1.0);

	r.out_view_dir = v_float3( vu->view_pos.xyz - world_position.xyz );

	tmp = _float4( normal, 0.0) * vu->inv_model;
	r.out_normal = v_float3( tmp.xyz );

	tmp = _float4( tangent, 0.0) * vu->inv_model;
	r.out_tangent = v_float3( tmp.xyz );


#if defined SHADOW_MAP && defined LIGHTING
	_float4 shadow_texcoord0 = vu->shadow_matrix0 * world_position;
	r.out_texcoord4 = v_float2( shadow_texcoord0.xy / shadow_texcoord0.w );
	r.out_texcoord4z = shadow_texcoord0.z / shadow_texcoord0.w;

	_float4 shadow_texcoord1 = vu->shadow_matrix1 * world_position;
	r.out_texcoord5 = v_float2( shadow_texcoord1.xy / shadow_texcoord1.w );
	r.out_texcoord5z = shadow_texcoord1.z / shadow_texcoord1.w;
#endif

#endif


#if defined FOG
	_float4 fog_position = vu->mv * _float4( position, 1.0);
	r.fog_distance = clamp( -fog_position.z * vu->fog_density, _float(0.0), _float(1.0));
#endif

#if defined DOF
	_float4 distance_position = vu->mv * _float4( position, 1.0);
	dof = clamp( _float(1.0) - abs( -distance_position.z - _float(vu->camera_focus)) / _float(20.0), _float(-1.0), _float(1.0) );
#endif

#ifdef ANIMATE_NORMAL
	r.out_texcoord01 = out_texcoord0 * _float(1.3) + (_float(4.0) * vu->time) * wave0;
	r.out_texcoord02 = out_texcoord0 * _float(1.5) + (_float(4.0) * vu->time) * wave1;
	r.out_texcoord03 = out_texcoord0 * _float(3.0) + (_float(4.0) * vu->time) * wave2;
	r.out_texcoord04 = out_texcoord0 * _float(1.1) + (_float(4.0) * vu->time) * wave3;
#endif
    
    return r;
}


#else  // ZPREPASS


vertex _1_InOut shader_main(VertexInput    vi      [[ stage_in  ]],
							constant VertexUniforms *vu [[ buffer(VERTEX_UNIFORM_DATA_INDEX) ]],
							constant SkeletalData   *sd [[ buffer(SKELETAL_DATA_INDEX) ]])
{
	_1_InOut r ; // result variable
    
    _float2 in_texcoord0 = vi.in_texcoord0.xy ;
    
    
	_float4 tmp;
	_float3 position;

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

	_float2 r_out_texcoord0 = in_texcoord0;

#if defined TRANSLATE_UV
	r_out_texcoord0 += vu->translate_uv.xy;
#endif
	r.out_texcoord0 = v_float2( r_out_texcoord0 );

#if defined RELIEF
	r.out_tangent = v_float3( 1.0, 0.0, 0.0);
	r.out_normal = v_float3( 0.0, 1.0, 0.0);
	r.out_view_dir = v_float3( 0.0, 0.0, 1.0);
#endif

#ifdef ANIMATE_NORMAL
	r.out_texcoord01 = out_texcoord0 * _float(1.3) + (_float(4.0) * time) * wave0;
	r.out_texcoord02 = out_texcoord0 * _float(1.5) + (_float(4.0) * time) * wave1;
	r.out_texcoord03 = out_texcoord0 * _float(3.0) + (_float(4.0) * time) * wave2;
	r.out_texcoord04 = out_texcoord0 * _float(1.1) + (_float(4.0) * time) * wave3;
#endif

	return r ;   
}


#endif

