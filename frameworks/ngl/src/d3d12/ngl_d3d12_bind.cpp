/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "ngl_d3d12_bind.h"

#include "ngl_d3d12.h"
#include "ngl_d3d12_command.h"
#include "ngl_d3d12_util.h"
#include "ngl_d3d12_renderer.h"
#include "ngl_d3d12_resource.h"


// D3D12_heap ////////////////////////////////////////////////////////////////////////

D3D12_descriptor_heap::D3D12_descriptor_heap()
{
	m_max_num = 0;
	m_num = 0;
	m_descriptor_size = 0;
	m_heap = nullptr;
}


D3D12_descriptor_heap::~D3D12_descriptor_heap()
{
	SAFE_RELEASE(m_heap);
}


ID3D12DescriptorHeap *D3D12_descriptor_heap::Init(ID3D12Device *device, uint32_t num, D3D12_DESCRIPTOR_HEAP_TYPE type, D3D12_DESCRIPTOR_HEAP_FLAGS flags)
{
	m_max_num = num;

	D3D12_DESCRIPTOR_HEAP_DESC heap_desc = {};
	heap_desc.NumDescriptors = num;
	heap_desc.Type = type;
	heap_desc.Flags = flags;

	ThrowIfFailed(device->CreateDescriptorHeap(&heap_desc, IID_PPV_ARGS(&m_heap)));

	m_cpu_handle = m_heap->GetCPUDescriptorHandleForHeapStart();
	m_descriptor_size = device->GetDescriptorHandleIncrementSize(type);

	return m_heap;
}


CD3DX12_CPU_DESCRIPTOR_HANDLE D3D12_descriptor_heap::AddCPUHandle(UINT &index)
{
	assert(m_num + 1 < m_max_num);

	CD3DX12_CPU_DESCRIPTOR_HANDLE handle(m_cpu_handle);
	handle.Offset(m_num, m_descriptor_size);
	index = m_num;
	m_num++;
	return handle;
}


CD3DX12_CPU_DESCRIPTOR_HANDLE D3D12_descriptor_heap::GetCPUHandle(UINT index)
{
	CD3DX12_CPU_DESCRIPTOR_HANDLE handle(m_cpu_handle);
	handle.Offset(index, m_descriptor_size);
	return handle;
}


CD3DX12_GPU_DESCRIPTOR_HANDLE D3D12_descriptor_heap::GetGPUHandle(UINT index)
{
	CD3DX12_GPU_DESCRIPTOR_HANDLE sampler_heap_start(m_heap->GetGPUDescriptorHandleForHeapStart());
	return sampler_heap_start.Offset(index, m_descriptor_size);
}


// D3D12_view_bind_heap ////////////////////////////////////////////////////////////////////////

D3D12_view_bind_heap::D3D12_view_bind_heap()
{
	m_source_heap = nullptr;
	m_next_start_index = 0;
	m_last_available_index = 0;
	m_max_used_index = 0;
	m_has_active_block = false;
}


D3D12_view_bind_heap::~D3D12_view_bind_heap()
{
}


void D3D12_view_bind_heap::Init(D3D12_descriptor_heap *source_heap, D3D12_command_queue *command_queue, uint32_t heap_size)
{
	m_command_queue = command_queue;
	m_source_heap = source_heap;
	m_heap_size = heap_size;
	m_heap.Init(m_command_queue->m_backend->m_device, m_heap_size, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE);
	m_last_available_index = m_heap_size - 1;
	m_has_active_block = false;
}


void D3D12_view_bind_heap::BeginBlock()
{
	assert(m_has_active_block == false);
	m_has_active_block = true;
	m_active_block = D3D12_bind_heap_block();
	m_active_block.m_range.m_start_index = m_next_start_index;
	m_active_block.m_range.m_num = 0;
}


