/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "ngl_d3d12_command.h"

#include "ngl_d3d12.h"
#include "ngl_d3d12_memory.h"
#include "ngl_d3d12_resource.h"
#include "ngl_d3d12_job.h"
#include "ngl_d3d12_util.h"
#include <cassert>


// D3D12_allocator_manager ///////////////////////////////// 

D3D12_allocator_manager::D3D12_allocator_manager(ID3D12Device *device, D3D12_command_queue *command_queue)
{
	m_device = device;
	m_command_queue = command_queue;
}


D3D12_allocator_manager::~D3D12_allocator_manager()
{
	while (!m_allocators.empty())
	{
		m_allocators.front().m_allocator->Release();
		m_allocators.pop();
	}
}


D3D12_command_allocator *D3D12_allocator_manager::RequestAllocator()
{
	// Check if there are any discarded allocators

	if (!m_discarded_allocators.empty())
	{
		D3D12_command_allocator *discarded_allocator = m_discarded_allocators.front();
		if (m_command_queue->IsFenceComplete(discarded_allocator->m_reset_fence_value))
		{
			ThrowIfFailed(discarded_allocator->m_allocator->Reset());
			discarded_allocator->m_reset_fence_value = NGL_DX12_FENCE_VALUE_UNSET;
			m_discarded_allocators.pop();

			return discarded_allocator;
		}
	}

	// If no allocators were ready to be reused, create a new one

	ID3D12CommandAllocator *new_allocator = nullptr;
	ThrowIfFailed(m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&new_allocator)));
	m_allocators.push(D3D12_command_allocator(new_allocator));
	return &m_allocators.back();
}


void D3D12_allocator_manager::DiscardAllocator(D3D12_command_allocator *command_allocator, uint64_t fence_value)
{
	assert(fence_value != NGL_DX12_FENCE_VALUE_UNSET);
	command_allocator->m_reset_fence_value = fence_value;
	m_discarded_allocators.push(command_allocator);
}


void D3D12_allocator_manager::ReleaseAll()
{
	while (!m_discarded_allocators.empty())
	{
		m_discarded_allocators.pop();
	}

	while (!m_allocators.empty())
	{
		m_allocators.front().m_allocator->Release();
		m_allocators.pop();
	}
}


// D3D12_command_context ////////////////////////////////////////////////////////////////////////

D3D12_command_context::D3D12_command_context(D3D12_backend *backend, D3D12_command_queue *command_queue, bool has_memory_manager)
{
	m_backend = backend;
	m_command_queue = command_queue;
	m_allocator_mgr = new D3D12_allocator_manager(backend->m_device, command_queue);
	m_memory_mgr = nullptr;
	m_command_allocator = nullptr;
	m_command_list = nullptr;

	m_is_recordable = false;
	m_need_to_discard = false;
	m_is_using_system_color_attachment = false;

	m_fence_value = NGL_DX12_FENCE_VALUE_UNSET;

	if (has_memory_manager)
	{
		m_memory_mgr = new D3D12_memory_manager(backend);
		m_memory_mgr->Init();
	}
	
	m_view_copy_batch.m_dst.reserve(4096);
	m_view_copy_batch.m_src.reserve(4096);
	m_sampler_copy_batch.m_dst.reserve(64);
	m_sampler_copy_batch.m_src.reserve(64);
}


D3D12_command_context::~D3D12_command_context()
{
	delete m_memory_mgr;
	delete m_allocator_mgr;

	SAFE_RELEASE(m_command_list);
}

void D3D12_command_context::SetName(const std::string &name)
{
	m_name = std::wstring(name.begin(), name.end());

	if (m_command_list != nullptr)
	{
		m_command_list->SetName(m_name.c_str());
	}
}


void D3D12_command_context::Reset()
{
	if (m_fence_value != NGL_DX12_FENCE_VALUE_UNSET)
	{
		m_command_queue->WaitForCompletion(m_fence_value);
		m_fence_value = NGL_DX12_FENCE_VALUE_UNSET;
	}

	if (m_command_allocator == nullptr)
	{
		m_command_allocator = m_allocator_mgr->RequestAllocator();
	}

	if (m_command_list == nullptr)
	{
		ThrowIfFailed(m_backend->m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_command_allocator->m_allocator, nullptr, IID_PPV_ARGS(&m_command_list)));
		m_command_list->SetName(m_name.c_str());
	}
	else
	{
		assert(!m_is_recordable);
		ThrowIfFailed(m_command_list->Reset(m_command_allocator->m_allocator, nullptr));
	}

	m_need_to_discard = true;
	m_is_recordable = true;
	m_is_using_system_color_attachment = false;

