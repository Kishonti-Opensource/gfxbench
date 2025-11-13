/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "ngl_d3d12_util.h"

#include <sstream>


std::string WcharToString(const wchar_t *wstr)
{
	const size_t buffer_size = 1024;
	char buffer[buffer_size];

	size_t converted_count = 0;
	size_t ret = wcstombs_s(&converted_count, buffer, wstr, buffer_size);
	if (ret == buffer_size)
	{
		buffer[buffer_size - 1] = '\0';
	}

	return std::string(buffer);
}


void PrintAdapters(IDXGIFactory2 *dxgi_factory)
{
	IDXGIAdapter1 *adapter = nullptr;
	DXGI_ADAPTER_DESC1 adapter_desc;
	UINT i = 0;
	_logf("");
	_logf("DX12 - Available adapters:");
	while (dxgi_factory->EnumAdapters1(i, &adapter) == S_OK)
	{
		ThrowIfFailed(adapter->GetDesc1(&adapter_desc));
		adapter->Release();
		i++;

		_logf("DX12 - %s", WcharToString(adapter_desc.Description).c_str());
	}
	_logf("");
}


void PrintAdapterDesc(DXGI_ADAPTER_DESC1 &adapter_desc)
{
	const size_t MB = 1024 * 1024;

	_logf("DX12 - Adapter: %s", WcharToString(adapter_desc.Description).c_str());
	_logf("DX12 - Dedicated video memory: %dmb", int32_t(adapter_desc.DedicatedVideoMemory / MB));
	_logf("DX12 - Dedicated system memory: %dmb", int32_t(adapter_desc.DedicatedSystemMemory / MB));
	_logf("DX12 - Shared system memory: %dmb", int32_t(adapter_desc.SharedSystemMemory / MB));
	_logf("");
}


IDXGIAdapter1 *GetAdapter(IDXGIFactory2 *dxgi_factory, const std::string &selected_device, DXGI_ADAPTER_DESC1 &adapter_desc)
{
	size_t delimiter_pos = selected_device.find_last_of(";");
	if (delimiter_pos == std::string::npos || selected_device.size() == 0|| delimiter_pos >= selected_device.size() - 1)
	{
		return nullptr;
	}

	std::string selected_luid = selected_device.substr(delimiter_pos + 1);
	uint64_t luid = 0;
	
	IDXGIAdapter1 *adapter = nullptr;
	UINT i = 0;
	while (true)
	{
		if (dxgi_factory->EnumAdapters1(i, &adapter) == S_OK)
		{
			ThrowIfFailed(adapter->GetDesc1(&adapter_desc));

			assert(sizeof(luid) == sizeof(adapter_desc.AdapterLuid));
			memcpy(&luid, &adapter_desc.AdapterLuid, sizeof(luid));

			if (std::to_string(luid) == selected_luid)
			{
				return adapter;
			}
		}
		else
		{
			return nullptr;
		}

		adapter->Release();
		i++;
	}

	assert(false);
	return nullptr;
}


IDXGIAdapter1 *GetAdapter(IDXGIFactory2 *dxgi_factory, UINT adapter_index, DXGI_ADAPTER_DESC1 &adapter_desc)
{
	IDXGIAdapter1 *adapter = nullptr;
	if (dxgi_factory->EnumAdapters1(adapter_index, &adapter) == S_OK)
	{
		ThrowIfFailed(adapter->GetDesc1(&adapter_desc));
		return adapter;
	}

	return nullptr;
}


IDXGIAdapter1 *GetSoftwareAdapter(IDXGIFactory2 *dxgi_factory, DXGI_ADAPTER_DESC1 &adapter_desc)
{
	IDXGIAdapter1 *adapter = nullptr;
	UINT i = 0;
	while (dxgi_factory->EnumAdapters1(i, &adapter) == S_OK)
	{
		adapter->Release();
		i++;
	}

	adapter = nullptr;
	if (i > 0)
	{
		dxgi_factory->EnumAdapters1(i - 1, &adapter);
		ThrowIfFailed(adapter->GetDesc1(&adapter_desc));
	}

	return adapter;
}


void PrintShaders(NGL_shader_source_descriptor ssd[NGL_NUM_SHADER_TYPES])
{
	char *types[] = {
		"NGL_VERTEX_SHADER", //NGL_VERTEX_SHADER = 0,
		"NGL_FRAGMENT_SHADER", //NGL_FRAGMENT_SHADER,
		"NGL_GEOMETRY_SHADER", //NGL_GEOMETRY_SHADER,
		"NGL_TESS_CONTROL_SHADER", //NGL_TESS_CONTROL_SHADER,
		"NGL_TESS_EVALUATION_SHADER", //NGL_TESS_EVALUATION_SHADER,
		"NGL_COMPUTE_SHADER" //NGL_COMPUTE_SHADER,
	};

	_logf("\n\n\n\n\n\n");
	_logf("# Shader Printout #####################################################################");
	_logf("");
	for (uint32_t shader_type = NGL_VERTEX_SHADER; shader_type < NGL_NUM_SHADER_TYPES; shader_type++)
	{
		if (ssd[shader_type].m_source_data.empty())
		{
			continue;
		}
		_logf("\n\n\n\n\n\n");
		_logf("# SHADER TYPE: %s ######################################################################", types[shader_type]);
		_logf("%s", ssd[shader_type].m_source_data.c_str());
	}

	_logf("");
	_logf("# Shader Printout End #################################################################\n\n\n\n\n\n");
}


