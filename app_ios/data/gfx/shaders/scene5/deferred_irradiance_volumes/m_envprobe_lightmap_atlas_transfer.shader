/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#define envprobe_atlas_inv_size float2(1.0 / (6.0 * 16.0), 1.0 / (16.0 * MAX_NUM_OF_PROBES))

#ifdef TYPE_vertex

in float2 in_position;

void main()
{    
	gl_Position = float4( in_position.xy, 0.0, 1.0);
}

#endif


#ifdef TYPE_fragment

#define SKY_LIGHTING_FACTOR 0.001

uniform sampler2D<float> m_direct_lightmap;
uniform sampler2D<float> m_envprobe_indirect_uv_map;
uniform float4 sky_color;


out float4 FragColor { color(0) }; 

void main()
{

	float2 texcoord2 = gl_FragCoord.xy * envprobe_atlas_inv_size;
	float2 texcoord = texture(m_envprobe_indirect_uv_map, texcoord2).xy;
	FragColor = texture(m_direct_lightmap, texcoord);
		
	//FragColor = float4(texcoord, 0.0, 1.0);

	if(texcoord.x == 0.0 && texcoord.y == 0.0)
	{
		//FragColor = float4(0x94, 0xa4, 0xc0, 1.0);
		//FragColor *= SKY_LIGHTING_FACTOR;
		
		FragColor = float4(sky_color.xyz, 1.0);
		//FragColor = float4(1.0);
		
		//FragColor *= 0.0;
	}	
}

#endif