#ifdef NGL_DX12_DEBUG_TRANSITIONS
	m_first_texture_states.clear();
	m_first_buffer_states.clear();
	m_texture_states.clear();
	m_buffer_states.clear();
#endif
	
	m_view_copy_batch.Clear();
	m_sampler_copy_batch.Clear();

	// Set descriptor heaps for command list

	ID3D12DescriptorHeap* ppDescriptorHeaps[2] =
	{
		m_backend->m_resource_mgr->m_view_bind_heap.m_heap.m_heap,
		m_backend->m_resource_mgr->m_sampler_bind_heap.m_heap.m_heap
	};

	m_command_list->SetDescriptorHeaps(2, ppDescriptorHeaps);
}


void D3D12_command_context::Close()
{
	assert(m_command_list != nullptr);
	assert(m_command_allocator != nullptr);
	assert(m_is_recordable);

	if (m_is_using_system_color_attachment)
	{
		assert(m_backend->m_swap_chain->GetCurrentSwapBuffer()->m_current_state == D3D12_RESOURCE_STATE_RENDER_TARGET);
		AddResourceBarrier(m_backend->m_swap_chain->GetCurrentSwapBuffer(), 0, m_backend->m_swap_chain->GetCurrentSwapBuffer()->m_current_state, D3D12_RESOURCE_STATE_PRESENT);
		m_is_using_system_color_attachment = false;
	}

	ThrowIfFailed(m_command_list->Close());
	m_is_recordable = false;
}


