/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifdef TYPE_vertex

uniform float4x4 mvp;
uniform float4x4 mv; // used as view matrix (positions a world coords -> model is identity)

in float3 in_position;
in float2 in_texcoord0_;

out float2 v_texcoord0;
out float particle_opacity;
//out float3x3 billboard_base;
out float3 normal_base_n;
out float3 normal_base_t;
out float3 normal_base_b;
out float3 particle_pos; // flat // all corners have the same value
out float particle_size;
out float2 param_coords;
out float pos_w;

// struct ParticleShaderParams
// {
	// float4 particle_position;
	// float4 particle_size_rotation_opacity;
	// float4 flipbook_frame_a_area;
// };

void MakeBillboardBase(inout float3x3 billboard_base)
{
#ifdef AALIGNED
	float3 bbb1 = float3(0.0, 1.0, 0.0);
#else
	float3 bbb1 = float3(mv[0][1], mv[1][1], mv[2][1]);
#endif

	float3 bbb0 = float3( mv[0][0], mv[1][0], mv[2][0]);
	float3 bbb2 = normalize(-float3(mv[3][0], mv[3][1], mv[3][2]));

	billboard_base = float3x3(bbb0,bbb1,bbb2);
}

#ifdef PARTICLE_BATCH
uniform uint particle_upload_size;
uniform float4 particle_upload_pool[PARTICLE_UPLOAD_POOL_SIZE * PARTICLE_UPLOAD_STRUCT_SIZE];
#else
uniform float4 particle_position;
uniform float4 particle_size_rotation_opacity;
uniform float4 flipbook_frame_a_area;
uniform float4 particle_upload_pool[PARTICLE_UPLOAD_POOL_SIZE * PARTICLE_UPLOAD_STRUCT_SIZE];
#endif

float3 ViewSpaceRotateAndScale(float3 pos, float angle, float size)
{
	float c = cos(angle);
	float s = sin(angle);
	return float3(
		(pos.x * c + pos.y * -s) * size,
		(pos.x * s + pos.y * c) * size,
		pos.z);
}

float2 SelectFlipbookFrame(float2 tex_coord, float4 frame_area)
{
	return frame_area.xy + tex_coord * frame_area.zw;
}

void main()
{
#ifdef PARTICLE_BATCH

	uint vertex_id = uint(gl_VertexID);

	if (vertex_id / 4u >= particle_upload_size)
	{
		gl_Position = float4(1.0, 1.0, 1.0, 0.1);
		return;
	}

	uint particle_base_index = (vertex_id / 4u) * PARTICLE_UPLOAD_STRUCT_SIZE;

	particle_size = max(0.2, particle_upload_pool[particle_base_index + 1u].x);

	float3x3 billboard_base;
	MakeBillboardBase(billboard_base);
	float3 position_offset = billboard_base * ViewSpaceRotateAndScale(in_position, particle_upload_pool[particle_base_index + 1u].y, particle_size);
	gl_Position = mvp * float4(particle_upload_pool[particle_base_index].xyz + position_offset, 1.0);
	particle_pos = particle_upload_pool[particle_base_index].xyz;

	v_texcoord0 = SelectFlipbookFrame(in_texcoord0_, particle_upload_pool[particle_base_index + 2u]);
	particle_opacity = particle_upload_pool[particle_base_index + 1u].z;
	param_coords = in_position.xy;
	pos_w = gl_Position.w;

#else

	float3 position_offset = makeBillboardPosition(ViewSpaceRotateAndScale(in_position,	particle_size_rotation_opacity.y, particle_size_rotation_opacity.x));
	float3 debug_particle_offset = float3(0.0, 0.0, 1.0 * (vertex_id / 4u));
	gl_Position = mvp * float4(particle_position.xyz + position_offset + debug_particle_offset, 1.0);
	v_texcoord0 = SelectFlipbookFrame(in_texcoord0_, flipbook_frame_a_area);
	particle_opacity = particle_size_rotation_opacity.z;
	param_coords = in_position.xy;

#endif
}

#endif /////////////////////////////////////////////////////////////////////
#ifdef TYPE_fragment

uniform float4x4 mv; // used as view matrix (positions a world coords -> model is identity)

uniform sampler2D<float> color_texture;
uniform sampler2D<float> normal_texture;
uniform float4 light_pos;

in float2 v_texcoord0;
in float particle_opacity;
in float3 normal_base_n;
in float3 normal_base_t;
in float3 normal_base_b;
in float3 particle_pos;
in float particle_size;
in float2 param_coords;
in float pos_w;

out float4 out_color { color(0) };

void main()
{
	//float3x3 normal_base2;
	//normal_base2[0] = normalize(normal_base_t);
	//normal_base2[1] = normalize(normal_base_b);
	//normal_base2[2] = normalize(normal_base_n);
	float3 tex_normal = texture(normal_texture, v_texcoord0).xyz * 2.0 - 1.0;
	//out_color = texture(color_texture, v_texcoord0);
	out_color = float4(1.0, 1.0, 1.0, 0.0);

	float2 param_dist = abs(param_coords.xy);
	float t = 0.005 / particle_size * pos_w;
	if (max(param_dist.x, param_dist.y) > 0.5 * (1.0 - t))
		out_color = float4(1.0, 1.0, 1.0, 0.5);

	out_color *= out_color.w;
}

#endif