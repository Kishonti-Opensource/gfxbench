/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "ngl_d3d12_resource.h"

#include "ngl_d3d12.h"
#include "ngl_d3d12_command.h"
#include "ngl_d3d12_util.h"


// D3D12_vertex_buffer //////////////////////////////////////////////////////////////////

D3D12_vertex_buffer::D3D12_vertex_buffer(uint32_t ngl_id)
	: D3D12_resource(ngl_id)
{
	m_resource_type = NGL_D3D12_RESOURCE_TYPE_VERTEX_BUFFER;
	m_srv_index = 0;
	m_uav_index = 0;
	m_resource_manager = nullptr;

	memset(&m_vertex_buffer_view_desc, 0, sizeof(m_vertex_buffer_view_desc));
}

bool D3D12_vertex_buffer::Init(NGL_vertex_descriptor &vertex_layout, uint32_t num_vertices)
{
	m_ngl_vertex_buffer.m_hash = GenerateHash(vertex_layout);
	m_ngl_vertex_buffer.m_vertex_descriptor = vertex_layout;
	m_ngl_vertex_buffer.m_datasize = num_vertices * vertex_layout.m_stride;
	m_num_vertices = num_vertices;

	return true;
}


uint64_t D3D12_vertex_buffer::GetSubresourceKey(const NGL_buffer_subresource &buffer_subresource)
{
	uint64_t key = buffer_subresource.m_offset;
	key <<= 32;
	key += buffer_subresource.m_size;
	return key;
}


uint32_t D3D12_vertex_buffer::GetSubresourceUAV(const NGL_buffer_subresource &buffer_subresource)
{
	assert(buffer_subresource.m_buffer == m_ngl_id);

	if (buffer_subresource.m_offset == 0 && buffer_subresource.m_size >= m_ngl_vertex_buffer.m_datasize)
	{
		return m_uav_index;
	}

	uint64_t key = GetSubresourceKey(buffer_subresource);
	auto i = m_subresource_uav_indices.find(key);
	if (i != m_subresource_uav_indices.end())
	{
		return i->second;
	}
	else
	{
#ifdef NGL_DX12_DEBUG_BUFFER_SUBRESOURCE_VIEWS
		_logf("Created buffer subresource UAV. buffer: %u offset: %u size: %u", buffer_subresource.m_buffer, buffer_subresource.m_offset, buffer_subresource.m_size);
#endif

		uint32_t uav_index = 0;
		uint32_t size = buffer_subresource.m_size == (uint32_t)~0 ? m_ngl_vertex_buffer.m_datasize - buffer_subresource.m_offset : buffer_subresource.m_size;
		D3D12_UNORDERED_ACCESS_VIEW_DESC uav_desc = m_base_uav_desc;
		uav_desc.Buffer.FirstElement = buffer_subresource.m_offset / m_ngl_vertex_buffer.m_vertex_descriptor.m_stride;
		uav_desc.Buffer.NumElements = size / m_ngl_vertex_buffer.m_vertex_descriptor.m_stride;
		m_resource_manager->m_backend->m_device->CreateUnorderedAccessView(m_res, nullptr, &uav_desc, m_resource_manager->m_view_heap.AddCPUHandle(uav_index));

		m_subresource_uav_indices[key] = uav_index;
		return uav_index;
	}
}


uint32_t D3D12_vertex_buffer::GetSubresourceSRV(const NGL_buffer_subresource &buffer_subresource)
{
	assert(buffer_subresource.m_buffer == m_ngl_id);

	if (buffer_subresource.m_offset == 0 && buffer_subresource.m_size >= m_ngl_vertex_buffer.m_datasize)
	{
		return m_srv_index;
	}

	uint64_t key = GetSubresourceKey(buffer_subresource);
	auto i = m_subresource_srv_indices.find(key);
	if (i != m_subresource_srv_indices.end())
	{
		return i->second;
	}
	else
	{
#ifdef NGL_DX12_DEBUG_BUFFER_SUBRESOURCE_VIEWS
		_logf("Created buffer subresource SRV. buffer: %u offset: %u size: %u", buffer_subresource.m_buffer, buffer_subresource.m_offset, buffer_subresource.m_size);
#endif

		uint32_t srv_index = 0;
		uint32_t size = buffer_subresource.m_size == (uint32_t)~0 ? m_ngl_vertex_buffer.m_datasize - buffer_subresource.m_offset : buffer_subresource.m_size;
		D3D12_SHADER_RESOURCE_VIEW_DESC srv_desc = m_base_srv_desc;
		srv_desc.Buffer.FirstElement = buffer_subresource.m_offset / m_ngl_vertex_buffer.m_vertex_descriptor.m_stride;
		srv_desc.Buffer.NumElements = size / m_ngl_vertex_buffer.m_vertex_descriptor.m_stride;
		m_resource_manager->m_backend->m_device->CreateShaderResourceView(m_res, &srv_desc, m_resource_manager->m_view_heap.AddCPUHandle(srv_index));

		m_subresource_srv_indices[key] = srv_index;
		return srv_index;
	}
}


// D3D12_index_buffer //////////////////////////////////////////////////////////////////

D3D12_index_buffer::D3D12_index_buffer(uint32_t ngl_id)
	: D3D12_resource(ngl_id)
{
	m_resource_type = NGL_D3D12_RESOURCE_TYPE_INDEX_BUFFER;

	m_num_indices = 0;
	m_stride = 0;
	m_size = 0;
	memset(&m_index_buffer_view_desc, 0, sizeof(D3D12_INDEX_BUFFER_VIEW));
}


bool D3D12_index_buffer::Init(NGL_format index_format, uint32_t num_indices)
{
	m_ngl_index_buffer.m_format = index_format;
	m_ngl_index_buffer.m_num_indices = num_indices;
	m_num_indices = num_indices;

	if (!GetIndexFormat(index_format, m_format, m_stride))
	{
		return false;
	}

	m_size = m_num_indices * m_stride;

	return true;
}


// D3D12_texture //////////////////////////////////////////////////////////////////

D3D12_texture::D3D12_texture(uint32_t ngl_id)
	: D3D12_resource(ngl_id)
{
	m_resource_type = NGL_D3D12_RESOURCE_TYPE_TEXTURE;

	m_width = 0;
	m_height = 0;

	m_num_surfaces = 0;
	m_num_faces = 0;
	m_num_depth_slices = 0;
	m_num_mip_levels = 0;

	m_num_subtextures = 0;
	m_num_subresources = 0;

	m_block_size = 0; // in bits
	m_block_dim_x = 1;
	m_block_dim_y = 1;

	m_default_clear_value = nullptr;

	m_srv_offset = 0;
	m_uav_offset = 0;
}

D3D12_texture::~D3D12_texture()
{
	delete m_default_clear_value;
}

