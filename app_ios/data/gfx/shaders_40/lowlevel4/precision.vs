/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifdef GL_ES

#ifdef HIGHP
precision highp float;
#else
precision mediump float;
#endif

#endif

#if defined(HIGHP) && defined(GL_ES)
uniform highp vec4 u_quadrantScaleBias;
#else
uniform vec4 u_quadrantScaleBias;
#endif

attribute vec3 myVertex;
varying vec2 v_texCoord;

void main()
{
	gl_Position = vec4(myVertex.xyz * vec3(u_quadrantScaleBias.xy, 1.0) + vec3(u_quadrantScaleBias.zw, 0.0), 1.0);
	v_texCoord = ((myVertex.xy * u_quadrantScaleBias.xy) + u_quadrantScaleBias.zw + 1.0) * 0.5;
}

