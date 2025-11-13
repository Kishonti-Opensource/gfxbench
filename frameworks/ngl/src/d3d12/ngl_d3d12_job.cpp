/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "ngl_d3d12_job.h"

#include "ngl_d3d12.h"
#include "ngl_d3d12_renderer.h"
#include "ngl_d3d12_memory.h"
#include "ngl_d3d12_action.h"
#include "ngl_d3d12_util.h"
#include <cassert>

// Local functions ///////////////////////////////////////////////////////////////////////////////////
void ResetRasterizerDesc(D3D12_RASTERIZER_DESC& rasterizer_desc)
{
	memset(&rasterizer_desc, 0, sizeof(rasterizer_desc));

	rasterizer_desc.FillMode = D3D12_FILL_MODE_SOLID;
	rasterizer_desc.CullMode = D3D12_CULL_MODE_BACK;
	rasterizer_desc.FrontCounterClockwise = FALSE;
	rasterizer_desc.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
	rasterizer_desc.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
	rasterizer_desc.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
	rasterizer_desc.DepthClipEnable = TRUE;
	rasterizer_desc.MultisampleEnable = FALSE;
	rasterizer_desc.AntialiasedLineEnable = FALSE;
	rasterizer_desc.ForcedSampleCount = 0;
	rasterizer_desc.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;
}


void ResetBlendStateDesc(D3D12_BLEND_DESC& blend_state_desc)
{
	memset(&blend_state_desc, 0, sizeof(blend_state_desc));

	blend_state_desc.AlphaToCoverageEnable = FALSE;
	blend_state_desc.IndependentBlendEnable = FALSE;

	D3D12_RENDER_TARGET_BLEND_DESC default_rt_desc;
	memset(&default_rt_desc, 0, sizeof(default_rt_desc));

	default_rt_desc.BlendEnable = FALSE;
	default_rt_desc.LogicOpEnable = FALSE;
	default_rt_desc.SrcBlend = D3D12_BLEND_ONE;
	default_rt_desc.DestBlend = D3D12_BLEND_ZERO;
	default_rt_desc.BlendOp = D3D12_BLEND_OP_ADD;
	default_rt_desc.SrcBlendAlpha = D3D12_BLEND_ONE;
	default_rt_desc.DestBlendAlpha = D3D12_BLEND_ZERO;
	default_rt_desc.BlendOpAlpha = D3D12_BLEND_OP_ADD;
	default_rt_desc.LogicOp = D3D12_LOGIC_OP_NOOP;
	default_rt_desc.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	for (int i = 0; i < 8; ++i)
	{
		memcpy(&blend_state_desc.RenderTarget[i], &default_rt_desc, sizeof(D3D12_RENDER_TARGET_BLEND_DESC));
	}
}


void ResetDepthStencilDesc(D3D12_DEPTH_STENCIL_DESC& depth_stencil_desc)
{
	memset(&depth_stencil_desc, 0, sizeof(depth_stencil_desc));

	depth_stencil_desc.DepthEnable = TRUE;
	depth_stencil_desc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	depth_stencil_desc.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
	depth_stencil_desc.StencilEnable = FALSE;
	depth_stencil_desc.StencilReadMask = D3D12_DEFAULT_STENCIL_READ_MASK;
	depth_stencil_desc.StencilWriteMask = D3D12_DEFAULT_STENCIL_WRITE_MASK;

	D3D12_DEPTH_STENCILOP_DESC default_stencil_op_desc;
	memset(&default_stencil_op_desc, 0, sizeof(default_stencil_op_desc));

	default_stencil_op_desc.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	default_stencil_op_desc.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
	default_stencil_op_desc.StencilPassOp = D3D12_STENCIL_OP_KEEP;
	default_stencil_op_desc.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;

	depth_stencil_desc.FrontFace = default_stencil_op_desc;
	depth_stencil_desc.BackFace = default_stencil_op_desc;
}


// D3D12_job ////////////////////////////////////////////////////////////////////////////////////

D3D12_job::D3D12_job(D3D12_backend* backend, bool is_compute)
{
	m_descriptor = NGL_job_descriptor();

	m_is_compute = is_compute;
	m_backend = backend;
	m_useRenderpass = false;
#if defined(USE_RENDER_PASS)
	m_useRenderpass = m_backend->m_optimized_renderpasses_supported;
#endif
	//m_command_context = m_backend->m_direct_queue->CreateCommandContext(true);
	m_has_previous_state = false;
	m_is_recording = false;

	m_command_context = nullptr;

	m_current_subpass = 0;

	m_last_renderer = nullptr;
	m_last_pso = nullptr;
	m_renderer_has_changed = false;
}


D3D12_job::~D3D12_job()
{
	//delete m_command_context;
}


void D3D12_job::Init(NGL_job_descriptor& descriptor)
{
	assert(m_is_compute == descriptor.m_is_compute);
	m_descriptor = descriptor;

	//m_command_context->SetName(descriptor.m_subpasses[0].m_name);
}

void D3D12_job::FindMidpassDroppedHandles()
{
	// Create lists for transitions from one pass to next. 3 passes, 2 transitions.
	m_midpass_dropped_handles[0].clear();
	m_midpass_dropped_handles[1].clear();
	// Create a list of handles of each pass
	std::vector<SIZE_T> rtv_handles[3];
	std::vector<SIZE_T> srv_handles[3];
	// Scan all sub passes of this job.
	for (int current_subpass = 0; current_subpass < m_pass_size; current_subpass++) {
		NGL_subpass& sp = m_descriptor.m_subpasses[current_subpass];
		D3D12_resource_manager* res_mgr = m_backend->m_resource_mgr;
		// Scan all attachments of this sub pass.
		for (size_t i = 0; i < m_descriptor.m_attachments.size(); i++) {
			NGL_attachment_descriptor& color_desc = m_descriptor.m_attachments[i];
			uint32_t texture_id = color_desc.m_attachment.m_idx;
			D3D12_texture* color_texture = res_mgr->m_textures[texture_id];
			// Render targets
			if (sp.m_usages[i] == NGL_COLOR_ATTACHMENT) {
				uint32_t subresource_index = color_texture->GetSubresourceIndex(color_desc.m_attachment.m_level, color_desc.m_attachment.m_layer, color_desc.m_attachment.m_face, 0);
				rtv_handles[current_subpass].push_back(res_mgr->m_color_attachment_heap.GetCPUHandle(color_texture->m_rtv_offsets[subresource_index]).ptr);
			}
			// Shader resources
			else if (sp.m_usages[i] == NGL_COLOR_ATTACHMENT_AND_INPUT_ATTACHMENT_AND_SHADER_RESOURCE)
			{
				uint32_t subresource_index = color_texture->GetSubresourceIndex(color_desc.m_attachment.m_level, color_desc.m_attachment.m_layer, color_desc.m_attachment.m_face, 0);
				srv_handles[current_subpass].push_back(res_mgr->m_color_attachment_heap.GetCPUHandle(color_texture->m_rtv_offsets[subresource_index]).ptr);
			}
		}
	}
	// Create a list of handles that are not bound to next passes and must be potentially saved mid multi pass.
	// Iterate over two pass begins. First one is not needed.
	for (int current_subpass = 1; current_subpass < m_pass_size; current_subpass++) {
		auto& rtv_handles_prev = rtv_handles[current_subpass - 1];
		auto& rtv_handles_current = rtv_handles[current_subpass];
		auto& srv_handles_current = srv_handles[current_subpass];
		// Loop over render targets of previous stage.
		for (int index = 0; index < rtv_handles_prev.size(); index++) {
			// Try to locate previous render target handler from next render targets.
			if (std::find(rtv_handles_current.begin(), rtv_handles_current.end(), rtv_handles_prev[index]) == rtv_handles_current.end()) {
				// If we did not find it, try the shader resources also.
				if (std::find(srv_handles_current.begin(), srv_handles_current.end(), rtv_handles_prev[index]) == srv_handles_current.end()) {
					// If handles was not present in either of the lists, this render target gets dropped.
					m_midpass_dropped_handles[current_subpass - 1].push_back(rtv_handles_prev[index]);
				}
			}
		}
	}
	return;
}

void D3D12_job::Begin(D3D12_command_context* command_context)
{
	m_command_context = command_context;
	assert(m_command_context->m_is_recordable);

	//m_command_context->Reset();
	//m_resource_states.resize(m_backend->m_resource_mgr->m_tracked_resources.size());
	m_is_recording = true;
	m_has_previous_state = false;

	m_last_renderer = nullptr;
	m_last_pso = nullptr;
	m_renderer_has_changed = false;

	// Multi pass related helper variables for the job.
	m_is_singlepass = m_descriptor.m_subpasses.size() <= 1 ? true : false;
	m_pass_size = static_cast<int>(m_descriptor.m_subpasses.size());

	if (m_useRenderpass)
	{
		if (!m_is_compute && !m_is_singlepass)
		{
			FindMidpassDroppedHandles();
		}
	}

}


void D3D12_job::End()
{
	if (m_useRenderpass)
	{
		if (!m_is_compute)
		{
			ID3D12GraphicsCommandList* cmd_list = m_command_context->m_command_list;

			ID3D12GraphicsCommandList4* cmd_list4;
			cmd_list->QueryInterface(IID_PPV_ARGS(&cmd_list4));
			cmd_list4->Release();

			if (!m_is_singlepass)
				cmd_list4->EndRenderPass();
		}
	}

	//m_command_context->Close();
	m_is_recording = false;

}