void D3D12_texture::InitNull(uint32_t width, uint32_t height)
{
	m_width = width;
	m_height = height;

	m_num_surfaces = 1;
	m_num_faces = 1;
	m_num_depth_slices = 1;
	m_num_mip_levels = 1;

	m_num_subtextures = 1;
	m_num_subresources = 1;
}

void D3D12_texture::Init(const NGL_texture_descriptor &texture_layout, uint32_t num_data_subresources)
{
	m_ngl_texture.m_texture_descriptor = texture_layout;
	SetDataFormat(texture_layout.m_format);
	SetDimensions(num_data_subresources);

	if (texture_layout.m_is_renderable)
	{
		if (m_ngl_texture.m_is_color)
		{
			m_default_clear_value = new CD3DX12_CLEAR_VALUE(m_format, texture_layout.m_clear_value);
		}
		else
		{
			m_default_clear_value = new CD3DX12_CLEAR_VALUE(GetDSVFormat(m_format), texture_layout.m_clear_value[0], (uint8_t)texture_layout.m_clear_value[1]);
		}
	}
}


void D3D12_texture::SetDataFormat(NGL_format ngl_format)
{
	m_format = GetDXGITextureFormat(ngl_format);

	m_ngl_texture.m_is_color = true;
	m_block_size = 0;
	m_block_dim_x = 1;
	m_block_dim_y = 1;

	switch (ngl_format)
	{
	case NGL_R8_UNORM:
		m_block_size = 8;
		break;

	case NGL_R8_G8_B8_UNORM:
	case NGL_R8_G8_B8_UNORM_SRGB:
	case NGL_R8_G8_B8_A8_UNORM:
	case NGL_R8_G8_B8_A8_UNORM_SRGB:
	case NGL_R10_G10_B10_A2_UNORM:
		m_block_size = 32;
		break;

	case NGL_R16_G16_B16_FLOAT:
	case NGL_R16_G16_B16_A16_FLOAT:
		m_block_size = 64;
		break;

	case NGL_R16_G16_FLOAT:
		m_block_size = 32;
		break;

	case NGL_R32_G32_B32_FLOAT:
	case NGL_R32_G32_B32_A32_FLOAT:
		m_block_size = 128;
		break;

	case NGL_R16_FLOAT:
		m_block_size = 16;
		break;

	case NGL_R32_FLOAT:
		m_block_size = 32;
		break;

	case NGL_R8_G8_B8_DXT1_UNORM:
	case NGL_R8_G8_B8_A1_DXT1_UNORM:
	case NGL_R8_G8_B8_DXT1_UNORM_SRGB:
	case NGL_R8_G8_B8_A1_DXT1_UNORM_SRGB:
		m_block_dim_x = 4;
		m_block_dim_y = 4;
		m_block_size = 64;
		break;

	case NGL_R8_G8_B8_A8_DXT5_UNORM:
	case NGL_R8_G8_B8_A8_DXT5_UNORM_SRGB:
		m_block_dim_x = 4;
		m_block_dim_y = 4;
		m_block_size = 128;
		break;

	case NGL_R11_B11_B10_FLOAT:
		m_block_size = 32;
		break;

	case NGL_R9_G9_B9_E5_SHAREDEXP:
		m_block_size = 32;
		break;

		/*
		case NGL_D16_UNORM:
		m_ngl_texture.m_is_color = false;
		m_block_size = 16;
		break;

		case NGL_D24_UNORM:
		m_ngl_texture.m_is_color = false;
		m_block_size = 32;
		break;
		*/

	case NGL_D16_UNORM:
	case NGL_D24_UNORM:
		m_ngl_texture.m_is_color = false;
		m_block_size = 32;
		break;

	default:
		_logf("DX12: Unsupported texture format: %d\n", (int32_t)ngl_format);
		assert(false);
		break;
	}
}


void D3D12_texture::SetDimensions(uint32_t num_data_subresources)
{
	const NGL_texture_descriptor texture_layout = m_ngl_texture.m_texture_descriptor;

	m_width = texture_layout.m_size[0];
	m_height = texture_layout.m_size[1];
	m_num_mip_levels = texture_layout.m_num_levels;
	m_num_surfaces = 1;
	m_num_faces = 1;
	m_num_depth_slices = 1;

	switch (texture_layout.m_type)
	{
	case NGL_TEXTURE_2D:
		break;

	case NGL_TEXTURE_2D_ARRAY:
		m_num_surfaces = texture_layout.m_num_array;
		assert(m_num_surfaces > 0);
		break;

	case NGL_TEXTURE_CUBE:
		m_num_faces = 6;
		break;

	case NGL_RENDERBUFFER:
		break;

	default:
		_logf("DX12: Unsupported texture type: %d\n", (int32_t)texture_layout.m_type);
		assert(false);
		break;
	}

	m_num_subtextures = m_num_surfaces * m_num_faces * m_num_depth_slices;

	if (num_data_subresources > 0 && m_num_mip_levels > num_data_subresources / m_num_subtextures)
	{
		_logf("DX12 - Texture data does not contain enough mipmap levels! Mipmapping won't be used!");
		m_num_mip_levels = 1;
	}

	m_num_subresources = m_num_subtextures * m_num_mip_levels;
}


// D3D12_resource_manager //////////////////////////////////////////////////////////////////

D3D12_resource_manager::D3D12_resource_manager(D3D12_backend *backend)
{
	m_backend = backend;
}


D3D12_resource_manager::~D3D12_resource_manager()
{
	for (size_t i = 0; i < m_vertex_buffers.size(); i++)
	{
		delete m_vertex_buffers[i];
	}
	for (size_t i = 0; i < m_index_buffers.size(); i++)
	{
		delete m_index_buffers[i];
	}
	for (size_t i = 0; i < m_textures.size(); i++)
	{
		delete m_textures[i];
	}
}


