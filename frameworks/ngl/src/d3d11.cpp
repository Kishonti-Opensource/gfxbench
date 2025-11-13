/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "ngl.h"
#include "ngl_internal.h"
#include "shader_reflection.h"
#include <d3d11_1.h>
#include <d3dcompiler.h>
#include <assert.h>
#include <map>
#include <set>
#include <sstream>
#include <exception>
#include <algorithm>

static const bool LOG_WARNINGS = true;

//http://www.rastertek.com/dx11tut05.html
//https://anteru.net/2011/12/27/1830/
//http://www.sunandblackcat.com/tipFullView.php?l=eng&topicid=26


inline void DX_THROW_IF_FAILED(HRESULT expression)
{
	if (FAILED(expression))
	{
		throw std::exception();
	}
}


template <class T> void SafeRelease(T* &resource)
{
	if (resource != nullptr)
	{
		resource->Release();
		resource = nullptr;
	}
}


namespace D3D11
{
#include "d3d11.h"


D3D11_texture::~D3D11_texture()
{
	Release();
}


void D3D11_texture::Release()
{
	SafeRelease(m_default_srv);
	SafeRelease(m_tex);
	SafeRelease(m_uav);

	for (size_t i = 0; i < m_sr_views.size(); i++)
	{
		SafeRelease(m_sr_views[i]);
	}
	for (size_t i = 0; i < m_rt_views.size(); i++)
	{
		SafeRelease(m_rt_views[i]);
	}
	for (size_t i = 0; i < m_ds_views.size(); i++)
	{
		SafeRelease(m_ds_views[i]);
	}
	for (size_t i = 0; i < m_readonly_ds_views.size(); i++)
	{
		SafeRelease(m_readonly_ds_views[i]);
	}

	m_sr_views.clear();
	m_rt_views.clear();
	m_ds_views.clear();
	m_readonly_ds_views.clear();

	m_tex = nullptr;
	m_sampler = nullptr; // Should not released, handled by D3D11_Instance
	m_default_srv = nullptr;
	m_uav = nullptr;
	m_subresource_count = 0;

	memset(&m_descriptor, 0, sizeof(D3D11_TEXTURE2D_DESC));
}


ID3D11ShaderResourceView *D3D11_texture::GetSRV(const NGL_texture_subresource &subresource)
{
	assert(m_texture_descriptor.m_is_renderable);

	if (m_subresource_count < 2)
	{
		return m_default_srv;
	}
	else
	{
		return m_sr_views[GetSubresourceIndex(subresource)];
	}
}


ID3D11RenderTargetView *D3D11_texture::GetRTV(const NGL_texture_subresource &subresource)
{
	return m_rt_views[GetSubresourceIndex(subresource)];
}


ID3D11DepthStencilView *D3D11_texture::GetDSV(const NGL_texture_subresource &subresource)
{
	return m_ds_views[GetSubresourceIndex(subresource)];
}


ID3D11DepthStencilView *D3D11_texture::GetReadOnlyDSV(const NGL_texture_subresource &subresource)
{
	return m_readonly_ds_views[GetSubresourceIndex(subresource)];
}


uint32_t D3D11_texture::GetSubresourceIndex(const NGL_texture_subresource &subresource) const
{
	uint32_t array_slice = 0;

	switch (m_texture_descriptor.m_type)
	{
	case NGL_TEXTURE_2D:
		array_slice = 0;
		break;
	case NGL_TEXTURE_2D_ARRAY:
		array_slice = subresource.m_layer;
		break;
	case NGL_TEXTURE_CUBE:
		array_slice = subresource.m_face;
		break;
	default:
		assert(0);
	}

	return D3D11CalcSubresource(subresource.m_level, array_slice, m_texture_descriptor.m_num_levels);
}


D3D11_vertex_buffer::~D3D11_vertex_buffer()
{
	Release();
}


uint64_t D3D11_vertex_buffer::GetKey(const NGL_buffer_subresource &subresource)
{
	uint64_t key = subresource.m_offset;
	key <<= 32;
	key += subresource.m_size;
	return key;
}


ID3D11ShaderResourceView *D3D11_vertex_buffer::GetSRV(const NGL_buffer_subresource &subresource)
{
	if (subresource.m_offset == 0 && subresource.m_size >= m_datasize)
	{
		return m_srv;
	}

	uint64_t key = GetKey(subresource);
	auto i = m_subresource_srvs.find(key);
	if (i != m_subresource_srvs.end())
	{
		return i->second;
	}
	else
	{
		ID3D11ShaderResourceView *srv = nullptr;
		uint32_t size = subresource.m_size == (uint32_t)~0 ? m_datasize - subresource.m_offset : subresource.m_size;
		D3D11_SHADER_RESOURCE_VIEW_DESC srv_desc = m_base_srv_desc;
		srv_desc.Buffer.FirstElement = subresource.m_offset / m_vertex_descriptor.m_stride;
		srv_desc.Buffer.NumElements = size / m_vertex_descriptor.m_stride;
		DX_THROW_IF_FAILED(D3D11_instance::This->m_d3dDevice->CreateShaderResourceView(m_structured_buffer, &srv_desc, &srv));
		SetDebugObjectName(srv, "vertex_buffer_subresource_srv");

		m_subresource_srvs[key] = srv;
		return srv;
	}
}


ID3D11UnorderedAccessView *D3D11_vertex_buffer::GetUAV(const NGL_buffer_subresource &subresource)
{
	if (subresource.m_offset == 0 && subresource.m_size >= m_datasize)
	{
		return m_uav;
	}

	uint64_t key = GetKey(subresource);
	auto i = m_subresource_uavs.find(key);
	if (i != m_subresource_uavs.end())
	{
		return i->second;
	}
	else
	{
		ID3D11UnorderedAccessView *uav = nullptr;
		uint32_t size = subresource.m_size == (uint32_t)~0 ? m_datasize - subresource.m_offset : subresource.m_size;
		D3D11_UNORDERED_ACCESS_VIEW_DESC uav_desc = m_base_uav_desc;
		uav_desc.Buffer.FirstElement = subresource.m_offset / m_vertex_descriptor.m_stride;
		uav_desc.Buffer.NumElements = size / m_vertex_descriptor.m_stride;
		DX_THROW_IF_FAILED(D3D11_instance::This->m_d3dDevice->CreateUnorderedAccessView(m_structured_buffer, &uav_desc, &uav));
		SetDebugObjectName(uav, "vertex_buffer_subresource_uav");

		m_subresource_uavs[key] = uav;
		return uav;
	}
}


void D3D11_vertex_buffer::Release()
{
	SafeRelease(pVBuffer);
	SafeRelease(m_structured_buffer);
	SafeRelease(m_srv);
	SafeRelease(m_uav);
}


D3D11_index_buffer::~D3D11_index_buffer()
{
	Release();
}


void D3D11_index_buffer::Release()
{
	SafeRelease(pIBuffer);
}


void D3D11_ubo::Allocate(uint32_t memory_size)
{
	SafeRelease(m_buffer);

	if (memory_size == 0)
	{
		_logf("DX11 - D3D11_ubo::Allocate(): Warning UBO size is zero!");
		return;
	}

	D3D11_BUFFER_DESC descriptor;

	ZeroMemory(&descriptor, sizeof(D3D11_BUFFER_DESC));

	descriptor.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	descriptor.ByteWidth = memory_size;
	descriptor.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	descriptor.Usage = D3D11_USAGE_DYNAMIC;
	descriptor.StructureByteStride = 0;
	descriptor.MiscFlags = 0;

	DX_THROW_IF_FAILED(D3D11_instance::This->m_d3dDevice->CreateBuffer(&descriptor, NULL, &m_buffer));
	SetDebugObjectName(m_buffer, "ubo");
}


D3D11_ubo::~D3D11_ubo()
{
	SafeRelease(m_buffer);
}


D3D11_renderer::D3D11_renderer()
{
	pVS = nullptr;
	pPS = nullptr;
	pCS = nullptr;
	pLayout = nullptr;
}


D3D11_renderer::~D3D11_renderer()
{
	SafeRelease(pVS);
	SafeRelease(pPS);
	SafeRelease(pCS);
	SafeRelease(pLayout);
}


void D3D11_subpass::BindRenderTargets()
{
	ID3D11RenderTargetView *const*rtvs = nullptr;
	uint32_t rtv_count = (uint32_t)m_color_attachment_views.size();
	if (rtv_count)
	{
		rtvs = m_color_attachment_views.data();
	}

	D3D11_instance::This->m_d3dContext->OMSetRenderTargets(rtv_count, rtvs, m_depth_stencil_view);
}


void D3D11_subpass::ExecuteLoadOperations()
{
	// Load clear color
	for (size_t i = 0; i < m_color_clears.size(); i++)
	{
		const _color_clear_data &clear_data = m_color_clears[i];
		D3D11_instance::This->m_d3dContext->ClearRenderTargetView(clear_data.m_rtv, clear_data.m_value);
	}

	// Load clear depth
	for (size_t i = 0; i < m_depth_clear.size(); i++)
	{
		const _depth_clear_data &clear_data = m_depth_clear[i];
		D3D11_instance::This->m_d3dContext->ClearDepthStencilView(clear_data.m_dsv, D3D11_CLEAR_DEPTH, clear_data.m_value, 0);
	}

	// Load discard
	for (size_t i = 0; i < m_load_discards.size(); i++)
	{
		D3D11_instance::This->m_d3dContext->DiscardView(m_load_discards[i]);
	}
}


void D3D11_subpass::ExecuteStoreOperations()
{
	// Store discard
	for (size_t i = 0; i < m_store_discards.size(); i++)
	{
		D3D11_instance::This->m_d3dContext->DiscardView(m_store_discards[i]);
	}
}


void D3D11_job::CreateSubpasses()
{
	m_subpasses.clear();

	ValidateSubpassses(m_descriptor);
	m_subpasses.resize(m_descriptor.m_subpasses.size());

	std::vector<bool> is_cleared(m_descriptor.m_attachments.size(), false);
	std::vector<bool> is_discarded(m_descriptor.m_attachments.size(), false);

	for (size_t sp_idx = 0; sp_idx < m_subpasses.size(); sp_idx++)
	{
		NGL_subpass &src_sp = m_descriptor.m_subpasses[sp_idx];
		D3D11_subpass &dst_sp = m_subpasses[sp_idx];
		bool is_last_pass = true;

		if (sp_idx < m_subpasses.size() - 1)
		{
			is_last_pass = false;
		}

		dst_sp.m_name = src_sp.m_name;

		for (size_t reference = 0; reference < src_sp.m_usages.size(); reference++)
		{
			const NGL_resource_state &f = src_sp.m_usages[reference];
			const NGL_attachment_descriptor &atd = m_descriptor.m_attachments[reference];

			D3D11_texture *texture = D3D11_instance::This->m_textures[atd.m_attachment.m_idx];

			if (f == NGL_COLOR_ATTACHMENT)
			{
				ID3D11RenderTargetView *rtv = texture->GetRTV(atd.m_attachment);

				if (atd.m_attachment_load_op == NGL_LOAD_OP_CLEAR)
				{
					if (!is_cleared[reference])
					{
						D3D11_subpass::_color_clear_data clear_data;
						clear_data.m_rtv = rtv;
						clear_data.m_value[0] = texture->m_texture_descriptor.m_clear_value[0];
						clear_data.m_value[1] = texture->m_texture_descriptor.m_clear_value[1];
						clear_data.m_value[2] = texture->m_texture_descriptor.m_clear_value[2];
						clear_data.m_value[3] = texture->m_texture_descriptor.m_clear_value[3];

						dst_sp.m_color_clears.push_back(clear_data);
						is_cleared[reference] = true;
					}
				}
				else if (atd.m_attachment_load_op == NGL_LOAD_OP_DONT_CARE)
				{
					if (!is_discarded[reference])
					{
						dst_sp.m_load_discards.push_back(rtv);
						is_discarded[reference] = true;
					}
				}

				if (atd.m_attachment_store_op == NGL_STORE_OP_DONT_CARE)
				{
					if (is_last_pass)
					{
						dst_sp.m_store_discards.push_back(rtv);
					}
				}

				dst_sp.m_color_attachments.push_back(atd);
				dst_sp.m_color_attachment_views.push_back(rtv);
			}
			else if (f == NGL_DEPTH_ATTACHMENT || f == NGL_READ_ONLY_DEPTH_ATTACHMENT || f == NGL_READ_ONLY_DEPTH_ATTACHMENT_AND_SHADER_RESOURCE)
			{
				ID3D11DepthStencilView *dsv = f == NGL_DEPTH_ATTACHMENT ? texture->GetDSV(atd.m_attachment) : texture->GetReadOnlyDSV(atd.m_attachment);

				if (atd.m_attachment_load_op == NGL_LOAD_OP_CLEAR)
				{
					if (!is_cleared[reference])
					{
						D3D11_subpass::_depth_clear_data clear_data;
						clear_data.m_dsv = texture->GetDSV(atd.m_attachment);
						clear_data.m_value = texture->m_texture_descriptor.m_clear_value[0];

						dst_sp.m_depth_clear.push_back(clear_data);
						is_cleared[reference] = true;
					}
				}
				else if (atd.m_attachment_load_op == NGL_LOAD_OP_DONT_CARE)
				{
					if (!is_discarded[reference])
					{
						dst_sp.m_load_discards.push_back(dsv);
						is_discarded[reference] = true;
					}
				}
				if (atd.m_attachment_store_op == NGL_STORE_OP_DONT_CARE)
				{
					if (is_last_pass)
					{
						dst_sp.m_store_discards.push_back(dsv);
					}
				}

				dst_sp.m_depth_attachment = atd;
				dst_sp.m_depth_stencil_view = dsv;
			}
		}
	}
}


NGL_renderer* D3D11_job::CreateRenderer(NGL_state &sh, uint32_t num_vbos, uint32_t *vbos)
{
	NGL_shader_source_descriptor ssd[7];
	std::vector<NGL_shader_uniform> application_uniforms;

	m_descriptor.m_load_shader_callback(m_descriptor, m_current_subpass, sh.m_shader.m_shader_code, ssd, application_uniforms);

	D3D11_renderer *renderer = new D3D11_renderer;

	ID3DBlob *blobs[NGL_NUM_SHADER_TYPES];

	memset(blobs, 0, sizeof(blobs));

	bool compile_ok = true;
	for (uint32_t shader_type = NGL_VERTEX_SHADER; shader_type < NGL_NUM_SHADER_TYPES; shader_type++)
	{
		ID3DBlob *blob = blobs[shader_type];

		if (ssd[shader_type].m_source_data.length())
		{
			ID3DBlob *blob = D3D11_renderer::CompileShader(ssd[shader_type].m_source_data, ssd[shader_type].m_entry_point, ssd[shader_type].m_version, ssd[shader_type].m_info_string);
			if (blob)
			{
				switch (shader_type)
				{
				case NGL_VERTEX_SHADER:
				{
					DX_THROW_IF_FAILED(D3D11_instance::This->m_d3dDevice->CreateVertexShader(blob->GetBufferPointer(), blob->GetBufferSize(), NULL, &renderer->pVS));
					SetDebugObjectName(renderer->pVS, "vs_" + ssd[shader_type].m_info_string + '_' + m_descriptor.m_subpasses[m_current_subpass].m_name);
					break;
				}
				case NGL_FRAGMENT_SHADER:
				{
					DX_THROW_IF_FAILED(D3D11_instance::This->m_d3dDevice->CreatePixelShader(blob->GetBufferPointer(), blob->GetBufferSize(), NULL, &renderer->pPS));
					SetDebugObjectName(renderer->pPS, "ps_" + ssd[shader_type].m_info_string + '_' + m_descriptor.m_subpasses[m_current_subpass].m_name);
					break;
				}
				case NGL_COMPUTE_SHADER:
				{
					DX_THROW_IF_FAILED(D3D11_instance::This->m_d3dDevice->CreateComputeShader(blob->GetBufferPointer(), blob->GetBufferSize(), NULL, &renderer->pCS));
					SetDebugObjectName(renderer->pCS, "cs_" + ssd[shader_type].m_info_string + '_' + m_descriptor.m_subpasses[m_current_subpass].m_name);
					break;
				}
				}

				blobs[shader_type] = blob;
			}

			compile_ok &= (blob != NULL);
		}
	}

	if (!compile_ok)
	{
		delete renderer;
		return NULL;
	}

	_shader_reflection r;
	{
		HRESULT enumInputLayout(_shader_reflection &r, ID3D11Device * d3dDevice, ID3DBlob * VSBlob);
		HRESULT enumConstantBuffers(_shader_reflection &r, ID3D11Device * d3dDevice, ID3DBlob * blob, NGL_shader_type type);

		if (!m_descriptor.m_is_compute)
		{
			enumInputLayout(r, D3D11_instance::This->m_d3dDevice, blobs[NGL_VERTEX_SHADER]);
			enumConstantBuffers(r, D3D11_instance::This->m_d3dDevice, blobs[NGL_VERTEX_SHADER], NGL_VERTEX_SHADER);
			enumConstantBuffers(r, D3D11_instance::This->m_d3dDevice, blobs[NGL_FRAGMENT_SHADER], NGL_FRAGMENT_SHADER);
		}
		else
		{
			enumConstantBuffers(r, D3D11_instance::This->m_d3dDevice, blobs[NGL_COMPUTE_SHADER], NGL_COMPUTE_SHADER);
		}
	}

	renderer->m_my_state = sh;
	renderer->LinkShader();
	if (!m_descriptor.m_is_compute)
	{
		std::vector<D3D11_INPUT_ELEMENT_DESC> input_elemets;

		bool shader_matches_with_mesh = renderer->GetActiveAttribs2(r, num_vbos, vbos, input_elemets);

		if (!shader_matches_with_mesh)
		{
			_logf("Error: shader-mesh mismatch in job (%s)\n", m_descriptor.m_subpasses[m_current_subpass].m_name.c_str());
		}

		renderer->pLayout = 0;
		if (input_elemets.size())
		{
			DX_THROW_IF_FAILED(D3D11_instance::This->m_d3dDevice->CreateInputLayout(input_elemets.data(), (UINT)input_elemets.size(), blobs[NGL_VERTEX_SHADER]->GetBufferPointer(), blobs[NGL_VERTEX_SHADER]->GetBufferSize(), &renderer->pLayout));
			SetDebugObjectName(renderer->pLayout, "input_layout");
		}
	}

	renderer->GetActiveResources2(r, application_uniforms);

	if (!renderer->m_used_uniforms[0].size() && !renderer->m_used_uniforms[1].size() && !renderer->m_used_uniforms[1].size() && !renderer->m_used_uniforms[2].size())
	{
		_logf("Warning: no uniforms in renderer of job (%s)\n", m_descriptor.m_subpasses[m_current_subpass].m_name.c_str());
	}

	m_renderers.push_back(renderer);

	return renderer;
}


void D3D11_job::UnbindResources(const D3D11_renderer *renderer)
{
	D3D11_instance *dx = D3D11_instance::This;

	size_t **zeros = dx->m_null_resources.data();
	ID3D11DeviceContext1 *context = dx->m_d3dContext;

	if (m_descriptor.m_is_compute)
	{
		if (renderer->m_max_CB_bindpoint[NGL_COMPUTE_SHADER] > -1)
		{
			context->CSSetConstantBuffers(0, renderer->m_max_CB_bindpoint[NGL_COMPUTE_SHADER] + 1, (ID3D11Buffer**)zeros);
		}
		if (renderer->m_max_UAV_bindpoint[NGL_COMPUTE_SHADER] > -1)
		{
			context->CSSetUnorderedAccessViews(0, renderer->m_max_UAV_bindpoint[NGL_COMPUTE_SHADER] + 1, (ID3D11UnorderedAccessView**)zeros, (UINT*)&zeros);
		}
		if (renderer->m_max_SR_bindpoint[NGL_COMPUTE_SHADER] > -1)
		{
			context->CSSetShaderResources(0, renderer->m_max_SR_bindpoint[NGL_COMPUTE_SHADER] + 1, (ID3D11ShaderResourceView**)zeros);
			context->CSSetSamplers(0, renderer->m_max_SR_bindpoint[NGL_COMPUTE_SHADER] + 1, (ID3D11SamplerState**)zeros);
		}
	}
	else
	{
		if (renderer->m_max_CB_bindpoint[NGL_VERTEX_SHADER] > -1)
		{
			context->VSSetConstantBuffers(0, renderer->m_max_CB_bindpoint[NGL_VERTEX_SHADER] + 1, (ID3D11Buffer**)zeros);
		}
		if (renderer->m_max_CB_bindpoint[NGL_VERTEX_SHADER] > -1)
		{
			context->VSSetShaderResources(0, renderer->m_max_SR_bindpoint[NGL_VERTEX_SHADER] + 1, (ID3D11ShaderResourceView**)zeros);
			context->VSSetSamplers(0, renderer->m_max_SR_bindpoint[NGL_VERTEX_SHADER] + 1, (ID3D11SamplerState**)zeros);
		}

		if (renderer->m_max_CB_bindpoint[NGL_FRAGMENT_SHADER] > -1)
		{
			context->PSSetConstantBuffers(0, renderer->m_max_CB_bindpoint[NGL_FRAGMENT_SHADER] + 1, (ID3D11Buffer**)zeros);
		}
		if (renderer->m_max_SR_bindpoint[NGL_FRAGMENT_SHADER] > -1)
		{
			context->PSSetShaderResources(0, renderer->m_max_SR_bindpoint[NGL_FRAGMENT_SHADER] + 1, (ID3D11ShaderResourceView**)zeros);
			context->PSSetSamplers(0, renderer->m_max_SR_bindpoint[NGL_FRAGMENT_SHADER] + 1, (ID3D11SamplerState**)zeros);
		}
	}
}


ID3DBlob* D3D11_renderer::CompileShader(const std::string &str, const std::string &entry_point, const std::string &version, std::string &info_string)
{
	std::stringstream sstream;
	sstream << str << "#_|_#" << entry_point << "#_|_#" << version;

	auto it = D3D11_instance::This->m_shader_cache.find(sstream.str());
	if (it != D3D11_instance::This->m_shader_cache.end())
	{
		return it->second;
	}

	ID3DBlob *S = 0;
	ID3DBlob *err = 0;

	D3DCompile( &str[0], str.size(), 0, 0, 0, entry_point.c_str(), version.c_str(), 0, 0, &S, &err);

	const bool has_error = err && !S;
	const bool has_warning = err && S;

	if (has_error || (has_warning && LOG_WARNINGS))
	{
		// Compiler info
		std::string buffer(err->GetBufferSize() + 1, 0);
		memcpy(&buffer[0], err->GetBufferPointer(), err->GetBufferSize());

		SafeRelease(err);

		// Print source code
		std::string line;
		int line_count = 1;
		int error_count = 0;
		std::stringstream sstream(str);
		while (std::getline(sstream, line))
		{
			bool suppressed_warning = buffer.find("pow(f, e) will not work for negative") != std::string::npos;
			suppressed_warning |= buffer.find("loop only executes for 1 iteration") != std::string::npos;

			if (suppressed_warning == false)
			{
				_logf("%d: %s", line_count, line.c_str());
				line_count++;
				error_count++;
			}
		}

		if (error_count > 0)
		{
			_logf("Shader log:: %s", buffer.data());

			_logf("Info: %s", info_string.c_str());
		}
	}

	if (S)
	{
		D3D11_instance::This->m_shader_cache[sstream.str()] = S;
	}

	return S;
}


void D3D11_renderer::LinkShader()
{
}


bool D3D11_renderer::GetActiveAttribs2(_shader_reflection &r, uint32_t num_vbos, uint32_t *vbos, std::vector<D3D11_INPUT_ELEMENT_DESC> &input_elements)
{
	std::map<uint32_t, D3D11_renderer::_used_vertex_buffer> used_layouts;

	for( size_t i=0; i<r.attributes.size(); i++)
	{
		_shader_reflection::Block &attrib = r.attributes[i];

		bool found;
		D3D11_vertex_buffer *vb = NULL;
		uint32_t idx = 0;
		uint32_t buffer_idx;

		for (buffer_idx = 0; buffer_idx<num_vbos; buffer_idx++)
		{
			vb = D3D11_instance::This->m_vertex_buffers[vbos[buffer_idx]];
			found = SearchAttribBySemanticAndSize(vb->m_vertex_descriptor, idx, attrib.name, attrib.format);
			if (found)
			{
				break;
			}
		}

		if (!found)
		{
			return false;
		}

		D3D11_INPUT_ELEMENT_DESC l;
		ZeroMemory(&l, sizeof(D3D11_INPUT_ELEMENT_DESC));

		l.SemanticName = vb->m_vertex_descriptor.m_attribs[idx].m_semantic.c_str();
		l.SemanticIndex = 0;
		l.InputSlot = buffer_idx;
		l.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
		l.InstanceDataStepRate = 0;
		l.AlignedByteOffset = vb->m_vertex_descriptor.m_attribs[idx].m_offset;
		switch (vb->m_vertex_descriptor.m_attribs[idx].m_format)
		{
		case NGL_R32_FLOAT:
		{
			l.Format = DXGI_FORMAT_R32_FLOAT;
			break;
		}
		case NGL_R32_G32_FLOAT:
		{
			l.Format = DXGI_FORMAT_R32G32_FLOAT;
			break;
		}
		case NGL_R32_G32_B32_FLOAT:
		{
			l.Format = DXGI_FORMAT_R32G32B32_FLOAT;
			break;
		}
		case NGL_R32_G32_B32_A32_FLOAT:
		{
			l.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
			break;
		}
		case NGL_R8_UINT:
		{
			l.Format = DXGI_FORMAT_R8_UINT;
			break;
		}
		case NGL_R8_G8_UINT:
		{
			l.Format = DXGI_FORMAT_R8G8_UINT;
			break;
		}
		case NGL_R8_G8_B8_A8_UINT:
		{
			l.Format = DXGI_FORMAT_R8G8B8A8_UINT;
			break;
		}
		default:
		{
			_logf("Unhandled attrib format!\n");
		}
		}

		input_elements.push_back(l);

		used_layouts[buffer_idx].m_slot = buffer_idx;
		used_layouts[buffer_idx].m_buffer_idx = buffer_idx;
	}

	{
		std::map<uint32_t, D3D11_renderer::_used_vertex_buffer>::iterator i = used_layouts.begin();
		while( i != used_layouts.end())
		{
			m_used_vbs.push_back( i->second);

			i++;
		}
	}

	return true;
}


void D3D11_renderer::GetActiveResources2(_shader_reflection &reflection, std::vector<NGL_shader_uniform> &application_uniforms)
{
	std::map<std::string, NGL_used_uniform> uniforms[3];
	std::map<std::string, NGL_used_uniform> textures[3];
	NGL_shader_type shader_types[6] =
	{
		NGL_VERTEX_SHADER,
		NGL_TESS_CONTROL_SHADER,
		NGL_TESS_EVALUATION_SHADER,
		NGL_GEOMETRY_SHADER,
		NGL_FRAGMENT_SHADER,
		NGL_COMPUTE_SHADER
	};

	for (size_t g = 0; g < 3; g++)
	{
		for (uint32_t shader_type = NGL_VERTEX_SHADER; shader_type < NGL_NUM_SHADER_TYPES; shader_type++)
		{
			m_uniform_groups[g].m_ubo_memory_sizes[shader_type] = 0;
		}
	}

	for (uint32_t shader_type = NGL_VERTEX_SHADER; shader_type < NGL_NUM_SHADER_TYPES; shader_type++)
	{
		m_max_SR_bindpoint[shader_type] = -1;
		m_max_UAV_bindpoint[shader_type] = -1;
		m_max_CB_bindpoint[shader_type] = -1;
	}

	for (size_t i = 0; i < reflection.uniforms.size(); i++)
	{
		_shader_reflection::Block &mb = reflection.uniforms[i];

		uint32_t group;
		int32_t application_location;

		FindUniform(group, application_location, application_uniforms, mb.name);

		if (mb.format == 1000) // ubo
		{
			for (size_t i = 0; i < mb.blocks.size(); i++)
			{
				_shader_reflection::Block &b = mb.blocks[i];

				FindUniform(group, application_location, application_uniforms, b.name);

				NGL_used_uniform &uu = uniforms[group][b.name];

				uu.m_uniform.m_name = b.name;
				uu.m_uniform.m_size = b.size;
				uu.m_application_location = application_location;
				uu.m_shader_location[shader_types[b.stage]] = b.binding_or_offset_or_location;

				switch (b.format)
				{
				case 1:
				{
					uu.m_uniform.m_format = NGL_FLOAT;
					break;
				}
				case 2:
				{
					uu.m_uniform.m_format = NGL_FLOAT2;
					break;
				}
				case 4:
				{
					uu.m_uniform.m_format = NGL_FLOAT4;
					break;
				}
				case 5:
				{
					uu.m_uniform.m_format = NGL_FLOAT16;
					break;
				}
				default:
				{
					assert(0);
				}
				}
			}

			m_uniform_groups[group].m_ubos[shader_types[mb.stage]].Allocate(mb.size);
			m_uniform_groups[group].m_ubo_memory_sizes[shader_types[mb.stage]] = mb.size;
			m_uniform_groups[group].m_ubo_binding_points[shader_types[mb.stage]] = mb.binding_or_offset_or_location;
			m_max_CB_bindpoint[shader_types[mb.stage]] = max(mb.binding_or_offset_or_location, m_max_CB_bindpoint[shader_types[mb.stage]]);
		}
		else if (mb.format == 1001) // ssbo
		{
			NGL_used_uniform &uu = textures[group][mb.name];

			uu.m_uniform.m_name = mb.name;
			uu.m_uniform.m_size = mb.size;
			uu.m_uniform.m_format = NGL_BUFFER;
			uu.m_binding_type = 1;
			uu.m_shader_location[shader_types[mb.stage]] = mb.binding_or_offset_or_location;
			uu.m_application_location = application_location;

			m_max_UAV_bindpoint[shader_types[mb.stage]] = max(mb.binding_or_offset_or_location, m_max_UAV_bindpoint[shader_types[mb.stage]]);

			if (application_uniforms[application_location].m_format == NGL_BUFFER_SUBRESOURCE)
			{
				uu.m_uniform.m_format = NGL_BUFFER_SUBRESOURCE;
			}
		}
		else if (mb.format == 1002) // readonly ssbo
		{
			NGL_used_uniform &uu = textures[group][mb.name];

			uu.m_uniform.m_name = mb.name;
			uu.m_uniform.m_size = mb.size;
			uu.m_uniform.m_format = NGL_BUFFER;
			uu.m_binding_type = 0;
			uu.m_shader_location[shader_types[mb.stage]] = mb.binding_or_offset_or_location;
			uu.m_application_location = application_location;

			m_max_SR_bindpoint[shader_types[mb.stage]] = max(mb.binding_or_offset_or_location, m_max_SR_bindpoint[shader_types[mb.stage]]);

			if (application_uniforms[application_location].m_format == NGL_BUFFER_SUBRESOURCE)
			{
				uu.m_uniform.m_format = NGL_BUFFER_SUBRESOURCE;
			}
		}
		else if (mb.format == 2000 || mb.format == 2001 || mb.format == 2002) // sampler, shadow sampler, subpass input
		{
			NGL_used_uniform &uu = textures[group][mb.name];

			uu.m_uniform.m_name = mb.name;
			uu.m_uniform.m_size = mb.size;
			uu.m_uniform.m_format = NGL_TEXTURE;
			uu.m_binding_type = 0; // Texture without sampler
			uu.m_binding_type = mb.format == 2000 ? 1 : uu.m_binding_type; // Sampler
			uu.m_binding_type = mb.format == 2001 ? 2 : uu.m_binding_type; // Shadow sampler
			uu.m_shader_location[shader_types[mb.stage]] = mb.binding_or_offset_or_location;
			uu.m_application_location = application_location;
			m_max_SR_bindpoint[shader_types[mb.stage]] = max(mb.binding_or_offset_or_location, m_max_SR_bindpoint[shader_types[mb.stage]]);

			if (application_uniforms[application_location].m_format == NGL_TEXTURE_SUBRESOURCE)
			{
				uu.m_uniform.m_format = NGL_TEXTURE_SUBRESOURCE;
			}
		}
		else if (mb.format == 2003) // image
		{
			NGL_used_uniform &uu = textures[group][mb.name];

			uu.m_uniform.m_name = mb.name;
			uu.m_uniform.m_size = mb.size;
			uu.m_uniform.m_format = NGL_TEXTURE;
			uu.m_binding_type = 10; // RWTexture
			uu.m_shader_location[shader_types[mb.stage]] = mb.binding_or_offset_or_location;
			uu.m_application_location = application_location;

			m_max_UAV_bindpoint[shader_types[mb.stage]] = max(mb.binding_or_offset_or_location, m_max_UAV_bindpoint[shader_types[mb.stage]]);
		}
		else
		{
			_logf("DX11 - GetActiveResources: unknown reflection format: %d", mb.format);
			assert(0);
		}
	}

	for (int g = 0; g < 3; g++)
	{
		for (std::map<std::string, NGL_used_uniform>::iterator i = uniforms[g].begin(); i != uniforms[g].end(); i++)
		{
			m_used_uniforms[g].push_back(i->second);
		}

		for (std::map<std::string, NGL_used_uniform>::iterator i = textures[g].begin(); i != textures[g].end(); i++)
		{
			m_used_uniforms[g].push_back(i->second);
		}
	}
}


void D3D11_uniform_group::BindUniform(const NGL_used_uniform &uu, const void *ptr)
{
	uint32_t uu_size = 0;

	switch (uu.m_uniform.m_format)
	{
	case NGL_TEXTURE:
	case NGL_TEXTURE_SUBRESOURCE:
	{
		if (uu.m_binding_type == 10) // RWTexture
		{
			if (uu.m_shader_location[NGL_COMPUTE_SHADER] > -1)
			{
				assert(uu.m_uniform.m_format == NGL_TEXTURE); // Unique subresource as UAV is not yet supported

				D3D11_texture *texture = D3D11_instance::This->m_textures[*(uint32_t*)ptr];
				D3D11_instance::This->m_d3dContext->CSSetUnorderedAccessViews(uu.m_shader_location[NGL_COMPUTE_SHADER], 1, &texture->m_uav, 0);
			}
		}
		else
		{
			// Select the texture and the shader resource view (default or subresource specific)
			D3D11_texture *texture;
			ID3D11ShaderResourceView *srv;
			if (uu.m_uniform.m_format == NGL_TEXTURE_SUBRESOURCE)
			{
				const NGL_texture_subresource &subresource = *(NGL_texture_subresource*)ptr;
				texture = D3D11_instance::This->m_textures[subresource.m_idx];
				srv = texture->GetSRV(subresource);
			}
			else
			{
				texture = D3D11_instance::This->m_textures[*(uint32_t*)ptr];
				srv = texture->m_default_srv;
			}

			// Select the sampler
			ID3D11SamplerState *sampler = nullptr;
			if (uu.m_binding_type == 1)
			{
				sampler = texture->m_sampler;
			}
			else if (uu.m_binding_type == 2)
			{
				sampler = D3D11_instance::This->m_shadow_sampler;
			}

			// Bind the texture and the sampler
			if (uu.m_shader_location[NGL_VERTEX_SHADER] > -1)
			{
				D3D11_instance::This->m_d3dContext->VSSetShaderResources(uu.m_shader_location[NGL_VERTEX_SHADER], 1, &srv);
				if (sampler)
				{
					D3D11_instance::This->m_d3dContext->VSSetSamplers(uu.m_shader_location[NGL_VERTEX_SHADER], 1, &sampler);
				}
			}
			if (uu.m_shader_location[NGL_FRAGMENT_SHADER] > -1)
			{
				D3D11_instance::This->m_d3dContext->PSSetShaderResources(uu.m_shader_location[NGL_FRAGMENT_SHADER], 1, &srv);
				if (sampler)
				{
					D3D11_instance::This->m_d3dContext->PSSetSamplers(uu.m_shader_location[NGL_FRAGMENT_SHADER], 1, &sampler);
				}
			}
			if (uu.m_shader_location[NGL_COMPUTE_SHADER] > -1)
			{
				D3D11_instance::This->m_d3dContext->CSSetShaderResources(uu.m_shader_location[NGL_COMPUTE_SHADER], 1, &srv);
				if (sampler)
				{
					D3D11_instance::This->m_d3dContext->CSSetSamplers(uu.m_shader_location[NGL_COMPUTE_SHADER], 1, &sampler);
				}
			}
		}
		break;
	}
	case NGL_BUFFER:
	case NGL_BUFFER_SUBRESOURCE:
	{
		D3D11_vertex_buffer *buffer = nullptr;
		ID3D11ShaderResourceView *srv = nullptr;
		ID3D11UnorderedAccessView *uav = nullptr;
		NGL_buffer_subresource subresource(0);

		if (uu.m_uniform.m_format == NGL_BUFFER_SUBRESOURCE)
		{
			subresource = *(NGL_buffer_subresource*)ptr;
			buffer = D3D11_instance::This->m_vertex_buffers[subresource.m_buffer];
		}
		else
		{
			buffer = D3D11_instance::This->m_vertex_buffers[*(uint32_t*)ptr];
		}

		if (uu.m_binding_type == 0)
		{
			if (uu.m_uniform.m_format == NGL_BUFFER_SUBRESOURCE)
			{
				srv = buffer->GetSRV(subresource);
			}
			else
			{
				srv = buffer->m_srv;
			}

			if (uu.m_shader_location[NGL_VERTEX_SHADER] > -1)
			{
				D3D11_instance::This->m_d3dContext->VSSetShaderResources(uu.m_shader_location[NGL_VERTEX_SHADER], 1, &srv);
			}
			if (uu.m_shader_location[NGL_FRAGMENT_SHADER] > -1)
			{
				D3D11_instance::This->m_d3dContext->PSSetShaderResources(uu.m_shader_location[NGL_FRAGMENT_SHADER], 1, &srv);
			}
			if (uu.m_shader_location[NGL_COMPUTE_SHADER] > -1)
			{
				D3D11_instance::This->m_d3dContext->CSSetShaderResources(uu.m_shader_location[NGL_COMPUTE_SHADER], 1, &srv);
			}
		}
		else
		{
			if (uu.m_uniform.m_format == NGL_BUFFER_SUBRESOURCE)
			{
				uav = buffer->GetUAV(subresource);
			}
			else
			{
				uav = buffer->m_uav;
			}

			if (uu.m_shader_location[NGL_COMPUTE_SHADER] > -1)
			{
				D3D11_instance::This->m_d3dContext->CSSetUnorderedAccessViews(uu.m_shader_location[NGL_COMPUTE_SHADER], 1, &uav, 0);
			}
		}

		break;
	}
	case NGL_FLOAT16:
	{
		uu_size = 64;

		break;
	}
	case NGL_FLOAT4:
	{
		uu_size = 16;
		break;
	}
	case NGL_FLOAT:
	{
		uu_size = 4;
		break;
	}
	case NGL_FLOAT2:
	{
		uu_size = 8;
		break;
	}
	case NGL_INT:
	{
		uu_size = 4;
		break;
	}
	case NGL_INT2:
	{
		uu_size = 8;
		break;
	}
	case NGL_INT4:
	{
		uu_size = 16;
		break;
	}
	case NGL_UINT:
	{
		uu_size = 4;
		break;
	}
	case NGL_UINT2:
	{
		uu_size = 8;
		break;
	}
	case NGL_UINT4:
	{
		uu_size = 16;
		break;
	}
	default:
	{
		_logf("Warning!! Unhandled bind uniform!!\n");
	}
	}
	if (uu_size)
	{
		for (uint32_t shader_type = NGL_VERTEX_SHADER; shader_type < NGL_NUM_SHADER_TYPES; shader_type++)
		{
			if (uu.m_shader_location[shader_type] > -1)
			{
				memcpy(m_ubos[shader_type].m_mapped_ptr + uu.m_shader_location[shader_type], ptr, uu_size * uu.m_uniform.m_size);
			}
		}
	}
}


void D3D11_uniform_group::MapUBO(uint32_t shader_type)
{
	D3D11_ubo &ubo = m_ubos[shader_type];

	assert(!ubo.m_mapped_ptr);

	D3D11_MAPPED_SUBRESOURCE subresource;
	DX_THROW_IF_FAILED(D3D11_instance::This->m_d3dContext->Map(ubo.m_buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &subresource));

	ubo.m_mapped_ptr = (uint8_t*)subresource.pData;

	switch (shader_type)
	{
	case NGL_VERTEX_SHADER:
	{
		D3D11_instance::This->m_d3dContext->VSSetConstantBuffers(m_ubo_binding_points[shader_type], 1, &ubo.m_buffer);
		break;
	}
	case NGL_FRAGMENT_SHADER:
	{
		D3D11_instance::This->m_d3dContext->PSSetConstantBuffers(m_ubo_binding_points[shader_type], 1, &ubo.m_buffer);
		break;
	}
	case NGL_COMPUTE_SHADER:
	{
		D3D11_instance::This->m_d3dContext->CSSetConstantBuffers(m_ubo_binding_points[shader_type], 1, &ubo.m_buffer);
		break;
	}
	default:
		assert(0);
	}
}


void D3D11_uniform_group::UnmapUBO(uint32_t shader_type)
{
	D3D11_ubo &ubo = m_ubos[shader_type];

	assert(ubo.m_mapped_ptr);

	D3D11_instance::This->m_d3dContext->Unmap(ubo.m_buffer, 0);

	ubo.m_mapped_ptr = 0;
}


static DXGI_FORMAT GetDSVFormat(DXGI_FORMAT format)
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


static DXGI_FORMAT GetDepthFormat(DXGI_FORMAT format)
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


static D3D11_SAMPLER_DESC DescribeSampler(const NGL_texture_descriptor &texture_descriptor)
{
	D3D11_SAMPLER_DESC desc;
	ZeroMemory(&desc, sizeof(D3D11_SAMPLER_DESC));
	desc.MinLOD = 0.0f; 
	desc.MaxLOD = float(texture_descriptor.m_num_levels) - 1.0f;
	desc.ComparisonFunc = D3D11_COMPARISON_NEVER;

	switch (texture_descriptor.m_filter)
	{
	case NGL_NEAREST:
	{
		desc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
		break;
	}
	case NGL_LINEAR:
	{
		desc.Filter = D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT;
		break;
	}
	case NGL_NEAREST_MIPMAPPED:
	{
		desc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
		break;
	}
	case NGL_LINEAR_MIPMAPPED:
	{
		desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		break;
	}
	case NGL_ANISO_4:
	{
		desc.Filter = D3D11_FILTER_ANISOTROPIC;
		desc.MaxAnisotropy = 4;
		break;
	}
	default:
	{
		_logf("DX11 - Unsupported texture filter: %d, %s", texture_descriptor.m_filter, texture_descriptor.m_name.c_str());
		assert(0);
	}
	}

	switch (texture_descriptor.m_wrap_mode)
	{
	case NGL_REPEAT_ALL:
	{
		desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
		desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
		desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
		break;
	}
	case NGL_CLAMP_ALL:
	{
		desc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
		desc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
		desc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
		break;
	}
	default:
	{
		_logf("DX11 - Unsupported warp mode: %d, %s", texture_descriptor.m_wrap_mode, texture_descriptor.m_name.c_str());
		assert(0);
	}
	}

	return desc;
}


static D3D11_SHADER_RESOURCE_VIEW_DESC DescribeSRV(const D3D11_texture *texture, uint32_t first_mipmap, uint32_t mipmap_levels, uint32_t first_array_level, uint32_t array_levels)
{
	D3D11_SHADER_RESOURCE_VIEW_DESC desc;
	ZeroMemory(&desc, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));

