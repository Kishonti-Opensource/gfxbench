/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef NGL_D3D12_BIND_H
#define NGL_D3D12_BIND_H

#include "ngl_d3d12_define.h"
#include <map>
#include <list>
#include <vector>
#include <queue>

struct D3D12_renderer;
struct D3D12_bind_heap;
struct D3D12_root_signature;
struct D3D12_root_signature_shader_stage;
struct D3D12_command_queue;


struct StringMapComparator
{
	bool operator()(const std::string *a, const std::string *b) const
	{
		return *a < *b;
	}
};


typedef std::map<const std::string*, uint32_t, StringMapComparator> AppUniformMap;


enum D3D12_uniform_group
{
	NGL_DX12_UNIFORM_GROUP_PER_DRAW = 0,
	NGL_DX12_UNIFORM_GROUP_PER_RENDERER_CHANGE,
	NGL_DX12_NUM_UNIFORM_GROUPS
};


inline NGL_shader_uniform_group GetUsedUniformGroup(D3D12_uniform_group uniform_group)
{
	switch (uniform_group)
	{
	case NGL_DX12_UNIFORM_GROUP_PER_DRAW:
		return NGL_GROUP_PER_DRAW;

	case NGL_DX12_UNIFORM_GROUP_PER_RENDERER_CHANGE:
		return NGL_GROUP_PER_RENDERER_CHANGE;

	default:
		assert(false);
		return NGL_GROUP_MANUAL;
	}
}


enum D3D12_binding_type_flag
{
	NGL_DX12_BINDING_TYPE_FLAG_IS_SHADOW_SAPLER = 1,
	NGL_DX12_BINDING_TYPE_FLAG_IS_SUBRESOURCE = 2,
	NGL_DX12_NUM_BINDING_TYPE_FLAGS = 2
};


struct D3D12_descriptor_heap
{
	D3D12_descriptor_heap();
	~D3D12_descriptor_heap();
	ID3D12DescriptorHeap* Init(ID3D12Device *device, uint32_t num, D3D12_DESCRIPTOR_HEAP_TYPE type, D3D12_DESCRIPTOR_HEAP_FLAGS flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE);
	CD3DX12_CPU_DESCRIPTOR_HANDLE AddCPUHandle(UINT &offset);
	CD3DX12_CPU_DESCRIPTOR_HANDLE GetCPUHandle(UINT idx);
	CD3DX12_GPU_DESCRIPTOR_HANDLE GetGPUHandle(UINT idx);

	inline void SetName(const std::string &name)
	{
#if NGL_D3D12_ENABLE_DEBUG_LAYER
		assert(m_heap != nullptr);

		m_name = std::wstring(name.begin(), name.end());
		m_heap->SetName(m_name.c_str());
#endif
	}

	uint32_t m_max_num;
	ID3D12DescriptorHeap *m_heap;
	CD3DX12_CPU_DESCRIPTOR_HANDLE m_cpu_handle;
	uint32_t m_descriptor_size;
	uint32_t m_num;
	std::wstring m_name;
};


struct D3D12_bind_heap_range
{
	D3D12_bind_heap_range()
	{
		m_start_index = 0;
		m_num = 0;
	}

	uint32_t m_start_index;
	uint32_t m_num;
};


struct D3D12_bind_heap_block
{
	D3D12_bind_heap_block()
	{
		m_fence_value = NGL_DX12_FENCE_VALUE_UNSET;
	}

	D3D12_bind_heap_range m_range;
	uint64_t m_fence_value;
};


struct D3D12_descriptor_copy_batch
{
	inline void Clear()
	{
		m_dst.clear();
		m_src.clear();
	}

	inline void Add(D3D12_CPU_DESCRIPTOR_HANDLE &dst, D3D12_CPU_DESCRIPTOR_HANDLE &src)
	{
		m_dst.push_back(dst);
		m_src.push_back(src);
	}

	inline bool HasItems()
	{
		return m_dst.size() > 0 || m_src.size() > 0;
	}

	std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> m_dst;
	std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> m_src;
};


struct D3D12_view_bind_heap
{
	D3D12_view_bind_heap();
	~D3D12_view_bind_heap();
	void Init(D3D12_descriptor_heap *source_heap, D3D12_command_queue *command_queue, uint32_t heap_size);

	void BeginBlock();
	void RequestRange(const D3D12_root_signature &rs, D3D12_root_signature_shader_stage &rs_shader, D3D12_descriptor_copy_batch &copy_batch);
	void EndBlock(uint64_t fence_value);

