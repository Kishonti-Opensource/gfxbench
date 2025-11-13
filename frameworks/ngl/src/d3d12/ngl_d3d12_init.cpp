/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "ngl_d3d12.h"


int32_t GetInteger(NGL_backend_property prop)
{
	return D3D12_backend::INSTANCE->GetInteger(prop);
}


const char* GetString(NGL_backend_property prop)
{
	return D3D12_backend::INSTANCE->GetString(prop);
}


void BeginCommandBuffer(uint32_t command_context_id)
{
	D3D12_command_context *command_context = D3D12_backend::INSTANCE->GetCommandContext(command_context_id);
	command_context->Reset();
	D3D12_backend::INSTANCE->m_resource_mgr->m_view_bind_heap.BeginBlock();
}


void EndCommandBuffer(uint32_t command_context_id)
{
	D3D12_command_context *command_context = D3D12_backend::INSTANCE->GetCommandContext(command_context_id);
	command_context->Close();
}


void SubmitCommandBuffer(uint32_t command_context_id)
{
	D3D12_command_context *command_context = D3D12_backend::INSTANCE->GetCommandContext(command_context_id);
	uint64_t fence_value = command_context->Execute();
	command_context->Discard(fence_value);
	D3D12_backend::INSTANCE->m_resource_mgr->m_view_bind_heap.EndBlock(fence_value);
}


void Barrier(uint32_t command_context_id, std::vector<NGL_texture_subresource_transition> &texture_barriers, std::vector<NGL_buffer_transition> &buffer_barriers)
{
	D3D12_command_context *command_context = D3D12_backend::INSTANCE->GetCommandContext(command_context_id);
	command_context->AddNGLBarrierList(texture_barriers, buffer_barriers);
}


uint32_t GenJob(NGL_job_descriptor &descriptor)
{
	return D3D12_backend::INSTANCE->GenJob(descriptor);
}


void Begin(uint32_t job_id, uint32_t command_context_id)
{
	D3D12_job *job = D3D12_backend::INSTANCE->Job(job_id);
	D3D12_command_context *command_context = D3D12_backend::INSTANCE->GetCommandContext(command_context_id);
	job->Begin(command_context);
}


void NextSubpass(uint32_t job_id)
{
	D3D12_graphics_job *job = D3D12_backend::INSTANCE->GraphicsJob(job_id);
	job->NextSubpass();
}


void End(uint32_t job_id)
{
	D3D12_job *job = D3D12_backend::INSTANCE->Job(job_id);
	job->End();
}


void Flush()
{
	//NOP
}


void Finish()
{
	D3D12_backend::INSTANCE->m_direct_queue->Finish();
}


void ViewportScissor(uint32_t job_id, int32_t viewport[4], int32_t scissor[4])
{
	D3D12_graphics_job *job = D3D12_backend::INSTANCE->GraphicsJob(job_id);
	job->SetViewportScissor(viewport, scissor);
}


void BlendState(uint32_t job_id, uint32_t attachment_id, NGL_blend_func blend_func, NGL_color_channel_mask color_channel_mask)
{
	D3D12_graphics_job *job = D3D12_backend::INSTANCE->GraphicsJob(job_id);
	job->SetBlendState(attachment_id, blend_func, color_channel_mask);
}


void DepthState(uint32_t job_id, NGL_depth_func depth_func, bool depth_mask)
{
	D3D12_graphics_job *job = D3D12_backend::INSTANCE->GraphicsJob(job_id);
	job->SetDepthState(depth_func, depth_mask);
}


bool CreateVertexBuffer(uint32_t &buffer_id, NGL_vertex_descriptor &vertex_layout, uint32_t num_vertices, void *data)
{
	return D3D12_backend::INSTANCE->m_resource_mgr->CreateVertexBuffer(buffer_id, vertex_layout, num_vertices, data);
}


bool CreateIndexBuffer(uint32_t &buffer_id, NGL_format index_format, uint32_t num_indices, void *data)
{
	return D3D12_backend::INSTANCE->m_resource_mgr->CreateIndexBuffer(buffer_id, index_format, num_indices, data);
}


bool GenTexture(uint32_t &texture_id, NGL_texture_descriptor &texture_layout, std::vector< std::vector<uint8_t> > *data)
{
	return D3D12_backend::INSTANCE->m_resource_mgr->CreateTexture(texture_id, texture_layout, data);
}


bool GetTextureContent(uint32_t texture_id, uint32_t mip_level, uint32_t surface_index, uint32_t face_index, NGL_format format, NGL_resource_state state, uint32_t &width, uint32_t &height, std::vector<uint8_t> &data)
{
	return D3D12_backend::INSTANCE->m_resource_mgr->GetTextureContent(texture_id, mip_level, surface_index, face_index, format, state, width, height, data);
}


void Draw(uint32_t job_id, NGL_primitive_type primitive_type, uint32_t shader_id, uint32_t num_vbos, uint32_t *vbos, uint32_t ebo, NGL_cull_mode cull_mode, const void** parameters)
{
	D3D12_graphics_job *job = D3D12_backend::INSTANCE->GraphicsJob(job_id);
	job->Draw(primitive_type, shader_id, num_vbos, vbos, ebo, cull_mode, parameters);
}


bool Dispatch(uint32_t job_id, uint32_t shader_id, uint32_t x, uint32_t y, uint32_t z, const void** parameters)
{
	D3D12_compute_job *job = D3D12_backend::INSTANCE->ComputeJob(job_id);
	return job->Dispatch(shader_id, x, y, z, parameters);
}


void DeletePipelines(uint32_t job_id)
{
	D3D12_job *job = D3D12_backend::INSTANCE->Job(job_id);
	job->DeleteRenderers();
}


void CustomAction(uint32_t job_id, uint32_t parameter)
{
	D3D12_backend::INSTANCE->CustomAction(job_id, parameter);
}


void DestroyContext()
{
	delete D3D12_backend::INSTANCE;
	D3D12_backend::INSTANCE = nullptr;
}


void SetProcInterface()
{
	nglGetInteger = GetInteger;
	nglGetString = GetString;
	nglBeginCommandBuffer = BeginCommandBuffer;
	nglEndCommandBuffer = EndCommandBuffer;
	nglSubmitCommandBuffer = SubmitCommandBuffer;
	nglBarrier = Barrier;
	nglGenJob = GenJob;
	nglBegin = Begin;
	nglNextSubpass = NextSubpass;
	nglEnd = End;
	nglFlush = Flush;
	nglFinish = Finish;
	nglViewportScissor = ViewportScissor;
	nglBlendState = BlendState;
	nglDepthState = DepthState;
	nglGenVertexBuffer = CreateVertexBuffer;
	nglGenIndexBuffer = CreateIndexBuffer;
	nglGenTexture = GenTexture;
	nglGetTextureContent = GetTextureContent;
	nglDraw = Draw;
	nglDispatch = Dispatch;
	nglDeletePipelines = DeletePipelines;
	nglCustomAction = CustomAction;
	nglDestroyContext = DestroyContext;
}


void nglCreateContextD3D12(NGL_context_descriptor& descriptor)
{
	D3D12_backend::INSTANCE = new D3D12_backend();
	D3D12_backend::INSTANCE->CreateContext(descriptor);

	SetProcInterface();
}
