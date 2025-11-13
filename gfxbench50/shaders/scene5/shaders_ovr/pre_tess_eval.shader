/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
uniform sampler2D color_texture;
uniform sampler2D normal_texture;
uniform sampler2D specular_texture;
uniform float alpha_test_threshold;
uniform vec4 view_pos;
uniform vec4 view_dir;

uniform sampler2D displacement_tex;
uniform vec4 tessellation_factor; 
uniform float cam_near;
uniform vec4 frustum_planes[6];
uniform float tessellation_multiplier;
uniform mat4 mv;
uniform mat4 model;
uniform mat4 inv_model;
uniform mat4 mvp;

layout(triangles, fractional_odd_spacing, ccw) in;

in vec4 tc_position[];
in vec2 tc_texcoord[];
in vec3 tc_normal[];
in vec3 tc_tangent[];

out vec2 texcoord;
out vec3 world_position;
out vec3 normal;
out vec3 tangent;

void main()
{	
	float u = gl_TessCoord.x;
	float v = gl_TessCoord.y;
	float w = gl_TessCoord.z;

	texcoord = tc_texcoord[0] * u + tc_texcoord[1] * v + tc_texcoord[2] * w;
	normal = tc_normal[0] * u + tc_normal[1] * v + tc_normal[2] * w;
	tangent = tc_tangent[0] * u + tc_tangent[1] * v + tc_tangent[2] * w;
	
	vec3 p2 = tc_position[0].xyz * u + tc_position[1].xyz * v + tc_position[2].xyz * w;
	
	float t0_w = texture( displacement_tex, texcoord ).x;
	
	float d = t0_w - 0.5 + tessellation_factor.z;
	p2 += normal * d * tessellation_factor.y * clamp(tessellation_factor.x - 1.0, 0.0, tessellation_factor.w); 
	
	world_position = (model * vec4( p2, 1.0)).xyz;

    gl_Position = mvp * vec4( p2, 1.0);
}