	inline void SetName(const std::string &name)
	{
#if NGL_D3D12_ENABLE_DEBUG_LAYER
		m_heap.SetName(name);
		m_name = name;
#endif
	}

	bool m_has_active_block;
	D3D12_bind_heap_block m_active_block;
	std::queue<D3D12_bind_heap_block> m_active_blocks;

	std::string m_name;
	uint32_t m_heap_size;
	D3D12_descriptor_heap *m_source_heap;
	D3D12_command_queue *m_command_queue;
	D3D12_descriptor_heap m_heap;
	std::queue<D3D12_bind_heap_block> m_discarded_blocks;
	uint32_t m_next_start_index;
	uint32_t m_last_available_index;
	uint32_t m_max_used_index;
};


struct D3D12_sampler_bind_heap
{
	D3D12_sampler_bind_heap();
	~D3D12_sampler_bind_heap();
	void Init(D3D12_descriptor_heap *source_heap, D3D12_command_queue *command_queue, uint32_t heap_size);

	void RequestRange(const D3D12_root_signature &rs, D3D12_root_signature_shader_stage &rs_shader, D3D12_descriptor_copy_batch &copy_batch);

	inline void SetName(const std::string &name)
	{
#if NGL_D3D12_ENABLE_DEBUG_LAYER
		m_heap.SetName(name);
		m_name = name;
#endif
	}

	std::string m_name;
	uint32_t m_heap_size;
	D3D12_descriptor_heap *m_source_heap;
	D3D12_command_queue *m_command_queue;
	D3D12_descriptor_heap m_heap;
	std::map<uint64_t, D3D12_bind_heap_range> m_ranges;
	uint32_t m_next_start_index;
};


struct D3D12_bind_heap_index_range
{
	D3D12_bind_heap_index_range()
	{
		m_start_index = 0;
		m_num = 0;
	}

	D3D12_bind_heap_index_range(uint32_t start_index, uint32_t num)
	{
		m_start_index = start_index;
		m_num = num;
	}

	uint32_t m_start_index;
	uint32_t m_num;
};


struct D3D12_shader_variable
{
	D3D12_shader_variable(const D3D12_SHADER_VARIABLE_DESC &desc, const D3D12_SHADER_TYPE_DESC &type_desc)
	{
		m_desc = desc;
		m_type_desc = type_desc;
		m_is_active = (desc.uFlags & D3D_SVF_USED) > 0;
	}

	D3D12_SHADER_VARIABLE_DESC m_desc;
	D3D12_SHADER_TYPE_DESC m_type_desc;
	bool m_is_active;
};


struct D3D12_shader_constant_buffer
{
	D3D12_shader_constant_buffer()
	{
		m_size = 0;
		m_bind_point = 0;
	}

	uint32_t m_size;
	uint32_t m_bind_point;
	std::string m_name;
	std::vector<D3D12_shader_variable> m_variables;
};


struct D3D12_shader_reflection
{
	std::vector<D3D12_shader_constant_buffer> m_constant_buffers;
	std::vector<D3D12_SHADER_INPUT_BIND_DESC> m_shader_inputs;

	ID3D12ShaderReflection *m_d3d12_reflection;

	D3D12_shader_reflection();
	~D3D12_shader_reflection();
	void Init(NGL_shader_type shader_type, ID3DBlob *shader, const AppUniformMap &app_uniform_map);
	void ReleaseD3D12Resources();
	//bool FindShaderInput(const char *name, D3D12_SHADER_INPUT_BIND_DESC &desc);
};


struct D3D12_root_signature_shader_stage
{
	D3D12_root_signature_shader_stage()
	{
		m_cb_root_index = 0;
		m_cb_size = 0;
		m_cb_num_constants = 0;
		m_cb_bind_point = 0;
		m_cb_bind_as_root_constants = false;
		m_root_constant_cpu_address = nullptr;
		m_cb_has_array = false;
		m_view_root_index = 0;
		m_sampler_root_index = 0;
		m_num_view_bind_heap_entries = 0;
		m_num_sampler_bind_heap_entries = 0;
		m_shader_type = (NGL_shader_type)-1;
		m_uniform_group = (D3D12_uniform_group)-1;
		m_view_gpu_handle = CD3DX12_GPU_DESCRIPTOR_HANDLE();
		m_sampler_gpu_handle = CD3DX12_GPU_DESCRIPTOR_HANDLE();
	}