void D3D12_view_bind_heap::RequestRange(const D3D12_root_signature &rs, D3D12_root_signature_shader_stage &rs_shader, D3D12_descriptor_copy_batch &copy_batch)
{
	assert(m_has_active_block == true);

	const std::vector<uint32_t> &view_indices = rs.m_view_indices;
	uint32_t num = (uint32_t)rs_shader.m_view_index_range.m_num;
	assert(num > 0);
	assert(m_next_start_index <= m_last_available_index + 1);

	uint32_t available_space = (m_last_available_index + 1) - m_next_start_index;

	while (available_space < num)
	{
		if (m_discarded_blocks.empty())
		{
			_logf("View bind heap error: not enough space");
			assert(false);
		}

		D3D12_bind_heap_block &discarded_block = m_discarded_blocks.front();
		if (discarded_block.m_range.m_start_index < m_next_start_index) // has looped
		{
			m_last_available_index = m_heap_size - 1;
			available_space = (m_last_available_index + 1) - m_next_start_index;

			if (available_space >= num)
			{
				break;
			}
			else
			{
				assert(discarded_block.m_range.m_start_index == 0);

				m_active_blocks.push(m_active_block);

				m_next_start_index = 0;
				m_last_available_index = discarded_block.m_range.m_num - 1;
				available_space = (m_last_available_index + 1) - m_next_start_index;

				m_active_block = D3D12_bind_heap_block();
				m_active_block.m_range.m_start_index = m_next_start_index;
				m_active_block.m_range.m_num = 0;
			}
		}
		else
		{
			if (!m_command_queue->IsFenceComplete(discarded_block.m_fence_value))
			{
				_logf("Performance warning: waiting for view bind heap space"); // increase view bind heap size
				m_command_queue->WaitForCompletion(discarded_block.m_fence_value);
			}

			m_last_available_index = discarded_block.m_range.m_start_index + (discarded_block.m_range.m_num - 1);
			available_space = (m_last_available_index + 1) - m_next_start_index;
		}

		m_discarded_blocks.pop();
	}

	assert(available_space >= num);

	D3D12_bind_heap_range new_range;
	new_range.m_start_index = m_next_start_index;
	new_range.m_num = num;

	for (uint32_t k = 0; k < num; k++)
	{
		uint32_t view_index = view_indices[rs_shader.m_view_index_range.m_start_index + k];

		//m_command_queue->m_backend->m_device->CopyDescriptorsSimple(1,
		//	m_heap.GetCPUHandle(new_range.m_start_index + k),
		//	m_source_heap->GetCPUHandle(view_index),
		//	D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

		copy_batch.Add(m_heap.GetCPUHandle(new_range.m_start_index + k), m_source_heap->GetCPUHandle(view_index));
	}

	m_active_block.m_range.m_num += num;
	m_next_start_index += num;

	//rs_shader.m_view_range = new_range;
	rs_shader.m_view_gpu_handle = m_heap.GetGPUHandle(new_range.m_start_index);
}


void D3D12_view_bind_heap::EndBlock(uint64_t fence_value)
{
	m_has_active_block = false;

	while (!m_active_blocks.empty())
	{
		D3D12_bind_heap_block block = m_active_blocks.front();
		block.m_fence_value = fence_value;
		if (block.m_range.m_num > 0)
		{
			m_discarded_blocks.push(block);
		}
		m_active_blocks.pop();
	}

	m_active_block.m_fence_value = fence_value;
	if (m_active_block.m_range.m_num > 0)
	{
		m_discarded_blocks.push(m_active_block);
	}
	m_active_block = D3D12_bind_heap_block();
}


// D3D12_sampler_bind_heap ////////////////////////////////////////////////////////////////////////

D3D12_sampler_bind_heap::D3D12_sampler_bind_heap()
{
	m_source_heap = nullptr;
	m_next_start_index = 0;
}


D3D12_sampler_bind_heap::~D3D12_sampler_bind_heap()
{

}


void D3D12_sampler_bind_heap::Init(D3D12_descriptor_heap *source_heap, D3D12_command_queue *command_queue, uint32_t heap_size)
{
	assert(heap_size <= 2048); // sampler heap can't be larger
	m_source_heap = source_heap;
	m_command_queue = command_queue;
	m_heap_size = heap_size;
	m_heap.Init(m_command_queue->m_backend->m_device, m_heap_size, D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE);
}


void D3D12_sampler_bind_heap::RequestRange(const D3D12_root_signature &rs, D3D12_root_signature_shader_stage &rs_shader, D3D12_descriptor_copy_batch &copy_batch)
{
	const std::vector<uint32_t> &sampler_indices = rs.m_sampler_indices;
	const D3D12_bind_heap_index_range &index_range = rs_shader.m_sampler_index_range;
	assert(index_range.m_num > 0);

	// Generate key

	uint64_t key = 0;
	assert(index_range.m_num <= 14); // if sampler count exceeds this, hashing needs to change
	for (uint32_t i = 0; i < rs_shader.m_sampler_index_range.m_num; i++)
	{
		assert(sampler_indices[index_range.m_start_index + i] <= 15);
		key += sampler_indices[index_range.m_start_index + i];
		key <<= 4;
	}
	key <<= 4;
	key += (uint64_t)index_range.m_num;

	// Select range

	if (m_ranges.find(key) != m_ranges.end())
	{
		//rs_shader.m_sampler_range = m_ranges.at(key);
		rs_shader.m_sampler_gpu_handle = m_heap.GetGPUHandle(m_ranges.at(key).m_start_index);
	}
	else
	{
		assert(m_next_start_index + index_range.m_num <= m_heap.m_max_num);

		D3D12_bind_heap_range new_range;
		new_range.m_start_index = m_next_start_index;
		new_range.m_num = index_range.m_num;

		for (uint32_t k = 0; k < index_range.m_num; k++)
		{
			uint32_t sampler_index = sampler_indices[index_range.m_start_index + k];

			//m_command_queue->m_backend->m_device->CopyDescriptorsSimple(1,
			//	m_heap.GetCPUHandle(new_range.m_start_index + k),
			//	m_source_heap->GetCPUHandle(sampler_index),
			//	D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);

			copy_batch.Add(m_heap.GetCPUHandle(new_range.m_start_index + k), m_source_heap->GetCPUHandle(sampler_index));
		}

		m_next_start_index += index_range.m_num;
		m_ranges[key] = new_range;

		//rs_shader.m_sampler_range = new_range;
		rs_shader.m_sampler_gpu_handle = m_heap.GetGPUHandle(new_range.m_start_index);

		//_logf("Sampler heap allocation (total entries: %d, range count: %d)", m_next_start_index, (uint32_t)m_ranges.size());
	}
}


