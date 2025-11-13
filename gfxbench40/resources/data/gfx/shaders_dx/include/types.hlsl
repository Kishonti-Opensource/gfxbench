/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#pragma warning( disable : 3578 )

struct VertexShaderInput
{
    float2 texcoord0   : TEX_COORD0;
#ifndef SV_30
    float2 texcoord1   : TEX_COORD1;
#endif
    float3 position    : POSITION;
    float3 normal      : NORMAL;
    float3 tangent     : TANGENT;
#ifdef SKELETAL
	uint4  bone_index  : BONE_INDEX;
	float4 bone_weight : BONE_WEIGHT;	
#elif defined INSTANCING
	float4x4 instance_mv   		: INSTANCE_MV;
	float4x4 instance_inv_mv   	: INSTANCE_INV_MV;
#endif
};

struct MinimalInput
{
	float3 position    : POSITION;
};

struct VertexShaderOutput
{
	//1
    float4 position  : SV_POSITION;
	//2
    float2 texcoord0 : TEX_COORD0;
    float2 texcoord1 : TEX_COORD1;
	//3
    float2 coord0 : COORD0;
    float2 coord1 : COORD1;
	//4
    float3 normal    : NORMAL;
    float coord0z    : COORD0Z;
	//5
    float3 tangent    : TANGENT;
    float coord1z     : COORD1Z;
	//6
    float2 animatecoord0 : ANIMATE_COORD0;
    float2 animatecoord1 : ANIMATE_COORD1;
	//7
    float2 animatecoord2 : ANIMATE_COORD2;
    float2 animatecoord3 : ANIMATE_COORD3;
	//8
    float3 view_dir    : VIEW_DIR;
	float  fog_distance : FOG_DISTANCE;
};

struct GBufferStruct
{
    float4 position  : SV_POSITION;
    float4 color : COLOR;
	float3 normal : NORMAL0;
	float3 tangent : TANGENT;
	float3 worldPos : POSITION;
	float3 viewDir : VIEW_DIR;
    float2 texcoord0 : TEX_COORD0;
	float3 eyeSpaceNormal : NORMAL1;
	float2 glFragCoord : TEX_COORD1;
};

struct LightVSOutput
{
	float4 position : SV_POSITION;
	float4 view_dir : VIEW_DIR;
    float4 screen_coord : TEX_COORD0;
};

struct ScreenVSOutput
{
	float4 position : SV_POSITION;
    float2 texcoord0 : TEX_COORD0;
};

struct FogStruct
{	
	float4 position : SV_POSITION;
	float3 world_pos  : POSITION;
	float4 shadow_texcoord : TEX_COORD0;
    float4 screen_coord : TEX_COORD1;
};

struct particleStruct
{
    float4 position  : SV_POSITION;
    float4 color : COLOR;
    float2 texcoord0 : TEX_COORD0;
    float2 texcoord1 : TEX_COORD1;
    float life : LIFE;
};

struct ParticleRenderData
{
	float2 VertexPos : POSITION0;
	float2 VertexTexCoord : TEX_COORD0;

	float3 InstancePos : TEX_COORD1;
	float InstanceAge : TEX_COORD2;
	float3 Velocity : TEX_COORD5;
};

struct ParticleAnimationData
{
	float3 Pos : TEX_COORD0;
	float3 Age01_Speed_Accel : TEX_COORD1;
	float3 Amplitude : TEX_COORD2;
	float3 Phase : TEX_COORD3;
	float3 Frequency : TEX_COORD4;
	float3 T : TANGENT;
	float3 B : BINORMAL;
	float3 N : NORMAL;
	float3 Velocity : TEX_COORD5;
};

struct ParticleVSOutput
{
	float4 position  : SV_POSITION;
	float3 texcoord0 : TEX_COORD0;
	float  visibility : LIFE;
};

struct MultiTexCoordVertexStruct
{
    float4 position  : SV_POSITION;
	float2 texcoord0 : TEX_COORD0;
    float2 texcoord1 : TEX_COORD1;
    float2 texcoord2 : TEX_COORD2;
    float2 texcoord3 : TEX_COORD3;
};

struct MBlurVertexShaderOutput
{
    float4 position  : SV_POSITION;
	float4 curr_pos  : CURR_POSITION;	
	float4 prev_pos  : PREV_POSITION;
};
