/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef NGL_D3D12_JOB_H
#define NGL_D3D12_JOB_H

#include "ngl_d3d12_define.h"
#include "ngl_d3d12_command.h"
#include "ngl_d3d12_resource.h"
#include "ngl_d3d12_bind.h"
#include "ngl_d3d12_memory.h"
#include <map>
#include <vector>

// Enable render pass if platform supports it.
// If platform does not support render pass, application will not attempt to use the render pass interfaces.
#define USE_RENDER_PASS 1

// NGL_DX12_DEBUG_TRANSITIONS related flag ()
//	 This suppresses assert that happens if states are the same in queue and command list.
//   This is a warning for a potential performance impact. By default debug and release version not handled as error.
#define DISABLE_FIRST_STATE_ERROR_ASSERTS 1

class D3D12_backend;
struct D3D12_shader;
struct D3D12_graphics_renderer;
struct D3D12_compute_renderer;
struct D3D12_renderer;

enum AttachementType
{
	NGL_DX12_COLOR_ATTACHEMENT,
	NGL_DX12_DEPTH_ATTACHEMENT,
	NGL_DX12_NOT_ATTACHEMENT
};

typedef struct dsv_pair_cpu_descriptors_handle
{
	SIZE_T dsv_rw_cpu_ptr;
	SIZE_T dsv_ro_cpu_ptr;

	bool operator==(dsv_pair_cpu_descriptors_handle rhs) const
	{
		return ((this->dsv_ro_cpu_ptr == rhs.dsv_ro_cpu_ptr) &&
			(this->dsv_rw_cpu_ptr == rhs.dsv_rw_cpu_ptr));
	}

}dsv_pair_cpu_descriptors_handle;

struct D3D12_job : public NGL_job
{
	D3D12_job(D3D12_backend* backend, bool is_compute);
	virtual ~D3D12_job();

	virtual void Init(NGL_job_descriptor& descriptor);
	virtual void Begin(D3D12_command_context* command_context);
	virtual void End();
	void FillConstantBuffers(D3D12_renderer* renderer, const void* parameters[], std::vector<D3D12_memory_allocation>& mem_allocs, D3D12_uniform_group uniform_group);
	void FillDescriptorTables(D3D12_renderer* renderer, const void* parameters[], D3D12_uniform_group uniform_group);
	void FillBindHeaps(D3D12_renderer* renderer);
	void BindUniforms(D3D12_renderer* renderer, const void* parameters[]);
	virtual void DeleteRenderers() = 0;
	virtual void CustomAction(uint32_t parameter) = 0;

	bool m_is_compute;
	int m_current_subpass;
	D3D12_backend* m_backend;
	D3D12_command_context* m_command_context; // weak reference (no delete)
	bool m_has_previous_state;
	bool m_is_recording;

	D3D12_renderer* m_last_renderer;
	ID3D12PipelineState* m_last_pso;
	bool m_renderer_has_changed;

	std::vector<D3D12_memory_allocation> m_renderer_mem_allocs;

	// BEGIN: Render pass items
	bool m_useRenderpass;
	bool m_is_singlepass;
	int m_pass_size;
	void FindMidpassDroppedHandles();
	std::vector<SIZE_T> m_midpass_dropped_handles[2];
	// END: Render pass items

};


struct D3D12_graphics_job : public D3D12_job
{
	D3D12_graphics_job(D3D12_backend* backend);
	virtual ~D3D12_graphics_job();

