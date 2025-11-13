/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
out float4 out_color { color(0) };

uniform sampler2D<float> color_texture;
//uniform sampler2D<float> gbuffer_depth_texture;
uniform float4 orig_texture_size;

in float2 texcoord;
in float3 world_position;

void main()
{
	float2 uv = gl_FragCoord.xy * orig_texture_size.xy;
	/*float depth = texture(gbuffer_depth_texture, uv).x;
	if(depth != 1.0) //only draw where there's no geometry
	{
		discard;
	}*/

	// Color
	float4 color = texture(color_texture, texcoord);

	out_color = float4(RGBMEncode(color.xyz));
}
