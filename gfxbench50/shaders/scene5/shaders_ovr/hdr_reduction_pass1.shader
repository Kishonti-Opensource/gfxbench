/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */

numthreads(WORKGROUP_SIZE_X, WORKGROUP_SIZE_Y, WORKGROUP_SIZE_Z);

#define THREAD_COUNT 128

uniform sampler2D<float> input_texture;

uniform float2 sample_count;
uniform float2 texel_center;  // float texel size of the original texture
uniform float2 step_uv;       // Step between the samples

uniform uint4 NumWorkGroups;


// Sum of the tile samples
buffer float values[] { ssbo };

shared float samples[THREAD_COUNT];

void main()
{
	// Collect the samples
	float result = 0.0;
	float2 offset = step_uv * (float2(gl_GlobalInvocationID.xy) * sample_count) + texel_center;
	for (int y = 0; y < int(sample_count.y); y++)
	{
		float v = offset.y + float(y) * step_uv.y;
		for (int x = 0; x < int(sample_count.x); x++)
		{
			float u = offset.x + float(x) * step_uv.x;
			float3 tap_color = textureLod(input_texture, float2(u, v), 0.0).xyz;
			float l = max(0.000001, get_luminance(tap_color)); // Clamp to a small value to deal with log(0) and with accidental negative pixels(float texture)
			result += log(l);
		}
	}
	samples[gl_LocalInvocationIndex] = result;

	workgroupMemoryBarrierShared();

	// Parallel reduction
	for(uint s = uint(THREAD_COUNT) / uint(2); s > uint(0); s = s >> uint(1))
	{
		if(gl_LocalInvocationIndex < s)
		{
			samples[gl_LocalInvocationIndex] += samples[gl_LocalInvocationIndex + s];
		}
		workgroupMemoryBarrierShared();
	}

	// The first thread stores the result
	if (gl_LocalInvocationIndex == uint(0))
	{
		values[gl_WorkGroupID.y * NumWorkGroups.x + gl_WorkGroupID.x] = samples[0];
	}
}