uint64_t D3D12_command_context::Execute()
{
	assert(m_command_list != nullptr);
	assert(m_command_allocator != nullptr);
	assert(!m_is_recordable);

	// Copy descriptors

	if (m_view_copy_batch.HasItems())
	{
		m_backend->m_device->CopyDescriptors(
			(UINT)m_view_copy_batch.m_dst.size(), m_view_copy_batch.m_dst.data(), nullptr,
			(UINT)m_view_copy_batch.m_src.size(), m_view_copy_batch.m_src.data(), nullptr,
			D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

		//_logf("View copy batch size: %d", (uint32_t)m_view_copy_batch.m_dst.size());
	}

	if (m_sampler_copy_batch.HasItems())
	{
		m_backend->m_device->CopyDescriptors(
			(UINT)m_sampler_copy_batch.m_dst.size(), m_sampler_copy_batch.m_dst.data(), nullptr,
			(UINT)m_sampler_copy_batch.m_src.size(), m_sampler_copy_batch.m_src.data(), nullptr,
			D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);

		//_logf("Sampler copy batch size: %d", (uint32_t)m_sampler_copy_batch.m_dst.size());
	}

	// Execute command list

	uint64_t fence_value = m_command_queue->ExecuteCommandContext(this);
	return fence_value;
}


uint64_t D3D12_command_context::CloseExecute()
{
	Close();
	return Execute();
}


void D3D12_command_context::Discard(uint64_t fence_value)
{
	if (m_need_to_discard)
	{
		assert(m_command_list != nullptr);
		assert(m_command_allocator != nullptr);
		assert(m_is_recordable == false);

		m_allocator_mgr->DiscardAllocator(m_command_allocator, fence_value);
		if (m_memory_mgr != nullptr)
		{
			m_memory_mgr->DiscardPages(fence_value);
		}

		m_fence_value = fence_value;

		m_command_allocator = nullptr;
		m_need_to_discard = false;
	}
}


#ifdef NGL_DX12_DEBUG_TRANSITIONS
void D3D12_command_context::TrackTextureState(D3D12_texture &texture, D3D12_RESOURCE_STATES state)
{
	for (uint32_t i = 0; i < texture.m_num_subresources; i++)
	{
		TextureSubresourceKey key = TextureSubresourceKey(texture.m_ngl_id, i);
		assert(m_texture_states.find(key) == m_texture_states.end());
		m_first_texture_states[key] = state;
		m_texture_states[key] = state;
	}
}


void D3D12_command_context::TrackBufferState(D3D12_vertex_buffer &vb, D3D12_RESOURCE_STATES state)
{
	uint32_t id = vb.m_ngl_id;
	assert(m_buffer_states.find(id) == m_buffer_states.end());
	m_first_buffer_states[id] = state;
	m_buffer_states[id] = state;
}


void D3D12_command_context::UpdateTrackedTextureState(D3D12_texture &texture, uint32_t subresource_index, D3D12_RESOURCE_BARRIER &barrier)
{
	if (barrier.Type == D3D12_RESOURCE_BARRIER_TYPE_TRANSITION)
	{
		TextureSubresourceKey key = TextureSubresourceKey(texture.m_ngl_id, subresource_index);

		if (m_texture_states.find(key) == m_texture_states.end())
		{
			m_first_texture_states[key] = barrier.Transition.StateAfter;
		}
		else
		{
			if (m_texture_states.at(key) != barrier.Transition.StateBefore)
			{
				_logf("Before state does not match current tracked texture state (res: %p sub: %d ngl_id: %d ngl_name: \"%s\").\n  Before state: %s\n  Current state: %s",
					(void*)texture.m_res, subresource_index, texture.m_ngl_id, texture.m_ngl_texture.m_texture_descriptor.m_name.c_str(),
					GetResourceStateList(barrier.Transition.StateBefore).c_str(),
					GetResourceStateList(m_texture_states.at(key)).c_str());
				assert(false);
			}
		}
		
		//if (((barrier.Transition.StateBefore & D3D12_RESOURCE_STATE_DEPTH_WRITE) > 0 || (barrier.Transition.StateBefore & D3D12_RESOURCE_STATE_DEPTH_READ) > 0)
		//	&& ((barrier.Transition.StateAfter & D3D12_RESOURCE_STATE_DEPTH_WRITE) == 0 && (barrier.Transition.StateAfter & D3D12_RESOURCE_STATE_DEPTH_READ) == 0))
		//{
		//	_logf("Transition from a depth state to a non depth state (res: %p sub: %d ngl_id: %d ngl_name: \"%s\").\n  Before state: %s\n  After state: %s",
		//		(void*)texture.m_res, subresource_index, texture.m_ngl_id, texture.m_ngl_texture.m_texture_descriptor.m_name.c_str(),
		//		GetResourceStateList(barrier.Transition.StateBefore).c_str(),
		//		GetResourceStateList(barrier.Transition.StateAfter).c_str());
		//	assert(false);
		//}

		m_texture_states[key] = barrier.Transition.StateAfter;
	}
}


void D3D12_command_context::UpdateTrackedBufferState(D3D12_vertex_buffer &vb, D3D12_RESOURCE_BARRIER &barrier)
{
	if (barrier.Type == D3D12_RESOURCE_BARRIER_TYPE_TRANSITION)
	{
		uint32_t id = vb.m_ngl_id;

		if (m_buffer_states.find(id) == m_buffer_states.end())
		{
			m_first_buffer_states[id] = barrier.Transition.StateAfter;
		}
		else
		{
			if (m_buffer_states.at(id) != barrier.Transition.StateBefore)
			{
				_logf("Barrier before state does not match current tracked buffer state (res: %p ngl_id: %d).\n  Barrier state: %s\n  Current state: %s",
					(void*)vb.m_res, vb.m_ngl_id,
					GetResourceStateList(barrier.Transition.StateBefore).c_str(),
					GetResourceStateList(m_buffer_states.at(id)).c_str());
				assert(false);
			}
		}

		m_buffer_states[id] = barrier.Transition.StateAfter;
	}
}


void D3D12_command_context::VerifyTextureState(D3D12_texture &texture, uint32_t subresource_index, D3D12_RESOURCE_STATES state, bool all_subresources)
{
	assert(m_is_recordable);
	uint32_t id = texture.m_ngl_id;

	uint32_t first_subresource = all_subresources ? 0 : subresource_index;
	uint32_t last_subresource = all_subresources ? texture.m_num_subresources : subresource_index;
	for (uint32_t i = first_subresource; i <= last_subresource; i++)
	{
		TextureSubresourceKey key = TextureSubresourceKey(id, i);

		if (m_texture_states.find(key) == m_texture_states.end())
		{
			return;
		}

		if (id != NGL_DX12_SYSTEM_ATTACHMENT)
		{
			D3D12_RESOURCE_STATES current_state = m_texture_states.at(key);
			bool current_state_is_valid = IsValidState(current_state);
			bool requested_state_is_valid = IsValidState(state);
			assert(current_state_is_valid && requested_state_is_valid);
			if (current_state != state && (current_state & state) == 0)
			{
				_logf("The requested texture state is missing (res: %p sub: %d ngl_id: %d ngl_name: \"%s\").\n  Requested state: %s\n  Current state: %s",
					(void*)texture.m_res, i, texture.m_ngl_id, texture.m_ngl_texture.m_texture_descriptor.m_name.c_str(),
					GetResourceStateList(state).c_str(),
					GetResourceStateList(current_state).c_str());
				assert(false);
			}
		}
	}
}


void D3D12_command_context::VerifyBufferState(D3D12_vertex_buffer &vb, D3D12_RESOURCE_STATES state)
{
	assert(m_is_recordable);
	uint32_t id = vb.m_ngl_id;

	if (m_buffer_states.find(id) == m_buffer_states.end())
	{
		return;
	}

	assert(id != NGL_DX12_SYSTEM_ATTACHMENT);
	D3D12_RESOURCE_STATES current_state = m_buffer_states.at(id);
	bool current_state_is_valid = IsValidState(current_state);
	bool requested_state_is_valid = IsValidState(state);
	assert(current_state_is_valid && requested_state_is_valid);
	if (current_state != state && (current_state & state) == 0)
	{
		_logf("The requested buffer state is missing (res: %p , sub: %d).\n  Requested state: %s\n  Current state: %s",
			(void*)vb.m_res, 0,
			GetResourceStateList(state).c_str(),
			GetResourceStateList(current_state).c_str());
		assert(false);
	}
}
#endif


#if 0
void D3D12_command_context::RequestResourceState(uint32_t id, D3D12_RESOURCE_STATES state)
{
	assert(m_is_recordable);
	assert(state != NGL_DX12_RESOURCE_STATE_UNKNOWN); // cannot set to unknown state

	if (id != (uint32_t)-1)
	{
		D3D12_resource_state_transition &resource_state = m_resource_states[id];

		if (resource_state.m_before == NGL_DX12_RESOURCE_STATE_UNKNOWN)
		{
			resource_state.m_before = state;
			resource_state.m_after = state;
		}
		else
		{
			if (resource_state.m_after != state)
			{
				if ((resource_state.m_after & D3D12_RESOURCE_STATE_DEPTH_READ) > 0 && state != D3D12_RESOURCE_STATE_DEPTH_WRITE)
				{
					if (state == D3D12_RESOURCE_STATE_DEPTH_READ)
					{
						return;
					}

					state = D3D12_RESOURCE_STATE_DEPTH_READ | state;

					if (resource_state.m_after == state)
					{
						return;
					}
				}

				AddResourceBarrier(m_backend->m_resource_mgr->m_tracked_resources[id], resource_state.m_after, state);
				resource_state.m_after = state;
			}
		}
	}
}
#endif


void D3D12_command_context::AddTextureBarrier(D3D12_texture *texture, D3D12_RESOURCE_STATES &current_state, D3D12_RESOURCE_STATES new_state)
{
	assert(m_is_recordable);

	if (texture == nullptr)
	{
		_logf("DX12: AddTextureBarrier: texture is nullptr!\n");
		assert(false);
	}

	if (current_state == new_state)
	{
		return;
	}

	D3D12_RESOURCE_BARRIER barrier = {};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Transition.pResource = texture->m_res;
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	barrier.Transition.StateBefore = current_state;
	barrier.Transition.StateAfter = new_state;

#ifdef NGL_DX12_PRINT_TRANSITIONS
	PrintTransitionBarrier(barrier);
#endif

	m_command_list->ResourceBarrier(1, &barrier);
	current_state = new_state; // track new state in caller

#ifdef NGL_DX12_DEBUG_TRANSITIONS
	for (uint32_t subresource_index = 0; subresource_index < texture->m_num_subresources; subresource_index++)
	{
		if (barrier.Type == D3D12_RESOURCE_BARRIER_TYPE_TRANSITION)
		{
			TextureSubresourceKey key = TextureSubresourceKey(texture->m_ngl_id, subresource_index);
			if (m_texture_states.find(key) != m_texture_states.end())
			{
				assert(m_texture_states.at(key) == barrier.Transition.StateBefore);
			}

			m_texture_states[key] = barrier.Transition.StateAfter;
		}
	}
#endif
}


void D3D12_command_context::AddResourceBarrier(D3D12_resource *resource, uint32_t subresource_index, D3D12_RESOURCE_STATES &current_state, D3D12_RESOURCE_STATES new_state)
{
	assert(m_is_recordable);

	if (resource == nullptr)
	{
		_logf("DX12: AddResourceBarrier: resource is nullptr!\n");
		assert(false);
	}

	if (current_state == new_state)
	{
		return;
	}

	D3D12_RESOURCE_BARRIER barrier = {};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Transition.pResource = resource->m_res;
	barrier.Transition.Subresource = subresource_index;// D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	barrier.Transition.StateBefore = current_state;
	barrier.Transition.StateAfter = new_state;

#ifdef NGL_DX12_PRINT_TRANSITIONS
	PrintTransitionBarrier(barrier);
#endif

	m_command_list->ResourceBarrier(1, &barrier);
	current_state = new_state; // track new state in caller

#ifdef NGL_DX12_DEBUG_TRANSITIONS
	if (barrier.Type == D3D12_RESOURCE_BARRIER_TYPE_TRANSITION)
	{
		switch (resource->m_resource_type)
		{
		case NGL_D3D12_RESOURCE_TYPE_TEXTURE:
		{
			TextureSubresourceKey key = TextureSubresourceKey(resource->m_ngl_id, subresource_index);
			if (m_texture_states.find(key) != m_texture_states.end())
			{
				assert(m_texture_states.at(key) == barrier.Transition.StateBefore);
			}

			m_texture_states[key] = barrier.Transition.StateAfter;
		}
		break;

		case NGL_D3D12_RESOURCE_TYPE_VERTEX_BUFFER:
		{
			uint32_t id = resource->m_ngl_id;
			if (m_buffer_states.find(id) != m_buffer_states.end())
			{
				assert(m_buffer_states.at(id) == barrier.Transition.StateBefore);
			}

			m_buffer_states[id] = barrier.Transition.StateAfter;
		}
		break;
		}
	}
#endif
}


struct D3D12_texture_barrier_merger
{
	struct _comparator
	{
		bool operator()(const NGL_texture_subresource_transition& A, const NGL_texture_subresource_transition& B) const
		{
			if (A.m_texture.m_idx == B.m_texture.m_idx)
			{
				if (A.m_old_state == B.m_old_state)
				{
					return A.m_new_state < B.m_new_state;
				}
				
				return A.m_old_state < B.m_old_state;
			}
			return A.m_texture.m_idx < B.m_texture.m_idx;
		}
	};
	
	struct _range
	{
		uint32_t num_subresources;
		_range() : num_subresources(0) { }
	};
	
	std::map<NGL_texture_subresource_transition, _range, _comparator> m_ranges;
	
	D3D12_texture_barrier_merger(std::vector<NGL_texture_subresource_transition> &texture_barriers)
	{
		for (size_t i = 0; i < texture_barriers.size(); i++)
		{
			const NGL_texture_subresource_transition &y = texture_barriers[i];
	
			m_ranges[y].num_subresources++;
		}
	}
};


struct D3D12_texture_subresource_transition
{
	D3D12_RESOURCE_STATES m_old_state;
	D3D12_RESOURCE_STATES m_new_state;
	D3D12_texture *m_texture;
	uint32_t m_subresource_index;
};


void D3D12_command_context::AddNGLBarrierList(std::vector<NGL_texture_subresource_transition>& texture_barriers, std::vector<NGL_buffer_transition>& buffer_barriers)
{
	D3D12_resource_manager *res_mgr = m_backend->m_resource_mgr;

	m_barrier_batch.clear();
	D3D12_RESOURCE_BARRIER barrier;
	bool is_valid_barrier = false;

	// Collect the barriers

	std::vector<D3D12_texture_subresource_transition> d3d12_texture_barriers;
	d3d12_texture_barriers.reserve(texture_barriers.size());

	D3D12_texture_barrier_merger merger(texture_barriers);

	for (auto i = merger.m_ranges.begin(); i != merger.m_ranges.end(); ++i)
	{
		const NGL_texture_subresource_transition &ngl_barrier = i->first;
		D3D12_texture *texture = res_mgr->m_textures[i->first.m_texture.m_idx];
		uint32_t counted_num_subresources = i->second.num_subresources;

		if (counted_num_subresources > 1 && counted_num_subresources >= texture->m_num_subresources)
		{
			D3D12_texture_subresource_transition transition;
			transition.m_texture = texture;
			transition.m_old_state = GetResourceState(ngl_barrier.m_old_state);
			transition.m_new_state = GetResourceState(ngl_barrier.m_new_state);
			transition.m_subresource_index = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

			d3d12_texture_barriers.push_back(transition);
		}
	}

	for (size_t i = 0; i < texture_barriers.size(); i++)
	{
		const NGL_texture_subresource_transition &ngl_barrier = texture_barriers[i];
		D3D12_texture *texture = res_mgr->m_textures[ngl_barrier.m_texture.m_idx];
		uint32_t counted_num_subresources = merger.m_ranges.at(ngl_barrier).num_subresources;

		if (counted_num_subresources <= 1 || counted_num_subresources < texture->m_num_subresources)
		{
			D3D12_texture_subresource_transition transition;
			transition.m_texture = texture;
			transition.m_old_state = GetResourceState(ngl_barrier.m_old_state);
			transition.m_new_state = GetResourceState(ngl_barrier.m_new_state);
			transition.m_subresource_index = texture->GetSubresourceIndex(
				ngl_barrier.m_texture.m_level, ngl_barrier.m_texture.m_layer, ngl_barrier.m_texture.m_face, 0);

			d3d12_texture_barriers.push_back(transition);
		}
	}
	
	for (size_t i = 0; i < d3d12_texture_barriers.size(); i++)
	{
		D3D12_texture_subresource_transition &transition = d3d12_texture_barriers[i];
		D3D12_RESOURCE_STATES old_state = transition.m_old_state;
		D3D12_RESOURCE_STATES new_state = transition.m_new_state;
		D3D12_texture *texture = transition.m_texture;

		uint32_t subresource_index = transition.m_subresource_index;

		is_valid_barrier = DescribeBarrier(texture->m_res, subresource_index, old_state, new_state, barrier);
		if (!is_valid_barrier)
		{
#ifdef NGL_DX12_DEBUG_TRANSITIONS
			_logf("Redundant state request (res: %p sub: %d) %s -> %s", 
				(void*)texture->m_res, subresource_index, 
				GetResourceStateList(old_state).c_str(), GetResourceStateList(new_state).c_str());
#endif
			continue;
		}

		assert(barrier.Transition.pResource != nullptr || barrier.UAV.pResource != nullptr);
		m_barrier_batch.push_back(barrier);

		UpdateTrackedTextureState(*texture, subresource_index, barrier);

		if (old_state != new_state && new_state == D3D12_RESOURCE_STATE_UNORDERED_ACCESS)
		{
			is_valid_barrier = DescribeBarrier(texture->m_res, subresource_index, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, barrier);
			assert(is_valid_barrier);
			assert(barrier.Transition.pResource != nullptr || barrier.UAV.pResource != nullptr);
			m_barrier_batch.push_back(barrier);
		}
	}

	for (size_t i = 0; i < buffer_barriers.size(); i++)
	{
		NGL_buffer_transition &ngl_barrier = buffer_barriers[i];
		D3D12_RESOURCE_STATES old_state = GetResourceState(ngl_barrier.m_old_state);
		D3D12_RESOURCE_STATES new_state = GetResourceState(ngl_barrier.m_new_state);
		D3D12_vertex_buffer *vb = res_mgr->m_vertex_buffers[ngl_barrier.m_idx];

		is_valid_barrier = DescribeBarrier(vb->m_res, 0, old_state, new_state, barrier);
		if (!is_valid_barrier)
		{
#ifdef NGL_DX12_DEBUG_TRANSITIONS
			_logf("Redundant state request (res: %p sub: %d) %s -> %s", 
				(void*)vb->m_res, 0,
				GetResourceStateList(old_state).c_str(), GetResourceStateList(new_state).c_str());
#endif
			continue;
		}
		assert(barrier.Transition.pResource != nullptr || barrier.UAV.pResource != nullptr);
		m_barrier_batch.push_back(barrier);

		UpdateTrackedBufferState(*vb, barrier);

		if (old_state != new_state && new_state == D3D12_RESOURCE_STATE_UNORDERED_ACCESS)
		{
			is_valid_barrier = DescribeBarrier(vb->m_res, 0, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, barrier);
			assert(is_valid_barrier);
			assert(barrier.Transition.pResource != nullptr || barrier.UAV.pResource != nullptr);
			m_barrier_batch.push_back(barrier);
		}
	}

	// Execute the barriers

	if (m_barrier_batch.empty() == false)
	{
		AddResourceBarrierList(m_barrier_batch);
	}
}


void D3D12_command_context::AddResourceBarrierList(const std::vector<D3D12_RESOURCE_BARRIER> barrier_list)
{
#ifdef NGL_DX12_PRINT_TRANSITIONS
	for (size_t i = 0; i < barrier_list.size(); i++)
	{
		PrintTransitionBarrier(barrier_list[i]);
	}
#endif

	m_command_list->ResourceBarrier((uint32_t)barrier_list.size(), barrier_list.data());
}


// D3D12_command_queue ////////////////////////////////////////////////////////////////////////

D3D12_command_queue::D3D12_command_queue(D3D12_backend *backend, D3D12_COMMAND_LIST_TYPE queue_type)
{
	m_backend = nullptr;

	m_command_queue = nullptr;

	m_last_signaled_fence_value = 0;
	m_current_fence_value = 0;
	m_fence = nullptr;
	m_ready_event = 0;

	m_texture_states.clear();
	m_buffer_states.clear();

	m_backend = backend;

	// Create the command quque

	D3D12_COMMAND_QUEUE_DESC queue_desc = {};
	queue_desc.Type = queue_type;
	queue_desc.NodeMask = 1;
	ThrowIfFailed(m_backend->m_device->CreateCommandQueue(&queue_desc, IID_PPV_ARGS(&m_command_queue)));

	// Create fence for syncronizations

	m_last_signaled_fence_value = 0;
	ThrowIfFailed(m_backend->m_device->CreateFence(m_last_signaled_fence_value, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence)));
	m_current_fence_value = m_last_signaled_fence_value;
	m_ready_event = CreateEvent(nullptr, false, false, nullptr);

}


