/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
//#define HALF_SIZED_LIGHTING

#ifdef INSTANCING
#undef USE_UBOs
#include cameraConsts;
#include lightInstancingConsts;
flat in int instanceID;
#else //INSTANCING
#ifdef USE_UBOs
	#include cameraConsts;
	#include lightConsts;
#else	
	uniform vec3 view_pos;
	uniform vec4 depth_parameters;
	
	uniform vec3 light_color;
	uniform vec3 light_pos;
	uniform float attenuation_parameter;
	uniform vec4 light_x;
	uniform vec2 spot_cos;
#endif	
#endif //INSTANCING


uniform sampler2D texture_unit0;
uniform sampler2D texture_unit1;
uniform highp sampler2D texture_unit3;
uniform sampler2D texture_unit4;

uniform mat4 shadow_matrix0;
uniform sampler2DShadow shadow_unit0;

in vec4 out_view_dir;
in vec4 out_pos;
out vec4 frag_color;

float DecodeFloatRG( highp vec2 value, float max_value) 
{
	const highp float max16int = 256.0 * 256.0 - 1.0;
	highp float result = 255.0 * dot(value, vec2(256.0, 1.0)) / max16int;
	
	result *= max_value;

	return result;
} 


vec3 get_world_pos( vec2 out_texcoord) 
{
	float d = texture( texture_unit3, out_texcoord).x;

	d = depth_parameters.y / (d - depth_parameters.x);

#if defined USE_UBOs || defined INSTANCING
	return d * out_view_dir.xyz / out_view_dir.w + view_posXYZ_normalized_time.xyz;
#else
	return d * out_view_dir.xyz / out_view_dir.w + view_pos;
#endif
}

void main()
{
#ifdef INSTANCING
	vec3 light_color = lights[instanceID].light_color.xyz;
	vec3 light_pos = lights[instanceID].light_pos.xyz;
	float attenuation_parameter = lights[instanceID].attenuation_parameter.x;
	vec4 light_x = lights[instanceID].light_x;
	vec2 spot_cos = lights[instanceID].spot_cos.xy;
	vec3 view_pos = view_posXYZ_normalized_time.xyz;
#endif

	vec2 out_texcoord = (out_pos.xy / out_pos.w * 0.5) + vec2(0.5,0.5);

	float shadow = 1.0;
		
	vec3 view_dir = normalize( out_view_dir.xyz);

	vec4 texel_color = texture( texture_unit0, out_texcoord);

	vec4 normal = texture( texture_unit1, out_texcoord);
	normal.xyz = normal.xyz * 2.0 - 1.0;
	normal.xyz = normalize( normal.xyz);

	vec4 float_params = texture( texture_unit4, out_texcoord);
#if defined POINT_LIGHT
	vec3 position = get_world_pos( out_texcoord);

	#ifdef USE_UBOs
		vec3 light_dir = light_posXYZ_pad.xyz - position;
	#else
		vec3 light_dir = light_pos - position;
	#endif
	float sq_distance = dot( light_dir, light_dir);

	#ifdef USE_UBOs
		float atten = spotcosXY_attenZ_pad.z * sq_distance + 1.0;
	#else
		float atten = attenuation_parameter * sq_distance + 1.0;
	#endif
	
	if( atten <= 0.0)
	{
		discard;
	}

	atten = clamp( atten, 0.0, 1.0);
	
	light_dir = light_dir / sqrt( sq_distance);

#if defined SHADOW_MAP

	vec4 shadow_texcoord = shadow_matrix0 * vec4( position, 1.0);
	
	shadow = textureProj( shadow_unit0, shadow_texcoord);

	if( shadow == 0.0)
	{
		discard;
	}

	atten *= shadow;

#endif

#elif defined SPOT_LIGHT
	vec3 position = get_world_pos( out_texcoord);

	#ifdef USE_UBOs
		vec3 light_dir = light_posXYZ_pad.xyz - position;
	#else
		vec3 light_dir = light_pos - position;
	#endif
	float sq_distance = dot( light_dir, light_dir);

	#ifdef USE_UBOs
		float atten = spotcosXY_attenZ_pad.z * sq_distance + 1.0;
	#else
		float atten = attenuation_parameter * sq_distance + 1.0;
	#endif
		
	if( atten <= 0.0)
	{
		discard;
	}

	atten = clamp( atten, 0.0, 1.0);
	
	light_dir = light_dir / sqrt( sq_distance);

	float fov_atten = dot( light_dir, -light_x.xyz);
	
	#ifdef USE_UBOs
		fov_atten = fov_atten - spotcosXY_attenZ_pad.x;
	#else
		fov_atten = fov_atten - spot_cos.x;
	#endif
	
	if( fov_atten <= 0.0)
	{
		discard;
	}

	//fov_atten /= (1.0 - 0.9);
	#ifdef USE_UBOs
		fov_atten *= spotcosXY_attenZ_pad.y;
	#else
		fov_atten *= spot_cos.y;
	#endif

	atten *= fov_atten;

#if defined SHADOW_MAP

	vec4 shadow_texcoord = shadow_matrix0 * vec4( position, 1.0);
	
	shadow = textureProj( shadow_unit0, shadow_texcoord);

	if( shadow == 0.0)
	{
		discard;
	}
	
	atten *= shadow;

#endif


#else
	vec3 light_dir = light_x.xyz;
	const float atten = 1.0;
#endif

	vec3 half_vector = normalize( light_dir - view_dir);

	float diffuse = dot( normal.xyz, light_dir);	

#ifdef SPECIAL_DIFFUSE_CLAMP
	diffuse = diffuse * 0.5 + 0.5;
#else
	diffuse = clamp( diffuse, 0.0, 1.0);
#endif

	float specular = dot( normal.xyz, half_vector);
	specular = clamp( specular, 0.0, 1.0);

	float A = DecodeFloatRG( float_params.rg, 128.0);
	float B = DecodeFloatRG( float_params.ba, 4096.0);

	specular = A * pow( specular, B);
	
#ifdef SV_30
	diffuse *= 3.0;
	specular *= 1.0;
#endif

#ifdef SV_31
	diffuse *= 3.0;
	specular *= 1.0;
#endif


#ifdef HALF_SIZED_LIGHTING
	#ifdef USE_UBOs
		frag_color = vec4( diffuse * light_colorXYZ_pad.xyz * atten, diffuse * specular * atten);		
	#else
		frag_color = vec4( diffuse * light_color * atten, diffuse * specular * atten);		
	#endif
#else			
	#ifdef USE_UBOs
		vec3 c = (texel_color.xyz * diffuse + specular) * light_colorXYZ_pad.xyz * atten;
	#else                                   
		vec3 c = (texel_color.xyz * diffuse + specular) * light_color * atten;
	#endif
	frag_color = vec4( c, 1.0);	
#endif

#ifdef SV_31
	frag_color.xyz *= 0.25; //target is GL_RGB10_A2, which has 4x precision compared to GL_RGBA8, so convert it to range, compress it
#endif
}
