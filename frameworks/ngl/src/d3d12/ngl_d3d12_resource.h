/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef NGL_D3D12_RESOURCE_H
#define NGL_D3D12_RESOURCE_H

#include "ngl_d3d12_define.h"
#include "ngl_d3d12_bind.h"
#include <map>
#include <list>
#include <vector>
#include <queue>

class D3D12_backend;
struct D3D12_command_queue;
struct D3D12_texture;
struct D3D12_renderer;
struct D3D12_bind_heap;
struct D3D12_root_signature;
struct D3D12_root_signature_shader_stage;
struct D3D12_resource_manager;


struct D3D12_sampler_comparator
{
	bool operator()(const D3D12_SAMPLER_DESC& a, const D3D12_SAMPLER_DESC& b) const
	{
		return memcmp(&a, &b, sizeof(D3D12_SAMPLER_DESC)) > 0;
	}
};


struct D3D12_color_attachment
{
	D3D12_color_attachment()
	{
		m_texture = nullptr;
		m_texture_id = 0;
		m_attachment_id = 0;
		m_isPSOBound = true;
	}

	D3D12_color_attachment(D3D12_texture *texture, uint32_t texture_id, uint32_t attachment_id)
	{
		m_texture 		= texture;
		m_texture_id 	= texture_id;
		m_attachment_id = attachment_id;
		m_isPSOBound 	= true;
	}

	D3D12_color_attachment(D3D12_texture* texture, uint32_t texture_id, uint32_t attachment_id, bool isPSOBound)
	{
		m_texture 		= texture;
		m_texture_id	= texture_id;
		m_attachment_id = attachment_id;
		m_isPSOBound 	= isPSOBound;
	}

	D3D12_texture *m_texture;
	uint32_t m_texture_id;
	uint32_t m_attachment_id;
	bool m_isPSOBound;
};


struct D3D12_depth_attachment
{
	D3D12_depth_attachment()
	{
		m_texture = nullptr;
		m_texture_id = 0;
		m_attachment_id = 0;
		m_is_read_only = false;
	}

	D3D12_depth_attachment(D3D12_texture *texture, uint32_t texture_id, uint32_t attachment_id, bool is_read_only)
	{
		m_texture = texture;
		m_texture_id = texture_id;
		m_attachment_id = attachment_id;
		m_is_read_only = is_read_only;
	}

	D3D12_texture *m_texture;
	uint32_t m_texture_id;
	uint32_t m_attachment_id;
	bool m_is_read_only;
};


enum D3D12_resource_type
{
	NGL_D3D12_RESOURCE_TYPE_UNSPECIFIED,
	NGL_D3D12_RESOURCE_TYPE_TEXTURE,
	NGL_D3D12_RESOURCE_TYPE_VERTEX_BUFFER,
	NGL_D3D12_RESOURCE_TYPE_INDEX_BUFFER,
	NGL_D3D12_RESOURCE_TYPE_MEMORY_PAGE
};


struct D3D12_resource
{
	D3D12_resource(uint32_t ngl_id)
	{
		m_ngl_id = ngl_id;

		m_res = nullptr;
		m_format = DXGI_FORMAT_UNKNOWN;
		m_resource_type = NGL_D3D12_RESOURCE_TYPE_UNSPECIFIED;
		m_resource_id = (uint32_t)-1;
		m_current_state = D3D12_RESOURCE_STATE_COMMON;
	}

	~D3D12_resource()
	{
		SAFE_RELEASE(m_res);
	}

	inline void SetName(const std::string &name)
	{
#if NGL_D3D12_ENABLE_DEBUG_LAYER
		assert(m_res != nullptr);

		m_name = name;
		m_wname = std::wstring(name.begin(), name.end());
		m_res->SetName(m_wname.c_str());
#endif
	}

	inline D3D12_GPU_VIRTUAL_ADDRESS GetGPUAddress()
	{
		return m_res->GetGPUVirtualAddress();
	}

	uint32_t m_ngl_id;

	ID3D12Resource *m_res;
	DXGI_FORMAT m_format;
	D3D12_resource_type m_resource_type;
	uint32_t m_resource_id;
	D3D12_RESOURCE_STATES m_current_state;
	std::string m_name;
	std::wstring m_wname;
};


