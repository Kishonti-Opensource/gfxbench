/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifdef TYPE_compute

#include "lightning_header.shader"

#define WORK_GROUP_SIZE 16

layout(std430, binding = 0) readonly buffer lightning_buffer_layout
{
	float vertex_counts[LIGHTNING_COUNT];
	vec4 positions[];
} lightning_buffer;

layout(std430, binding = 1) writeonly buffer render_buffer_layout
{		
	int count;
	int instance_count;
	int first;
	int base_instance;	

	vec4 positions[];
} render_buffer;

layout(location = 1) uniform int current_lightning_count;

layout(location = 2) uniform mat4 mvp;

shared int vertex_count;
shared int output_offset;
shared int pass_count;

int get_lightning_vertex_count(int lightning_index)
{
	return int(lightning_buffer.vertex_counts[lightning_index]);
}
vec4 load_vertex(int lightning_index, int index)
{
	return lightning_buffer.positions[BUFFER_SIZE * lightning_index + index];
}
void store_vertex(int index, vec4 value)
{
	render_buffer.positions[index] = value;
}
void store_indirect_draw_command(int vertex_count)
{
	render_buffer.count = vertex_count;
	render_buffer.instance_count = 1;
	render_buffer.first = 0;
	render_buffer.base_instance = 0;	
}

