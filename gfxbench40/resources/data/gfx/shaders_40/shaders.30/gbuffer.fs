/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
uniform sampler2D texture_unit0;
uniform sampler2D texture_unit3;
uniform sampler2D texture_unit2;
uniform sampler2D texture_unit1;
uniform sampler2D texture_unit4;
uniform samplerCube envmap0;
uniform samplerCube envmap1;
uniform sampler2DArray texture_array_unit0;

#ifdef USE_UBOs
	#include cameraConsts;
	#include matConsts;
	#include envmapsInterpolatorConsts;
#else
	uniform float specular_intensity;
	uniform float specular_exponent;
	uniform vec3 fresnel_params;
	
	uniform float time;
	uniform vec3 view_dir;
	uniform float envmaps_interpolator;
#endif

in vec2 out_texcoord0;
in vec3 out_normal;
in vec3 out_tangent;
in vec3 out_view_dir;

in vec3 out_eye_space_normal;
in vec3 out_world_pos;

out vec4 frag_color[4];


vec2 EncodeFloatRG( highp float value, float max_value) 
{
	const highp float max16int = 256.0 * 256.0 - 1.0;

	value = clamp( value / max_value, 0.0, 1.0);

	value *= max16int;
	highp vec2 result = floor(value / vec2(256.0, 1.0));
	result.g -= result.r * 256.0;
	result /= 255.0;
	
	return result;
} 

void main()
{
#ifdef MASK
	vec4 mask = texture( texture_unit2, out_texcoord0);
#if defined ALPHA_TEST
	if( mask.x < 0.25)
	{
		discard;
	}
#endif
#else
	vec4 mask = vec4(0.0, 0.0, 0.0, 1.0);
#if defined ALPHA_TEST
	// The mask is always (0, 0, 0, 1) if no mask is defined, so (mask.x < 0.25) would always be hit.
	discard;
#endif
#endif

#ifdef EMISSION
	vec4 emission = texture( texture_unit4, out_texcoord0);
#endif
	vec3 normal = normalize( out_normal);
	vec3 vtx_normal = normal;
	vec3 tangent = normalize( out_tangent);
	vec3 bitangent = cross( tangent, normal);

	vec3 texel_color = texture( texture_unit0, out_texcoord0).xyz;
	vec3 ts_normal = texture( texture_unit3, out_texcoord0).xyz;

	ts_normal.xyz = ts_normal.xyz * 2.0 - 1.0;
		
	mat3 mat = mat3( tangent, bitangent, normal); 

	normal = mat * ts_normal;
	
	vec3 reflect_vector = reflect( out_view_dir, normal);
	vec3 env_color0 = texture( envmap0, reflect_vector).xyz;
	vec3 env_color1 = texture( envmap1, reflect_vector).xyz;

#ifdef USE_UBOs
	vec3 env_color = mix( env_color1.xyz, env_color0.xyz, envmaps_interpolator_pad3.x);
#else
	vec3 env_color = mix( env_color1.xyz, env_color0.xyz, envmaps_interpolator);
#endif

	vec3 normal_enc = normal * 0.5 + 0.5;
	
	frag_color[0] = vec4( texel_color, 0.0);
	frag_color[1] = vec4( normal_enc, 0.0);
	frag_color[2] = vec4( env_color, mask.z);
	#ifdef USE_UBOs
		frag_color[3].xy = EncodeFloatRG( mask.y * matparams_disiseri.y, 128.0); 
	#else
		frag_color[3].xy = EncodeFloatRG( mask.y * specular_intensity, 128.0); 
	#endif
	#ifdef USE_UBOs
		frag_color[3].zw = EncodeFloatRG( matparams_disiseri.z, 4096.0); 
	#else
		frag_color[3].zw = EncodeFloatRG( specular_exponent, 4096.0); 
	#endif

#ifdef EMISSION
	frag_color[1].w = emission.x * 8.0;
#endif

#ifdef TRANSITION_EFFECT
	float t = 0.0;

	#ifdef USE_UBOs
		if( view_posXYZ_normalized_time.w > 0.37)
		{
			t = (view_posXYZ_normalized_time.w - 0.37) * 9.0;
		}

		t = clamp( t, 0.0, 1.0);
	#else
		if( time > 0.37)
		{
			t = (time - 0.37) * 9.0;
		}

		t = clamp( t, 0.0, 1.0);
	#endif

	vec3 nnn = normalize( out_eye_space_normal);
	
	float idx = fract( t * 10.0) * 95.0;
	
	vec3 normal2 = normal * normal;
	vec3 secondary_color0 = texture( texture_array_unit0, vec3( out_world_pos.yz, idx)).xyz;
	vec3 secondary_color1 = texture( texture_array_unit0, vec3( out_world_pos.xz, idx)).xyz;
	vec3 secondary_color2 = texture( texture_array_unit0, vec3( out_world_pos.xy, idx)).xyz;
	vec3 secondary_color = secondary_color0 * normal2.x + secondary_color1 * normal2.y + secondary_color2 * normal2.z;

	float a = 2.5 * t + secondary_color.x;
	
	secondary_color *= vec3( 1.0 - nnn.z);
	secondary_color = 64.0 * pow( secondary_color, vec3( 2.0));

	frag_color[0].xyz = mix( texel_color, secondary_color * vec3( 0.2, 0.7, 1.0), (1.0 - pow( t, 8.0)));
	
	if(a < 1.0)
	{
		discard;
	}
	
	
#ifdef EMISSION
	frag_color[1].w = mix( secondary_color.x * 128.0, emission.x, t);
#endif

#endif

#ifdef FRESNEL
	#ifdef USE_UBOs
		float val = pow(1.0-dot(vtx_normal, -view_dirXYZ_pad.xyz), fresnelXYZ_transp.x);
	#else
		float val = pow(1.0-dot(vtx_normal, -view_dir), fresnel_params.x);
	#endif
	
	float texLum = dot(frag_color[0].xyz, vec3(0.3, 0.59, 0.11));
	vec3 diffColNorm = frag_color[0].xyz / (texLum + 0.001); //avoid divide by zero
	
	val *= mask.x;

	#ifdef USE_UBOs
		frag_color[0].xyz = mix(frag_color[0].xyz, diffColNorm, fresnelXYZ_transp.z * clamp(val * 1.0,0.0,1.0));
		frag_color[1].w = val * fresnelXYZ_transp.y; //emissive
	#else
		frag_color[0].xyz = mix(frag_color[0].xyz, diffColNorm, fresnel_params.z * clamp(val * 1.0,0.0,1.0));
		frag_color[1].w = val * fresnel_params.y; //emissive
	#endif
#endif
}
