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

varying vec4 v_color;

void main()
{
	gl_FragColor = vec4(v_color.xyz, 1.0);
}