void PrintRootSignature(const D3D12_ROOT_SIGNATURE_DESC &root_signature)
{
	int rs_size = 0;

	_logf("# Root Signature Info ###################################################################");
	_logf("");
	_logf("Flags:");

	if (root_signature.Flags == D3D12_ROOT_SIGNATURE_FLAG_NONE)
	{
		_logf("None");
	}
	else
	{
		char *flags[] = {
			"ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT", //D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT = 0x1,
			"DENY_VERTEX_SHADER_ROOT_ACCESS", //D3D12_ROOT_SIGNATURE_FLAG_DENY_VERTEX_SHADER_ROOT_ACCESS = 0x2,
			"DENY_HULL_SHADER_ROOT_ACCESS", //D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS = 0x4,
			"DENY_DOMAIN_SHADER_ROOT_ACCESS", //D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS = 0x8,
			"DENY_GEOMETRY_SHADER_ROOT_ACCESS", //D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS = 0x10,
			"DENY_PIXEL_SHADER_ROOT_ACCESS", //D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS = 0x20,
			"FLAG_ALLOW_STREAM_OUTPUT" //D3D12_ROOT_SIGNATURE_FLAG_ALLOW_STREAM_OUTPUT = 0x40
		};

		int32_t mask = 1;
		for (size_t i = 0; i < sizeof(flags) / sizeof(flags[0]); i++)
		{
			if (root_signature.Flags & mask)
			{
				_logf("  %s", flags[i]);
			}

			mask <<= 1;
		}
	}

	if ((root_signature.Flags & D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT) > 0)
	{
		rs_size += 1;
	}


	char *visibility_types[] = {
		"ALL", //D3D12_SHADER_VISIBILITY_ALL = 0,
		"VERTEX", //D3D12_SHADER_VISIBILITY_VERTEX = 1,
		"HULL", //D3D12_SHADER_VISIBILITY_HULL = 2,
		"DOMAIN", //D3D12_SHADER_VISIBILITY_DOMAIN = 3,
		"GEOMETRY", //D3D12_SHADER_VISIBILITY_GEOMETRY = 4,
		"PIXEL" //D3D12_SHADER_VISIBILITY_PIXEL = 5
	};


	_logf("");
	_logf("Parameters (%d):", root_signature.NumParameters);
	for (size_t i = 0; i < root_signature.NumParameters; i++)
	{
		char *parameter_types[] = {
			"DESCRIPTOR_TABLE", //D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE = 0,
			"32BIT_CONSTANTS", //D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS = (D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE + 1),
			"CBV", //D3D12_ROOT_PARAMETER_TYPE_CBV = (D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS + 1),
			"SRV", //D3D12_ROOT_PARAMETER_TYPE_SRV = (D3D12_ROOT_PARAMETER_TYPE_CBV + 1),
			"UAV" //D3D12_ROOT_PARAMETER_TYPE_UAV = (D3D12_ROOT_PARAMETER_TYPE_SRV + 1)
		};

		char *range_types[] = {
			"SRV", //D3D12_DESCRIPTOR_RANGE_TYPE_SRV = 0,
			"UAV", //D3D12_DESCRIPTOR_RANGE_TYPE_UAV = (D3D12_DESCRIPTOR_RANGE_TYPE_SRV + 1),
			"CBV", //D3D12_DESCRIPTOR_RANGE_TYPE_CBV = (D3D12_DESCRIPTOR_RANGE_TYPE_UAV + 1),
			"SAMPLER" //D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER = (D3D12_DESCRIPTOR_RANGE_TYPE_CBV + 1)
		};


		if (root_signature.pParameters[i].ParameterType == D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE)
		{
			_logf("  Parameter #%d:  Type: %s, Visibility: %s", (int)i, parameter_types[root_signature.pParameters[i].ParameterType], visibility_types[root_signature.pParameters[i].ShaderVisibility]);

			const D3D12_ROOT_DESCRIPTOR_TABLE &dt = root_signature.pParameters[i].DescriptorTable;
			for (UINT j = 0; j < dt.NumDescriptorRanges; j++)
			{
				//TODO: OffsetInDescriptorsFromTableStart, RegisterSpace
				_logf("    Range #%d:  Type: %s, Register: %d, Count: %d", j, range_types[dt.pDescriptorRanges[j].RangeType], dt.pDescriptorRanges[j].BaseShaderRegister, dt.pDescriptorRanges[j].NumDescriptors);
			}

			rs_size += 1;
		}
		else if (root_signature.pParameters[i].ParameterType == D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS)
		{
			_logf("  Parameter #%d:  Type: %s, Size: %d, Visibility: %s", (int)i, parameter_types[root_signature.pParameters[i].ParameterType], root_signature.pParameters[i].Constants.Num32BitValues, visibility_types[root_signature.pParameters[i].ShaderVisibility]);
			rs_size += root_signature.pParameters[i].Constants.Num32BitValues;
		}
		else // CBV, SRV, UAV
		{
			_logf("  Parameter #%d:  Type: %s, Visibility: %s", (int)i, parameter_types[root_signature.pParameters[i].ParameterType], visibility_types[root_signature.pParameters[i].ShaderVisibility]);
			rs_size += 2;
		}
		
		//_logf("  Register:%s  (Space: %s)\n", root_signature.pParameters[i]., root_signature.pParameters[i].RegisterSpace);
	}

	_logf("");
	_logf("Static Samplers (%d):", root_signature.NumStaticSamplers);
	for (size_t i = 0; i < root_signature.NumStaticSamplers; i++)
	{
		//TODO: RegisterSpace
		_logf("  Sampler #%d:  Visibility: %s, Register:%d", (int)i, visibility_types[root_signature.pStaticSamplers[i].ShaderVisibility],
			root_signature.pStaticSamplers[i].ShaderRegister);
	}

	_logf("");
	_logf("Total root signature size: %d DWORDs", rs_size);
	_logf("");
	_logf("# Root Signature Info End #################################################################");
	_logf("");
}