	void Init(NGL_job_descriptor& descriptor) override;
	void InitViewportScissor();
	void ClearAttachments();
	void SetAttachmentStates();
	AttachementType GetAttachementType(NGL_resource_state state);
	void SelectRenderTarget();
	void PrepareRenderTarget();
	void SetViewportScissor(int32_t* viewport, int32_t* scissor);
	void SetBlendState(uint32_t attachment_id, NGL_blend_func blend_func, NGL_color_channel_mask color_channel_mask);
	void SetDepthState(NGL_depth_func depth_func, bool depth_mask);
	void Begin(D3D12_command_context* command_context) override;
	void NextSubpass();
	void End() override;
	void InitPipelineState();
	void FillPipelineStateRenderTargetFields();
	void UpdateCurrentPipelineState(NGL_primitive_type primitive_type, uint32_t shader_id, uint32_t num_vbos, uint32_t* vbos, NGL_cull_mode cull_mode);
	void Draw(NGL_primitive_type primitive_type, uint32_t shader_id, uint32_t num_vbos, uint32_t* vbos, uint32_t ebo, NGL_cull_mode cull_mode, const void* parameters[]);
	D3D12_graphics_renderer* SetRenderer(NGL_state& state, uint32_t num_vbos, uint32_t* vbos, D3D12_GRAPHICS_PIPELINE_STATE_DESC& pso_desc_stub);
	D3D12_graphics_renderer* GetRenderer(NGL_state& state, uint32_t num_vbos, uint32_t* vbos);
	NGL_renderer* CreateRenderer(NGL_state& state, uint32_t num_vbos, uint32_t* vbos) override;
	uint64_t GenerateRendererHash(NGL_state& state, uint32_t num_vbos, uint32_t* vbos);
	void DeleteRenderers() override;
	void CustomAction(uint32_t parameter) override;

	D3D12_GRAPHICS_PIPELINE_STATE_DESC m_pso_desc_stub;
	D3D12_VIEWPORT m_viewport;
	D3D12_RECT m_scissor;
	std::map<uint64_t, D3D12_graphics_renderer*> m_graphics_renderers;

	std::vector<D3D12_color_attachment> m_color_attachments;
	D3D12_depth_attachment m_depth_attachment;

	std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> m_color_attachment_handles; // one for each attachment
	std::vector<D3D12_RENDER_PASS_RENDER_TARGET_DESC> m_renderPassRenderTargetDescs;

	std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> m_depth_attachment_handles; // 0: read only, 1: read/write
	std::vector<D3D12_RENDER_PASS_DEPTH_STENCIL_DESC> m_renderPassDepthStencilDescs;

	std::vector<D3D12_RESOURCE_STATES> m_attachement_states;
	std::vector<D3D12_RESOURCE_STATES> m_attachement_states_next;
	std::vector<D3D12_RESOURCE_BARRIER> m_barrier_batch;
	bool m_is_using_system_color_attachment;

	// BEGIN: Render pass items
	std::vector<D3D12_RENDER_PASS_ENDING_ACCESS_TYPE> m_attachments_endAccessType;
	std::vector<SIZE_T> m_attachments_cpu_desc_handles;
	std::vector<D3D12_RENDER_PASS_ENDING_ACCESS_TYPE> m_dsv_attachments_endAccessType;
	std::vector<dsv_pair_cpu_descriptors_handle> m_dsv_attachments_cpu_desc_handles;
	void CreateRTDescriptors(D3D12_texture* color_texture, NGL_attachment_descriptor& color_desc, uint32_t attachmentID);
	void CreateDSDescriptors(D3D12_texture* depth_texture, NGL_attachment_descriptor& depth_desc);
	// END: Render pass items

	uint32_t m_num_drawcalls;
};


struct D3D12_compute_job : public D3D12_job
{
	D3D12_compute_job(D3D12_backend* backend);
	virtual ~D3D12_compute_job();

	void Init(NGL_job_descriptor& descriptor) override;
	void Begin(D3D12_command_context* command_context) override;
	void End() override;
	void UpdateCurrentPipelineState(uint32_t shader_id);
	bool Dispatch(uint32_t shader_id, uint32_t x, uint32_t y, uint32_t z, const void** parameters);
	D3D12_compute_renderer* SetRenderer(NGL_state& state, D3D12_COMPUTE_PIPELINE_STATE_DESC& pso_desc_stub);
	D3D12_compute_renderer* GetRenderer(NGL_state& state);
	NGL_renderer* CreateRenderer(NGL_state& state, uint32_t num_vbos, uint32_t* vbos) override;
	void DeleteRenderers() override;
	void CustomAction(uint32_t parameter) override;

	D3D12_COMPUTE_PIPELINE_STATE_DESC m_pso_desc_stub;
	std::map<uint32_t, D3D12_compute_renderer*> m_compute_renderers;
};

#endif