struct D3D12_resource_state_transition
{
	D3D12_resource_state_transition()
	{
		m_before = NGL_DX12_RESOURCE_STATE_UNKNOWN;
		m_after = NGL_DX12_RESOURCE_STATE_UNKNOWN;
	}

	D3D12_RESOURCE_STATES m_before;
	D3D12_RESOURCE_STATES m_after;
};


struct D3D12_vertex_buffer : public D3D12_resource
{
	D3D12_vertex_buffer(uint32_t ngl_id);
	bool Init(NGL_vertex_descriptor &vertex_layout, uint32_t num_vertices);
	uint64_t GetSubresourceKey(const NGL_buffer_subresource &buffer_subresource);
	uint32_t GetSubresourceUAV(const NGL_buffer_subresource &buffer_subresource);
	uint32_t GetSubresourceSRV(const NGL_buffer_subresource &buffer_subresource);

	NGL_vertex_buffer m_ngl_vertex_buffer;

	// vertex buffer members
	uint32_t m_num_vertices;
	D3D12_VERTEX_BUFFER_VIEW m_vertex_buffer_view_desc;

	// storage buffer members (unordered access)
	uint32_t m_uav_index;
	uint32_t m_srv_index;
	D3D12_resource_manager *m_resource_manager; // weak reference
	D3D12_UNORDERED_ACCESS_VIEW_DESC m_base_uav_desc;
	D3D12_SHADER_RESOURCE_VIEW_DESC m_base_srv_desc;
	std::map<uint64_t, uint32_t> m_subresource_uav_indices;
	std::map<uint64_t, uint32_t> m_subresource_srv_indices;
};


struct D3D12_index_buffer : public D3D12_resource
{
	D3D12_index_buffer(uint32_t ngl_id);
	bool Init(NGL_format index_format, uint32_t num_indices);

	NGL_index_buffer m_ngl_index_buffer;

	uint32_t m_num_indices;
	uint32_t m_stride; // in bytes
	uint32_t m_size; // in bytes
	D3D12_INDEX_BUFFER_VIEW m_index_buffer_view_desc;
};


struct D3D12_texture : public D3D12_resource
{
	D3D12_texture(uint32_t ngl_id);
	~D3D12_texture();
	void InitNull(uint32_t width, uint32_t height);
	void Init(const NGL_texture_descriptor &texture_layout, uint32_t num_data_layouts);
	void SetDataFormat(NGL_format ngl_format);
	void SetDimensions(uint32_t num_data_subresources);

	inline uint32_t GetSubresourceIndex(uint32_t mip_level, uint32_t surface_index, uint32_t face_index, uint32_t depth_slice_index) const
	{
		// PVR order: mip_level, surface, face, depth_slice
		// Subresource order: surface, face, depth_slice, mip_level
		// (DX doesn't differentiate surface, face and depth_slice, so linearization is arbitrary)

		return surface_index * m_num_faces * m_num_depth_slices * m_num_mip_levels 
			+ face_index * m_num_depth_slices * m_num_mip_levels 
			+ depth_slice_index * m_num_mip_levels
			+ mip_level;
	}

	inline uint32_t GetSubtextureIndex(uint32_t surface_index, uint32_t face_index, uint32_t depth_slice_index) const
	{
		return surface_index * m_num_faces * m_num_depth_slices * m_num_mip_levels
			+ face_index * m_num_depth_slices * m_num_mip_levels
			+ depth_slice_index;
	}
	
	inline void DescribeSubresource(uint32_t subresource_index, NGL_texture_subresource &subresource) const
	{
		uint32_t remainder = subresource_index;
		uint32_t denominator = m_num_faces * m_num_depth_slices * m_num_mip_levels;
		uint32_t surface_index = remainder / denominator;

		remainder = remainder % denominator;
		denominator = m_num_depth_slices * m_num_mip_levels;
		uint32_t face_index = remainder / denominator;

		remainder = remainder % denominator;
		denominator = m_num_mip_levels;
		uint32_t depth_slice_index = remainder / denominator;

		remainder = remainder % denominator;
		uint32_t mip_level = remainder;

		subresource.m_idx = m_ngl_id;
		subresource.m_level = mip_level;
		subresource.m_layer = surface_index;
		subresource.m_face = face_index;
	}

	NGL_texture m_ngl_texture;

	uint32_t m_width;
	uint32_t m_height;

	uint32_t m_num_surfaces; // texture array item count
	uint32_t m_num_faces; // cubemap face count
	uint32_t m_num_depth_slices;
	uint32_t m_num_mip_levels;