// D3D12_shader_reflection //////////////////////////////////////////////////////////////////////

D3D12_shader_reflection::D3D12_shader_reflection()
{
	m_d3d12_reflection = nullptr;
}


D3D12_shader_reflection::~D3D12_shader_reflection()
{
	ReleaseD3D12Resources();
}


void D3D12_shader_reflection::ReleaseD3D12Resources()
{
	if (m_d3d12_reflection != nullptr)
	{
		m_d3d12_reflection->Release();
		m_d3d12_reflection = nullptr;
	}
}


void D3D12_shader_reflection::Init(NGL_shader_type shader_type, ID3DBlob *shader, const AppUniformMap &app_uniform_map)
{
	ThrowIfFailed(D3DReflect(shader->GetBufferPointer(), shader->GetBufferSize(), IID_PPV_ARGS(&m_d3d12_reflection)));

	D3D12_SHADER_DESC shader_desc;
	ThrowIfFailed(m_d3d12_reflection->GetDesc(&shader_desc));
	m_constant_buffers.resize(shader_desc.ConstantBuffers);

	for (UINT i = 0; i < shader_desc.ConstantBuffers; i++)
	{
		ID3D12ShaderReflectionConstantBuffer *cbuffer = m_d3d12_reflection->GetConstantBufferByIndex(i);

		D3D12_SHADER_BUFFER_DESC buffer_desc;
		ThrowIfFailed(cbuffer->GetDesc(&buffer_desc));

		if (buffer_desc.Type != D3D_CT_CBUFFER)
		{
			//_logf("DX12 - ShaderReflection: Unhandled constant buffer type: %d", (int32_t)buffer_desc.Type);
			//assert(false);
			continue;
		}

		m_constant_buffers[i].m_name = buffer_desc.Name;
		m_constant_buffers[i].m_size = buffer_desc.Size;

		for (UINT j = 0; j < buffer_desc.Variables; j++)
		{
			ID3D12ShaderReflectionVariable *variable = cbuffer->GetVariableByIndex(j);
			D3D12_SHADER_VARIABLE_DESC variable_desc;
			ThrowIfFailed(variable->GetDesc(&variable_desc));

			ID3D12ShaderReflectionType *type = variable->GetType();
			D3D12_SHADER_TYPE_DESC type_desc;
			type->GetDesc(&type_desc);

			m_constant_buffers[i].m_variables.push_back(D3D12_shader_variable(variable_desc, type_desc));
		}
	}

	uint32_t cb_index = 0;
	for (UINT i = 0; i < shader_desc.BoundResources; i++)
	{
		D3D12_SHADER_INPUT_BIND_DESC input_desc;
		ThrowIfFailed(m_d3d12_reflection->GetResourceBindingDesc(i, &input_desc));

		if (input_desc.Type == D3D_SIT_CBUFFER)
		{
			assert(m_constant_buffers[cb_index].m_name == input_desc.Name);
			//result.m_cbuffer_desc = input_desc;
			m_constant_buffers[cb_index].m_bind_point = input_desc.BindPoint;
			cb_index++;
		}
		else
		{
			m_shader_inputs.push_back(input_desc);
		}
	}
}


//bool D3D12_shader_reflection::FindShaderInput(const char *name, D3D12_SHADER_INPUT_BIND_DESC &desc)
//{
//	for (size_t i = 0; i < m_shader_inputs.size(); i++)
//	{
//		if (strcmp(m_shader_inputs[i].Name, name) == 0)
//		{
//			desc = m_shader_inputs[i];
//			return true;
//		}
//	}
//	return false;
//}


// D3D12_root_signature_shader_stage ///////////////////////////////////////////////////////////////////////////////

void D3D12_root_signature_shader_stage::Init(D3D12_uniform_group uniform_group, NGL_shader_type shader_type)
{
	m_uniform_group = uniform_group;
	m_shader_type = shader_type;
}


// D3D12_root_signature ///////////////////////////////////////////////////////////////////////////////