void D3D12_job::FillConstantBuffers(D3D12_renderer* renderer, const void* parameters[], std::vector<D3D12_memory_allocation>& mem_allocs, D3D12_uniform_group uniform_group)
{
	assert(mem_allocs.size() == NGL_NUM_SHADER_TYPES);
	NGL_shader_uniform_group ngl_group = GetUsedUniformGroup(uniform_group);

	D3D12_memory_manager* mem_mgr = m_command_context->m_memory_mgr;
	D3D12_root_signature& rs = renderer->m_root_signature;

	for (uint32_t shader_type = 0; shader_type < NGL_NUM_SHADER_TYPES; shader_type++)
	{
		D3D12_root_signature_shader_stage& rs_shader = rs.m_groups[uniform_group].m_stages[shader_type];

		if (rs_shader.m_cb_size > 0)
		{
			if (!rs_shader.m_cb_bind_as_root_constants)
			{
				mem_allocs[shader_type] = mem_mgr->Allocate(rs_shader.m_cb_size);

#ifdef NGL_DX12_DEBUG_CONSTANT_BUFFER_UPLOAD
				printf("\n");
				printf("Allocated %d bytes (%d bytes used)\n", (int)mem_allocs[shader_type].m_size, rs_shader.m_cb_size);
#endif
			}
		}
	}

	for (size_t i = 0; i < renderer->m_used_uniforms[ngl_group].size(); i++)
	{
		NGL_used_uniform& uu = renderer->m_used_uniforms[ngl_group][i];

		if ((uu.m_binding_type >> NGL_DX12_NUM_BINDING_TYPE_FLAGS) == D3D_SIT_CBUFFER)
		{
			assert(uu.m_application_location >= 0 && parameters[uu.m_application_location] != nullptr);

			const void* data_src = parameters[uu.m_application_location];
			uint32_t variable_size = uu.m_uniform.m_size;

			for (uint32_t shader_type = NGL_VERTEX_SHADER; shader_type < NGL_NUM_SHADER_TYPES; shader_type++)
			{
				int32_t data_dst_offset = uu.m_shader_location[shader_type];

				if (data_dst_offset > -1)
				{
					D3D12_root_signature_shader_stage& rs_shader = rs.m_groups[uniform_group].m_stages[shader_type];

					uint8_t* data_dst_base = nullptr;
					if (rs_shader.m_cb_bind_as_root_constants)
					{
						data_dst_base = rs_shader.m_root_constant_cpu_address;
					}
					else
					{
						data_dst_base = mem_allocs[shader_type].m_cpu_address;
					}
					assert(data_dst_base != nullptr);

					memcpy(data_dst_base + data_dst_offset, data_src, variable_size);

#ifdef NGL_DX12_DEBUG_CONSTANT_BUFFER_UPLOAD
					printf("\n");
					printf("Variable %s at offset %d, size %d\n", uu.m_uniform.m_name.c_str(), data_dst_offset, variable_size);
					if (rs_shader.m_cb_bind_as_root_constants)
					{
						printf("  as Root Constant\n");
					}
					else
					{
						printf("  as CBV\n");
					}
					printf("App variable %s\n", renderer->m_application_uniforms[uu.m_application_location].m_name.c_str());
					uint32_t l = 0;
					for (l = 0; l < variable_size / 4; l++)
					{
						//printf("%02X ", ((uint8_t *)data_src)[l]);
						float* fp = (float*)data_src;

						printf("%f ", fp[l]);
						if ((l % 4) == 3)
						{
							printf("\n");
						}
					}
					if ((l % 4) != 3)
					{
						printf("\n");
					}
#endif
				}
			}
		}
	}

#ifdef NGL_DX12_DEBUG_CONSTANT_BUFFER_UPLOAD
	for (size_t i = 0; i < mem_allocs.size(); i++)
	{
		if (mem_allocs[i].m_cpu_address != nullptr)
		{
			printf("\n\n");
			printf("Mem alloc dump for stage %d\n", (int)i);
			for (size_t l = 0; l < mem_allocs[i].m_size; l += 16)
			{
				//printf("%02X ", (mem_allocs[i].m_cpu_address)[l]);
				float* fp = (float*)&(mem_allocs[i].m_cpu_address[l]);
				printf("%f %f %f %f\n", fp[0], fp[1], fp[2], fp[3]);
			}
		}
	}
#endif
}

