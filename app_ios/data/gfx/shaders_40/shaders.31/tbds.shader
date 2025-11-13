/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "tbds_header.shader"

#define MAX_LIGHTS 256
#define UINT_MAX uint(0xFFFFFFFF)

layout ( local_size_x = WORK_GROUP_WIDTH, local_size_y = WORK_GROUP_HEIGHT) in;

layout ( binding = 0, offset = 0) uniform atomic_uint light_counter;

#ifdef GL_ES
layout( binding = 1, rgba8) writeonly uniform mediump image2D outTexture;
#else
layout( binding = 1, rgba8) writeonly uniform image2D outTexture;
#endif

layout (std430, binding = 2) buffer BufferObject
{
    _light lights[MAX_LIGHTS];
};

uniform sampler2D texture_unit0;
uniform sampler2D texture_unit1;
uniform sampler2D texture_unit3;
uniform sampler2D texture_unit4;
uniform vec3 corners[4];
uniform vec4 depth_parameters;
uniform vec3 view_pos;
uniform vec2 inv_resolution;


shared uint minDepth;
shared uint maxDepth;
shared int used_light_index[MAX_LIGHTS];
shared int num_used_lights;

vec3 frustum_corners[8];
vec4 frustum_planes[6];

 
float DecodeFloatRG( vec2 value, float max_value) 
{
	const float max16int = 256.0 * 256.0 - 1.0;
	float result = 255.0 * dot(value, vec2(256.0, 1.0)) / max16int;
	
	result *= max_value;

	return result;
} 


vec3 get_world_pos(vec2 uv, float d) 
{
	vec3 dir0;
	vec3 dir1;
	vec3 dir2;
	
	dir0 = mix( corners[0].xyz, corners[1].xyz, uv.x);
	dir1 = mix( corners[2].xyz, corners[3].xyz, uv.x);
	dir2 = mix( dir0, dir1, uv.y);
	return d * dir2 + view_pos;
}


vec3 get_world_pos(vec2 uv) 
{
	float d  = texture( texture_unit3, uv).x; 
	d = depth_parameters.y / (d - depth_parameters.x);

	return get_world_pos( uv, d);
}


vec4 CreatePlane( vec3 a, vec3 b, vec3 c)
{
	vec3 e0 = a - c;
	vec3 e1 = b - c;
	
	vec4 result;

	result.xyz = cross( e0, e1);
	result.xyz  = normalize( result.xyz);
	result.w = -dot( result.xyz, a);
	return result;	
}


void CreateFrustum( vec2 l_b, vec2 r_b, vec2 l_t, vec2 r_t, float minDepthf, float maxDepthf)
{
	frustum_corners[0] = get_world_pos( l_b, minDepthf);
	frustum_corners[1] = get_world_pos( r_b, minDepthf);
	frustum_corners[2] = get_world_pos( l_t, minDepthf);
	frustum_corners[3] = get_world_pos( r_t, minDepthf);
	frustum_corners[4] = get_world_pos( l_b, maxDepthf);
	frustum_corners[5] = get_world_pos( r_b, maxDepthf);
	frustum_corners[6] = get_world_pos( l_t, maxDepthf);
	frustum_corners[7] = get_world_pos( r_t, maxDepthf);

	frustum_planes[0] = CreatePlane( frustum_corners[0], frustum_corners[1], view_pos);
	frustum_planes[1] = CreatePlane( frustum_corners[1], frustum_corners[3], view_pos);
	frustum_planes[2] = CreatePlane( frustum_corners[3], frustum_corners[2], view_pos);
	frustum_planes[3] = CreatePlane( frustum_corners[2], frustum_corners[0], view_pos);
	frustum_planes[4] = CreatePlane( frustum_corners[0], frustum_corners[1], frustum_corners[2]);
	frustum_planes[5] = CreatePlane( frustum_corners[4], frustum_corners[6], frustum_corners[5]);
}		


bool InFrustum( vec3 p_, float r)
{
	vec4 p = vec4( p_, 1.0);
	
	for( int i=0; i<6; i++)
	{
		float d = dot( frustum_planes[i], p);
		if( d > r)
		{
			return false;
		}
	}

	return true;
}


bool InFrustum3( vec3 p_, float r, vec3 pp_)
{
	if( !InFrustum( p_, r) )
	{
		return false;
	}

	vec4 pp = vec4( pp_, -dot( p_,pp_));
	
	for( int i=0; i<8; i++)
	{
		vec4 p = vec4( frustum_corners[i], 1.0);
		float d = dot( pp, p);
		if( d > 0.0)
		{
			return true;
		}
	}
	return false;
}


float Lighting( out vec3 light_dir, vec3 position, vec3 light_position, vec4 atten_parameters)
{
	light_dir = light_position - position;
	float sq_distance = dot( light_dir, light_dir);

	float atten = atten_parameters.y * sq_distance + 1.0;
	
	light_dir = light_dir / sqrt( sq_distance);

	return atten = clamp( atten, 0.0, 1.0);
}


