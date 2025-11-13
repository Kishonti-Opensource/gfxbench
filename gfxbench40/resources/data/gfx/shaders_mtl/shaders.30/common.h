/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */

#include <metal_stdlib>

using namespace metal;


#if 1

#define RGBA10A2_ENCODE_CONST 0.25
#define RGBA10A2_DECODE_CONST 4.00

#define RGBA10A2_ENCODE(X) X *= RGBA10A2_ENCODE_CONST ;
#define RGBA10A2_DECODE(X) X * RGBA10A2_DECODE_CONST

#else

#define RGBA10A2_ENCODE_CONST 1.00
#define RGBA10A2_DECODE_CONST 1.00

#define RGBA10A2_ENCODE(X) ;
#define RGBA10A2_DECODE(X) X

#endif

//
//
//	Common structures
//
//


//MTL_TODO MUST MATCH STRUCT IN particleAdvect and mtl_emitterAdvectConst.h!
#define MAX_SUBSTEPS 5 
//MUST match with kcl_particlesystem2.h/KCL::_emitter::max_num_substeps !!!!
struct EmitterAdvectConsts
{
	hfloat4 emitter_apertureXYZ_focusdist;
	hfloat4x4 emitter_worldmat;
	
	hfloat4 emitter_min_freqXYZ_speed;
	hfloat4 emitter_max_freqXYZ_speed;
	hfloat4 emitter_min_ampXYZ_accel;
	hfloat4 emitter_max_ampXYZ_accel;
	hfloat4 emitter_externalVel_gravityFactor;
	hfloat4 emitter_maxlifeX_sizeYZ_pad;
	hfloat4 emitter_color;
	
	vec<uint,4> particleBufferParamsXYZ_pad[ MAX_SUBSTEPS ]; //startBirthIdx, endBirthIdx, noOverflow
	vec<uint,4> emitter_numSubstepsX_pad_pad_pad;
};


//per frame constants
struct FrameConsts
{
	hfloat4 depth_parameters;
	hfloat4 global_light_dirXYZ_pad;
	hfloat4 global_light_colorXYZ_pad;
	hfloat4 view_dirXYZ_pad;
	hfloat4 view_posXYZ_time;
	hfloat4 background_colorXYZ_fogdensity;
	hfloat4 inv_resolutionXY_pad;
};


struct LightConsts
{
	hfloat4 light_colorXYZ_pad;
	hfloat4 light_posXYZ_pad;
	hfloat4 light_x;
	hfloat4 spotcosXY_attenZ_pad;
	hfloat4x4   shadowMatrix0;
};


struct MaterialConsts
{
	hfloat4 fresnelXYZ_transp;
	hfloat4 matparams_disiseri;
	hfloat4 translate_uv;
};


//MUST MATCH STRUCT IN mtl_meshConst.h!
struct MeshConsts
{
	hfloat4x4 mvp;
	hfloat4x4 mv;
	hfloat4x4 model;
	hfloat4x4 inv_model;
	hfloat4x4 inv_modelview;
	hfloat4 envmapInterpolator;
};


#ifdef INSTANCING

struct CameraConsts
{
	hfloat4x4 vp ;
};

struct InstancedLightConsts
{
	hfloat4x4 model;
    //xyz - color, w - unused
    hfloat4 light_color;
    //xyz - position, w - unused
    hfloat4 light_pos;
	//x - radius, y - atten, z - spotx, w - spoty
    hfloat4 attenuation_parameter;
    //xyz - dir, w - unused
    hfloat4 light_x;
    hfloat4 spot_cos; 
};

#endif


//
//
//	Vertex Input Layouts
//
//


struct VertexInput
{
	hfloat3 in_position  [[attribute(0)]];
	hfloat2 in_texcoord0 [[attribute(1)]];
	hfloat2 in_texcoord1 [[attribute(2)]];
	hfloat3 in_normal    [[attribute(3)]];
	hfloat3 in_tangent   [[attribute(4)]];
};


struct VertexInputInstancing
{
    hfloat4x4 in_instance_mv;
    hfloat4x4 in_instance_inv_mv;
};


struct FogVertexInput
{
	hfloat3 in_position [[attribute(0)]] ;
};


struct LightingVertexInput
{
	hfloat3 in_position [[attribute(0)]] ;
};


struct OCVertexInput
{
	hfloat4 in_position;
};


//MTL_TODO: must match all the other particle shaders!
//Must match mtl_instanced_particle_layout.h!
//NOTE: must match input/output of particleAdvect
struct ParticleVertexInput
{
	hfloat3 in_Pos;
	hfloat3 in_Age01_Speed_Accel;
	hfloat3 in_Amplitude;
	hfloat3 in_Phase;
	hfloat3 in_Frequency;
	hfloat3 in_T;
	hfloat3 in_B;
	hfloat3 in_N;
	hfloat4 in_Velocity;
};


struct PPVertexInput
{
	hfloat2 in_pos [[attribute(0)]] ;
	hfloat2 in_uv  [[attribute(1)]] ;
};




//
//
//	Vertex Outputs Layouts
//
//


struct BillboardVertexOutput
{
	hfloat4 position [[position]];
	v_float2 texcoord0;
};


struct ColorBlurVertexOutput
{
	hfloat4 position [[position]];
	v_float2 texcoord0;
	v_float2 texcoord1;
	v_float2 texcoord2;
};


struct FogVertexOutput
{
	hfloat4 position [[position]];
	hfloat4 out_pos;
	v_float4 out_pos_hom;
	v_float4 shadow_texcoord;
};


struct CommonVertexOutput
{
	hfloat4 position [[position]];
	v_float2 texcoord0;
	//out vec2 out_texcoord1; //doesn't seem to be assigned ever...
	//out vec3 out_texcoord4; //also doesn't seem to be assigned....
	v_float3 view_dir;
	v_float3 normal;
	v_float3 tangent;
//	v_float2 texcoord01;
//	v_float2 texcoord02;
//	v_float2 texcoord03;
//	v_float2 texcoord04;
	v_float3 eye_space_normal;
	v_float3 world_pos;
};


struct LensflareVertexOutput
{
	hfloat4 position [[position]];
	v_float2 texcoord0;
};


struct LightingVertexOutput
{
	hfloat4 position [[position]];
	v_float4 view_dir;
	v_float4 out_pos;
#ifdef INSTANCING
	int instanceID;
#endif
};


struct OCVertexOutput
{
	hfloat4 position [[position]];
	hfloat point_size [[point_size]];
};


struct ParticleVertexOutput
{
	hfloat4 position [[position]];
	v_float4 out_UVxyz_Visibility;
};


struct ParticleAdvectVertexOutput
{
	hfloat4 position [[position]];
};


struct PPVertexOutput
{
	hfloat4 position [[position]];
	v_float2 texcoord0;
};


struct ShadowCaster0VertexOutput
{
	hfloat4 position [[position]];
};


struct SkyVertexOutput
{
	hfloat4 position [[position]];
	v_float2 texCoord;
};