	switch (texture->m_texture_descriptor.m_type)
	{
	case NGL_TEXTURE_2D:
	case NGL_RENDERBUFFER:
		desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		desc.Texture2D.MostDetailedMip = first_mipmap;
		desc.Texture2D.MipLevels = mipmap_levels;
		break;

	case NGL_TEXTURE_2D_ARRAY:
		desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
		desc.Texture2DArray.MostDetailedMip = first_mipmap;
		desc.Texture2DArray.MipLevels = mipmap_levels;
		desc.Texture2DArray.FirstArraySlice = first_array_level;
		desc.Texture2DArray.ArraySize = array_levels;
		break;

	case NGL_TEXTURE_CUBE:
		desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
		desc.TextureCube.MostDetailedMip = first_mipmap;
		desc.TextureCube.MipLevels = mipmap_levels;
		break;

	default:
		assert(false);
		break;
	}

	if (texture->m_texture_descriptor.m_is_renderable && texture->m_is_color == false)
	{
		desc.Format = GetDepthFormat(texture->m_descriptor.Format);
	}
	else
	{
		desc.Format = texture->m_descriptor.Format;
	}

	return desc;
}


static D3D11_RENDER_TARGET_VIEW_DESC DescribeRTV(const D3D11_texture *texture, uint32_t mip_level, int32_t array_level)
{
	D3D11_RENDER_TARGET_VIEW_DESC desc;
	ZeroMemory(&desc, sizeof(D3D11_RENDER_TARGET_VIEW_DESC));

	desc.Format = texture->m_descriptor.Format;

	switch (texture->m_texture_descriptor.m_type)
	{
	case NGL_TEXTURE_2D:
		desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
		desc.Texture2D.MipSlice = mip_level;
		break;

	case NGL_TEXTURE_2D_ARRAY:
	case NGL_TEXTURE_CUBE:
		desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
		desc.Texture2DArray.MipSlice = mip_level;
		desc.Texture2DArray.ArraySize = 1;
		desc.Texture2DArray.FirstArraySlice = array_level;
		break;

	case NGL_RENDERBUFFER:
	default:
		assert(0);
		break;
	}

	return desc;
}


