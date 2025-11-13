/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "ngl_d3d12.h"

#include "ngl_d3d12_resource.h"
#include "ngl_d3d12_command.h"
#include "ngl_d3d12_memory.h"
#include "ngl_d3d12_job.h"
#include "ngl_d3d12_util.h"
#include <cassert>
#include <string>
#include <sstream>

// D3D12_backend ///////////////////////////////////////////////////////////////////////////////

D3D12_backend *D3D12_backend::INSTANCE = nullptr;


D3D12_backend::D3D12_backend()
{
	m_hWnd = nullptr;
	m_width = 0;
	m_height = 0;

	m_debug_layer = nullptr;
	m_dxgi_debug_layer = nullptr;
	m_device = nullptr;

	m_resource_mgr = nullptr;
	m_swap_chain = nullptr;
	m_direct_queue = nullptr;
	m_main_context = nullptr;
	m_shader_cache = nullptr;

	m_renderer_string = "Direct3D 12";
	m_vendor_string = "Direct3D 12";
}


D3D12_backend::~D3D12_backend()
{
	m_direct_queue->Finish();

	for (size_t i = 0; i < m_jobs.size(); i++)
	{
		delete m_jobs[i];
	}

	delete m_shader_cache;
	delete m_direct_queue;
	delete m_swap_chain;
	delete m_resource_mgr;

	SAFE_RELEASE(m_device);

#if NGL_D3D12_ENABLE_DEBUG_LAYER
	if (m_dxgi_debug_layer != nullptr)
	{
		OutputDebugString("\nNGL DX12 - Live Objects Report (Application held resources only)\n\n");
		m_dxgi_debug_layer->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_IGNORE_INTERNAL);

		OutputDebugString("\nNGL DX12 - Live Objects Report (Full Report)\n\n");
		m_dxgi_debug_layer->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_ALL);
		OutputDebugString("\n");
	}

	SAFE_RELEASE(m_dxgi_debug_layer);
#endif

	SAFE_RELEASE(m_debug_layer);
}


// DX12 objects are handled by the backend, not testfw
void D3D12_backend::CreateContext(NGL_context_descriptor& descriptor)
{
	m_context_descriptor = descriptor;

	uint32_t width = descriptor.m_display_width;
	uint32_t height = descriptor.m_display_height;
	HWND hwnd = (HWND)descriptor.m_user_data[0];
	bool enable_validation = descriptor.m_enable_validation;
	std::string selected_device = descriptor.m_selected_device_id;

	m_enable_vsync = descriptor.m_enable_vsync;

	_logf("DX12 - Create context...");

	IDXGIFactory2 *dxgi_factory = nullptr;
	InitDevice(hwnd, width, height, enable_validation, selected_device, dxgi_factory);

	m_direct_queue = new D3D12_command_queue(this, D3D12_COMMAND_LIST_TYPE_DIRECT);
	m_main_context = m_direct_queue->CreateCommandContext(false);
	m_resource_mgr = new D3D12_resource_manager(this);
	m_resource_mgr->InitTextures();
	m_swap_chain = new D3D12_swap_chain(this, dxgi_factory, m_direct_queue);
	m_shader_cache = new D3D12_shader_cache();

	dxgi_factory->Release();

	GetCommandContext(8); // create some command contexts in advance
}


void D3D12_backend::SwapBuffer()
{
#ifdef NGL_DX12_VISUALIZE_BUFFER_SWAP
	m_main_context->Reset();
	m_main_context->AddResourceBarrier(m_swap_chain->GetCurrentSwapBuffer(), D3D12_RESOURCE_STATE_RENDER_TARGET);

	float clear_color[12] = {
		1.0, 0.0, 0.0, 1.0,
		0.0, 1.0, 0.0, 1.0,
		0.0, 0.0, 1.0, 1.0
	};

	idx = idx % 3;
	D3D12_CPU_DESCRIPTOR_HANDLE color_attachment = m_resource_mgr->m_color_attachment_heap.GetCPUHandle((uint32_t)m_swap_chain->m_system_rtv_offsets[m_swap_chain->GetCurrentSwapBufferIndex()]);
	m_main_context->m_command_list->ClearRenderTargetView(color_attachment, &clear_color[idx * 4], 0, nullptr);
	idx = (idx + 1) % 3;

	m_main_context->CloseExecute();
#endif

	//m_direct_queue->Discard(fence_value);
	//m_direct_queue->WaitForCompletion(fence_value);

	m_swap_chain->Present();
}


