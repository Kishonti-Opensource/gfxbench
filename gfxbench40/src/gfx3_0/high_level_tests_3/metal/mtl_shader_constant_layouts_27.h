/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */

#include "kcl_math3d.h"
#include "krl_scene.h"

namespace MetalRender
{
    
typedef KCL::Vector2D float2 ;
typedef KCL::Vector3D float3 ;
typedef KCL::Vector4D float4  ;
typedef KCL::Matrix4x4 float4x4 ;

    const int MAX_BONES = 3*32 ;
    const int PARTICLE_DATA_COUNT = 50 ;
    
#define VERTEX_UNIFORM_DATA_INDEX   10
#define FRAGMENT_UNIFORM_DATA_INDEX 11
#define SKELETAL_DATA_INDEX         12
#define SKELETAL_MOTION_BLUR_DATA_INDEX         13
#define PARTICLE_DATA_INDEX         14
#define PARTICLE_COLOR_INDEX         15
    

#define SHADOW_TEXTURE_SLOT_0 4
#define SHADOW_TEXTURE_SLOT_1 5
    
#define PLANAR_TEXTURE_SLOT 8

    struct VertexUniforms
    {
        float4x4 mvp;
        float4x4 mvp2;
        float4x4 mv;
        float4x4 model;
        float4x4 inv_model;
        
        float4x4 shadow_matrix0;
        float4x4 shadow_matrix1;
        
        float4   clip_plane;
        float4   view_pos;
        float4   light_pos;
        float4   translate_uv;
        
        float4x4 world_fit_matrix;
        
        float    time;
        float    fog_density;
        float    camera_focus;
        
        float	 __padding ;
    };
    
    
#if 0
    struct SkeletalData
    {
        float4   bones[MAX_BONES];
    };
    
    
    struct SkeletalMotionBlurData
    {
        float4   prev_bones[MAX_BONES];        
    };
    
    
    struct ParticleData
    {
        float4 particle_data[PARTICLE_DATA_COUNT];
        float4 particle_color[PARTICLE_DATA_COUNT];
    };
#endif
    

    struct FragmentUniforms
    {
        float4 global_light_dir;
        float4 global_light_color;
        float4 view_dir;
        float4 background_color;
        
        float4 inv_resolution;
        
        float  diffuse_intensity;
        float  specular_intensity;
        float  specular_exponent;
        float  reflect_intensity;
        
        float  envmaps_interpolator;
        float  transparency;
        float  mblur_mask;
        float  time;
        
        float alpha_threshold ;
        
        float __padding1 ;
        float __padding2 ;
        float __padding3 ;
    };


}
