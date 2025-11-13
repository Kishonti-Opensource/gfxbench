/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
uniform sampler2D<float> color_texture;
uniform sampler2D<float> normal_texture;
uniform sampler2D<float> specular_texture;
uniform float alpha_test_threshold;
uniform float4 view_pos;
uniform float4 view_dir;

uniform sampler2D<float> displacement_tex;
uniform float4 tessellation_factor; 
uniform float cam_near;
uniform float4 frustum_planes[6];
uniform float tessellation_multiplier;
uniform float4x4 mv;
uniform float4x4 model;
uniform float4x4 inv_model;
uniform float4x4 mvp;

in float3 in_position;
in float2 in_texcoord0_;
in float3 in_normal;
in float3 in_tangent;

out float4 v_position;
out float2 v_texcoord;
out float3 v_normal;
out float3 v_tangent;

void main()
{	
	float4 p = float4( in_position, 1.0);

	v_position = p;
	v_texcoord = in_texcoord0_;
	v_normal = in_normal; 
	v_tangent = in_tangent; 
}
