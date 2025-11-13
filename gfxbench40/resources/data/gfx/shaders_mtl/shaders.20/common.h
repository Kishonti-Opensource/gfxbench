/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include <metal_stdlib>
using namespace metal;


//
//
//	Data Types
//
//


#define VERTEX_UNIFORM_DATA_INDEX   10
#define FRAGMENT_UNIFORM_DATA_INDEX 11
#define SKELETAL_DATA_INDEX         12
#define SKELETAL_MOTION_BLUR_DATA_INDEX         13
#define PARTICLE_DATA_INDEX         14
#define PARTICLE_COLOR_INDEX         15

#define MAX_BONES 3*32
#define PARTICLE_DATA_COUNT 50 

struct VertexInput
{
	hfloat3 in_position  [[attribute(0)]];
	hfloat2 in_texcoord0 [[attribute(1)]];
	hfloat2 in_texcoord1 [[attribute(2)]];
	hfloat3 in_normal    [[attribute(3)]];
	hfloat3 in_tangent   [[attribute(4)]];
#ifdef SKELETAL
	hfloat4 in_bone_weight [[attribute(5)]];
	uchar4  in_bone_index  [[attribute(6)]];
#endif
};


struct PPVertexInput
{
	hfloat2 in_pos [[attribute(0)]] ;
	hfloat2 in_uv  [[attribute(1)]] ;
};


struct VertexUniforms
{
	hfloat4x4 mvp;
	hfloat4x4 mvp2;
	hfloat4x4 mv;
	hfloat4x4 model;
	hfloat4x4 inv_model;
	
	hfloat4x4 shadow_matrix0;
	hfloat4x4 shadow_matrix1;
	
	hfloat4   clip_plane;
	hfloat4   view_pos;
	hfloat4   light_pos;
	hfloat4   translate_uv;
	
	hfloat4x4 world_fit_matrix;
	
	hfloat    time;
	hfloat    fog_density;
	hfloat    camera_focus;
	
	hfloat	 __padding ;
};


struct SkeletalData
{
	hfloat4   bones[MAX_BONES];
};


struct SkeletalMotionBlurData
{
	hfloat4   prev_bones[MAX_BONES];        
};


struct ParticleData
{
	hfloat4 d[PARTICLE_DATA_COUNT];
};


struct FragmentUniforms
{
	hfloat4 global_light_dir;
	hfloat4 global_light_color;
	hfloat4 view_dir;
	hfloat4 background_color;
	
	hfloat4 inv_resolution;
	
	hfloat  diffuse_intensity;
	hfloat  specular_intensity;
	hfloat  specular_exponent;
	hfloat  reflect_intensity;
	
	hfloat  envmaps_interpolator;
	hfloat  transparency;
	hfloat  mblur_mask;
	hfloat  time;
	
	hfloat alpha_threshold ;
	
	hfloat __padding1 ;
	hfloat __padding2 ;
	hfloat __padding3 ;
};


#if defined ZPREPASS
struct _1_InOut
{
    hfloat4 out_position  [[position]];
    
    v_float2 out_texcoord0 ;
#if defined RELIEF
    v_float3 out_view_dir;
    v_float3 out_normal;
    v_float3 out_tangent;
#endif
#if defined ANIMATE_NORMAL
    v_float2 out_texcoord01;
    v_float2 out_texcoord02;
    v_float2 out_texcoord03;
    v_float2 out_texcoord04;
#endif
};
#else // !ZPREPASS
struct _1_InOut
{
    hfloat4 out_position  [[position]];

    v_float2 out_texcoord0 ;
#if defined LIGHTMAP
	v_float2 out_texcoord1;
#endif
#if defined SHADOW_MAP && defined LIGHTING
	v_float2 out_texcoord4;
	v_float2 out_texcoord5;
	v_float  out_texcoord4z;
	v_float  out_texcoord5z;
#endif
#if defined LIGHTING || defined REFLECTION || defined RELIEF
	v_float3 out_view_dir;
	v_float3 out_normal;
	v_float3 out_tangent;
#endif
#if defined FOG
	v_float  fog_distance;
#endif
#if defined ANIMATE_NORMAL
    v_float2 out_texcoord01;
    v_float2 out_texcoord02;
    v_float2 out_texcoord03;
    v_float2 out_texcoord04;
#endif
};
#endif // !ZPREPASS


struct BikeTrackInOut
{
    hfloat4 out_position [[position]];
    
    v_float2 out_texcoord0 ;
    v_float2 out_texcoord1 ;
};


struct JungleBackGroundInOut
{
    hfloat4 out_position [[position]];
    
    v_float2 out_texcoord0 ;
	
#if defined FOG
	v_float fog_distance ;
#endif
};


struct MBlurInOut
{
    hfloat4 out_position [[position]];
    
    v_float4 out_curpos  ;
    v_float4 out_prevpos ;
};


struct ParticleInOut
{
    hfloat4 out_position [[position]];
    
    v_float2 out_texcoord0;
#if defined USE_STEAM
	v_float4 out_color;
#endif
	v_float out_life;
#if defined USE_SMOKE
	v_float2 world_texcoord;
#endif
};


struct PostProcessInOut
{
	hfloat4 out_position [[position]];
	v_float2 out_texcoord0 ;
};


struct PlanarReflectionInOut
{
    hfloat4 out_position [[position]];
    
    v_float2 out_texcoord0 ;
#if defined FOG
	v_float  fog_distance;
#endif
};


struct ShadowCaster0InOut
{
    hfloat4 out_position [[position]];
};


struct ShadowReceiver0InOut
{
    hfloat4 out_position [[position]];
    v_float3 out_texcoord0;
};


struct SkyInOut
{
    hfloat4 out_position [[position]];
    v_float2 out_texcoord0 ;
};


//
//
//	Helper functions
//
//

void decodeFromByteVec3(thread _float3 & myVec)
{
#ifdef UBYTE_NORMAL_TANGENT
	myVec = _float(2.0) * myVec - _float(1.0);
#endif
}


