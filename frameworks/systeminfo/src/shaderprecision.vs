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

attribute vec4 a_position;

void main( void )
{
    gl_Position = a_position;
}