int32_t D3D12_backend::GetInteger(NGL_backend_property prop)
{
	switch (prop)
	{
	case NGL_API: return NGL_DIRECT3D_12;
	case NGL_NEED_SWAPBUFFERS: return true; // TODO
	//	NGL_VENDOR,
	//	NGL_RENDERER,
	case NGL_MAJOR_VERSION: return 12;
	case NGL_MINOR_VERSION: return 0;
	//	NGL_VERSION,
	case NGL_RASTERIZATION_CONTROL_MODE: return NGL_ORIGIN_UPPER_LEFT_AND_NDC_FLIP;
	case NGL_DEPTH_MODE: return NGL_ZERO_TO_ONE;
	case NGL_TEXTURE_MAX_ANISOTROPY: return D3D12_REQ_MAXANISOTROPY;
	case NGL_TEXTURE_MAX_SIZE_2D: return D3D12_REQ_TEXTURE2D_U_OR_V_DIMENSION;
	case NGL_TEXTURE_MAX_SIZE_CUBE: return D3D12_REQ_TEXTURECUBE_DIMENSION;
	case NGL_TEXTURE_COMPRESSION_ASTC: return false;
	case NGL_TEXTURE_COMPRESSION_DXT1: return true;
	case NGL_TEXTURE_COMPRESSION_DXT5: return true;
	case NGL_TEXTURE_COMPRESSION_ETC1: return false;
	case NGL_TEXTURE_COMPRESSION_ETC2: return false;
	case NGL_FLOATING_POINT_RENDERTARGET: return true;
	case NGL_TESSELLATION: return true;
//	NGL_PIPELINE_STATISTICS,
	case NGL_MAX_COMPUTE_WORK_GROUP_SIZE_X: return D3D12_CS_THREAD_GROUP_MAX_X;
	case NGL_MAX_COMPUTE_WORK_GROUP_SIZE_Y: return D3D12_CS_THREAD_GROUP_MAX_Y;
	case NGL_MAX_COMPUTE_WORK_GROUP_SIZE_Z: return D3D12_CS_THREAD_GROUP_MAX_Z;
	case NGL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS: return D3D12_CS_THREAD_GROUP_MAX_THREADS_PER_GROUP;
	case NGL_MAX_COMPUTE_SHARED_MEMORY_SIZE: return D3D12_CS_THREAD_LOCAL_TEMP_REGISTER_POOL;
	case NGL_SUBPASS_ENABLED: return false;
	case NGL_D16_LINEAR_SHADOW_FILTER: return true;
	case NGL_D24_LINEAR_SHADOW_FILTER: return true;
	case NGL_SWAPCHAIN_COLOR_FORMAT: return 8888;
	case NGL_SWAPCHAIN_DEPTH_FORMAT: return 0;

	default:
		assert(0);
		return false;
	}
}

const char *D3D12_backend::GetString(NGL_backend_property prop)
{
	switch (prop)
	{
	case NGL_VENDOR: return m_vendor_string.c_str();
	case NGL_RENDERER: return m_renderer_string.c_str();
	case NGL_SWAPCHAIN_COLOR_FORMAT: return "unorm";
	case NGL_SWAPCHAIN_DEPTH_FORMAT: return "";

	default:
		assert(0);
		return false;
	}
}


D3D12_command_context* D3D12_backend::GetCommandContext(uint32_t command_context_id)
{
	while (m_command_contexts.size() <= command_context_id)
	{
		m_command_contexts.push_back(m_direct_queue->CreateCommandContext(true));
	}

	return m_command_contexts[command_context_id];
}