	uint32_t m_num_subresources;
	uint32_t m_num_subtextures; // number of subresources, not counting mip levels

	uint32_t m_block_size; // in bits
	uint32_t m_block_dim_x;
	uint32_t m_block_dim_y;

	D3D12_CLEAR_VALUE *m_default_clear_value;

	std::vector<UINT> m_sampler_offsets; // 0: regular sampler, 1: shadow sampler
	UINT m_srv_offset;
	std::vector<UINT> m_subresource_srv_offsets;
	UINT m_uav_offset;
	std::vector<UINT> m_rtv_offsets; // one for each subresource (mipmap levels * faces)
	std::vector<UINT> m_dsv_offsets; // two for each subresource (read and read/write variant per subresource)
};


struct D3D12_resource_manager
{
	typedef std::map<D3D12_SAMPLER_DESC, uint32_t, D3D12_sampler_comparator> SamplerMap;

	D3D12_resource_manager(D3D12_backend *backend);
	~D3D12_resource_manager();
	void InitTextures();
	//void TrackResource(D3D12_resource &resource);
	bool CreateVertexBuffer(uint32_t &buffer_id, NGL_vertex_descriptor &vertex_layout, uint32_t num_vertices, void *data);
	bool CreateIndexBuffer(uint32_t &buffer_id, NGL_format index_format, uint32_t num_indices, void *data);
	bool CreateTexture(uint32_t &texture_id, NGL_texture_descriptor &texture_layout, std::vector< std::vector<uint8_t> > *data);
	void InitTextureData(const D3D12_texture &texture, std::vector<std::vector<uint8_t> > *data, std::vector<D3D12_SUBRESOURCE_DATA> &subresource_data, std::vector<uint8_t*> &temp_data);
	bool GetTextureContent(uint32_t texture_id, uint32_t mip_level, uint32_t surface_index, uint32_t face_index, NGL_format format, NGL_resource_state state, uint32_t &width, uint32_t &height, std::vector<uint8_t> &data);
	//void UpdateVertexBuffer(uint32_t buffer_id, uint32_t range_in_bytes[2], void *data); // TODO (see cpp file)
	//bool GetResourceData(D3D12_resource *src_resource, std::vector<uint8_t> &data_buffer); // TODO (see cpp file)
	
	D3D12_backend *m_backend;

	D3D12_descriptor_heap m_view_heap;
	D3D12_descriptor_heap m_sampler_heap;
	D3D12_descriptor_heap m_color_attachment_heap;
	D3D12_descriptor_heap m_depth_attachment_heap;
	SamplerMap m_sampler_cache;

	D3D12_view_bind_heap m_view_bind_heap;
	D3D12_sampler_bind_heap m_sampler_bind_heap;

	std::vector<D3D12_vertex_buffer*> m_vertex_buffers;
	std::vector<D3D12_index_buffer*> m_index_buffers;
	std::vector<D3D12_texture*> m_textures;
	std::vector<D3D12_resource*> m_tracked_resources;
};


struct D3D12_swap_chain
{
	D3D12_swap_chain(D3D12_backend *backend, IDXGIFactory2 *dxgi_factory, D3D12_command_queue *command_queue);
	~D3D12_swap_chain();
	HRESULT Present();
	void CreateRTViews(D3D12_backend *backend);
	void CreateDSTexture(D3D12_backend *backend);
	void ReleaseRTViews();
	inline uint32_t GetCurrentSwapBufferIndex() { return m_current_swap_buffer; }
	inline D3D12_texture *GetCurrentSwapBuffer() { return m_system_color_buffer[m_current_swap_buffer]; }
	void Resize(D3D12_backend *backend);

	bool m_is_inited;
	bool m_is_windowed;
	bool m_enable_vsync;
	bool m_set_fullscreen_state;
	UINT m_swap_chain_flags;

	IDXGISwapChain1 *m_swap_chain;

	D3D12_texture *m_system_color_buffer[NGL_DX12_SWAP_CHAIN_BUFFER_COUNT];
	D3D12_texture *m_system_depth_buffer;
	uint32_t m_system_rtv_offsets[NGL_DX12_SWAP_CHAIN_BUFFER_COUNT];
	uint32_t m_system_dsv_offset;
	uint32_t m_current_swap_buffer;
};


#endif
