/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
//WARNING: DO NOT INCLUDE THIS DIRECTLY, include mtl_shader_constant_layouts.h instead!


//MAX_SUBSTEPS must match value in kcl_particlesystem2.h/KCL::_emitter::max_num_substeps !!!!
#define MAX_SUBSTEPS 5

struct EmitterAdvectConsts
{
	vec4 emitter_apertureXYZ_focusdist;
	mat4 emitter_worldmat;
	
	vec4 emitter_min_freqXYZ_speed;
	vec4 emitter_max_freqXYZ_speed;
	vec4 emitter_min_ampXYZ_accel;
	vec4 emitter_max_ampXYZ_accel;
	
	vec4 emitter_externalVel_gravityFactor;
	
	vec4 emitter_maxlifeX_sizeYZ_pad;
	vec4 emitter_color;
	
	ivec4 particleBufferParamsXYZ_pad[MAX_SUBSTEPS];
	ivec4 emitter_numSubstepsX_pad_pad_pad;
};

//must match this, in particleAdvect.vs, fire, flame, smoke
/*
 float4 emitter_apertureXYZ_focusdist;
 float4 emitter_worldmat[4];
 
 float4 emitter_min_freqXYZ_speed;
 float4 emitter_max_freqXYZ_speed;
 float4 emitter_min_ampXYZ_accel;
 float4 emitter_max_ampXYZ_accel;
 
 float4 emitter_externalVel_gravityFactor;
 
 float4 emitter_maxlifeX_sizeYZ_pad;
 vec<uint,4> particleBufferParamsXYZ_pad[ MAX_SUBSTEPS ]; //startBirthIdx, endBirthIdx, noOverflow
 vec<uint,4> emitter_numSubstepsX_pad_pad_pad;
 */

#undef MAX_SUBSTEPS