D3D12_command_queue::~D3D12_command_queue()
{
	WaitForCompletion(m_last_signaled_fence_value);

	for (std::list<D3D12_command_context *>::iterator i = m_command_contexts.begin(); i != m_command_contexts.end(); i++)
	{
		delete *i;
	}
	m_command_contexts.clear();

	SAFE_RELEASE(m_fence);
	SAFE_RELEASE(m_command_queue);
}


D3D12_command_context *D3D12_command_queue::CreateCommandContext(bool has_memory_manager)
{
	D3D12_command_context *new_command_context = new D3D12_command_context(m_backend, this, has_memory_manager);
	m_command_contexts.push_back(new_command_context);
	return new_command_context;
}


//uint64_t D3D12_command_queue::Submit(D3D12_job *job)
//{
//	assert(job != nullptr);
//	assert(job->m_is_recording == false); // Didn't call End() before submitting
//
//	ResolveStateTransitions(job);
//	uint64_t fence_value = job->m_command_context->Execute();
//
//	// hold back cpu if the gpu can't keep up
//
//	uint64_t fence_window_size = 1;// (uint64_t)(m_backend->m_jobs.size() / 4);
//	if (m_last_signaled_fence_value > fence_window_size)
//	{
//		WaitForCompletion(m_last_signaled_fence_value - fence_window_size);
//	}
//
//	return fence_value;
//}


