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

uniform sampler2D texture0;
uniform sampler2D texture1;
varying vec2 v_texCoord0;
varying vec2 v_texCoord1;
varying vec2 v_texCoord2;
varying vec2 v_texCoord3;

void main()
{
	vec4 color = texture2D(texture0, v_texCoord0);
	vec4 light1= texture2D(texture1, v_texCoord1);
	vec4 light2= texture2D(texture1, v_texCoord2);
	vec4 light3= texture2D(texture1, v_texCoord3);
	vec4 light = light1 * 0.5 + light2 * 0.3 + light3 * 0.2;
	gl_FragColor = vec4(color.xyz * light.xyz, 1.0);
}
