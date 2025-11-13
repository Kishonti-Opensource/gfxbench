/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
// http://community.arm.com/groups/arm-mali-graphics/blog/2013/05/29/benchmarking-floating-point-precision-in-mobile-gpus
// Youi Labs GPU precision shader (slightly modified)
#ifdef GL_ES
precision highp float;
#else
#define highp
#endif

uniform vec2 resolution;

void main( void )
{
    float y = ( gl_FragCoord.y / resolution.y ) * 26.0;
    float x = 1.0 - ( gl_FragCoord.x / resolution.x );
    float b = fract( pow( 2.0, floor(y) ) + x );
    if(fract(y) >= 0.8)
        b = 0.0;
    gl_FragColor = vec4(b, b, b, 1.0 );
}
