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

uniform sampler2D texIn;
varying vec2 v_texCoord;

void main()
{
#ifdef ROTATE_RESULTS
	vec2 t = vec2(v_texCoord.y, 1.0 - v_texCoord.x);
#else
	vec2 t = v_texCoord;
#endif
	gl_FragColor = texture2D(texIn, t);
}
