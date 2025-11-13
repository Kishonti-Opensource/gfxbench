/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef NGL_D3D12_H
#define NGL_D3D12_H

#include "ngl_d3d12_define.h"
#include "ngl_d3d12_memory.h"
#include "ngl_d3d12_job.h"

struct IDXGIDebug;
struct D3D12_swap_chain;
struct D3D12_command_queue;
struct D3D12_command_context;
struct D3D12_graphics_job;
struct D3D12_shader;
class D3D12_shader_cache;


class D3D12_backend : public NGL_instance
{
	friend D3D12_job;
	friend D3D12_graphics_job;
	friend D3D12_compute_job;
	friend D3D12_swap_chain;
	friend D3D12_resource_manager;
	friend D3D12_shader;
	friend D3D12_command_queue;

public:
	static D3D12_backend* INSTANCE;

public:
	D3D12_backend();
	virtual ~D3D12_backend();

	void CreateContext(NGL_context_descriptor& descriptor);
	void SwapBuffer();
	
	int32_t GetInteger(NGL_backend_property prop);
	const char* GetString(NGL_backend_property prop);
	D3D12_command_context* GetCommandContext(uint32_t command_context_id);
	uint32_t GenJob(NGL_job_descriptor &descriptor);
	void CustomAction(uint32_t job_id, uint32_t parameter);

	inline D3D12_job* Job(uint32_t job_id)
	{
		return m_jobs[job_id];
	}

	inline D3D12_graphics_job* GraphicsJob(uint32_t job_id)
	{
		assert(!m_jobs[job_id]->m_is_compute);
		return (D3D12_graphics_job*)m_jobs[job_id];
	}

	inline D3D12_compute_job* ComputeJob(uint32_t job_id)
	{
		assert(m_jobs[job_id]->m_is_compute);
		return (D3D12_compute_job*)m_jobs[job_id];
	}
		
private:
	void InitDevice(HWND hWnd, uint32_t width, uint32_t height, bool enable_validation, const std::string &selected_device, IDXGIFactory2 *&dxgi_factory);
	
public:
	ID3D12Device *m_device;

	D3D12_resource_manager *m_resource_mgr;
	D3D12_swap_chain *m_swap_chain;
	D3D12_command_queue *m_direct_queue;
	D3D12_command_context *m_main_context;
	D3D12_shader_cache *m_shader_cache;

	bool m_optimized_renderpasses_supported;

private:
	HWND m_hWnd;
	uint32_t m_width;
	uint32_t m_height;

	bool m_enable_vsync;

	ID3D12Debug *m_debug_layer;
	IDXGIDebug *m_dxgi_debug_layer;

	std::vector<D3D12_command_context*> m_command_contexts; // weak reference (no delete)
	std::vector<D3D12_job*> m_jobs;

	std::string m_renderer_string;
	std::string m_vendor_string;

#ifdef NGL_DX12_VISUALIZE_BUFFER_SWAP
	int idx;
#endif
};


class D3D12_shader_cache
{
public:
	D3D12_shader_cache();
	~D3D12_shader_cache();
	ID3DBlob* CompileShader(const std::string &source, const std::string &entry_point, const std::string &version);
	
private:
	ID3DBlob* FindShader(size_t hash);
	void CacheShader(size_t hash, ID3DBlob *shader);
	
	std::mutex m_shader_cache_mutex;
	std::map<size_t, ID3DBlob*> m_shader_cache;
};

#endif