static D3D11_DEPTH_STENCIL_VIEW_DESC DescribeDSV(const D3D11_texture *texture, uint32_t mip_level, int32_t array_level)
{
	D3D11_DEPTH_STENCIL_VIEW_DESC desc;
	ZeroMemory(&desc, sizeof(D3D11_DEPTH_STENCIL_VIEW_DESC));

	desc.Format = GetDSVFormat(texture->m_descriptor.Format);

	switch (texture->m_texture_descriptor.m_type)
	{
	case NGL_TEXTURE_2D:
		desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		desc.Texture2D.MipSlice = mip_level;
		break;

	case NGL_TEXTURE_2D_ARRAY:
	case NGL_TEXTURE_CUBE:
		desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DARRAY;
		desc.Texture2DArray.MipSlice = mip_level;
		desc.Texture2DArray.ArraySize = 1;
		desc.Texture2DArray.FirstArraySlice = array_level;
		break;

	case NGL_RENDERBUFFER:
	default:
		assert(0);
		break;
	}

	return desc;
}


bool GenTexture(uint32_t &buffer, NGL_texture_descriptor &texture_layout, std::vector<std::vector<uint8_t> > *datas)
{
	if ((int32_t)texture_layout.m_size[0] > D3D11_instance::This->m_propertiesi[NGL_TEXTURE_MAX_SIZE_2D] || (int32_t)texture_layout.m_size[1] > D3D11_instance::This->m_propertiesi[NGL_TEXTURE_MAX_SIZE_2D])
	{
		_logf("m_GL_MAX_TEXTURE_SIZE");
		buffer = 0xBADF00D;
		return 0;
	}
	if (texture_layout.m_is_renderable && datas)
	{
		_logf("m_is_renderable && datas");
		buffer = 0xBADF00D;
		return 0;
	}
	if (!texture_layout.m_is_renderable && !datas && !texture_layout.m_unordered_access)
	{
		_logf("!m_is_renderable && !datas");
		buffer = 0xBADF00D;
		return 0;
	}

	if (buffer && buffer >= D3D11_instance::This->m_textures.size())
	{
		_logf("texture index overflow");
		buffer = 0xBADF00D;
		return false;
	}

	if (!buffer)
	{
		D3D11_instance::This->m_textures.push_back(new D3D11_texture());
		buffer = (uint32_t)D3D11_instance::This->m_textures.size() - 1;
	}
	else
	{
		D3D11_texture *texture = D3D11_instance::This->m_textures[buffer];

		texture->Release();
	}

	D3D11_texture *texture = D3D11_instance::This->m_textures[buffer];

	texture->m_texture_descriptor = texture_layout;
	texture->m_is_color = true;

	uint32_t block_dim_x = 1;
	uint32_t block_dim_y = 1;
	uint32_t block_size = 0; // in bits

	std::vector<uint8_t*> rgba_data;
	std::vector<uint8_t*> converted_data;
	if (datas)
	{
		rgba_data.resize(datas->size(), 0);

		bool need_rgb82rgba8_conversion = texture_layout.m_format == NGL_R8_G8_B8_UNORM || texture_layout.m_format == NGL_R8_G8_B8_UNORM_SRGB;

		if (need_rgb82rgba8_conversion)
		{
			converted_data.resize(datas->size(), 0);

			for (uint32_t i = 0; i < datas->size(); i++)
			{
				converted_data[i] = RGB888toRGBA8888(texture_layout.m_size[0], texture_layout.m_size[1], (*datas)[i].data());
				rgba_data[i] = converted_data[i];
			}
		}
		else
		{
			for (uint32_t i = 0; i < datas->size(); i++)
			{
				rgba_data[i] = (uint8_t*)(*datas)[i].data();
			}
		}
	}

	DXGI_FORMAT format = DXGI_FORMAT_UNKNOWN;

	switch (texture_layout.m_format)
	{
	case NGL_R8_UNORM:
	{
		format = DXGI_FORMAT_R8_UNORM;
		block_size = 8;
		break;
	}
	case NGL_R8_G8_B8_UNORM:
	{
		format = DXGI_FORMAT_R8G8B8A8_UNORM;
		block_size = 32;
		break;
	}
	case NGL_R8_G8_B8_UNORM_SRGB:
	{
		format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
		block_size = 32;
		break;
	}
	case NGL_R8_G8_B8_A8_UNORM:
	{
		format = DXGI_FORMAT_R8G8B8A8_UNORM;
		block_size = 32;
		break;
	}
	case NGL_R8_G8_B8_A8_UNORM_SRGB:
	{
		format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
		block_size = 32;
		break;
	}
	case NGL_R16_FLOAT:
	{
		format = DXGI_FORMAT_R16_FLOAT;
		block_size = 16;
		break;
	}
	case NGL_R32_FLOAT:
	{
		format = DXGI_FORMAT_R32_FLOAT;
		block_size = 32;
		break;
	}
	case NGL_R16_G16_B16_A16_FLOAT:
	{
		format = DXGI_FORMAT_R16G16B16A16_FLOAT;
		block_size = 64;
		break;
	}
	case NGL_R16_G16_FLOAT:
	{
		format = DXGI_FORMAT_R16G16_FLOAT;
		block_size = 32;
		break;
	}
	case NGL_R11_B11_B10_FLOAT:
	{
		format = DXGI_FORMAT_R11G11B10_FLOAT;
		block_size = 32;
		break;
	}
	case NGL_R10_G10_B10_A2_UNORM:
	{
		format = DXGI_FORMAT_R10G10B10A2_UNORM;
		block_size = 32;
		break;
	}
	case NGL_R32_G32_B32_A32_FLOAT:
	{
		format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		block_size = 128;
		break;
	}
	case NGL_R9_G9_B9_E5_SHAREDEXP:
	{
		format = DXGI_FORMAT_R9G9B9E5_SHAREDEXP;
		block_size = 32;
		break;
	}
	case NGL_R8_G8_B8_DXT1_UNORM:
	case NGL_R8_G8_B8_A1_DXT1_UNORM:
	{
		format = DXGI_FORMAT_BC1_UNORM;
		block_dim_x = 4;
		block_dim_y = 4;
		block_size = 64;
		break;
	}
	case NGL_R8_G8_B8_DXT1_UNORM_SRGB:
	case NGL_R8_G8_B8_A1_DXT1_UNORM_SRGB:
	{
		format = DXGI_FORMAT_BC1_UNORM_SRGB;
		block_dim_x = 4;
		block_dim_y = 4;
		block_size = 64;
		break;
	}
	case NGL_R8_G8_B8_A8_DXT5_UNORM:
	{
		format = DXGI_FORMAT_BC3_UNORM;
		block_dim_x = 4;
		block_dim_y = 4;
		block_size = 128;
		break;
	}
	case NGL_R8_G8_B8_A8_DXT5_UNORM_SRGB:
	{
		format = DXGI_FORMAT_BC3_UNORM_SRGB;
		block_dim_x = 4;
		block_dim_y = 4;
		block_size = 128;
		break;
	}
	case NGL_D16_UNORM:
	{
		format = DXGI_FORMAT_R16_TYPELESS;
		block_size = 16;
		texture->m_is_color = false;
		break;
	}
	case NGL_D24_UNORM:
	{
		//http://www.gamedev.net/topic/662891-sampling-the-depth-buffer-in-a-shader-in-dx11/
		format = DXGI_FORMAT_R24G8_TYPELESS;
		block_size = 32;
		texture->m_is_color = false;
		break;
	}
	default:
		_logf("DX11 - Unsupported texture type: %d", texture_layout.m_format);
		assert(0);
	}

	if (datas && texture_layout.m_num_levels > datas->size())
	{
		_logf("DX11 - Texture (%s) data does not contain enough mipmap levels! Mipmapping won't be used!", texture_layout.m_name.c_str());
		texture_layout.m_num_levels = 1;
	}

	texture->m_descriptor = CD3D11_TEXTURE2D_DESC(format, texture_layout.m_size[0], texture_layout.m_size[1]);
	// Bind flags
	{
		if (texture_layout.m_unordered_access)
		{
			texture->m_descriptor.BindFlags |= D3D11_BIND_UNORDERED_ACCESS;
		}
		if (texture_layout.m_type != NGL_RENDERBUFFER)
		{
			texture->m_descriptor.BindFlags |= D3D11_BIND_SHADER_RESOURCE;
		}
		if (texture_layout.m_is_renderable)
		{
			if (texture->m_is_color)
			{
				texture->m_descriptor.BindFlags |= D3D11_BIND_RENDER_TARGET;
			}
			else
			{
				texture->m_descriptor.BindFlags |= D3D11_BIND_DEPTH_STENCIL;
			}
		}
	}

	// Describe texture dimension
	{
		switch (texture->m_texture_descriptor.m_type)
		{
		case NGL_TEXTURE_2D:
			texture->m_descriptor.MipLevels = texture_layout.m_num_levels;
			texture->m_descriptor.ArraySize = 1;
			break;
		case NGL_TEXTURE_2D_ARRAY:
			texture->m_descriptor.MipLevels = texture_layout.m_num_levels;
			texture->m_descriptor.ArraySize = texture_layout.m_num_array;
			break;
		case NGL_TEXTURE_CUBE:
			texture->m_descriptor.MipLevels = texture_layout.m_num_levels;
			texture->m_descriptor.ArraySize = 6;
			texture->m_descriptor.MiscFlags |= D3D11_RESOURCE_MISC_TEXTURECUBE;
			break;
		case NGL_RENDERBUFFER:
			break;
		}
	}

 	texture->m_subresource_count = texture->m_descriptor.MipLevels * texture->m_descriptor.ArraySize;

	std::vector<D3D11_SUBRESOURCE_DATA> subresources;

	if (datas)
	{
		subresources.resize(texture->m_descriptor.MipLevels * texture->m_descriptor.ArraySize);

		uint32_t src_index = 0;

		for (uint32_t level = 0; level < texture->m_descriptor.MipLevels; level++)
		{
			// Calculate the dimensions of the mipmap
			uint32_t mipmap_width = texture_layout.m_size[0] / (1 << level);
			uint32_t mipmap_height = texture_layout.m_size[1] / (1 << level);
			mipmap_width = mipmap_width ? mipmap_width : 1;
			mipmap_height = mipmap_height ? mipmap_height : 1;

			// Calculate the number of blocks for the mipmap
			uint32_t block_count_x = (mipmap_width + block_dim_x - 1) / block_dim_x;
			uint32_t block_count_y = (mipmap_height + block_dim_y - 1) / block_dim_y;

			for (uint32_t array_level = 0; array_level < texture->m_descriptor.ArraySize; array_level++) // cube face or array surface
			{
				UINT subresource_id = D3D11CalcSubresource(level, array_level, texture->m_descriptor.MipLevels);

				const void *raw_data = rgba_data[src_index];
				subresources[subresource_id].pSysMem = raw_data;
				subresources[subresource_id].SysMemPitch = block_count_x * block_size / 8; // to bytes
				subresources[subresource_id].SysMemSlicePitch = block_count_y * subresources[subresource_id].SysMemPitch;

				src_index++;
			}
		}
	}

	DX_THROW_IF_FAILED(D3D11_instance::This->m_d3dDevice->CreateTexture2D(&texture->m_descriptor, subresources.empty() ? 0 : subresources.data(), &texture->m_tex));

	SetDebugObjectName(texture->m_tex, texture->m_texture_descriptor.m_name);

	for (size_t i = 0; i < converted_data.size(); i++)
	{
		delete[] converted_data[i];
	}

	if( texture_layout.m_is_renderable)
	{
		if( texture->m_is_color)
		{
			// Create the render-target views
			texture->m_rt_views.resize(texture->m_subresource_count, 0);
			for (uint32_t mipmap_level = 0; mipmap_level < texture->m_descriptor.MipLevels; mipmap_level++)
			{
				for (uint32_t array_level = 0; array_level < texture->m_descriptor.ArraySize; array_level++) // cube face or array surface
				{
					UINT id = D3D11CalcSubresource(mipmap_level, array_level, texture->m_descriptor.MipLevels);

					D3D11_RENDER_TARGET_VIEW_DESC desc = DescribeRTV(texture, mipmap_level, array_level);
					DX_THROW_IF_FAILED(D3D11_instance::This->m_d3dDevice->CreateRenderTargetView(texture->m_tex, &desc, &texture->m_rt_views[id]));
					SetDebugObjectName(texture->m_rt_views[id], texture->m_texture_descriptor.m_name + "_rtv");
				}
			}
		}
		else
		{
			// Create the depth-stencil views
			texture->m_ds_views.resize(texture->m_subresource_count, 0);
			texture->m_readonly_ds_views.resize(texture->m_subresource_count, 0);
			for (uint32_t mipmap_level = 0; mipmap_level < texture->m_descriptor.MipLevels; mipmap_level++)
			{
				for (uint32_t array_level = 0; array_level < texture->m_descriptor.ArraySize; array_level++) // cube face or array surface
				{
					UINT id = D3D11CalcSubresource(mipmap_level, array_level, texture->m_descriptor.MipLevels);

					// Read-write view
					D3D11_DEPTH_STENCIL_VIEW_DESC desc = DescribeDSV(texture, mipmap_level, array_level);
					DX_THROW_IF_FAILED(D3D11_instance::This->m_d3dDevice->CreateDepthStencilView(texture->m_tex, &desc, &texture->m_ds_views[id]));

					// Read only view
					desc.Flags = D3D11_DSV_READ_ONLY_DEPTH | D3D11_DSV_READ_ONLY_STENCIL;
					DX_THROW_IF_FAILED(D3D11_instance::This->m_d3dDevice->CreateDepthStencilView(texture->m_tex, &desc, &texture->m_readonly_ds_views[id]));

					SetDebugObjectName(texture->m_ds_views[id], texture->m_texture_descriptor.m_name + "_dsv");
					SetDebugObjectName(texture->m_readonly_ds_views[id], texture->m_texture_descriptor.m_name + "_readonly_dsv");
				}
			}
		}
	}

	if (texture_layout.m_type != NGL_RENDERBUFFER)
	{
		// Create the default shader-resource views
		{
			D3D11_SHADER_RESOURCE_VIEW_DESC desc = DescribeSRV(texture, 0, texture->m_descriptor.MipLevels, 0, texture->m_descriptor.ArraySize);

			DX_THROW_IF_FAILED(
				D3D11_instance::This->m_d3dDevice->CreateShaderResourceView(
					texture->m_tex,
					&desc,
					&texture->m_default_srv)
			);

			SetDebugObjectName(texture->m_default_srv, texture->m_texture_descriptor.m_name + "_srv");
		}

		{
			// Create the shader-resource views for every subresources
			if (texture->m_texture_descriptor.m_is_renderable && texture->m_subresource_count > 1)
			{
				texture->m_sr_views.resize(texture->m_subresource_count, 0);
				for (uint32_t mipmap_level = 0; mipmap_level < texture->m_descriptor.MipLevels; mipmap_level++)
				{
					for (uint32_t array_level = 0; array_level < texture->m_descriptor.ArraySize; array_level++) // cube face or array surface
					{
						UINT id = D3D11CalcSubresource(mipmap_level, array_level, texture->m_descriptor.MipLevels);

						D3D11_SHADER_RESOURCE_VIEW_DESC desc = DescribeSRV(texture, mipmap_level, 1, array_level, 1);
						DX_THROW_IF_FAILED(D3D11_instance::This->m_d3dDevice->CreateShaderResourceView(texture->m_tex, &desc, &texture->m_sr_views[id]));

						std::stringstream sstream;
						sstream << texture->m_texture_descriptor.m_name << "_srv_" << mipmap_level << '_' << array_level;
						SetDebugObjectName(texture->m_sr_views[id], sstream.str());
					}
				}
			}
		}

		// Create the sampler
		{
			D3D11_SAMPLER_DESC desc = DescribeSampler(texture->m_texture_descriptor);
			texture->m_sampler = D3D11_instance::This->GetSamplerState(desc);
		}
	}


	if (texture->m_texture_descriptor.m_unordered_access)
	{
		if (texture->m_texture_descriptor.m_type != NGL_TEXTURE_2D)
		{
			_logf("DX11 - GenTexture: UAV is only implemented for texture 2D: %s", texture->m_texture_descriptor.m_name.c_str());
			assert(false);
		}

		D3D11_UNORDERED_ACCESS_VIEW_DESC uav_desc;
		ZeroMemory(&uav_desc, sizeof(uav_desc));
		uav_desc.Format = DXGI_FORMAT_UNKNOWN;
		uav_desc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
		uav_desc.Texture2D.MipSlice = 0;
		DX_THROW_IF_FAILED(D3D11_instance::This->m_d3dDevice->CreateUnorderedAccessView(texture->m_tex, &uav_desc, &texture->m_uav));
		SetDebugObjectName(texture->m_uav, texture->m_texture_descriptor.m_name + "_uav");
	}

	return true;
}


