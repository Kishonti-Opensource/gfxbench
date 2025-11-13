/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "ngl_d3d12_memory.h"

#include "ngl_d3d12.h"
#include "ngl_d3d12_command.h"
#include <cassert>

// D3D12_memory_page ///////////////////////////////// 

D3D12_memory_page::D3D12_memory_page(D3D12_backend *backend, size_t page_size)
	: D3D12_resource(0)
{
	m_cpu_address = nullptr;
	m_gpu_address = 0;

	m_resource_type = NGL_D3D12_RESOURCE_TYPE_MEMORY_PAGE;

	D3D12_HEAP_PROPERTIES HeapProps;
	HeapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	HeapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	HeapProps.CreationNodeMask = 1;
	HeapProps.VisibleNodeMask = 1;

	D3D12_RESOURCE_DESC ResourceDesc;
	ResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	ResourceDesc.Alignment = 0;
	ResourceDesc.Height = 1;
	ResourceDesc.DepthOrArraySize = 1;
	ResourceDesc.MipLevels = 1;
	ResourceDesc.Format = DXGI_FORMAT_UNKNOWN;
	ResourceDesc.SampleDesc.Count = 1;
	ResourceDesc.SampleDesc.Quality = 0;
	ResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

	m_current_state = D3D12_RESOURCE_STATE_GENERIC_READ;

	{
		HeapProps.Type = D3D12_HEAP_TYPE_UPLOAD;
		ResourceDesc.Width = page_size;
		ResourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
	}
	
	ThrowIfFailed(backend->m_device->CreateCommittedResource(&HeapProps, D3D12_HEAP_FLAG_NONE, &ResourceDesc, m_current_state, nullptr, IID_PPV_ARGS(&m_res)));

	SetName("memory_page");

	m_res->Map(0, nullptr, (void**)&m_cpu_address);
	m_gpu_address = m_res->GetGPUVirtualAddress();
}


D3D12_memory_page::~D3D12_memory_page()
{
	m_res->Unmap(0, nullptr);
}


// D3D12_memory_manager ///////////////////////////////// 

D3D12_memory_manager::D3D12_memory_manager(D3D12_backend *backend)
{
	this->m_backend = backend;
	m_current_page = nullptr;
	m_current_offset = 0;
}


D3D12_memory_manager::~D3D12_memory_manager()
{
	while (m_available_pages.size() > 0)
	{
		D3D12_memory_page *page = m_available_pages.front();
		m_available_pages.pop();
		delete page;
	}

	while (m_old_pages.size() > 0)
	{
		D3D12_memory_page *page = m_old_pages.front();
		m_old_pages.pop();
		delete page;
	}
}


void D3D12_memory_manager::Init()
{
	// Warmup

	m_available_pages.push(new D3D12_memory_page(m_backend, PAGE_SIZE));
}


D3D12_memory_allocation D3D12_memory_manager::Allocate(size_t size)
{
	if (size > PAGE_SIZE)
	{
		_logf("Exceeded max linear allocator page size with single allocation\n");
		assert(0);
	}

	size_t mod = size % ALIGNMENT;
	size_t aligned_size = mod ? size + ALIGNMENT - mod : size;

	if (m_current_offset + aligned_size > PAGE_SIZE)
	{
		m_current_page = nullptr;
	}

	if (m_current_page == nullptr)
	{
		m_current_page = GetPage();
		m_current_offset = 0;

		m_current_pages.push_back(m_current_page);
	}

	assert(m_current_page != nullptr);

	D3D12_memory_allocation mem_alloc(m_current_page, aligned_size, m_current_page->GetCPUAddress(m_current_offset), m_current_page->GetGPUAddress(m_current_offset));

	m_current_offset += aligned_size;

	return mem_alloc;
}


void D3D12_memory_manager::DiscardPages(uint64_t fence_value)
{
	if (m_current_page == nullptr)
	{
		return;
	}

	for (auto it = m_current_pages.begin(); it != m_current_pages.end(); it++)
	{
		(*it)->m_fence_value = fence_value;
		m_old_pages.push(*it);
	}

	m_current_pages.clear();
	m_current_page = nullptr;
	m_current_offset = 0;
}


D3D12_memory_page *D3D12_memory_manager::GetPage()
{
	while (m_old_pages.size() > 0)
	{
		D3D12_memory_page *page = m_old_pages.front();

		if (!m_backend->m_direct_queue->IsFenceComplete(page->m_fence_value))
		{
			break;
		}

		m_available_pages.push(page);
		m_old_pages.pop();
	}

	D3D12_memory_page *page = nullptr;
	if (m_available_pages.size())
	{
		page = m_available_pages.front();
		m_available_pages.pop();
	}
	else
	{
		page = new D3D12_memory_page(m_backend, PAGE_SIZE);
		_logf("DX12: Created memory page");
	}

	return page;
}