float Lighting( out vec3 light_dir, vec3 position, vec3 light_position, vec4 atten_parameters, vec3 light_x)
{
	light_dir = light_position - position;
	float sq_distance = dot( light_dir, light_dir);

	float atten = atten_parameters.y * sq_distance + 1.0;
	
	atten = clamp( atten, 0.0, 1.0);
	
	light_dir = light_dir / sqrt( sq_distance);

	float fov_atten = dot( light_dir, -light_x.xyz);
	
	fov_atten = fov_atten - atten_parameters.z;
	
	fov_atten = clamp( fov_atten, 0.0, 1.0);

	fov_atten *= atten_parameters.w;

	atten *= fov_atten;

	return atten;	
}

#if 1
void main() 
{
	minDepth = UINT_MAX;
	maxDepth = 0u;
	num_used_lights = 0;
	
	barrier();

	ivec2 pixelPos = ivec2(gl_GlobalInvocationID.xy);
	vec2 uv = vec2( float( pixelPos.x) * inv_resolution.x, float( pixelPos.y) * inv_resolution.y);
	float d = texture( texture_unit3, uv).x; 

	uint depth = uint(d * float(UINT_MAX));

	atomicMin(minDepth, depth);
	atomicMax(maxDepth, depth);

	barrier();
	
	
	
	vec2 tilePos_l_b = vec2(gl_WorkGroupID.xy) * vec2(gl_WorkGroupSize.xy) * inv_resolution;
	vec2 tilePos_r_t = vec2( vec2(gl_WorkGroupID.xy + uvec2( 1u, 1u) ) * vec2(gl_WorkGroupSize.xy)) * inv_resolution;
	float minDepthf = float(minDepth) / float(UINT_MAX);
	float maxDepthf = float(maxDepth) / float(UINT_MAX);

	minDepthf = depth_parameters.y / (minDepthf - depth_parameters.x);
	maxDepthf = depth_parameters.y / (maxDepthf - depth_parameters.x);

	CreateFrustum( tilePos_l_b, vec2( tilePos_r_t.x, tilePos_l_b.y), vec2( tilePos_l_b.x, tilePos_r_t.y), tilePos_r_t, minDepthf, maxDepthf); 

	int num_lights = int( atomicCounter( light_counter));
	
	int threadCount = WORK_GROUP_HEIGHT * WORK_GROUP_WIDTH;
	int passCount = (num_lights + threadCount - 1) / threadCount;

	for (int passIt = 0; passIt < passCount; ++passIt)
	{
		int light_idx =  passIt * threadCount + int(gl_LocalInvocationIndex);

		if( light_idx >= num_lights)
		{
			break;
		}
		
		// light_idx = min( light_idx, num_lights);

		_light light = lights[light_idx];

		bool b;

		if( light.position_type.w == 0.0)
		{
			b = InFrustum( light.position_type.xyz, light.atten_parameters.x);
		}
		else
		{
			//b = InFrustum( light.position_type.xyz, light.atten_parameters.x);
			b = InFrustum3( light.position_type.xyz, light.atten_parameters.x, light.dir.xyz);
		}
		
		if( b)
		{
			int id = atomicAdd(num_used_lights, 1);
			used_light_index[id] = light_idx;
		}
	}

	barrier();

	vec4 result_color = vec4( 0.0, 0.0, 0.0, 1.0);
	vec3 originVS = get_world_pos( uv);
	vec3 normal = texture( texture_unit1, uv).xyz;
	vec4 texel_color = texture( texture_unit0, uv);
	vec4 float_params = texture( texture_unit4, uv);
	vec3 view_dir = normalize( originVS - view_pos);

	normal.xyz = normal.xyz * 2.0 - 1.0;
	normal.xyz = normalize( normal.xyz);

	float A = DecodeFloatRG( float_params.rg, 128.0);
	float B = DecodeFloatRG( float_params.ba, 4096.0);

	for (int j = 0; j < num_used_lights; j++)
	{
		int lightIndex = used_light_index[j];

		_light light = lights[lightIndex];
	
		float atten;

		vec3 light_dir;
			
		if( light.position_type.w == 0.0)
		{
			atten = Lighting( light_dir, originVS, light.position_type.xyz, light.atten_parameters);
		}
		else
		{
			atten = Lighting( light_dir, originVS, light.position_type.xyz, light.atten_parameters, light.dir.xyz);
		}

		vec3 half_vector = normalize( light_dir - view_dir);
		float diffuse = dot( normal.xyz, light_dir);	
		diffuse = clamp( diffuse, 0.0, 1.0);

		float specular = dot( normal.xyz, half_vector);
		specular = clamp( specular, 0.0, 1.0);

		specular = A * pow( specular, B);

		vec3 c = (texel_color.xyz * diffuse * 3.0 + specular) * light.color.xyz * atten;

		result_color.xyz += c;
	}

	imageStore(outTexture, pixelPos, result_color);
}
#else
void main() {ivec2 pixelPos = ivec2(gl_GlobalInvocationID.xy);imageStore(outTexture, pixelPos, vec4( 1.0));}
#endif