bool GenVertexBuffer(uint32_t &buffer, NGL_vertex_descriptor &vertex_layout, uint32_t num, void *data)
{
	if (buffer && buffer >= D3D11_instance::This->m_vertex_buffers.size())
	{
		_logf("vertex buffer index overflow");
		buffer = 0xBADF00D;
		return false;
	}

	D3D11_vertex_buffer *vb;

	if (!buffer)
	{
		vb = new D3D11_vertex_buffer();
		D3D11_instance::This->m_vertex_buffers.push_back(vb);

		buffer = (uint32_t)D3D11_instance::This->m_vertex_buffers.size() - 1;
	}
	else
	{
		vb = D3D11_instance::This->m_vertex_buffers[buffer];
		vb->Release();
	}

	vb->m_hash = GenerateHash(vertex_layout);
	vb->m_vertex_descriptor = vertex_layout;
	vb->m_datasize = num * vertex_layout.m_stride;

	ID3D11Buffer **buffer_ = nullptr;

	D3D11_BUFFER_DESC desc;
	ZeroMemory(&desc, sizeof(desc));

	if (!vertex_layout.m_unordered_access) // vertex buffer
	{
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.ByteWidth = vertex_layout.m_stride * num;
		desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		desc.CPUAccessFlags = 0;

		buffer_ = &vb->pVBuffer;
	}
	else // UAV
	{
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.ByteWidth = vertex_layout.m_stride * num;
		desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
		desc.CPUAccessFlags = 0;

		desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
		desc.StructureByteStride = vertex_layout.m_stride;

		buffer_ = &vb->m_structured_buffer;
	}

	D3D11_SUBRESOURCE_DATA init_data;
	init_data.pSysMem = data;
	init_data.SysMemPitch = 0;
	init_data.SysMemSlicePitch = 0;

	// Create the buffer
	DX_THROW_IF_FAILED(D3D11_instance::This->m_d3dDevice->CreateBuffer(&desc, data ? &init_data : nullptr, buffer_));
	SetDebugObjectName(*buffer_, "vertex_buffer");

	if (vertex_layout.m_unordered_access)
	{
		D3D11_SHADER_RESOURCE_VIEW_DESC srv_d;
		ZeroMemory(&srv_d, sizeof(srv_d));
		srv_d.Format = DXGI_FORMAT_UNKNOWN;
		srv_d.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
		srv_d.Buffer.FirstElement = 0;
		srv_d.Buffer.ElementWidth = vertex_layout.m_stride;
		srv_d.Buffer.NumElements = num;
		DX_THROW_IF_FAILED(D3D11_instance::This->m_d3dDevice->CreateShaderResourceView(vb->m_structured_buffer, &srv_d, &vb->m_srv));
		vb->m_base_srv_desc = srv_d;
		SetDebugObjectName(vb->m_srv, "vertex_buffer_srv");

		D3D11_UNORDERED_ACCESS_VIEW_DESC uav_d;
		ZeroMemory(&uav_d, sizeof(uav_d));
		uav_d.Format = DXGI_FORMAT_UNKNOWN;
		uav_d.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
		uav_d.Buffer.FirstElement = 0;
		uav_d.Buffer.NumElements = num;
		DX_THROW_IF_FAILED(D3D11_instance::This->m_d3dDevice->CreateUnorderedAccessView(vb->m_structured_buffer, &uav_d, &vb->m_uav));
		vb->m_base_uav_desc = uav_d;
		SetDebugObjectName(vb->m_uav, "vertex_buffer_uav");
	}

	return true;
}


