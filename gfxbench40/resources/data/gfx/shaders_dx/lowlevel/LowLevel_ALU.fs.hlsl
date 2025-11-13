/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include"LowLevel_Common.hlsli"

cbuffer ConstantBufferALU : register(b0)
{
	float4x4 c_orientation;
	float3 c_eyePosition;
	float c_aspectRatio;
	float3 c_lightDir;
	float c_time;
}

float rand2D(float2 co)
{
	return frac(sin(dot(co, float2(12.9898, 78.233))) * 43758.5453);
}

// Returns a pseudo-random number
float rand2D_C1(float2 co)
{
	float2 flr = floor(co);
	float2 frc = co - flr;
	float v0 = rand2D(flr);
	float v1 = rand2D(flr + float2(1.0, 0.0));
	float mix1 = lerp(v0, v1, frc.x);
	float v2 = rand2D(flr + float2(0.0, 1.0));
	float v3 = rand2D(flr + float2(1.0, 1.0));
	float mix2 = lerp(v2, v3, frc.x);
	return lerp(mix1, mix2, frc.y);
}

float3 getSkyColor(float3 direction)
{
	float cosDir = dot(direction, c_lightDir) * 0.5 + 0.5;
	float3 horizon = pow(abs(cosDir), 0.3) * lerp(float3(1.0, 0.75, 0.2), float3(0.5, 0.9, 1.0), c_lightDir.y);
	float3 zenit = lerp(float3(0.0, 0.2, 0.4), float3(0.5, 0.9, 1.0), c_lightDir.y);
	float3 sky = lerp(horizon, zenit, direction.y);
	float3 direct = float3(1.0, 1.0, 1.0) * (0.1 * cosDir  + 0.5 * pow(abs(cosDir), 50.0) + step(0.999, cosDir));
	float3 light = sky + direct;
	return light;
}

float3 getWaterNormal(float2 coords, float distance)
{
	float t = c_time * 0.002;
	float r1 = rand2D_C1(coords * float2(0.9, 1.8)) * 6.28318;
	float r2 = rand2D_C1(coords * float2(2.0, 4.0)) * 6.28318;
	float nx = cos(r1 + t) + 0.5 * cos(r2 + t * 2.0);
	float ny = sin(r1 + t) + 0.5 * sin(r2 + t * 2.0) + 0.5 * sin(coords.y + cos(coords.x) - t);
	return normalize(float3(nx, 0.01 + 0.1 * pow(distance, 1.5), ny));
}

float3 getGroundColor(float2 coords)
{
	float2 c = step(0.5, frac(coords));
	return lerp(float3(0.2, 0.8, 1.0), float3(0.1, 0.4, 0.7), abs(c.x - c.y));
}

float4 main(PixelShaderInput input) : SV_TARGET
{
	float fov = 1.5;
	float thf = tan(fov * 0.5);
	float3 viewDirection = normalize(float3(input.texCoord0.x * c_aspectRatio * thf, input.texCoord0.y * thf, 1.0));
	viewDirection = mul(float4(viewDirection, 1.0), c_orientation).xyz;
	
	float targetHeight = (viewDirection.y >= 0.0) ? 5.0 : 0.0;
	float3 eyeToPlane = viewDirection * ((c_eyePosition.y - targetHeight) / viewDirection.y);
	float3 intersectPosition = c_eyePosition + eyeToPlane;
	float3 planeNormal = getWaterNormal(intersectPosition.xz, length(eyeToPlane));
	
	if (viewDirection.y >= 0.0)
	{
		float3 skyColor = getSkyColor(viewDirection);
		float cloud = max(dot(planeNormal, viewDirection), 0.3 - 0.3 * planeNormal.y);
		return float4(lerp(skyColor, float3(1.0, 1.0, 1.0), cloud), 1.0);
	}
	else
	{
		float3 refractedViewDir = viewDirection - 0.25 * planeNormal;
		float3 waterToGround = refractedViewDir * (1.0 / refractedViewDir.y);
		float3 groundPosition = intersectPosition + waterToGround;
		float3 refractedLightDir = normalize(c_lightDir + planeNormal);
		float3 groundColor = getGroundColor(groundPosition.xz);
		groundColor *= refractedLightDir.y;	// dot(refractedLightDir, (0, 1, 0)
		groundColor = lerp(float3(0.0, 0.2, 0.4), groundColor, abs(dot(planeNormal, viewDirection)));
		
		float3 reflectedView = max(float3(-1.0, 0.0, -1.0), reflect(viewDirection, planeNormal));
		float3 reflectedSky = 0.25 * getSkyColor(reflectedView);
		
		return float4(groundColor + reflectedSky, 1.0);
	}
}