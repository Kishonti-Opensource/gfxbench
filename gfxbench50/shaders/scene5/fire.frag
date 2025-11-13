/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
uniform sampler2D<float> emissive_texture; // fire color texture
uniform sampler2D<float> aux_texture0; // fire alpha texture
uniform sampler2D<float> aux_texture1; // fire noise texture

uniform float emissive_intensity;

in float2 v_texcoord0;
in float2 v_texcoord1;
in float2 v_texcoord2;
in float2 v_texcoord3;

out float4 out_color { color(0) };

void main()
{
	// parameters
	const float4 distortion = float4(0.2,0.3,0.1,1.0);

	float2 noise1 = texture(aux_texture1, v_texcoord1).xy;
	float2 noise2 = texture(aux_texture1, v_texcoord2).xy;
	float2 noise3 = texture(aux_texture1, v_texcoord3).xy;

	noise1 = noise1 * float2(2.0) - float2(1.0);
	noise2 = noise2 * float2(2.0) - float2(1.0);
	noise3 = noise3 * float2(2.0) - float2(1.0);

	noise1 *= float2(0.1, distortion.x);
	noise2 *= float2(0.1, distortion.y);
	noise3 *= float2(0.1, distortion.z);

	float2 finalNoise = noise1 + noise2 + noise3;

	float P = 1.0 - pow(v_texcoord0.y, 2.0);

	float2 noise_coords = (finalNoise.xy * P) + v_texcoord0.xy * float2(1.0, 1.0);

	float alpha = texture(aux_texture0, noise_coords).x;
	alpha *= 2.0;

	out_color = emissive_intensity * texture(emissive_texture, noise_coords) * alpha;
	out_color.w = min(out_color.w, 1.0);
}