bool GenIndexBuffer(uint32_t &buffer, NGL_format format, uint32_t num, void *data)
{
	if (buffer && buffer >= D3D11_instance::This->m_index_buffers.size())
	{
		_logf("index buffer index overflow");
		buffer = 0xBADF00D;
		return false;
	}

	D3D11_index_buffer *ib;

	if (!buffer)
	{
		ib = new D3D11_index_buffer();
		D3D11_instance::This->m_index_buffers.push_back(ib);

		buffer = (uint32_t)D3D11_instance::This->m_index_buffers.size() - 1;
	}
	else
	{
		ib = D3D11_instance::This->m_index_buffers[buffer];
		ib->Release();
	}

	uint32_t stride = 0;

	ib->m_format = format;
	ib->m_num_indices = num;

	switch (format)
	{
	case NGL_R16_UINT:
	{
		stride = 2;
		ib->m_D3D11_format = DXGI_FORMAT_R16_UINT;
		break;
	}
	case NGL_R32_UINT:
	{
		stride = 4;
		ib->m_D3D11_format = DXGI_FORMAT_R32_UINT;
		break;
	}
	default:
	{
		_logf("DX11 - GenIndexBuffer: Unknown index buffer format: %s", format);
		assert(0);
		break;
	}
	}

	D3D11_BUFFER_DESC desc;
	ZeroMemory(&desc, sizeof(desc));

	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.ByteWidth = stride * num;
	desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	desc.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA init_data;
	init_data.pSysMem = data;
	init_data.SysMemPitch = 0;
	init_data.SysMemSlicePitch = 0;

	DX_THROW_IF_FAILED(D3D11_instance::This->m_d3dDevice->CreateBuffer(&desc, data ? &init_data : nullptr, &ib->pIBuffer));
	SetDebugObjectName(ib->pIBuffer, "index_buffer");

	return true;
}


uint32_t GenJob(NGL_job_descriptor &descriptor)
{
	D3D11_job *job = new D3D11_job;

	if (!descriptor.m_load_shader_callback)
	{
		_logf("!!!error: m_load_shader_callback is not set for %s.\n", descriptor.m_subpasses[job->m_current_subpass].m_name.c_str());
	}

	job->m_descriptor = descriptor;

	job->CreateSubpasses();

	CreateJobFBO(job);

	D3D11_instance::This->m_jobs.push_back(job);

	return (uint32_t)D3D11_instance::This->m_jobs.size() - 1;
}


void ViewportScissor(uint32_t job_, int32_t viewport[4], int32_t scissor[4])
{
	D3D11_job *job = D3D11_instance::This->m_jobs[job_];

	if (viewport)
	{
		memcpy(job->m_current_state.m_viewport, viewport, sizeof(int32_t) * 4);
	}
	if (scissor)
	{
		memcpy(job->m_current_state.m_scissor, scissor, sizeof(int32_t) * 4);
	}
}


void LineWidth(uint32_t job_, float width)
{
	D3D11_job *job = D3D11_instance::This->m_jobs[job_];

	//glLineWidth(width);
}


void BlendState(uint32_t job_, uint32_t attachment, NGL_blend_func func, NGL_color_channel_mask mask)
{
	D3D11_job *job = D3D11_instance::This->m_jobs[job_];

	job->m_current_state.m_blend_state.m_funcs[attachment] = func;
	job->m_current_state.m_blend_state.m_masks[attachment] = mask;
}


void DepthState(uint32_t job_, NGL_depth_func func, bool mask)
{
	D3D11_job *job = D3D11_instance::This->m_jobs[job_];

	job->m_current_state.m_depth_state.m_func = func;
	job->m_current_state.m_depth_state.m_mask = mask;
}


void Begin(uint32_t job_, uint32_t command_buffer)
{
	D3D11_job *job = D3D11_instance::This->m_jobs[job_];
	const NGL_job_descriptor &descriptor = job->m_descriptor;

	if (job->m_is_started)
	{
		_logf("Error: job (%s) has already started!", descriptor.m_subpasses[job->m_current_subpass].m_name.c_str());
	}

	job->m_is_started = true;

	memset(&job->m_previous_state, ~0, sizeof(NGL_state));

	if (descriptor.m_is_compute)
	{
		return;
	}

	job->m_current_subpass = 0;

	D3D11_subpass &first_subpass = job->m_subpasses.front();
	first_subpass.ExecuteLoadOperations();
	first_subpass.BindRenderTargets();
}


void NextSubpass(uint32_t job_)
{
	D3D11_job *job = D3D11_instance::This->m_jobs[job_];
	const NGL_job_descriptor &descriptor = job->m_descriptor;

	job->m_current_subpass++;

	D3D11_subpass &prev_subpass = job->F();
	D3D11_subpass &current_subpass = job->G();

	prev_subpass.ExecuteStoreOperations();

	current_subpass.ExecuteLoadOperations();
	current_subpass.BindRenderTargets();
}


void End(uint32_t job_)
{
	D3D11_job *job = D3D11_instance::This->m_jobs[job_];
	const NGL_job_descriptor &descriptor = job->m_descriptor;

	if (!job->m_is_started)
	{
		_logf("Error: job (%s) hasn't started!", descriptor.m_subpasses[job->m_current_subpass].m_name.c_str());
	}

	job->m_is_started = false;

	if (job->m_active_renderer)
	{
		job->UnbindResources((D3D11_renderer*) job->m_active_renderer);

		if (descriptor.m_is_compute)
		{
			D3D11_instance::This->m_d3dContext->CSSetShader(0, 0, 0);
		}
		else
		{
			D3D11_instance::This->m_d3dContext->VSSetShader(0, 0, 0);
			D3D11_instance::This->m_d3dContext->PSSetShader(0, 0, 0);
		}
	}

	ID3D11RenderTargetView *null_fbo = 0;

	D3D11_instance::This->m_d3dContext->OMSetRenderTargets(1, &null_fbo, 0);
}


inline D3D11_PRIMITIVE_TOPOLOGY Predraw(D3D11_job *job, NGL_primitive_type primitive_type, uint32_t shader_code, uint32_t num_vbos, uint32_t *vbos, NGL_cull_mode cull_type, const void** parameters)
{
	const NGL_job_descriptor &descriptor = job->m_descriptor;
	const D3D11_subpass &current_subpass = job->G();
	D3D11_PRIMITIVE_TOPOLOGY primitive_GL_type = D3D_PRIMITIVE_TOPOLOGY_UNDEFINED;
	uint32_t uniform_update_mask = NGL_GROUP_PER_DRAW;

	if (!job->m_is_started)
	{
		_logf("Error: job (%s) hasn't started!", descriptor.m_subpasses[job->m_current_subpass].m_name.c_str());
	}

	job->m_current_state.m_subpass = (uint32_t)job->m_current_subpass;
	job->m_current_state.m_cull_mode = cull_type;
	job->m_current_state.m_primitive_type = primitive_type;
	job->m_current_state.m_shader.m_shader_code = shader_code;
	job->m_current_state.m_shader.m_vbo_hash = 0;
	for (uint32_t i = 0; i < num_vbos; i++)
	{
		job->m_current_state.m_shader.m_vbo_hash += D3D11_instance::This->m_vertex_buffers[vbos[i]]->m_hash;
	}

	uint32_t changed_mask = NGL_state::ChangedMask(job->m_current_state, job->m_previous_state);

	if (changed_mask & NGL_VIEWPORT_MASK || changed_mask & NGL_DEPTH_FUNC_MASK)
	{
		D3D11_VIEWPORT vp;

		vp.TopLeftX = (FLOAT)job->m_current_state.m_viewport[0];
		vp.TopLeftY = (FLOAT)job->m_current_state.m_viewport[1];
		vp.Width = (FLOAT)job->m_current_state.m_viewport[2];
		vp.Height = (FLOAT)job->m_current_state.m_viewport[3];
		vp.MinDepth = 0.0f;
		vp.MaxDepth = 1.0f;

		if (job->m_current_state.m_depth_state.m_func == NGL_DEPTH_TO_FAR)
		{
			vp.MinDepth = 1.0f;
		}

		D3D11_instance::This->m_d3dContext->RSSetViewports(1, &vp);
	}
	if (changed_mask & NGL_SCISSOR_MASK)
	{
		D3D11_RECT r;

		r.left = job->m_current_state.m_scissor[0];
		r.right = job->m_current_state.m_scissor[0] + job->m_current_state.m_scissor[2];
		r.bottom = job->m_current_state.m_scissor[1] + job->m_current_state.m_scissor[3];
		r.top = job->m_current_state.m_scissor[1];

		D3D11_instance::This->m_d3dContext->RSSetScissorRects(1, &r);
	}
	if (changed_mask & NGL_SHADER_MASK)
	{
		NGL_renderer *renderer = 0;

		for (size_t j = 0; j < job->m_renderers.size(); j++)
		{
			if (memcmp(&job->m_renderers[j]->m_my_state.m_shader, &job->m_current_state.m_shader, sizeof(uint32_t) * 2) == 0)
			{
				renderer = job->m_renderers[j];
				break;
			}
		}

		if (!renderer)
		{
			renderer = job->CreateRenderer(job->m_current_state, num_vbos, vbos);
		}

		if (job->m_active_renderer)
		{
			D3D11_renderer *renderer = (D3D11_renderer *)job->m_active_renderer;

			job->UnbindResources(renderer);
		}

		job->m_active_renderer = renderer;

		{
			D3D11_renderer *renderer = (D3D11_renderer *)job->m_active_renderer;

			D3D11_instance::This->m_d3dContext->VSSetShader(renderer->pVS, 0, 0);
			D3D11_instance::This->m_d3dContext->PSSetShader(renderer->pPS, 0, 0);
			D3D11_instance::This->m_d3dContext->IASetInputLayout(renderer->pLayout);
		}

		uniform_update_mask = NGL_GROUP_PER_DRAW | NGL_GROUP_PER_RENDERER_CHANGE | NGL_GROUP_MANUAL;
	}
	if (changed_mask & NGL_CULL_MODE_MASK || changed_mask & NGL_DEPTH_FUNC_MASK)
	{
		D3D11_instance::This->m_d3dContext->RSSetState(D3D11_instance::This->GetRasterizerState(job->m_current_state));
	}
	if (changed_mask & NGL_COLOR_BLEND_FUNCS_MASK || changed_mask & NGL_COLOR_MASKS_MASK)
	{
		D3D11_instance::This->m_d3dContext->OMSetBlendState(D3D11_instance::This->GetBlendState(job->m_current_state), NULL, 0xffffffff);
	}
	if (changed_mask & NGL_DEPTH_FUNC_MASK || changed_mask & NGL_DEPTH_MASK_MASK)
	{
		D3D11_instance::This->m_d3dContext->OMSetDepthStencilState(D3D11_instance::This->GetDepthState(job->m_current_state), 0);
	}

	switch (primitive_type)
	{
	case NGL_POINTS:
	{
		primitive_GL_type = D3D_PRIMITIVE_TOPOLOGY_POINTLIST;
		break;
	}
	case NGL_LINES:
	{
		primitive_GL_type = D3D_PRIMITIVE_TOPOLOGY_LINELIST;
		break;
	}
	case NGL_TRIANGLES:
	{
		primitive_GL_type = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		break;
	}
	case NGL_PATCH3:
	{
		primitive_GL_type = D3D_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST;
		break;
	}
	case NGL_PATCH4:
	{
		primitive_GL_type = D3D_PRIMITIVE_TOPOLOGY_4_CONTROL_POINT_PATCHLIST;
		break;
	}
	case NGL_PATCH16:
	{
		primitive_GL_type = D3D_PRIMITIVE_TOPOLOGY_16_CONTROL_POINT_PATCHLIST;
		break;
	}
	default:
		_logf("unknown primitive type\n");
	}

	D3D11_renderer *renderer = (D3D11_renderer *)job->m_active_renderer;

	for (uint32_t g = 0; g < 3; g++)
	{
		D3D11_uniform_group &group = renderer->m_uniform_groups[g];

		if (uniform_update_mask & (1 << g))
		{
			for (uint32_t shader_type = NGL_VERTEX_SHADER; shader_type < NGL_NUM_SHADER_TYPES; shader_type++)
			{
				if (group.m_ubo_memory_sizes[shader_type])
				{
					group.MapUBO(shader_type);
				}
			}

			for (size_t i = 0; i<renderer->m_used_uniforms[g].size(); i++)
			{
				const NGL_used_uniform &uu = renderer->m_used_uniforms[g][i];

				if (uu.m_application_location > -1)
				{
					if (parameters[uu.m_application_location])
					{
						group.BindUniform(uu, parameters[uu.m_application_location]);
					}
					else
					{
						_logf("not set uniform: %s in %s\n", uu.m_uniform.m_name.c_str(), job->m_descriptor.m_subpasses[job->m_current_subpass].m_name.c_str());
					}
				}
			}

			for (uint32_t shader_type = NGL_VERTEX_SHADER; shader_type < NGL_NUM_SHADER_TYPES; shader_type++)
			{
				if (group.m_ubo_memory_sizes[shader_type])
				{
					group.UnmapUBO(shader_type);
				}
			}
		}
	}

	for (size_t j = 0; j < renderer->m_used_vbs.size(); j++)
	{
		D3D11_renderer::_used_vertex_buffer &uvb = renderer->m_used_vbs[j];
		D3D11_vertex_buffer *vb = D3D11_instance::This->m_vertex_buffers[vbos[uvb.m_buffer_idx]];
		UINT offset = 0;

		if (vb->m_structured_buffer)
		{
			if (!vb->pVBuffer)
			{
				D3D11_BUFFER_DESC bd;
				ZeroMemory(&bd, sizeof(bd));

				bd.Usage = D3D11_USAGE_DEFAULT;
				bd.ByteWidth = vb->m_datasize;
				bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
				bd.CPUAccessFlags = 0;

				DX_THROW_IF_FAILED(D3D11_instance::This->m_d3dDevice->CreateBuffer(&bd, nullptr, &vb->pVBuffer));       // create the buffer
			}
			D3D11_instance::This->m_d3dContext->CopyResource(vb->pVBuffer, vb->m_structured_buffer);
		}

		D3D11_instance::This->m_d3dContext->IASetVertexBuffers(uvb.m_slot, 1, &vb->pVBuffer, &vb->m_vertex_descriptor.m_stride, &offset);
	}

	job->m_previous_state = job->m_current_state;

	return primitive_GL_type;
}


inline void Postdraw(D3D11_job *job)
{
	D3D11_renderer *renderer = (D3D11_renderer *)job->m_active_renderer;
}


void Draw(uint32_t job_, NGL_primitive_type primitive_type, uint32_t shader_code, uint32_t num_vbos, uint32_t *vbos, uint32_t ebo, NGL_cull_mode cull_type, const void** parameters)
{
	D3D11_job *job = D3D11_instance::This->m_jobs[job_];

	D3D11_PRIMITIVE_TOPOLOGY primitive_GL_type = Predraw(job, primitive_type, shader_code, num_vbos, vbos, cull_type, parameters);

	D3D11_index_buffer *ib = D3D11_instance::This->m_index_buffers[ebo];

	D3D11_instance::This->m_d3dContext->IASetIndexBuffer(ib->pIBuffer, ib->m_D3D11_format, 0);
	D3D11_instance::This->m_d3dContext->IASetPrimitiveTopology(primitive_GL_type);
	D3D11_instance::This->m_d3dContext->DrawIndexed(ib->m_num_indices, 0, 0);

	Postdraw(job);
}


void DrawInstanced(uint32_t job_, NGL_primitive_type primitive_type, uint32_t shader_code, uint32_t num_vbos, uint32_t *vbos, uint32_t ebo, NGL_cull_mode cull_type, const void** parameters, uint32_t instance_count)
{
	D3D11_job *job = D3D11_instance::This->m_jobs[job_];

	D3D11_PRIMITIVE_TOPOLOGY primitive_GL_type = Predraw(job, primitive_type, shader_code, num_vbos, vbos, cull_type, parameters);

	D3D11_index_buffer *ib = D3D11_instance::This->m_index_buffers[ebo];

	D3D11_instance::This->m_d3dContext->IASetIndexBuffer(ib->pIBuffer, ib->m_D3D11_format, 0);
	D3D11_instance::This->m_d3dContext->IASetPrimitiveTopology(primitive_GL_type);
	D3D11_instance::This->m_d3dContext->DrawIndexedInstanced(ib->m_num_indices, instance_count, 0, 0, 0);

	Postdraw(job);
}


void DrawIndirect(uint32_t job_, NGL_primitive_type primitive_type, uint32_t shader_code, uint32_t num_vbos, uint32_t *vbos, uint32_t ebo, NGL_cull_mode cull_type, const void** parameters, uint32_t indirect_buffer, void* indirect_buffer_offset)
{
	D3D11_job *job = D3D11_instance::This->m_jobs[job_];

	D3D11_PRIMITIVE_TOPOLOGY primitive_GL_type = Predraw(job, primitive_type, shader_code, num_vbos, vbos, cull_type, parameters);

	D3D11_index_buffer *ib = D3D11_instance::This->m_index_buffers[ebo];
	D3D11_vertex_buffer *vb = D3D11_instance::This->m_vertex_buffers[indirect_buffer];

	D3D11_instance::This->m_d3dContext->IASetIndexBuffer(ib->pIBuffer, ib->m_D3D11_format, 0);
	D3D11_instance::This->m_d3dContext->IASetPrimitiveTopology(primitive_GL_type);
	D3D11_instance::This->m_d3dContext->DrawIndexedInstancedIndirect(0, 0);

	Postdraw(job);
}


