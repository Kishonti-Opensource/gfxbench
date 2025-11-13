/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "../include/cbuffer.hlsl"
#include "../include/functions.hlsl"
#include "../include/types.hlsl"

Texture2D texture_unit0 : register(t0);
Texture2D texture_unit2 : register(t2);
Texture2D texture_unit3 : register(t3);
Texture2D texture_unit4 : register(t4);
TextureCube envmap0 : register(t8);
TextureCube envmap1 : register(t9);
Texture2DArray textureArray : register(t15);

SamplerState SampleType;

struct GBufferOutputStruct
{
	float4 color0 : SV_TARGET0;
	float4 color1 : SV_TARGET1;
	float4 color2 : SV_TARGET2;
	float4 color3 : SV_TARGET3;
};

GBufferOutputStruct main(GBufferStruct input)
{
#ifdef MASK
	float4 mask = texture_unit2.Sample(SampleType, input.texcoord0);
#ifdef ALPHA_TEST
	if (mask.x < 0.25)
	{
		discard;
	}
#endif
#elif defined ALPHA_TEST
	// The mask is always (0, 0, 0, 1) if no mask is defined, so (mask.x < 0.25) would always be hit.
	discard;
#else
	float4 mask = float4(0, 0, 0, 1);
#endif

#ifdef EMISSION
	float4 emission = texture_unit4.Sample(SampleType, input.texcoord0);
#endif

	float3 normal = normalize(input.normal);
	float3 vtx_normal = normal;
	float3 tangent = normalize(input.tangent);
	float3 bitangent = cross(tangent, normal);

	float3 texel_color = texture_unit0.Sample(SampleType, input.texcoord0).xyz;
	float3 ts_normal = texture_unit3.Sample(SampleType, input.texcoord0).xyz;

	GBufferOutputStruct output;

	ts_normal.xyz = ts_normal.xyz * 2.0 - 1.0;
	float3x3 mat = float3x3(tangent, bitangent, normal); 
	normal = mul(ts_normal, mat);
	
	float3 reflect_vector = reflect(input.viewDir, normal);
	float3 env_color0 = envmap0.Sample(SampleType, reflect_vector).xyz;
	float3 env_color1 = envmap1.Sample(SampleType, reflect_vector).xyz;
	float3 env_color = lerp(env_color1.xyz, env_color0.xyz, envmaps_interpolator);

	float3 out_normal = normal * 0.5 + 0.5;
	output.color0 = float4(texel_color, 0);
	output.color1 = float4(out_normal, 0.0);
	output.color2 = float4(env_color, mask.z);
	output.color3.xy = floatToVector2(clamp(mask.y * specular_intensity / 128, 0, 1)); 
	output.color3.zw = floatToVector2(clamp(specular_exponent / 4096, 0, 1));

#ifdef EMISSION
	output.color1.w = emission.x * 8.0;
#endif

#ifdef TRANSITION_EFFECT
	float t = clamp((time - 0.37) * 9.0, 0.0, 1.0);
	float3 nnn = normalize(input.eyeSpaceNormal);
	float idx = frac(t * 10) * 95.0;
	float3 normal2 = normal * normal;

	float3 secondary_color0 = textureArray.Sample(SampleType, float3(input.worldPos.yz, idx)).xyz;
	float3 secondary_color1 = textureArray.Sample(SampleType, float3(input.worldPos.xz, idx)).xyz;
	float3 secondary_color2 = textureArray.Sample(SampleType, float3(input.worldPos.xy, idx)).xyz;
	float3 secondary_color = secondary_color0 * normal2.x + secondary_color1 * normal2.y + secondary_color2 * normal2.z;

	float a = 2.5 * t + secondary_color.x;
	secondary_color *= 1.0 - nnn.z;
	secondary_color = 64.0 * pow(secondary_color, 2.0);
	
	output.color0.xyz = lerp(texel_color, secondary_color * float3(0.2, 0.7, 1.0), 1.0 - pow(t, 8.0));
	
	if(a < 1.0f)
	{
		discard;
	}

#ifdef EMISSION
	output.color1.w = lerp(secondary_color.x * 128.0, emission.x, t);
#endif
#endif

#ifdef FRESNEL
	float dotValue = dot(vtx_normal, -view_dir.xyz);
	float val = pow(abs(1.0 - dotValue), fresnel_params.x);
	
	float texLum = dot(output.color0.xyz, float3(0.3f, 0.59f, 0.11f));
	float3 diffColNorm = output.color0.xyz / (texLum + 0.001f); //avoid divide by zero
	
	val *= mask.x;
	
	output.color0.xyz = lerp(output.color0.xyz, diffColNorm, fresnel_params.z * clamp(val, 0.0, 1.0));
	output.color1.w = val * fresnel_params.y; //emissive
#endif

	return output;
}