void D3D12_resource_manager::InitTextures()
{
	m_sampler_cache.clear();

	// Init heaps

	m_view_heap.Init(m_backend->m_device, 512 * 8, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	m_sampler_heap.Init(m_backend->m_device, 512 * 4, D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
	m_color_attachment_heap.Init(m_backend->m_device, 64, D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	m_depth_attachment_heap.Init(m_backend->m_device, 64, D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
	m_view_heap.SetName("global_view_heap");
	m_sampler_heap.SetName("global_sampler_heap");
	m_color_attachment_heap.SetName("color_attachment_heap");
	m_depth_attachment_heap.SetName("depth_attachment_heap");

	m_view_bind_heap.Init(&m_backend->m_resource_mgr->m_view_heap, m_backend->m_direct_queue, 512 * 16 * 4);
	m_sampler_bind_heap.Init(&m_backend->m_resource_mgr->m_sampler_heap, m_backend->m_direct_queue, 512 * 4);
	m_view_bind_heap.SetName("view_bind_heap");
	m_sampler_bind_heap.SetName("sampler_bind_heap");

	ID3D12DescriptorHeap* ppDescriptorHeaps[2] =
	{
		m_view_bind_heap.m_heap.m_heap,
		m_sampler_bind_heap.m_heap.m_heap
	};

	m_backend->m_main_context->Reset();
	m_backend->m_main_context->m_command_list->SetDescriptorHeaps(2, ppDescriptorHeaps);
	m_backend->m_main_context->CloseExecute();

	// Create "null" texture target (proxy for NGL_DX12_SYSTEM_ATTACHMENT)

	D3D12_texture *null_texture = new D3D12_texture(0);
	null_texture->InitNull(m_backend->m_width, m_backend->m_height);
	null_texture->m_ngl_texture.m_texture_descriptor.m_name = "SYSTEM_ATTACHMENT";

	D3D12_SHADER_RESOURCE_VIEW_DESC nullShaderResourceViewDesc = {};
	nullShaderResourceViewDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	nullShaderResourceViewDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	nullShaderResourceViewDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	nullShaderResourceViewDesc.Texture2D.MipLevels = 1;
	nullShaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
	nullShaderResourceViewDesc.Texture2D.ResourceMinLODClamp = 0.0f;

	m_backend->m_device->CreateShaderResourceView(nullptr, &nullShaderResourceViewDesc, m_view_heap.AddCPUHandle(null_texture->m_srv_offset));

	// Create sampler for "null" texture (doesn't have shadow sampler)

	D3D12_SAMPLER_DESC sampler_desc;
	memset(&sampler_desc, 0, sizeof(sampler_desc));

	sampler_desc.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
	sampler_desc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	sampler_desc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	sampler_desc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;

	sampler_desc.MinLOD = 0;
	sampler_desc.MaxLOD = D3D12_FLOAT32_MAX;
	sampler_desc.MipLODBias = 0.0f;
	sampler_desc.MaxAnisotropy = 1;
	sampler_desc.ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;

	UINT sampler_offset;
	m_backend->m_device->CreateSampler(&sampler_desc, m_sampler_heap.AddCPUHandle(sampler_offset));
	null_texture->m_sampler_offsets.push_back(sampler_offset);

	m_textures.push_back(null_texture);
}


#if 0
void D3D12_resource_manager::TrackResource(D3D12_resource &resource)
{
	assert(resource.m_res != nullptr);
	assert(resource.m_resource_id == (uint32_t)-1);

	resource.m_resource_id = (uint32_t)m_tracked_resources.size();
	m_tracked_resources.push_back(&resource);
}
#endif


bool D3D12_resource_manager::CreateVertexBuffer(uint32_t &buffer_id, NGL_vertex_descriptor &vertex_layout, uint32_t num_vertices, void *data)
{
	if (buffer_id > 0 && buffer_id >= m_vertex_buffers.size())
	{
		_logf("DX12 - Illegal vertex buffer id: %d", (int32_t)buffer_id);
		buffer_id = 0xBADF00D;
		return false;
	}
	
	// Create vertex buffer

	if (buffer_id == 0)
	{
		buffer_id = (uint32_t)m_vertex_buffers.size();
		D3D12_vertex_buffer *vb = new D3D12_vertex_buffer(buffer_id);
		m_vertex_buffers.push_back(vb);
	}

	D3D12_vertex_buffer &vb = *m_vertex_buffers[buffer_id];
	vb.Init(vertex_layout, num_vertices);
	
	// Create vertex buffer resource

	CD3DX12_RESOURCE_DESC desc = CD3DX12_RESOURCE_DESC::Buffer(vb.m_ngl_vertex_buffer.m_datasize);
	if (vertex_layout.m_unordered_access)
	{
		desc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
	}

	D3D12_RESOURCE_STATES new_state;
	if (vertex_layout.m_unordered_access)
	{
		new_state = GetResourceState(NGL_SHADER_RESOURCE_AND_UNORDERED_ACCESS_AND_NON_FRAGMENT_SHADER_RESOURCE);
	}
	else
	{
		new_state = D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
	}

	vb.m_current_state = new_state;

	ThrowIfFailed(m_backend->m_device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&desc,
		vb.m_current_state,
		nullptr,
		IID_PPV_ARGS(&vb.m_res)));

	vb.SetName("vertex_buffer");

	// Upload the vertex buffer

	m_backend->m_main_context->Reset();

	ID3D12Resource *upload_heap = nullptr;

	if (data != nullptr)
	{
		// Schedule a copy to get data in the buffer
		m_backend->m_main_context->AddResourceBarrier(&vb, 0, vb.m_current_state, D3D12_RESOURCE_STATE_COPY_DEST);

		ThrowIfFailed(m_backend->m_device->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(vb.m_ngl_vertex_buffer.m_datasize),
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&upload_heap)));

		// copy data to the intermediate upload heap and then schedule a copy from the upload heap to the default texture
		D3D12_SUBRESOURCE_DATA buffer_res = {};
		buffer_res.pData = data;
		buffer_res.RowPitch = vb.m_ngl_vertex_buffer.m_datasize;
		buffer_res.SlicePitch = buffer_res.RowPitch;
		UpdateSubresources<1>(m_backend->m_main_context->m_command_list, vb.m_res, upload_heap, 0, 0, 1, &buffer_res);
	}

	m_backend->m_main_context->AddResourceBarrier(&vb, 0, vb.m_current_state, new_state);// D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
	uint64_t fence_value = m_backend->m_main_context->CloseExecute();
	m_backend->m_main_context->Discard(fence_value);
	m_backend->m_direct_queue->WaitForCompletion(fence_value);

	SAFE_RELEASE(upload_heap);

	if (vertex_layout.m_unordered_access)
	{
		m_backend->m_main_context->TrackBufferState(vb, vb.m_current_state);
		vb.m_resource_manager = this;

		// Create the UAV

		D3D12_UNORDERED_ACCESS_VIEW_DESC uav_desc;
		memset(&uav_desc, 0, sizeof(uav_desc));

		uav_desc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
		uav_desc.Format = vb.m_format;
		uav_desc.Buffer.StructureByteStride = vb.m_ngl_vertex_buffer.m_vertex_descriptor.m_stride;
		uav_desc.Buffer.NumElements = vb.m_ngl_vertex_buffer.m_datasize / vb.m_ngl_vertex_buffer.m_vertex_descriptor.m_stride;
		m_backend->m_device->CreateUnorderedAccessView(vb.m_res, nullptr, &uav_desc, m_view_heap.AddCPUHandle(vb.m_uav_index));

		vb.m_base_uav_desc = uav_desc;

		// Create the SRV

		D3D12_SHADER_RESOURCE_VIEW_DESC srv_desc;
		memset(&srv_desc, 0, sizeof(srv_desc));

		srv_desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srv_desc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
		srv_desc.Format = vb.m_format;
		srv_desc.Buffer.StructureByteStride = vb.m_ngl_vertex_buffer.m_vertex_descriptor.m_stride;
		srv_desc.Buffer.NumElements = vb.m_ngl_vertex_buffer.m_datasize / vb.m_ngl_vertex_buffer.m_vertex_descriptor.m_stride;
		m_backend->m_device->CreateShaderResourceView(vb.m_res, &srv_desc, m_view_heap.AddCPUHandle(vb.m_srv_index));

		vb.m_base_srv_desc = srv_desc;
	}

	vb.m_vertex_buffer_view_desc.BufferLocation = vb.m_res->GetGPUVirtualAddress();
	vb.m_vertex_buffer_view_desc.StrideInBytes = vb.m_ngl_vertex_buffer.m_vertex_descriptor.m_stride;
	vb.m_vertex_buffer_view_desc.SizeInBytes = vb.m_ngl_vertex_buffer.m_datasize;

	return true;
}


bool D3D12_resource_manager::CreateIndexBuffer(uint32_t &buffer_id, NGL_format index_format, uint32_t num_indices, void *data)
{
	if (buffer_id > 0 && buffer_id >= m_index_buffers.size())
	{
		_logf("DX12 - Illegal index buffer id: %d", (int32_t)buffer_id);
		buffer_id = 0xBADF00D;
		return false;
	}
	
	// Create index buffer

	if (buffer_id == 0)
	{
		buffer_id = (uint32_t)m_index_buffers.size();
		D3D12_index_buffer *ib = new D3D12_index_buffer(buffer_id);
		m_index_buffers.push_back(ib);
	}

	D3D12_index_buffer &ib = *m_index_buffers[buffer_id];
	if (!ib.Init(index_format, num_indices))
	{
		return false;
	}
	
	// Create index buffer resource

	ib.m_current_state = D3D12_RESOURCE_STATE_COMMON;

	ThrowIfFailed(m_backend->m_device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(ib.m_size),
		ib.m_current_state,
		nullptr,
		IID_PPV_ARGS(&ib.m_res)));
	
	ib.SetName("index_buffer");
	
	// Upload the index buffer

	m_backend->m_main_context->Reset();

	ID3D12Resource *upload_heap = nullptr;

	if (data != nullptr)
	{
		// Schedule a copy to get data in the buffer
		m_backend->m_main_context->AddResourceBarrier(&ib, 0, ib.m_current_state, D3D12_RESOURCE_STATE_COPY_DEST);

		ThrowIfFailed(m_backend->m_device->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(ib.m_size),
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&upload_heap)));

		// Copy data to the intermediate upload heap and then schedule a copy from the upload heap to the default texture
		D3D12_SUBRESOURCE_DATA buffer_res = {};
		buffer_res.pData = data;
		buffer_res.RowPitch = ib.m_size;
		buffer_res.SlicePitch = buffer_res.RowPitch;
		UpdateSubresources<1>(m_backend->m_main_context->m_command_list, ib.m_res, upload_heap, 0, 0, 1, &buffer_res);
	}

	m_backend->m_main_context->AddResourceBarrier(&ib, 0, ib.m_current_state, D3D12_RESOURCE_STATE_INDEX_BUFFER);
	uint64_t fence_value = m_backend->m_main_context->CloseExecute();
	m_backend->m_main_context->Discard(fence_value);
	m_backend->m_direct_queue->WaitForCompletion(fence_value);
	
	SAFE_RELEASE(upload_heap);

	ib.m_index_buffer_view_desc.BufferLocation = ib.m_res->GetGPUVirtualAddress();
	ib.m_index_buffer_view_desc.Format = ib.m_format;
	ib.m_index_buffer_view_desc.SizeInBytes = ib.m_size;

	return true;
}


