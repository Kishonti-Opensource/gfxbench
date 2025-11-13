/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#line 1

#ifdef vertex_main

layout(location = 0) in vec3 in_position;
layout(location = 1) in vec2 in_texcoord;
layout(location = 2) in vec3 in_normal;
layout(location = 3) in vec3 in_tangent;

layout(location = 0) out vec2 v_texcoord;
layout(location = 1) out vec3 v_normal;
layout(location = 2) out vec3 v_tangent;

void main()
{	
	gl_Position = vec4( in_position, 1.0);
	v_texcoord = in_texcoord;
	v_normal = in_normal;
	v_tangent = in_tangent;
}

#endif


#ifdef tess_control_main

layout(vertices = 3) out;

layout(std140, binding = 3) uniform uniformObject0 {
uniform vec4 tess_factor;
};

layout(location = 0) in vec2 v_texcoord[];
layout(location = 1) in vec3 v_normal[];
layout(location = 2) in vec3 v_tangent[];

layout(location = 0) out vec2 tc_texcoord[];
layout(location = 1) out vec3 tc_normal[];
layout(location = 2) out vec3 tc_tangent[];

void main()
{
	gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;
	tc_texcoord[gl_InvocationID] = v_texcoord[gl_InvocationID];
	tc_normal[gl_InvocationID] = v_normal[gl_InvocationID];
	tc_tangent[gl_InvocationID] = v_tangent[gl_InvocationID];
	
	gl_TessLevelInner[0] = tess_factor.x;
	gl_TessLevelInner[1] = tess_factor.x;

	gl_TessLevelOuter[0] = tess_factor.x;
	gl_TessLevelOuter[1] = tess_factor.x;
	gl_TessLevelOuter[2] = tess_factor.x;
	gl_TessLevelOuter[3] = tess_factor.x;

	vec3 view_dir = vec3(-1.0, -1.0, -1.0);
	
	view_dir = normalize(view_dir);
	
	float ndotv = dot(view_dir, v_normal[gl_InvocationID]);
	if( ndotv > 0.0)
	{
		gl_TessLevelInner[0] = 0.0;
		gl_TessLevelInner[1] = 0.0;

		gl_TessLevelOuter[0] = 0.0;
		gl_TessLevelOuter[1] = 0.0;
		gl_TessLevelOuter[2] = 0.0;
		gl_TessLevelOuter[3] = 0.0;
	}	
}

#endif


#ifdef tess_eval_main

layout(triangles, fractional_odd_spacing, cw) in;

layout(std140, binding = 4) uniform uniformObject1 {
uniform mat4 vp;
uniform mat4 model;
};
layout(binding = 5) uniform sampler2D displacement_map;

layout(location = 0) in vec2 tc_texcoord[];
layout(location = 1) in vec3 tc_normal[];
layout(location = 2) in vec3 tc_tangent[];

layout(location = 0) out vec2 te_texcoord;
layout(location = 1) out vec3 te_normal;
layout(location = 2) out vec3 te_tangent;
layout(location = 3) out vec3 te_bitangent;

void main()
{	
	float u = gl_TessCoord.x;
	float v = gl_TessCoord.y;
	float w = gl_TessCoord.z;

	te_texcoord = tc_texcoord[0] * u + tc_texcoord[1] * v + tc_texcoord[2] * w;
	te_normal = tc_normal[0] * u + tc_normal[1] * v + tc_normal[2] * w;
	te_tangent = tc_tangent[0] * u + tc_tangent[1] * v + tc_tangent[2] * w;
	vec3 position = gl_in[0].gl_Position.xyz * u + gl_in[1].gl_Position.xyz * v + gl_in[2].gl_Position.xyz * w;
	
	float height = texture(displacement_map, te_texcoord).x - 0.5;
	position += te_normal * height * 0.25;
	
	vec4 world_position = model * vec4(position, 1.0);
	
	gl_Position = vp * world_position;
	te_normal = (model * vec4( te_normal, 0.0)).xyz;
	te_tangent = (model * vec4( te_tangent , 0.0)).xyz;
	te_bitangent = normalize( cross( te_tangent, te_normal));
}

#endif



#ifdef fragment_main

layout(std140, binding = 0) uniform uniformObject2 {
uniform vec4 color;
};
layout(binding = 1) uniform sampler2D color_map;
layout(binding = 2) uniform sampler2D normal_map;

layout(location = 0) in vec2 te_texcoord;
layout(location = 1) in vec3 te_normal;
layout(location = 2) in vec3 te_tangent;
layout(location = 3) in vec3 te_bitangent;

layout(location = 0) out vec4 frag_color;

void main()
{	
	vec3 light_dir = vec3(1.0, 2.0, 3.0);
	light_dir = normalize(light_dir);
	
	vec3 n = normalize( te_normal);
	vec3 t = normalize( te_tangent);
	vec3 b = normalize( te_bitangent);
	mat3 tbn = mat3( t, b, n);


	vec3 texel_normal = texture(normal_map, te_texcoord).xyz;
	texel_normal.xy = texel_normal.xy * 2.0 - 1.0;
	texel_normal.xyz = normalize( tbn * texel_normal.xyz);

	float ndotl = dot(light_dir, texel_normal);
	ndotl = clamp(ndotl, 0.0, 1.0);

	vec4 texel_color = texture(color_map, te_texcoord);

	frag_color = texel_color * mix(color, vec4(1.0), ndotl);
}

#endif