	inline int32_t AddRange(D3D12_DESCRIPTOR_RANGE_TYPE range_type, uint32_t num, uint32_t base_register)
	{
		switch (range_type)
		{
		case D3D12_DESCRIPTOR_RANGE_TYPE_SRV:
		case D3D12_DESCRIPTOR_RANGE_TYPE_UAV:
			m_view_table.push_back(CD3DX12_DESCRIPTOR_RANGE(range_type, num, base_register, 0, (UINT)m_view_table.size()));
			return (int32_t)m_view_table.size() - 1;

		case D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER:
			m_sampler_table.push_back(CD3DX12_DESCRIPTOR_RANGE(range_type, num, base_register, 0, (UINT)m_sampler_table.size()));
			return (int32_t)m_sampler_table.size() - 1;

		default:
			assert(false);
			return -1;
		}
	}

	void Init(D3D12_uniform_group uniform_group, NGL_shader_type shader_type);
	
	D3D12_uniform_group m_uniform_group;
	NGL_shader_type m_shader_type;

	std::vector<CD3DX12_ROOT_PARAMETER> m_root_descriptors;
	std::vector<CD3DX12_DESCRIPTOR_RANGE> m_view_table;
	std::vector<CD3DX12_DESCRIPTOR_RANGE> m_sampler_table;

	uint32_t m_cb_root_index;
	uint32_t m_cb_size;
	uint32_t m_cb_num_constants;
	uint32_t m_cb_bind_point;
	bool m_cb_bind_as_root_constants;
	uint8_t *m_root_constant_cpu_address;
	bool m_cb_has_array;

	uint32_t m_view_root_index;
	uint32_t m_sampler_root_index;
	uint32_t m_num_view_bind_heap_entries;
	uint32_t m_num_sampler_bind_heap_entries;

	//D3D12_bind_heap_range m_view_range;
	//D3D12_bind_heap_range m_sampler_range;
	D3D12_GPU_DESCRIPTOR_HANDLE m_view_gpu_handle;
	D3D12_GPU_DESCRIPTOR_HANDLE m_sampler_gpu_handle;
	D3D12_bind_heap_index_range m_view_index_range;
	D3D12_bind_heap_index_range m_sampler_index_range;
};


struct D3D12_root_signature
{
	D3D12_root_signature()
	{
		m_renderer = nullptr;
		m_root_signature = nullptr;
		m_flags = D3D12_ROOT_SIGNATURE_FLAG_NONE;
		m_has_root_contants = false;
	}

	~D3D12_root_signature()
	{
		SAFE_RELEASE(m_root_signature);
	}

	inline void SetViewIndex(uint32_t uniform_group, NGL_shader_type shader_type, int32_t locations[NGL_NUM_SHADER_TYPES], int32_t index)
	{
		m_view_indices[m_groups[uniform_group].m_stages[shader_type].m_view_index_range.m_start_index + locations[shader_type]] = index;
	}

	inline void SetSamplerIndex(uint32_t uniform_group, NGL_shader_type shader_type, int32_t locations[NGL_NUM_SHADER_TYPES], int32_t index)
	{
		m_sampler_indices[m_groups[uniform_group].m_stages[shader_type].m_sampler_index_range.m_start_index + locations[shader_type]] = index;
	}

	void Init(D3D12_renderer *renderer, const std::vector<NGL_shader_uniform> &application_uniforms);
	void ConstantBufferReflection(D3D12_root_signature_shader_stage &rs_shader, D3D12_shader_reflection &reflection, const AppUniformMap &app_uniform_map, const std::vector<NGL_shader_uniform> &application_uniforms);
	void DescriptorTableReflection(D3D12_root_signature_shader_stage &rs_shader, D3D12_shader_reflection &reflection, const AppUniformMap &app_uniform_map, const std::vector<NGL_shader_uniform> &application_uniforms);
	bool GetVariableFormat(const D3D12_SHADER_TYPE_DESC &type_desc, NGL_shader_uniform_format &format, uint32_t &element_size);

	D3D12_renderer *m_renderer;
	D3D12_shader_reflection m_reflections[NGL_NUM_SHADER_TYPES];
	ID3D12RootSignature *m_root_signature;
	std::vector<D3D12_STATIC_SAMPLER_DESC> m_static_samplers;
	bool m_has_root_contants;
	std::vector<uint32_t> m_root_constant_buffer;

	D3D12_ROOT_SIGNATURE_FLAGS m_flags;
	struct {
		D3D12_root_signature_shader_stage m_stages[NGL_NUM_SHADER_TYPES];
	} m_groups[NGL_DX12_NUM_UNIFORM_GROUPS];
	std::vector<uint32_t> m_view_indices;
	std::vector<uint32_t> m_sampler_indices;
};

#endif
