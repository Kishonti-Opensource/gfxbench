/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#define EPS 0.0001
#define SOFT_Z_EXTENT_INV 10.0

uniform sampler2D input_texture;
uniform sampler2D gbuffer_velocity_texture;
uniform sampler2D gbuffer_depth_texture;
uniform sampler2D neighbor_max_texture;
//uniform float tap_offsets[SAMPLE_COUNT];
uniform vec4 tap_offsets[SAMPLE_COUNT];
uniform vec4 depth_parameters;

float softDepthCompare(float z_a, float z_b)
{
  return clamp(1.0 - (z_a - z_b) * SOFT_Z_EXTENT_INV, 0.0, 1.0);
}

float cone(float length_xy, float length_v)
{
	return clamp(1.0 - length_xy/(length_v + 0.001), 0.0, 1.0);
}

float cylinder(float length_xy, float length_v)
{
	return 1.0 - smoothstep(0.95 * length_v, 1.05 * length_v, length_xy);
}

// Vectorized version
vec4 softDepthCompare4(vec4 z_a, vec4 z_b)
{
  return clamp(vec4(1.0, 1.0, 1.0, 1.0) - (z_a - z_b) * SOFT_Z_EXTENT_INV, vec4(0.0, 0.0, 0.0, 0.0), vec4(1.0, 1.0, 1.0, 1.0));
}

vec4 cone4(vec4 length_xy, vec4 length_v)
{
	return clamp(vec4(1.0, 1.0, 1.0, 1.0) - length_xy/(length_v + 0.001), vec4(0.0, 0.0, 0.0, 0.0), vec4(1.0, 1.0, 1.0, 1.0));
}

vec4 cylinder4(vec4 length_xy, vec4 length_v)
{
	return vec4(1.0, 1.0, 1.0, 1.0) - smoothstep(0.95 * length_v, 1.05 * length_v, length_xy);
}

#if 0
float get_linear_depth(vec2 uv)
{
	float depth = textureLod(gbuffer_depth_texture, uv, 0.0).x;
	return depth_parameters.y / (depth - depth_parameters.x);
}
#endif

// texture_unit0 - color
// texture_unit1 - depth
// texture_unit4 - velocity buffer
// texture_unit6 - neighbor max

#ifdef KSL_COMPILER
out float4 out_res { color(0) };
#endif

in vec2 texcoord;
void main()
{
	int debug_sample_count = 0;

	vec3 res = vec3(0.0, 0.0, 0.0);

	vec3 c0 = textureLod( input_texture, texcoord, 0.0).xyz;  // color
	vec2 v_n = unpack_vec2(textureLod( neighbor_max_texture, texcoord, 0.0)); // neighbor max


	float length_vn = length(v_n);
	if ( length_vn < (EPS + 0.5 / float(VP_WIDTH)) )
	{
		res = c0;
	}
	else
	{
		//float Z_X = -get_linear_depth(texcoord);
		
		float Z_X = textureLod(gbuffer_depth_texture, texcoord, 0.0).x;
		Z_X = -depth_parameters.y / (Z_X - depth_parameters.x);
		
		vec4 velocity_sample = textureLod( gbuffer_velocity_texture, texcoord, 0.0);
		float length_vx = length(unpack_velocity( velocity_sample));
		vec2 X = texcoord;

		const float p = 0.1;
		float w = 1.0 / (length_vx + p);
		vec3 sum = 1.0 / (length_vx + p) * c0;

		int sample_count = int(length_vn * velocity_min_max_scale_factor.z);
		sample_count = clamp(sample_count, 1, SAMPLE_COUNT);

		sample_count = SAMPLE_COUNT;

		debug_sample_count = sample_count;
		for (int i = 0; i < sample_count; i++)
		{
			float t = tap_offsets[i].x;

			vec2 Y = X + t * v_n;

			//float Z_Y = -get_linear_depth(Y);
			
			float Z_Y = textureLod(gbuffer_depth_texture, Y, 0.0).x;
			Z_Y = -depth_parameters.y / (Z_Y - depth_parameters.x);

			vec2 sample_uv = Y;

			vec4 velocity_sampleY = textureLod( gbuffer_velocity_texture, sample_uv, 0.0);
			vec2 V_Y = unpack_velocity( velocity_sampleY);
			float length_vy = length(V_Y);
			float length_xy = length(X-Y);

			float f = softDepthCompare(Z_X, Z_Y);
			float b = softDepthCompare(Z_Y, Z_X);

			float a = f * cone(length_xy, length_vy);
			a += b * cone(length_xy, length_vx);
			a += cylinder(length_xy, length_vy) * cylinder(length_xy, length_vx) * 2.0;
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

	//res.x = 1.0;

	//res = textureLod( gbuffer_velocity_texture, texcoord, 0.0).xyz;
	//res = textureLod( neighbor_max_texture, texcoord, 0.0).xyz;

	//res.x = 0.0; //get_linear_depth(texcoord) / depth_parameters.w * 10.0;
	//res.y = tap_offsets[22] * 10.0;
	//res.z = 0.0;

#ifdef KSL_COMPILER
	out_res = float4(res, 1.0);
#else
	gl_FragData[0] = vec4(res, 1.0);
#endif
	
}

