/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */

float3 normalize2(float3 v)
{
#ifdef USE_NORMALIZATION_CUBE_MAP
	/*vec3 r = textureCube( normalization_cubemap, v).xyz;
	r = r * 2.0 - 1.0;
	return r;*/
	return normalize( v );
#else
	return normalize( v );
#endif
}

float3 screen_position_normalize(in float4 screen_position)
{
	screen_position /= screen_position.w;
	float x = ( screen_position.x + 1 ) * 0.5f;
	float y = ( screen_position.y - 1 ) *  - 0.5f;
	float z = ( screen_position.z );

	return float3(x, y, z);
}

// Returns a pseudo-random number
float rand1(in float2 co)
{
	return frac(sin(dot(co.xy, float2(12.9898f, 78.233f))) * 43758.5453f);
}

// Returns a pseudo-random number
float rand2(in float2 co, in float t)
{
	return frac(sin(dot(co.xy, float2(12.9898f, 78.233f) + t)) * 43758.5453f);
}

float3 randVec3(in float seed)
{
	return frac(sin((seed + 635.0) * float3(12.9898f, 78.233f, 53.363f)) * float3(43758.5453f, 87362.2314f, 12234.6432f));
}

// Encodes a float value in [0..1] range in a float2 vector to be stored in two 8 bit channels.
float2 floatToVector2(in float x)
{
	const float max16int = 256.0 * 256.0 - 1.0;

	float2 result = floor(x * max16int / float2(256.0, 1.0));
	result.g -= result.r * 256.0;
	result /= 255.0;
	
	return result;
}

// Decodes a float value previously encoded on two 8-bit channes.
float vector2ToFloat(in float2 x)
{
	const float max16int = 256.0 * 256.0 - 1.0;
	return 255.0 * dot(x, float2(256.0f, 1.0f)) / max16int;
	//return dot(x, float2(1.0f, 1.0f / 256));
}