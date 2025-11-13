/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
uniform sampler2D<float> texture_unit0;
uniform sampler2D<float> texture_unit7;
uniform int gauss_lod_level;
uniform float4 depth_parameters; //

uniform float2 inv_resolution;

in float2 out_texcoord0;
out float4 out_color { color(0) };

float bilateralBlendWeight(float centerDepth, float sampleDepth)
{
	float tolerance = 100.0;
    float weights = min(1.0/(abs(centerDepth - sampleDepth)*tolerance+0.001), 1.0);
	return weights;
}

float get_linear_depth( float depth )
{
	return depth_parameters.y / (depth - depth_parameters.x);
}

void main()
{
	float center_linear_depth = textureLod(texture_unit7, out_texcoord0, float(gauss_lod_level)).x;
	
	float3 color = float3(0.0);
	float weight_accum = 0.0;
	for (int i = 0; i < KS; i++)
	{
		// TODO: This can be pre-multiplied
#if defined HORIZONTAL
		float2 offset = float2(inv_resolution.x * gauss_offsets[i].x, 0.0);
#elif defined VERTICAL
		float2 offset = float2(0.0, inv_resolution.y * gauss_offsets[i].x);
#endif
		float sample_linear_depth = textureLod(texture_unit7, out_texcoord0 + offset, float(gauss_lod_level)).x;
		float bilateral_weight = bilateralBlendWeight(center_linear_depth, sample_linear_depth);
		weight_accum += bilateral_weight;
		color += gauss_weights[i].x * bilateral_weight * textureLod( texture_unit0, out_texcoord0 + offset, float(gauss_lod_level)).xyz;
	}
	out_color = float4(color / weight_accum, 1.0);
}
