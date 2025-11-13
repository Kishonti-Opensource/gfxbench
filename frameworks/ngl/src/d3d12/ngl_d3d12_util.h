/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef NGL_D3D12_UTIL_H
#define NGL_D3D12_UTIL_H

#include "ngl_d3d12_define.h"
#include "ngl_d3d12_resource.h"


//TODO?: GetAdapter(): check if selected device supports dx12

std::string WcharToString(const wchar_t *wstr);
void PrintAdapters(IDXGIFactory2 *dxgi_factory);
void PrintAdapterDesc(DXGI_ADAPTER_DESC1 &adapter_desc);

IDXGIAdapter1 *GetAdapter(IDXGIFactory2 *dxgi_factory, const std::string &selected_device, DXGI_ADAPTER_DESC1 &adapter_desc);

// Returns first adapter (same behaviour as D3D12CreateDevice() with pAdapter == NULL)
IDXGIAdapter1 *GetAdapter(IDXGIFactory2 *dxgi_factory, UINT adapter_index, DXGI_ADAPTER_DESC1 &adapter_desc);

// Returns last adapter
IDXGIAdapter1 *GetSoftwareAdapter(IDXGIFactory2 *dxgi_factory, DXGI_ADAPTER_DESC1 &adapter_desc);

void PrintShaders(NGL_shader_source_descriptor ssd[NGL_NUM_SHADER_TYPES]);
void PrintRootSignature(const D3D12_ROOT_SIGNATURE_DESC &root_signature);
void PrintTransitionBarrier(const D3D12_RESOURCE_BARRIER &barrier);
std::string GetResourceStateList(D3D12_RESOURCE_STATES state);

void DescribeRTV(const D3D12_texture &texture, UINT mip_level, UINT slice_index, D3D12_RENDER_TARGET_VIEW_DESC &desc);
void DescribeDSV(const D3D12_texture &texture, UINT mip_level, UINT slice_index, bool is_read_only, D3D12_DEPTH_STENCIL_VIEW_DESC &desc);
DXGI_FORMAT GetDSVFormat(DXGI_FORMAT format);

DXGI_FORMAT GetDXGIVertexFormat(NGL_format ngl_format);

bool GetIndexFormat(NGL_format index_format, DXGI_FORMAT &dx_format, uint32_t &stride);
D3D_PRIMITIVE_TOPOLOGY GetPrimitiveTopology(NGL_primitive_type type);

DXGI_FORMAT GetDXGITextureFormat(NGL_format ngl_format);
void DescribeTexture(const D3D12_texture &texture, const NGL_texture_descriptor &texture_layout, D3D12_RESOURCE_DESC &desc);
D3D12_RESOURCE_DIMENSION GetResourceDimension(NGL_texture_type ngl_type);
const void *ConvertPixelData(const void *raw_data, NGL_format ngl_format, uint32_t width, uint32_t height, std::vector<uint8_t*> &temp_data);

void DescribeSampler(const NGL_texture_descriptor &texture_layout, bool is_shadow_sampler, D3D12_SAMPLER_DESC &desc);
void DescribeStaticSampler(bool is_shadow_sampler, uint32_t base_register, D3D12_SHADER_VISIBILITY visibility, D3D12_STATIC_SAMPLER_DESC &desc, float min_level = 0.0f, float max_level = D3D12_FLOAT32_MAX);
D3D12_FILTER GetSamplerFilter(NGL_texture_filter ngl_filter);
D3D12_TEXTURE_ADDRESS_MODE GetWrapMode(NGL_texture_wrap_mode ngl_mode);

D3D12_SHADER_VISIBILITY GetShaderVisibility(NGL_shader_type shader_type);
D3D12_ROOT_SIGNATURE_FLAGS GetShaderDenyFlag(NGL_shader_type shader_type);
void DescribeSRV(const D3D12_texture &texture, D3D12_SHADER_RESOURCE_VIEW_DESC &desc);
void DescribeSubresourceSRV(const D3D12_texture &texture, uint32_t subresource_index, D3D12_SHADER_RESOURCE_VIEW_DESC &desc);
DXGI_FORMAT GetDepthTextureFormat(DXGI_FORMAT format);

void DescribeBlendState(NGL_blend_func blend_func, NGL_color_channel_mask blend_mask, D3D12_RENDER_TARGET_BLEND_DESC &desc);
void DescribeDepthStencilState(NGL_depth_func depth_func, bool depth_mask, D3D12_DEPTH_STENCIL_DESC &desc);
void DescribeDepthFuncRange(NGL_depth_func depth_func, float &depth_min, float &depth_max);

D3D12_RESOURCE_STATES GetResourceState(NGL_resource_state ngl_state);
bool DescribeBarrier(ID3D12Resource *resource, uint32_t subresource_index, D3D12_RESOURCE_STATES old_state, D3D12_RESOURCE_STATES new_state, D3D12_RESOURCE_BARRIER &barrier);
bool IsValidState(D3D12_RESOURCE_STATES state);

void DescribeBuffer(uint64_t size, D3D12_RESOURCE_DESC &desc);

#endif