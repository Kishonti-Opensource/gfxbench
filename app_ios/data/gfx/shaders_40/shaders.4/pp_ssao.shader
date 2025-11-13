/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
//https://gist.github.com/fisch0920/6770311
//https://research.nvidia.com/publication/scalable-ambient-obscurance

#include "common.h"

#ifdef TYPE_fragment

in vec2 out_texcoord0;
out float frag_color;

// Total number of samples at each fragment
#define NUM_SAMPLES           7
#define NUM_SPIRAL_TURNS      3

// Beta4 sample counts
//#define NUM_SAMPLES           10
//#define NUM_SPIRAL_TURNS      6

//#define NUM_SAMPLES           32
//#define NUM_SPIRAL_TURNS      3

//#define NUM_SAMPLES           64
//#define NUM_SPIRAL_TURNS      17

#define MAX_DISTANCE				(500.0)
#define MAX_DISTANCE_HALF	 		(MAX_DISTANCE * 0.5)
#define MAX_DISTANCE_HALF_INV		(1.0 / MAX_DISTANCE_HALF)

#define LOG_MAX_OFFSET (2)

#define RADIUS 1.0

#define RADIUS2 (RADIUS * RADIUS)

uniform float projection_scale;

highp float unpackFloatFromVec4i(const highp vec4 value)
{
	// http://aras-p.info/blog/2009/07/30/encoding-floats-to-rgba-the-final/
	return dot( value, vec4(1.0, 1.0/255.0, 1.0/65025.0, 1.0/160581375.0) );
}

vec3 getOffsetPosition(vec2 uv, ivec2 ssC, vec2 unitOffset, float ssR)
{
	vec2 sample_uv = (ssR / projection_scale * unitOffset) + uv;

#ifdef USE_HIZ_DEPTH
	int mipLevel = clamp(findMSB(int(ssR)) - LOG_MAX_OFFSET, 0, MAX_MIP_LEVEL);

	highp float linear_z = unpackFloatFromVec4i(textureLod(texture_unit0, sample_uv, float(mipLevel)));

	highp float linear_depth = mix(depth_parameters.z, depth_parameters.w, linear_z);
	return getPositionVS(linear_depth, sample_uv);
#else
	return getPositionVS(depth_unit0, sample_uv);
#endif
}


/** Returns a unit vector and a screen-space radius for the tap on a unit disk (the caller should scale by the actual disk radius) */
vec3 tapLocation(int sampleNumber, float spinAngle)
{
	// Radius relative to ssR
	float alpha = (float(sampleNumber) + 0.5) * (1.0 / float(NUM_SAMPLES));
	float angle = alpha * (float(NUM_SPIRAL_TURNS) * 2.0 * PI) + spinAngle;
	return vec3(cos(angle), sin(angle), alpha);
}

float sampleAO(vec2 uv, ivec2 sspace_coords, vec3 vs_pos, vec3 vs_normal, in float sspace_radius, int tapIndex, in float random_angle)
{
	// Offset on the unit disk, spun for this pixel
	vec3 unitOffset = tapLocation(tapIndex, random_angle);

	float alpha = unitOffset.z * sspace_radius;

	// The occluding point in camera space
	vec3 Q = getOffsetPosition(uv, sspace_coords, unitOffset.xy, alpha);

	vec3 v = Q - vs_pos;

	float vv = dot(v, v);
	float vn = dot(v, vs_normal) - 0.1;

	float f = max(RADIUS2 * 4.0 - vv, 0.0) / RADIUS2 / 4.0;
	f = pow(f, 0.01);

#define EPSILON 0.01
	return f * max(vn / (EPSILON + vv), 0.0);
}


void main()
{
#ifdef USE_HIZ_DEPTH
	highp float linear_z = unpackFloatFromVec4i(textureLod(texture_unit0, out_texcoord0, 0.0));
	highp float linear_depth  = mix(depth_parameters.z, depth_parameters.w, linear_z);
#else
	highp float linear_depth = getLinearDepth(depth_unit0, out_texcoord0);
#endif

	if (linear_depth >= MAX_DISTANCE)
	{
		frag_color = 1.0;
		return;
	}

	highp vec3 vs_pos = getPositionVS(linear_depth, out_texcoord0);
	vec3 vs_normal = decodeNormal(texture_unit1, out_texcoord0);

	// Flip coords for correct algo
	vs_normal = vec3(-vs_normal.y, vs_normal.x, vs_normal.z);

	ivec2 sspace_coords = ivec2(gl_FragCoord.xy);

	float random_angle = 2.0 * PI * rand( out_texcoord0);

	float sspace_radius = projection_scale * RADIUS /  linear_depth;

	float occlusion = 0.0;
	for (int i = 0; i < NUM_SAMPLES; i++)
	{
		occlusion += sampleAO(out_texcoord0, sspace_coords, vs_pos, vs_normal, sspace_radius, i, random_angle);
	}

	occlusion = 1.0 - occlusion / float(  NUM_SAMPLES);
	occlusion = pow(occlusion, 4.0);

	// Remove artifacts
	float distanceFade = clamp((linear_depth - MAX_DISTANCE_HALF) * MAX_DISTANCE_HALF_INV, 0.0, 1.0);
	frag_color = mix(occlusion, 1.0, sqrt(distanceFade));
}

#endif

#ifdef TYPE_vertex

in vec3 in_position;
out vec2 out_texcoord0;

void main()
{
	gl_Position = vec4( in_position, 1.0);
	out_texcoord0 = in_position.xy * 0.5 + 0.5;
}


#endif
