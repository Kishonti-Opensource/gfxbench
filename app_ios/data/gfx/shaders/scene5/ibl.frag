/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
uniform subpassInput<float> gbuffer_color_texture { color(0) };
uniform subpassInput<float> gbuffer_normal_texture { color(1) };
uniform subpassInput<float> gbuffer_specular_texture { color(2) };

#if SHADER_METAL_IOS && defined(GBUFFER_COLOR_DEPTH_RT_INDEX)
uniform subpassInput<float> gbuffer_depth_texture { color(GBUFFER_COLOR_DEPTH_RT_INDEX) };
#else
uniform subpassInput<float> gbuffer_depth_texture { color(3) };
#endif

uniform sampler2D<float> environment_brdf;
uniform samplerCube<float> envmap0;

uniform float4 view_pos;
uniform float4 depth_parameters;

uniform float ibl_diffuse_intensity;
uniform float ibl_reflection_intensity;

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


#define REFLECTION_CAPTURE_ROUGHEST_MIP 1.0
#define REFLECTION_CAPTURE_ROUGHNESS_MIP_SCALE 1.2

float get_roughness_lod_level(float roughness)
{
	float LevelFrom1x1 = REFLECTION_CAPTURE_ROUGHEST_MIP - REFLECTION_CAPTURE_ROUGHNESS_MIP_SCALE * log2(roughness + 0.0001);
	const float HardcodedNumCaptureArrayMips = float(MAX_LOD_LEVEL);
	float level = HardcodedNumCaptureArrayMips - 1.0 - LevelFrom1x1;
	return level;
}


in float2 out_texcoord;
in float4 out_view_dir;
out float4 out_res { color(LIGHTING_RT_INDEX) };
void main()
{
	float depth = subpassLoad(gbuffer_depth_texture).x;
	/*
	if(depth == 1.0)
	{
		discard;
	}
	*/

	// Material parameters from G-buffers
	float3 albedo = subpassLoad(gbuffer_color_texture).xyz;
	float3 specular_color = subpassLoad(gbuffer_specular_texture).xyz;
	float4 tex_norm_gloss = subpassLoad(gbuffer_normal_texture);

	albedo = srgb_to_linear(albedo);
	specular_color = srgb_to_linear(specular_color);

	// Energy conservation
	albedo = get_energy_conservative_albedo(albedo, specular_color);

	float roughness = 1.0 - tex_norm_gloss.w;
	roughness = max(roughness, 0.02);

	float3 normal = decode_normal(tex_norm_gloss.xyz);
	normal = normalize(normal);

	// Shading
	float3 view_dir = out_view_dir.xyz / out_view_dir.w;
	float3 world_pos = get_world_pos(depth, depth_parameters, view_dir, view_pos.xyz);
	view_dir = normalize(view_dir);

	// Diffuse
	float3 diffuse_light = get_sh_irradiance( normal.xyz);
	float3 diffuse_color = diffuse_light * albedo;

	// Specular
	float NdotV = dot(normal, -view_dir);
	NdotV =  clamp(NdotV, 0.0, 1.0);
	float2 fresnel_scale_bias = texture( environment_brdf, float2(roughness, NdotV)).xy;

	float mip_level = get_roughness_lod_level(roughness);
	float3 R = reflect( view_dir, normal);
	float3 prefiltered_color = textureLod( envmap0, R, mip_level).xyz;
	float3 reflected_color = prefiltered_color * (specular_color * fresnel_scale_bias.x + fresnel_scale_bias.y);

	float3 color = (ibl_diffuse_intensity * diffuse_color + ibl_reflection_intensity * reflected_color);

	out_res = float4(color, 1.0);
}
