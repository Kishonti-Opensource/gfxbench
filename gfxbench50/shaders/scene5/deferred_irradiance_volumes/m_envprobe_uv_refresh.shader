/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifdef TYPE_vertex

uniform float4x4 mvp;

in float3 in_position;
in float2 in_texcoord0_;

out float2 v_texcoord;

void main()
{    
	gl_Position = mvp * float4( in_position.xyz, 1.0);
	
#ifdef NGL_ORIGIN_UPPER_LEFT_AND_NDC_FLIP
	gl_Position.y *= -1.0;
#endif
	
	v_texcoord = in_texcoord0_;
}

#endif


#ifdef TYPE_fragment

in float2 v_texcoord;

out float4 FragColor { color(0) }; 

void main()
{    
	FragColor = float4(v_texcoord, 0.0, 1.0);
}

#endif