layout (local_size_x = WORK_GROUP_SIZE, local_size_y = 1, local_size_z = 1) in;
void main()
{
	int lightning_index = int(gl_WorkGroupID.x);
	int local_index = int(gl_LocalInvocationID.x);	
	if (local_index == 0)
	{			
		if (lightning_index == 0)
		{
			vertex_count = int(lightning_buffer.vertex_counts[0]);
			pass_count = (vertex_count + WORK_GROUP_SIZE - 1) / WORK_GROUP_SIZE;
			output_offset = 0;
			
			int all_render_vertices = vertex_count; 
			for (int i = 1; i < current_lightning_count; i++)
			{
				all_render_vertices += int(lightning_buffer.vertex_counts[i]);
			}
			store_indirect_draw_command(all_render_vertices * 3);
		}
		else
		{
			int prev_vertices = 0;
			for (int i = 0; i < lightning_index; i++)
			{		
				prev_vertices += int(lightning_buffer.vertex_counts[i]);
			}
			
			vertex_count = int(lightning_buffer.vertex_counts[lightning_index]);
			pass_count = (vertex_count + WORK_GROUP_SIZE - 1) / WORK_GROUP_SIZE;
			output_offset = prev_vertices * 6; // 1 vertex -> 1 triangle: 3 pos + 3 texture = 6
		}
	}
	
	memoryBarrier();
	barrier();
	
	for (int pass_it = 0; pass_it < pass_count; pass_it++)
	{	
		int idx = pass_it * WORK_GROUP_SIZE + local_index;

		if (idx * 2 >= vertex_count)
		{
			break;
		}

		int input_index = idx * 2;
		vec4 lightning_a = load_vertex(lightning_index, input_index);
		vec4 lightning_b = load_vertex(lightning_index, input_index + 1);

		vec4 a = mvp * vec4(lightning_a.xyz, 1.0);
		vec4 b = mvp * vec4(lightning_b.xyz, 1.0);
		vec2 delta = b.xy / b.w - a.xy / a.w;

		int output_index = idx * 12 + output_offset;

		vec2 packed_b = unpack_w(lightning_b.w);
		float intensity_b = packed_b.x;
		float min_b = 0.0009 * b.z * intensity_b;
		float max_b = 0.0038 * b.z * intensity_b;
		float line_width_b = mix(min_b, max_b, packed_b.y);			

		vec2 dir_b = normalize(vec2(delta.y, -delta.x)) * line_width_b;
		vec4 b0 =  vec4(b.x - dir_b.x, b.y - dir_b.y, b.z, b.w);
		vec4 b1 =  vec4(b.x + dir_b.x, b.y + dir_b.y, b.z, b.w);
		vec4 b0uv =  vec4(1.0, 0.0, intensity_b, 0.0);
		vec4 b1uv =  vec4(1.0, 1.0, intensity_b, 0.0);

		store_vertex(output_index + 4, b0);
		store_vertex(output_index + 5, b0uv);

		store_vertex(output_index + 6, b0);		
		store_vertex(output_index + 7, b0uv);	

		store_vertex(output_index + 8, b1);
		store_vertex(output_index + 9, b1uv);
	}		

	for (int pass_it = 0; pass_it < pass_count; pass_it++)
	{	
		int idx = pass_it * WORK_GROUP_SIZE + local_index;

		if (idx * 2 >= vertex_count)
		{
			break;
		}

		int input_index = idx * 2;
		vec4 lightning_a = load_vertex(lightning_index, input_index);
		vec4 lightning_b = load_vertex(lightning_index, input_index + 1);

		vec4 a = mvp * vec4(lightning_a.xyz, 1.0);
		vec4 b = mvp * vec4(lightning_b.xyz, 1.0);
		vec2 delta = b.xy / b.w - a.xy / a.w;

		int output_index = idx * 12 + output_offset;

		vec2 packed_a = unpack_w(lightning_a.w);
		float intensity_a = packed_a.x;
		float min_a = 0.0009 * a.z * intensity_a;
		float max_a = 0.0038 * a.z * intensity_a;
		float line_width_a = mix(min_a, max_a, packed_a.y);

		vec2 dir_a = normalize(vec2(delta.y, -delta.x)) * line_width_a;
		vec4 a0 =  vec4(a.x - dir_a.x, a.y - dir_a.y, a.z, a.w);
		vec4 a1 =  vec4(a.x + dir_a.x, a.y + dir_a.y, a.z, a.w);
		vec4 a0uv =  vec4(0.0, 0.0, intensity_a, 0.0);
		vec4 a1uv =  vec4(0.0, 1.0, intensity_a, 0.0);

		store_vertex(output_index, a1);
		store_vertex(output_index + 1, a1uv);

		store_vertex(output_index + 2, a0);
		store_vertex(output_index + 3, a0uv);

		store_vertex(output_index + 10, a1);
		store_vertex(output_index + 11, a1uv);
	}		
	/*
	
	int idx;
	float intensity_a;
	float intensity_b;
	float end_point_distance_a;
	float end_point_distance_b;
	float line_width_a;
	float line_width_b;
	for (int pass_it = 0; pass_it < pass_count; pass_it++)
	{	
		idx = pass_it * WORK_GROUP_SIZE + local_index;

		if (idx * 2 >= vertex_count)
		{
			break;
		}						
	
		int input_index = idx * 2;
		int output_index = idx * 12 + output_offset;
		
		vec4 lightning_a = load_vertex(lightning_index, input_index);
		vec4 lightning_b = load_vertex(lightning_index, input_index + 1);
		
		vec4 a = mvp * vec4(lightning_a.xyz, 1.0);
		vec4 b = mvp * vec4(lightning_b.xyz, 1.0);
		
		vec2 packed_a = unpack_w(lightning_a.w);
		vec2 packed_b = unpack_w(lightning_b.w);
		
		intensity_a = packed_a.x;
		end_point_distance_a = packed_a.y;
		
		intensity_b = packed_b.x;
		end_point_distance_b = packed_b.y;
		
		float min_a = 0.0009 * a.z * intensity_a;
		float max_a = 0.0038 * a.z * intensity_a;
		
		float min_b = 0.0009 * b.z * intensity_b;
		float max_b = 0.0038 * b.z * intensity_b;
								
		line_width_a = mix(min_a, max_a, end_point_distance_a);
		line_width_b = mix(min_b, max_b, end_point_distance_b);			
		
		vec2 delta = b.xy / b.w - a.xy / a.w;
		vec2 dir_a = normalize(vec2(delta.y, -delta.x)) * line_width_a;
		vec2 dir_b = normalize(vec2(delta.y, -delta.x)) * line_width_b;
		
		vec4 a0 =  vec4(a.x - dir_a.x, a.y - dir_a.y, a.z, a.w);
		vec4 a1 =  vec4(a.x + dir_a.x, a.y + dir_a.y, a.z, a.w);
		
		vec4 b0 =  vec4(b.x - dir_b.x, b.y - dir_b.y, b.z, b.w);
		vec4 b1 =  vec4(b.x + dir_b.x, b.y + dir_b.y, b.z, b.w);
		
		vec4 a0uv =  vec4(0.0, 0.0, intensity_a, 0.0);
		vec4 a1uv =  vec4(0.0, 1.0, intensity_a, 0.0);
		
		vec4 b0uv =  vec4(1.0, 0.0, intensity_b, 0.0);
		vec4 b1uv =  vec4(1.0, 1.0, intensity_b, 0.0);
		
		store_vertex(output_index, a1);
		store_vertex(output_index + 1, a1uv);
		
		store_vertex(output_index + 2, a0);
		store_vertex(output_index + 3, a0uv);

		store_vertex(output_index + 4, b0);
		store_vertex(output_index + 5, b0uv);
		
		store_vertex(output_index + 6, b0);		
		store_vertex(output_index + 7, b0uv);	

		store_vertex(output_index + 8, b1);
		store_vertex(output_index + 9, b1uv);
		
		store_vertex(output_index + 10, a1);
		store_vertex(output_index + 11, a1uv);
	}	
*/	
}

#endif