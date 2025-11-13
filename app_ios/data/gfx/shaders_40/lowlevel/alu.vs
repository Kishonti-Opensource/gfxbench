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
varying vec2 v_texCoord;

void main()
{
	v_texCoord = myTexCoord * 2.0 - 1.0;
	gl_Position = vec4(myVertex.xyz, 1.0);
}
