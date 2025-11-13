/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
Texture2D texture0 : register(t0);
Texture2D texture1 : register(t1);
Texture2D texture2 : register(t2);
Texture2D texture3 : register(t3);
Texture2D texture10 : register(t10);
Texture2D texture11 : register(t11);

TextureCube envmap0 : register(t8);
TextureCube envmap1 : register(t9);

SamplerState SampleType;
SamplerState SampleTypeClamp;

#include "../include/cbuffer.hlsl"

#include "../include/types.hlsl"

#include "../include/functions.hlsl"



#if defined RELIEF

void ray_intersect_rm_linear( inout float3 p, inout float3 v)
{
	const int linear_search_steps =	30;

	v /= float(linear_search_steps);

	for( int i=0; i<linear_search_steps-1; i++)
	{
		float4 t = texture2.Sample( SampleType, p.xy);
		
		if (p.z < t.x )
		{
			p += v;
		}
	}
}

void ray_intersect_rm_binary( inout float3 p, inout float3 v)
{
	const int binary_search_steps=4;
   
	for( int i=0; i<binary_search_steps; i++)
	{
		v *= 0.5;
		float4 t = texture2.Sample( SampleType, p.xy);

		if (p.z < t.x )
		{
			p += 2.0 * v;
		}
		p -= v;
	}
}


float2 relief(in float3 normal, in float3 tangent, float2 uv, float3 vd)
{
	float3 v;

	float3 t = normalize2( tangent );
	float3 n = normalize2( normal );
	float3 b = cross( n, t );

	t = -t;

	v.x = dot( vd, float3( t.x, t.y, t.z));
	v.y = dot( vd, float3( b.x, b.y, b.z));
	v.z = dot( vd, float3( n.x, n.y, n.z));
	
	float3 scale = float3( 1.0, 1.0, 0.025);
	v *= scale.z / ( scale * v.z);
	
	float3 p = float3( uv, 0.0);
	
	ray_intersect_rm_linear( p, v);
	ray_intersect_rm_binary( p, v);
		
	return p.xy;
}


float2 parallax(in float3 normal, in float3 tangent, float2 uv, float3 vd)
{
	float3 v;

	float3 t = normalize2( tangent );
	float3 n = normalize2( normal );
	float3 b = cross( n, t );

	t = -t;

	v.x = dot( vd, float3( t.x, t.y, t.z));
	v.y = dot( vd, float3( b.x, b.y, b.z));
	v.z = dot( vd, float3( n.x, n.y, n.z));

	float h = texture2.Sample(SampleType, uv).x;
	
	//h -= 0.5;
	h *= 0.05;
	
	return uv + v.xy * h;
}

#endif

#ifdef LIGHTMAP
float3 lightmapping(in float2 texcoord1)
{
	return (float3)texture1.Sample(SampleType, texcoord1)*1.333;
}
#endif

#if defined LIGHTING || defined REFLECTION

	#ifdef ANIMATE_NORMAL

