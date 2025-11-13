/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef NGL_D3D12_MEMORY_H
#define NGL_D3D12_MEMORY_H

#include "ngl_d3d12_resource.h"
#include <queue>

class D3D12_backend;

struct D3D12_memory_allocation
{
	D3D12_memory_allocation()
	{
		m_buffer = nullptr;
		m_size = 0;
		m_cpu_address = nullptr;
		m_gpu_address = 0;
	}

	D3D12_memory_allocation(D3D12_resource *buffer, size_t size, uint8_t *cpu_address, D3D12_GPU_VIRTUAL_ADDRESS gpu_address)
	{
		m_buffer = buffer;
		m_size = size;
		m_cpu_address = cpu_address;
		m_gpu_address = gpu_address;
	}

	D3D12_resource *m_buffer;
	size_t m_size;
	uint8_t *m_cpu_address;
	D3D12_GPU_VIRTUAL_ADDRESS m_gpu_address;
};


struct D3D12_memory_page : public D3D12_resource
{
	D3D12_memory_page(D3D12_backend *backend, size_t page_size);
	~D3D12_memory_page();

	inline uint8_t* GetCPUAddress(size_t offset) 
	{
		return m_cpu_address + offset; 
	}

	inline D3D12_GPU_VIRTUAL_ADDRESS GetGPUAddress(size_t offset) 
	{
		return m_gpu_address + offset; 
	}

	uint8_t *m_cpu_address;
	D3D12_GPU_VIRTUAL_ADDRESS m_gpu_address;

	uint64_t m_fence_value;
};


struct D3D12_memory_manager
{
	static const size_t ALIGNMENT = 256;
	static const size_t PAGE_SIZE = 2 * 1024 * 1024; // 2MB

	D3D12_memory_manager(D3D12_backend *backend);
	~D3D12_memory_manager();
	void Init();

	D3D12_memory_allocation Allocate(size_t size);

	void DiscardPages(uint64_t fence_value);

private:
	D3D12_memory_page *GetPage();

private:
	D3D12_backend *m_backend;

	D3D12_memory_page *m_current_page;
	size_t m_current_offset;

	std::vector<D3D12_memory_page*> m_current_pages;

	std::queue<D3D12_memory_page*> m_old_pages;
	std::queue<D3D12_memory_page*> m_available_pages;
};


#endif