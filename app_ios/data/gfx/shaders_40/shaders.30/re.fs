/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
//#define HALF_SIZED_LIGHTING

in vec2 out_texcoord0;
out vec4 frag_color;

uniform sampler2D texture_unit0; //col
uniform sampler2D texture_unit1; //rgb: norm, a: emimask
uniform sampler2D texture_unit2; //light (diff, spec)
uniform sampler2D texture_unit3; //rgb: refl, a: reflmask
uniform sampler2D texture_unit4; //depth

uniform vec2 inv_resolution;

void main()
{
	vec4 color;
	vec4 normal;
	vec4 light;
	vec4 reflection;

	vec4 color_tex = texture( texture_unit0, out_texcoord0);
	vec4 normal_tex = texture( texture_unit1, out_texcoord0);
	vec4 light_tex = texture( texture_unit2, out_texcoord0);
#ifdef SV_31
	light_tex *= 4.0; //target is GL_RGB10_A2, which has 4x precision compared to GL_RGBA8, and we use it as range instead, decompress it
#endif	
	vec4 reflection_tex = texture( texture_unit3, out_texcoord0);	
	
#ifdef HALF_SIZED_LIGHTING
	ivec2 texcoord0 = ivec2(out_texcoord0 / inv_resolution / 2.0);
	vec4 l0 = texelFetch ( texture_unit2, texcoord0,0);
	vec4 l1 = texelFetchOffset ( texture_unit2, texcoord0, 0, ivec2(  1,  0));
	vec4 l2 = texelFetchOffset ( texture_unit2, texcoord0, 0, ivec2(  0,  1));
	vec4 l3 = texelFetchOffset ( texture_unit2, texcoord0, 0, ivec2( -1,  0));
	vec4 l4 = texelFetchOffset ( texture_unit2, texcoord0, 0, ivec2(  0, -1));
		
	float d0 = texelFetch( texture_unit4, texcoord0,0).x;
	float d1 = texelFetchOffset( texture_unit4, texcoord0, 0, ivec2(  1,  0)).x;
	float d2 = texelFetchOffset( texture_unit4, texcoord0, 0, ivec2(  0,  1)).x;
	float d3 = texelFetchOffset( texture_unit4, texcoord0, 0, ivec2( -1,  0)).x;
	float d4 = texelFetchOffset( texture_unit4, texcoord0, 0, ivec2(  0, -1)).x;
	
	float diff1 = abs(d0 - d1);
	float diff2 = abs(d0 - d2);
	float diff3 = abs(d0 - d3);
	float diff4 = abs(d0 - d4);
	
	float eps = 1.0;
	float w0 = 1.0;
	float w1 = 1.0 / (eps + diff1 * 10000.0);
	float w2 = 1.0 / (eps + diff2 * 10000.0);
	float w4 = 1.0 / (eps + diff4 * 10000.0);
	float w3 = 1.0 / (eps + diff3 * 10000.0);
	
	float tw = w0 + w1 + w2 + w3 + w4;
	
	light_tex = (w0 * l0 + w1 * l1 + w2 * l2 + w3 * l3 + w4 * l4) / tw;
#endif
	
	color = color_tex;
	normal = normal_tex;
	light = light_tex;
	reflection = reflection_tex;

#ifdef SV_30
	vec3 ambiLightCol = vec3(0.0/255.0, 30.0/255.0, 50.0/255.0);
#endif

#ifdef SV_31
	vec3 ambiLightCol = vec3(0.0/255.0, 15.0/255.0, 20.0/255.0);
#endif

#ifdef HALF_SIZED_LIGHTING
	
	light *= 3.0;
	
	float lum = dot(light.xyz, vec3(0.3, 0.59, 0.11)) + 0.001; //avoid divide by zero
	vec3 diffChrominance = light.xyz / lum; //reconstruct light color to modulate spec scalar for l-buffer
	vec3 diffCol = light.xyz;
	vec3 specCol = light.w * diffChrominance * 0.3;
	vec3 matCol = color.xyz;
	//vec4 final = vec4(matCol * (ambiLightCol + diffCol + specCol), 1.0);
	vec4 final = vec4(matCol * (ambiLightCol + diffCol) + specCol, 1.0);
	
	final = mix( final, reflection, reflection.a); //final vs refl
	final = mix( final, color, normal.a); //(final vs refl) vs boosted albedo (emissive)
	
	frag_color.rgb = final.xyz;
	frag_color.a = normal.a;
#else
	light.xyz += color.xyz * ambiLightCol;

	light = mix( light, reflection, reflection.a);
	light = mix( light, color, normal.a);	
	
	frag_color = light;
	frag_color.a = normal.a;
#endif

#ifdef SV_31
	frag_color.xyz *= 0.25; //target is GL_RGB10_A2, which has 4x precision compared to GL_RGBA8, so convert it to range, compress it
#endif	
}