void D3D12_root_signature::Init(D3D12_renderer *renderer, const std::vector<NGL_shader_uniform> &application_uniforms)
{
	m_renderer = renderer;
	m_static_samplers.clear();

	// Root Signatere parameter ordering rules decreasing by precedence:
	//  - Root descriptors (e.i. CBVs) first
	//  - Most frequently invoked first (shader_type)
	//  - Most frequently changed first (uniform_group)
	//  - Root constants last

	// decreasing by invocation frequency
	NGL_shader_type shader_stage_order[NGL_NUM_SHADER_TYPES] = {
		NGL_FRAGMENT_SHADER,
		NGL_TESS_EVALUATION_SHADER,
		NGL_TESS_CONTROL_SHADER,
		NGL_GEOMETRY_SHADER,
		NGL_VERTEX_SHADER,
		NGL_COMPUTE_SHADER
	};

	// decreasing by change frequency
	D3D12_uniform_group uniform_group_order[NGL_DX12_NUM_UNIFORM_GROUPS] = {
		NGL_DX12_UNIFORM_GROUP_PER_DRAW,
		NGL_DX12_UNIFORM_GROUP_PER_RENDERER_CHANGE
	};

	AppUniformMap app_uniform_map;
	for (size_t i = 0; i < application_uniforms.size(); i++)
	{
		app_uniform_map[&application_uniforms[i].m_name] = (uint32_t)i;
	}

	std::vector<CD3DX12_ROOT_PARAMETER> root_parameters;
	std::vector<CD3DX12_ROOT_PARAMETER> root_tables;

	for (uint32_t j = 0; j < NGL_NUM_SHADER_TYPES; j++)
	{
		NGL_shader_type shader_type = shader_stage_order[j];

		ID3DBlob *shader_blob = m_renderer->m_shader_blobs[shader_type];
		if (shader_blob == nullptr)
		{
			m_flags |= GetShaderDenyFlag(shader_type);
			continue;
		}

		if (shader_type == NGL_VERTEX_SHADER)
		{
			m_flags |= D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
		}

		// Execture shader reflection
		
		D3D12_shader_reflection &reflection = m_reflections[shader_type];
		reflection.Init(shader_type, shader_blob, app_uniform_map);

		for (uint32_t k = 0; k < NGL_DX12_NUM_UNIFORM_GROUPS; k++)
		{
			D3D12_uniform_group uniform_group = uniform_group_order[k];

			D3D12_root_signature_shader_stage &rs_shader = m_groups[uniform_group].m_stages[shader_type];
			rs_shader.Init(uniform_group, shader_type);

			// Constant buffer reflection

			if (reflection.m_constant_buffers.size() > 0)
			{
				ConstantBufferReflection(rs_shader, reflection, app_uniform_map, application_uniforms);
			}

			// Shader input reflection (textures, samplers, buffers...)

			DescriptorTableReflection(rs_shader, reflection, app_uniform_map, application_uniforms);
		}

		reflection.ReleaseD3D12Resources();
	}

	// Collect RS parameters

	int32_t root_argument_budget = 0;
	int32_t root_constant_budget = 0;
	int32_t num_total_root_constants = 0;
	m_has_root_contants = false;

#ifdef NGL_DX12_DYNAMIC_ROOT_CONSTANT_UPLOAD

	uint32_t num_tables = 0;
	int32_t num_root_cbvs = 0;

	for (uint32_t k = 0; k < NGL_DX12_NUM_UNIFORM_GROUPS; k++)
	{
		D3D12_uniform_group uniform_group = uniform_group_order[k];

		for (uint32_t j = 0; j < NGL_NUM_SHADER_TYPES; j++)
		{
			NGL_shader_type shader_type = shader_stage_order[j];
			D3D12_SHADER_VISIBILITY visibility = GetShaderVisibility(shader_type);

			D3D12_root_signature_shader_stage &rs_shader = m_groups[uniform_group].m_stages[shader_type];

			rs_shader.m_num_view_bind_heap_entries = (uint32_t)rs_shader.m_view_table.size();
			rs_shader.m_num_sampler_bind_heap_entries = (uint32_t)rs_shader.m_sampler_table.size();

			if (rs_shader.m_num_view_bind_heap_entries > 0)
			{
				num_tables++;
			}

			if (rs_shader.m_num_sampler_bind_heap_entries > 0)
			{
				num_tables++;
			}
		}
	}

	for (uint32_t k = 0; k < NGL_DX12_NUM_UNIFORM_GROUPS; k++)
	{
		D3D12_uniform_group uniform_group = uniform_group_order[k];

		for (uint32_t j = 0; j < NGL_NUM_SHADER_TYPES; j++)
		{
			NGL_shader_type shader_type = shader_stage_order[j];

			D3D12_root_signature_shader_stage &rs_shader = m_groups[uniform_group].m_stages[shader_type];
			D3D12_shader_reflection &reflection = m_reflections[shader_type];

			if (rs_shader.m_cb_size > 0)
			{
				num_root_cbvs++;
			}
		}
	}

	root_argument_budget = NGL_DX12_ROOT_ARGUMENT_SPACE_TARGET_SIZE; // 64 DWORDs are the max (e.i. 64 floats)
	assert(root_argument_budget >= 0 && root_argument_budget <= 64);

	root_argument_budget -= (m_flags & D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT) > 0 ? 1 : 0; // -1 for using input assembler
	root_argument_budget -= num_tables; // tables cost 1
	root_argument_budget -= num_root_cbvs * 2; // root descriptors cost 2

	root_constant_budget = root_argument_budget;// >= 0 ? root_argument_budget : 0;

	if (root_constant_budget > 0)
	{
		for (uint32_t k = 0; k < NGL_DX12_NUM_UNIFORM_GROUPS; k++)
		{
			D3D12_uniform_group uniform_group = uniform_group_order[k];

			for (uint32_t j = 0; j < NGL_NUM_SHADER_TYPES; j++)
			{
				NGL_shader_type shader_type = shader_stage_order[j];

				D3D12_root_signature_shader_stage &rs_shader = m_groups[uniform_group].m_stages[shader_type];

				if (rs_shader.m_cb_has_array)
				{
					continue;
				}

#ifdef NGL_DX12_DEBUG_LARGE_CBVS
				if (rs_shader.m_cb_size > 0 && (int32_t)rs_shader.m_cb_num_constants > root_constant_budget + 2)
				{
					_logf("DX12: CBV too large for root constant. CBV constant count: %d Root constant budget: %d", (int32_t)rs_shader.m_cb_num_constants, root_constant_budget + 2);
				}
#endif

				if (rs_shader.m_cb_size > 0 && (int32_t)rs_shader.m_cb_num_constants <= root_constant_budget + 2)
				{
					rs_shader.m_cb_bind_as_root_constants = true;
					root_constant_budget -= ((int32_t)rs_shader.m_cb_num_constants - 2);
					num_total_root_constants += rs_shader.m_cb_num_constants;
					m_has_root_contants = true;
				}
			}
		}
	}
	else
	{
		root_constant_budget = 0;
	}

#else

	root_constant_budget = 32; // 128 bytes
	root_argument_budget = (int32_t)root_constant_budget;

	{
		D3D12_uniform_group uniform_group = NGL_DX12_UNIFORM_GROUP_PER_DRAW;
		NGL_shader_type shader_type = NGL_FRAGMENT_SHADER;

		D3D12_root_signature_shader_stage &rs_shader = m_groups[uniform_group].m_stages[shader_type];

#ifdef NGL_DX12_DEBUG_LARGE_CBVS
		if (rs_shader.m_cb_size > 0 && (int32_t)rs_shader.m_cb_num_constants > root_constant_budget)
		{
			_logf("DX12: CBV too large for root constant. CBV constant count: %d Root constant budget: %d", (int32_t)rs_shader.m_cb_num_constants, root_constant_budget);
		}
#endif

		if (rs_shader.m_cb_size > 0 && (int32_t)rs_shader.m_cb_num_constants <= root_constant_budget)
		{
			rs_shader.m_cb_bind_as_root_constants = true;
			root_constant_budget -= (int32_t)rs_shader.m_cb_num_constants;
			num_total_root_constants += rs_shader.m_cb_num_constants;
			m_has_root_contants = true;
		}
	}

#endif

	if (m_has_root_contants)
	{
		assert(root_argument_budget >= 0);
		assert(root_constant_budget >= 0);
		if (num_total_root_constants > 0)
		{
			m_root_constant_buffer.resize(num_total_root_constants);
		}
	}

	int32_t num_root_constants = 0;
	for (uint32_t k = 0; k < NGL_DX12_NUM_UNIFORM_GROUPS; k++)
	{
		D3D12_uniform_group uniform_group = uniform_group_order[k];

		for (uint32_t j = 0; j < NGL_NUM_SHADER_TYPES; j++)
		{
			NGL_shader_type shader_type = shader_stage_order[j];

			D3D12_root_signature_shader_stage &rs_shader = m_groups[uniform_group].m_stages[shader_type];

			if (rs_shader.m_cb_size > 0)
			{
				if (rs_shader.m_cb_bind_as_root_constants)
				{
					rs_shader.m_root_constant_cpu_address = (uint8_t*)&m_root_constant_buffer[num_root_constants];
					num_root_constants += rs_shader.m_cb_num_constants;
				}
				else
				{
					rs_shader.m_cb_root_index = (uint32_t)root_parameters.size();

					CD3DX12_ROOT_PARAMETER parameter;
					parameter.InitAsConstantBufferView(rs_shader.m_cb_bind_point, 0, GetShaderVisibility(shader_type));
					root_parameters.push_back(parameter);
				}
			}
		}
	}

	uint32_t next_view_range_start = 0;
	uint32_t next_sampler_range_start = 0;

	for (uint32_t k = 0; k < NGL_DX12_NUM_UNIFORM_GROUPS; k++)
	{
		D3D12_uniform_group uniform_group = uniform_group_order[k];

		for (uint32_t j = 0; j < NGL_NUM_SHADER_TYPES; j++)
		{
			NGL_shader_type shader_type = shader_stage_order[j];
			D3D12_SHADER_VISIBILITY visibility = GetShaderVisibility(shader_type);

			D3D12_root_signature_shader_stage &rs_shader = m_groups[uniform_group].m_stages[shader_type];

			rs_shader.m_num_view_bind_heap_entries = (uint32_t)rs_shader.m_view_table.size();
			rs_shader.m_num_sampler_bind_heap_entries = (uint32_t)rs_shader.m_sampler_table.size();

			if (rs_shader.m_num_view_bind_heap_entries > 0)
			{
				CD3DX12_ROOT_PARAMETER parameter;
				parameter.InitAsDescriptorTable(rs_shader.m_num_view_bind_heap_entries, (D3D12_DESCRIPTOR_RANGE*)rs_shader.m_view_table.data(), visibility);
				rs_shader.m_view_root_index = (uint32_t)root_parameters.size();
				root_parameters.push_back(parameter);

				rs_shader.m_view_index_range = D3D12_bind_heap_index_range(next_view_range_start, rs_shader.m_num_view_bind_heap_entries);
				next_view_range_start += rs_shader.m_num_view_bind_heap_entries;
			}

			if (rs_shader.m_num_sampler_bind_heap_entries > 0)
			{
				CD3DX12_ROOT_PARAMETER parameter;
				parameter.InitAsDescriptorTable(rs_shader.m_num_sampler_bind_heap_entries, (D3D12_DESCRIPTOR_RANGE*)rs_shader.m_sampler_table.data(), visibility);
				rs_shader.m_sampler_root_index = (uint32_t)root_parameters.size();
				root_parameters.push_back(parameter);

				rs_shader.m_sampler_index_range = D3D12_bind_heap_index_range(next_sampler_range_start, rs_shader.m_num_sampler_bind_heap_entries);
				next_sampler_range_start += rs_shader.m_num_sampler_bind_heap_entries;
			}
		}
	}

	m_view_indices.resize(next_view_range_start);
	m_sampler_indices.resize(next_sampler_range_start);
	
	for (uint32_t k = 0; k < NGL_DX12_NUM_UNIFORM_GROUPS; k++)
	{
		D3D12_uniform_group uniform_group = uniform_group_order[k];

		for (uint32_t j = 0; j < NGL_NUM_SHADER_TYPES; j++)
		{
			NGL_shader_type shader_type = shader_stage_order[j];
			D3D12_SHADER_VISIBILITY visibility = GetShaderVisibility(shader_type);

			D3D12_root_signature_shader_stage &rs_shader = m_groups[uniform_group].m_stages[shader_type];

			if (rs_shader.m_cb_size > 0 && rs_shader.m_cb_bind_as_root_constants)
			{
				rs_shader.m_cb_root_index = (uint32_t)root_parameters.size();

				CD3DX12_ROOT_PARAMETER parameter;
				parameter.InitAsConstants(rs_shader.m_cb_num_constants, rs_shader.m_cb_bind_point, 0, visibility);
				root_parameters.push_back(parameter);
			}
		}
	}

	// Create RS

	CD3DX12_ROOT_SIGNATURE_DESC signature_desc;
	signature_desc.Init(
		(uint32_t)root_parameters.size(), root_parameters.size() > 0 ? root_parameters.data() : nullptr,
		(UINT)m_static_samplers.size(), m_static_samplers.data(),
		m_flags);


#ifdef NGL_DX12_PRINT_ROOT_SIGNATURE
	PrintRootSignature(signature_desc);

	static const char* shader_stage_names[NGL_NUM_SHADER_TYPES] = {
		"VERTEX_SHADER",
		"FRAGMENT_SHADER",
		"GEOMETRY_SHADER",
		"TESS_CONTROL_SHADER",
		"TESS_EVALUATION_SHADER",
		"COMPUTE_SHADER"
	};

	static const char* uniform_group_names[NGL_DX12_NUM_UNIFORM_GROUPS] = {
		"PER_DRAW",
		"RENDERER_CHANGE"
	};

	for (uint32_t k = 0; k < NGL_DX12_NUM_UNIFORM_GROUPS; k++)
	{
		D3D12_uniform_group uniform_group = uniform_group_order[k];

		for (uint32_t j = 0; j < NGL_NUM_SHADER_TYPES; j++)
		{
			NGL_shader_type shader_type = shader_stage_order[j];

			D3D12_root_signature_shader_stage &rs_shader = m_groups[uniform_group].m_stages[shader_type];

			if (rs_shader.m_cb_size > 0)
			{
				if (rs_shader.m_cb_bind_as_root_constants)
				{
					_logf("constant buffer ROOT size: %d (%d) group: %s stage: %s", rs_shader.m_cb_size, rs_shader.m_cb_num_constants, uniform_group_names[uniform_group], shader_stage_names[shader_type]);
				}
				else
				{
					_logf("constant buffer CBV  size: %d (%d) group: %s stage: %s", rs_shader.m_cb_size, rs_shader.m_cb_num_constants, uniform_group_names[uniform_group], shader_stage_names[shader_type]);
				}
			}
		}
	}
#endif

	{
		ID3DBlob *signature_blob = nullptr;
		ID3DBlob *error_blob = nullptr;
		HRESULT hr = D3D12SerializeRootSignature(&signature_desc, D3D_ROOT_SIGNATURE_VERSION_1, &signature_blob, &error_blob);

		if (error_blob != nullptr)
		{
			std::vector<char> error_string(error_blob->GetBufferSize() + 1, 0);
			memcpy(error_string.data(), error_blob->GetBufferPointer(), error_blob->GetBufferSize());
			_logf("DX12 - ERROR! Serialize root signature: %s", error_string.data());
		}

		ThrowIfFailed(hr);

		if (signature_blob != nullptr)
		{
			ThrowIfFailed(m_renderer->m_backend->m_device->CreateRootSignature(0, signature_blob->GetBufferPointer(), signature_blob->GetBufferSize(), IID_PPV_ARGS(&m_root_signature)));
		}
	}
}


