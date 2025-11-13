/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
uniform sampler2D texture_unit0;
uniform sampler2D texture_unit3;
uniform sampler2D texture_unit2;
uniform sampler2D texture_unit1;
uniform sampler2D texture_unit4;
uniform samplerCube envmap0;
uniform samplerCube envmap1;
uniform sampler2DArray texture_array_unit0;

#ifdef USE_UBOs
	#include matConsts;
	#include envmapsInterpolatorConsts;
#else
	uniform float reflect_intensity;
	uniform float transparency; 
	uniform float envmaps_interpolator;
#endif

in vec2 out_texcoord0;
in vec3 out_normal;
in vec3 out_tangent;
in vec3 out_view_dir;

in vec3 out_eye_space_normal;
out vec4 frag_color;


void main()
{
	vec3 normal = normalize( out_normal);
	vec3 tangent = normalize( out_tangent);
	vec3 bitangent = cross( tangent, normal);

	vec3 texel_color = texture( texture_unit0, out_texcoord0).xyz;
	vec3 ts_normal = texture( texture_unit3, out_texcoord0).xyz;
#ifdef MASK
	vec4 mask = texture( texture_unit2, out_texcoord0);
#else
	vec4 mask = vec4( 1.0, 0.0, 0.0, 1.0);
#endif

#ifdef EMISSION
	vec4 emission = texture( texture_unit4, out_texcoord0);
#endif

#if defined ALPHA_TEST
	if( mask.x < 0.25)
	{
		discard;
	}
#endif

	ts_normal.xyz = ts_normal.xyz * 2.0 - 1.0;
		
	mat3 mat = mat3( tangent, bitangent, normal); 

	normal = mat * ts_normal;
	
	vec3 reflect_vector = reflect( out_view_dir, normal);
	vec3 env_color0 = texture( envmap0, reflect_vector).xyz;
	vec3 env_color1 = texture( envmap1, reflect_vector).xyz;

#ifdef USE_UBOs
	vec3 env_color = mix( env_color1.xyz, env_color0.xyz, envmaps_interpolator_pad3.x);
#else
	vec3 env_color = mix( env_color1.xyz, env_color0.xyz, envmaps_interpolator);
#endif
	
	#ifdef USE_UBOs
		vec3 color = mix( texel_color, env_color, matparams_disiseri.w * mask.z); 
	#else
		vec3 color = mix( texel_color, env_color, reflect_intensity * mask.z); 
	#endif
	
#ifdef EMISSION
	//color = emission.x * 8.0;
#endif

	#ifdef USE_UBOs
		frag_color = vec4( color, mask.x * fresnelXYZ_transp.w);
	#else
		frag_color = vec4( color, mask.x * transparency);
	#endif
	//frag_color = vec4( color, transparency);
	//frag_color = vec4( color, 0.5);
	
#ifdef SV_31
	frag_color.xyz *= 0.25; //target is GL_RGB10_A2, which has 4x precision compared to GL_RGBA8, so convert it to range, compress it
#endif			
}