void D3D12_job::FillDescriptorTables(D3D12_renderer* renderer, const void* parameters[], D3D12_uniform_group uniform_group)
{
	D3D12_resource_manager* res_mgr = m_backend->m_resource_mgr;
	D3D12_root_signature& rs = renderer->m_root_signature;
	NGL_shader_uniform_group ngl_group = GetUsedUniformGroup(uniform_group);

	// Collect descriptors for buffers, textures, samplers

	for (size_t k = 0; k < renderer->m_used_uniforms[ngl_group].size(); k++)
	{
		NGL_used_uniform& uu = renderer->m_used_uniforms[ngl_group][k];
		if (uu.m_application_location >= 0 && parameters[uu.m_application_location] != nullptr)
		{
			if (uu.m_uniform.m_format == NGL_TEXTURE)
			{
				bool is_texture_subresource = (uu.m_binding_type & NGL_DX12_BINDING_TYPE_FLAG_IS_SUBRESOURCE) > 0;
				NGL_texture_subresource texture_subresource(0);
				uint32_t texture_id = 0;
				uint32_t subresource_index = 0;
				UINT srv_index;

				if (is_texture_subresource)
				{
					texture_subresource = *(NGL_texture_subresource*)parameters[uu.m_application_location];
					texture_id = texture_subresource.m_idx;
				}
				else
				{
					texture_id = *(uint32_t*)parameters[uu.m_application_location];
				}

				if (texture_id >= res_mgr->m_textures.size())
				{
					_logf("DX12: Illegal texture id: %d for uniform: %s\n", (int32_t)texture_id, uu.m_uniform.m_name.c_str());
					assert(false);
				}

				D3D12_texture& texture = *res_mgr->m_textures[texture_id];
				D3D_SHADER_INPUT_TYPE input_type = (D3D_SHADER_INPUT_TYPE)(uu.m_binding_type >> NGL_DX12_NUM_BINDING_TYPE_FLAGS);

				if (is_texture_subresource)
				{
					subresource_index = texture.GetSubresourceIndex(
						texture_subresource.m_level, texture_subresource.m_layer, texture_subresource.m_face, 0);

					if (texture.m_num_subresources == 1)
					{
						assert(subresource_index == 0);
						srv_index = texture.m_srv_offset;
					}
					else
					{
						srv_index = texture.m_subresource_srv_offsets[subresource_index];
					}
				}
				else
				{
					srv_index = texture.m_srv_offset;
				}

				if (input_type == D3D_SIT_TEXTURE)
				{
					if (uu.m_shader_location[NGL_FRAGMENT_SHADER] > -1)
					{
						m_command_context->VerifyTextureState(texture, subresource_index, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, !is_texture_subresource);
						rs.SetViewIndex(uniform_group, NGL_FRAGMENT_SHADER, uu.m_shader_location, srv_index);
						assert(rs.m_groups[uniform_group].m_stages[NGL_FRAGMENT_SHADER].m_view_table.at(uu.m_shader_location[NGL_FRAGMENT_SHADER]).RangeType == D3D12_DESCRIPTOR_RANGE_TYPE_SRV);
					}
					else if (uu.m_shader_location[NGL_COMPUTE_SHADER] > -1)
					{
						m_command_context->VerifyTextureState(texture, subresource_index, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, !is_texture_subresource);
						rs.SetViewIndex(uniform_group, NGL_COMPUTE_SHADER, uu.m_shader_location, srv_index);
					}
					else
					{
						assert(false);
					}
				}
				else if (input_type == D3D_SIT_SAMPLER)
				{
					bool is_shadow_sampler = (uu.m_binding_type & NGL_DX12_BINDING_TYPE_FLAG_IS_SHADOW_SAPLER) > 0;

					if (uu.m_shader_location[NGL_FRAGMENT_SHADER] > -1)
					{
						rs.SetSamplerIndex(uniform_group, NGL_FRAGMENT_SHADER, uu.m_shader_location, texture.m_sampler_offsets[is_shadow_sampler ? 1 : 0]);
						assert(rs.m_groups[uniform_group].m_stages[NGL_FRAGMENT_SHADER].m_sampler_table.at(uu.m_shader_location[NGL_FRAGMENT_SHADER]).RangeType == D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER);
					}
					else if (uu.m_shader_location[NGL_COMPUTE_SHADER] > -1)
					{
						rs.SetSamplerIndex(uniform_group, NGL_COMPUTE_SHADER, uu.m_shader_location, texture.m_sampler_offsets[is_shadow_sampler ? 1 : 0]);
						assert(rs.m_groups[uniform_group].m_stages[NGL_COMPUTE_SHADER].m_sampler_table.at(uu.m_shader_location[NGL_COMPUTE_SHADER]).RangeType == D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER);
					}
				}
				else if (input_type == D3D_SIT_UAV_RWTYPED)
				{
					if (uu.m_shader_location[NGL_COMPUTE_SHADER] > -1)
					{
						m_command_context->VerifyTextureState(texture, subresource_index, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, !is_texture_subresource);
						rs.SetViewIndex(uniform_group, NGL_COMPUTE_SHADER, uu.m_shader_location, texture.m_uav_offset);
					}
					else
					{
						assert(false);
					}
				}
				else
				{
					assert(false);
				}
			}

			if (uu.m_uniform.m_format == NGL_BUFFER)
			{
				bool is_buffer_subresource = (uu.m_binding_type & NGL_DX12_BINDING_TYPE_FLAG_IS_SUBRESOURCE) > 0;
				NGL_buffer_subresource buffer_subresource(0);
				uint32_t buffer_id = 0;

				if (is_buffer_subresource)
				{
					buffer_subresource = *(NGL_buffer_subresource*)parameters[uu.m_application_location];
					buffer_id = buffer_subresource.m_buffer;
				}
				else
				{
					buffer_id = *(uint32_t*)parameters[uu.m_application_location];
				}

				if (buffer_id >= res_mgr->m_vertex_buffers.size())
				{
					_logf("DX12: Illegal buffer id: %d for uniform: %s\n", (int32_t)buffer_id, uu.m_uniform.m_name.c_str());
					assert(false);
				}

				D3D12_vertex_buffer& buffer = *res_mgr->m_vertex_buffers[buffer_id];
				assert(buffer.m_ngl_vertex_buffer.m_vertex_descriptor.m_unordered_access);
				D3D_SHADER_INPUT_TYPE input_type = (D3D_SHADER_INPUT_TYPE)(uu.m_binding_type >> NGL_DX12_NUM_BINDING_TYPE_FLAGS);

				if (input_type == D3D_SIT_STRUCTURED)
				{
					uint32_t srv_index = 0;

					if (is_buffer_subresource)
					{
						srv_index = buffer.GetSubresourceSRV(buffer_subresource);
					}
					else
					{
						srv_index = buffer.m_srv_index;
					}

					if (uu.m_shader_location[NGL_VERTEX_SHADER] > -1)
					{
						m_command_context->VerifyBufferState(buffer, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
						rs.SetViewIndex(uniform_group, NGL_VERTEX_SHADER, uu.m_shader_location, srv_index);
					}
					else if (uu.m_shader_location[NGL_FRAGMENT_SHADER] > -1)
					{
						m_command_context->VerifyBufferState(buffer, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
						rs.SetViewIndex(uniform_group, NGL_FRAGMENT_SHADER, uu.m_shader_location, srv_index);
					}
					else if (uu.m_shader_location[NGL_COMPUTE_SHADER] > -1)
					{
						m_command_context->VerifyBufferState(buffer, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
						rs.SetViewIndex(uniform_group, NGL_COMPUTE_SHADER, uu.m_shader_location, srv_index);
					}
					else
					{
						assert(false);
					}
				}
				else if (input_type == D3D_SIT_UAV_RWSTRUCTURED)
				{
					uint32_t uav_index = 0;

					if (is_buffer_subresource)
					{
						uav_index = buffer.GetSubresourceUAV(buffer_subresource);
					}
					else
					{
						uav_index = buffer.m_uav_index;
					}

					if (uu.m_shader_location[NGL_VERTEX_SHADER] > -1)
					{
						m_command_context->VerifyBufferState(buffer, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
						rs.SetViewIndex(uniform_group, NGL_VERTEX_SHADER, uu.m_shader_location, uav_index);
					}
					else if (uu.m_shader_location[NGL_FRAGMENT_SHADER] > -1)
					{
						m_command_context->VerifyBufferState(buffer, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
						rs.SetViewIndex(uniform_group, NGL_FRAGMENT_SHADER, uu.m_shader_location, uav_index);
					}
					else if (uu.m_shader_location[NGL_COMPUTE_SHADER] > -1)
					{
						m_command_context->VerifyBufferState(buffer, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
						rs.SetViewIndex(uniform_group, NGL_COMPUTE_SHADER, uu.m_shader_location, uav_index);
					}
					else
					{
						assert(false);
					}
				}
				else
				{
					assert(false);
				}
			}
		}
	}
}


void D3D12_job::FillBindHeaps(D3D12_renderer* renderer)
{
	D3D12_root_signature& rs = renderer->m_root_signature;

	// Copy descriptors to bind heap

	uint32_t space_needed = 0;
	for (uint32_t uniform_group = 0; uniform_group < NGL_DX12_NUM_UNIFORM_GROUPS; uniform_group++)
	{
		for (uint32_t shader_type = 0; shader_type < NGL_NUM_SHADER_TYPES; shader_type++)
		{
			D3D12_root_signature_shader_stage& rs_shader = rs.m_groups[uniform_group].m_stages[shader_type];
			space_needed += rs_shader.m_num_view_bind_heap_entries;
		}
	}

	for (uint32_t uniform_group = 0; uniform_group < NGL_DX12_NUM_UNIFORM_GROUPS; uniform_group++)
	{
		for (uint32_t shader_type = 0; shader_type < NGL_NUM_SHADER_TYPES; shader_type++)
		{
			D3D12_root_signature_shader_stage& rs_shader = rs.m_groups[uniform_group].m_stages[shader_type];

			if (rs_shader.m_num_view_bind_heap_entries > 0)
			{
				m_backend->m_resource_mgr->m_view_bind_heap.RequestRange(rs, rs_shader, m_command_context->m_view_copy_batch);
			}

			if (rs_shader.m_num_sampler_bind_heap_entries > 0)
			{
				m_backend->m_resource_mgr->m_sampler_bind_heap.RequestRange(rs, rs_shader, m_command_context->m_sampler_copy_batch);
			}
		}
	}
}


void D3D12_job::BindUniforms(D3D12_renderer* renderer, const void* parameters[])
{
	assert(m_is_recording);
	ID3D12GraphicsCommandList* cmd_list = m_command_context->m_command_list;
	D3D12_root_signature& rs = renderer->m_root_signature;

	// Fill CBVs

	std::vector<D3D12_memory_allocation> draw_mem_allocs(NGL_NUM_SHADER_TYPES);
	FillConstantBuffers(renderer, parameters, draw_mem_allocs, NGL_DX12_UNIFORM_GROUP_PER_DRAW);
	if (m_renderer_has_changed)
	{
		m_renderer_mem_allocs.clear();
		m_renderer_mem_allocs.resize(NGL_NUM_SHADER_TYPES);
		FillConstantBuffers(renderer, parameters, m_renderer_mem_allocs, NGL_DX12_UNIFORM_GROUP_PER_RENDERER_CHANGE);
	}

	// Fill descriptor tables

	FillDescriptorTables(renderer, parameters, NGL_DX12_UNIFORM_GROUP_PER_DRAW);
	if (m_renderer_has_changed)
	{
		FillDescriptorTables(renderer, parameters, NGL_DX12_UNIFORM_GROUP_PER_RENDERER_CHANGE);
	}

	FillBindHeaps(renderer);

	// Set root CBVs

	uint8_t* cpu_address;
	D3D12_GPU_VIRTUAL_ADDRESS gpu_address;

	for (uint32_t uniform_group = 0; uniform_group < NGL_DX12_NUM_UNIFORM_GROUPS; uniform_group++)
	{
		if (!m_renderer_has_changed && uniform_group == NGL_DX12_UNIFORM_GROUP_PER_RENDERER_CHANGE)
		{
			continue;
		}

		for (uint32_t shader_type = 0; shader_type < NGL_NUM_SHADER_TYPES; shader_type++)
		{
			D3D12_root_signature_shader_stage& rs_shader = rs.m_groups[uniform_group].m_stages[shader_type];

			if (rs_shader.m_cb_bind_as_root_constants)
			{
				cpu_address = rs_shader.m_root_constant_cpu_address;
				assert(cpu_address != nullptr);

				if (shader_type != NGL_COMPUTE_SHADER)
				{
					cmd_list->SetGraphicsRoot32BitConstants(rs_shader.m_cb_root_index, rs_shader.m_cb_num_constants, cpu_address, 0);
				}
				else
				{
					cmd_list->SetComputeRoot32BitConstants(rs_shader.m_cb_root_index, rs_shader.m_cb_num_constants, cpu_address, 0);
				}
			}
		}
	}

	for (uint32_t uniform_group = 0; uniform_group < NGL_DX12_NUM_UNIFORM_GROUPS; uniform_group++)
	{
		if (!m_renderer_has_changed && uniform_group == NGL_DX12_UNIFORM_GROUP_PER_RENDERER_CHANGE)
		{
			continue;
		}

		for (uint32_t shader_type = 0; shader_type < NGL_NUM_SHADER_TYPES; shader_type++)
		{
			D3D12_root_signature_shader_stage& rs_shader = rs.m_groups[uniform_group].m_stages[shader_type];

			if (rs_shader.m_cb_bind_as_root_constants)
			{
				continue;
			}

			switch (uniform_group)
			{
			case NGL_DX12_UNIFORM_GROUP_PER_DRAW:
				cpu_address = draw_mem_allocs[shader_type].m_cpu_address;
				gpu_address = draw_mem_allocs[shader_type].m_gpu_address;
				break;

			case NGL_DX12_UNIFORM_GROUP_PER_RENDERER_CHANGE:
				cpu_address = m_renderer_mem_allocs[shader_type].m_cpu_address;
				gpu_address = m_renderer_mem_allocs[shader_type].m_gpu_address;
				break;

			default:
				assert(false);
				break;
			}

			if (cpu_address != nullptr)
			{
				assert(rs_shader.m_cb_size > 0);

				if (shader_type != NGL_COMPUTE_SHADER)
				{
					cmd_list->SetGraphicsRootConstantBufferView(rs_shader.m_cb_root_index, gpu_address);
				}
				else
				{
					cmd_list->SetComputeRootConstantBufferView(rs_shader.m_cb_root_index, gpu_address);
				}
			}
		}
	}

	// Set descriptor tables

	for (uint32_t uniform_group = 0; uniform_group < NGL_DX12_NUM_UNIFORM_GROUPS; uniform_group++)
	{
		if (!m_renderer_has_changed && uniform_group == NGL_DX12_UNIFORM_GROUP_PER_RENDERER_CHANGE)
		{
			continue;
		}

		for (uint32_t shader_type = 0; shader_type < NGL_NUM_SHADER_TYPES; shader_type++)
		{
			D3D12_root_signature_shader_stage& rs_shader = rs.m_groups[uniform_group].m_stages[shader_type];

			if (shader_type != NGL_COMPUTE_SHADER)
			{
				if (rs_shader.m_num_view_bind_heap_entries > 0)
				{
					cmd_list->SetGraphicsRootDescriptorTable(rs_shader.m_view_root_index, rs_shader.m_view_gpu_handle);
				}

				if (rs_shader.m_num_sampler_bind_heap_entries > 0)
				{
					cmd_list->SetGraphicsRootDescriptorTable(rs_shader.m_sampler_root_index, rs_shader.m_sampler_gpu_handle);
				}
			}
			else
			{
				if (rs_shader.m_num_view_bind_heap_entries > 0)
				{
					cmd_list->SetComputeRootDescriptorTable(rs_shader.m_view_root_index, rs_shader.m_view_gpu_handle);
				}

				if (rs_shader.m_num_sampler_bind_heap_entries > 0)
				{
					cmd_list->SetComputeRootDescriptorTable(rs_shader.m_sampler_root_index, rs_shader.m_sampler_gpu_handle);
				}
			}
		}
	}
}

// D3D12_graphics_job ////////////////////////////////////////////////////////////////////////////////////

D3D12_graphics_job::D3D12_graphics_job(D3D12_backend* backend)
	: D3D12_job(backend, false)
{
	m_num_drawcalls = 0;
	m_is_using_system_color_attachment = false;
	memset(&m_pso_desc_stub, 0, sizeof(m_pso_desc_stub));
	memset(&m_viewport, 0, sizeof(m_viewport));
	memset(&m_scissor, 0, sizeof(m_scissor));
	if (m_useRenderpass)
	{
		m_attachments_endAccessType.clear();
		m_attachments_cpu_desc_handles.clear();
		m_dsv_attachments_endAccessType.clear();
		m_dsv_attachments_cpu_desc_handles.clear();
	}
}


D3D12_graphics_job::~D3D12_graphics_job()
{
	DeleteRenderers();
}


void D3D12_graphics_job::Init(NGL_job_descriptor& descriptor)
{
	D3D12_job::Init(descriptor);

	InitViewportScissor();

	for (size_t i = 0; i < m_descriptor.m_attachments.size(); i++)
	{
		if (m_descriptor.m_attachments[i].m_attachment.m_idx == NGL_DX12_SYSTEM_ATTACHMENT)
		{
			m_is_using_system_color_attachment = true;
		}
	}
}


void D3D12_graphics_job::InitViewportScissor()
{
	NGL_subpass& sp = m_descriptor.m_subpasses[m_current_subpass];

	D3D12_resource_manager* res_mgr = m_backend->m_resource_mgr;
	D3D12_swap_chain* swap_chain = m_backend->m_swap_chain;

	int32_t viewport[4];
	memset(viewport, 0, sizeof(uint32_t) * 4);

	for (size_t i = 0; i < m_descriptor.m_attachments.size(); i++)
	{
		// Select color attachment

		if (sp.m_usages[i] == NGL_COLOR_ATTACHMENT)
		{
			NGL_attachment_descriptor& color_desc = m_descriptor.m_attachments[i];
			uint32_t texture_id = color_desc.m_attachment.m_idx;

			if (texture_id == NGL_DX12_SYSTEM_ATTACHMENT)
			{
				viewport[2] = m_backend->m_width;
				viewport[3] = m_backend->m_height;
			}
			else
			{
				D3D12_texture* color_texture = res_mgr->m_textures[texture_id];
				viewport[2] = color_texture->m_ngl_texture.m_texture_descriptor.m_size[0] / (1 << color_desc.m_attachment.m_level);
				viewport[3] = color_texture->m_ngl_texture.m_texture_descriptor.m_size[1] / (1 << color_desc.m_attachment.m_level);
			}
		}

		// Select depth attachment

		if (sp.m_usages[i] == NGL_DEPTH_ATTACHMENT
			|| sp.m_usages[i] == NGL_READ_ONLY_DEPTH_ATTACHMENT
			|| sp.m_usages[i] == NGL_READ_ONLY_DEPTH_ATTACHMENT_AND_SHADER_RESOURCE)
		{
			NGL_attachment_descriptor& depth_desc = m_descriptor.m_attachments[i];
			uint32_t texture_id = depth_desc.m_attachment.m_idx;

			if (texture_id == NGL_DX12_SYSTEM_ATTACHMENT)
			{
				viewport[2] = m_backend->m_width;
				viewport[3] = m_backend->m_height;
			}
			else
			{
				D3D12_texture* depth_texture = res_mgr->m_textures[texture_id];
				viewport[2] = depth_texture->m_ngl_texture.m_texture_descriptor.m_size[0] / (1 << depth_desc.m_attachment.m_level);
				viewport[3] = depth_texture->m_ngl_texture.m_texture_descriptor.m_size[1] / (1 << depth_desc.m_attachment.m_level);
			}
		}
	}


	// Init current state

	memcpy(m_current_state.m_viewport, viewport, sizeof(uint32_t) * 4);
	memcpy(m_current_state.m_scissor, viewport, sizeof(uint32_t) * 4);
}


void D3D12_graphics_job::ClearAttachments()
{
	assert(m_current_subpass == 0);
	NGL_subpass& sp = m_descriptor.m_subpasses[0];
	NGL_subpass sp_next;
	if (m_useRenderpass)
	{
		if (m_descriptor.m_subpasses.size() > 1)
		{
			sp_next = m_descriptor.m_subpasses[1];
		}
	}
	ID3D12GraphicsCommandList* cmd_list = m_command_context->m_command_list;
	D3D12_resource_manager* res_mgr = m_backend->m_resource_mgr;
	D3D12_swap_chain* swap_chain = m_backend->m_swap_chain;

	m_attachement_states.clear();
	m_attachement_states.resize(m_descriptor.m_attachments.size(), NGL_DX12_RESOURCE_STATE_UNKNOWN);

	if (m_useRenderpass)
	{
		m_attachement_states_next.clear();
		m_attachement_states_next.resize(m_descriptor.m_attachments.size(), NGL_DX12_RESOURCE_STATE_UNKNOWN);
	}

	for (size_t i = 0; i < m_descriptor.m_attachments.size(); i++)
	{
		m_attachement_states[i] = GetResourceState(sp.m_usages[i]);
	}

	if (m_useRenderpass)
	{
		if (m_descriptor.m_subpasses.size() > 1)
		{
			for (size_t i = 0; i < m_descriptor.m_attachments.size(); i++)
			{
				m_attachement_states_next[i] = GetResourceState(sp_next.m_usages[i]);
			}
		}
	}

	for (size_t i = 0; i < m_descriptor.m_attachments.size(); i++)
	{
		NGL_attachment_descriptor& desc = m_descriptor.m_attachments[i];

		if (desc.m_attachment_load_op == NGL_LOAD_OP_CLEAR)
		{
			D3D12_texture* texture = nullptr;
			uint32_t texture_id = desc.m_attachment.m_idx;
			D3D12_CPU_DESCRIPTOR_HANDLE attachment_handle;
			uint32_t subresource_index;

			assert(sp.m_usages[i] != NGL_COLOR_ATTACHMENT_AND_INPUT_ATTACHMENT_AND_SHADER_RESOURCE); // can only occur in subsequent subpasses

			if (GetAttachementType(sp.m_usages[i]) == NGL_DX12_COLOR_ATTACHEMENT)
			{
				if (texture_id == NGL_DX12_SYSTEM_ATTACHMENT)
				{
					texture = swap_chain->m_system_color_buffer[swap_chain->m_current_swap_buffer];
					subresource_index = 0;
					attachment_handle = res_mgr->m_color_attachment_heap.GetCPUHandle(swap_chain->m_system_rtv_offsets[swap_chain->GetCurrentSwapBufferIndex()]);
				}
				else
				{
					texture = res_mgr->m_textures[texture_id];
					subresource_index = texture->GetSubresourceIndex(desc.m_attachment.m_level, desc.m_attachment.m_layer, desc.m_attachment.m_face, 0);
					attachment_handle = res_mgr->m_color_attachment_heap.GetCPUHandle(texture->m_rtv_offsets[subresource_index]);
				}

				m_command_context->VerifyTextureState(*texture, subresource_index, D3D12_RESOURCE_STATE_RENDER_TARGET);

				if (!m_useRenderpass)
				{
					cmd_list->ClearRenderTargetView(attachment_handle, res_mgr->m_textures[texture_id]->m_ngl_texture.m_texture_descriptor.m_clear_value, 0, nullptr);
				}
				else if (m_is_singlepass)
				{
					cmd_list->ClearRenderTargetView(attachment_handle, res_mgr->m_textures[texture_id]->m_ngl_texture.m_texture_descriptor.m_clear_value, 0, nullptr);
				}

#ifdef NGL_DX12_DEBUG_TRANSITIONS
				_logf("Clear Color Attachement (res: %p sub: %d name: %s)", (void*)(texture->m_res), subresource_index, texture->m_name.c_str());
#endif
			}

			if (GetAttachementType(sp.m_usages[i]) == NGL_DX12_DEPTH_ATTACHEMENT)
			{
				if (texture_id == NGL_DX12_SYSTEM_ATTACHMENT)
				{
					texture = swap_chain->m_system_depth_buffer;
					subresource_index = 0;
				}
				else
				{
					texture = res_mgr->m_textures[texture_id];
					subresource_index = texture->GetSubresourceIndex(desc.m_attachment.m_level, desc.m_attachment.m_layer, desc.m_attachment.m_face, 0);
				}
				attachment_handle = res_mgr->m_depth_attachment_heap.GetCPUHandle(texture->m_dsv_offsets[2 * subresource_index]);

				m_command_context->VerifyTextureState(*texture, subresource_index, D3D12_RESOURCE_STATE_DEPTH_WRITE);

				if (!m_useRenderpass)
				{
					cmd_list->ClearDepthStencilView(attachment_handle, D3D12_CLEAR_FLAG_DEPTH, res_mgr->m_textures[texture_id]->m_ngl_texture.m_texture_descriptor.m_clear_value[0], 0, 0, nullptr);
				}
				else if (m_is_singlepass)
				{
					cmd_list->ClearDepthStencilView(attachment_handle, D3D12_CLEAR_FLAG_DEPTH, res_mgr->m_textures[texture_id]->m_ngl_texture.m_texture_descriptor.m_clear_value[0], 0, 0, nullptr);
				}

#ifdef NGL_DX12_DEBUG_TRANSITIONS
				_logf("Clear Depth Attachement (res: %p sub: %d name: %s)", (void*)(texture->m_res), subresource_index, texture->m_name.c_str());
#endif
			}
		}
	}
}


void D3D12_graphics_job::SetAttachmentStates()
{
	assert(m_current_subpass != 0);
	NGL_subpass& sp = m_descriptor.m_subpasses[m_current_subpass];
	NGL_subpass sp_next;
	if (m_useRenderpass)
	{
		if ((m_descriptor.m_subpasses.size() > 1) && m_current_subpass < 2)
		{
			sp_next = m_descriptor.m_subpasses[m_current_subpass + 1];
		}
	}

	ID3D12GraphicsCommandList* cmd_list = m_command_context->m_command_list;
	D3D12_resource_manager* res_mgr = m_backend->m_resource_mgr;
	D3D12_swap_chain* swap_chain = m_backend->m_swap_chain;

	m_barrier_batch.clear();
	D3D12_RESOURCE_BARRIER barrier;

	// Collect the barriers

	if (m_useRenderpass)
	{
		if ((m_descriptor.m_subpasses.size() > 1) && m_current_subpass < 2)
		{
			for (size_t i = 0; i < m_descriptor.m_attachments.size(); i++)
			{
				if (sp_next.m_usages[i] != m_attachement_states_next[i])
				{
					m_attachement_states_next[i] = GetResourceState(sp_next.m_usages[i]);
				}
			}
		}
	}

	for (size_t i = 0; i < m_descriptor.m_attachments.size(); i++)
	{
		D3D12_RESOURCE_STATES new_state = GetResourceState(sp.m_usages[i]);
		if (m_attachement_states[i] != new_state)
		{
			D3D12_texture* texture = nullptr;
			NGL_attachment_descriptor& desc = m_descriptor.m_attachments[i];
			uint32_t texture_id = desc.m_attachment.m_idx;

			uint32_t subresource_index;

			if (GetAttachementType(sp.m_usages[i]) == NGL_DX12_COLOR_ATTACHEMENT)
			{
				if (texture_id == NGL_DX12_SYSTEM_ATTACHMENT)
				{
					texture = swap_chain->m_system_color_buffer[swap_chain->m_current_swap_buffer];
					subresource_index = 0;
				}
				else
				{
					texture = res_mgr->m_textures[texture_id];
					subresource_index = texture->GetSubresourceIndex(desc.m_attachment.m_level, desc.m_attachment.m_layer, desc.m_attachment.m_face, 0);
				}
			}

			if (GetAttachementType(sp.m_usages[i]) == NGL_DX12_DEPTH_ATTACHEMENT)
			{
				if (texture_id == NGL_DX12_SYSTEM_ATTACHMENT)
				{
					texture = swap_chain->m_system_depth_buffer;
					subresource_index = 0;
				}
				else
				{
					texture = res_mgr->m_textures[texture_id];
					subresource_index = texture->GetSubresourceIndex(desc.m_attachment.m_level, desc.m_attachment.m_layer, desc.m_attachment.m_face, 0);
				}
			}

			DescribeBarrier(texture->m_res, subresource_index, m_attachement_states[i], new_state, barrier);
			m_barrier_batch.push_back(barrier);
			m_attachement_states[i] = new_state;

			m_command_context->UpdateTrackedTextureState(*texture, subresource_index, barrier);
		}
	}

	// Execute the barriers
	if (m_barrier_batch.empty() == false)
	{
		m_command_context->AddResourceBarrierList(m_barrier_batch);
	}
}


AttachementType D3D12_graphics_job::GetAttachementType(NGL_resource_state state)
{
	switch (state)
	{
	case NGL_COLOR_ATTACHMENT:
	case NGL_COLOR_ATTACHMENT_AND_PRESERVED_ATTACHMENT:
	case NGL_COLOR_ATTACHMENT_AND_INPUT_ATTACHMENT_AND_SHADER_RESOURCE:
		return NGL_DX12_COLOR_ATTACHEMENT;

	case NGL_DEPTH_ATTACHMENT:
	case NGL_READ_ONLY_DEPTH_ATTACHMENT:
	case NGL_READ_ONLY_DEPTH_ATTACHMENT_AND_SHADER_RESOURCE:
		return NGL_DX12_DEPTH_ATTACHEMENT;

	default:
		return NGL_DX12_NOT_ATTACHEMENT;
	}
}


void D3D12_graphics_job::SelectRenderTarget()
{
	NGL_subpass& sp = m_descriptor.m_subpasses[m_current_subpass];

	D3D12_resource_manager* res_mgr = m_backend->m_resource_mgr;
	D3D12_swap_chain* swap_chain = m_backend->m_swap_chain;

	m_color_attachments.clear();
	m_depth_attachment = D3D12_depth_attachment();

	for (size_t i = 0; i < m_descriptor.m_attachments.size(); i++)
	{
		// Select color attachment

		if (sp.m_usages[i] == NGL_COLOR_ATTACHMENT)
		{
			NGL_attachment_descriptor& color_desc = m_descriptor.m_attachments[i];
			uint32_t texture_id = color_desc.m_attachment.m_idx;
			D3D12_texture* color_texture = res_mgr->m_textures[texture_id];

			if (texture_id != NGL_DX12_SYSTEM_ATTACHMENT)
			{
				if (color_texture->m_ngl_texture.m_texture_descriptor.m_is_renderable == false)
				{
					_logf("DX12 - GenJob: Color attachment (%d) is not renderable!", texture_id);
					assert(0);
				}
				if (color_texture->m_ngl_texture.m_is_color == false)
				{
					_logf("DX12 - GenJob: Color attachment (%d) is not a color texture!", texture_id);
					assert(0);
				}
			}

			m_color_attachments.push_back(D3D12_color_attachment(color_texture, texture_id, (uint32_t)i, true /*bind the RT format in PSO*/));
		}
		else if (m_useRenderpass && (m_current_subpass > 0) && (sp.m_usages[i] == NGL_COLOR_ATTACHMENT_AND_INPUT_ATTACHMENT_AND_SHADER_RESOURCE))
		{
			NGL_attachment_descriptor& color_desc = m_descriptor.m_attachments[i];
			uint32_t                   texture_id = color_desc.m_attachment.m_idx;
			D3D12_texture* color_texture = res_mgr->m_textures[texture_id];

			if (texture_id != NGL_DX12_SYSTEM_ATTACHMENT)
			{
				if (color_texture->m_ngl_texture.m_texture_descriptor.m_is_renderable == false)
				{
					_logf("DX12 - GenJob: Color attachment (%d) is not renderable!", texture_id);
					assert(0);
				}
				if (color_texture->m_ngl_texture.m_is_color == false)
				{
					_logf("DX12 - GenJob: Color attachment (%d) is not a color texture!", texture_id);
					assert(0);
				}
			}

			m_color_attachments.push_back(D3D12_color_attachment(color_texture, texture_id, (uint32_t)i, false /*Don't bind the RT format in PSO*/));
		}
		// Select depth attachment

		if (sp.m_usages[i] == NGL_DEPTH_ATTACHMENT
			|| sp.m_usages[i] == NGL_READ_ONLY_DEPTH_ATTACHMENT
			|| sp.m_usages[i] == NGL_READ_ONLY_DEPTH_ATTACHMENT_AND_SHADER_RESOURCE)
		{
			NGL_attachment_descriptor& depth_desc = m_descriptor.m_attachments[i];
			uint32_t texture_id = depth_desc.m_attachment.m_idx;
			D3D12_texture* depth_texture;
			bool depth_is_read_only = sp.m_usages[i] == NGL_READ_ONLY_DEPTH_ATTACHMENT || sp.m_usages[i] == NGL_READ_ONLY_DEPTH_ATTACHMENT_AND_SHADER_RESOURCE;

			if (texture_id == NGL_DX12_SYSTEM_ATTACHMENT)
			{
				depth_texture = swap_chain->m_system_depth_buffer;
			}
			else
			{
				depth_texture = res_mgr->m_textures[texture_id];

				if (depth_texture->m_ngl_texture.m_texture_descriptor.m_is_renderable == false)
				{
					_logf("DX12 - GenJob: Depth attachment (%d) is not renderable!", texture_id);
					assert(0);
				}
				if (depth_texture->m_ngl_texture.m_is_color)
				{
					_logf("DX12 - GenJob: Depth attachment (%d) is not a depth texture!", texture_id);
					assert(0);
				}
			}

			m_depth_attachment = D3D12_depth_attachment(depth_texture, texture_id, (uint32_t)i, depth_is_read_only);
		}
	}

	assert(m_color_attachments.size() <= 8);
}

static D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE ToBeginningAccessType(NGL_attachment_load_op loadOp, bool render, bool isFirstSubPass)
{
	switch (loadOp)
	{
	case NGL_LOAD_OP_LOAD:
		if (isFirstSubPass)
			return D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_PRESERVE;
		else if (render)
			return D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_PRESERVE_LOCAL_RENDER;
		else
			return D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_PRESERVE_LOCAL_SRV;
	case NGL_LOAD_OP_DONT_CARE:
		return D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_DISCARD;
	case NGL_LOAD_OP_CLEAR:
		return D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_CLEAR;

	default:
		std::abort();
	}
}

static D3D12_RENDER_PASS_ENDING_ACCESS_TYPE ToEndingAccessType(NGL_attachment_store_op storeOp, bool render, BOOL isLastSubpass)
{
	switch (storeOp)
	{
	case NGL_STORE_OP_DONT_CARE:
		if (isLastSubpass)
			return D3D12_RENDER_PASS_ENDING_ACCESS_TYPE_DISCARD;
	case NGL_STORE_OP_STORE:
		if (isLastSubpass)
			return D3D12_RENDER_PASS_ENDING_ACCESS_TYPE_PRESERVE;
		else if (render)
			return D3D12_RENDER_PASS_ENDING_ACCESS_TYPE_PRESERVE_LOCAL_RENDER;
		else
			return D3D12_RENDER_PASS_ENDING_ACCESS_TYPE_PRESERVE_LOCAL_SRV;
	default:
		std::abort();
	}
}

D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE SelectMatchingBeginAccessType(D3D12_RENDER_PASS_ENDING_ACCESS_TYPE ea)
{
	switch (ea)
	{
	case D3D12_RENDER_PASS_ENDING_ACCESS_TYPE_DISCARD:
		return D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_DISCARD;
	case D3D12_RENDER_PASS_ENDING_ACCESS_TYPE_PRESERVE:
		return D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_PRESERVE;
	case D3D12_RENDER_PASS_ENDING_ACCESS_TYPE_RESOLVE:
		return D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_PRESERVE;
	case D3D12_RENDER_PASS_ENDING_ACCESS_TYPE_NO_ACCESS:
		return D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_NO_ACCESS;
	case D3D12_RENDER_PASS_ENDING_ACCESS_TYPE_PRESERVE_LOCAL_RENDER:
		return D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_PRESERVE_LOCAL_RENDER;
	case D3D12_RENDER_PASS_ENDING_ACCESS_TYPE_PRESERVE_LOCAL_SRV:
		return D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_PRESERVE_LOCAL_SRV;
	case D3D12_RENDER_PASS_ENDING_ACCESS_TYPE_PRESERVE_LOCAL_UAV:
		return D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_PRESERVE_LOCAL_UAV;
	default:
		return D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_DISCARD;
	}
}

void D3D12_graphics_job::CreateRTDescriptors(D3D12_texture* color_texture, NGL_attachment_descriptor& color_desc, uint32_t attachmentID)
{
	BOOL isRender = false;
	BOOL isRender_next = false;
	D3D12_RESOURCE_STATES currentResourceType = D3D12_RESOURCE_STATE_COMMON;
	D3D12_RESOURCE_STATES nextResourceType = D3D12_RESOURCE_STATE_COMMON;

	// DX12 render passes
	{
		const BOOL isLastSubPass = (m_current_subpass == (m_descriptor.m_subpasses.size() - 1)) ? TRUE : FALSE;
		const BOOL isFirstSubPass = (m_current_subpass == 0) ? TRUE : FALSE;

		currentResourceType = m_attachement_states[attachmentID];

		if (((currentResourceType & D3D12_RESOURCE_STATE_RENDER_TARGET) == D3D12_RESOURCE_STATE_RENDER_TARGET) ||
			((currentResourceType & D3D12_RESOURCE_STATE_DEPTH_WRITE) == D3D12_RESOURCE_STATE_DEPTH_WRITE) ||
			((currentResourceType & D3D12_RESOURCE_STATE_UNORDERED_ACCESS) == D3D12_RESOURCE_STATE_UNORDERED_ACCESS))
		{
			isRender = true;
		}
		else
		{
			isRender = false;
		}

		nextResourceType = m_attachement_states_next[attachmentID];
		if (((nextResourceType & D3D12_RESOURCE_STATE_RENDER_TARGET) == D3D12_RESOURCE_STATE_RENDER_TARGET) ||
			((nextResourceType & D3D12_RESOURCE_STATE_DEPTH_WRITE) == D3D12_RESOURCE_STATE_DEPTH_WRITE) ||
			((nextResourceType & D3D12_RESOURCE_STATE_UNORDERED_ACCESS) == D3D12_RESOURCE_STATE_UNORDERED_ACCESS))
		{
			isRender_next = true;
		}
		else
		{
			isRender_next = false;
		}
		m_renderPassRenderTargetDescs.emplace_back();
		auto& rtDesc = m_renderPassRenderTargetDescs.back();

		rtDesc.BeginningAccess.Type = ToBeginningAccessType(color_desc.m_attachment_load_op, isRender, isFirstSubPass);
		rtDesc.EndingAccess.Type = ToEndingAccessType(color_desc.m_attachment_store_op, isRender_next, isLastSubPass);

		rtDesc.EndingAccess.Resolve = {};
		rtDesc.cpuDescriptor = m_color_attachment_handles.back();

		// Force PRESERVE for the surfaces that get dropped before final pass.
		if ((m_current_subpass < m_pass_size - 1) &&
			(rtDesc.EndingAccess.Type == D3D12_RENDER_PASS_ENDING_ACCESS_TYPE_PRESERVE_LOCAL_RENDER))
		{
			for (int index = 0; index < m_midpass_dropped_handles[m_current_subpass].size(); index++) {
				auto& dropped_handle = m_midpass_dropped_handles[m_current_subpass][index];
				auto& current_ptr = rtDesc.cpuDescriptor.ptr;
				if (dropped_handle == current_ptr) {
					rtDesc.EndingAccess.Type = D3D12_RENDER_PASS_ENDING_ACCESS_TYPE_PRESERVE;
				}
			}
		}

		auto it = std::find(m_attachments_cpu_desc_handles.begin(), m_attachments_cpu_desc_handles.end(), rtDesc.cpuDescriptor.ptr);

		if (it == m_attachments_cpu_desc_handles.end())
		{
			//New RTV. Not seen in any previous RPs.
			m_attachments_cpu_desc_handles.push_back(rtDesc.cpuDescriptor.ptr);
			m_attachments_endAccessType.push_back(rtDesc.EndingAccess.Type);
		}
		else
		{
			//Old RTV. Seen in at least one of the previous RPs.
			UINT index = static_cast<UINT>(it - m_attachments_cpu_desc_handles.begin());

			rtDesc.BeginningAccess.Type = SelectMatchingBeginAccessType(m_attachments_endAccessType[index]);

			//Need to update with new EA.
			if (rtDesc.EndingAccess.Type != m_attachments_endAccessType[index])
				m_attachments_endAccessType[index] = rtDesc.EndingAccess.Type;
		}

		switch (rtDesc.BeginningAccess.Type)
		{
		case D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_CLEAR:
			rtDesc.BeginningAccess.Clear.ClearValue.Format = color_texture->m_format;
			std::copy_n(&color_texture->m_ngl_texture.m_texture_descriptor.m_clear_value[0], 4, &rtDesc.BeginningAccess.Clear.ClearValue.Color[0]);
			break;
		case D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_PRESERVE_LOCAL_RENDER:
		case D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_PRESERVE_LOCAL_SRV:
			rtDesc.BeginningAccess.PreserveLocal.AdditionalHeight = 0;
			rtDesc.BeginningAccess.PreserveLocal.AdditionalWidth = 0;
			break;
		default:
			break;
		}

		switch (rtDesc.EndingAccess.Type)
		{
		case D3D12_RENDER_PASS_ENDING_ACCESS_TYPE_PRESERVE_LOCAL_RENDER:
		case D3D12_RENDER_PASS_ENDING_ACCESS_TYPE_PRESERVE_LOCAL_SRV:
			rtDesc.EndingAccess.PreserveLocal.AdditionalHeight = 0;
			rtDesc.EndingAccess.PreserveLocal.AdditionalWidth = 0;
			break;
		case D3D12_RENDER_PASS_ENDING_ACCESS_TYPE_RESOLVE:
			break;
		default:
			break;
		}
	}

	return;

}

void D3D12_graphics_job::CreateDSDescriptors(D3D12_texture* depth_texture, NGL_attachment_descriptor& depth_desc)
{

	BOOL isRender = false;
	BOOL isRender_next = false;
	D3D12_RESOURCE_STATES currentResourceType = D3D12_RESOURCE_STATE_COMMON;
	D3D12_RESOURCE_STATES nextResourceType = D3D12_RESOURCE_STATE_COMMON;

	// DX12 render passes
	{
		const BOOL isLastSubPass = (m_current_subpass == (m_descriptor.m_subpasses.size() - 1)) ? TRUE : FALSE;
		const BOOL isFirstSubPass = (m_current_subpass == 0) ? TRUE : FALSE;

		currentResourceType = m_attachement_states[m_depth_attachment.m_attachment_id];

		if (((currentResourceType & D3D12_RESOURCE_STATE_RENDER_TARGET) == D3D12_RESOURCE_STATE_RENDER_TARGET) ||
			((currentResourceType & D3D12_RESOURCE_STATE_DEPTH_WRITE) == D3D12_RESOURCE_STATE_DEPTH_WRITE) ||
			((currentResourceType & D3D12_RESOURCE_STATE_UNORDERED_ACCESS) == D3D12_RESOURCE_STATE_UNORDERED_ACCESS))
		{
			isRender = true;
		}
		else
		{
			isRender = false;
		}

		nextResourceType = m_attachement_states_next[m_depth_attachment.m_attachment_id];

		if (((nextResourceType & D3D12_RESOURCE_STATE_RENDER_TARGET) == D3D12_RESOURCE_STATE_RENDER_TARGET) ||
			((nextResourceType & D3D12_RESOURCE_STATE_DEPTH_WRITE) == D3D12_RESOURCE_STATE_DEPTH_WRITE) ||
			((nextResourceType & D3D12_RESOURCE_STATE_UNORDERED_ACCESS) == D3D12_RESOURCE_STATE_UNORDERED_ACCESS))
		{
			isRender_next = true;
		}
		else
		{
			isRender_next = false;
		}

		m_renderPassDepthStencilDescs.emplace_back();
		auto& dsDesc = m_renderPassDepthStencilDescs.back();

		dsDesc.DepthBeginningAccess.Type = ToBeginningAccessType(depth_desc.m_attachment_load_op, isRender, isFirstSubPass);
		dsDesc.DepthEndingAccess.Type = ToEndingAccessType(depth_desc.m_attachment_store_op, isRender_next, isLastSubPass);

		dsDesc.DepthEndingAccess.Resolve = {};

		dsv_pair_cpu_descriptors_handle dsv_pair_cpu_descs = { m_depth_attachment_handles[1].ptr, m_depth_attachment_handles[0].ptr };

		auto it = std::find(m_dsv_attachments_cpu_desc_handles.begin(),
			m_dsv_attachments_cpu_desc_handles.end(),
			dsv_pair_cpu_descs);

		if (it == m_dsv_attachments_cpu_desc_handles.end())
		{
			//New DSV. Not seen in any previous RPs.
			m_dsv_attachments_cpu_desc_handles.push_back(dsv_pair_cpu_descs);
			m_dsv_attachments_endAccessType.push_back(dsDesc.DepthEndingAccess.Type);
		}
		else
		{
			//Old DSV. Seen in at least one of the previous RPs.
			UINT index = static_cast<UINT>(it - m_dsv_attachments_cpu_desc_handles.begin());

			dsDesc.DepthBeginningAccess.Type = SelectMatchingBeginAccessType(m_dsv_attachments_endAccessType[index]);

			//Need to update with new EA.
			if (dsDesc.DepthEndingAccess.Type != m_dsv_attachments_endAccessType[index])
				m_dsv_attachments_endAccessType[index] = dsDesc.DepthEndingAccess.Type;

		}
		switch (dsDesc.DepthBeginningAccess.Type)
		{
		case D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_CLEAR:
			dsDesc.DepthBeginningAccess.Clear.ClearValue.Format = GetDSVFormat(depth_texture->m_format);
			dsDesc.DepthBeginningAccess.Clear.ClearValue.DepthStencil.Depth = depth_texture->m_ngl_texture.m_texture_descriptor.m_clear_value[0];
			break;
		case D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_PRESERVE_LOCAL_RENDER:
		case D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_PRESERVE_LOCAL_SRV:
			dsDesc.DepthBeginningAccess.PreserveLocal.AdditionalHeight = 0;
			dsDesc.DepthBeginningAccess.PreserveLocal.AdditionalWidth = 0;
			break;
		default:
			break;
		}

		switch (dsDesc.DepthEndingAccess.Type)
		{
		case D3D12_RENDER_PASS_ENDING_ACCESS_TYPE_PRESERVE_LOCAL_RENDER:
		case D3D12_RENDER_PASS_ENDING_ACCESS_TYPE_PRESERVE_LOCAL_SRV:
			dsDesc.DepthEndingAccess.PreserveLocal.AdditionalHeight = 0;
			dsDesc.DepthEndingAccess.PreserveLocal.AdditionalWidth = 0;
			break;
		case D3D12_RENDER_PASS_ENDING_ACCESS_TYPE_RESOLVE:
			break;
		default:
			break;
		}
		// We do not have stencil and it is not bound. Use NO_ACCESS for that.
		dsDesc.StencilBeginningAccess.Type = D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_NO_ACCESS;
		dsDesc.StencilEndingAccess.Type = D3D12_RENDER_PASS_ENDING_ACCESS_TYPE_NO_ACCESS;

		// Clone it into the second desc
		m_renderPassDepthStencilDescs.push_back(m_renderPassDepthStencilDescs[0]);

		m_renderPassDepthStencilDescs[0].cpuDescriptor = m_depth_attachment_handles[0];
		m_renderPassDepthStencilDescs[1].cpuDescriptor = m_depth_attachment_handles[1];

		// The second depth is read-only and we don't want to clear it.
		if (m_renderPassDepthStencilDescs[1].DepthBeginningAccess.Type == D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_CLEAR)
		{
			m_renderPassDepthStencilDescs[1].DepthBeginningAccess.Type = D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_PRESERVE;
		}
	}

	return;
}

void D3D12_graphics_job::PrepareRenderTarget()
{
	// Define how to execute this pass.
	bool execute_as_multipass = false;
	if (m_useRenderpass)
	{
		execute_as_multipass = !m_is_singlepass;
	}

	D3D12_resource_manager* res_mgr = m_backend->m_resource_mgr;
	D3D12_swap_chain* swap_chain = m_backend->m_swap_chain;
	ID3D12GraphicsCommandList* cmd_list = m_command_context->m_command_list;
	NGL_subpass& sp = m_descriptor.m_subpasses[m_current_subpass];

	m_color_attachment_handles.clear();
	m_depth_attachment_handles.clear();

	m_color_attachment_handles.reserve(m_color_attachments.size());
	m_depth_attachment_handles.reserve(2);

	if (execute_as_multipass)
	{
		m_renderPassRenderTargetDescs.clear();
		m_renderPassDepthStencilDescs.clear();
		m_renderPassRenderTargetDescs.reserve(m_color_attachments.size());
		m_renderPassDepthStencilDescs.reserve(2);
	}
	// Prepare color attachment

	for (size_t i = 0; i < m_color_attachments.size(); i++)
	{
		D3D12_texture* color_texture;
		NGL_attachment_descriptor& color_desc = m_descriptor.m_attachments[m_color_attachments[i].m_attachment_id];

		uint32_t subresource_index;
		if (color_desc.m_attachment.m_idx == NGL_DX12_SYSTEM_ATTACHMENT)
		{
			color_texture = swap_chain->m_system_color_buffer[swap_chain->m_current_swap_buffer];
			m_color_attachment_handles.push_back(res_mgr->m_color_attachment_heap.GetCPUHandle((uint32_t)swap_chain->m_system_rtv_offsets[swap_chain->GetCurrentSwapBufferIndex()]));

			subresource_index = 0;
		}
		else
		{
			color_texture = m_color_attachments[i].m_texture;

			subresource_index = color_texture->GetSubresourceIndex(color_desc.m_attachment.m_level, color_desc.m_attachment.m_layer, color_desc.m_attachment.m_face, 0);
			m_color_attachment_handles.push_back(res_mgr->m_color_attachment_heap.GetCPUHandle(color_texture->m_rtv_offsets[subresource_index]));
		}
		if (execute_as_multipass)
		{
			CreateRTDescriptors(color_texture, color_desc, m_color_attachments[i].m_attachment_id);
		}
		if (m_color_attachments[i].m_isPSOBound)
		{
			m_command_context->VerifyTextureState(*color_texture, subresource_index, D3D12_RESOURCE_STATE_RENDER_TARGET);
		}
		else
		{
			m_command_context->VerifyTextureState(*color_texture, subresource_index, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
		}
	}

	// Prepare depth attachment

	if (m_depth_attachment.m_texture != nullptr)
	{
		D3D12_texture* depth_texture = m_depth_attachment.m_texture;
		NGL_attachment_descriptor& depth_desc = m_descriptor.m_attachments[m_depth_attachment.m_attachment_id];

		uint32_t subresource_index = depth_texture->GetSubresourceIndex(depth_desc.m_attachment.m_level, depth_desc.m_attachment.m_layer, depth_desc.m_attachment.m_face, 0);
		m_depth_attachment_handles.push_back(res_mgr->m_depth_attachment_heap.GetCPUHandle(depth_texture->m_dsv_offsets[2 * subresource_index]));
		m_depth_attachment_handles.push_back(res_mgr->m_depth_attachment_heap.GetCPUHandle(depth_texture->m_dsv_offsets[2 * subresource_index + 1]));

		if (execute_as_multipass)
			CreateDSDescriptors(depth_texture, depth_desc);

		if (m_depth_attachment.m_is_read_only)
		{
			m_command_context->VerifyTextureState(*depth_texture, subresource_index, D3D12_RESOURCE_STATE_DEPTH_READ);
		}
		else
		{
			m_command_context->VerifyTextureState(*depth_texture, subresource_index, D3D12_RESOURCE_STATE_DEPTH_WRITE);
		}
	}

	// Set render target

	if (execute_as_multipass)
	{
		D3D12_RENDER_PASS_FLAGS rpFlags = D3D12_RENDER_PASS_FLAG_NONE;

		if ((m_depth_attachment.m_texture != nullptr) &&
			(m_depth_attachment.m_is_read_only == true))
		{
			rpFlags |= D3D12_RENDER_PASS_FLAG_BIND_READ_ONLY_DEPTH;
		}

		ID3D12GraphicsCommandList4* cmd_list4;
		cmd_list->QueryInterface(IID_PPV_ARGS(&cmd_list4));
		cmd_list4->Release();
		cmd_list4->BeginRenderPass(
			(UINT)m_renderPassRenderTargetDescs.size(),
			m_renderPassRenderTargetDescs.data(),
			m_depth_attachment.m_texture != nullptr ? &m_renderPassDepthStencilDescs[m_depth_attachment.m_is_read_only ? 1 : 0] : nullptr,
			rpFlags);
	}
	else
	{
		cmd_list->OMSetRenderTargets(
			(uint32_t)m_color_attachment_handles.size(),
			m_color_attachment_handles.data(),
			false,
			m_depth_attachment.m_texture != nullptr ? &m_depth_attachment_handles[m_depth_attachment.m_is_read_only ? 1 : 0] : nullptr);
	}

#ifdef NGL_DX12_DEBUG_TRANSITIONS

	for (size_t i = 0; i < m_color_attachments.size(); i++)
	{
		NGL_texture_subresource& ts = m_descriptor.m_attachments[m_color_attachments[i].m_attachment_id].m_attachment;
		uint32_t subresource_index = m_color_attachments[i].m_texture->GetSubresourceIndex(ts.m_level, ts.m_layer, ts.m_face, 0);
		_logf("Set Color Attachement (res: %p sub: %d name: %s)", (void*)(m_color_attachments[i].m_texture->m_res), subresource_index, m_color_attachments[i].m_texture->m_name.c_str());
	}

	if (m_depth_attachment.m_texture != nullptr)
	{
		NGL_texture_subresource& ts = m_descriptor.m_attachments[m_depth_attachment.m_attachment_id].m_attachment;
		uint32_t subresource_index = m_depth_attachment.m_texture->GetSubresourceIndex(ts.m_level, ts.m_layer, ts.m_face, 0);
		_logf("Set Depth Attachement (res: %p sub: %d name: %s)", (void*)(m_depth_attachment.m_texture->m_res), subresource_index, m_depth_attachment.m_texture->m_name.c_str());
	}
#endif
}


void D3D12_graphics_job::SetViewportScissor(int32_t* viewport, int32_t* scissor)
{
	if (viewport)
	{
		memcpy(m_current_state.m_viewport, viewport, sizeof(int32_t) * 4);
	}

	if (scissor)
	{
		memcpy(m_current_state.m_scissor, scissor, sizeof(int32_t) * 4);
	}
}


void D3D12_graphics_job::SetBlendState(uint32_t attachment_id, NGL_blend_func blend_func, NGL_color_channel_mask color_channel_mask)
{
	m_current_state.m_blend_state.m_funcs[attachment_id] = blend_func;
	m_current_state.m_blend_state.m_masks[attachment_id] = color_channel_mask;
}


void D3D12_graphics_job::SetDepthState(NGL_depth_func depth_func, bool depth_mask)
{
	m_current_state.m_depth_state.m_func = depth_func;
	m_current_state.m_depth_state.m_mask = depth_mask;
}


void D3D12_graphics_job::Begin(D3D12_command_context* command_context)
{
	D3D12_job::Begin(command_context);

	m_current_subpass = 0;

#ifdef NGL_DX12_DEBUG_TRANSITIONS
	_logf("Graphics Job Begin: %s", m_descriptor.m_subpasses[m_current_subpass].m_name.c_str());
#endif

	if (m_is_using_system_color_attachment && !m_command_context->m_is_using_system_color_attachment)
	{
		assert(m_backend->m_swap_chain->GetCurrentSwapBuffer()->m_current_state == D3D12_RESOURCE_STATE_PRESENT);
		m_command_context->AddResourceBarrier(m_backend->m_swap_chain->GetCurrentSwapBuffer(), 0, m_backend->m_swap_chain->GetCurrentSwapBuffer()->m_current_state, D3D12_RESOURCE_STATE_RENDER_TARGET);

		m_command_context->m_is_using_system_color_attachment = true;
	}

	// Reset multipass access-type tracking lists since this is the first sub-pass of multipass.
	if (m_useRenderpass)
	{
		m_attachments_endAccessType.clear();
		m_attachments_cpu_desc_handles.clear();
		m_dsv_attachments_endAccessType.clear();
		m_dsv_attachments_cpu_desc_handles.clear();
	}

	ClearAttachments();
	SelectRenderTarget();
	PrepareRenderTarget();
}

void D3D12_graphics_job::NextSubpass()
{
	assert(m_is_recording);

	if (m_useRenderpass)
	{
		ID3D12GraphicsCommandList* cmd_list = m_command_context->m_command_list;

		ID3D12GraphicsCommandList4* cmd_list4;
		cmd_list->QueryInterface(IID_PPV_ARGS(&cmd_list4));
		cmd_list4->Release();

		cmd_list4->EndRenderPass();
	}

	m_current_subpass++;

#ifdef NGL_DX12_DEBUG_TRANSITIONS
	_logf("Subpass (%d): %s", m_current_subpass, m_descriptor.m_subpasses[m_current_subpass].m_name.c_str());
	_logf("Subpass Transitions Begin");
#endif

	SetAttachmentStates();

#ifdef NGL_DX12_DEBUG_TRANSITIONS
	_logf("Subpass Transitions End");
#endif

	SelectRenderTarget();
	PrepareRenderTarget();
}


void D3D12_graphics_job::End()
{
	D3D12_job::End();

#ifdef NGL_DX12_DEBUG_TRANSITIONS
	_logf("Graphics Job End: %s", m_descriptor.m_subpasses[m_current_subpass].m_name.c_str());
#endif
}


void D3D12_graphics_job::InitPipelineState()
{
	D3D12_GRAPHICS_PIPELINE_STATE_DESC& pso_desc = m_pso_desc_stub;

	memset(&pso_desc, 0, sizeof(pso_desc));

	ResetRasterizerDesc(pso_desc.RasterizerState);
	ResetBlendStateDesc(pso_desc.BlendState);
	ResetDepthStencilDesc(pso_desc.DepthStencilState);

	pso_desc.RasterizerState.FrontCounterClockwise = TRUE;
	pso_desc.DepthStencilState.DepthEnable = FALSE;
	pso_desc.DepthStencilState.StencilEnable = FALSE;
	pso_desc.SampleMask = UINT_MAX;
	pso_desc.SampleDesc.Count = 1;

	FillPipelineStateRenderTargetFields();
}


void D3D12_graphics_job::FillPipelineStateRenderTargetFields()
{
	D3D12_GRAPHICS_PIPELINE_STATE_DESC& pso_desc = m_pso_desc_stub;

	// Reset fields

	pso_desc.NumRenderTargets = 0;
	memset(pso_desc.RTVFormats, 0, sizeof(DXGI_FORMAT) * 8);
	memset(&pso_desc.DSVFormat, 0, sizeof(DXGI_FORMAT));

	// Fill fields

	for (size_t i = 0; i < m_color_attachments.size(); i++)
	{
		D3D12_color_attachment& color_attachment = m_color_attachments[i];

		if (color_attachment.m_isPSOBound == false)
		{
			continue;
		}

		if (color_attachment.m_texture_id == NGL_DX12_SYSTEM_ATTACHMENT)
		{
			pso_desc.RTVFormats[pso_desc.NumRenderTargets] = DXGI_FORMAT_R8G8B8A8_UNORM;
		}
		else
		{
			pso_desc.RTVFormats[pso_desc.NumRenderTargets] = color_attachment.m_texture->m_format;
		}

		pso_desc.NumRenderTargets++;
	}

	if (m_depth_attachment.m_texture != nullptr)
	{
		if (m_depth_attachment.m_texture_id == NGL_DX12_SYSTEM_ATTACHMENT)
		{
			pso_desc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
		}
		else
		{
			pso_desc.DSVFormat = GetDSVFormat(m_depth_attachment.m_texture->m_format);
		}
	}
}


void D3D12_graphics_job::UpdateCurrentPipelineState(NGL_primitive_type primitive_type, uint32_t shader_id, uint32_t num_vbos, uint32_t* vbos, NGL_cull_mode cull_mode)
{
	D3D12_resource_manager* res_mgr = m_backend->m_resource_mgr;
	ID3D12GraphicsCommandList* cmd_list = m_command_context->m_command_list;
	D3D12_GRAPHICS_PIPELINE_STATE_DESC& pso_desc = m_pso_desc_stub;

	if (!m_has_previous_state)
	{
		InitPipelineState();
	}

	// Parse NGL state

	m_current_state.m_subpass = (uint32_t)m_current_subpass;
	m_current_state.m_cull_mode = cull_mode;
	m_current_state.m_primitive_type = primitive_type;
	m_current_state.m_shader.m_shader_code = shader_id;
	m_current_state.m_shader.m_vbo_hash = 0;
	for (uint32_t i = 0; i < num_vbos; i++)
	{
		m_current_state.m_shader.m_vbo_hash += res_mgr->m_vertex_buffers[vbos[i]]->m_ngl_vertex_buffer.m_hash;
	}

	// Apply changes

	uint32_t changed_mask = m_has_previous_state ? NGL_state::ChangedMask(m_current_state, m_previous_state) : ~0;

	if (changed_mask & NGL_SHADER_MASK)
	{
		// GetRenderer() always returns the appropriate renderer for the specified shader
	}

	if (changed_mask & NGL_SUBPASS_MASK)
	{
		FillPipelineStateRenderTargetFields();
	}

	if (changed_mask & NGL_PRIMITIVE_TYPE_MASK)
	{
		// TODO encapsulate switch
		switch (m_current_state.m_primitive_type)
		{
		case NGL_POINTS:
			pso_desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;
			break;
		case NGL_LINES:
			pso_desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;
			break;
		case NGL_TRIANGLES:
			pso_desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
			break;
		case NGL_PATCH3:
		case NGL_PATCH4:
		case NGL_PATCH16:
			pso_desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH;
			break;

		default:
			pso_desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_UNDEFINED;
			assert(0);
			break;
		}

		cmd_list->IASetPrimitiveTopology(GetPrimitiveTopology(primitive_type));
	}

	if (changed_mask & NGL_CULL_MODE_MASK)
	{
		// TODO encapsulate switch
		switch (m_current_state.m_cull_mode)
		{
		case NGL_FRONT_SIDED:
			pso_desc.RasterizerState.CullMode = D3D12_CULL_MODE_BACK;
			break;
		case NGL_BACK_SIDED:
			pso_desc.RasterizerState.CullMode = D3D12_CULL_MODE_FRONT;
			break;
		case NGL_TWO_SIDED:
			pso_desc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
			break;

		default:
			assert(0);
			break;
		}
	}

	if ((changed_mask & NGL_COLOR_BLEND_FUNCS_MASK) | (changed_mask & NGL_COLOR_MASKS_MASK))
	{
		ResetBlendStateDesc(pso_desc.BlendState);

		for (size_t i = 0; i < m_color_attachments.size(); i++)
		{
			DescribeBlendState(m_current_state.m_blend_state.m_funcs[i], m_current_state.m_blend_state.m_masks[i], pso_desc.BlendState.RenderTarget[i]);
		}
	}

	if ((changed_mask & NGL_DEPTH_FUNC_MASK) | (changed_mask & NGL_DEPTH_MASK_MASK))
	{
		if (m_depth_attachment.m_texture != nullptr)
		{
			DescribeDepthStencilState(m_current_state.m_depth_state.m_func, m_current_state.m_depth_state.m_mask, pso_desc.DepthStencilState);

			if (m_current_state.m_depth_state.m_func == NGL_DEPTH_LESS_WITH_OFFSET)
			{
				pso_desc.RasterizerState.DepthBiasClamp = 200.0f;
				pso_desc.RasterizerState.SlopeScaledDepthBias = 1.0f;
			}

			NGL_attachment_descriptor& depth_desc = m_descriptor.m_attachments[m_depth_attachment.m_attachment_id];
			uint32_t subresource_index = m_depth_attachment.m_texture->GetSubresourceIndex(depth_desc.m_attachment.m_level, depth_desc.m_attachment.m_layer, depth_desc.m_attachment.m_face, 0);
			bool depth_is_read_only = !m_current_state.m_depth_state.m_mask;
			if (depth_is_read_only)
			{
				m_command_context->VerifyTextureState(*m_depth_attachment.m_texture, subresource_index, D3D12_RESOURCE_STATE_DEPTH_READ);
			}
			else
			{
				m_command_context->VerifyTextureState(*m_depth_attachment.m_texture, subresource_index, D3D12_RESOURCE_STATE_DEPTH_WRITE);
			}

			if (!m_useRenderpass)
			{
				cmd_list->OMSetRenderTargets((uint32_t)m_color_attachment_handles.size(), m_color_attachment_handles.data(), false, &m_depth_attachment_handles[depth_is_read_only ? 1 : 0]);
			}
		}
	}

	if ((changed_mask & NGL_VIEWPORT_MASK) | (changed_mask & NGL_DEPTH_FUNC_MASK))
	{
		m_viewport.TopLeftX = (FLOAT)m_current_state.m_viewport[0];
		m_viewport.TopLeftY = (FLOAT)m_current_state.m_viewport[1];
		m_viewport.Width = (FLOAT)m_current_state.m_viewport[2];
		m_viewport.Height = (FLOAT)m_current_state.m_viewport[3];
		DescribeDepthFuncRange(m_current_state.m_depth_state.m_func, m_viewport.MinDepth, m_viewport.MaxDepth);

		cmd_list->RSSetViewports(1, &m_viewport);
	}

	if (changed_mask & NGL_SCISSOR_MASK)
	{
		m_scissor.left = (LONG)m_current_state.m_scissor[0];
		m_scissor.top = (LONG)m_current_state.m_scissor[1];
		m_scissor.right = (LONG)m_current_state.m_scissor[0] + (LONG)m_current_state.m_scissor[2];
		m_scissor.bottom = (LONG)m_current_state.m_scissor[1] + (LONG)m_current_state.m_scissor[3];

		cmd_list->RSSetScissorRects(1, &m_scissor);
	}

	m_previous_state = m_current_state;
	m_has_previous_state = true;
}


void D3D12_graphics_job::Draw(NGL_primitive_type primitive_type, uint32_t shader_id, uint32_t num_vbos, uint32_t* vbos, uint32_t ebo, NGL_cull_mode cull_mode, const void* parameters[])
{
	assert(m_is_recording);

	ID3D12GraphicsCommandList* cmd_list = m_command_context->m_command_list;
	D3D12_resource_manager* res_mgr = m_backend->m_resource_mgr;

	UpdateCurrentPipelineState(primitive_type, shader_id, num_vbos, vbos, cull_mode);
	D3D12_graphics_renderer* renderer = SetRenderer(m_current_state, num_vbos, vbos, m_pso_desc_stub);
	if (renderer == nullptr)
	{
		return;
	}

	BindUniforms(renderer, parameters);

	D3D12_index_buffer& ib = *res_mgr->m_index_buffers[ebo];
	cmd_list->IASetIndexBuffer(&ib.m_index_buffer_view_desc);
	for (size_t i = 0; i < num_vbos; i++)
	{
		D3D12_vertex_buffer& vb = *res_mgr->m_vertex_buffers[vbos[i]];
		cmd_list->IASetVertexBuffers((uint32_t)i, 1, &vb.m_vertex_buffer_view_desc);
	}

	cmd_list->DrawIndexedInstanced(ib.m_ngl_index_buffer.m_num_indices, 1, 0, 0, 0);
}


D3D12_graphics_renderer* D3D12_graphics_job::SetRenderer(NGL_state& state, uint32_t num_vbos, uint32_t* vbos, D3D12_GRAPHICS_PIPELINE_STATE_DESC& pso_desc_stub)
{
	ID3D12GraphicsCommandList* cmd_list = m_command_context->m_command_list;

	m_renderer_has_changed = false;

	D3D12_graphics_renderer* renderer = GetRenderer(m_current_state, num_vbos, vbos);
	if (renderer == nullptr)
	{
		return nullptr;
	}

	if (m_last_renderer == nullptr || m_last_renderer != renderer)
	{
		cmd_list->SetGraphicsRootSignature(renderer->m_root_signature.m_root_signature);
		m_last_renderer = renderer;
		m_renderer_has_changed = true;
	}

	ID3D12PipelineState* pso = renderer->GetPipelineState(m_pso_desc_stub);
	assert(pso != nullptr);
	if (m_last_pso == nullptr || m_last_pso != pso)
	{
		cmd_list->SetPipelineState(pso);
		m_last_pso = pso;
		m_renderer_has_changed = true;
	}

	return renderer;
}


D3D12_graphics_renderer* D3D12_graphics_job::GetRenderer(NGL_state& state, uint32_t num_vbos, uint32_t* vbos)
{
	uint64_t hash = GenerateRendererHash(state, num_vbos, vbos);

	auto item = m_graphics_renderers.find(hash);
	if (item != m_graphics_renderers.end())
	{
		return item->second;
	}
	else
	{
		D3D12_graphics_renderer* new_renderer = (D3D12_graphics_renderer*)(CreateRenderer(state, num_vbos, vbos));
		m_graphics_renderers.insert(std::pair<uint64_t, D3D12_graphics_renderer*>(hash, new_renderer));
		return new_renderer;
	}
}


NGL_renderer* D3D12_graphics_job::CreateRenderer(NGL_state& state, uint32_t num_vbos, uint32_t* vbos)
{
	D3D12_graphics_renderer* new_renderer = new D3D12_graphics_renderer(this);
	bool is_init_successful = new_renderer->Init(this, state, num_vbos, vbos);
	if (is_init_successful)
	{
		return new_renderer;
	}
	else
	{
		delete new_renderer;
		return nullptr;
	}
}

uint64_t D3D12_graphics_job::GenerateRendererHash(NGL_state& state, uint32_t num_vbos, uint32_t* vbos)
{
	uint64_t hash = 0;
	hash = state.m_shader.m_shader_code;
	hash <<= 32;
	hash += state.m_shader.m_vbo_hash;
	return hash;
}


void D3D12_graphics_job::DeleteRenderers()
{
	for (auto i = m_graphics_renderers.begin(); i != m_graphics_renderers.end(); i++)
	{
		delete i->second;
	}

	m_graphics_renderers.clear();
}


void D3D12_graphics_job::CustomAction(uint32_t parameter)
{
	switch (parameter)
	{
	case NGL_D3D12_ACTION_SET_CLEAR_RED:
	{
		assert(m_is_recording);
		float clear_color[4] = { 1.0, 0.0, 0.0, 1.0 };
		D3D12_CPU_DESCRIPTOR_HANDLE color_attachment = m_backend->m_resource_mgr->m_color_attachment_heap.GetCPUHandle((uint32_t)m_backend->m_swap_chain->m_system_rtv_offsets[m_backend->m_swap_chain->m_current_swap_buffer]);
		m_command_context->m_command_list->ClearRenderTargetView(color_attachment, clear_color, 0, nullptr);
		break;
	}
	case NGL_D3D12_ACTION_SET_CLEAR_GREEN:
	{
		assert(m_is_recording);
		float clear_color[4] = { 0.0, 1.0, 0.0, 1.0 };
		D3D12_CPU_DESCRIPTOR_HANDLE color_attachment = m_backend->m_resource_mgr->m_color_attachment_heap.GetCPUHandle((uint32_t)m_backend->m_swap_chain->m_system_rtv_offsets[m_backend->m_swap_chain->m_current_swap_buffer]);
		m_command_context->m_command_list->ClearRenderTargetView(color_attachment, clear_color, 0, nullptr);
		break;
	}
	case NGL_D3D12_ACTION_SET_CLEAR_BLUE:
	{
		assert(m_is_recording);
		float clear_color[4] = { 0.0, 0.0, 1.0, 1.0 };
		D3D12_CPU_DESCRIPTOR_HANDLE color_attachment = m_backend->m_resource_mgr->m_color_attachment_heap.GetCPUHandle((uint32_t)m_backend->m_swap_chain->m_system_rtv_offsets[m_backend->m_swap_chain->m_current_swap_buffer]);
		m_command_context->m_command_list->ClearRenderTargetView(color_attachment, clear_color, 0, nullptr);
		break;
	}

	default:
		assert(false); // Unsupported custom action
		break;
	}
}


// D3D12_compute_job ////////////////////////////////////////////////////////////////////////////////////

D3D12_compute_job::D3D12_compute_job(D3D12_backend* backend)
	: D3D12_job(backend, true)
{
	memset(&m_pso_desc_stub, 0, sizeof(m_pso_desc_stub));
}


D3D12_compute_job::~D3D12_compute_job()
{
	DeleteRenderers();
}


void D3D12_compute_job::Init(NGL_job_descriptor& descriptor)
{
	D3D12_job::Init(descriptor);
}


void D3D12_compute_job::Begin(D3D12_command_context* command_context)
{
	D3D12_job::Begin(command_context);

#ifdef NGL_DX12_DEBUG_TRANSITIONS
	_logf("Compute Job Begin (%d): %s", m_current_subpass, m_descriptor.m_subpasses[m_current_subpass].m_name.c_str());
#endif
}


void D3D12_compute_job::End()
{
	D3D12_job::End();

#ifdef NGL_DX12_DEBUG_TRANSITIONS
	_logf("Compute Job End (%d): %s", m_current_subpass, m_descriptor.m_subpasses[m_current_subpass].m_name.c_str());
#endif
}


void D3D12_compute_job::UpdateCurrentPipelineState(uint32_t shader_id)
{
	D3D12_COMPUTE_PIPELINE_STATE_DESC& pso_desc = m_pso_desc_stub;

	if (!m_has_previous_state)
	{
		memset(&pso_desc, 0, sizeof(pso_desc));
	}

	// Parse NGL state

	m_current_state.m_shader.m_shader_code = shader_id;

	m_previous_state = m_current_state;
	m_has_previous_state = true;
}


bool D3D12_compute_job::Dispatch(uint32_t shader_id, uint32_t x, uint32_t y, uint32_t z, const void** parameters)
{
	ID3D12GraphicsCommandList* cmd_list = m_command_context->m_command_list;
	D3D12_resource_manager* res_mgr = m_backend->m_resource_mgr;
	D3D12_memory_manager* mem_mgr = m_command_context->m_memory_mgr;

	UpdateCurrentPipelineState(shader_id);
	D3D12_compute_renderer* renderer = SetRenderer(m_current_state, m_pso_desc_stub);
	if (renderer == nullptr)
	{
		return false;
	}

	BindUniforms(renderer, parameters);

	cmd_list->Dispatch(x, y, z);
	return true;
}


D3D12_compute_renderer* D3D12_compute_job::SetRenderer(NGL_state& state, D3D12_COMPUTE_PIPELINE_STATE_DESC& pso_desc_stub)
{
	ID3D12GraphicsCommandList* cmd_list = m_command_context->m_command_list;

	m_renderer_has_changed = false;

	D3D12_compute_renderer* renderer = GetRenderer(m_current_state);
	if (renderer == nullptr)
	{
		return nullptr;
	}

	if (m_last_renderer == nullptr || m_last_renderer != renderer)
	{
		cmd_list->SetComputeRootSignature(renderer->m_root_signature.m_root_signature);
		m_last_renderer = renderer;
		m_renderer_has_changed = true;
	}

	ID3D12PipelineState* pso = renderer->GetPipelineState(m_pso_desc_stub);
	assert(pso != nullptr);
	if (m_last_pso == nullptr || m_last_pso != pso)
	{
		cmd_list->SetPipelineState(pso);
		m_last_pso = pso;
		m_renderer_has_changed = true;
	}

	return renderer;
}


NGL_renderer* D3D12_compute_job::CreateRenderer(NGL_state& state, uint32_t num_vbos, uint32_t* vbos)
{
	D3D12_compute_renderer* new_renderer = new D3D12_compute_renderer(this);
	bool is_init_successful = new_renderer->Init(this, state);
	if (is_init_successful)
	{
		return new_renderer;
	}
	else
	{
		delete new_renderer;
		return nullptr;
	}
}


D3D12_compute_renderer* D3D12_compute_job::GetRenderer(NGL_state& state)
{
	auto item = m_compute_renderers.find(state.m_shader.m_shader_code);
	if (item != m_compute_renderers.end())
	{
		return item->second;
	}
	else
	{
		D3D12_compute_renderer* new_renderer = (D3D12_compute_renderer*)(CreateRenderer(state, 0, nullptr));
		m_compute_renderers.insert(std::pair<uint32_t, D3D12_compute_renderer*>(state.m_shader.m_shader_code, new_renderer));
		return new_renderer;
	}
}


void D3D12_compute_job::DeleteRenderers()
{
	for (auto i = m_compute_renderers.begin(); i != m_compute_renderers.end(); i++)
	{
		delete i->second;
	}

	m_compute_renderers.clear();
}


void D3D12_compute_job::CustomAction(uint32_t parameter)
{
	assert(false); // Unsupported custom action
}
