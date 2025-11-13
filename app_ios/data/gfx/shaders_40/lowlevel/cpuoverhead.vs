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

attribute vec3 myVertex;
attribute vec2 myTexCoord;

uniform float u_rotation;
uniform vec2 u_position;
uniform vec2 u_scale;
uniform vec2 u_matrixSize;
uniform vec4 u_color0;
uniform vec4 u_color1;
uniform vec4 u_color2;
uniform vec4 u_color3;
uniform vec2 u_screenResolution;

varying vec4 v_color;

mat2 rotateZ(float alpha)
{
	return mat2(
	cos(alpha), -sin(alpha),
	sin(alpha), cos(alpha));
}

void main()
{
	vec4 color01 = mix(u_color0, u_color1, myTexCoord.x);
	vec4 color23 = mix(u_color2, u_color3, myTexCoord.x);
	v_color = mix(color01, color23, myTexCoord.y);
	mat2 rot = rotateZ(u_rotation);
	vec2 localCoord = myVertex.xy * u_scale * rot * 10.0 / u_screenResolution;
	vec2 offset = 2.0 * (u_position / u_matrixSize) - vec2(1.0, 1.0);
	vec2 scrCoord = localCoord + offset;
	gl_Position = vec4(scrCoord, myVertex.z, 1.0);
}
