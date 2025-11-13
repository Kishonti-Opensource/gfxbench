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

uniform sampler2D tex2D;
varying vec2 v_texCoord;

void main()
{
	gl_FragColor = texture2D(tex2D, v_texCoord);
}