void PrintTransitionBarrier(const D3D12_RESOURCE_BARRIER &barrier)
{
	assert(barrier.Type != D3D12_RESOURCE_BARRIER_TYPE_ALIASING);

	if (barrier.Type == D3D12_RESOURCE_BARRIER_TYPE_UAV)
	{
		_logf("UAV Barrier (res: %p)\n", (void*)barrier.Transition.pResource);
	}
	else if(barrier.Type == D3D12_RESOURCE_BARRIER_TYPE_TRANSITION)
	{
		D3D12_RESOURCE_STATES states[] = {
			//D3D12_RESOURCE_STATE_COMMON,
			D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER,
			D3D12_RESOURCE_STATE_INDEX_BUFFER,
			D3D12_RESOURCE_STATE_RENDER_TARGET,
			D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
			D3D12_RESOURCE_STATE_DEPTH_WRITE,
			D3D12_RESOURCE_STATE_DEPTH_READ,
			D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
			D3D12_RESOURCE_STATE_STREAM_OUT,
			D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT,
			D3D12_RESOURCE_STATE_COPY_DEST,
			D3D12_RESOURCE_STATE_COPY_SOURCE,
			D3D12_RESOURCE_STATE_RESOLVE_DEST,
			D3D12_RESOURCE_STATE_RESOLVE_SOURCE
		};

		char *state_names[] = {
			//"COMMON", //D3D12_RESOURCE_STATE_COMMON = 0,
			"VERTEX_AND_CONSTANT_BUFFER", //D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER = 0x1,
			"INDEX_BUFFER", //D3D12_RESOURCE_STATE_INDEX_BUFFER = 0x2,
			"RENDER_TARGET", //D3D12_RESOURCE_STATE_RENDER_TARGET = 0x4,
			"UNORDERED_ACCESS", //D3D12_RESOURCE_STATE_UNORDERED_ACCESS = 0x8,
			"DEPTH_WRITE", //D3D12_RESOURCE_STATE_DEPTH_WRITE = 0x10,
			"DEPTH_READ", //D3D12_RESOURCE_STATE_DEPTH_READ = 0x20,
			"NON_PIXEL_SHADER_RESOURCE", //D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE = 0x40,
			"PIXEL_SHADER_RESOURCE", //D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE = 0x80,
			"STREAM_OUT", //D3D12_RESOURCE_STATE_STREAM_OUT = 0x100,
			"INDIRECT_ARGUMENT", //D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT = 0x200,
			"COPY_DEST", //D3D12_RESOURCE_STATE_COPY_DEST = 0x400,
			"COPY_SOURCE", //D3D12_RESOURCE_STATE_COPY_SOURCE = 0x800,
			"RESOLVE_DEST", //D3D12_RESOURCE_STATE_RESOLVE_DEST = 0x1000,
			"RESOLVE_SOURCE" //D3D12_RESOURCE_STATE_RESOLVE_SOURCE = 0x2000,
			//D3D12_RESOURCE_STATE_GENERIC_READ	= ( ( ( ( ( 0x1 | 0x2 )  | 0x40 )  | 0x80 )  | 0x200 )  | 0x800 ) ,
			//D3D12_RESOURCE_STATE_PRESENT = 0,
			//D3D12_RESOURCE_STATE_PREDICATION = 0x200
		};

		std::stringstream before_state;
		std::stringstream after_state;
		for (size_t i = 0; i < sizeof(states) / sizeof(states[0]); i++)
		{
			if ((barrier.Transition.StateBefore & states[i]) == states[i])
			{
				if (before_state.str().length() > 0)
				{
					before_state << " | ";
				}

				before_state << state_names[i];
			}

			if ((barrier.Transition.StateAfter & states[i]) == states[i])
			{
				if (after_state.str().length() > 0)
				{
					after_state << " | ";
				}

				after_state << state_names[i];
			}
		}

		if (barrier.Transition.StateBefore == D3D12_RESOURCE_STATE_COMMON)
		{
			before_state << "COMMON";
		}

		if (barrier.Transition.StateAfter == D3D12_RESOURCE_STATE_COMMON)
		{
			after_state << "COMMON";
		}

		assert(before_state.str().length() > 0);
		assert(after_state.str().length() > 0);

		_logf("Transition Barrier (res: %p, sub: %d) %s -> %s\n", 
			(void*)barrier.Transition.pResource, barrier.Transition.Subresource, 
			before_state.str().c_str(), after_state.str().c_str());
	}
}


std::string GetResourceStateList(D3D12_RESOURCE_STATES state)
{
	D3D12_RESOURCE_STATES states[] = {
		//D3D12_RESOURCE_STATE_COMMON,
		D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER,
		D3D12_RESOURCE_STATE_INDEX_BUFFER,
		D3D12_RESOURCE_STATE_RENDER_TARGET,
		D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
		D3D12_RESOURCE_STATE_DEPTH_WRITE,
		D3D12_RESOURCE_STATE_DEPTH_READ,
		D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
		D3D12_RESOURCE_STATE_STREAM_OUT,
		D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT,
		D3D12_RESOURCE_STATE_COPY_DEST,
		D3D12_RESOURCE_STATE_COPY_SOURCE,
		D3D12_RESOURCE_STATE_RESOLVE_DEST,
		D3D12_RESOURCE_STATE_RESOLVE_SOURCE
	};

	char *state_names[] = {
		//"COMMON", //D3D12_RESOURCE_STATE_COMMON = 0,
		"VERTEX_AND_CONSTANT_BUFFER", //D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER = 0x1,
		"INDEX_BUFFER", //D3D12_RESOURCE_STATE_INDEX_BUFFER = 0x2,
		"RENDER_TARGET", //D3D12_RESOURCE_STATE_RENDER_TARGET = 0x4,
		"UNORDERED_ACCESS", //D3D12_RESOURCE_STATE_UNORDERED_ACCESS = 0x8,
		"DEPTH_WRITE", //D3D12_RESOURCE_STATE_DEPTH_WRITE = 0x10,
		"DEPTH_READ", //D3D12_RESOURCE_STATE_DEPTH_READ = 0x20,
		"NON_PIXEL_SHADER_RESOURCE", //D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE = 0x40,
		"PIXEL_SHADER_RESOURCE", //D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE = 0x80,
		"STREAM_OUT", //D3D12_RESOURCE_STATE_STREAM_OUT = 0x100,
		"INDIRECT_ARGUMENT", //D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT = 0x200,
		"COPY_DEST", //D3D12_RESOURCE_STATE_COPY_DEST = 0x400,
		"COPY_SOURCE", //D3D12_RESOURCE_STATE_COPY_SOURCE = 0x800,
		"RESOLVE_DEST", //D3D12_RESOURCE_STATE_RESOLVE_DEST = 0x1000,
		"RESOLVE_SOURCE" //D3D12_RESOURCE_STATE_RESOLVE_SOURCE = 0x2000,
		//D3D12_RESOURCE_STATE_GENERIC_READ	= ( ( ( ( ( 0x1 | 0x2 )  | 0x40 )  | 0x80 )  | 0x200 )  | 0x800 ) ,
		//D3D12_RESOURCE_STATE_PRESENT = 0,
		//D3D12_RESOURCE_STATE_PREDICATION = 0x200
	};

	std::stringstream state_list;
	for (size_t i = 0; i < sizeof(states) / sizeof(states[0]); i++)
	{
		if ((state & states[i]) == states[i])
		{
			if (state_list.str().length() > 0)
			{
				state_list << " | ";
			}

			state_list << state_names[i];
		}
	}

	if (state == D3D12_RESOURCE_STATE_COMMON)
	{
		state_list << "COMMON";
	}

	assert(state_list.str().length() > 0);
	return state_list.str();
}


void DescribeRTV(const D3D12_texture &texture, UINT mip_level, UINT slice_index, D3D12_RENDER_TARGET_VIEW_DESC &desc)
{
	memset(&desc, 0, sizeof(desc));

	desc.Format = texture.m_format;

	switch (texture.m_ngl_texture.m_texture_descriptor.m_type)
	{
	case NGL_TEXTURE_2D:
		desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
		desc.Texture2D.MipSlice = mip_level;
		break;

	case NGL_TEXTURE_2D_ARRAY:
		desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DARRAY;
		desc.Texture2DArray.MipSlice = mip_level;
		desc.Texture2DArray.ArraySize = 1;
		desc.Texture2DArray.FirstArraySlice = slice_index; // e.i. ad.m_attachment_layer;
		break;

	case NGL_TEXTURE_CUBE:
		desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DARRAY;
		desc.Texture2DArray.MipSlice = mip_level;
		desc.Texture2DArray.ArraySize = 1;
		desc.Texture2DArray.FirstArraySlice = slice_index; // e.i. ad.m_attachment_face;
		break;

	case NGL_RENDERBUFFER:
	default:
		_logf("DX12: Unsupported texture type: %d\n", (int32_t)texture.m_ngl_texture.m_texture_descriptor.m_type);
		assert(0);
		break;
	}
}