uint32_t D3D12_backend::GenJob(NGL_job_descriptor &descriptor)
{
	D3D12_job *new_job;
	if (descriptor.m_is_compute)
	{
		new_job = new D3D12_compute_job(this);
	}
	else
	{
		new_job = new D3D12_graphics_job(this);
	}

	new_job->Init(descriptor);
	m_jobs.push_back(new_job);

	return (uint32_t)m_jobs.size() - 1;
}


void D3D12_backend::CustomAction(uint32_t job_id, uint32_t parameter)
{
	switch (parameter)
	{
	case NGL_CUSTOM_ACTION_SWAPBUFFERS:
		SwapBuffer();
		break;

	case NGL_CUSTOM_ACTION_WAIT_FINISH:
		break;

	case 235173:
		m_swap_chain->Resize(this);
		break;

	default:
		if (job_id < m_jobs.size())
		{
			Job(job_id)->CustomAction(parameter);
		}
		break;
	}
}

inline bool D3D12GetRenderpassSupportInfo(ID3D12Device* pDevice)
{
	CD3DX12FeatureSupport dx12_feature_support;

	dx12_feature_support.Init(pDevice);

	switch (dx12_feature_support.RenderPassesTier())
	{
	case D3D12_RENDER_PASS_TIER_0:
		return false;
	case D3D12_RENDER_PASS_TIER_1:
	case D3D12_RENDER_PASS_TIER_2:
		return true;
	default:
		// Future tiers will also support the older ones.
		return true;
	}
}

void D3D12_backend::InitDevice(HWND hWnd, uint32_t width, uint32_t height, bool enable_validation, const std::string &selected_device, IDXGIFactory2 *&dxgi_factory)
{
	m_hWnd = hWnd;
	m_width = width;
	m_height = height;

#ifdef NGL_D3D12_ENABLE_DEBUG_LAYER
	enable_validation = true;
#endif

#if NGL_D3D12_ENABLE_DEBUG_LAYER
	if (enable_validation)
	{
		// Enable D3D12 debug layer

		HRESULT result = S_FALSE;

		result = D3D12GetDebugInterface(IID_PPV_ARGS(&m_debug_layer));

		if (result == S_OK)
		{
			m_debug_layer->EnableDebugLayer();
			_logf("DX12 - Debug interface initialized");
		}
		else
		{
			_logf("DX12 - Can not initialize debug interface!");
		}

		// Enable DXGI debug layer

		result = S_FALSE;

		HMODULE module = GetModuleHandle("Dxgidebug.dll");
		if (module != NULL)
		{
			typedef HRESULT(WINAPI * LPDXGIGETDEBUGINTERFACE)(REFIID, void **);
			auto dxgiGetDebugInterface = (LPDXGIGETDEBUGINTERFACE)GetProcAddress(module, "DXGIGetDebugInterface");
			result = dxgiGetDebugInterface(IID_PPV_ARGS(&m_dxgi_debug_layer));
		}

		if (result == S_OK)
		{
			_logf("DX12 - DXGI Debug interface initialized");
		}
		else
		{
			_logf("DX12 - Can not initialize DXGI debug interface!");
		}
	}

	// if fails, check if graphics debugging tools is installed
	// or set NGL_D3D12_ENABLE_DEBUG_LAYER to 0 in ngl_d3d12_define.h to disable debug layers
	ThrowIfFailed(CreateDXGIFactory1(IID_PPV_ARGS(&dxgi_factory)));
#else
	ThrowIfFailed(CreateDXGIFactory1(IID_PPV_ARGS(&dxgi_factory)));
#endif

	PrintAdapters(dxgi_factory);

	{
		IDXGIAdapter1 *adapter;
		DXGI_ADAPTER_DESC1 adapter_desc;

#ifndef NGL_DX12_USE_SOFTWARE_ADAPTER
		if (selected_device.length() > 0)
		{
			adapter = GetAdapter(dxgi_factory, selected_device, adapter_desc);
		}
		else
		{
			adapter = GetAdapter(dxgi_factory, 0, adapter_desc);
		}

		if (adapter == nullptr)
		{
			_logf("Cannot find adapter matching the selected device ID.");
		}
#else
		adapter = GetSoftwareAdapter(dxgi_factory, adapter_desc);
#endif

		assert(adapter != nullptr); // there's no device available

		ThrowIfFailed(D3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&m_device)));
		m_vendor_string = WcharToString(adapter_desc.Description);
		m_optimized_renderpasses_supported = D3D12GetRenderpassSupportInfo(m_device);
		//m_device->SetStablePowerState(TRUE);

		_logf("Selected adapter:");
		PrintAdapterDesc(adapter_desc);
		_logf("m_optimized_renderpasses_supported: %d", m_optimized_renderpasses_supported);

		adapter->Release();
	}