uint64_t D3D12_command_queue::ExecuteCommandContext(D3D12_command_context *command_context)
{
	assert(command_context->m_command_queue == this);
	assert(!command_context->m_is_recordable);

	//ResolveStateTransitions(command_context);
	UpdateTrackedResourceStates(command_context);

	ID3D12CommandList* ppCommandLists[] = { command_context->m_command_list };
	m_command_queue->ExecuteCommandLists(1, ppCommandLists);

	m_last_signaled_fence_value++;
	m_command_queue->Signal(m_fence, m_last_signaled_fence_value);
	return m_last_signaled_fence_value;
}


void D3D12_command_queue::WaitForCompletion(uint64_t fence_value)
{
	if (IsFenceComplete(fence_value))
	{
		return;
	}

	ThrowIfFailed(m_fence->SetEventOnCompletion(fence_value, m_ready_event));
	DWORD wait_result = WaitForSingleObject(m_ready_event, INFINITE);
	assert(wait_result == WAIT_OBJECT_0);
}


#if 0
void D3D12_command_queue::ResolveStateTransitions(D3D12_command_context *command_context)
{
	m_barrier_batch.clear();

	// Collect the barriers

	for (size_t i = 0; i < command_context->m_resource_states.size(); i++)
	{
		D3D12_resource *resource = m_backend->m_resource_mgr->m_tracked_resources[i];
		D3D12_resource_state_transition &resource_state = command_context->m_resource_states[i];

		if (resource_state.m_before != (D3D12_RESOURCE_STATES)-1)
		{
			if (resource_state.m_before != resource->m_current_state)
			{
				// Insert transition barrier

				D3D12_RESOURCE_BARRIER barrier;
				barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
				barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
				barrier.Transition.pResource = resource->m_res;
				barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
				barrier.Transition.StateBefore = resource->m_current_state;
				barrier.Transition.StateAfter = resource_state.m_before;

				m_barrier_batch.push_back(barrier);
				resource->m_current_state = resource_state.m_after;
			}
			else if (resource_state.m_before == D3D12_RESOURCE_STATE_UNORDERED_ACCESS)
			{
				// Insert UAV barrier

				D3D12_RESOURCE_BARRIER barrier;
				barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
				barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
				barrier.UAV.pResource = resource->m_res;

				m_barrier_batch.push_back(barrier);
			}
		}

		resource_state = D3D12_resource_state_transition();
	}

	// Execute the barriers

	if (m_barrier_batch.empty() == false)
	{
		m_backend->m_main_context->Reset();
		m_backend->m_main_context->AddResourceBarrierList(m_barrier_batch);
		m_backend->m_main_context->CloseExecute();
		//job->m_command_context->AddResourceBarrierList(m_barrier_batch);

		m_barrier_batch.clear();
	}
}
#endif