void DescribeDSV(const D3D12_texture &texture, UINT mip_level, UINT slice_index, bool is_read_only, D3D12_DEPTH_STENCIL_VIEW_DESC &desc)
{
	memset(&desc, 0, sizeof(desc));

	desc.Format = GetDSVFormat(texture.m_format);
	desc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	desc.Flags = is_read_only ? D3D12_DSV_FLAG_READ_ONLY_DEPTH : D3D12_DSV_FLAG_NONE;

	switch (texture.m_ngl_texture.m_texture_descriptor.m_type)
	{
	case NGL_TEXTURE_2D:
		desc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
		desc.Texture2D.MipSlice = mip_level;
		break;

	case NGL_TEXTURE_2D_ARRAY:
		desc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DARRAY;
		desc.Texture2DArray.MipSlice = mip_level;
		desc.Texture2DArray.ArraySize = 1;
		desc.Texture2DArray.FirstArraySlice = slice_index;
		break;

	case NGL_TEXTURE_CUBE:
		desc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DARRAY;
		desc.Texture2DArray.MipSlice = mip_level;
		desc.Texture2DArray.ArraySize = 1;
		desc.Texture2DArray.FirstArraySlice = slice_index;
		break;

	case NGL_RENDERBUFFER:
	default:
		_logf("DX12: Unsupported texture type: %d\n", (int32_t)texture.m_ngl_texture.m_texture_descriptor.m_type);
		assert(0);
		break;
	}
}


DXGI_FORMAT GetDSVFormat(DXGI_FORMAT format)
{
	switch (format)
	{
		// 32-bit Z w/ Stencil
	case DXGI_FORMAT_R32G8X24_TYPELESS:
	case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:
	case DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS:
	case DXGI_FORMAT_X32_TYPELESS_G8X24_UINT:
		return DXGI_FORMAT_D32_FLOAT_S8X24_UINT;

		// No Stencil
	case DXGI_FORMAT_R32_TYPELESS:
	case DXGI_FORMAT_D32_FLOAT:
	case DXGI_FORMAT_R32_FLOAT:
		return DXGI_FORMAT_D32_FLOAT;

		// 24-bit Z
	case DXGI_FORMAT_R24G8_TYPELESS:
	case DXGI_FORMAT_D24_UNORM_S8_UINT:
	case DXGI_FORMAT_R24_UNORM_X8_TYPELESS:
	case DXGI_FORMAT_X24_TYPELESS_G8_UINT:
		return DXGI_FORMAT_D24_UNORM_S8_UINT;

		// 16-bit Z w/o Stencil
	case DXGI_FORMAT_R16_TYPELESS:
	case DXGI_FORMAT_D16_UNORM:
	case DXGI_FORMAT_R16_UNORM:
		return DXGI_FORMAT_D16_UNORM;

	default:
		assert(false);
		return format;
	}
}


DXGI_FORMAT GetDXGIVertexFormat(NGL_format ngl_format)
{
	switch (ngl_format)
	{
	case NGL_R32_FLOAT:	return DXGI_FORMAT_R32_FLOAT;
	case NGL_R32_G32_FLOAT: return DXGI_FORMAT_R32G32_FLOAT;
	case NGL_R32_G32_B32_FLOAT: return DXGI_FORMAT_R32G32B32_FLOAT;
	case NGL_R32_G32_B32_A32_FLOAT:	return DXGI_FORMAT_R32G32B32A32_FLOAT;
	case NGL_R8_UINT: return DXGI_FORMAT_R8_UINT;
	case NGL_R8_G8_UINT: return DXGI_FORMAT_R8G8_UINT;
	case NGL_R8_G8_B8_A8_UINT: return DXGI_FORMAT_R8G8B8A8_UINT;

	default:
		_logf("DX12 - Unsupported vertex NGL format: %d\n", (int32_t)ngl_format);
		assert(false);
		return DXGI_FORMAT_UNKNOWN;
	}
}


bool GetIndexFormat(NGL_format index_format, DXGI_FORMAT &dx_format, uint32_t &stride)
{
	stride = 0;

	switch (index_format)
	{
	case NGL_R16_UINT:
		dx_format = DXGI_FORMAT_R16_UINT;
		stride = 2;
		break;

	case NGL_R32_UINT:
		dx_format = DXGI_FORMAT_R32_UINT;
		stride = 4;
		break;

	default:
		_logf("DX12 - Unsupported index format: %d\n", (int32_t)index_format);
		assert(0);
		return false;
	}

	return true;
}


D3D_PRIMITIVE_TOPOLOGY GetPrimitiveTopology(NGL_primitive_type type)
{
	switch (type)
	{
	case NGL_POINTS: return D3D_PRIMITIVE_TOPOLOGY_POINTLIST;
	case NGL_LINES: return D3D_PRIMITIVE_TOPOLOGY_LINELIST;
	case NGL_TRIANGLES: return D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	//case NGL_PATCH1: return D3D_PRIMITIVE_TOPOLOGY_1_CONTROL_POINT_PATCHLIST;
	//case NGL_PATCH2: return D3D_PRIMITIVE_TOPOLOGY_2_CONTROL_POINT_PATCHLIST;
	case NGL_PATCH3: return D3D_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST;
	case NGL_PATCH4: return D3D_PRIMITIVE_TOPOLOGY_4_CONTROL_POINT_PATCHLIST;

	default:
		_logf("DX12 - Unsupported primitive type: %d\n", (int32_t)type);
		assert(false);
		return D3D_PRIMITIVE_TOPOLOGY_UNDEFINED;
	}
}


