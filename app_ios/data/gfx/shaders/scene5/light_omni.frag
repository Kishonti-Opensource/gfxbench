/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
uniform subpassInput<float> gbuffer_color_texture { color(JOB_ATTACHMENT0) };
uniform subpassInput<float> gbuffer_normal_texture { color(JOB_ATTACHMENT1) };
uniform subpassInput<float> gbuffer_specular_texture { color(JOB_ATTACHMENT2) };

#if SHADER_METAL_IOS && defined(GBUFFER_COLOR_DEPTH_RT_INDEX)
uniform subpassInput<float> gbuffer_depth_texture { color(GBUFFER_COLOR_DEPTH_RT_INDEX) };
#else
uniform subpassInput<float> gbuffer_depth_texture { color(JOB_ATTACHMENT4) };
#endif

uniform float4 view_pos;
uniform float4 depth_parameters;

uniform float4 light_pos;
uniform float4 light_color;
uniform float4 attenuation_parameters;


#ifdef HAS_SHADOW

uniform float4 shadow_light_pos;

#define CUBE_SHADOW 1
#if CUBE_SHADOW // CUBE_SHADOW
uniform float4x4 shadow_matrix;
uniform samplerCubeShadow<float> shadow_map;

half get_cube_shadow(float3 Ldir)
{
	float3 abs_Ldir = abs(Ldir);
	float z = max(abs_Ldir.x,max(abs_Ldir.y,abs_Ldir.z));

#if 0
	float4 t = shadow_matrix * float4(0.0,0.0,z,1.0);
	float d = t.z / t.w;
#else
	float d = (z*shadow_matrix[2][2] + shadow_matrix[3][2])/(z*shadow_matrix[2][3] + shadow_matrix[3][3]);
#endif

	// comment out to clip shadow at light radius
	// if (d > 1.0) return 1.0;

	return half(texture( shadow_map, float4(-Ldir, d)));
}

#else // !CUBE_SHADOW (paraboloid)

uniform float4x4 shadow_matrix;
uniform sampler2DArrayShadow<float> shadow_map;

half get_paraboloid_shadow(float3 R, float depth)
{
	//transform to DP-map space (z = fwd, y = up)

	float3 Rdp = normalize(shadow_matrix * float4(R, 0.0)).xyz;

	if(Rdp.z > 0.0)
	{
		float2 frontUV = (Rdp.xy / (2.0 * (1.0 + Rdp.z))) + 0.5;
		return texture( shadow_map, float4(frontUV, 0.0, depth));
	}
	else
	{
		float2 backUV = (Rdp.xy / (2.0 * (1.0 - Rdp.z))) + 0.5;
		return texture( shadow_map, float4(backUV, 1.0, depth));
	}

	return 1.0h;
}

#endif  // CUBE_SHADOW

#endif // HAS_SHADOW


in float4 out_view_dir;
in float4 out_pos;
out float4 out_color { color(LIGHTING_RT_INDEX) };
void main()
{
	float2 tc = (out_pos.xy / out_pos.w * 0.5) + float2(0.5, 0.5);

	float depth = subpassLoad(gbuffer_depth_texture).x;
	/*
	if(depth == 1.0)
	{
		discard;
	}
	*/

	float3 view_dir = out_view_dir.xyz / out_view_dir.w;
	float3 world_pos = get_world_pos(depth, depth_parameters, view_dir, view_pos.xyz);
	view_dir = normalize(view_dir);

	float3 L_dir = light_pos.xyz - world_pos;
	float3 L = L_dir;
	float L_length2 = dot(L, L);

	// Attenuation
	float atten = 1.0 - L_length2 * attenuation_parameters.x;
	if(atten <= 0.0)
	{
		discard;
	}

	L = L / sqrt(L_length2);

	atten = pow(atten, attenuation_parameters.y);

	// Material params
	float3 albedo = subpassLoad(gbuffer_color_texture).xyz;
	float3 specular_color = subpassLoad(gbuffer_specular_texture).xyz;
	albedo = srgb_to_linear(albedo);
	specular_color = srgb_to_linear(specular_color);

	// PREC_TODO
	albedo = float3(get_energy_conservative_albedo(half3(albedo), half3(specular_color)));

	float4 tex_norm_gloss = subpassLoad(gbuffer_normal_texture);
	
	// PREC_TODO
	float3 normal = decode_normal_highp(tex_norm_gloss.xyz);

	float roughness = 1.0 - tex_norm_gloss.w;

#ifdef HAS_SHADOW
	#if CUBE_SHADOW
		half shadow = get_cube_shadow(shadow_light_pos.xyz - world_pos);
	#else
		half shadow = get_paraboloid_shadow(-normalize(L), depth);
	#endif
#else
	const half shadow = 1.0h;
#endif

	// Shading
	// PREC_TODO
	float3 diffuse = float3(direct_diffuse(half3(albedo), half3(normal), half3(L), half3(light_color.xyz), half(atten)));
	float3 specular = direct_specular(specular_color, roughness, normal, -view_dir, L, light_color.xyz, atten);

	float3 color = merge_direct_lighting(diffuse, specular, float(shadow));

	out_color = float4(color, 1.0);
}
