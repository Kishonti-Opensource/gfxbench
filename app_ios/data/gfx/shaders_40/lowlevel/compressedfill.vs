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

uniform float u_rotation;
uniform float u_aspectRatio;
attribute vec3 myVertex;
attribute vec2 myTexCoord;
varying vec2 v_texCoord0;
varying vec2 v_texCoord1;
varying vec2 v_texCoord2;
varying vec2 v_texCoord3;

mat2 rotateZ(float alpha)
{
	return mat2(
	cos(alpha), -sin(alpha),
	sin(alpha), cos(alpha));
}

void main()
{
	mat2 rot0 = rotateZ(u_rotation);
	v_texCoord0 = (myTexCoord - vec2(0.5, 0.5)) * vec2(u_aspectRatio * 2.0, 2.0) * rot0 + vec2(0.5, 0.5);
	mat2 rot1 = rotateZ(-u_rotation);
	v_texCoord1 = myTexCoord * vec2(u_aspectRatio, 1.0) * rot1;
	mat2 rot2 = rotateZ(-u_rotation * 0.5);
	v_texCoord2 = myTexCoord * vec2(u_aspectRatio, 1.0) * rot2;
	mat2 rot3 = rotateZ(u_rotation * 0.5);
	v_texCoord3 = myTexCoord * vec2(u_aspectRatio, 1.0) * rot3;
	gl_Position = vec4(myVertex, 1.0);
}
