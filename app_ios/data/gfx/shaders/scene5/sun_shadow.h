/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifdef TYPE_fragment

#if SHADOW_SAMPLER_ENABLED
#define SAMPLER_2D			sampler2DShadow<float>
#define SAMPLER_2D_ARRAY	sampler2DArrayShadow<float>
#else
#define SAMPLER_2D			sampler2D<float>
#define SAMPLER_2D_ARRAY	sampler2DArray<float>
#endif

#if CSM_ENABLED
// Cascaded shadow map
uniform SAMPLER_2D_ARRAY shadow_map0;
uniform float4x4 shadow_matrices[SHADOW_CASCADE_COUNT];
#else
// Simple sun shadow
uniform SAMPLER_2D shadow_map0;
uniform float4x4 shadow_matrix0;
#endif

// Actor shadows
uniform SAMPLER_2D shadow_map1;
uniform SAMPLER_2D shadow_map2;
uniform float4x4 shadow_matrix1;
uniform float4x4 shadow_matrix2;

// For manual PCF
uniform float4 shadow_map_size0;
uniform float4 shadow_map_size1;
uniform float4 shadow_map_size2;

int get_cascade_index(float3 world_pos)
{
#if CSM_ENABLED
	float4 world_pos4 = float4(world_pos, 1.0);

	for (int i = 0; i < SHADOW_CASCADE_COUNT; i++)
	{
		float4 shadow_coord = shadow_matrices[i] * world_pos4;
		if ( min(shadow_coord.x, shadow_coord.y) >= 0.0 && max(shadow_coord.x, shadow_coord.y) < 1.0)
		{
			return i;
		}
	}
	return SHADOW_CASCADE_COUNT;
#else
	return 0;
#endif
}


float3 get_cascade_color(float3 world_pos)
{
	int index = get_cascade_index(world_pos);
	if (index == 0)
	{
		return float3(1.0, 0.0, 0.0);
	}
	if (index == 1)
	{
		return float3(0.0, 1.0, 0.0);
	}
	if (index == 2)
	{
		return float3(0.0, 0.0, 1.0);
	}
	if (index == 3)
	{
		return float3(1.0, 0.0, 1.0);
	}
	return float3(1.0, 1.0, 1.0);
}


#if !SHADOW_SAMPLER_ENABLED
float manual_pcf(SAMPLER_2D texture, float2 size, float3 shadow_coord)
{
	float2 uv = shadow_coord.xy - (0.5 / size);

	float2 fraction = fract(size * uv.xy);
	uv.xy -= fraction / size;

	float sample00 = textureLodOffset(texture, uv, 0, int2(0, 0)).x;
	float sample10 = textureLodOffset(texture, uv, 0, int2(1, 0)).x;
	float sample01 = textureLodOffset(texture, uv, 0, int2(0, 1)).x;
	float sample11 = textureLodOffset(texture, uv, 0, int2(1, 1)).x;

	sample00 = sample00 < clamp(shadow_coord.z, 0.0, 1.0) ? 0.0 : 1.0;
	sample10 = sample10 < clamp(shadow_coord.z, 0.0, 1.0) ? 0.0 : 1.0;
	sample01 = sample01 < clamp(shadow_coord.z, 0.0, 1.0) ? 0.0 : 1.0;
	sample11 = sample11 < clamp(shadow_coord.z, 0.0, 1.0) ? 0.0 : 1.0;

	float sampleX0 = mix(sample00, sample10, fraction.x);
	float sampleX1 = mix(sample01, sample11, fraction.x);
	float sampleXY = mix(sampleX0, sampleX1, fraction.y);

	return sampleXY;
}

float manual_pcf_gather(SAMPLER_2D texture, float2 size, float3 shadow_coord)
{
	float2 uv = shadow_coord.xy + (0.5 / size);

	float2 fraction = fract(size * uv.xy);
	uv.xy -= fraction / size;

	float4 samples = textureGather(texture, uv);

	float sample01 = samples.x < clamp(shadow_coord.z, 0.0, 1.0) ? 0.0 : 1.0;
	float sample11 = samples.y < clamp(shadow_coord.z, 0.0, 1.0) ? 0.0 : 1.0;
	float sample10 = samples.z < clamp(shadow_coord.z, 0.0, 1.0) ? 0.0 : 1.0;
	float sample00 = samples.w < clamp(shadow_coord.z, 0.0, 1.0) ? 0.0 : 1.0;

	float sampleX0 = mix(sample00, sample10, fraction.x);
	float sampleX1 = mix(sample01, sample11, fraction.x);
	float sampleXY = mix(sampleX0, sampleX1, fraction.y);

	return sampleXY;
}
#endif


