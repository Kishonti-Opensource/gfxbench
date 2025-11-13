/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifdef GL_ES
precision highp float;
#else
#define highp
#endif

uniform vec2 u_scale;
uniform vec2 u_offset;
attribute vec3 myVertex;
attribute vec2 myTexCoord;
varying vec2 v_texCoord;

void main()
{
	v_texCoord = myTexCoord;
	vec2 scrCoords = myVertex.xy * u_scale + u_offset;
	gl_Position = vec4(scrCoords, myVertex.z, 1.0);
}
