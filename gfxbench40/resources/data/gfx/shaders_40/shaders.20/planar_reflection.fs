/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
uniform vec3 global_light_dir;
uniform vec3 global_light_color;
uniform vec3 view_dir;
uniform vec3 background_color;
uniform float time;
uniform sampler2D texture_unit0;
uniform sampler2D texture_unit1;
uniform sampler2D texture_unit2;
uniform sampler2D texture_unit3;
uniform float diffuse_intensity;
uniform float specular_intensity;
uniform float specular_exponent;
uniform float reflect_intensity;
uniform samplerCube envmap0;
uniform samplerCube envmap1;
uniform float envmaps_interpolator;
uniform float transparency;
uniform sampler2D shadow_unit0;
uniform samplerCube normalization_cubemap;
uniform sampler2D planar_reflection;
uniform mediump vec2 inv_resolution; //defaults to highp in shader.cpp / kept mediump here for legacy reasons


varying vec2 out_texcoord0;


#if defined FOG
varying float fog_distance;
#endif	


vec3 normalize2( vec3 v)
{
#ifdef USE_NORMALIZATION_CUBE_MAP
	vec3 r = textureCube( normalization_cubemap, v).xyz;
	r = r * 2.0 - 1.0;
	return r;
#else
	return normalize( v);
#endif
}




vec3 planar_reflection2( vec3 color)
{
	vec3 reflection_color;

	vec3 ts_normal = texture2D( texture_unit3, out_texcoord0).xyz * 0.04 - 0.02;
	
	vec2 tc = gl_FragCoord.xy * inv_resolution + ts_normal.xy;
	
	reflection_color = texture2D( planar_reflection, tc).xyz;
	
	vec3 result = reflection_color;
		
	return result;
}






void main()
{   
#if defined MASK
	vec4 mask = texture2D( texture_unit2, out_texcoord0);
#else
	const vec4 mask = vec4( 1.0, 1.0, 1.0, 1.0);
#endif

#if defined ALPHA_TEST
	if( mask.x < 0.6)
	{
		discard;
	}
#endif

	vec3 color = texture2D( texture_unit0, out_texcoord0).xyz;

	color = planar_reflection2( color);

#if defined FOG
	color = mix( color, background_color, fog_distance * fog_distance);
#endif	

	
#if defined TRANSPARENCY
	float trp = mask.z * transparency;
	gl_FragColor = vec4( color, trp);
#else
	gl_FragColor = vec4( color, 1.0);
#endif

}
