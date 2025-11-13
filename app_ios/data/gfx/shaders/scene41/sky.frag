/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifdef KSL_COMPILER

#define sampler2D hsampler2D
#define vec4 float4
#define vec3 float3
#define vec2 float2

out float4 res { color(0) };

#endif

uniform sampler2D color_texture;
vec3 RGBMtoRGB( vec4 rgbm )
{
	vec3 dec = 6.0 * rgbm.rgb * rgbm.a;
	return dec*dec; //from "gamma" to linear
}

vec3 RGBEtoRGB( vec4 rgbe )
{
 	float exponent = rgbe.a * 255.0 - 128.0;
	return rgbe.rgb * exp2(exponent);
}

in vec2 texcoord0;
void main()
{
	vec4 color_hdr = texture(color_texture, texcoord0);

#ifdef SKY_FORMAT_RGBM
	vec3 color_rgb = RGBMtoRGB(color_hdr);
#elif SKY_FORMAT_RGBE
	vec3 color_rgb = RGBEtoRGB(color_hdr);
#elif SKY_FORMAT_RGB
	vec3 color_rgb = color_hdr.xyz;
#else
	"ERROR! Sky format not set!"
#endif

#ifdef KSL_COMPILER
	res = float4(color_rgb, 1.0);
#else
	gl_FragData[0] = vec4(color_rgb, 1.0);
#endif
}
