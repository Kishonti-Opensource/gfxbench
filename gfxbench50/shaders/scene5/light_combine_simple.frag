/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
uniform sampler2D<float> gbuffer_color_texture;
uniform sampler2D<float> gbuffer_normal_texture;
uniform sampler2D<float> gbuffer_specular_texture;
uniform sampler2D<float> gbuffer_emissive_texture;
uniform sampler2D<float> gbuffer_depth_texture;

uniform float4 view_pos;
uniform float4 depth_parameters;

// Shadow
uniform float4 light_dir;
uniform float4 shadow_frustum_distances;
uniform sampler2DArray<float> shadow_map;
uniform float4x4 shadow_matrices[4];

uniform sampler2D<float> texture_unit7; // SSAO

#define M_PI 3.14159265358

float linear_to_srgb(float x)
{
	if (x < 0.0031308)
	{
		return 12.92 * x;
	}
	else
	{
		return 1.055 * pow(x, 0.41666) - 0.055;
	}
}

float3 get_sh_irradiance(float3 normal)
{
	float3 coeffL0;
	float3 coeffL1[3];
	float3 coeffL2[5];

	// sky cloudy free hdr
	coeffL0 = float3(0.47617,0.481866,0.502918);
	coeffL1[0] = float3(0.367666, 0.38842, 0.490323);
	coeffL1[1] = float3(0.079516, 0.0699033, 0.0649335);
	coeffL1[2] = float3(0.234263, 0.213554, 0.210651);

	coeffL2[0] = float3(0.203956,0.193104, 0.189979);
	coeffL2[1] = float3(0.0721589, 0.0646777, 0.0605624);
	coeffL2[2] = float3(-0.0235643, -0.0221851,-0.0234574);
	coeffL2[3] = float3(0.0662994,0.0593324, 0.053132);
	coeffL2[4] = float3(0.0591019, 0.0454807 ,0.0288728);

	//l = 0 band
	float3 light = coeffL0;

	//l = 1 band
	light += coeffL1[0].xyz * normal.y;
	light += coeffL1[1].xyz * normal.z;
	light += coeffL1[2].xyz * normal.x;

	//l = 2 band
	float3 swz = normal.yyz * normal.xzx;
	light += coeffL2[0].xyz * swz.x;
	light += coeffL2[1].xyz * swz.y;
	light += coeffL2[2].xyz * swz.z;

	float3 sqr = normal * normal;
	light += coeffL2[3].xyz * ( 3.0*sqr.z - 1.0 );
	light += coeffL2[4].xyz * ( sqr.x - sqr.y );

	return light;
}

in float2 out_texcoord;
in float4 out_view_dir;
in float4 out_pos;
out float4 out_color { color(0) };
void main()
{
	float depth = texture( gbuffer_depth_texture, out_texcoord).x;
	if(depth == 1.0)
	{
		discard;
	}

	// Material parameters from G-buffers
	float3 albedo = texture(gbuffer_color_texture, out_texcoord).xyz;
	float3 specular_color = texture(gbuffer_specular_texture, out_texcoord).xyz;
	float4 tex_norm_gloss = texture(gbuffer_normal_texture, out_texcoord);

	albedo = srgb_to_linear(albedo);
	specular_color = srgb_to_linear(specular_color);

	// Energy conservation
	//float reflectivity = get_reflectivity(specular_color);
	albedo = albedo * (float3(1.0) - specular_color);

	float roughness = 1.0 - tex_norm_gloss.w;
	roughness = max(roughness, 0.02);

	float3 normal = 2.0 * tex_norm_gloss.xyz - 1.0;
	normal = normalize(normal);

	// Shading
	float3 view_dir = out_view_dir.xyz / out_view_dir.w;
	float3 world_pos = get_world_pos(depth, depth_parameters, view_dir, view_pos.xyz);
	view_dir = normalize(view_dir);

	float3 diffuse_light = get_sh_irradiance( normal.xyz);
	float3 diffuse_color = diffuse_light * albedo;

	float NdotV = dot(normal, -view_dir);
	NdotV =  clamp(NdotV, 0.0, 1.0);

	float3 R = reflect( view_dir, normal);
	float3 reflected_color = float3(0.0);

	//////////////////////////////////////////////////////////////

	float shadow = 1.0;
	const float shadow_strength = 1.2;
#if SHADOW_ENABLED
	// Map based selection
	/*
	const float bias_values[4] = float[4]
	(
		0.000,
		0.000,
		0.0001,
		0.0001
	);
	*/

	int cascade_index = 4;
	float4 world_pos_h = float4(world_pos, 1.0);
	float4 shadow_coord = shadow_matrices[0] * world_pos_h;
	bool shadow_test = min(shadow_coord.x, shadow_coord.y) >= 0.0 && max(shadow_coord.x, shadow_coord.y) < 1.0;
	if ( shadow_test)
	{
		cascade_index = 0;
	}
	else
	{
		shadow_coord = shadow_matrices[1] * world_pos_h;
		shadow_test = min(shadow_coord.x, shadow_coord.y) >= 0.0 && max(shadow_coord.x, shadow_coord.y) < 1.0;
		if ( shadow_test)
		{
			cascade_index =  1;
		}
		else
		{
			shadow_coord = shadow_matrices[2] * world_pos_h;
			shadow_test = min(shadow_coord.x, shadow_coord.y) >= 0.0 && max(shadow_coord.x, shadow_coord.y) < 1.0;
			if ( shadow_test)
			{
				cascade_index =  2;
			}
			else
			{
				shadow_coord = shadow_matrices[3] * world_pos_h;
				shadow_test = min(shadow_coord.x, shadow_coord.y) >= 0.0 && max(shadow_coord.x, shadow_coord.y) < 1.0;
				if ( shadow_test)
				{
					cascade_index =  3;
				}
			}
		}
	}

	// Apply the bias and sample the shadow map
	highp float z = shadow_coord.z - 0.001; /* - bias_values[cascade_index]*/;
	shadow_coord.z = float(cascade_index);
	shadow_coord.w = z;

	//PCF
	float shadow_depth = texture(shadow_map, shadow_coord.xyz).x;

	float compare_depth = z;
	bool in_shadow = shadow_depth < clamp(compare_depth, 0.0, 1.0);
	if (in_shadow)
	{
		shadow = shadow_strength;
	}
#else
	shadow = shadow_strength;
#endif

	// Shadow attenuation
	//float border = shadow_frustum_distances.w - 0.00001;
	//float attenuation = clamp((fragment_depth - border) / 0.00001, 0.0, 1.0);
	//shadow_dist = mix(shadow_dist, 1.0, attenuation);


#if SSAO_ENABLED
	float ao = texture(texture_unit7, out_texcoord).x;
#else
	float ao = 1.0;
#endif

	ao = 1.0;

	float3 color = (diffuse_color * shadow  + reflected_color) * ao;
	
	out_color = float4(color, 1.0);
}