bool D3D12_resource_manager::CreateTexture(uint32_t &texture_id, NGL_texture_descriptor &texture_layout, std::vector< std::vector<uint8_t> > *data)
{
	if (texture_layout.m_is_renderable && data != nullptr)
	{
		_logf("DX12 - Illegal texture!");
		texture_id = 0xBADF00D;
		return false;
	}

	if (texture_id > 0 && texture_id >= m_textures.size())
	{
		_logf("DX12 - Illegal texture id: %d", (int32_t)texture_id);
		texture_id = 0xBADF00D;
		return false;
	}
	
	// Create texture

	if (texture_id == 0)
	{
		texture_id = (uint32_t)m_textures.size();
		D3D12_texture *texture = new D3D12_texture(texture_id);
		m_textures.push_back(texture);
	}

	const uint32_t num_data_subresources = data != nullptr ? (uint32_t)data->size() : 0;

	D3D12_texture &texture = *m_textures[texture_id];
	texture.Init(texture_layout, num_data_subresources);

	// Create the texture resource

	D3D12_RESOURCE_DESC texture_desc;
	DescribeTexture(texture, texture_layout, texture_desc);
	
	D3D12_RESOURCE_STATES new_state;
	if (texture_layout.m_unordered_access)
	{
		new_state = GetResourceState(NGL_SHADER_RESOURCE_AND_UNORDERED_ACCESS_AND_NON_FRAGMENT_SHADER_RESOURCE);
	}
	else if (texture_layout.m_is_renderable && texture.m_ngl_texture.m_is_color)
	{
		new_state = GetResourceState(NGL_COLOR_ATTACHMENT);
	}
	else if (texture_layout.m_is_renderable && !texture.m_ngl_texture.m_is_color)
	{
		new_state = GetResourceState(NGL_DEPTH_ATTACHMENT);
	}
	else
	{
		new_state = GetResourceState(NGL_SHADER_RESOURCE);
	}

	texture.m_current_state = new_state;

	ThrowIfFailed(m_backend->m_device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&texture_desc,
		texture.m_current_state,
		texture.m_default_clear_value,
		IID_PPV_ARGS(&texture.m_res)));

	texture.SetName(texture_layout.m_name);
		
	// Upload the texture

	if (texture_layout.m_is_renderable == false && num_data_subresources > 0)
	{
		ID3D12Resource *upload_heap = nullptr;

		// Create an intermediate upload heap

		const uint64_t upload_buffer_size = GetRequiredIntermediateSize(texture.m_res, 0, texture.m_num_subresources);
		ThrowIfFailed(m_backend->m_device->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(upload_buffer_size),
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&upload_heap)));

		// Init the subresource descriptors (Convert the pixel data if needed)

		std::vector<uint8_t*> temp_data;
		std::vector<D3D12_SUBRESOURCE_DATA> subresource_data;
		InitTextureData(texture, data, subresource_data, temp_data);

		// Upload the texture data

		m_backend->m_main_context->Reset();

		m_backend->m_main_context->AddTextureBarrier(&texture, texture.m_current_state, D3D12_RESOURCE_STATE_COPY_DEST);
		UpdateSubresources(m_backend->m_main_context->m_command_list, texture.m_res, upload_heap, 0, 0, texture.m_num_subresources, subresource_data.data());
		m_backend->m_main_context->AddTextureBarrier(&texture, texture.m_current_state, new_state);//D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

		uint64_t fence_value = m_backend->m_main_context->CloseExecute();
		m_backend->m_main_context->Discard(fence_value);
		m_backend->m_direct_queue->WaitForCompletion(fence_value);

		// Cleanup

		SAFE_RELEASE(upload_heap);

		for (size_t i = 0; i < temp_data.size(); i++)
		{
			delete[] temp_data[i];
		}
	}
	
	// Create resource views

	if (texture_layout.m_type != NGL_RENDERBUFFER)
	{
		// Create the sampler

		D3D12_SAMPLER_DESC sampler_desc;
		DescribeSampler(texture_layout, false, sampler_desc);

		if (m_sampler_cache.find(sampler_desc) != m_sampler_cache.end())
		{
			texture.m_sampler_offsets.push_back(m_sampler_cache.at(sampler_desc));
		}
		else
		{
			UINT sampler_offset;
			m_backend->m_device->CreateSampler(&sampler_desc, m_sampler_heap.AddCPUHandle(sampler_offset));
			texture.m_sampler_offsets.push_back(sampler_offset);
			m_sampler_cache[sampler_desc] = sampler_offset;
		}

		// Create the shadow sampler

		//DescribeSampler(texture_layout, true, sampler_desc, 0.0f, float(texture.m_num_mip_levels) - 1.0f);

		//m_backend->m_device->CreateSampler(&sampler_desc, m_sampler_heap.AddCPUHandle(sampler_offset));
		//texture.m_sampler_offsets.push_back(sampler_offset);

		// Create the SRVs

		D3D12_SHADER_RESOURCE_VIEW_DESC srv_desc;
		DescribeSRV(texture, srv_desc);

		m_backend->m_device->CreateShaderResourceView(texture.m_res, &srv_desc, m_view_heap.AddCPUHandle(texture.m_srv_offset));

		if (texture.m_num_subresources > 1)
		{
			texture.m_subresource_srv_offsets.resize(texture.m_num_subresources);
			for (uint32_t i = 0; i < texture.m_num_subresources; i++)
			{
				DescribeSubresourceSRV(texture, i, srv_desc);

				m_backend->m_device->CreateShaderResourceView(texture.m_res, &srv_desc, m_view_heap.AddCPUHandle(texture.m_subresource_srv_offsets[i]));
			}
		}
	}

	if (texture_layout.m_unordered_access)
	{
		assert(texture_layout.m_is_renderable == false);
		assert(texture_layout.m_type != NGL_RENDERBUFFER);

		m_backend->m_main_context->TrackTextureState(texture, texture.m_current_state);

		D3D12_UNORDERED_ACCESS_VIEW_DESC uav_desc;
		memset(&uav_desc, 0, sizeof(uav_desc));

		uav_desc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
		uav_desc.Format = texture.m_format;
		uav_desc.Texture2D.MipSlice = 0;
		uav_desc.Texture2D.PlaneSlice = 0;
		m_backend->m_device->CreateUnorderedAccessView(texture.m_res, nullptr, &uav_desc, m_view_heap.AddCPUHandle(texture.m_uav_offset));
	}

	if (texture_layout.m_is_renderable)
	{
		assert(texture_layout.m_unordered_access == false);

		m_backend->m_main_context->TrackTextureState(texture, texture.m_current_state);
		
		if (texture.m_ngl_texture.m_is_color)
		{
			texture.m_rtv_offsets.resize(texture.m_num_subresources);			
			D3D12_RENDER_TARGET_VIEW_DESC rtv_desc;

			for (uint32_t mip_level = 0; mip_level < texture.m_num_mip_levels; mip_level++)
			{
				for (uint32_t surface_index = 0; surface_index < texture.m_num_surfaces; surface_index++)
				{
					for (uint32_t face_index = 0; face_index < texture.m_num_faces; face_index++)
					{
						for (uint32_t depth_slice_index = 0; depth_slice_index < texture.m_num_depth_slices; depth_slice_index++)
						{
							uint32_t subresource_id = texture.GetSubresourceIndex(mip_level, surface_index, face_index, depth_slice_index);
							uint32_t subtexture_id = texture.GetSubtextureIndex(surface_index, face_index, depth_slice_index);

							DescribeRTV(texture, mip_level, subtexture_id, rtv_desc);
							m_backend->m_device->CreateRenderTargetView(texture.m_res, &rtv_desc, m_color_attachment_heap.AddCPUHandle(texture.m_rtv_offsets[subresource_id]));
						}
					}
				}
			}
		}
		else
		{
			texture.m_dsv_offsets.resize(2 * texture.m_num_subresources);
			D3D12_DEPTH_STENCIL_VIEW_DESC dsv_desc;

			for (uint32_t is_read_only = 0; is_read_only < 2; is_read_only++)
			{
				for (uint32_t mip_level = 0; mip_level < texture.m_num_mip_levels; mip_level++)
				{
					for (uint32_t surface_index = 0; surface_index < texture.m_num_surfaces; surface_index++)
					{
						for (uint32_t face_index = 0; face_index < texture.m_num_faces; face_index++)
						{
							for (uint32_t depth_slice_index = 0; depth_slice_index < texture.m_num_depth_slices; depth_slice_index++)
							{
								uint32_t subresource_id = texture.GetSubresourceIndex(mip_level, surface_index, face_index, depth_slice_index);
								uint32_t subtexture_id = texture.GetSubtextureIndex(surface_index, face_index, depth_slice_index);

								DescribeDSV(texture, mip_level, subtexture_id, is_read_only > 0, dsv_desc);
								m_backend->m_device->CreateDepthStencilView(texture.m_res, &dsv_desc, m_depth_attachment_heap.AddCPUHandle(texture.m_dsv_offsets[2 * subresource_id + is_read_only]));
							}
						}
					}
				}
			}
		}
	}

	return true;
}


