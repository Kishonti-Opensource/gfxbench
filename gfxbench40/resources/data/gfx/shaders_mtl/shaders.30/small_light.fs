/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */

fragment float4 shader_main(	LightingVertexVertexOutput input [[stage_in]],
								constant FrameConsts* frameConst [[buffer(10)]],
								constant LightConsts* lightConst [[buffer(13)]],
								texture2d<float> unit0 [[texture(0)]],
								texture2d<float> unit1 [[texture(1)]],
								texture2d<float> unit3 [[texture(2)]],
								texture2d<float> shadow_unit0 [[texture(4)]],
								sampler sam0 [[sampler(0)]],
								sampler sam1 [[sampler(1)]],
								sampler sam3 [[sampler(2)]],
								sampler shadowSampler [[sampler(4)]])

{
	float4 frag_color;

	float2 out_texcoord = float2( input.position.xy) * frameConst[0].inv_resolutionXY_pad.xy;
	//TODO bug in the shader? they normalize but never use it, and use the unnormalized direction?
	float3 view_dir = normalize( input.view_dir.xyz);
	float4 texel_color = unit0.sample(sam0, out_texcoord);
	float4 float_params = unit3.sample(sam3, out_texcoord);
	
	float d = float_params.r;
	float3 position = d * input.view_dir.xyz / input.view_dir.w + frameConst[0].view_posXYZ_time.xyz;
	
	float3 light_dir = lightConst[0].light_posXYZ_pad.xyz - position;
	float sq_distance = dot( light_dir, light_dir);
	
	float atten = lightConst[0].spotcosXY_attenZ_pad.z * sq_distance + 1.0;
	
	if( atten <= 0.0)
	{
		discard_fragment();
	}
	
	atten = clamp( atten, 0.0, 1.0);
	
	light_dir = light_dir / sqrt( sq_distance);
	
	float3 c = texel_color.xyz * lightConst[0].light_colorXYZ_pad.xyz * atten;
	frag_color = float4( c, 1.0);
	//frag_color = vec4( 1.0);
    
	return frag_color;
}