void D3D12_root_signature::ConstantBufferReflection(D3D12_root_signature_shader_stage &rs_shader, D3D12_shader_reflection &reflection, const AppUniformMap &app_uniform_map, const std::vector<NGL_shader_uniform> &application_uniforms)
{
	NGL_shader_type shader_type = rs_shader.m_shader_type;
	NGL_shader_uniform_group ngl_group = GetUsedUniformGroup(rs_shader.m_uniform_group);

	for (size_t j = 0; j < reflection.m_constant_buffers.size(); j++)
	{
		D3D12_shader_constant_buffer &cb = reflection.m_constant_buffers[j];

		// Enumerate variables

		bool has_array = false;
		uint32_t num_used_uniforms = 0;
		for (size_t i = 0; i < cb.m_variables.size(); i++)
		{
			const D3D12_shader_variable &variable = cb.m_variables[i];

			NGL_used_uniform uu;
			uu.m_uniform.m_name = variable.m_desc.Name;

			if (!variable.m_is_active)
			{
				continue;
			}

			if (app_uniform_map.find(&uu.m_uniform.m_name) != app_uniform_map.end())
			{
				if (application_uniforms[app_uniform_map.at(&uu.m_uniform.m_name)].m_group != ngl_group)
				{
					continue;
				}
			}

			if (variable.m_type_desc.Type != D3D_SVT_INT && variable.m_type_desc.Type != D3D_SVT_UINT && variable.m_type_desc.Type != D3D_SVT_FLOAT)
			{
				_logf("DX12 - Unsupported uniform type: %d", (int32_t)variable.m_type_desc.Type);
				assert(false);
				continue;
			}

			uint32_t element_size = 0;
			uint32_t array_size = variable.m_type_desc.Elements ? variable.m_type_desc.Elements : 1;
			
			if (variable.m_type_desc.Elements > 0)
			{
				has_array = true;
			}
			
			GetVariableFormat(variable.m_type_desc, uu.m_uniform.m_format, element_size);
			uu.m_uniform.m_size = element_size * array_size;

			uu.m_shader_location[shader_type] = (int32_t)variable.m_desc.StartOffset;
			uu.m_binding_type = (uint32_t)D3D_SIT_CBUFFER << NGL_DX12_NUM_BINDING_TYPE_FLAGS;
						
			uu.m_application_location = -1;
			if (app_uniform_map.find(&uu.m_uniform.m_name) != app_uniform_map.end())
			{
				uu.m_application_location = app_uniform_map.at(&uu.m_uniform.m_name);
			}
			else
			{
				_logf("DX12 - Not set uniform: %s in %s\n", uu.m_uniform.m_name.c_str(), m_renderer->m_job->m_descriptor.m_subpasses[m_renderer->m_job->m_current_subpass].m_name.c_str());
				continue;
			}

			m_renderer->m_used_uniforms[ngl_group].push_back(uu);
			num_used_uniforms++;
		}
		
		// Add root descriptor

		if (num_used_uniforms > 0)
		{
			rs_shader.m_cb_size = cb.m_size;
			rs_shader.m_cb_num_constants = cb.m_size / 4;
			rs_shader.m_cb_bind_point = cb.m_bind_point;
			rs_shader.m_cb_has_array = has_array;
		}
	}
}