bool Predispatch(uint32_t job_, uint32_t shader_code, const void** parameters)
{
	D3D11_job *job = D3D11_instance::This->m_jobs[job_];
	const NGL_job_descriptor &descriptor = job->m_descriptor;
	uint32_t uniform_update_mask = NGL_GROUP_PER_DRAW;

	if (!job->m_is_started)
	{
		_logf("Error: job (%s) hasn't started!", descriptor.m_subpasses[job->m_current_subpass].m_name.c_str());
	}

	job->m_current_state.m_shader.m_shader_code = shader_code;
	job->m_current_state.m_shader.m_vbo_hash = 0;

	uint32_t changed_mask = NGL_state::ChangedMask(job->m_current_state, job->m_previous_state);

	if (changed_mask & NGL_SHADER_MASK)
	{
		NGL_renderer *renderer = 0;

		for (size_t j = 0; j < job->m_renderers.size(); j++)
		{
			if (memcmp(&job->m_renderers[j]->m_my_state.m_shader, &job->m_current_state.m_shader, sizeof(uint32_t) * 2) == 0)
			{
				renderer = job->m_renderers[j];
				break;
			}
		}

		if (!renderer)
		{
			renderer = job->CreateRenderer(job->m_current_state, 0, 0);
		}

		if (!renderer)
		{
			return false;
		}

		if (job->m_active_renderer)
		{
			D3D11_renderer *renderer = (D3D11_renderer *)job->m_active_renderer;

			job->UnbindResources(renderer);
		}

		job->m_active_renderer = renderer;

		{
			D3D11_renderer *renderer = (D3D11_renderer *)job->m_active_renderer;

			D3D11_instance::This->m_d3dContext->CSSetShader(renderer->pCS, 0, 0);
		}

		uniform_update_mask = NGL_GROUP_PER_DRAW | NGL_GROUP_PER_RENDERER_CHANGE | NGL_GROUP_MANUAL;
	}

	D3D11_renderer *renderer = (D3D11_renderer *)job->m_active_renderer;

	for (uint32_t g = 0; g < 3; g++)
	{
		D3D11_uniform_group &group = renderer->m_uniform_groups[g];

		if (uniform_update_mask & (1 << g))
		{
			if (group.m_ubo_memory_sizes[NGL_COMPUTE_SHADER])
			{
				group.MapUBO(NGL_COMPUTE_SHADER);
			}

			for (size_t i = 0; i<renderer->m_used_uniforms[g].size(); i++)
			{
				const NGL_used_uniform &uu = renderer->m_used_uniforms[g][i];

				if (uu.m_application_location > -1)
				{
					if (parameters[uu.m_application_location])
					{
						group.BindUniform(uu, parameters[uu.m_application_location]);
					}
					else
					{
						_logf("not set uniform: %s in %s\n", uu.m_uniform.m_name.c_str(), job->m_descriptor.m_subpasses[job->m_current_subpass].m_name.c_str());
					}
				}
			}

			if (group.m_ubo_memory_sizes[NGL_COMPUTE_SHADER])
			{
				group.UnmapUBO(NGL_COMPUTE_SHADER);
			}
		}
	}

	job->m_previous_state = job->m_current_state;

	return true;
}


bool Dispatch(uint32_t job_, uint32_t shader_code, uint32_t x, uint32_t y, uint32_t z, const void** parameters)
{
	bool s = Predispatch(job_, shader_code, parameters);
	if (!s) return false;

	D3D11_instance::This->m_d3dContext->Dispatch(x, y, z);

	return true;
}


bool DispatchIndirect(uint32_t job_, uint32_t shader_code, uint32_t x, uint32_t y, uint32_t z, const void** parameters, uint32_t indirect_buffer, void* indirect_buffer_offset)
{
	bool s = Predispatch(job_, shader_code, parameters);
	if (!s) return false;

	D3D11_vertex_buffer *buffer = D3D11_instance::This->m_vertex_buffers[*(uint32_t*)(size_t)indirect_buffer];

	D3D11_instance::This->m_d3dContext->DispatchIndirect(buffer->pVBuffer, (UINT)(size_t)indirect_buffer_offset);

	return true;
}


const char* GetString(NGL_backend_property prop)
{
	return D3D11_instance::This->m_propertiess[prop].c_str();
}


int32_t GetInteger(NGL_backend_property prop)
{
	return D3D11_instance::This->m_propertiesi[prop];
}


void DeletePipelines(uint32_t job_)
{
	D3D11_job *job = D3D11_instance::This->m_jobs[job_];

	job->DeleteRenderers();

	job->m_active_renderer = 0;
}


void CustomAction(uint32_t job_, uint32_t parameter)
{
	if (parameter == NGL_CUSTOM_ACTION_SWAPBUFFERS)
	{
		/*
		DXGI_PRESENT_PARAMETERS parameters = { 0 };
		parameters.DirtyRectsCount = 0;
		parameters.pDirtyRects = nullptr;
		parameters.pScrollRect = nullptr;
		parameters.pScrollOffset = nullptr;

		DX_THROW_IF_FAILED(D3D11_instance::This->m_swapChain->Present1(1, 0, &parameters));
		*/
	}
	else if (parameter == 100)
	{
	}
	else if (parameter == 101)
	{
	}
	else if (parameter == 102)
	{
	}
	else if (parameter == 200)
	{
		D3D11_instance::This->m_textures[0]->m_texture_descriptor.m_clear_value[0] = 1.0f;
		D3D11_instance::This->m_textures[0]->m_texture_descriptor.m_clear_value[1] = 0.0f;
		D3D11_instance::This->m_textures[0]->m_texture_descriptor.m_clear_value[2] = 0.0f;
	}
	else if (parameter == 201)
	{
		D3D11_instance::This->m_textures[0]->m_texture_descriptor.m_clear_value[0] = 0.0f;
		D3D11_instance::This->m_textures[0]->m_texture_descriptor.m_clear_value[1] = 1.0f;
		D3D11_instance::This->m_textures[0]->m_texture_descriptor.m_clear_value[2] = 0.0f;
	}
	else if (parameter == 202)
	{
		D3D11_instance::This->m_textures[0]->m_texture_descriptor.m_clear_value[0] = 0.0f;
		D3D11_instance::This->m_textures[0]->m_texture_descriptor.m_clear_value[1] = 0.0f;
		D3D11_instance::This->m_textures[0]->m_texture_descriptor.m_clear_value[2] = 1.0f;
	}
	else if (parameter == 202)
	{
	}
}


void SetLineWidth(uint32_t job_, float width)
{
}


bool GetTextureContent(uint32_t texture_, uint32_t level, uint32_t layer, uint32_t face, NGL_format format, NGL_resource_state state, uint32_t &width, uint32_t &height, std::vector<uint8_t> &data)
{
	if ((format != NGL_R8_G8_B8_UNORM) && (format != NGL_R8_G8_B8_A8_UNORM))
	{
		_logf("DX11 - GetTextureContent: Requested format is not NGL_R8_G8_B8_UNORM or NGL_R8_G8_B8_A8_UNORM !");
		return false;
	}

	bool release_src_texture = false;
	ID3D11Texture2D *src_texture = nullptr;
	ID3D11Resource *src_resource = nullptr;
	if (texture_)
	{
		D3D11_texture *texture = D3D11_instance::This->m_textures[texture_];
		src_texture = texture->m_tex;
	}
	else
	{
		D3D11_instance::This->m_d3dRenderTargetView->GetResource(&src_resource);

		D3D11_RESOURCE_DIMENSION src_dim = D3D11_RESOURCE_DIMENSION_UNKNOWN;
		src_resource->GetType(&src_dim);
		if (src_dim != D3D11_RESOURCE_DIMENSION_TEXTURE2D)
		{
			_logf("DX11 - GetTextureContent: Source texture is not texture 2D!");
			SafeRelease(src_resource);
			return false;
		}

		src_resource->QueryInterface(__uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&src_texture));
		release_src_texture = true;
	}

	assert(src_texture);


	D3D11_TEXTURE2D_DESC src_desc;
	src_texture->GetDesc(&src_desc);
	if (src_desc.Format != DXGI_FORMAT_R8G8B8A8_UNORM && src_desc.Format != DXGI_FORMAT_B8G8R8A8_UNORM)
	{
		_logf("DX11 - GetTextureContent: Source texture is not DXGI_FORMAT_R8G8B8A8_UNORM or DXGI_FORMAT_B8G8R8A8_UNORM!");
		return false;
	}

	// Create a temporary staging texture with the same settings
	D3D11_TEXTURE2D_DESC dst_desc;
	memcpy(&dst_desc, &src_desc, sizeof(D3D11_TEXTURE2D_DESC));
	dst_desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
	dst_desc.Usage = D3D11_USAGE_STAGING;
	dst_desc.BindFlags = 0;

	ID3D11Texture2D *dst_texture = nullptr;
	HRESULT result = D3D11_instance::This->m_d3dDevice->CreateTexture2D(&dst_desc, nullptr, &dst_texture);
	SetDebugObjectName(dst_texture, "staging_texture");
	if (FAILED(result))
	{
		_logf("DX11 - GetTextureContent: Can not create staging texture!");
		return false;
	}

	D3D11_instance::This->m_d3dContext->CopyResource(dst_texture, src_texture);

	D3D11_MAPPED_SUBRESOURCE subresource;
	result = D3D11_instance::This->m_d3dContext->Map(dst_texture, 0, D3D11_MAP::D3D11_MAP_READ, 0, &subresource);
	if (FAILED(result) || subresource.pData == nullptr)
	{
		_logf("DX11 - GetTextureContent: Can not map staging texture!");
		return false;
	}

	const uint32_t dst_pixel_size = (format == NGL_R8_G8_B8_A8_UNORM)?4:3;
	const uint32_t src_pixel_size = 4;
	bool src_rgba = src_desc.Format == DXGI_FORMAT_R8G8B8A8_UNORM;

	width = src_desc.Width;
	height = src_desc.Height;
	data.resize(width * height * dst_pixel_size);

	uint8_t *src_ptr = (uint8_t*)subresource.pData;

	for (uint32_t y = 0; y < height; y++)
	{
		for (uint32_t x = 0; x < width; x++)
		{
			uint32_t src_y = height - y - 1;

			uint32_t dst_index = (x + width * y) * dst_pixel_size;
			uint32_t src_index = x * src_pixel_size + subresource.RowPitch * src_y;

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

	D3D11_instance::This->m_d3dContext->Unmap(dst_texture, 0);

	SafeRelease(dst_texture);
	if (release_src_texture)
	{
		SafeRelease(src_texture);
		SafeRelease(src_resource);
	}
	return true;
}


bool GetVertexBufferContent(uint32_t buffer_id, NGL_resource_state state, std::vector<uint8_t> &data)
{
	if (buffer_id >= uint32_t(D3D11_instance::This->m_vertex_buffers.size()))
	{
		_logf("DX11 - GetVertexBufferContent: Invalid buffer id: %s", buffer_id);
		return false;
	}

	D3D11_vertex_buffer *buffer = D3D11_instance::This->m_vertex_buffers[buffer_id];
	if (buffer->m_datasize == 0)
	{
		data.clear();
		return true;
	}

	ID3D11Buffer *src_buffer = buffer->m_structured_buffer ? buffer->m_structured_buffer : buffer->pVBuffer;
	if (src_buffer == nullptr)
	{
		_logf("DX11 - GetVertexBufferContent: Invalid buffer: %s", buffer_id);
		return false;
	}

	// Create the staging buffer
	ID3D11Buffer *staging_buffer = nullptr;
	D3D11_BUFFER_DESC staging_desc;

	ZeroMemory(&staging_desc, sizeof(staging_desc));
	staging_desc.Usage = D3D11_USAGE_STAGING;
	staging_desc.ByteWidth = buffer->m_datasize;
	staging_desc.BindFlags = 0;
	staging_desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;

	HRESULT result = D3D11_instance::This->m_d3dDevice->CreateBuffer(&staging_desc, nullptr, &staging_buffer);
	SetDebugObjectName(staging_buffer, "staging_buffer");
	if FAILED(result)
	{
		_logf("DX11 - GetVertexBufferContent: Can not create staging buffer!");
		return false;
	}

	// Copy to the staging buffer
	D3D11_instance::This->m_d3dContext->CopyResource(staging_buffer, src_buffer);

	// Map the staging buffer
	D3D11_MAPPED_SUBRESOURCE src_data;
	bool succeeded = SUCCEEDED(D3D11_instance::This->m_d3dContext->Map(staging_buffer, 0, D3D11_MAP_READ, 0, &src_data));

	if (succeeded)
	{
		// Copy the data
		data.resize(buffer->m_datasize);
		memcpy(data.data(), src_data.pData, buffer->m_datasize);

		D3D11_instance::This->m_d3dContext->Unmap(staging_buffer, 0);
	}
	else
	{
		_logf("DX11 - GetVertexBufferContent: Can not map staging buffer!");
	}

	if (staging_buffer)
	{

		staging_buffer->Release();
	}

	return succeeded;
}


bool ResizeTextures(uint32_t num_textures, uint32_t *textures, uint32_t size[3])
{
	std::set<D3D11_job*> affected_jobs;

	for (uint32_t i = 0; i < num_textures; i++)
	{
		uint32_t texture_ = textures[i];

		if (!texture_ || texture_ >= D3D11_instance::This->m_textures.size())
		{
			_logf("OGL - ResizeTexture: Illegal texture id: %d!\n", texture_);
			continue;
		}
		D3D11_texture *texture = D3D11_instance::This->m_textures[texture_];
		if (texture->m_texture_descriptor.m_type == NGL_TEXTURE_2D)
		{
			NGL_texture_descriptor d = texture->m_texture_descriptor;

			d.m_size[0] = size[0];
			d.m_size[1] = size[1];
			d.m_size[2] = size[2];

			GenTexture(textures[i], d, 0);
		}
		else
		{
			//assert(0);
		}

		for (size_t j = 0; j < D3D11_instance::This->m_jobs.size(); j++)
		{
			D3D11_job *job = D3D11_instance::This->m_jobs[j];
			NGL_job_descriptor &descriptor = job->m_descriptor;

			for (size_t i = 0; i < descriptor.m_attachments.size(); i++)
			{
				const NGL_attachment_descriptor &atd = descriptor.m_attachments[i];
				if (atd.m_attachment.m_idx == texture_)
				{
					affected_jobs.insert(job);
				}
			}
		}
	}

	for (std::set<D3D11_job*>::iterator i = affected_jobs.begin(); i != affected_jobs.end(); i++)
	{
		(*i)->CreateSubpasses();
		CreateJobFBO(*i);
	}

	return true;
}

void Submit(uint32_t job_)
{
	// NOP
}


void Flush()
{
	D3D11_instance *dx = D3D11_instance::This;

	dx->m_d3dContext->Flush();
}


void Finish()
{
	D3D11_instance *dx = D3D11_instance::This;

	// Insert the fence to the command buffer
	dx->m_d3dContext->End(dx->m_finish_query);

	// Wait until finished
	while (dx->m_d3dContext->GetData(dx->m_finish_query, 0, 0, 0) != S_OK) {};
}


void DestroyContext()
{
	delete D3D11_instance::This;
	D3D11_instance::This = 0;
}

void Barrier(uint32_t cmd_buffer, std::vector<NGL_texture_subresource_transition> &texture_barriers, std::vector<NGL_buffer_transition> &buffer_barriers)
{
	//NOP
}
void BeginCommandBuffer(uint32_t command_buffer)
{
	//NOP
}
void EndCommandBuffer(uint32_t command_buffer)
{
	//NOP
}
void SubmitCommandBuffer(uint32_t command_buffer)
{
	//NOP
}

D3D11_instance *D3D11_instance::This = 0;


D3D11_instance::D3D11_instance()
{
	This = this;

	nglBarrier = Barrier;
	nglBeginCommandBuffer = BeginCommandBuffer;
	nglEndCommandBuffer = EndCommandBuffer;
	nglSubmitCommandBuffer = SubmitCommandBuffer;
	nglGenJob = GenJob;
	nglBegin = Begin;
	nglNextSubpass = NextSubpass;
	nglEnd = End;
	nglBlendState = BlendState;
	nglDepthState = DepthState;
	nglDraw = Draw;
	nglDrawInstanced = DrawInstanced;
	nglDrawIndirect = DrawIndirect;
	nglDispatch = Dispatch;
	nglDispatchIndirect = DispatchIndirect;
	nglGenTexture = GenTexture;
	nglGenVertexBuffer = GenVertexBuffer;
	nglGenIndexBuffer = GenIndexBuffer;
	nglViewportScissor = ViewportScissor;
	nglGetString = GetString;
	nglGetInteger = GetInteger;
	nglDeletePipelines = DeletePipelines;
	nglCustomAction = CustomAction;
	nglLineWidth = LineWidth;
	nglGetTextureContent = GetTextureContent;
	nglGetVertexBufferContent = GetVertexBufferContent;
	nglResizeTextures = ResizeTextures;
	nglFlush = Flush;
	nglFinish = Finish;
	nglDestroyContext = DestroyContext;

	// Device
	m_d3dDevice = nullptr;
	m_d3dContext = nullptr;
	m_d3dDebug = nullptr;
	m_d3dInfoQueue = nullptr;
	m_driverType = D3D_DRIVER_TYPE_UNKNOWN;
	m_major = 0;
	m_minor = 0;

	// Swap chain
	m_swapChain = nullptr;
	m_renderTargetTexture = nullptr;
	m_d3dRenderTargetView = nullptr;

	// Common resources
	m_shadow_sampler = nullptr;
	m_finish_query = nullptr;
}


D3D11_instance::~D3D11_instance()
{
	for (size_t i = 0; i < m_vertex_buffers.size(); i++)
	{
		delete m_vertex_buffers[i];
	}

	for (size_t i = 0; i < m_index_buffers.size(); i++)
	{
		delete m_index_buffers[i];
	}

	// onscreen rendertarget view released by testfw
	for (size_t i = 1; i < m_textures.size(); i++)
	{
		delete m_textures[i];
	}

	{
		m_textures[0]->m_rt_views.clear();
		delete m_textures[0];
	}

	for (size_t i = 0; i < m_jobs.size(); i++)
	{
		delete m_jobs[i];
	}
	m_jobs.clear();

	for (auto it = m_shader_cache.begin(); it != m_shader_cache.end(); it++)
	{
		SafeRelease(it->second);
	}

	for (auto it = m_samplers.begin(); it != m_samplers.end(); it++)
	{
		SafeRelease(it->second);
	}

	for (auto it = m_blend_states.begin(); it != m_blend_states.end(); it++)
	{
		SafeRelease(it->second);
	}

	for (auto it = m_depth_states.begin(); it != m_depth_states.end(); it++)
	{
		SafeRelease(it->second);
	}

	for (auto it = m_raster_states.begin(); it != m_raster_states.end(); it++)
	{
		SafeRelease(it->second);
	}

	// Common resources
	SafeRelease(m_shadow_sampler);
	SafeRelease(m_finish_query);

	// Swap chain
	// onscreen rendertarget view released by testfw
	//SafeRelease(m_d3dRenderTargetView);
	SafeRelease(m_renderTargetTexture);
	SafeRelease(m_swapChain);

	if (m_d3dDebug)
	{
		if (m_d3dContext)
		{
			// Deferred destruction: Release any bound resources
			// https://msdn.microsoft.com/en-us/library/windows/desktop/ff476425(v=vs.85).aspx
			m_d3dContext->ClearState();
			m_d3dContext->Flush();
		}

		m_d3dDebug->ReportLiveDeviceObjects(D3D11_RLDO_SUMMARY);
	}

	// Devices
	SafeRelease(m_d3dInfoQueue);
	SafeRelease(m_d3dDebug);
	//SafeRelease(m_d3dContext);
	//SafeRelease(m_d3dDevice);
}


ID3D11SamplerState* D3D11_instance::GetSamplerState(const D3D11_SAMPLER_DESC& desc)
{
	// Look up for existing one
	for (size_t i = 0; i < m_samplers.size(); i++)
	{
		std::pair<D3D11_SAMPLER_DESC, ID3D11SamplerState*> &other = m_samplers[i];
		if (!memcmp(&other.first, &desc, sizeof(D3D11_SAMPLER_DESC)))
		{
			return other.second;
		}
	}

	// Create new sampler state
	ID3D11SamplerState *sampler = nullptr;
	DX_THROW_IF_FAILED(D3D11_instance::This->m_d3dDevice->CreateSamplerState(&desc, &sampler));

	SetDebugObjectName(sampler, "sampler");

	m_samplers.push_back(std::pair<D3D11_SAMPLER_DESC, ID3D11SamplerState*>(desc, sampler));

	return sampler;
}


void D3D11_instance::getSwapChainTexture()
{
	if (m_driverType == D3D_DRIVER_TYPE_REFERENCE)
	{
		D3D11_TEXTURE2D_DESC backBufferDesc;
		backBufferDesc.Width = 1920;
		backBufferDesc.Height = 1080;
		backBufferDesc.MipLevels = 1;
		backBufferDesc.ArraySize = 1;
		backBufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
		backBufferDesc.SampleDesc.Count = 1;
		backBufferDesc.SampleDesc.Quality = 0;
		backBufferDesc.Usage = D3D11_USAGE_DEFAULT;
		backBufferDesc.BindFlags = D3D11_BIND_RENDER_TARGET;
		backBufferDesc.CPUAccessFlags = 0;
		backBufferDesc.MiscFlags = 0;

		DX_THROW_IF_FAILED(m_d3dDevice->CreateTexture2D(&backBufferDesc, NULL, &m_renderTargetTexture));
		SetDebugObjectName(m_renderTargetTexture, "default_rt_texture");
	}
	else
	{
		DXGI_FORMAT colorFormat = DXGI_FORMAT_R8G8B8A8_UNORM;

#if 0
		for (size_t i = 0; formats[i].format != DXGI_FORMAT_UNKNOWN; i++)
		{
			int red = 8;
			int green = 8;
			int blue = 8;

			if (formats[i].red == red &&
				formats[i].green == green &&
				formats[i].blue == blue)
			{
				colorFormat = formats[i].format;
				break;
			}
		}
#endif

		IDXGIDevice1* dxgiDevice;
		DX_THROW_IF_FAILED(m_d3dDevice->QueryInterface(IID_IDXGIDevice1, (void**)&dxgiDevice));

		IDXGIAdapter* dxgiAdapter;
		DX_THROW_IF_FAILED(dxgiDevice->GetAdapter(&dxgiAdapter));

		////UINT size = sizeof(m_driverType);
		////if (dxgiAdapter->GetPrivateData(GUID_DeviceType, &size, &m_driverType) != S_OK)
		////{
		////	m_driverType = D3D_DRIVER_TYPE_HARDWARE;
		////}

		// Create a descriptor for the swap chain.
		DXGI_SWAP_CHAIN_DESC1 swapChainDesc = { 0 };
		swapChainDesc.Format = colorFormat;           // this is the most common swapchain format
		swapChainDesc.Stereo = false;
		swapChainDesc.SampleDesc.Count = 1;
		swapChainDesc.SampleDesc.Quality = 0;
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.BufferCount = 2;
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL; //DXGI_SWAP_EFFECT_DISCARD;
		swapChainDesc.Flags = 0;

		// Once the desired swap chain description is configured, it must be created on the same adapter as our D3D Device
		// First, retrieve the underlying DXGI Device from the D3D Device (done)
		// Identify the physical adapter (GPU or card) this device is running on. (done)
		// And obtain the factory object that created it.
		IDXGIFactory2* dxgiFactory;
		DX_THROW_IF_FAILED(dxgiAdapter->GetParent(__uuidof(IDXGIFactory2), (void**)&dxgiFactory));

		swapChainDesc.Width = m_context_descriptor.m_display_width;                                     // use automatic sizing
		swapChainDesc.Height = m_context_descriptor.m_display_height;
		swapChainDesc.Scaling = DXGI_SCALING_STRETCH;// DXGI_SCALING_NONE;

		DXGI_SWAP_CHAIN_FULLSCREEN_DESC fullscreenDesc = { 0 };
		fullscreenDesc.RefreshRate.Numerator = 0; //use 60 for V-Sync
		fullscreenDesc.RefreshRate.Denominator = 1;
		fullscreenDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
		fullscreenDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_PROGRESSIVE;
		fullscreenDesc.Windowed = TRUE;

		// Create a swap chain for this window from the DXGI factory.
		DX_THROW_IF_FAILED(dxgiFactory->CreateSwapChainForHwnd(
			m_d3dDevice,
			hWnd,
			&swapChainDesc,
			&fullscreenDesc,
			nullptr,            // allow on all displays
			&m_swapChain
			));

		// Ensure that DXGI does not queue more than one frame at a time. This both reduces
		// latency and ensures that the application will only render after each VSync, minimizing
		// power consumption.
		DX_THROW_IF_FAILED(dxgiDevice->SetMaximumFrameLatency(1));

		{
			DXGI_ADAPTER_DESC d;

			DX_THROW_IF_FAILED(dxgiAdapter->GetDesc(&d));

			std::wstring ws(d.Description);
			std::string s;
			std::transform(ws.begin(), ws.end(), std::back_inserter(s), [](wchar_t c) {
				return (char)c;
			});

			m_propertiess[NGL_VENDOR] = s;

			_logf("D3D11 Adapter: %s", s.c_str());
		}

		dxgiDevice->Release();
		dxgiAdapter->Release();
		dxgiFactory->Release();

		DX_THROW_IF_FAILED(m_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&m_renderTargetTexture));
	}
	{
		D3D11_TEXTURE2D_DESC t2dd;

		m_renderTargetTexture->GetDesc(&t2dd);

		// Create a descriptor for the RenderTargetView.
		CD3D11_RENDER_TARGET_VIEW_DESC rtvd(D3D11_RTV_DIMENSION_TEXTURE2D);

		// Create a view interface on the rendertarget to use on bind.
		DX_THROW_IF_FAILED(m_d3dDevice->CreateRenderTargetView(
			m_renderTargetTexture,
			&rtvd,
			&m_d3dRenderTargetView));
		SetDebugObjectName(m_d3dRenderTargetView, "default_rtv");
	}
}


