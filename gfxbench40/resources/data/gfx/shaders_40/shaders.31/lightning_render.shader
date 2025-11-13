/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifdef TYPE_vertex

uniform mat4 mvp;

layout(location = 0) in vec4 in_position;
layout(location = 1) in vec4 in_texcoord;
out vec4 fs_texcoord;

void main()
{			
	gl_Position = in_position;		
	fs_texcoord = in_texcoord;
}

#endif

#ifdef TYPE_fragment

in vec4 fs_texcoord;
out vec4 frag_color;

void main()
{
	const vec4 max_color = vec4(0.627, 0.717, 0.878, 1.0); 
	const vec4 min_color = vec4(0.459, 0.509, 0.663, 1.0);     
    
    frag_color = mix(min_color, max_color,  sin(fs_texcoord.y * 3.14));
    
    float intensity = fs_texcoord.z;	
	if (intensity > 0.9)
	{
		frag_color *= 4.0;
	}	

#ifdef SV_31
	frag_color.xyz *= 0.25; //target is GL_RGB10_A2, which has 4x precision compared to GL_RGBA8, so convert it to range, compress it
#endif		
}

#endif 