DXGI_FORMAT GetDXGITextureFormat(NGL_format ngl_format)
{
	switch (ngl_format)
	{
	case NGL_R8_UNORM:
		return DXGI_FORMAT_R8_UNORM;

	case NGL_R8_G8_B8_UNORM:
	case NGL_R8_G8_B8_A8_UNORM:
		return DXGI_FORMAT_R8G8B8A8_UNORM;

	case NGL_R8_G8_B8_UNORM_SRGB:
	case NGL_R8_G8_B8_A8_UNORM_SRGB:
		return DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;

	case NGL_R10_G10_B10_A2_UNORM:
		return DXGI_FORMAT_R10G10B10A2_UNORM;

	case NGL_R16_G16_B16_FLOAT:
	case NGL_R16_G16_B16_A16_FLOAT:
		return DXGI_FORMAT_R16G16B16A16_FLOAT;

	case NGL_R16_G16_FLOAT:
		return DXGI_FORMAT_R16G16_FLOAT;

	case NGL_R32_G32_B32_FLOAT:
	case NGL_R32_G32_B32_A32_FLOAT:
		return DXGI_FORMAT_R32G32B32A32_FLOAT;

	case NGL_R16_FLOAT:
		return DXGI_FORMAT_R16_FLOAT;

	case NGL_R32_FLOAT:
		return DXGI_FORMAT_R32_FLOAT;

	case NGL_R8_G8_B8_DXT1_UNORM:
	case NGL_R8_G8_B8_A1_DXT1_UNORM:
		return DXGI_FORMAT_BC1_UNORM;

	case NGL_R8_G8_B8_DXT1_UNORM_SRGB:
	case NGL_R8_G8_B8_A1_DXT1_UNORM_SRGB:
		return DXGI_FORMAT_BC1_UNORM_SRGB;

	case NGL_R8_G8_B8_A8_DXT5_UNORM:
		return DXGI_FORMAT_BC3_UNORM;

	case NGL_R8_G8_B8_A8_DXT5_UNORM_SRGB:
		return DXGI_FORMAT_BC3_UNORM_SRGB;

	case NGL_R11_B11_B10_FLOAT:
		return DXGI_FORMAT_R11G11B10_FLOAT;

	case NGL_R9_G9_B9_E5_SHAREDEXP:
		return DXGI_FORMAT_R9G9B9E5_SHAREDEXP;

	/*
	case NGL_D16_UNORM:
	return DXGI_FORMAT_R16_TYPELESS;

	case NGL_D24_UNORM:
	return DXGI_FORMAT_R24G8_TYPELESS;
	*/

	case NGL_D16_UNORM:
	case NGL_D24_UNORM:
		return DXGI_FORMAT_R32_TYPELESS;

	default:
		_logf("DX12 - Unsupported texture NGL format: %d\n", (int32_t)ngl_format);
		assert(false);
		return DXGI_FORMAT_UNKNOWN;
	}
}


void DescribeTexture(const D3D12_texture &texture, const NGL_texture_descriptor &texture_layout, D3D12_RESOURCE_DESC &desc)
{
	memset(&desc, 0, sizeof(desc));

	desc.MipLevels = texture.m_num_mip_levels;
	desc.Format = texture.m_format;
	desc.Width = texture.m_width;
	desc.Height = texture.m_height;
	desc.DepthOrArraySize = texture.m_num_subtextures;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.Dimension = GetResourceDimension(texture.m_ngl_texture.m_texture_descriptor.m_type);
	
	desc.Flags = D3D12_RESOURCE_FLAG_NONE;
	if (texture_layout.m_is_renderable)
	{
		if (texture.m_ngl_texture.m_is_color)
		{
			desc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
		}
		else
		{
			desc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
		}
	}
	if (texture_layout.m_type == NGL_RENDERBUFFER)
	{
		desc.Flags |= D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE;
	}
	if (texture_layout.m_unordered_access)
	{
		desc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
	}
}


D3D12_RESOURCE_DIMENSION GetResourceDimension(NGL_texture_type ngl_type)
{
	switch (ngl_type)
	{
	case NGL_TEXTURE_2D:
	case NGL_TEXTURE_2D_ARRAY:
	case NGL_TEXTURE_CUBE:
	case NGL_RENDERBUFFER:
		return D3D12_RESOURCE_DIMENSION_TEXTURE2D;

	default:
		_logf("DX12: Unsupported texture type: %d\n", (int32_t)ngl_type);
		assert(false);
		return D3D12_RESOURCE_DIMENSION_UNKNOWN;
	}
}


const void *ConvertPixelData(const void *raw_data, NGL_format ngl_format, uint32_t width, uint32_t height, std::vector<uint8_t*> &temp_data)
{
	// TODO: Add floating point formats
	assert(ngl_format != NGL_R16_G16_B16_FLOAT);
	assert(ngl_format != NGL_R32_G32_B32_FLOAT);

	bool need_rgb82rgba8_conversion = false;
	need_rgb82rgba8_conversion |= ngl_format == NGL_R8_G8_B8_UNORM;
	need_rgb82rgba8_conversion |= ngl_format == NGL_R8_G8_B8_UNORM_SRGB;

	if (need_rgb82rgba8_conversion)
	{
		uint8_t *converted_data = new uint8_t[width * height * 4];
		temp_data.push_back(converted_data);

		uint8_t *tmp = new uint8_t[width*height * 4];

		const uint8_t *byte_data = (const uint8_t*)raw_data;

		for (uint32_t i = 0; i < width * height; i++)
		{
			converted_data[i * 4 + 0] = byte_data[i * 3 + 0];
			converted_data[i * 4 + 1] = byte_data[i * 3 + 1];
			converted_data[i * 4 + 2] = byte_data[i * 3 + 2];
			converted_data[i * 4 + 3] = 255;
		}

		return converted_data;
	}
	else
	{
		return raw_data;
	}
}


void DescribeSampler(const NGL_texture_descriptor &texture_layout, bool is_shadow_sampler, D3D12_SAMPLER_DESC &desc)
{
	memset(&desc, 0, sizeof(desc));

	desc.Filter = GetSamplerFilter(texture_layout.m_filter);
	desc.AddressU = GetWrapMode(texture_layout.m_wrap_mode);
	desc.AddressV = GetWrapMode(texture_layout.m_wrap_mode);
	desc.AddressW = GetWrapMode(texture_layout.m_wrap_mode);
	desc.MinLOD = 0.0f;
	desc.MaxLOD = float(texture_layout.m_num_levels) - 1.0f;
	desc.MipLODBias = 0.0f;
	desc.MaxAnisotropy = 1;
	
	if (is_shadow_sampler)
	{
		desc.ComparisonFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
		
		desc.Filter = D3D12_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
		desc.ComparisonFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
		desc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
		desc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
		desc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
		desc.MinLOD = 0.0f;
		desc.MaxLOD = D3D12_FLOAT32_MAX;
	}
	else
	{
		desc.ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;
	}
}


