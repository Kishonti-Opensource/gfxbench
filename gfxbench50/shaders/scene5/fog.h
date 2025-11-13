/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifdef TYPE_fragment

uniform float4 fog_color;
uniform float4 fog_parameters1; // amount, density, start distance
uniform float4 fog_parameters2; // amount, density, max_height

float get_fog_amount(float3 world_pos, float3 eye_pos, float ray_length)
{
	float distance_fog = 1.0 - exp(-max(ray_length - fog_parameters1.z, 0.0) * fog_parameters1.x);
	distance_fog = distance_fog * fog_parameters1.y;

	float height = max( -world_pos.y + fog_parameters2.z, 0.0);

	float vertical_fog = 1.0 - exp(-height * fog_parameters2.x);
	vertical_fog = vertical_fog * fog_parameters2.y;

	return distance_fog + vertical_fog;
}

/*
float get_vertical_amount(float3 world_pos, float3 eye_pos, float3 ray_dir)
{
	// Plane - ray intersection
	float3 p0 = eye_pos;
	float4 N = float4(0.0, 1.0, 0.0, 2.0);

	float denom = dot(ray_dir, N.xyz);
	if (denom == 0.0)
	{
		discard;
	}
	float t = -(dot(p0, N.xyz) + N.w) / denom;

	// Fog above the camera
	if (t < 0.0)
	{
		discard;
	}

	// Calculate the intersection
	float3 p = p0 + t * ray_dir;

	// Omit invalid intersection (The intersection is behind the world-space position)
	float l0 = length(p - eye_pos);
	float l1 = length(world_pos - eye_pos);
	if (l0 > l1)
	{
		discard;
	}

	// Apply wave as noise
	float c = sin(p.z * 0.5) + sin(p.x * 0.5);
	c += (sin(p.z * 2.9) + sin(p.x * 2.9)) * 0.2;
	c = -abs(c);
	c = c * 0.15;
	c = clamp(c, -0.1, 0.0);

	// Calculate the fog amount
	float l = length(p - world_pos);
	float fog = c + 0.03 * pow(l, 0.8);

	return clamp(fog, 0.0, 1.0);
}
*/

#endif
