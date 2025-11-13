/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
layout(std140) uniform emitterAdvectConsts
{
	ivec4 particleBufferParamsXYZ_pad;
	
	vec4 emitter_apertureXYZ_focusdist;
	mat4 emitter_worldmat;

	vec4 emitter_min_freqXYZ_speed;
	vec4 emitter_max_freqXYZ_speed;
	vec4 emitter_min_ampXYZ_accel;
	vec4 emitter_max_ampXYZ_accel;

	vec4 emitter_externalVel_gravityFactor;

	vec4 emitter_maxlifeX_sizeYZ_pad;
}; 