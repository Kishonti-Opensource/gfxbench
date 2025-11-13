/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
uniform sampler2D color_texture;
uniform sampler2D normal_texture;
uniform sampler2D specular_texture;
uniform sampler2D emissive_texture;

uniform float emissive_intensity;

in vec2 texcoord;
in vec3 world_position;
in vec3 normal;
in vec3 tangent;

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

vec3 linear_to_srgb(vec3 color)
{
	return vec3(
		linear_to_srgb(color.x),
		linear_to_srgb(color.y),
		linear_to_srgb(color.z));
}

mat3 calc_tbn(vec3 normal, vec3 tangent)
{
	vec3 n = normalize( normal);
	vec3 t = normalize( tangent);
	vec3 b = cross( t, n);
	b = normalize( b);
	return mat3( t, b, n);
}

vec3 calc_world_normal(vec3 tex_norm, vec3 normal, vec3 tangent)
{
	vec3 ts_normal = tex_norm * 2.0 - 1.0;

	mat3 tbn = calc_tbn( normal, tangent) ;

	// GL
	vec3 calc_normal = tbn * ts_normal;

	return normalize( calc_normal);
}

void main()
{
	// Color
	vec4 color = texture(color_texture, texcoord);
#ifdef ALPHA_TEST
	if( color.w < 0.5)
	{
		discard;
	}
#endif

	gl_FragData[0] = color;
}
