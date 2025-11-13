/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#define EPS_MB_HALF 0.001h
#define EPS_MB 0.001
#define SOFT_Z_EXTENT_INV 10.0h

uniform sampler2D<half> input_texture;
uniform sampler2D<float> gbuffer_velocity_texture;
uniform sampler2D<float> gbuffer_depth_texture;
uniform sampler2D<float> neighbor_max_texture;
uniform float4 tap_offsets[SAMPLE_COUNT];
uniform float4 depth_parameters;
uniform float4 velocity_min_max_scale_factor;

half softDepthCompare(half z_a, half z_b)
{
	return clamp(1.0h - (z_a - z_b) * SOFT_Z_EXTENT_INV, 0.0h, 1.0h);
}

half cone(half length_xy, half length_v)
{
	return clamp(1.0h - length_xy/(length_v + 0.001h), 0.0h, 1.0h);
}

half cylinder(half length_xy, half length_v)
{
	length_v = max(length_v,EPS_MB_HALF);
	return 1.0h - smoothstep(0.95h * length_v, 1.05h * length_v, length_xy);
}


// texture_unit0 - color
// texture_unit1 - depth
// texture_unit4 - velocity buffer
// texture_unit6 - neighbor max

in float2 texcoord;
out half4 out_res { color(0) };
void main()
{
	half3 res = half3(0.0h);

	half3 c0 = textureLod( input_texture, texcoord, 0.0).xyz;  // color
	float2 v_n = unpack_from_float4(textureLod( neighbor_max_texture, texcoord, 0.0)); // neighbor max

	float length_vn = length(v_n);
	if ( length_vn < (EPS_MB + 0.5 / float(VP_WIDTH)) )
	{
		res = c0;
	}
	else
	{
		float Z_X = textureLod(gbuffer_depth_texture, texcoord, 0.0).x;
		Z_X = -depth_parameters.y / (Z_X - depth_parameters.x);

		float2 velocity_sample = textureLod( gbuffer_velocity_texture, texcoord, 0.0).xy;
		float length_vx = length(velocity_sample);

		float2 X = texcoord;

		const half p = 0.1h;
		half w = 1.0h / (half(length_vx) + p);
		half3 sum = 1.0h / (half(length_vx) + p) * c0;

		for (int i = 0; i < SAMPLE_COUNT; i++)
		{
			float t = tap_offsets[i].x;

			float2 Y = X + t * v_n;

			float Z_Y = textureLod(gbuffer_depth_texture, Y, 0.0).x;
			Z_Y = -depth_parameters.y / (Z_Y - depth_parameters.x);

			float2 sample_uv = Y;

			float2 V_Y = textureLod( gbuffer_velocity_texture, sample_uv, 0.0).xy;
			half length_vy = half(length(V_Y));
			half length_xy = half(length(X-Y));

			half f = softDepthCompare(half(Z_X), half(Z_Y));
			half b = softDepthCompare(half(Z_Y), half(Z_X));

			half a = f * cone(length_xy, length_vy);
			a += b * cone(length_xy, half(length_vx));
			a += cylinder(length_xy, length_vy) * cylinder(length_xy, half(length_vx)) * 2.0h;
			w += a;

			sum += a * textureLod( input_texture, sample_uv, 0.0).xyz;
		}
		res = sum / w;
	}

	// NOTE: Comment this back to disable motion blur
	if (velocity_min_max_scale_factor.w < 0.5)
	{
		res = c0.xyz;
	}

	out_res = half4(res, 1.0h);
}