void DescribeStaticSampler(bool is_shadow_sampler, uint32_t base_register, D3D12_SHADER_VISIBILITY visibility, D3D12_STATIC_SAMPLER_DESC &desc, float min_level, float max_level)
{
	assert(is_shadow_sampler);

	memset(&desc, 0, sizeof(desc));

	desc.ShaderRegister = base_register;
	desc.RegisterSpace = 0;
	desc.ShaderVisibility = visibility;

	//desc.Filter = GetSamplerFilter(texture_layout.m_filter);
	//desc.AddressU = GetWrapMode(texture_layout.m_wrap_mode);
	//desc.AddressV = GetWrapMode(texture_layout.m_wrap_mode);
	//desc.AddressW = GetWrapMode(texture_layout.m_wrap_mode);
	//desc.MinLOD = min_level;
	//desc.MaxLOD = max_level;
	desc.MipLODBias = 0.0f;
	desc.MaxAnisotropy = 1;

	//if (is_shadow_sampler)
	{
		desc.ComparisonFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;

		desc.Filter = D3D12_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
		desc.ComparisonFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
		desc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
		desc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
		desc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
		desc.MinLOD = 0.0f;
		desc.MaxLOD = D3D12_FLOAT32_MAX;
	}
	//else
	//{
	//	desc.ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;
	//}
}


D3D12_FILTER GetSamplerFilter(NGL_texture_filter ngl_filter)
{
	switch (ngl_filter)
	{
	case NGL_NEAREST:
		return D3D12_FILTER_MIN_MAG_MIP_POINT;

	case NGL_NEAREST_MIPMAPPED:
		return D3D12_FILTER_MIN_MAG_MIP_POINT;

	case  NGL_LINEAR:
		return D3D12_FILTER_MIN_MAG_LINEAR_MIP_POINT;

	case  NGL_LINEAR_MIPMAPPED:
		return D3D12_FILTER_MIN_MAG_MIP_LINEAR;

	default:
		_logf("DX12: Unsupported texture filter type: %d\n", (int32_t)ngl_filter);
		assert(false);
		return D3D12_FILTER_MIN_MAG_MIP_POINT;
	}
}


D3D12_TEXTURE_ADDRESS_MODE GetWrapMode(NGL_texture_wrap_mode ngl_mode)
{
	switch (ngl_mode)
	{
	case NGL_REPEAT_ALL:
		return D3D12_TEXTURE_ADDRESS_MODE_WRAP;

	case NGL_CLAMP_ALL:
		return D3D12_TEXTURE_ADDRESS_MODE_CLAMP;

	default:
		_logf("DX12: Unsupported warp mode: %d\n", (int32_t)ngl_mode);
		assert(false);
		return D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	}
}


D3D12_SHADER_VISIBILITY GetShaderVisibility(NGL_shader_type shader_type)
{
	switch (shader_type)
	{
	case NGL_VERTEX_SHADER: return D3D12_SHADER_VISIBILITY_VERTEX;
	case NGL_TESS_CONTROL_SHADER: return D3D12_SHADER_VISIBILITY_HULL;
	case NGL_TESS_EVALUATION_SHADER: return D3D12_SHADER_VISIBILITY_DOMAIN;
	case NGL_GEOMETRY_SHADER: return D3D12_SHADER_VISIBILITY_GEOMETRY;
	case NGL_FRAGMENT_SHADER: return D3D12_SHADER_VISIBILITY_PIXEL;
	case NGL_COMPUTE_SHADER: return D3D12_SHADER_VISIBILITY_ALL;

	default:
		_logf("DX12 - Unknown shader type: %d", (int32_t)shader_type);
		assert(false);
		return D3D12_SHADER_VISIBILITY_ALL;
	}
}


D3D12_ROOT_SIGNATURE_FLAGS GetShaderDenyFlag(NGL_shader_type shader_type)
{
	switch (shader_type)
	{
	case NGL_VERTEX_SHADER: return D3D12_ROOT_SIGNATURE_FLAG_DENY_VERTEX_SHADER_ROOT_ACCESS;
	case NGL_TESS_CONTROL_SHADER: return D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS;
	case NGL_TESS_EVALUATION_SHADER: return D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS;
	case NGL_GEOMETRY_SHADER: return D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;
	case NGL_FRAGMENT_SHADER: return D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS;
	case NGL_COMPUTE_SHADER: return D3D12_ROOT_SIGNATURE_FLAG_NONE;

	default:
		_logf("DX12 - Unknown shader type: %d", (int32_t)shader_type);
		assert(false);
		return D3D12_ROOT_SIGNATURE_FLAG_NONE;
	}
}


void DescribeSRV(const D3D12_texture &texture, D3D12_SHADER_RESOURCE_VIEW_DESC &desc)
{
	const NGL_texture_type &type = texture.m_ngl_texture.m_texture_descriptor.m_type;

	memset(&desc, 0, sizeof(desc));

	desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

	switch (type)
	{
	case NGL_TEXTURE_2D:
	case NGL_RENDERBUFFER:
		desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		desc.Texture2D.MipLevels = texture.m_num_mip_levels;
		break;

	case NGL_TEXTURE_2D_ARRAY:
		desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
		desc.Texture2DArray.MipLevels = texture.m_num_mip_levels;
		desc.Texture2DArray.ArraySize = texture.m_num_surfaces;
		break;

	case NGL_TEXTURE_CUBE:
		desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
		desc.TextureCube.MipLevels = texture.m_num_mip_levels;
		break;

	default:
		_logf("DX12: Unsupported texture type: %d\n", (int32_t)type);
		assert(false);
		break;
	}

	if (texture.m_ngl_texture.m_texture_descriptor.m_is_renderable && texture.m_ngl_texture.m_is_color == false)
	{
		desc.Format = GetDepthTextureFormat(texture.m_format);
	}
	else
	{
		desc.Format = texture.m_format;
	}
}


void DescribeSubresourceSRV(const D3D12_texture &texture, uint32_t subresource_index, D3D12_SHADER_RESOURCE_VIEW_DESC &desc)
{
	const NGL_texture_type &type = texture.m_ngl_texture.m_texture_descriptor.m_type;
	NGL_texture_subresource subresource(texture.m_ngl_id);
	texture.DescribeSubresource(subresource_index, subresource);

	memset(&desc, 0, sizeof(desc));

	desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

	switch (type)
	{
	case NGL_TEXTURE_2D:
	case NGL_RENDERBUFFER:
		desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		desc.Texture2D.MipLevels = 1;
		desc.Texture2D.MostDetailedMip = subresource.m_level;
		break;

	case NGL_TEXTURE_2D_ARRAY:
		desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
		desc.Texture2DArray.MipLevels = 1;
		desc.Texture2DArray.ArraySize = 1;
		desc.Texture2DArray.MostDetailedMip = subresource.m_level;
		desc.Texture2DArray.FirstArraySlice = texture.m_num_faces > 1 ? subresource.m_face : subresource.m_layer;
		break;

	case NGL_TEXTURE_CUBE:
		desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
		desc.TextureCube.MipLevels = 1;
		desc.TextureCube.MostDetailedMip = subresource.m_level;
		break;

	default:
		_logf("DX12: Unsupported texture type: %d\n", (int32_t)type);
		assert(false);
		break;
	}

	if (texture.m_ngl_texture.m_texture_descriptor.m_is_renderable && texture.m_ngl_texture.m_is_color == false)
	{
		desc.Format = GetDepthTextureFormat(texture.m_format);
	}
	else
	{
		desc.Format = texture.m_format;
	}
}