#if NGL_D3D12_ENABLE_DEBUG_LAYER
	if (enable_validation)
	{
		ID3D12InfoQueue *info_queue = nullptr;
		if (m_device->QueryInterface(IID_PPV_ARGS(&info_queue)) == S_OK)
		{
			info_queue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, true);
			info_queue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, true);
			//info_queue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, true);
		}
	}
#endif
}


// D3D12_shader_cache ///////////////////////////////////////////////////////////////////////////////

D3D12_shader_cache::D3D12_shader_cache()
{

}

D3D12_shader_cache::~D3D12_shader_cache()
{
	for (std::map<size_t, ID3DBlob*>::iterator i = m_shader_cache.begin(); i != m_shader_cache.end(); i++)
	{
		SAFE_RELEASE(i->second);
	}
}

ID3DBlob* D3D12_shader_cache::CompileShader(const std::string &source, const std::string &entry_point, const std::string &version)
{
	// TODO: Create better cache mechanism (reference counting, key hashing...)
	std::stringstream sstream;
	sstream << entry_point << "_" << version << "_" << source;

	std::hash<std::string> hash_function;
	size_t hash = hash_function(sstream.str());

	ID3DBlob *shader = FindShader(hash);
	if (shader != nullptr)
	{
		_logf("Shader loaded from cache");
		return shader;
	}

	ID3DBlob *error_msg = nullptr;
	D3DCompile(source.data(), source.size(), 0, 0, 0, entry_point.c_str(), version.c_str(), 0, 0, &shader, &error_msg);
	if (error_msg != nullptr)
	{
		// Print the error message
		std::vector<char> error_string(error_msg->GetBufferSize() + 1, 0);
		memcpy(error_string.data(), error_msg->GetBufferPointer(), error_msg->GetBufferSize());
		std::string s(error_string.data());

		if (!shader
			|| (s.find("warning X") == std::string::npos // ignore warnings altogether
				&& s.find("warning X3571: pow(f, e) will not work for negative f") == std::string::npos // ignore warning for pow(f, e)
				&& s.find("warning X4714: sum of temp registers and indexable temp registers") == std::string::npos)
			)
		{
			if (shader)
			{
				_logf("=====SHADER COMPILE WARNING=====\n");
			}
			else
			{
				_logf("=====SHADER COMPILE ERROR=====\n");
			}

			// Dump the shader source
			std::stringstream ss(source);
			std::string line;
			int counter = 1;
			while (std::getline(ss, line))
			{
				_logf("%d: %s", counter++, line.c_str());
			}

			// Print the error message
			_logf("%s", error_string.data());
		}

		//return nullptr;
	}

	if (shader)
	{
		CacheShader(hash, shader);
	}

	return shader;
}


ID3DBlob* D3D12_shader_cache::FindShader(size_t hash)
{
	std::lock_guard<std::mutex> lock(m_shader_cache_mutex);

	auto it = m_shader_cache.find(hash);
	if (it != m_shader_cache.end())
	{
		return it->second;
	}
	return nullptr;
}


void D3D12_shader_cache::CacheShader(size_t hash, ID3DBlob *shader)
{
	std::lock_guard<std::mutex> lock(m_shader_cache_mutex);

	m_shader_cache[hash] = shader;
}
