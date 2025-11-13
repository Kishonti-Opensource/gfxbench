/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
//Left top of the screen:
//count shader fp precision bits as bars, differentiate round-to-zero from round-to-even by allowing the range [0.0, 1.0] instead of [0.0, 1.0)
//http://community.arm.com/groups/arm-mali-graphics/blog/2013/06/11/benchmarking-floating-point-precision-in-mobile-gpus--part-ii

//Right top of the screen:
//search for precision around the zero-hole of fp representations
//http://community.arm.com/groups/arm-mali-graphics/blog/2013/10/10/benchmarking-floating-point-precision-in-mobile-gpus--part-iii

#if defined(HIGHP) && defined(GL_ES)
uniform highp vec2 resolution;
uniform highp vec2 exp_limits;
#else
uniform vec2 resolution;
uniform vec2 exp_limits;	
#endif

varying vec2 v_texCoord;

void main( void )
{
	gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0);
	//if(gl_FragCoord.y / resolution.y > 0.5)
	{
		if(gl_FragCoord.x / resolution.x < 0.5)
		{
			float y = ( gl_FragCoord.y / resolution.y * 2.0 - 1.0) * 26.0;
			float x = 1.0 - ( gl_FragCoord.x / (resolution.x * 0.5));
			float p = pow( 2.0, floor(y) );
			float b = clamp( p + x, p, p + 1.0); //avoid compiler optimization of (p + x) - p = x, we want underflow to happen here
			float c = b - p;
			if(fract(y) >= 0.9)
				c = 0.0;
			gl_FragColor = vec4(c, c, c, 1.0 );
		}
		else
		{
#ifdef TEST_DENORMALS
			float y = (gl_FragCoord.y / resolution.y * 2.0 - 1.0) * (exp_limits.y - exp_limits.x);
			float x = (1.0 - (gl_FragCoord.x / resolution.x));
			x *= 2.0;
			float row = floor(y) + (-exp_limits.y);
			for (float c = 0.0; c < row; c = c + 1.0) 
			{
				x = x / 2.0;
			}
			//x will underflow to 0 for higher rows (smaller exponents)
			for (float c = 0.0; c < row; c = c + 1.0)
			{
				x = x * 2.0;
			}
			
			gl_FragColor = vec4(vec3(x), 1.0);
			if (x == 0.0) 
				gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0);
			if (fract(y) > 0.9) 
				gl_FragColor = vec4(0.0, 0.0, 0.0, 1.0);
#else
			gl_FragColor = vec4(0.0,0.0,0.0,1.0) ;
#endif
		}
	}
}