DXGI_FORMAT GetDepthTextureFormat(DXGI_FORMAT format)
{
	switch (format)
	{
		// 32-bit Z w/ Stencil
	case DXGI_FORMAT_R32G8X24_TYPELESS:
	case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:
	case DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS:
	case DXGI_FORMAT_X32_TYPELESS_G8X24_UINT:
		return DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS;

		// No Stencil
	case DXGI_FORMAT_R32_TYPELESS:
	case DXGI_FORMAT_D32_FLOAT:
	case DXGI_FORMAT_R32_FLOAT:
		return DXGI_FORMAT_R32_FLOAT;

		// 24-bit Z
	case DXGI_FORMAT_R24G8_TYPELESS:
	case DXGI_FORMAT_D24_UNORM_S8_UINT:
	case DXGI_FORMAT_R24_UNORM_X8_TYPELESS:
	case DXGI_FORMAT_X24_TYPELESS_G8_UINT:
		return DXGI_FORMAT_R24_UNORM_X8_TYPELESS;

		// 16-bit Z w/o Stencil
	case DXGI_FORMAT_R16_TYPELESS:
	case DXGI_FORMAT_D16_UNORM:
	case DXGI_FORMAT_R16_UNORM:
		return DXGI_FORMAT_R16_UNORM;

	default:
		assert(false);
		return DXGI_FORMAT_UNKNOWN;
	}
}


void DescribeBlendState(NGL_blend_func blend_func, NGL_color_channel_mask blend_mask, D3D12_RENDER_TARGET_BLEND_DESC &desc)
{
	memset(&desc, 0, sizeof(desc));

	desc.BlendEnable = TRUE;
	desc.RenderTargetWriteMask = 0;
	desc.RenderTargetWriteMask += ((blend_mask | NGL_CHANNEL_R) > 0) ? D3D12_COLOR_WRITE_ENABLE_RED : 0;
	desc.RenderTargetWriteMask += ((blend_mask | NGL_CHANNEL_G) > 0) ? D3D12_COLOR_WRITE_ENABLE_GREEN : 0;
	desc.RenderTargetWriteMask += ((blend_mask | NGL_CHANNEL_B) > 0) ? D3D12_COLOR_WRITE_ENABLE_BLUE : 0;
	desc.RenderTargetWriteMask += ((blend_mask | NGL_CHANNEL_A) > 0) ? D3D12_COLOR_WRITE_ENABLE_ALPHA : 0;

	switch (blend_func)
	{
	case NGL_BLEND_ADDITIVE:
		desc.SrcBlend = D3D12_BLEND_ONE;
		desc.DestBlend = D3D12_BLEND_ONE;
		desc.BlendOp = D3D12_BLEND_OP_ADD;
		desc.SrcBlendAlpha = D3D12_BLEND_ONE;
		desc.DestBlendAlpha = D3D12_BLEND_ONE;
		desc.BlendOpAlpha = D3D12_BLEND_OP_ADD;
		break;

	case NGL_BLEND_ALFA:
		desc.SrcBlend = D3D12_BLEND_SRC_ALPHA;
		desc.DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
		desc.BlendOp = D3D12_BLEND_OP_ADD;
		desc.SrcBlendAlpha = D3D12_BLEND_SRC_ALPHA;
		desc.DestBlendAlpha = D3D12_BLEND_INV_SRC_ALPHA;
		desc.BlendOpAlpha = D3D12_BLEND_OP_ADD;
		break;

	case NGL_BLEND_ADDITIVE_ALFA:
		desc.SrcBlend = D3D12_BLEND_ONE;
		desc.SrcBlendAlpha = D3D12_BLEND_ONE;
		desc.DestBlend = D3D12_BLEND_SRC_ALPHA;
		desc.DestBlendAlpha = D3D12_BLEND_SRC_ALPHA;
		desc.BlendOp = D3D12_BLEND_OP_ADD;
		desc.BlendOpAlpha = D3D12_BLEND_OP_ADD;
		break;

	case NGL_BLEND_ADDITIVE_INVERSE_ALFA:
		desc.SrcBlend = D3D12_BLEND_ONE;
		desc.SrcBlendAlpha = D3D12_BLEND_ONE;
		desc.DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
		desc.DestBlendAlpha = D3D12_BLEND_INV_SRC_ALPHA;
		desc.BlendOp = D3D12_BLEND_OP_ADD;
		desc.BlendOpAlpha = D3D12_BLEND_OP_ADD;
		break;

	case NGL_BLEND_DECAL:
		desc.SrcBlend = D3D12_BLEND_DEST_COLOR;
		desc.SrcBlendAlpha = D3D12_BLEND_DEST_ALPHA;
		desc.DestBlend = D3D12_BLEND_SRC_COLOR;
		desc.DestBlendAlpha = D3D12_BLEND_SRC_ALPHA;
		desc.BlendOp = D3D12_BLEND_OP_ADD;
		desc.BlendOpAlpha = D3D12_BLEND_OP_ADD;
		break;

	case NGL_BLEND_MODULATIVE:
		desc.SrcBlend = D3D12_BLEND_DEST_COLOR;
		desc.SrcBlendAlpha = D3D12_BLEND_DEST_ALPHA;
		desc.DestBlend = D3D12_BLEND_ZERO;
		desc.DestBlendAlpha = D3D12_BLEND_ZERO;
		desc.BlendOp = D3D12_BLEND_OP_ADD;
		desc.BlendOpAlpha = D3D12_BLEND_OP_ADD;
		break;

	case NGL_BLEND_TRANSPARENT_ACCUMULATION:
		desc.SrcBlend = D3D12_BLEND_ONE;
		desc.SrcBlendAlpha = D3D12_BLEND_ZERO;
		desc.DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
		desc.DestBlendAlpha = D3D12_BLEND_INV_SRC_ALPHA;
		desc.BlendOp = D3D12_BLEND_OP_ADD;
		desc.BlendOpAlpha = D3D12_BLEND_OP_ADD;
		break;

	case NGL_BLEND_DISABLED:
		desc.BlendEnable = FALSE;
		break;

	default:
		_logf("DX12 - Unsupported blend func: %d\n", (int32_t)blend_func);
		desc.BlendEnable = FALSE;
		assert(false);
		break;
	}
}


