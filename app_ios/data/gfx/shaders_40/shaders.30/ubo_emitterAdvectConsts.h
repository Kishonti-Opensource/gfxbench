/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
layout(std140) uniform emitterAdvectConsts
{
	//uniform highp		ivec4 particleBufferParamsXYZ_pad;
	
	uniform mediump 	vec4 emitter_apertureXYZ_focusdist;
	uniform highp		mat4 emitter_worldmat;

	uniform mediump		vec4 emitter_min_freqXYZ_speed;
	uniform mediump		vec4 emitter_max_freqXYZ_speed;
	uniform mediump		vec4 emitter_min_ampXYZ_accel;
	uniform mediump		vec4 emitter_max_ampXYZ_accel;

	uniform mediump		vec4 emitter_externalVel_gravityFactor;

	uniform highp		vec4 emitter_maxlifeX_sizeYZ_numSubsteps;
}; 