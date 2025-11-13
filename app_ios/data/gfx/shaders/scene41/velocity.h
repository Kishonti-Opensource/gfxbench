/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifdef KSL_COMPILER

#define vec4 float4
#define vec3 float3
#define vec2 float2
#define sampler2D hsampler2D
#define ivec2 int2

#endif

uniform vec4 velocity_min_max_scale_factor;

// The max velocity value will be never bigger than 0.05
#define MAX_VELOCITY 0.05
vec4 pack_velocity(vec2 velocity)
{
	return vec4(velocity / (2.0 * MAX_VELOCITY) + 0.5, 0.0, 0.0);
}

vec2 unpack_velocity(vec4 velocity_sample)
{
	return (velocity_sample.xy - 0.5) * (2.0 * MAX_VELOCITY);
}

vec2 velocity_function(vec4 pos, vec4 prev_pos, vec4 velocity_min_max_scale_factor)
{
	const float blur_strength = 0.5;

	vec2 a = pos.xy / pos.w;
	vec2 b = prev_pos.xy / prev_pos.w;

	vec2 delta = a - b;

	delta *= blur_strength;

	float delta_length = length(delta);

	#define EPS 0.0001
	if (delta_length < EPS)
	{
		delta = vec2(0.0, 0.0);
	}
	else
	{
#ifdef NGL_DX_NDC
		delta.y = -delta.y;
#endif
		delta = ( delta * clamp( delta_length, velocity_min_max_scale_factor.x, velocity_min_max_scale_factor.y ) ) / ( delta_length + EPS);
	}
	return delta;
}

#define MAX_FLOAT 4.0
#define MAX_BYTE 255.0
vec2 pack_to_vec2(float v)
{
	v /= MAX_FLOAT;
	v = (v + 1.0) / 2.0;

	v *= MAX_BYTE;

	vec2 res;
	res.x = floor(v) / MAX_BYTE;

	v = fract(v);

	res.y = v;

	return res;
}

vec4 pack_to_vec4(vec2 v)
{
	vec2 ex = pack_to_vec2(v.x);
	vec2 ey = pack_to_vec2(v.y);

	return vec4(ex, ey);
}

float unpack_float(vec2 v)
{
	float r255 = v.x * MAX_BYTE + v.y;
	float x = r255 / MAX_BYTE;
	return MAX_FLOAT * (2.0 * x - 1.0);
}

vec2 unpack_vec2(vec4 value)
{
	return vec2(unpack_float(value.xy), unpack_float(value.zw));
}
