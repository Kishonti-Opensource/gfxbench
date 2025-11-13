/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */

//out vec2 texcoord;
struct Driver2InOut
{
    hfloat4 m_Position [[position]];
    v_float2 texcoord [[user(texcoord)]];
};


#ifdef TYPE_vertex 

//in vec2 myVertex;
struct VertexInput
{
    hfloat2 myVertex [[attribute(0)]];
};


vertex Driver2InOut shader_main(VertexInput in [[ stage_in]])
{
	Driver2InOut res;

	res.m_Position = _float4(in.myVertex, 1.0, 1.0);
	_float2 texcoord = (in.myVertex+_float2(1.0))/2.0;
	texcoord.y = 1.0-texcoord.y;
	
	res.texcoord = v_float2(texcoord);
	
	return res;
}
#endif

#ifdef TYPE_fragment

//uniform sampler2D texIn;
//in vec2 texcoord;
//out vec4 color;

fragment _float4 shader_main(Driver2InOut       in            [[ stage_in   ]],
					         texture2d<_float>  texIn         [[ texture(0) ]],
					         sampler            texIn_sampler [[ sampler(0) ]])
{
	_float4 color = texIn.sample(texIn_sampler, hfloat2(in.texcoord));
	return color;
}

#endif

