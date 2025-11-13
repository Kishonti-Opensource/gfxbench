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
out float3 tangent_space_n;
out float3 tangent_space_t;
out float3 tangent_space_b;
out float3 particle_pos; // flat // all corners have the same value
out float additive_factor;
out float shade_factor;

// struct ParticleShaderParams
// {
	// float4 particle_position;
	// float4 particle_size_rotation_opacity;
	// float4 flipbook_frame;
	// float4 particle_additive_roundness_shade;
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
	uint vertex_id = uint(gl_VertexID);
#ifdef PARTICLE_BATCH

	if (vertex_id / 4u >= particle_upload_size)
	{
		gl_Position = float4(1.0, 1.0, 1.0, 0.1);
		return;
	}

	uint particle_base_index = (vertex_id / 4u) * PARTICLE_UPLOAD_STRUCT_SIZE;

	float particle_size = particle_upload_pool[particle_base_index + 1u].x;

	float3x3 billboard_base;
	MakeBillboardBase(billboard_base);

	float3 position_offset = billboard_base * ViewSpaceRotateAndScale(in_position, particle_upload_pool[particle_base_index + 1u].y, particle_size);
	gl_Position = mvp * float4(particle_upload_pool[particle_base_index].xyz + position_offset, 1.0);
	particle_pos = particle_upload_pool[particle_base_index].xyz;

	v_texcoord0 = SelectFlipbookFrame(in_texcoord0_, particle_upload_pool[particle_base_index + 2u]);
	particle_opacity = particle_upload_pool[particle_base_index + 1u].z;

	float3 fts0 = normalize(billboard_base * ViewSpaceRotateAndScale(float3(1.0, 0.0, 0.0), particle_upload_pool[particle_base_index + 1u].y, 1.0));
	float3 fts1 = normalize(billboard_base * ViewSpaceRotateAndScale(float3(0.0, 1.0, 0.0), particle_upload_pool[particle_base_index + 1u].y, 1.0));
	float3 fts2 = normalize(cross(fts0, fts1));
	float3x3 flat_tangent_space = float3x3(fts0,fts1,fts2);

	float roundness = particle_upload_pool[particle_base_index + 3u].y;
	float3 s = in_position.xyz * 2.0;
	float3 sphere_tangent_space_b = normalize(mix(float3(0.0, 1.0, 0.0), float3(s.x*s.y*-0.14645, 0.85355, s.y*-0.5), roundness));
	float3 sphere_tangent_space_n = normalize(mix(float3(0.0, 0.0, 1.0), float3(s.x*0.5, s.y*0.5, 0.70711), roundness));

	tangent_space_b = flat_tangent_space * sphere_tangent_space_b;
	tangent_space_n = flat_tangent_space * sphere_tangent_space_n;
	tangent_space_t = normalize(cross(tangent_space_b, tangent_space_n));

	additive_factor = particle_upload_pool[particle_base_index + 3u].x;
	shade_factor = particle_upload_pool[particle_base_index + 3u].z;

#else

	float3 position_offset = makeBillboardPosition(ViewSpaceRotateAndScale(in_position,	particle_size_rotation_opacity.y, particle_size_rotation_opacity.x));
	float3 debug_particle_offset = float3(0.0, 0.0, 1.0 * float(vertex_id / 4u));
	gl_Position = mvp * float4(particle_position.xyz + position_offset + debug_particle_offset, 1.0);
	v_texcoord0 = SelectFlipbookFrame(in_texcoord0_, flipbook_frame_a_area);
	particle_opacity = particle_size_rotation_opacity.z;

#endif
}

#endif /////////////////////////////////////////////////////////////////////
#ifdef TYPE_fragment
uniform sampler2D<float> gbuffer_depth_texture;
uniform sampler2D<float> color_texture;
uniform sampler2D<float> normal_texture;
uniform float4 light_pos;
uniform float4 depth_parameters;

in float2 v_texcoord0;
in float particle_opacity;
in float3 tangent_space_n;
in float3 tangent_space_t;
in float3 tangent_space_b;
in float3 particle_pos;
in float additive_factor;
in float shade_factor;

out float4 out_color { color(0) };
void main()
{
	// Depth test with the half resolution depth texture
	float linear_fragcoord_z = depth_parameters.y / (gl_FragCoord.z - depth_parameters.x);
	if (linear_fragcoord_z > texelFetch(gbuffer_depth_texture, int2(gl_FragCoord.xy), 0).x)
	{
		discard;
	}

	float3x3 tangent_space = float3x3(
		normalize(tangent_space_t),
		normalize(tangent_space_b),
		normalize(tangent_space_n)
	);

	float3 tex_normal = texture(normal_texture, v_texcoord0).xyz * 2.0 - 1.0;
	out_color = texture(color_texture, v_texcoord0);
	float shade_value = clamp(dot(normalize(light_pos.xyz - particle_pos), normalize(tangent_space * tex_normal)) * 0.5 + 0.5, 0.0, 1.0) * shade_factor + (1.0 - shade_factor);
	out_color.xyz *= shade_value;
	out_color.w *= particle_opacity;
	out_color.xyz *= out_color.w;
	out_color.w = mix(out_color.w, 0.0, additive_factor);

	out_color.w = clamp(out_color.w, 0.0, 1.0);
}

#endif