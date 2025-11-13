/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifdef TYPE_vertex

uniform float4x4 model;

in float3 in_position;
in float3 in_normal;
in float2 in_texcoord0_;

out float3 v_position;
out float3 v_normal;

void main()
{
	float2 tc = in_texcoord0_;

#ifdef NGL_ORIGIN_UPPER_LEFT_AND_NDC_FLIP
	tc.y = 1.0 - tc.y;
#endif

	gl_Position = float4( tc * 2.0 - 1.0, 0.0, 1.0);
	v_position = (model * float4(in_position, 1.0)).xyz;
	v_normal = (model * float4(in_normal, 0.0)).xyz;
}

#endif


#ifdef TYPE_fragment

#define shadow_texture shadow_map
#define light_atten attenuation_parameters

#ifdef HAS_SHADOW
uniform float4x4 shadow_matrix;

#if SHADER_CODE == 0

uniform float4 shadow_light_pos;
uniform samplerCubeShadow<float> shadow_map;

// cube_map
float get_cube_shadow(float3 Ldir)
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
	
	return texture(shadow_map, float4(-Ldir, d));
}

#elif SHADER_CODE == 1
uniform sampler2D<float> shadow_texture;
#endif

#endif

uniform float4 light_color;

#if SHADER_CODE == 0
uniform float4 light_pos;
uniform float4 light_atten;
#elif SHADER_CODE == 1
uniform float4 light_dir;
#endif

in float3 v_position;
in float3 v_normal;

out float4 result_color { color(0) };

void main()
{
	float3 world_position = v_position;
	float3 normal = v_normal;

	normal.xyz = normalize(normal.xyz);

#if SHADER_CODE == 0

    // cube_map
	float3 lv = light_pos.xyz - world_position.xyz;
	float dd = dot(lv, lv);
	float falloff = 1.0 - dd * light_atten.x;
	if( falloff < 0.0) discard;
	lv /= sqrt(dd);
	float ndotl = dot(normal.xyz, lv);
	if( ndotl < 0.0) discard;
	float intensity = ndotl * pow(falloff, light_atten.y);
	
#ifdef HAS_SHADOW
	float shadow = get_cube_shadow(shadow_light_pos.xyz - world_position.xyz);
	intensity *= shadow;
#endif
	
	result_color = light_color * float4(intensity);
#elif SHADER_CODE == 1
	float ndotl = dot(normal.xyz, light_dir.xyz);
	if( ndotl <= 0.0) discard;

#ifdef HAS_SHADOW
	float4 shadow_tc = shadow_matrix * float4(world_position, 1.0);

	if (shadow_tc.x < 0.0 || shadow_tc.x > 1.0 || shadow_tc.y < 0.0 || shadow_tc.y > 1.0 || shadow_tc.z < 0.0 || shadow_tc.z > 1.0)
	{
		discard;
	}

	shadow_tc.z = clamp(shadow_tc.z, 0.0, 1.0);

	float shadow = texture(shadow_texture, shadow_tc.xy).x;
	if( shadow < shadow_tc.z)
		discard;
#endif

	float intensity = ndotl;
	result_color = light_color * float4(intensity);
#endif
}

#endif
