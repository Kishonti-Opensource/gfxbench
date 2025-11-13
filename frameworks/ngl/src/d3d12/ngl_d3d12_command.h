/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef NGL_D3D12_COMMAND_H
#define NGL_D3D12_COMMAND_H

#include "ngl_d3d12_define.h"
#include "ngl_d3d12_resource.h"
#include "ngl_d3d12_bind.h"
#include <vector>
#include <list>
#include <queue>

class D3D12_backend;
struct D3D12_allocator_manager;
struct D3D12_command_allocator;
struct D3D12_command_queue;
struct D3D12_command_context;
struct D3D12_memory_manager;
struct D3D12_resource;
struct D3D12_job;


struct TextureSubresourceKey
{
	TextureSubresourceKey()
	{
		m_texture_id = 0;
		m_subresource_index = 0;
	}

	TextureSubresourceKey(uint32_t texture_id, uint32_t subresource_index)
	{
		m_texture_id = texture_id;
		m_subresource_index = subresource_index;
	}

	uint32_t m_texture_id;
	uint32_t m_subresource_index;
};


struct TextureSubresourceKeyMapComparator
{
	bool operator()(TextureSubresourceKey a, TextureSubresourceKey b) const
	{
		return a.m_texture_id < b.m_texture_id || (a.m_texture_id == b.m_texture_id && a.m_subresource_index < b.m_subresource_index);
	}
};

typedef std::map<TextureSubresourceKey, D3D12_RESOURCE_STATES, TextureSubresourceKeyMapComparator> TextureSubresourceStateMap;
typedef std::map<uint32_t, D3D12_RESOURCE_STATES> BufferStateMap;


struct D3D12_command_allocator
{
	D3D12_command_allocator(ID3D12CommandAllocator *allocator)
	{
		m_allocator = allocator;
		m_reset_fence_value = NGL_DX12_FENCE_VALUE_UNSET;
	}

	ID3D12CommandAllocator *m_allocator;
	uint64_t m_reset_fence_value;
};


struct D3D12_allocator_manager
{
	D3D12_allocator_manager(ID3D12Device *device, D3D12_command_queue *command_queue);
	~D3D12_allocator_manager();

	D3D12_command_allocator* RequestAllocator();
	void DiscardAllocator(D3D12_command_allocator *command_allocator, uint64_t fence_value);
	void ReleaseAll();

private:
	ID3D12Device *m_device;
	D3D12_command_queue *m_command_queue;
	std::queue<D3D12_command_allocator> m_allocators;
	std::queue<D3D12_command_allocator*> m_discarded_allocators;
};


struct D3D12_command_context
{
	D3D12_command_context(D3D12_backend *backend, D3D12_command_queue *command_queue, bool has_memory_manager);
	virtual ~D3D12_command_context();
	void SetName(const std::string &name);
	void Reset();
	void Close();
	uint64_t Execute();
	uint64_t CloseExecute();
	void Discard(uint64_t fence_value);
#ifdef NGL_DX12_DEBUG_TRANSITIONS
	void TrackTextureState(D3D12_texture &texture, D3D12_RESOURCE_STATES state);
	void TrackBufferState(D3D12_vertex_buffer &vb, D3D12_RESOURCE_STATES state);
	void UpdateTrackedTextureState(D3D12_texture &texture, uint32_t subresource_index, D3D12_RESOURCE_BARRIER &barrier);
	void UpdateTrackedBufferState(D3D12_vertex_buffer &vb, D3D12_RESOURCE_BARRIER &barrier);
	void VerifyTextureState(D3D12_texture &texture, uint32_t subresource_index, D3D12_RESOURCE_STATES state, bool all_subresources = false);
	void VerifyBufferState(D3D12_vertex_buffer &vb, D3D12_RESOURCE_STATES state);
#else
	inline void TrackTextureState(D3D12_texture &texture, D3D12_RESOURCE_STATES state) { }
	inline void TrackBufferState(D3D12_vertex_buffer &vb, D3D12_RESOURCE_STATES state) { }
	inline void UpdateTrackedTextureState(D3D12_texture &texture, uint32_t subresource_index, D3D12_RESOURCE_BARRIER &barrier) { }
	inline void UpdateTrackedBufferState(D3D12_vertex_buffer &vb, D3D12_RESOURCE_BARRIER &barrier) { }
	inline void VerifyTextureState(D3D12_texture &texture, uint32_t subresource_index, D3D12_RESOURCE_STATES state, bool all_subresources = false) { }
	inline void VerifyBufferState(D3D12_vertex_buffer &vb, D3D12_RESOURCE_STATES state) { }
#endif
	void AddTextureBarrier(D3D12_texture *texture, D3D12_RESOURCE_STATES &current_state, D3D12_RESOURCE_STATES new_state);
	void AddResourceBarrier(D3D12_resource *resource, uint32_t subresource_index, D3D12_RESOURCE_STATES &current_state, D3D12_RESOURCE_STATES new_state);
	void AddNGLBarrierList(std::vector<NGL_texture_subresource_transition> &texture_barriers, std::vector<NGL_buffer_transition> &buffer_barriers);
	void AddResourceBarrierList(const std::vector<D3D12_RESOURCE_BARRIER> barrier_list);

	D3D12_backend *m_backend;
	D3D12_command_queue *m_command_queue;
	D3D12_allocator_manager *m_allocator_mgr;
	D3D12_memory_manager *m_memory_mgr;
	D3D12_command_allocator *m_command_allocator;
	ID3D12GraphicsCommandList *m_command_list;

	TextureSubresourceStateMap m_first_texture_states; // first requersted states
	BufferStateMap m_first_buffer_states; // first requersted states
	TextureSubresourceStateMap m_texture_states;
	BufferStateMap m_buffer_states;

	bool m_is_recordable;
	bool m_need_to_discard; 
	bool m_is_using_system_color_attachment;

	uint64_t m_fence_value;
	std::vector<D3D12_RESOURCE_BARRIER> m_barrier_batch;
	D3D12_descriptor_copy_batch m_view_copy_batch;
	D3D12_descriptor_copy_batch m_sampler_copy_batch;
	
	std::wstring m_name;
};


struct D3D12_command_queue
{
	D3D12_command_queue(D3D12_backend *backend, D3D12_COMMAND_LIST_TYPE queue_type);
	~D3D12_command_queue();
	D3D12_command_context *CreateCommandContext(bool has_memory_manager);
	uint64_t ExecuteCommandContext(D3D12_command_context *command_context);
	void WaitForCompletion(uint64_t fence_value);
#ifdef NGL_DX12_DEBUG_TRANSITIONS
	void UpdateTrackedResourceStates(D3D12_command_context *command_context);
#else
	inline void UpdateTrackedResourceStates(D3D12_command_context *command_context) { }
#endif
	bool IsFenceComplete(uint64_t fence_value);
	void Finish();
	void Discard(uint64_t fence_value);
	void DeleteContext(D3D12_command_context *command_context);

	D3D12_backend *m_backend;
	ID3D12CommandQueue *m_command_queue;

	uint64_t m_last_signaled_fence_value;
	uint64_t m_current_fence_value;
	ID3D12Fence *m_fence;
	HANDLE m_ready_event;

	std::list<D3D12_command_context *> m_command_contexts;
	//std::vector<D3D12_RESOURCE_BARRIER> m_barrier_batch;

	TextureSubresourceStateMap m_texture_states;
	BufferStateMap m_buffer_states;
};

#endif