float get_world_shadow(float fragment_depth, float4 world_pos4)
{
#if CSM_ENABLED
	/*
	const float bias_values[4] = float[4]
	(
		// TODO: Make these uniform
		0.008,
		0.008,
		0.008,
		0.008
	);
	*/

	int cascade_index = -1;
	float4 shadow_coord;
	for (int i = 0; i < SHADOW_CASCADE_COUNT; i++)
	{
		shadow_coord = shadow_matrices[i] * world_pos4;
		if ( min(shadow_coord.x, shadow_coord.y) >= 0.0 && max(shadow_coord.x, shadow_coord.y) < 1.0)
		{
			cascade_index = i;
			break;
		}
	}
	if (cascade_index == -1)
	{
		// Out of shadow range
		return 1.0;
	}

	// Apply the bias and sample the shadow map
	float z = shadow_coord.z - 0.008;//bias_values[cascade_index];
	shadow_coord.z = float(cascade_index);

#if SHADOW_SAMPLER_ENABLED
		shadow_coord.w = z;
		return texture(shadow_map0, shadow_coord);
#else
		float shadow_depth = texture(shadow_map0, shadow_coord.xyz).x;
		if (shadow_depth < clamp(z, 0.0, 1.0))
		{
			return 0.0;
		}
		return 1.0;
#endif

	// TODO: Shadow attenuation
	//float border = shadow_frustum_distances.w - 0.00001;
	//float attenuation = clamp((fragment_depth - border) / 0.00001, 0.0, 1.0);
	//shadow_dist = mix(shadow_dist, 1.0, attenuation);

#else // Simple shadow map

	float4 shadow_coord = shadow_matrix0 * world_pos4;

#if SHADOW_SAMPLER_ENABLED
		//shadow_coord.z = z;
		return texture(shadow_map0, shadow_coord.xyz);
#else
		return manual_pcf_gather(shadow_map0, shadow_map_size0.xy, shadow_coord.xyz);

		/*
		float shadow = texture(shadow_map0, shadow_coord.xy).x;
		if( shadow < clamp(shadow_coord.z, 0.0, 1.0))
			return 0.0;
		else
			return 1.0;
		*/
#endif

#endif
}


float get_sun_shadow(float fragment_depth, float3 world_pos)
{
	float4 world_pos4 = float4(world_pos, 1.0);

	float world_shadow = get_world_shadow(fragment_depth, world_pos4);

#if SHADOW_SAMPLER_ENABLED
	float tanya_shadow = texture(shadow_map1, (shadow_matrix1 * world_pos4).xyz);
#else
	float4 tanya_shadow_coord = shadow_matrix1 * world_pos4;

	float tanya_shadow = manual_pcf_gather(shadow_map1, shadow_map_size1.xy, tanya_shadow_coord.xyz);

	/*
	float tanya_shadow_sample = texture(shadow_map1, tanya_shadow_coord.xy).x;
	float tanya_shadow = 1.0;
	if( tanya_shadow_sample < clamp(tanya_shadow_coord.z, 0.0, 1.0))
	{
		tanya_shadow = 0.0;
	}
	*/
#endif

#if SHADOW_SAMPLER_ENABLED
	float golem_shadow = texture(shadow_map2, (shadow_matrix2 * world_pos4).xyz);
#else
	float4 golem_shadow_coord = shadow_matrix2 * world_pos4;

	float golem_shadow = manual_pcf(shadow_map2, shadow_map_size2.xy, golem_shadow_coord.xyz);

	/*
	float golem_shadow_sample = texture(shadow_map2, golem_shadow_coord.xy).x;
	float golem_shadow = 1.0;
	if( golem_shadow_sample < clamp(golem_shadow_coord.z, 0.0, 1.0))
	{
		golem_shadow = 0.0;
	}
	*/
#endif

	return world_shadow * tanya_shadow * golem_shadow;
}

#endif