void D3D12_resource_manager::InitTextureData(const D3D12_texture &texture, std::vector<std::vector<uint8_t> > *data, std::vector<D3D12_SUBRESOURCE_DATA> &subresource_data, std::vector<uint8_t*> &temp_data)
{
	subresource_data.resize(texture.m_num_subresources);

	uint32_t src_index = 0;
	for (uint32_t mip_level = 0; mip_level < texture.m_num_mip_levels; mip_level++)
	{
		// Calculate the dimensions of the mipmap
		uint32_t mipmap_width = texture.m_width / (1 << mip_level);
		uint32_t mipmap_height = texture.m_height / (1 << mip_level);
		mipmap_width = mipmap_width ? mipmap_width : 1;
		mipmap_height = mipmap_height ? mipmap_height : 1;

		// Calculate the number of blocks for the mipmap
		uint32_t block_count_x = (mipmap_width + texture.m_block_dim_x - 1) / texture.m_block_dim_x;
		uint32_t block_count_y = (mipmap_height + texture.m_block_dim_y - 1) / texture.m_block_dim_y;

		for (uint32_t surface_index = 0; surface_index < texture.m_num_surfaces; surface_index++)
		{
			for (uint32_t face_index = 0; face_index < texture.m_num_faces; face_index++)
			{
				for (uint32_t depth_slice_index = 0; depth_slice_index < texture.m_num_depth_slices; depth_slice_index++)
				{
					// Convert the pixel data if needed
					const void *raw_data = ConvertPixelData((*data)[src_index].data(), texture.m_ngl_texture.m_texture_descriptor.m_format, mipmap_width, mipmap_height, temp_data);

					uint32_t subresource_id = texture.GetSubresourceIndex(mip_level, surface_index, face_index, depth_slice_index);

					subresource_data[subresource_id].pData = raw_data;
					subresource_data[subresource_id].RowPitch = block_count_x * texture.m_block_size / 8;
					subresource_data[subresource_id].SlicePitch = block_count_y * subresource_data[subresource_id].RowPitch;

					src_index++;
				}
			}
		}
	}
}


