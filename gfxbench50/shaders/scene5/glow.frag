/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
uniform sampler2D<float> emissive_texture;

uniform float emissive_intensity;

in float2 v_texcoord0;

out float4 out_color { color(0) };

void main()
{
	out_color = texture(emissive_texture, v_texcoord0);
	out_color *= out_color.w;
	out_color.xyz *= float3(0.0, 0.7, 1.0) * emissive_intensity;
}

