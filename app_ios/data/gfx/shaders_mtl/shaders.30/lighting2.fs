/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */

#define HALF_SIZED_LIGHTING

struct LightInfo
{
	float4 light_color;
	float4 light_position;
	float4 light_xs;
//	float4 light_y;
//	float4 light_z;
	//packed:
	//float2 spot_coss
	//float attenuation_params
	float3 spot_coss_attenuation_params;
};


float DecodeFloatRG( float2 value, float max_value)
{
	const float max16int = 256.0 * 256.0 - 1.0;
	float result = 255.0 * dot(value, float2(256.0, 1.0)) / max16int;
	
	result *= max_value;
	
	return result;
}

/*
vec3 get_world_pos( vec2 out_texcoord)
{
	float d = texture( texture_unit3, out_texcoord).x;
	
	d = depth_parameters.y / (d - depth_parameters.x);
	
	return d * out_view_dir.xyz / out_view_dir.w + view_pos;
}
 */

float3 get_world_pos(texture2d<float> tex, sampler sam, float2 out_texcoord, float4 view_dir, constant FrameConsts* frameConsts)
{
	float d = tex.sample(sam, out_texcoord).x;
	
	d = frameConsts->depth_parameters.y / (d - frameConsts->depth_parameters.x);
	
	return d * view_dir.xyz / view_dir.w + frameConsts->view_posXYZ_time.xyz;
}

fragment float4 shader_main(LightingVertexOutput input [[stage_in]],
					 constant LightInfo* lights [[buffer(8)]],
					 constant uint* num_lights [[buffer(9)]],
					 constant FrameConsts* frameConst [[buffer(10)]],
					 constant LightConsts* lightConst [[buffer(13)]],
					 texture2d<float> unit0 [[texture(0)]],
					 texture2d<float> unit1 [[texture(1)]],
					 texture2d<float> unit3 [[texture(3)]],
					 texture2d<float> unit4 [[texture(4)]],
					 texture2d_depth<float> shadow_unit0 [[texture(5)]],
					 sampler sam0 [[sampler(0)]],
					 sampler sam1 [[sampler(1)]],
					 sampler sam3 [[sampler(3)]],
					 sampler sam4 [[sampler(4)]],
					 sampler shadowSampler [[sampler(5)]])
{
	float shadow = 1.0;
#ifdef HALF_SIZED_LIGHTING
	float2 out_texcoord = float2( input.position.xy) * frameConst[0].inv_resolutionXY_pad.xy*2.0f;
#else
	float2 out_texcoord = float2( input.position.xy) * frameConst[0].inv_resolutionXY_pad.xy;
#endif
	float3 view_dir = normalize( input.view_dir.xyz);
	
	float4 texel_color = unit0.sample(sam0, out_texcoord);
	
	float4 normal = unit1.sample(sam1, out_texcoord);
	
	normal.xyz = normal.xyz * 2.0 - 1.0;
	normal.xyz = normalize( normal.xyz);
	
	float4 float_params = unit4.sample(sam4, out_texcoord);
	
	float A = DecodeFloatRG( float_params.rg, 128.0);
	float B = DecodeFloatRG( float_params.ba, 4096.0);
	
	float3 position = get_world_pos(unit3, sam3, out_texcoord, input.view_dir, frameConst);
	
	float4 frag_color = float4( 0.0);
	
#if 1
	for( int i=0; i< num_lights[0]; i++)
	{
		//float3 light_dir = light_positions[i] - position;
		float3 light_dir = lights[i].light_position.xyz - position;
		float sq_distance = dot( light_dir, light_dir);
		
		float atten = lights[i].spot_coss_attenuation_params.z * sq_distance + 1.0;
		
		atten = clamp( atten, 0.0, 1.0);
		
		light_dir = light_dir / sqrt( sq_distance);
		
		float fov_atten = dot( light_dir, -lights[i].light_xs.xyz);
		
		fov_atten = fov_atten - lights[i].spot_coss_attenuation_params.x;
		
		fov_atten = clamp( fov_atten, 0.0, 1.0);
		
		//fov_atten /= (1.0 - 0.9);
		fov_atten *= lights[i].spot_coss_attenuation_params.y;
		
		atten *= fov_atten;
		
#if defined SHADOW_MAP
		float4 p = float4( position, 1.0);
		float4 shadow_texcoord = lightConst[0].shadowMatrix * p;
		shadow_texcoord = shadow_texcoord / shadow_texcoord.w;
		
		shadow_texcoord.xy = shadow_texcoord.xy * 0.5 + float2(0.5);
	
		//flip y in shadow lookup?
		shadow_texcoord.y = 1.0f - shadow_texcoord.y;
	
		float refvalue = clamp(shadow_texcoord.z, 0.0f, 1.0f);
		
		shadow = shadow_unit0.sample_compare(shadowSampler, shadow_texcoord.xy, refvalue);
		
		atten *= shadow;
#endif
		
		float3 half_vector = normalize( light_dir - view_dir);
		
		float diffuse = dot( normal.xyz, light_dir);
		
		diffuse = clamp( diffuse, 0.0, 1.0);
		
		float specular = dot( normal.xyz, half_vector);
		specular = clamp( specular, 0.0, 1.0);
		specular = A * pow( specular, B);
		
#ifdef HALF_SIZED_LIGHTING
		frag_color += float4( diffuse * lights[i].light_color.xyz * atten, diffuse * specular * atten);
#else
		float3 c = (texel_color.xyz * diffuse + specular) * lights[i].light_color.xyz * atten;
		
		
		
		
		//c = vec3( 0.01) * light_colors[i];
		frag_color += float4( c, 1.0);
#endif
	}
#endif
	
#if defined POINT_LIGHT || defined SPOT_LIGHT
	//frag_color = vec4( position, 1.0);
	//frag_color = vec4( d);
	//frag_color = vec4( d1 * 0.1);
	//frag_color = out_view_dir;
#endif
	
	//frag_color = vec4( d);
	//frag_color = vec4( specular);
	
	//frag_color = vec4( light_dir, 1.0);
	//frag_color = light_dir.y;
	//frag_color = vec4( atten);
	//frag_color = vec4( diffuse);
	//frag_color = vec4( texel_color);
	//frag_color = vec4( light_color * atten, 1.0);
	
#if defined SHADOW_MAP
	//frag_color.xyz = out_texcoord0;
#endif
	
	//frag_color = vec4( light_color, 0.0);

	return frag_color;
}