static std::string WcharToString(const wchar_t *wstr)
{
	const size_t buffer_size = 1024;
	char buffer[buffer_size];

	size_t ret = wcstombs(buffer, wstr, buffer_size);
	if (ret == buffer_size)
	{
		buffer[buffer_size - 1] = '\0';
	}

	return std::string(buffer);
}


bool D3D11_instance::Init()
{
	hWnd = (HWND)m_context_descriptor.m_user_data[1];
/*
	IDXGIFactory1 *dxgi_factory = nullptr;

 	HRESULT result = CreateDXGIFactory1(IID_PPV_ARGS(&dxgi_factory));
	if (FAILED(result))
	{
		_logf("Can not create DXGI factory!");
		return false;
	}

	UINT adapter_id = 0;

	IDXGIAdapter1 *enum_adapter = nullptr;
	while (dxgi_factory->EnumAdapters1(adapter_id, &enum_adapter) == S_OK)
	{
		DXGI_ADAPTER_DESC1 desc;
		HRESULT result = enum_adapter->GetDesc1(&desc);
		if (SUCCEEDED(result))
		{
			std::string desc_name = WcharToString(desc.Description);

			_logf("%d - %s", adapter_id, desc_name.c_str());
			enum_adapter->Release();
		}
		else
		{
			_logf("Can not query adapter descriptor #%d", adapter_id);

			enum_adapter->Release();

			continue;
		}

		adapter_id++;
	}

	dxgi_factory->Release();

	D3D_DRIVER_TYPE m_driverType = D3D_DRIVER_TYPE_HARDWARE;

	UINT creationFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;

#ifdef _DEBUG
	creationFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	// This array defines the set of DirectX hardware feature levels this app will support.
	// Note the ordering should be preserved.
	// Don't forget to declare your application's minimum required feature level in its
	// description.  All applications are assumed to support 9.1 unless otherwise stated.
	D3D_FEATURE_LEVEL featureLevels[] =
	{
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0
	};

	D3D_FEATURE_LEVEL featureLevel;

	// Create the DX11 API device object, and get a corresponding context.
	ID3D11Device* device;
	ID3D11DeviceContext* context;
	DX_THROW_IF_FAILED(D3D11CreateDevice(
		nullptr,                    // specify null to use the default adapter
		m_driverType,
		NULL,                    // leave as nullptr unless software device
		creationFlags,              // optionally set debug and Direct2D compatibility flags
		featureLevels,              // list of feature levels this app can support
		ARRAYSIZE(featureLevels),   // number of entries in above list
		D3D11_SDK_VERSION,          // always set this to D3D11_SDK_VERSION
		&device,                    // returns the Direct3D device created
		&featureLevel,            // returns feature level of device created
		&context                   // returns the device immediate context
	));

	switch (featureLevel)
	{
	case D3D_FEATURE_LEVEL_11_1:
		m_major = 11; m_minor = 1; break;
	case D3D_FEATURE_LEVEL_11_0:
		m_major = 11; m_minor = 0; break;
	case D3D_FEATURE_LEVEL_10_1:
		m_major = 10; m_minor = 1; break;
	case D3D_FEATURE_LEVEL_10_0:
		m_major = 10; m_minor = 0; break;
	case D3D_FEATURE_LEVEL_9_3:
		m_major = 9; m_minor = 3; break;
	case D3D_FEATURE_LEVEL_9_2:
		m_major = 9; m_minor = 2; break;
	case D3D_FEATURE_LEVEL_9_1:
		m_major = 9; m_minor = 1; break;
	default:
		m_major = 0; m_minor = 0; break;
	}

	DX_THROW_IF_FAILED(device->QueryInterface(IID_ID3D11Device1, (void**)&m_d3dDevice));
	DX_THROW_IF_FAILED(context->QueryInterface(IID_ID3D11DeviceContext1, (void**)&m_d3dContext));
	device->Release();
	context->Release();

	D3D11_FEATURE_DATA_SHADER_MIN_PRECISION_SUPPORT options;
	m_d3dDevice->CheckFeatureSupport(D3D11_FEATURE_SHADER_MIN_PRECISION_SUPPORT, &options, sizeof(D3D11_FEATURE_DATA_SHADER_MIN_PRECISION_SUPPORT));
	_logf("Pixel shader min precision: %d", options.PixelShaderMinPrecision);
	_logf("All other stage min precision: %d", options.AllOtherShaderStagesMinPrecision);

#ifdef _DEBUG
	m_d3dDebug = nullptr;
	HRESULT Result = m_d3dDevice->QueryInterface(__uuidof(ID3D11Debug), reinterpret_cast<void**>(&m_d3dDebug));
	//DebugDevice->ReportLiveDeviceObjects(D3D11_RLDO_SUMMARY);

	m_d3dInfoQueue = nullptr;
	if (m_d3dDebug->QueryInterface(__uuidof(ID3D11InfoQueue), (void**)&m_d3dInfoQueue) == S_OK)
	{
		//m_d3dInfoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_WARNING, true);
		m_d3dInfoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_CORRUPTION, true);
		m_d3dInfoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_ERROR, true);

		D3D11_MESSAGE_ID hide[] =
		{
			D3D11_MESSAGE_ID_SETPRIVATEDATA_CHANGINGPARAMS,
		};
		D3D11_INFO_QUEUE_FILTER filter;
		memset(&filter, 0, sizeof(filter));

		filter.DenyList.NumIDs = _countof(hide);
		filter.DenyList.pIDList = hide;
		m_d3dInfoQueue->AddStorageFilterEntries(&filter);
	}
#endif
	getSwapChainTexture();
	*/
	m_d3dDevice = (ID3D11Device1*)m_context_descriptor.m_user_data[0];
	m_d3dContext = (ID3D11DeviceContext1*)m_context_descriptor.m_user_data[1];
	m_d3dRenderTargetView = (ID3D11RenderTargetView*)m_context_descriptor.m_user_data[2];

	InitDevice();

	if (m_context_descriptor.m_enable_validation)
	{
		HRESULT result = m_d3dDevice->QueryInterface(__uuidof(ID3D11Debug), reinterpret_cast<void**>(&m_d3dDebug));
		if (FAILED(result))
		{
			_logf("DX11: Can not query ID3D11Debug interface!");
		}
		else
		{
			result = m_d3dDebug->QueryInterface(__uuidof(ID3D11InfoQueue), (void**)&m_d3dInfoQueue);
			if (FAILED(result))
			{
				_logf("DX11: Can not query ID3D11InfoQueue interface!");
			}
			else
			{
				m_d3dInfoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_WARNING, true);
				m_d3dInfoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_CORRUPTION, true);
				m_d3dInfoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_ERROR, true);

				D3D11_MESSAGE_ID hide[] =
				{
					D3D11_MESSAGE_ID_SETPRIVATEDATA_CHANGINGPARAMS,
				};
				D3D11_INFO_QUEUE_FILTER filter;
				memset(&filter, 0, sizeof(filter));

				filter.DenyList.NumIDs = _countof(hide);
				filter.DenyList.pIDList = hide;
				m_d3dInfoQueue->AddStorageFilterEntries(&filter);

				_logf("DX11: Validation layer is enabled!");
			}
		}
	}

	ID3D11Resource *render_target_resource = nullptr;
	m_d3dRenderTargetView->GetResource(&render_target_resource);

	render_target_resource->QueryInterface(__uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&m_renderTargetTexture));

	// Create shadow sampler
	{
		D3D11_SAMPLER_DESC desc;
		ZeroMemory(&desc, sizeof(D3D11_SAMPLER_DESC));
		desc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
		desc.ComparisonFunc = D3D11_COMPARISON_LESS_EQUAL;
		desc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
		desc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
		desc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
		desc.MinLOD = 0.0f;
		desc.MaxLOD = D3D11_FLOAT32_MAX;
		DX_THROW_IF_FAILED(m_d3dDevice->CreateSamplerState(&desc, &m_shadow_sampler));
		SetDebugObjectName(m_shadow_sampler, "shadow sampler");
	}

	// Create query for executing nglFinish
	{
		CD3D11_QUERY_DESC query_desc(D3D11_QUERY_EVENT);
		m_d3dDevice->CreateQuery(&query_desc, &m_finish_query);
	}

	// Null resources
	{
		m_null_resources.resize(D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT, 0);
	}

	{
		// First texture with zero index means default texture
		D3D11_texture *texture = new D3D11_texture();
		texture->m_tex = m_renderTargetTexture;
		texture->m_rt_views.push_back(m_d3dRenderTargetView);
		m_renderTargetTexture->GetDesc(&texture->m_descriptor);

		texture->m_texture_descriptor.m_name = "screen_texture";
		texture->m_texture_descriptor.m_size[0] = texture->m_descriptor.Width;
		texture->m_texture_descriptor.m_size[1] = texture->m_descriptor.Height;
		texture->m_texture_descriptor.m_is_renderable = true;
		texture->m_is_color = true;
		m_textures.push_back(texture);
	}

	m_propertiess[NGL_RENDERER] = "Direct3D";
	{
		char tmp[512];
		sprintf(tmp, "%d.%d", m_major, m_minor);

		m_propertiess[NGL_VERSION] = tmp;
	}

	m_propertiess[NGL_SWAPCHAIN_COLOR_FORMAT] = "unorm";
	m_propertiess[NGL_SWAPCHAIN_DEPTH_FORMAT] = "";


	for (size_t i = 0; i < NGL_NUM_PROPERTIES; i++)
	{
		m_propertiesi[i] = 0;
	}
	m_propertiesi[NGL_NEED_SWAPBUFFERS] = 1;
	m_propertiesi[NGL_API] = NGL_DIRECT3D_11;
	m_propertiesi[NGL_MAJOR_VERSION] = 11;
	m_propertiesi[NGL_MINOR_VERSION] = 0;
	m_propertiesi[NGL_RASTERIZATION_CONTROL_MODE] = NGL_ORIGIN_UPPER_LEFT_AND_NDC_FLIP;
	m_propertiesi[NGL_DEPTH_MODE] = NGL_ZERO_TO_ONE;
	m_propertiesi[NGL_TEXTURE_MAX_ANISOTROPY] = D3D11_REQ_MAXANISOTROPY;
	m_propertiesi[NGL_TEXTURE_MAX_SIZE_2D] = D3D11_REQ_TEXTURE2D_U_OR_V_DIMENSION;
	m_propertiesi[NGL_TEXTURE_MAX_SIZE_CUBE] = D3D11_REQ_TEXTURECUBE_DIMENSION;
	m_propertiesi[NGL_TEXTURE_COMPRESSION_DXT1] = 1;
	m_propertiesi[NGL_TEXTURE_COMPRESSION_DXT5] = 1;
	m_propertiesi[NGL_FLOATING_POINT_RENDERTARGET] = 1;
	m_propertiesi[NGL_TESSELLATION] = 1;

	m_propertiesi[NGL_MAX_COMPUTE_WORK_GROUP_SIZE_X] = D3D11_CS_THREAD_GROUP_MAX_X;
	m_propertiesi[NGL_MAX_COMPUTE_WORK_GROUP_SIZE_Y] = D3D11_CS_THREAD_GROUP_MAX_Y;
	m_propertiesi[NGL_MAX_COMPUTE_WORK_GROUP_SIZE_Z] = D3D11_CS_THREAD_GROUP_MAX_Z;

	m_propertiesi[NGL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS] = D3D11_CS_THREAD_GROUP_MAX_THREADS_PER_GROUP;

	// m_propertiesi[NGL_MAX_COMPUTE_SHARED_MEMORY_SIZE] = D3D11_CS_THREAD_LOCAL_TEMP_REGISTER_POOL; // it' ok?
	m_propertiesi[NGL_MAX_COMPUTE_SHARED_MEMORY_SIZE] = INT_MAX;

	m_propertiesi[NGL_D16_LINEAR_SHADOW_FILTER] = 1;
	m_propertiesi[NGL_D24_LINEAR_SHADOW_FILTER] = 1;

	m_propertiesi[NGL_SWAPCHAIN_COLOR_FORMAT] = 8888;
	m_propertiesi[NGL_SWAPCHAIN_DEPTH_FORMAT] = 0;

	return true;
}