void D3D12_root_signature::DescriptorTableReflection(D3D12_root_signature_shader_stage &rs_shader, D3D12_shader_reflection &reflection, const AppUniformMap &app_uniform_map, const std::vector<NGL_shader_uniform> &application_uniforms)
{
	NGL_shader_type shader_type = rs_shader.m_shader_type;
	NGL_shader_uniform_group ngl_group = GetUsedUniformGroup(rs_shader.m_uniform_group);
	D3D12_SHADER_VISIBILITY visibility = GetShaderVisibility(shader_type);

	D3D12_STATIC_SAMPLER_DESC static_sampler_desc;

	for (size_t k = 0; k < reflection.m_shader_inputs.size(); k++)
	{
		const D3D12_SHADER_INPUT_BIND_DESC &desc = reflection.m_shader_inputs[k];
		NGL_used_uniform uu;
		uu.m_uniform.m_name = desc.Name;
		uu.m_uniform.m_size = 0;
		uu.m_shader_location[(uint32_t)shader_type] = (int32_t)-1;
		uu.m_binding_type = (uint32_t)desc.Type << NGL_DX12_NUM_BINDING_TYPE_FLAGS;

		if (desc.Type == D3D_SIT_SAMPLER)
		{
			uu.m_uniform.m_name = uu.m_uniform.m_name.substr(0, uu.m_uniform.m_name.length() - 15); // erase "__ksl_sampler__"
		}

		if (app_uniform_map.find(&uu.m_uniform.m_name) != app_uniform_map.end())
		{
			if (application_uniforms[app_uniform_map.at(&uu.m_uniform.m_name)].m_group != ngl_group)
			{
				continue;
			}
		}

		// Describe uniform

		switch (desc.Type)
		{
		case D3D_SIT_TEXTURE:
		{
			uu.m_uniform.m_format = NGL_TEXTURE;
			uu.m_shader_location[shader_type] = rs_shader.AddRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, desc.BindPoint);
		}
		break;

		case D3D_SIT_SAMPLER:
		{
			uu.m_uniform.m_format = NGL_TEXTURE;

			if ((desc.uFlags & D3D_SIF_COMPARISON_SAMPLER) > 0)
			{
				uu.m_binding_type += NGL_DX12_BINDING_TYPE_FLAG_IS_SHADOW_SAPLER; // is_shadow_sampler flag

				DescribeStaticSampler(true, desc.BindPoint, visibility, static_sampler_desc);
				m_static_samplers.push_back(static_sampler_desc);
				continue;
			}

			uu.m_shader_location[shader_type] = rs_shader.AddRange(D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER, 1, desc.BindPoint);
		}
		break;

		case D3D_SIT_UAV_RWSTRUCTURED: // Read/write buffer
		{
			uu.m_uniform.m_format = NGL_BUFFER;
			uu.m_shader_location[shader_type] = rs_shader.AddRange(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, desc.BindPoint);
		}
		break;

		case D3D_SIT_STRUCTURED: // Read only buffer
		{
			uu.m_uniform.m_format = NGL_BUFFER;
			uu.m_shader_location[shader_type] = rs_shader.AddRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, desc.BindPoint);
		}
		break;

		case D3D_SIT_UAV_RWTYPED:
		{
			// Currently only RWTexture2D is supported
			if (desc.Dimension != D3D_SRV_DIMENSION_TEXTURE2D)
			{
				_logf("DX12 - Unsupported UAV dimension: %d %s", desc.Type, desc.Name);
				assert(false);
				continue;
			}

			uu.m_uniform.m_format = NGL_TEXTURE;
			uu.m_shader_location[shader_type] = rs_shader.AddRange(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, desc.BindPoint);
		}
		break;

		default:
			_logf("DX12 - Unsupported shader input type: %d %s", desc.Type, desc.Name);
			assert(false);
			continue;
		}

		// Find app location for uniform

		uu.m_application_location = -1;
		if (app_uniform_map.find(&uu.m_uniform.m_name) != app_uniform_map.end())
		{
			uu.m_application_location = app_uniform_map.at(&uu.m_uniform.m_name);

			if (application_uniforms[uu.m_application_location].m_format == NGL_TEXTURE_SUBRESOURCE)
			{
				assert(uu.m_uniform.m_format == NGL_TEXTURE);
				uu.m_binding_type |= NGL_DX12_BINDING_TYPE_FLAG_IS_SUBRESOURCE;
			}

			if (application_uniforms[uu.m_application_location].m_format == NGL_BUFFER_SUBRESOURCE)
			{
				assert(uu.m_uniform.m_format == NGL_BUFFER);
				uu.m_binding_type |= NGL_DX12_BINDING_TYPE_FLAG_IS_SUBRESOURCE;
			}
		}
		else
		{
			_logf("DX12 - Uniform not set: %s in %s", uu.m_uniform.m_name.c_str(), m_renderer->m_job->m_descriptor.m_subpasses[m_renderer->m_job->m_current_subpass].m_name.c_str());
			continue;
		}

		m_renderer->m_used_uniforms[ngl_group].push_back(uu);
	}
}


