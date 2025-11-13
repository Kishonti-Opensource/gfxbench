/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
out float4 out_color { color(0) };

uniform sampler2D<float> color_texture;
uniform sampler2D<float> bloom_texture;
uniform float4 bloom_parameters;

in float2 texcoord;
in float3 world_position;

void main()
{
	// Color
	float4 linear_color = float4(texture(color_texture, texcoord));
	float3 ldr_color;
	
	//out_color = linear_color;
	//return;

	//ldr_color = linear_color.xyz; //debug
	ldr_color = tonemapper(RGBMDecode(linear_color), float(hdr_exposure.y));

#if ENABLE_BLOOM
	float3 bloom0 = float3(textureLod( bloom_texture, texcoord, 0.0).xyz);
	//float3 bloom1 = float3(textureLod( bloom_texture, texcoord, 1.0).xyz);
	//float3 bloom2 = float3(textureLod( bloom_texture, texcoord, 2.0).xyz);
	//float3 bloom3 = float3(textureLod( bloom_texture, texcoord, 3.0).xyz);
	//float3 bloom = (bloom0 + bloom1 + bloom2 + bloom3) * 0.25h;
	float3 bloom = bloom0;

	bloom = bloom * bloom * float(bloom_parameters.y);
	ldr_color = ldr_color + bloom;
#endif

	// Gamma correction
//#if GAMMA_ENABLED
	ldr_color = clamp(ldr_color, float3(0.0), float3(1.0));
	ldr_color = linear_to_srgb(ldr_color);
//#endif

	out_color = float4(float3(ldr_color.xyz), 1.0);
}