void D3D11_instance::InitDevice()
{
	IDXGIDevice1* dxgiDevice;
	DX_THROW_IF_FAILED(m_d3dDevice->QueryInterface(IID_IDXGIDevice1, (void**)&dxgiDevice));

	IDXGIAdapter* dxgiAdapter;
	DX_THROW_IF_FAILED(dxgiDevice->GetAdapter(&dxgiAdapter));

	IDXGIFactory2* dxgiFactory;
	DX_THROW_IF_FAILED(dxgiAdapter->GetParent(__uuidof(IDXGIFactory2), (void**)&dxgiFactory));

	_logf("DX11: Adapters:");
	UINT adapter_id = 0;
	IDXGIAdapter1 *enum_adapter = nullptr;
	while (SUCCEEDED(dxgiFactory->EnumAdapters1(adapter_id, &enum_adapter)))
	{
		DXGI_ADAPTER_DESC1 desc;
		if (SUCCEEDED(enum_adapter->GetDesc1(&desc)))
		{
			std::string name = WcharToString(desc.Description);
			_logf("DX11: %d - %s", adapter_id, name.c_str());
		}
		else
		{
			_logf("DX11: Can not query adapter %d", adapter_id);
		}

		enum_adapter->Release();
		adapter_id++;
	}

	DXGI_ADAPTER_DESC desc;
	if SUCCEEDED(dxgiAdapter->GetDesc(&desc))
	{
		std::string name = WcharToString(desc.Description);
		_logf("DX11: Selected: %s", name.c_str());
	}
	else
	{
		_logf("DX11: Can not query the selected adapter!");
	}

	dxgiDevice->Release();
	dxgiAdapter->Release();
	dxgiFactory->Release();
}

}


void nglCreateContextD3D11(NGL_context_descriptor& descriptor)
{
	new D3D11::D3D11_instance;

	D3D11::D3D11_instance::This->m_context_descriptor = descriptor;

	D3D11::D3D11_instance::This->Init();
}


HRESULT enumInputLayout(_shader_reflection &r, ID3D11Device * d3dDevice, ID3DBlob * VSBlob)
{
	HRESULT hr = S_OK;

	ID3D11ShaderReflection *vertReflect;
	D3DReflect(VSBlob->GetBufferPointer(), VSBlob->GetBufferSize(), IID_ID3D11ShaderReflection, (void**)&vertReflect);

	D3D11_SHADER_DESC descVertex;
	vertReflect->GetDesc(&descVertex);

	uint32_t byteOffset = 0;
	D3D11_SIGNATURE_PARAMETER_DESC input_desc;
	for (uint32_t i = 0; i <descVertex.InputParameters; i++)
	{
		vertReflect->GetInputParameterDesc(i, &input_desc);

		if (input_desc.SystemValueType != D3D_NAME_UNDEFINED)
		{
			// Skip system values, like vertex id
			continue;
		}

		_shader_reflection::Block a;

		a.name = input_desc.SemanticName;
		a.stage = 0;
		a.size = 1;
		a.type = 0;
		a.binding_or_offset_or_location = byteOffset;
		if (input_desc.Mask == 1)
		{
			if (input_desc.ComponentType == D3D_REGISTER_COMPONENT_UINT32)
			{
				_logf("DX11 - Unsupported vertex attrib: %s", a.name.c_str());
				assert(0);
			}
			else if (input_desc.ComponentType == D3D_REGISTER_COMPONENT_SINT32)
			{
				_logf("DX11 - Unsupported vertex attrib: %s", a.name.c_str());
				assert(0);
			}
			else if (input_desc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32)
			{
				a.format = 1;
			}
			byteOffset += 4;
		}
		else if (input_desc.Mask <= 3)
		{
			if (input_desc.ComponentType == D3D_REGISTER_COMPONENT_UINT32)
			{
				_logf("DX11 - Unsupported vertex attrib: %s", a.name.c_str());
				assert(0);
			}
			else if (input_desc.ComponentType == D3D_REGISTER_COMPONENT_SINT32)
			{
				_logf("DX11 - Unsupported vertex attrib: %s", a.name.c_str());
				assert(0);
			}
			else if (input_desc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32)
			{
				a.format = 2;
			}
			byteOffset += 8;
		}
		else if (input_desc.Mask <= 7)
		{
			if (input_desc.ComponentType == D3D_REGISTER_COMPONENT_UINT32)
			{
				_logf("DX11 - Unsupported vertex attrib: %s", a.name.c_str());
				assert(0);
			}
			else if (input_desc.ComponentType == D3D_REGISTER_COMPONENT_SINT32)
			{
				_logf("DX11 - Unsupported vertex attrib: %s", a.name.c_str());
				assert(0);
			}
			else if (input_desc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32)
			{
				a.format = 3;
			}
			byteOffset += 12;
		}
		else if (input_desc.Mask <= 15)
		{
			if (input_desc.ComponentType == D3D_REGISTER_COMPONENT_UINT32)
			{
				_logf("DX11 - Unsupported vertex attrib: %s", a.name.c_str());
				assert(0);
			}
			else if (input_desc.ComponentType == D3D_REGISTER_COMPONENT_SINT32)
			{
				_logf("DX11 - Unsupported vertex attrib: %s", a.name.c_str());
				assert(0);
			}
			else if (input_desc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32)
			{
				a.format = 4;
			}
			byteOffset += 16;
		}


		r.attributes.push_back(a);
	}

	return S_OK;
}


HRESULT enumConstantBuffers(_shader_reflection &r, ID3D11Device * d3dDevice, ID3DBlob * blob, NGL_shader_type shader_type)
{
	int shader_types[6] =
	{
		0,
		4,
		3,
		1,
		2,
		5
	};

	int stage = shader_types[shader_type];

	ID3D11ShaderReflection *pReflector = NULL;
	D3DReflect(blob->GetBufferPointer(), blob->GetBufferSize(), IID_ID3D11ShaderReflection, (void**)&pReflector);

	D3D11_SHADER_DESC desc;
	pReflector->GetDesc(&desc);

	for (uint32_t i = 0; i < desc.ConstantBuffers; i++)
	{
		ID3D11ShaderReflectionConstantBuffer * pConstBuffer = pReflector->GetConstantBufferByIndex(i);

		D3D11_SHADER_BUFFER_DESC shaderBuffer;
		pConstBuffer->GetDesc(&shaderBuffer);

		D3D11_SHADER_INPUT_BIND_DESC bindingDesc;
		pReflector->GetResourceBindingDescByName(shaderBuffer.Name, &bindingDesc);

		if (bindingDesc.Type != D3D_SIT_CBUFFER)
		{
			// Here we get the UAVs too
			continue;
		}

		_shader_reflection::Block ubo;

		ubo.name = shaderBuffer.Name;
		ubo.stage = stage;
		ubo.binding_or_offset_or_location = bindingDesc.BindPoint;
		ubo.size = shaderBuffer.Size;
		ubo.format = 1000; // ubo
		ubo.type = 10;

		// Skip the default constant buffers
		// https://msdn.microsoft.com/en-us/library/windows/desktop/bb509581(v=vs.85).aspx
		if (ubo.name == "$Globals" || ubo.name == "$Param")
		{
			continue;
		}

		for (uint32_t j = 0; j < shaderBuffer.Variables; j++)
		{
			ID3D11ShaderReflectionVariable * pVariable = pConstBuffer->GetVariableByIndex(j);
			D3D11_SHADER_VARIABLE_DESC varDesc;
			pVariable->GetDesc(&varDesc);

			ID3D11ShaderReflectionType * pType = pVariable->GetType();
			D3D11_SHADER_TYPE_DESC varType;
			pType->GetDesc(&varType);

			_shader_reflection::Block b;

			b.name = varDesc.Name;
			b.stage = stage;
			b.binding_or_offset_or_location = varDesc.StartOffset;
			b.size = varType.Elements ? varType.Elements : 1;
			b.format = 0;
			b.type = 11;

			if (b.name[0] == '$')
			{
				assert(0);
			}

			if (varType.Type == D3D_SVT_FLOAT || varType.Type == D3D_SVT_INT || varType.Type == D3D_SVT_UINT)
			{
				if (varType.Rows == 1 && varType.Columns < 5)
				{
					b.format = varType.Columns;
				}
				else if (varType.Rows == 4 && varType.Columns == 4)
				{
					b.format = 5;
				}
			}

			if (b.format > 0)
			{
				ubo.blocks.push_back(b);
			}
			else
			{
				_logf("DX11 - Unsupported shader variable type: %s", varDesc.Name);
				assert(false);
			}
		}


		r.uniforms.push_back(ubo);
	}

	for (uint32_t i = 0; i < desc.BoundResources; i++)
	{
		D3D11_SHADER_INPUT_BIND_DESC inputBindDesc;
		pReflector->GetResourceBindingDesc(i, &inputBindDesc);

		if (inputBindDesc.Type == D3D_SIT_TEXTURE)
		{
			_shader_reflection::Block b;

			b.name = inputBindDesc.Name;
			b.stage = stage;
			b.binding_or_offset_or_location = inputBindDesc.BindPoint;
			b.size = 1;

			std::string sampler_name = b.name + "__ksl_sampler__";

			D3D11_SHADER_INPUT_BIND_DESC sampler_desc;

			HRESULT result = pReflector->GetResourceBindingDescByName(sampler_name.c_str(), &sampler_desc);
			if (FAILED(result))
			{
				//_logf("Reflection error: can not find sampler %s", sampler_name.c_str());

				b.format = 2002; // Subpass input (texture without sampler)
			}
			else
			{
				if (inputBindDesc.BindPoint != sampler_desc.BindPoint)
				{
					_logf("Reflection error: %s texture and its sampler have different binding points", inputBindDesc.Name);
				}

				if (sampler_desc.uFlags & D3D_SIF_COMPARISON_SAMPLER)
				{
					b.format = 2001; // Shadow sampler
				}
				else
				{
					b.format = 2000; // Sampler
				}
			}

			r.uniforms.push_back(b);
		}
		else if (inputBindDesc.Type == D3D_SIT_UAV_RWTYPED) // RWTexture
		{
			_shader_reflection::Block b;

			b.name = inputBindDesc.Name;
			b.stage = stage;
			b.binding_or_offset_or_location = inputBindDesc.BindPoint;
			b.size = 1;
			b.format = 2003; // Image
			b.type = 20;

			r.uniforms.push_back(b);
		}
		else if (inputBindDesc.Type == D3D_SIT_STRUCTURED)
		{
			_shader_reflection::Block b;

			b.name = inputBindDesc.Name;
			b.stage = stage;
			b.binding_or_offset_or_location = inputBindDesc.BindPoint;
			b.size = 1;
			b.format = 1002; // readonly ssbo
			b.type = 10;

			r.uniforms.push_back(b);
		}
		else if (inputBindDesc.Type == D3D_SIT_UAV_RWSTRUCTURED)
		{
			_shader_reflection::Block b;

			b.name = inputBindDesc.Name;
			b.stage = stage;
			b.binding_or_offset_or_location = inputBindDesc.BindPoint;
			b.size = 1;
			b.format = 1001; // ssbo
			b.type = 10;

			r.uniforms.push_back(b);
		}
		else if (inputBindDesc.Type == D3D_SIT_SAMPLER)
		{
			// We handle them differently
		}
		else if (inputBindDesc.Type == D3D_SIT_CBUFFER)
		{
			// We handle them differently
		}
		else
		{
			_logf("DX11 - Unsupported input binding type: %s", inputBindDesc.Name);
			assert(0);
		}
	}

	return S_OK;
}