bool D3D12_resource_manager::GetTextureContent(uint32_t texture_id, uint32_t mip_level, uint32_t surface_index, uint32_t face_index, NGL_format format, NGL_resource_state state, uint32_t &width, uint32_t &height, std::vector<uint8_t> &data)
{
	if ((format != NGL_R8_G8_B8_UNORM) && (format != NGL_R8_G8_B8_A8_UNORM))
	{
		_logf("DX11 - GetTextureContent: Requested format is not NGL_R8_G8_B8_UNORM or NGL_R8_G8_B8_A8_UNORM !");
		return false;
	}

	D3D12_texture *src_tex = nullptr;
	uint32_t subresource_index = 0;

	if (texture_id == 0)
	{
		src_tex = m_backend->m_swap_chain->GetCurrentSwapBuffer();
	}
	else
	{
		src_tex = m_textures[texture_id];
		subresource_index = src_tex->GetSubresourceIndex(mip_level, surface_index, face_index, 0);
	}

	D3D12_RESOURCE_DESC src_desc = src_tex->m_res->GetDesc();

	// Query the size of the copiable data

	D3D12_PLACED_SUBRESOURCE_FOOTPRINT psd = {};
	UINT num_rows = 0;
	UINT64 row_size_in_bytes = 0;
	UINT64 total_bytes = 0;
	m_backend->m_device->GetCopyableFootprints(&src_desc, subresource_index, 1, 0, &psd, &num_rows, &row_size_in_bytes, &total_bytes);

	// Create a destination buffer resource

	ID3D12Resource *dst_res = nullptr;
	D3D12_RESOURCE_DESC dst_desc;
	DescribeBuffer(total_bytes, dst_desc);

	HRESULT result = m_backend->m_device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_READBACK),
		D3D12_HEAP_FLAG_NONE,
		&dst_desc,
		D3D12_RESOURCE_STATE_COPY_DEST,
		nullptr,
		IID_PPV_ARGS(&dst_res));

	if (FAILED(result))
	{
		return false;
	}

	std::vector<uint8_t> src_data;
	src_data.resize((size_t)total_bytes);

	// Copy the resource to the staging resource

	if (texture_id > 0)
	{
		src_tex->m_current_state = GetResourceState(state);
	}

	D3D12_RESOURCE_STATES original_src_state = src_tex->m_current_state;

	m_backend->m_main_context->Reset();

	m_backend->m_main_context->AddResourceBarrier(src_tex, subresource_index, src_tex->m_current_state, D3D12_RESOURCE_STATE_COPY_SOURCE);

	CD3DX12_TEXTURE_COPY_LOCATION src_loc(src_tex->m_res, 0);
	CD3DX12_TEXTURE_COPY_LOCATION dest_loc(dst_res, psd);

	m_backend->m_main_context->m_command_list->CopyTextureRegion(&dest_loc, 0, 0, 0, &src_loc, nullptr);

	m_backend->m_main_context->AddResourceBarrier(src_tex, subresource_index, src_tex->m_current_state, original_src_state);

	uint64_t fence_value = m_backend->m_main_context->CloseExecute();
	m_backend->m_main_context->Discard(fence_value);
	m_backend->m_direct_queue->WaitForCompletion(fence_value);

	// Read back to data

	void *dst_res_ptr = nullptr;
	D3D12_RANGE read_range;
	read_range.Begin = 0;
	read_range.End = (SIZE_T)total_bytes;
	result = dst_res->Map(0, &read_range, &dst_res_ptr);
	if (result == S_OK)
	{
		memcpy(src_data.data(), dst_res_ptr, (size_t)total_bytes);
		dst_res->Unmap(0, 0);
	}
	else
	{
		assert(false);
		dst_res->Release();
		return false;
	}
	
	// Convert format

	const uint32_t dst_pixel_size = (format == NGL_R8_G8_B8_A8_UNORM) ? 4 : 3;
	const uint32_t src_pixel_size = 4;
	bool src_rgba = src_desc.Format == DXGI_FORMAT_R8G8B8A8_UNORM;

	width = (uint32_t)src_desc.Width;
	height = (uint32_t)src_desc.Height;
	data.resize(width * height * dst_pixel_size);

	uint8_t *src_ptr = (uint8_t*)src_data.data();

	for (uint32_t y = 0; y < height; y++)
	{
		for (uint32_t x = 0; x < width; x++)
		{
			uint32_t src_y = height - y - 1;

			uint32_t dst_index = (x + width * y) * dst_pixel_size;
			uint32_t src_index = x * src_pixel_size + (uint32_t)row_size_in_bytes * src_y;

			if (src_rgba)
			{
				data[dst_index + 0] = src_ptr[src_index + 0];
				data[dst_index + 1] = src_ptr[src_index + 1];
				data[dst_index + 2] = src_ptr[src_index + 2];
			}
			else
			{
				data[dst_index + 0] = src_ptr[src_index + 2];
				data[dst_index + 1] = src_ptr[src_index + 1];
				data[dst_index + 2] = src_ptr[src_index + 0];
			}

			if (dst_pixel_size == 4)
			{
				data[dst_index + 3] = 255;
			}
		}
	}

	dst_res->Release();
	return true;
}


