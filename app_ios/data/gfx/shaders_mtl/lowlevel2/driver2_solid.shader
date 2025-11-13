/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */

//out vec2 texpos;

struct Driver2InOut
{
    hfloat4 m_Position [[position]];
    v_float2 texpos [[user(texpos)]];
};


#ifdef TYPE_vertex 

//in vec3 myVertex;

struct VertexInput
{
    hfloat3 myVertex [[attribute(0)]];
};


//uniform vec2 u_position;
//uniform vec2 u_gridsize;
//uniform vec2 u_margin;
//uniform vec2 u_globmargin;
//uniform mat2 u_transform_matrix;

struct Uniforms
{
	hfloat2 u_position;
	hfloat2 u_gridsize;
	hfloat2 u_margin;
	hfloat2 u_globmargin;
	hfloat2x2 u_transform_matrix;
};


vertex Driver2InOut shader_main(         VertexInput in       [[ stage_in]],
		          				constant Uniforms*   uniforms [[ buffer(1) ]] )
{
	Driver2InOut res;

	_float2 cellsize = (_float2(2.0)-uniforms->u_globmargin*2.0)/uniforms->u_gridsize;
	_float2 localCoord = in.myVertex.xy * (_float2(1.0) - uniforms->u_margin) * cellsize / 2.0;
	_float2 pos = _float2(-1.0) + uniforms->u_globmargin + cellsize/2.0 + cellsize*uniforms->u_position;
	_float2 abspos = pos + localCoord;
	res.m_Position = _float4(abspos*uniforms->u_transform_matrix, in.myVertex.z, 1.0);
	res.texpos = v_float2(abspos/2.0+_float2(0.5));
	
	return res;
}

#endif

#ifdef TYPE_fragment

//uniform sampler2D texIn;
//uniform vec3 u_color;

struct Uniforms
{
	hfloat3 u_color;
};

//in vec2 texpos;
//out vec4 color;

fragment _float4 shader_main(Driver2InOut       in            [[ stage_in   ]],
							 constant Uniforms* uniforms      [[ buffer(1)  ]],
							 texture2d<_float>  texIn         [[ texture(0) ]],
					         sampler            texIn_sampler [[ sampler(0) ]])
{
	_float4 color = _float4(_float3(uniforms->u_color)+texIn.sample(texIn_sampler,hfloat2(in.texpos)).rgb-_float3(0.5), 1.0);
	return color;
} 

#endif

