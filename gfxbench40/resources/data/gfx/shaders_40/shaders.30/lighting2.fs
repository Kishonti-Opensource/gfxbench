/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#define HALF_SIZED_LIGHTING

uniform sampler2D texture_unit0;
uniform sampler2D texture_unit1;
uniform sampler2D texture_unit3;
uniform sampler2D texture_unit4;

uniform vec3 light_colors[32];
uniform vec3 light_positions[32];
uniform float attenuation_parameters[32];
uniform vec4 light_xs[32];
uniform vec4 light_y;
uniform vec4 light_z;
uniform vec2 spot_coss[32];
uniform vec3 view_pos;
uniform int num_lights;

uniform vec4 depth_parameters;
uniform vec2 inv_resolution;
uniform mat4 shadow_matrix0;
uniform sampler2DShadow shadow_unit0;

in vec4 out_view_dir;
out vec4 frag_color;

float DecodeFloatRG( vec2 value, float max_value) 
{
	const float max16int = 256.0 * 256.0 - 1.0;
	float result = 255.0 * dot(value, vec2(256.0, 1.0)) / max16int;
	
	result *= max_value;

	return result;
} 


vec3 get_world_pos( vec2 out_texcoord) 
{
	float d = texture( texture_unit3, out_texcoord).x;

	d = depth_parameters.y / (d - depth_parameters.x);

	return d * out_view_dir.xyz / out_view_dir.w + view_pos;
}

void main()
{
	float shadow = 1.0;
#ifdef HALF_SIZED_LIGHTING
	vec2 out_texcoord = vec2( gl_FragCoord.xy) * inv_resolution*2.0;
#else
	vec2 out_texcoord = vec2( gl_FragCoord.xy) * inv_resolution;
#endif
	vec3 view_dir = normalize( out_view_dir.xyz);

	vec4 texel_color = texture( texture_unit0, out_texcoord);

	vec4 normal = texture( texture_unit1, out_texcoord);
	normal.xyz = normal.xyz * 2.0 - 1.0;
	normal.xyz = normalize( normal.xyz);

	vec4 float_params = texture( texture_unit4, out_texcoord);

	float A = DecodeFloatRG( float_params.rg, 128.0);
	float B = DecodeFloatRG( float_params.ba, 4096.0);

	vec3 position = get_world_pos( out_texcoord);

	frag_color = vec4( 0.0);	

	#if 1
	for( int i=0; i<num_lights; i++)
	{
		vec3 light_dir = light_positions[i] - position;
		float sq_distance = dot( light_dir, light_dir);
		
		float atten = attenuation_parameters[i] * sq_distance + 1.0;
		
		atten = clamp( atten, 0.0, 1.0);
	
		light_dir = light_dir / sqrt( sq_distance);

		float fov_atten = dot( light_dir, -light_xs[i].xyz);
	
		fov_atten = fov_atten - spot_coss[i].x;
	
		fov_atten = clamp( fov_atten, 0.0, 1.0);

		//fov_atten /= (1.0 - 0.9);
		fov_atten *= spot_coss[i].y;

		atten *= fov_atten;

#if defined SHADOW_MAP
		vec4 shadow_texcoord = shadow_matrix0 * vec4( position, 1.0);
		shadow = textureProj( shadow_unit0, shadow_texcoord);

		atten *= shadow;
#endif

		vec3 half_vector = normalize( light_dir - view_dir);

		float diffuse = dot( normal.xyz, light_dir);	

		diffuse = clamp( diffuse, 0.0, 1.0);

		float specular = dot( normal.xyz, half_vector);
		specular = clamp( specular, 0.0, 1.0);
		specular = A * pow( specular, B);

#ifdef HALF_SIZED_LIGHTING
		frag_color += vec4( diffuse * light_colors[i] * atten, diffuse * specular * atten);	
#else		
		vec3 c = (texel_color.xyz * diffuse + specular) * light_colors[i] * atten;




		//c = vec3( 0.01) * light_colors[i];
		frag_color += vec4( c, 1.0);	
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
}
