/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */

numthreads(WORKGROUP_SIZE_X, WORKGROUP_SIZE_Y, WORKGROUP_SIZE_Z);

uniform sampler2D<float> input_texture;


// Sum of the tile samples
buffer float values[] { ssbo };

shared float samples[THREAD_COUNT];

#ifdef DEBUG_HDR_UVS
image2D<float> hdr_uv_test_texture { rgba8, writeonly };

void MarkUV(float2 uv, float4 c)
{
	int2 coords = int2(float(HDR_WIDTH)*uv.x, float(HDR_HEIGHT)*uv.y);
	imageStore(hdr_uv_test_texture, coords, c);
}

void MarkUV_scr(float2 uv, float4 c)
{
	int2 coords = int2(trunc(uv));
	imageStore(hdr_uv_test_texture, coords, c);
}

#if 1

#define COLOR_COUNT 6u
void CheckUV(float2 uv)
{
	//if (!((gl_WorkGroupID.x == 0u) && (gl_WorkGroupID.y == 0u) && (gl_WorkGroupID.z == 0u))) return;
	
	const float4 colors[COLOR_COUNT] =
	{
		float4(1.0,0.0,0.0,1.0),
		float4(0.0,1.0,0.0,1.0),
		
		float4(1.0,1.0,0.0,1.0),
		float4(1.0,0.0,1.0,1.0),
		float4(0.0,1.0,1.0,1.0),
		
		float4(1.0,1.0,1.0,1.0)
	};
	
	MarkUV_scr(uv, colors[gl_LocalInvocationIndex % COLOR_COUNT]);
}

#else
	
void CheckUV(float2 uv)
{
	float2 scr_pos = float2(float(HDR_WIDTH)*uv.x,float(HDR_HEIGHT)*uv.y);
	float2 scr_fract = abs(fract(scr_pos) - 0.5) * 2.0;
	
	float i = scr_fract.x * scr_fract.y;
	
	const float3 r = float3(1.0,0.0,0.0);
	const float3 g = float3(0.0,1.0,0.0);
	
	float3 c = mix(r,g,i);
	
	MarkUV_scr(uv, float4(c,1.0));
}
	
#endif

#endif
	

void main()
{
	// Collect the samples
	float result = 0.0;
	
	float2 wg_offset = float2(gl_WorkGroupID.xy)*float2(HDR_WG_STEP_X,HDR_WG_STEP_Y);
	float2 thread_offset = wg_offset + float2(gl_LocalInvocationID.xy)*float2(HDR_THREAD_STEP_X,HDR_THREAD_STEP_Y);
	
	for (int y = 0; y < HDR_SAMPLE_COUNT_Y; y++)
	{
		float v = thread_offset.y + float(y) * HDR_ITERATION_STEP_Y;
		for (int x = 0; x < HDR_SAMPLE_COUNT_X; x++)
		{
			float u = thread_offset.x + float(x) * HDR_ITERATION_STEP_X;	
			
			float3 tap_color = texelFetch(input_texture, int2(u,v), 0).xyz;
			float l = max(0.000001, get_luminance(tap_color)); // Clamp to a small value to deal with log(0) and with accidental negative pixels(float texture)
			result += log(l);
			
#ifdef DEBUG_HDR_UVS
			CheckUV(float2(u,v));
#endif			
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
		values[gl_WorkGroupID.y * uint(HDR_DISPATCH_X) + gl_WorkGroupID.x] = samples[0];
	}
}
