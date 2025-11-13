/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
// Depth texture
uniform sampler2D<float> gbuffer_depth_texture;

// Camera
uniform float4 view_pos;
uniform float4 view_dir;

// Light
uniform float4 light_color;
uniform float4 light_frustum_planes[6];
uniform float4 lightshaft_parameters; // y - intensity, z - sample count, w - 1/sample count

uniform sampler2D<float> shadow_map;
uniform float4x4 shadow_matrix;

#define INTENSITY			lightshaft_parameters.y
//#define SAMPLE_COUNT		lightshaft_parameters.z
#define SAMPLE_COUNT 32
#define SAMPLE_COUNT_RCP 	lightshaft_parameters.w
#define MAX_DIST			2048.0

float4 intersect_ray_plane( float3 ro, float3 rd, float4 plane)
{
	//plane:
	//xyz: plane normal
	//w: plane dist from origo

	float denom = dot( rd, plane.xyz );

	if( abs(denom) < 0.0001 || denom > 0.0 )
		return float4(MAX_DIST);

	float t = -(plane.w + dot( ro, plane.xyz )) / denom;

	if( t < 0.0 )
		return float4(MAX_DIST);

	return float4(ro + rd * t, t);
}

float bayer(float2 coords)
{
	uint x = uint(coords.x) % 4u;
	uint y = uint(coords.y) % 4u;
	
	float bayer_array[] = { BAYER_ARRAY };

	return bayer_array[4u * x + y];
}

in float4 world_position;
in float4 projected_pos;
in float4 out_view_dir;
out float4 out_color { color(0) };
void main()
{
	//texcoord
	float2 tc = (projected_pos.xy / projected_pos.w * 0.5) + float2(0.5, 0.5);

	float4 ray_dir = out_view_dir;
	ray_dir.xyz /= ray_dir.w;

	// Linear depth
	float gbuffer_depth = textureLod(gbuffer_depth_texture, tc, 0.0).x;
	//gbuffer_depth = depth_parameters.y / (gbuffer_depth - depth_parameters.x);

	// Begin point depth test
	if( gbuffer_depth < out_view_dir.w )
	{
		discard;
	}

	// Ray-plane intersection to optimize ray marching
	float min_dist = MAX_DIST;
	float3 int_point = float3(0.0);
	float3 rd = normalize(ray_dir.xyz);
	for(uint i = 0u; i < 6u; i++)
	{
		float4 tmp_int_point = intersect_ray_plane( view_pos.xyz, rd, light_frustum_planes[i]);
		if (tmp_int_point.w < min_dist)
		{
			min_dist = tmp_int_point.w;
			int_point = tmp_int_point.xyz;
		}
	}

	// Let's be paranoid with floating point errors...
	if (min_dist >= MAX_DIST / 2.0)
	{
		discard;
	}

	float len = dot(int_point - view_pos.xyz, view_dir.xyz);
	min_dist = min(len, gbuffer_depth); // End point depth test
	min_dist = min_dist - out_view_dir.w;

	float step_size = min_dist * SAMPLE_COUNT_RCP;
	float3 ray_march_step = ray_dir.xyz * step_size;

	// Offset the stating point with dithering
	float3 sampling_pos = world_position.xyz + ray_march_step * bayer(gl_FragCoord.xy);

	float3 fog_amount = float3(0.0);
	float alpha = 0.0;
	uint iteration_uint = uint(SAMPLE_COUNT);
	for(uint iteration = 0u; iteration < iteration_uint; iteration++)
	{
		// Discrete sampling positions
		//sampling_pos = float3(int3(sampling_pos * 100.0)) / 100.0;

		// Sample the shadow map
		float4 shadow_clip_pos = shadow_matrix * float4(sampling_pos, 1.0);
		float shadow_map_depth = texture(shadow_map, shadow_clip_pos.xy).x;

		if (shadow_map_depth > shadow_clip_pos.z)
		{
			fog_amount += min_dist;

			// (1 - dst) * src + dst
			alpha = (1.0 - alpha) * 1.0 + alpha;
		}

		/*
		float4 src = float4(light_color.xyz * shadow, shadow);
		src.a *= lightshaft_parameters.x; // Blend density

		src.rgb *= src.a;
		final_color = (1.0 - final_color.a) * src + final_color;
		*/

		// March
		sampling_pos.xyz += ray_march_step;
	}

	alpha = alpha * SAMPLE_COUNT_RCP;

	out_color.xyz = (light_color.xyz * INTENSITY * fog_amount * SAMPLE_COUNT_RCP) * alpha;

	out_color.w = clamp(alpha, 0.0, 1.0);
}