void DescribeDepthStencilState(NGL_depth_func depth_func, bool depth_mask, D3D12_DEPTH_STENCIL_DESC &desc)
{
	memset(&desc, 0, sizeof(desc));

	desc.DepthEnable = TRUE;
	desc.StencilEnable = FALSE;
	desc.DepthWriteMask = depth_mask ? D3D12_DEPTH_WRITE_MASK_ALL : D3D12_DEPTH_WRITE_MASK_ZERO;

	switch (depth_func)
	{
	case NGL_DEPTH_LESS:
		desc.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
		break;
	case NGL_DEPTH_GREATER:
		desc.DepthFunc = D3D12_COMPARISON_FUNC_GREATER;
		break;
	case NGL_DEPTH_TO_FAR:
		desc.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
		break;
	case NGL_DEPTH_LESS_WITH_OFFSET:
		desc.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
		break;
	case NGL_DEPTH_ALWAYS:
		desc.DepthFunc = D3D12_COMPARISON_FUNC_ALWAYS;
		break;
	case NGL_DEPTH_EQUAL:
		desc.DepthFunc = D3D12_COMPARISON_FUNC_EQUAL;
		break;
	case NGL_DEPTH_LEQUAL:
		desc.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
		break;
	case NGL_DEPTH_DISABLED:
		desc.DepthEnable = FALSE;
		desc.DepthFunc = D3D12_COMPARISON_FUNC_NEVER;
		break;

	default:
		_logf("DX12 - Unsupported depth func: %d\n", (int32_t)depth_func);
		desc.DepthEnable = FALSE;
		assert(false);
		break;
	}
}


void DescribeDepthFuncRange(NGL_depth_func depth_func, float &depth_min, float &depth_max)
{
	depth_min = 0.0f;
	depth_max = 1.0f;

	if (depth_func == NGL_DEPTH_TO_FAR)
	{
		depth_min = 1.0f;
	}
}


D3D12_RESOURCE_STATES GetResourceState(NGL_resource_state ngl_state)
{
	switch (ngl_state)
	{
	case NGL_COLOR_ATTACHMENT:
		return D3D12_RESOURCE_STATE_RENDER_TARGET;

	case NGL_COLOR_ATTACHMENT_AND_INPUT_ATTACHMENT_AND_SHADER_RESOURCE:
		return D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;

	case NGL_COLOR_ATTACHMENT_AND_PRESERVED_ATTACHMENT:
		return D3D12_RESOURCE_STATE_RENDER_TARGET;

	case NGL_DEPTH_ATTACHMENT:
		return D3D12_RESOURCE_STATE_DEPTH_WRITE;

	case NGL_READ_ONLY_DEPTH_ATTACHMENT:
		return D3D12_RESOURCE_STATE_DEPTH_READ;

	case NGL_READ_ONLY_DEPTH_ATTACHMENT_AND_SHADER_RESOURCE:
		return D3D12_RESOURCE_STATE_DEPTH_READ | D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;

	case NGL_SHADER_RESOURCE:
		return D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;

	case NGL_SHADER_RESOURCE_AND_UNORDERED_ACCESS:
		return D3D12_RESOURCE_STATE_UNORDERED_ACCESS;

	case NGL_SHADER_RESOURCE_AND_NON_FRAGMENT_SHADER_RESOURCE:
		return D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;

	case NGL_SHADER_RESOURCE_AND_UNORDERED_ACCESS_AND_NON_FRAGMENT_SHADER_RESOURCE:
		return D3D12_RESOURCE_STATE_UNORDERED_ACCESS;

	default:
		assert(false);
		return D3D12_RESOURCE_STATE_COMMON;
	}
}


bool DescribeBarrier(ID3D12Resource *resource, uint32_t subresource_index, D3D12_RESOURCE_STATES old_state, D3D12_RESOURCE_STATES new_state, D3D12_RESOURCE_BARRIER &barrier)
{
	memset(&barrier, 0, sizeof(barrier));

	if (old_state == D3D12_RESOURCE_STATE_UNORDERED_ACCESS && new_state == D3D12_RESOURCE_STATE_UNORDERED_ACCESS)
	{
		// UAV barrier

		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
		barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		barrier.UAV.pResource = resource;

		return true;
	}
	else if (old_state != new_state)
	{
		// Transition barrier

		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		barrier.Transition.pResource = resource;
		barrier.Transition.Subresource = subresource_index; // D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		barrier.Transition.StateBefore = old_state;
		barrier.Transition.StateAfter = new_state;

		return true;
	}

	return false;
}


bool IsValidState(D3D12_RESOURCE_STATES state)
{
	if (state == NGL_DX12_RESOURCE_STATE_UNKNOWN)
	{
		_logf("UNKNOWN is not a valid state.");
		return false;
	}

	D3D12_RESOURCE_STATES read_states[] = {
		D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER,
		D3D12_RESOURCE_STATE_INDEX_BUFFER,
		D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
		D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT,
		D3D12_RESOURCE_STATE_COPY_SOURCE,
		D3D12_RESOURCE_STATE_DEPTH_READ
	};

	D3D12_RESOURCE_STATES read_write_states[] = {
		D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
		D3D12_RESOURCE_STATE_DEPTH_WRITE
	};

	D3D12_RESOURCE_STATES write_only_states[] = {
		D3D12_RESOURCE_STATE_COPY_DEST,
		D3D12_RESOURCE_STATE_RENDER_TARGET,
		D3D12_RESOURCE_STATE_STREAM_OUT
	};

	int num_read_states = 0;
	int num_write_states = 0;
	for (size_t i = 0; i < sizeof(read_states) / sizeof(read_states[0]); i++)
	{
		if ((state & read_states[i]) > 0)
		{
			num_read_states++;
		}
	}
	for (size_t i = 0; i < sizeof(read_write_states) / sizeof(read_write_states[0]); i++)
	{
		if ((state & read_write_states[i]) > 0)
		{
			num_write_states++;
		}
	}
	for (size_t i = 0; i < sizeof(write_only_states) / sizeof(write_only_states[0]); i++)
	{
		if ((state & write_only_states[i]) > 0)
		{
			num_write_states++;
		}
	}

	if (num_write_states > 1)
	{
		_logf("At most one write bit can be set. State list: %s", GetResourceStateList(state).c_str());
		return false;
	}

	if (num_write_states == 1 && num_read_states > 0)
	{
		_logf(" If any write bit is set, then no read bit may be set. State list: %s", GetResourceStateList(state).c_str());
		return false;
	}

	return true; // If no write bit is set, then any number of read bits may be set.
}


void DescribeBuffer(uint64_t size, D3D12_RESOURCE_DESC &desc)
{
	memset(&desc, 0, sizeof(desc));
	desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	desc.Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
	desc.Height = 1;
	desc.DepthOrArraySize = 1;
	desc.MipLevels = 1;
	desc.Format = DXGI_FORMAT_UNKNOWN;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

	desc.Width = size;
}
