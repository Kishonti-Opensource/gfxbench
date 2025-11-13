/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */


fragment float4 shader_main(FogVertexOutput input [[stage_in]],
							constant FrameConsts* frame [[buffer(10)]],
							constant LightConsts* light [[buffer(13)]],
							texture2d<float> unit0 [[texture(0)]],
							texture2d<float> unit1 [[texture(1)]],
							texture3d<float> unit2 [[texture(2)]],
							sampler sam0 [[sampler(0)]],
							sampler sam1 [[sampler(1)]],
							sampler sam2 [[sampler(2)]])
{
	float t = frame[0].view_posXYZ_time.z * 8.0;
	
	float2 depth_texcoord = input.position.xy * frame[0].inv_resolutionXY_pad.xy;
	float d = unit0.sample(sam0, depth_texcoord).x;
	float z_view_depth = frame[0].depth_parameters.y / (d - frame[0].depth_parameters.x);
	float depthDiffScale = 0.1f;
	float depth_atten = clamp((z_view_depth - input.out_pos.w) * depthDiffScale, 0.0f, 1.0f);
	
	float3 out_texcoord0 = input.shadow_texcoord.xyz / input.shadow_texcoord.w;
	
	float world_atten = clamp((5.0f - input.out_pos.y) / 5.0f, 0.0f, 1.0f); ///texture( texture_unit0, out_pos.xz / 50.0f) *
	
	//atten = texture( texture_3D_unit0, vec3(out_pos.xz / 120.0f + t / 10.0f, t)).x * world_atten; //texture( texture_3D_unit0, vec3(out_texcoord0.xy,0));
	float atten = 1.0 * unit2.sample(sam2, float3(input.out_pos.xz / 120.0f + t / 10.0f, input.out_pos.y / 20.0f - t)).x * world_atten * 5.0f; //texture( texture_3D_unit0, vec3(out_texcoord0.xy,0));
	/*
	 vec2 offset = vec2(-112.0f, -98.0f);
	 float scale = 1.0f / 408.8f;
	 vec2 volColUV = (out_pos.zx - offset) * vec2(scale, scale);
	 vec3 volumeColor = texture( texture_unit1, volColUV ).xyz * 2.0f * clamp((15.0f - out_pos.y) / 5.0f, 0.0f, 1.0f);;
	 */
	float4 frag_color = float4( light[0].light_colorXYZ_pad.xyz /*+ volumeColor*/, atten) * float4( 0.2f, 0.2f, 0.2f, 0.2f);
	
	//frag_color = vec4(z_view_depth.xxx/15,1.0f);
	
	
	
	
	float fogValue = unit2.sample(sam2, float3(input.out_pos.xz / 60.0f + t / 2.0f, input.out_pos.y / 5.0f - t)).x;
	
	float fogColor = 0.2f;
	//frag_color = vec4(fogColor, fogColor, fogColor, fogValue * world_atten * depth_atten * 0.6f);
	frag_color = float4(fogColor, fogColor, fogColor, fogValue * 2.0 * world_atten * depth_atten * 1.0f);
	
	//frag_color = vec4(0,0,0,0);
	
	//discard;
	
	//frag_color = vec4(depth_atten,depth_atten,depth_atten,1.0f);
	//frag_color.xyz = out_pos.www/10;
	//frag_color.w = 1.0f;
	//frag_color = vec4(out_scrollspeed,out_scrollspeed,out_scrollspeed,1);
	
	//frag_color = vec4(texture( texture_unit0, depth_texcoord).xxx,1);
	
	return frag_color;
}

