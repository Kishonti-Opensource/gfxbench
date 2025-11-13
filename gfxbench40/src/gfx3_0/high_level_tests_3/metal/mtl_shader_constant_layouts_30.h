/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef MTL_SHADER_CONSTANT_LAYOUTS
#define MTL_SHADER_CONSTANT_LAYOUTS

#include "kcl_math3d.h"
#include "mtl_globals.h"

struct VectorI4D
{
	union
	{
		int v[4];
		struct
		{
			int x, y, z, w;
		};
	};
};

typedef KCL::Matrix4x4 mat4;
typedef KCL::Vector4D vec4;
typedef KCL::Vector3D vec3;
typedef KCL::Vector2D vec2;

typedef VectorI4D ivec4;
#include "mtl_frameConsts.h"
#include "mtl_matConsts.h"
#include "mtl_meshConsts.h"
#include "mtl_lightConsts.h"
#include "mtl_emitterAdvectConsts.h"
#include "mtl_instanced_particle_layout.h"

typedef NS_ENUM(NSUInteger, BufferIndicies)
{
    VERTEX_DATA_INDEX = 0,
    INSTANCE_DATA_INDEX = 1,
	COLOR_DATA_INDEX = 3,
	COLOR_BLUR_OFFSET_INDEX = 5,
	LENSFLARE_LIGHT_POS_INDEX = 6,
    DEPTH_OF_FIELD_CONST_INDEX = 9,
    FRAME_CONST_INDEX = 10,
    MATERIAL_CONST_INDEX = 11,
    MESH_CONST_INDEX = 12,
    LIGHT_CONST_INDEX = 13,
    EMITTER_ADVECT_CONST_INDEX = 14
};

const NSUInteger XFB_PARTICLE_SOURCE_DATA_INDEX = 0; //in particleAdvect, the buffer_index of the source data, indexed by vid
const NSUInteger XFB_PARTICLE_DEST_DATA_INDEX = 2; //in particleAdvect, the buffer_index of the destination data, indexed by vid

// Light Pass Texture Slots
typedef NS_ENUM(NSUInteger, LightTextureSlots)
{
    kLightPassColorMapSlot = 0,
    kLightPassNormalMapSlot,
    kLightPassDepthMapSlot,
    kLightPassParamMapSlot,
    NUM_LIGHT_PASS_SLOTS
};

// Reflection-Emission Filter Texture Slots
typedef NS_ENUM(NSUInteger, ReflectionEmissionTextureSlots)
{
    kReflectionEmissionFilterColorMapSlot = 0,
    kReflectionEmissionFilterNormalMapSlot,
    kReflectionEmissionFilterLightSkyMapSlot,
    kReflectionEmissionFilterReflectionMapSlot,
    NUM_REFLECTION_EMISSION_FILTER_SLOTS
};

// Depth-of-Field Filter Texture Slots
typedef NS_ENUM(NSUInteger, DepthOfFieldTextureSlots)
{
    kDepthOfFieldFilterMainBufferSlot = 0,
    kDepthOfFieldFilterBlurBufferSlot,
    kDepthOfFieldFilterDepthBufferSlot,
    NUM_DEPHT_OF_FIELD_FILTER_SLOTS
};


// Dynamic Shadow Pass
const NSUInteger SHADOW_TEXTURE_SLOT = 4;

const char ARRAY_TEXTURE_SLOT = 5;
const char ENVMAP_0_SLOT = 6;
const char ENVMAP_1_SLOT = 7;
const char TEXTURE_3D_SLOT = 8; //MUST MATCH texture3d<float> unit0 [[texture(8)]] in particle rendering shaders!

#endif // MTL_SHADER_CONSTANT_LAYOUTS
