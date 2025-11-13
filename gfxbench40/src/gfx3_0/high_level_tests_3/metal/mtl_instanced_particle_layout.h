/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef GFXBench30_mtl_instanced_particle_layout_h
#define GFXBench30_mtl_instanced_particle_layout_h

//NOTE: DO NOT include this file directly! Use mtl_shader_constant_layout.h

//NOTE: Very Important: This must match the struct in all the particle shaders!
//These are:
//particleAdvect
//fire
//spark
//fog
struct ParticleData
{
	vec4 in_Pos;
	vec4 in_Age01_Speed_Accel;
	vec4 in_Amplitude;
	vec4 in_Phase;
	vec4 in_Frequency;
	vec4 in_T;
	vec4 in_B;
	vec4 in_N;
	vec4 in_Velocity;
};

#endif //GFXBench30_mtl_instanced_particle_layout_h
