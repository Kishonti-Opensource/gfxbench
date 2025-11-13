/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#if defined(TYPE_fragment) || defined(TYPE_compute)

// The max velocity value will be never bigger than 0.05
#define MAX_VELOCITY 0.05
float4 pack_velocity(float2 velocity)
{
	return float4(velocity, 0.0, 0.0);
	//return float4(velocity / (2.0 * MAX_VELOCITY) + 0.5, 0.0, 0.0);
}

float2 unpack_velocity(float4 velocity_sample)
{
	return velocity_sample.xy;
	//return (velocity_sample.xy - 0.5) * (2.0 * MAX_VELOCITY);
}

float2 velocity_function(float4 pos, float4 prev_pos, float4 velocity_params)
{
	const float blur_strength = 1.0;

	float2 a = pos.xy / pos.w;
	float2 b = prev_pos.xy / prev_pos.w;

	float2 delta = a - b;

	delta *= blur_strength;

	float delta_length = length(delta);

	#define EPS 0.0001
	if (delta_length < EPS)
	{
		delta = float2(0.0, 0.0);
	}
	else
	{
#ifdef NGL_ORIGIN_UPPER_LEFT_AND_NDC_FLIP
		delta.y = -delta.y;
#endif
		delta = ( delta * clamp( delta_length, velocity_params.x, velocity_params.y ) ) / ( delta_length + EPS);
	}
	return delta;
}

#define MAX_FLOAT 4.0
#define MAX_BYTE 255.0
float2 pack_to_float2(float v)
{
	v /= MAX_FLOAT;
	v = (v + 1.0) / 2.0;

	v *= MAX_BYTE;

	float2 res;
	res.x = floor(v) / MAX_BYTE;

	v = fract(v);

	res.y = v;

	return res;
}

float4 pack_to_float4(float2 v)
{
	float2 ex = pack_to_float2(v.x);
	float2 ey = pack_to_float2(v.y);

	return float4(ex, ey);
}

float unpack_from_float(float2 v)
{
	float r255 = v.x * MAX_BYTE + v.y;
	float x = r255 / MAX_BYTE;
	return MAX_FLOAT * (2.0 * x - 1.0);
}

float2 unpack_from_float4(float4 value)
{
	return float2(unpack_from_float(value.xy), unpack_from_float(value.zw));
}

#endif