#if 0
void D3D12_resource_manager::UpdateVertexBuffer(uint32_t buffer_id, uint32_t range_in_bytes[2], void *data)
{
	if (!data)
	{
		return;
	}

	D3D12_vertex_buffer &vb = *m_vertex_buffers[buffer_id];

	ResetCommandList();

	ID3D12Resource *bufVertsUploadHeap;

	ThrowIfFailed(mDevice->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(range_in_bytes[1] - range_in_bytes[0]),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&bufVertsUploadHeap)));
	vb.m_usage_state = D3D12_RESOURCE_STATE_GENERIC_READ;

	// copy data to the intermediate upload heap and then schedule a copy from the upload heap to the default texture
	D3D12_SUBRESOURCE_DATA bufResource = {};
	bufResource.pData = data;
	bufResource.RowPitch = vb.m_size;
	bufResource.SlicePitch = bufResource.RowPitch;

	SetResourceBarrier(mCommandList, &vb, D3D12_RESOURCE_STATE_COPY_DEST);
	UpdateSubresources<1>(mCommandList, vb.m_res, bufVertsUploadHeap, 0, 0, 1, &bufResource);

	SetResourceBarrier(mCommandList, &vb, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
	ExecuteCommandList(true);

	bufVertsUploadHeap->Release();
}


bool D3D12_resource_manager::GetResourceData(D3D12_resource *src_resource, std::vector<uint8_t> &data_buffer)
{
	data_buffer.clear();

	D3D12_RESOURCE_DESC src_desc;
	src_desc = src_resource->m_res->GetDesc();

	// Create a staging resource
	ID3D12Resource *dst_resource = nullptr;
	D3D12_RESOURCE_DESC dst_desc = src_desc;
	HRESULT result = m_backend->m_device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_READBACK),
		D3D12_HEAP_FLAG_NONE,
		&dst_desc,
		D3D12_RESOURCE_STATE_COPY_DEST,
		nullptr,
		IID_PPV_ARGS(&dst_resource));
	if (FAILED(result))
	{
		return false;
	}

	// Copy the resource to the staging resource
	m_backend->m_main_context->Reset();

	// Remember the original src state
	D3D12_RESOURCE_STATES original_src_state = src_resource->m_current_state;
	m_backend->m_main_context->AddResourceBarrier(src_resource, D3D12_RESOURCE_STATE_COPY_SOURCE);

	m_backend->m_main_context->m_command_list->CopyResource(dst_resource, src_resource->m_res);

	m_backend->m_main_context->AddResourceBarrier(src_resource, original_src_state);

	//ExecuteAndWait();
	uint64_t fence_value = m_backend->m_main_context->CloseExecute();
	m_backend->m_main_context->Discard(fence_value);
	m_backend->m_direct_queue->WaitForCompletion(fence_value);


	// Query the size of the copiable data
	D3D12_PLACED_SUBRESOURCE_FOOTPRINT psd = {};
	UINT num_rows = 0;
	UINT64 row_size_in_bytes = 0;
	UINT64 total_bytes = 0;
	m_backend->m_device->GetCopyableFootprints(&src_desc, 0, 1, 0, &psd, &num_rows, &row_size_in_bytes, &total_bytes);

	data_buffer.resize((size_t)total_bytes);

	// Read back to data
	if (src_desc.Dimension == D3D12_RESOURCE_DIMENSION_BUFFER)
	{
		void *src_data = nullptr;
		result = dst_resource->Map(0, 0, &src_data);
		if (result == S_OK)
		{
			memcpy(data_buffer.data(), src_data, (size_t)total_bytes);
			dst_resource->Unmap(0, 0);
		}
		else
		{
			assert(false);
			dst_resource->Release();
			return false;
		}
	}
	else
	{
		//CD3DX12_BOX src_box(0, 0, width, 0);
		dst_resource->ReadFromSubresource(data_buffer.data(), (UINT)row_size_in_bytes, 0, 0, nullptr);
	}

	dst_resource->Release();

	return true;
}
#endif


// D3D12_swap_chain ////////////////////////////////////////////////////////////////////////

D3D12_swap_chain::D3D12_swap_chain(D3D12_backend *backend, IDXGIFactory2 *dxgi_factory, D3D12_command_queue *command_queue)
{
	m_is_inited = false;
	m_is_windowed = false;
	m_enable_vsync = backend->m_enable_vsync;
	m_set_fullscreen_state = false;
	m_swap_chain_flags = 0;

	for (uint32_t i = 0; i < NGL_DX12_SWAP_CHAIN_BUFFER_COUNT; i++)
	{
		m_system_rtv_offsets[i] = 0;
	}
	m_system_dsv_offset = 0;
	m_current_swap_buffer = 0;
	m_swap_chain = nullptr;
	
	// Create the swap chain

	DWORD dwStyle = (DWORD)GetWindowLong(backend->m_hWnd, GWL_STYLE);
	m_is_windowed = (dwStyle & WS_POPUP) == 0;

	if (!m_is_windowed)
	{
		// Ensure that our window is borderless
		SetWindowLong(backend->m_hWnd, GWL_STYLE, dwStyle & ~(WS_CAPTION | WS_MAXIMIZEBOX | WS_MINIMIZEBOX | WS_SYSMENU | WS_THICKFRAME));

		// Get the settings of the primary display. We want the app to go into
		// fullscreen mode on the display that supports Independent Flip.
		DEVMODE devMode = {};
		devMode.dmSize = sizeof(DEVMODE);
		EnumDisplaySettings(nullptr, ENUM_CURRENT_SETTINGS, &devMode);

		SetWindowPos(
			backend->m_hWnd,
			HWND_TOPMOST,
			devMode.dmPosition.x,
			devMode.dmPosition.y,
			devMode.dmPosition.x + devMode.dmPelsWidth,
			devMode.dmPosition.y + devMode.dmPelsHeight,
			SWP_FRAMECHANGED | SWP_NOACTIVATE);

		ShowWindow(backend->m_hWnd, SW_MAXIMIZE);
	}

	// https://msdn.microsoft.com/en-us/library/windows/desktop/ee417025.aspx#full-screen_issues
	// By omitting DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH, we use the default DXGI behavior
	// that matches the swap chain to the desktop resolution on a fullscreen window
	m_swap_chain_flags |= m_is_windowed ? DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH : 0;

	DXGI_SWAP_CHAIN_DESC1 swap_chain_desc = { };
	memset(&swap_chain_desc, 0, sizeof(swap_chain_desc));
	swap_chain_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swap_chain_desc.Stereo = false;
	swap_chain_desc.SampleDesc.Count = 1;
	swap_chain_desc.SampleDesc.Quality = 0;
	swap_chain_desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swap_chain_desc.BufferCount = NGL_DX12_SWAP_CHAIN_BUFFER_COUNT;
	swap_chain_desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL; //DXGI_SWAP_EFFECT_DISCARD;
	swap_chain_desc.Flags = m_swap_chain_flags;
	swap_chain_desc.Width = 0; // use automatic sizing
	swap_chain_desc.Height = 0;
	swap_chain_desc.Scaling = DXGI_SCALING_STRETCH;

	DXGI_SWAP_CHAIN_FULLSCREEN_DESC fullscreen_desc = { };
	memset(&fullscreen_desc, 0, sizeof(fullscreen_desc));
	fullscreen_desc.RefreshRate.Numerator = 0; // use 60 for V-Sync
	fullscreen_desc.RefreshRate.Denominator = 1;
	fullscreen_desc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	fullscreen_desc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_PROGRESSIVE;
	fullscreen_desc.Windowed = m_is_windowed ? TRUE : FALSE;

	HRESULT hr = dxgi_factory->CreateSwapChainForHwnd(
		command_queue->m_command_queue,
		backend->m_hWnd,
		&swap_chain_desc,
		nullptr, //&fullscreen_desc,
		nullptr, // allow on all displays
		&m_swap_chain
	); 
	ThrowIfFailed(hr);

	// Disable switching to windowed / full screen with alt+enter
	dxgi_factory->MakeWindowAssociation(backend->m_hWnd, DXGI_MWA_NO_ALT_ENTER);

	if (!m_is_windowed && !m_enable_vsync)
	{
		m_set_fullscreen_state = true;
	}

	if (m_set_fullscreen_state)
	{
		m_swap_chain->SetFullscreenState(true, nullptr);
		m_swap_chain->ResizeBuffers(NGL_DX12_SWAP_CHAIN_BUFFER_COUNT, 0, 0, DXGI_FORMAT_UNKNOWN, m_swap_chain_flags);
	}

	for (uint32_t i = 0; i < NGL_DX12_SWAP_CHAIN_BUFFER_COUNT; i++)
	{
		backend->m_resource_mgr->m_color_attachment_heap.AddCPUHandle(m_system_rtv_offsets[i]);
	}
	backend->m_resource_mgr->m_depth_attachment_heap.AddCPUHandle(m_system_dsv_offset);

	CreateRTViews(backend);
	CreateDSTexture(backend);

	m_is_inited = true;
}