float3 calculate_normal(in float3 normal, in float3 tangent, in float2 texcoord0, in float2 texcoord1, in float2 texcoord2, in float2 texcoord3)
{
		float4 ts_normal;
		float4x4 nmatrix = float4x4(
				texture3.Sample(SampleType, texcoord0),
				texture3.Sample(SampleType, texcoord1),
				texture3.Sample(SampleType, texcoord2),
				texture3.Sample(SampleType, texcoord3)
			);
		
		ts_normal = mul(transpose(nmatrix), float4(0.25, 0.25, 0.25, 0.25));
	#else
float3 calculate_normal(in float3 normal, in float3 tangent, in float2 texcoord0)
{
		float4 ts_normal;
		ts_normal = texture3.Sample(SampleType, texcoord0);
	#endif

	float3 ts_normal_xyz = (float3)ts_normal;
	ts_normal_xyz = (ts_normal_xyz * 2.0) - 1.0;

	ts_normal_xyz = normalize2( ts_normal_xyz);

	float3 t = normalize2( tangent );
	float3 n = normalize2( normal );
	float3 b = cross( n, t );
	float3x3 m = float3x3( t, b, n );

	float3 result = mul( ts_normal_xyz, m );

	return result;
}
#endif


#ifdef SHADOW_MAP

float DecodeFloatRGB( float3 rgba ) 
{
	return dot( rgba, float3(1.0, 1.0/255.0, 1.0/65025.0) );
}

float shadow( float2 tc1, float2 tc2, float tc1z, float tc2z)
{
#if defined RGB_ENCODED
	float color0 = DecodeFloatRGB( texture10.Sample( SampleTypeClamp, tc1 ).xyz);
	float color1 = DecodeFloatRGB( texture11.Sample( SampleTypeClamp, tc2 ).xyz);
#else
	float color0 = 0.0;
	float color1 = 0.0;
#endif
	
    if( tc1z < 1.0 )
        if( tc1z > 0.0)
            if( color0 < tc1z )
				return 0.33;
    
    if( tc2z < 1.0 )
        if( tc2z > 0.0)
            if( color1 < tc2z )
				return 0.33;
    
    return 1.0;

}
#endif

#ifdef LIGHTING
float3 lighting( float2 tc1, float2 tc2, float tc1z, float tc2z, float3 color, float3 normal, float3 view_vector, float3 light_dir, float3 light_color, float specular_mask)
{
	float3 half_vector = normalize2( view_vector + light_dir);

	float ambient = 0.1;

	#ifdef WIDE_DIFFUSE_CLAMP
		float diffuse = (dot( normal, light_dir) * 0.5 + 0.5) * diffuse_intensity;
	#else
		float diffuse = clamp( dot( normal, light_dir), 0.0, 1.0) * diffuse_intensity;
	#endif

	float specular = clamp( dot( normal, half_vector), 0.0, 1.0);
	specular = specular_intensity * pow( specular, specular_exponent) * specular_mask;

	#if defined SHADOW_MAP
		float s = shadow( tc1, tc2, tc1z, tc2z);
	
		diffuse = diffuse * s;

		specular = specular * s;
	#endif

	float3 result = (color * (ambient + diffuse) + specular) * light_color;
	return result;
}

#endif

#if defined REFLECTION
float3 reflection( float3 color, float3 normal, float3 view_vector, float reflection_mask)
{
	float3 reflect_vector = reflect( -view_vector, normal);
	
	float4 reflection0 = envmap0.Sample(SampleType, reflect_vector);  
	float4 reflection1 = envmap1.Sample(SampleType, reflect_vector);  
	
	float3 reflection_color = lerp( (float3)reflection1, (float3)reflection0, envmaps_interpolator);
	
	float3 result = lerp( color, reflection_color, reflect_intensity * reflection_mask);
	
	return result;
}
#endif


#ifndef ZPREPASS
float4 main(VertexShaderOutput input) : SV_TARGET
{
	float2 tc = input.texcoord0;
		
#if defined RELIEF
	{		
		float3 view_dir = normalize2( input.view_dir);
		tc = parallax(input.normal, input.tangent, input.texcoord0, view_dir);
	}
#endif


#ifdef MASK
	#ifdef ANIMATE_NORMAL
		float4x4 masks = float4x4(
				texture2.Sample(SampleType, input.animatecoord0),
				texture2.Sample(SampleType, input.animatecoord1),
				texture2.Sample(SampleType, input.animatecoord2),
				texture2.Sample(SampleType, input.animatecoord3)
			);
		
		float4 mask  = mul(transpose(masks), float4(0.25, 0.25, 0.25, 0.25));
	#else
		float4 mask  = texture2.Sample(SampleType, tc);
	#endif

#else
	const float4 mask = float4( 1.0f, 1.0f, 1.0f, 1.0f);
#endif

#ifdef ALPHA_TEST
	if ( mask.x < 0.5f ) 
		discard;
#endif
	
	float3 color  = (float3)texture0.Sample(SampleType, tc);

#if defined LIGHTING || defined REFLECTION
	float3 view_dir = normalize2( input.view_dir);
	#ifdef ANIMATE_NORMAL
		float3 normal = calculate_normal(input.normal, input.tangent, input.animatecoord0, input.animatecoord1, input.animatecoord2, input.animatecoord3 );
	#else
		float3 normal = calculate_normal(input.normal, input.tangent, tc);
	#endif
#endif

#ifdef LIGHTING
	//screen_position = shadow tex coord
	color = lighting( input.coord0, input.coord1, input.coord0z, input.coord1z, color, normal, view_dir, (float3)global_light_dir, (float3)global_light_color, mask.y);	
#endif

#ifdef LIGHTMAP
	color *= lightmapping(input.texcoord1);
#endif

#if defined REFLECTION
	color = reflection( color, normal, view_dir, mask.z);
#endif

#if defined FOG
	color = lerp( color, (float3)background_color, input.fog_distance * input.fog_distance);
#endif	
		
#if defined TRANSPARENCY
	float trp = mask.x * transparency;
	return float4( color, trp);
#else
	return float4( color, mblur_mask);
#endif
}
#else

float4 main(VertexShaderOutput input) : SV_TARGET
{
	float2 tc = input.texcoord0;
		
#if defined RELIEF
	{		
		float3 view_dir = normalize2( input.view_dir);
		tc = parallax(input.normal, input.tangent, input.texcoord0, view_dir);
	}
#endif


#ifdef MASK
	#ifdef ANIMATE_NORMAL
		float4x4 masks = float4x4(
				texture2.Sample(SampleType, input.animatecoord0),
				texture2.Sample(SampleType, input.animatecoord1),
				texture2.Sample(SampleType, input.animatecoord2),
				texture2.Sample(SampleType, input.animatecoord3)
			);
		
		float4 mask  = mul(transpose(masks), float4(0.25, 0.25, 0.25, 0.25));
	#else
		float4 mask  = texture2.Sample(SampleType, tc);
	#endif

#else
	const float4 mask = float4( 1.0f, 1.0f, 1.0f, 1.0f);
#endif

#ifdef ALPHA_TEST
	if ( mask.x < 0.5f ) 
		discard;
#endif

	return float4(1.0f, 1.0f, 1.0f, 1.0f);
}
#endif