bool D3D12_root_signature::GetVariableFormat(const D3D12_SHADER_TYPE_DESC &type_desc, NGL_shader_uniform_format &format, uint32_t &element_size)
{
	format = NGL_FLOAT;
	element_size = 0;

	if (type_desc.Class == D3D_SVC_SCALAR)
	{
		format = NGL_FLOAT;
		element_size = 4;
	}
	else if (type_desc.Class == D3D_SVC_VECTOR)
	{
		if (type_desc.Columns == 2)
		{
			format = NGL_FLOAT2;
			element_size = 2 * 4;
		}
		else if (type_desc.Columns == 4)
		{
			format = NGL_FLOAT4;
			element_size = 4 * 4;
		}
		else
		{
			_logf("DX12 - Unsupported vector type: %d", type_desc.Columns);
			assert(false);
			return false;
		}
	}
	else if (type_desc.Class == D3D_SVC_MATRIX_COLUMNS)
	{
		if (type_desc.Columns == 4 && type_desc.Rows == 4)
		{
			format = NGL_FLOAT16;
			element_size = 16 * 4;
		}
		else
		{
			_logf("DX12 - Unsupported matrix type: %dx%d", type_desc.Columns, type_desc.Rows);
			assert(false);
			return false;
		}
	}
	else
	{
		_logf("DX12 - Unsupported uniform class: %d", (int32_t)type_desc.Class);
		assert(false);
		return false;
	}

	return true;
}