void D3D12_swap_chain::CreateRTViews(D3D12_backend *backend)
{
	m_current_swap_buffer = 0;
	for (uint32_t i = 0; i < NGL_DX12_SWAP_CHAIN_BUFFER_COUNT; i++)
	{
		m_system_color_buffer[i] = new D3D12_texture(0);
		m_swap_chain->GetBuffer(i, IID_PPV_ARGS(&m_system_color_buffer[i]->m_res));
		assert(m_system_color_buffer[i]->m_res != nullptr);
		m_system_color_buffer[i]->SetName("ngl_system_color_buffer");

		backend->m_device->CreateRenderTargetView(m_system_color_buffer[i]->m_res, nullptr, backend->m_resource_mgr->m_color_attachment_heap.GetCPUHandle(m_system_rtv_offsets[i]));
		m_system_color_buffer[i]->m_current_state = D3D12_RESOURCE_STATE_COMMON;
		//backend->m_main_context->TrackTextureState(*m_system_color_buffer[i], m_system_color_buffer[i]->m_current_state);
	}
}


void D3D12_swap_chain::CreateDSTexture(D3D12_backend *backend)
{
	// Create the depth stencil for the scene
	{
		m_system_depth_buffer = new D3D12_texture(0);

		CD3DX12_RESOURCE_DESC desc(
			D3D12_RESOURCE_DIMENSION_TEXTURE2D,
			0,		// alignment
			backend->m_width, backend->m_height, 1,
			1,		// mip levels
			DXGI_FORMAT_D32_FLOAT,
			1, 0,	// sample count/quality
			D3D12_TEXTURE_LAYOUT_UNKNOWN,
			D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL |
			D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE);

		// Performance tip: Tell the runtime at resource creation the desired clear value.
		D3D12_CLEAR_VALUE clearValue;
		clearValue.Format = DXGI_FORMAT_D32_FLOAT;
		clearValue.DepthStencil.Depth = 1.0f;
		clearValue.DepthStencil.Stencil = 0;

		ThrowIfFailed(backend->m_device->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
			D3D12_HEAP_FLAG_NONE,
			&desc,
			D3D12_RESOURCE_STATE_COMMON,
			&clearValue,
			IID_PPV_ARGS(&m_system_depth_buffer->m_res)));

		m_system_depth_buffer->SetName("ngl_system_depth_buffer");

		// Create the depth stencil view
		D3D12_DEPTH_STENCIL_VIEW_DESC dsv_desc = {};
		dsv_desc.Format = DXGI_FORMAT_D32_FLOAT;
		dsv_desc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
		dsv_desc.Flags = D3D12_DSV_FLAG_NONE;

		backend->m_device->CreateDepthStencilView(m_system_depth_buffer->m_res, &dsv_desc, backend->m_resource_mgr->m_depth_attachment_heap.GetCPUHandle(m_system_dsv_offset));
		m_system_depth_buffer->m_current_state = D3D12_RESOURCE_STATE_COMMON;
		backend->m_main_context->TrackTextureState(*m_system_depth_buffer, m_system_depth_buffer->m_current_state);
	}
}


void D3D12_swap_chain::ReleaseRTViews()
{
	for (uint32_t i = 0; i < NGL_DX12_SWAP_CHAIN_BUFFER_COUNT; i++)
	{
		delete m_system_color_buffer[i];
	}
}


D3D12_swap_chain::~D3D12_swap_chain()
{
	if (m_set_fullscreen_state)
	{
		m_swap_chain->SetFullscreenState(false, nullptr);
	}

	delete m_system_depth_buffer;
	ReleaseRTViews();
	SAFE_RELEASE(m_swap_chain);
}


HRESULT D3D12_swap_chain::Present()
{
	UINT sync_interval = m_enable_vsync ? NGL_DX12_SYNC_INTERVAL_VSYNC : NGL_DX12_SYNC_INTERVAL;
	UINT flags = 0;

	HRESULT result = m_swap_chain->Present(sync_interval, flags);
	ThrowIfFailed(result);
	m_current_swap_buffer = (m_current_swap_buffer + 1) % NGL_DX12_SWAP_CHAIN_BUFFER_COUNT;
	return result;
}


void D3D12_swap_chain::Resize(D3D12_backend *backend)
{
	backend->m_direct_queue->Finish();

	ReleaseRTViews();

	if (m_set_fullscreen_state)
	{
		m_swap_chain->SetFullscreenState(true, nullptr);
	}
	m_swap_chain->ResizeBuffers(NGL_DX12_SWAP_CHAIN_BUFFER_COUNT, 0, 0, DXGI_FORMAT_UNKNOWN, m_swap_chain_flags);

	CreateRTViews(backend);
}
