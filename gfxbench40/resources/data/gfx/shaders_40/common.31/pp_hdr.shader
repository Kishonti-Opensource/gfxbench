/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifdef TYPE_fragment

uniform mediump sampler2D texture_unit0; // HDR color target
#if defined(SV_40) && !defined(LIGHTCOMBINE_FP_RENDERTARGET_ENABLED)
	uniform mediump sampler2D texture_unit3; // LDR transparent color target
#endif
uniform sampler2D texture_unit2; // RGBA_8888 bloom texture

out vec4 frag_color;
in vec2 out_texcoord0;

#ifdef SV_40
#include "sat_functions.h"
#include "rgbm_helper.h"
#endif

void main()
{
	vec4 final_color = vec4(0.0);

    // Color texture
	vec4 c0 = texture( texture_unit0, out_texcoord0);

    // Bloom textures
	vec3 b1 = texture( texture_unit2, out_texcoord0,0.0).xyz;
	vec3 b2 = texture( texture_unit2, out_texcoord0,1.0).xyz;
	vec3 b3 = texture( texture_unit2, out_texcoord0,2.0).xyz;
	vec3 b4 = texture( texture_unit2, out_texcoord0,3.0).xyz;

    //////////////////
	// Bloom debug
	bool a1 = (out_texcoord0.x < 0.5) && (out_texcoord0.y < 0.5);
	bool a2 = (out_texcoord0.x > 0.5) && (out_texcoord0.y < 0.5);
	bool a3 = (out_texcoord0.x < 0.5) && (out_texcoord0.y > 0.5);
	bool a4 = (out_texcoord0.x > 0.5) && (out_texcoord0.y > 0.5);
	if (a1) {
		vec2 tc = vec2(2.0)*out_texcoord0;
		final_color = texture( texture_unit2, tc,0.0);
	} else if(a2) {
		vec2 tc = vec2(2.0)*(out_texcoord0+vec2(-0.5,0));
		final_color = texture( texture_unit2, tc,1.0);
	} else if (a3) {
		vec2 tc = vec2(2.0)*(out_texcoord0+vec2(0,-0.5));
		final_color = texture( texture_unit2, tc,2.0);
	} else if (a4) {
		vec2 tc = vec2(2.0)*(out_texcoord0+vec2(-0.5,-0.5));
		final_color = texture( texture_unit2, tc,3.0);
	}
    //////////////////
#ifdef SV_40
	vec3 bloom = (b1 + b2 + b3 + b4) * 0.25; //0.05 * b1 + 0.1 * b2 + 0.2 * b3 + 0.4 * b4; //TODO: expose bloom magnitude setting
	bloom = pow(bloom, vec3(2.0)) * 50.0;
#endif
	
#if defined(SV_40) && !defined(LIGHTCOMBINE_FP_RENDERTARGET_ENABLED)
	highp vec3 solid_color = RGBDtoRGB_lightcombine( c0 );
	vec4 transparent_accum = texture( texture_unit3, out_texcoord0);
	transparent_accum.xyz = pow(transparent_accum.xyz, vec3(2.2)); //convert back to linear values

	//tonemap and add bloom
	highp vec3 ldr_solid_color = compose(solid_color,bloom);
	highp vec3 transp_color = compose(transparent_accum.xyz,bloom);

	//programmable bleding of offscreen transparents with premultiplied alpha :)
	highp vec3 merged = transp_color * 1.0 + ldr_solid_color * (1.0 - transparent_accum.w);

	highp vec3 fc = merged.xyz;
#elif defined(SV_40)
	vec3 fc = compose(RGBDtoRGB_lightcombine( c0 ),bloom);
#else // SV_31
	vec3 fc = compose(c0.xyz,b1,b2,b3,b4);
#endif

#ifdef SV_40
#ifdef DRAW_GS_WIREFRAME
	fc = RGBDtoRGB_lightcombine(c0);
#endif
#else
	//fc = c0.xyz;	
#endif

    // NOTE: Comment this out to debug bloom textures
	final_color = vec4( fc, 1.0);

    // NOTE: Remove comment to disable bloom
	//final_color = vec4(tone_map(c0.xyz * 4.0), 1.0); // uncompress GL_RGB10_A2

#ifdef USE_SHADER_GAMMA_CORRECTION
	final_color.x = pow(final_color.x, 0.45);
	final_color.y = pow(final_color.y, 0.45);
	final_color.z = pow(final_color.z, 0.45);
#endif

	frag_color = final_color;
}
#endif

#ifdef TYPE_vertex

in vec3 in_position;
out vec2 out_texcoord0;
void main()
{
	gl_Position = vec4( in_position, 1.0);
	out_texcoord0 = in_position.xy * 0.5 + 0.5;
}

#endif
