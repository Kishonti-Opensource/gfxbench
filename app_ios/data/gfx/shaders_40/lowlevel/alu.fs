/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifdef GL_ES
precision mediump float;
#else
#define mediump
#endif

uniform float u_time;
uniform vec2 u_aspectRatio;
uniform vec3 u_lightDir;
uniform vec3 u_eyePosition;
uniform mat4 u_orientation;

varying vec2 v_texCoord;

float rand2D(vec2 co)
{
	//return fract(sin(dot(co, vec2(12.9898, 78.233))) * 43758.5453);
    return fract(sin(dot(co, vec2(2.9898, 8.233))) * 8.5453);
}

// Returns a pseudo-random number
float rand2D_C1(vec2 co)
{
	vec2 flr = floor(co);
	vec2 frc = co - flr;
	float v0 = rand2D(flr);
	float v1 = rand2D(flr + vec2(1.0, 0.0));
	float mix1 = mix(v0, v1, frc.x);
	float v2 = rand2D(flr + vec2(0.0, 1.0));
	float v3 = rand2D(flr + vec2(1.0, 1.0));
	float mix2 = mix(v2, v3, frc.x);
	return mix(mix1, mix2, frc.y);
}

vec3 getSkyColor(vec3 direction)
{
	float cosDir = dot(direction, u_lightDir) * 0.5 + 0.5;
	vec3 horizon = pow(cosDir, 0.3) * mix(vec3(1.0, 0.75, 0.2), vec3(0.5, 0.9, 1.0), u_lightDir.y);
	vec3 zenit = mix(vec3(0.0, 0.2, 0.4), vec3(0.5, 0.9, 1.0), u_lightDir.y);
	vec3 sky = mix(horizon, zenit, direction.y);
	vec3 direct = vec3(1.0, 1.0, 1.0) * (0.1 * cosDir  + 0.5 * pow(cosDir, 16.0) + step(0.999, cosDir));
	vec3 light = sky + direct;
	return light;
}

vec3 getWaterNormal(vec2 coords, float distance)
{
	float t = u_time * 0.002;
	float r1 = rand2D_C1(coords * vec2(0.9, 1.8)) * 6.28318;
	float r2 = rand2D_C1(coords * vec2(2.0, 4.0)) * 6.28318;
	float nx = cos(r1 + t) + 0.5 * cos(r2 + t * 2.0);
	float ny = sin(r1 + t) + 0.5 * sin(r2 + t * 2.0) + 0.5 * sin(coords.y + cos(coords.x) - t);
	return normalize(vec3(nx, 0.01 + 0.1 * pow(distance, 1.5), ny));
}

vec3 getGroundColor(vec2 coords)
{
	vec2 c = step(0.5, fract(coords));
	return mix(vec3(0.2, 0.8, 1.0), vec3(0.1, 0.4, 0.7), abs(c.x - c.y));
}

void main()
{
	float fov = 1.5;
	float thf = tan(fov * 0.5);
	vec3 viewDirection = normalize(vec3(v_texCoord.x * u_aspectRatio.x * thf, v_texCoord.y * u_aspectRatio.y * thf, 1.0));
	viewDirection = (vec4(viewDirection, 1.0) * u_orientation).xyz;
	
	float targetHeight = (viewDirection.y >= 0.0) ? 5.0 : 0.0;
	vec3 eyeToPlane = viewDirection * ((u_eyePosition.y - targetHeight) / viewDirection.y);
	vec3 intersectPosition = u_eyePosition + eyeToPlane;
	vec3 planeNormal = getWaterNormal(intersectPosition.xz, length(eyeToPlane));
	
	if (viewDirection.y >= 0.0)
	{
		vec3 skyColor = getSkyColor(viewDirection);
		float cloud = max(dot(planeNormal, viewDirection), 0.3 - 0.3 * planeNormal.y);
		gl_FragColor = vec4(mix(skyColor, vec3(1.0, 1.0, 1.0), cloud), 1.0);
	}
	else
	{
		vec3 refractedViewDir = viewDirection - 0.25 * planeNormal;
		vec3 waterToGround = refractedViewDir * (1.0 / refractedViewDir.y);
		vec3 groundPosition = intersectPosition + waterToGround;
		vec3 refractedLightDir = normalize(u_lightDir + planeNormal);
		vec3 groundColor = getGroundColor(groundPosition.xz);
		groundColor *= refractedLightDir.y;	// dot(refractedLightDir, (0, 1, 0)
		groundColor = mix(vec3(0.0, 0.2, 0.4), groundColor, abs(dot(planeNormal, viewDirection)));
		
		vec3 reflectedView = max(vec3(-1.0, 0.0, -1.0), reflect(viewDirection, planeNormal));
		vec3 reflectedSky = 0.25 * getSkyColor(reflectedView);
		
		gl_FragColor = vec4(groundColor + reflectedSky, 1.0);
	}
}
