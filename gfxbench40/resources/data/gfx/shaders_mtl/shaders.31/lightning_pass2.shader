/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifdef TYPE_compute

#include "lightning_pass2.h"
#include "lightning_header.shader"

#define WORK_GROUP_SIZE LP2_WORK_GROUP_SIZE


struct lightning_buffer_layout
{
	_float vertex_counts[LIGHTNING_COUNT];
	_float4 positions[MAX_LIGHTNING_BUFFER];
}; // lightning_buffer;


struct render_buffer_layout
{		
	int count;
	int instance_count;
	int first;
	int base_instance;	

#ifdef OSX
	int __padding[60];
#endif

	_float4 positions[6*MAX_LIGHTNING_BUFFER];
}; // render_buffer;


int get_lightning_vertex_count(int lightning_index, device lightning_buffer_layout *lightning_buffer)
{
	return int(lightning_buffer->vertex_counts[lightning_index]);
}
_float4 load_vertex(int lightning_index, int index, device lightning_buffer_layout *lightning_buffer)
{
	return lightning_buffer->positions[BUFFER_SIZE * lightning_index + index];
}
void store_vertex(int index, _float4 value, device render_buffer_layout *render_buffer)
{
	render_buffer->positions[index] = value;
}
void store_indirect_draw_command(int vertex_count, device render_buffer_layout *render_buffer)
{
	render_buffer->count = vertex_count;
	render_buffer->instance_count = 1;
	render_buffer->first = 0;
	render_buffer->base_instance = 0;	
}


kernel void shader_main(device   lightning_buffer_layout  *lightning_buffer [[ buffer(LP2_LIGHTNING_BFR_SLOT) ]],
					    constant lightning_pass2_uniforms *lu 				[[ buffer(LP2_UNIFORMS_BFR_SLOT)  ]],
					    device   render_buffer_layout     *render_buffer    [[ buffer(LP2_RENDER_BFR_SLOT)    ]],
					             uint                      lightning_index  [[ threadgroup_position_in_grid   ]],
					             uint                      local_index      [[ thread_position_in_threadgroup ]])
{
	threadgroup int vertex_count;
	threadgroup int output_offset;
	threadgroup int pass_count;
	
	if (local_index == 0)
	{			
		if (lightning_index == 0)
		{
			vertex_count = int(lightning_buffer->vertex_counts[0]);
			pass_count = (vertex_count + WORK_GROUP_SIZE - 1) / WORK_GROUP_SIZE;
			output_offset = 0;
			
			int all_render_vertices = vertex_count; 
			for (int i = 1; i < lu->current_lightning_count; i++)
			{
				all_render_vertices += int(lightning_buffer->vertex_counts[i]);
			}
			store_indirect_draw_command(all_render_vertices * 3, render_buffer);
		}
		else
		{
			int prev_vertices = 0;
			for (int i = 0; i < lightning_index; i++)
			{		
				prev_vertices += int(lightning_buffer->vertex_counts[i]);
			}
			
			vertex_count = int(lightning_buffer->vertex_counts[lightning_index]);
			pass_count = (vertex_count + WORK_GROUP_SIZE - 1) / WORK_GROUP_SIZE;
			output_offset = prev_vertices * 6; // 1 vertex -> 1 triangle: 3 pos + 3 texture = 6
		}
	}
	
	threadgroup_barrier(mem_flags::mem_device_and_threadgroup) ;
	
	for (int pass_it = 0; pass_it < pass_count; pass_it++)
	{	
		int idx = pass_it * WORK_GROUP_SIZE + local_index;

		if (idx * 2 >= vertex_count)
		{
			break;
		}

		int input_index = idx * 2;
		_float4 lightning_a = load_vertex(lightning_index, input_index, lightning_buffer);
		_float4 lightning_b = load_vertex(lightning_index, input_index + 1, lightning_buffer);

		_float4 a = lu->mvp * _float4(lightning_a.xyz, 1.0);
		_float4 b = lu->mvp * _float4(lightning_b.xyz, 1.0);
		_float2 delta = b.xy / b.w - a.xy / a.w;

		int output_index = idx * 12 + output_offset;

		_float2 packed_b = unpack_w(lightning_b.w);
		_float intensity_b = packed_b.x;
		_float min_b = 0.0009 * b.z * intensity_b;
		_float max_b = 0.0038 * b.z * intensity_b;
		_float line_width_b = mix(min_b, max_b, packed_b.y);			

		_float2 dir_b = normalize(_float2(delta.y, -delta.x)) * line_width_b;
		_float4 b0 =  _float4(b.x - dir_b.x, b.y - dir_b.y, b.z, b.w);
		_float4 b1 =  _float4(b.x + dir_b.x, b.y + dir_b.y, b.z, b.w);
		_float4 b0uv =  _float4(1.0, 0.0, intensity_b, 0.0);
		_float4 b1uv =  _float4(1.0, 1.0, intensity_b, 0.0);

		store_vertex(output_index + 4, b0, render_buffer);
		store_vertex(output_index + 5, b0uv, render_buffer);

		store_vertex(output_index + 6, b0, render_buffer);		
		store_vertex(output_index + 7, b0uv, render_buffer);	

		store_vertex(output_index + 8, b1, render_buffer);
		store_vertex(output_index + 9, b1uv, render_buffer);
	}		

	for (int pass_it = 0; pass_it < pass_count; pass_it++)
	{	
		int idx = pass_it * WORK_GROUP_SIZE + local_index;

		if (idx * 2 >= vertex_count)
		{
			break;
		}

		int input_index = idx * 2;
		_float4 lightning_a = load_vertex(lightning_index, input_index, lightning_buffer);
		_float4 lightning_b = load_vertex(lightning_index, input_index + 1, lightning_buffer);

		_float4 a = lu->mvp * _float4(lightning_a.xyz, 1.0);
		_float4 b = lu->mvp * _float4(lightning_b.xyz, 1.0);
		_float2 delta = b.xy / b.w - a.xy / a.w;

		int output_index = idx * 12 + output_offset;

		_float2 packed_a = unpack_w(lightning_a.w);
		_float intensity_a = packed_a.x;
		_float min_a = 0.0009 * a.z * intensity_a;
		_float max_a = 0.0038 * a.z * intensity_a;
		_float line_width_a = mix(min_a, max_a, packed_a.y);

		_float2 dir_a = normalize(_float2(delta.y, -delta.x)) * line_width_a;
		_float4 a0 =  _float4(a.x - dir_a.x, a.y - dir_a.y, a.z, a.w);
		_float4 a1 =  _float4(a.x + dir_a.x, a.y + dir_a.y, a.z, a.w);
		_float4 a0uv =  _float4(0.0, 0.0, intensity_a, 0.0);
		_float4 a1uv =  _float4(0.0, 1.0, intensity_a, 0.0);

		store_vertex(output_index, a1, render_buffer);
		store_vertex(output_index + 1, a1uv, render_buffer);

		store_vertex(output_index + 2, a0, render_buffer);
		store_vertex(output_index + 3, a0uv, render_buffer);

		store_vertex(output_index + 10, a1, render_buffer);
		store_vertex(output_index + 11, a1uv, render_buffer);
	}		
}

#endif