#ifdef NGL_DX12_DEBUG_TRANSITIONS
void D3D12_command_queue::UpdateTrackedResourceStates(D3D12_command_context *command_context)
{
	// Verify if current state matches the first state of the command context

	for (auto i = command_context->m_first_texture_states.begin(); i != command_context->m_first_texture_states.end(); i++)
	{
		if (m_texture_states.find(i->first) != m_texture_states.end()
			&& m_texture_states.at(i->first) == command_context->m_first_texture_states.at(i->first))
		{
#if !defined(DISABLE_FIRST_STATE_ERROR_ASSERTS)
			printf("UpdateTrackedResourceStates Texture State \"%s\"\n", this->m_backend->m_resource_mgr->m_textures[i->first.m_texture_id]->m_name.c_str());
			assert(false);
#endif
		}
	}

	for (auto i = command_context->m_first_buffer_states.begin(); i != command_context->m_first_buffer_states.end(); i++)
	{
		if (m_buffer_states.find(i->first) != m_buffer_states.end()
			&& m_buffer_states.at(i->first) == command_context->m_first_buffer_states.at(i->first))
		{
#if !defined(DISABLE_FIRST_STATE_ERROR_ASSERTS)
			assert(false);
#endif
		}
	}

	// Set resulting state

	for (auto i = command_context->m_texture_states.begin(); i != command_context->m_texture_states.end(); i++)
	{
		m_texture_states[i->first] = i->second;
	}

	for (auto i = command_context->m_buffer_states.begin(); i != command_context->m_buffer_states.end(); i++)
	{
		m_buffer_states[i->first] = i->second;
	}
}
#endif


bool D3D12_command_queue::IsFenceComplete(uint64_t fence_value)
{
	if (fence_value > m_current_fence_value)
	{
		//m_current_fence_value = max(m_current_fence_value, m_fence->GetCompletedValue());

		uint64_t new_fence_value = m_fence->GetCompletedValue();
		assert(new_fence_value >= m_current_fence_value);
		m_current_fence_value = new_fence_value;
	}

	return fence_value <= m_current_fence_value;
}


void D3D12_command_queue::Finish()
{
	m_last_signaled_fence_value++;
	m_command_queue->Signal(m_fence, m_last_signaled_fence_value);
	WaitForCompletion(m_last_signaled_fence_value);
}


void D3D12_command_queue::Discard(uint64_t fence_value)
{
	for (std::list<D3D12_command_context *>::iterator i = m_command_contexts.begin(); i != m_command_contexts.end(); i++)
	{
		(*i)->Discard(fence_value);
	}
}


void D3D12_command_queue::DeleteContext(D3D12_command_context *command_context)
{
	m_command_contexts.remove(command_context);
	delete command_context;
}
