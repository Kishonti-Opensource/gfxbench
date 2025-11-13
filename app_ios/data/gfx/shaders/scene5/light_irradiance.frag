/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
uniform subpassInput<half> gbuffer_color_texture { color(JOB_ATTACHMENT0) };
uniform subpassInput<half> gbuffer_normal_texture { color(JOB_ATTACHMENT1) };
uniform subpassInput<half> gbuffer_specular_texture { color(JOB_ATTACHMENT2) };

#if SHADER_METAL_IOS && defined(GBUFFER_COLOR_DEPTH_RT_INDEX)
uniform subpassInput<float> gbuffer_depth_texture { color(GBUFFER_COLOR_DEPTH_RT_INDEX) };
#else
uniform subpassInput<float> gbuffer_depth_texture { color(JOB_ATTACHMENT4) };
#endif

#if INDIRECT_SSAO_ENABLED
uniform sampler2D<float> ssao_texture;
#endif

#ifdef USE_TEXTURE_SH_ATLAS
uniform sampler2D<half> m_envprobe_sh_atlas_texture;
#else
buffer float4 m_envprobe_sh_atlas[9];
#endif
uniform samplerCube<half> envmap0;
uniform sampler2D<half> environment_brdf;

uniform float4 view_pos;
uniform float4 depth_parameters;

uniform float4 light_pos;
#ifdef USE_TEXTURE_SH_ATLAS
uniform float envprobe_index;
#endif

//uniform float indirect_lighting_factor;
uniform float ibl_reflection_intensity;

uniform float4 envprobe_inv_half_extent;

in half4 out_view_dir;
in float4 out_pos;
out half3 out_color { color(LIGHTING_RT_INDEX) };
out half out_alpha { color(LIGHTING_WEIGHT_RT_INDEX) };

half3 getCoefficient(uint m)
{
#ifdef USE_TEXTURE_SH_ATLAS
	int2 coord = int2(int(m), int(envprobe_index));
	return texelFetch(m_envprobe_sh_atlas_texture, coord, 0).rgb;
#else
	return half3(m_envprobe_sh_atlas[m].xyz);
#endif
}

#if 0
half3 sphericalHarmonics(half index, half3 normal){
	half x = normal.x;
	half y = normal.y;
	half z = normal.z;

	half3 l00 = getCoefficient(index, 0.0h);

	half3 l10 = getCoefficient(index, 1.0h);
	half3 l11 = getCoefficient(index, 2.0h);
	half3 l12 = getCoefficient(index, 3.0h);

	half3 l20 = getCoefficient(index, 4.0h);
	half3 l21 = getCoefficient(index, 5.0h);
	half3 l22 = getCoefficient(index, 6.0h);
	half3 l23 = getCoefficient(index, 7.0h);
	half3 l24 = getCoefficient(index, 8.0h);

	half3 result = (
		l00 +
		l10 * y +
		l11 * z +
		l12 * x +
		l20 * x*y +
		l21 * y*z +
		l22 * (3.0h*z*z - 1.0h) +
		l23 * x*z +
		l24 * (x*x - y*y)
	);
	return max(result, half3(0.0));
}
#endif

#define REFLECTION_CAPTURE_ROUGHEST_MIP 1.0h
#define REFLECTION_CAPTURE_ROUGHNESS_MIP_SCALE 1.2h

half get_roughness_lod_level(half roughness)
{
	half LevelFrom1x1 = REFLECTION_CAPTURE_ROUGHEST_MIP - REFLECTION_CAPTURE_ROUGHNESS_MIP_SCALE * log2(roughness + 0.0001h);
	const half HardcodedNumCaptureArrayMips = half(MAX_LOD_LEVEL);
	half level = HardcodedNumCaptureArrayMips - 1.0h - LevelFrom1x1;
	return level;
}

void main()
{
	half2 tc = ( half2(out_pos.xy / out_pos.w) * 0.5h) + half2(0.5h);

	float depth = subpassLoad(gbuffer_depth_texture).x;
	/*
	if(depth == 1.0)
	{
		discard;
	}
	*/

	half3 view_dir = out_view_dir.xyz / out_view_dir.w;
	float3 world_pos = get_world_pos(depth, depth_parameters, float3(view_dir), view_pos.xyz);
	view_dir = normalize(view_dir);

	half4 tex_norm_gloss = subpassLoad(gbuffer_normal_texture);
	half3 normal = decode_normal(tex_norm_gloss.xyz);

	// apply the global illumination
	float3 diff = abs(light_pos.xyz - world_pos);

	half falloff_x = 1.0h - half(diff.x * envprobe_inv_half_extent.x);
	half falloff_y = 1.0h - half(diff.y * envprobe_inv_half_extent.y);
	half falloff_z = 1.0h - half(diff.z * envprobe_inv_half_extent.z);

	falloff_x = clamp(falloff_x, 0.0h, 1.0h);
	falloff_y = clamp(falloff_y, 0.0h, 1.0h);
	falloff_z = clamp(falloff_z, 0.0h, 1.0h);

	half intensity = falloff_x * falloff_y * falloff_z;

	if(intensity == 0.0h) discard;

	half3 albedo = subpassLoad(gbuffer_color_texture).xyz;
	albedo = srgb_to_linear_half(albedo);

	half3 R = reflect(half3(view_dir),normal);
	half3 L = getCoefficient(0u);

	half3 harmonics = L;
	half3 harmonics2 = L;

	L = getCoefficient(1u);
	harmonics += L * normal.y;
	harmonics2 += L * R.y;

	L = getCoefficient(2u);
	harmonics += L * normal.z;
	harmonics2 += L * R.z;

	L = getCoefficient(3u);
	harmonics += L * normal.x;
	harmonics2 += L * R.x;

	L = getCoefficient(4u);
	harmonics += L * normal.x * normal.y;
	harmonics2 += L * R.x * R.y;

	L = getCoefficient(5u);
	harmonics += L * normal.y * normal.z;
	harmonics2 += L * R.y * R.z;

	L = getCoefficient(6u);
	harmonics += L * (3.0h * normal.z * normal.z - 1.0h);
	harmonics2 += L * (3.0h * R.z * R.z - 1.0h);

	L = getCoefficient(7u);
	harmonics += L * normal.x * normal.z;
	harmonics2 += L * R.x * R.z;

	L = getCoefficient(8u);
	harmonics += L * (normal.x * normal.x - normal.y * normal.y);
	harmonics2 += L * (R.x * R.x - R.y * R.y);

	half3 irradiance = max(harmonics, half3(0.0));
	half3 irradiance2 = max(harmonics2, half3(0.0));

	half3 specular_color = subpassLoad(gbuffer_specular_texture).xyz;

	half roughness = 1.0h - tex_norm_gloss.w;
	roughness = max(roughness, 0.02h);

	half NdotV = dot(normal, -half3(view_dir));
	NdotV = clamp(NdotV, 0.0h, 1.0h);
	half2 fresnel_scale_bias = texture( environment_brdf, float2(float(roughness), float(NdotV))).xy;
	half mip_level = get_roughness_lod_level(roughness);
	half3 prefiltered_color = textureLod( envmap0, float3(R), float(mip_level)).xyz;
	half3 reflected_color = prefiltered_color * (specular_color * fresnel_scale_bias.x + fresnel_scale_bias.y);

	out_color = (irradiance * albedo + irradiance2 * reflected_color * half(ibl_reflection_intensity)) * intensity;
	out_alpha = intensity;

#if INDIRECT_SSAO_ENABLED
	out_color *= texture(ssao_texture, tc).x;
#endif
}
