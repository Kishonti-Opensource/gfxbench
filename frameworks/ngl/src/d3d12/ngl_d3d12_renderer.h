/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef NGL_D3D12_RENDERER_H
#define NGL_D3D12_RENDERER_H

#include "ngl_d3d12_define.h"
#include "ngl_d3d12_resource.h"
#include "ngl_d3d12_bind.h"
#include "ngl_d3d12_memory.h"
#include "ngl_d3d12_job.h"

class D3D12_backend;


// use for: D3D12_GRAPHICS_PIPELINE_STATE_DESC, D3D12_COMPUTE_PIPELINE_STATE_DESC
template<class T>
struct D3D12_pipeline_state_comparator
{
	bool operator()(const T& a, const T& b) const
	{
		// the m_psos map keys are only descriptor stubs, and do not contain pointers, so the bitwise comparison is valid
		// the fields are initialized after the comparison by FillPipelineStateShaderFields()
		return memcmp(&a, &b, sizeof(T)) > 0;
	}
};


struct D3D12_renderer : public NGL_renderer
{
	D3D12_renderer(D3D12_job *job);
	virtual ~D3D12_renderer();
	bool Init(D3D12_job *job, NGL_state &state);

	D3D12_job *m_job;
	D3D12_backend *m_backend;
	ID3DBlob *m_shader_blobs[NGL_NUM_SHADER_TYPES]; // weak reference (no delete)
	bool has_shaders;
	D3D12_root_signature m_root_signature;

	std::vector<NGL_shader_uniform> m_application_uniforms;
};


struct D3D12_graphics_renderer : public D3D12_renderer
{
	typedef std::map<D3D12_GRAPHICS_PIPELINE_STATE_DESC, ID3D12PipelineState*, D3D12_pipeline_state_comparator<D3D12_GRAPHICS_PIPELINE_STATE_DESC> > PSOMap;

	D3D12_graphics_renderer(D3D12_graphics_job *job);
	virtual ~D3D12_graphics_renderer();
	bool Init(D3D12_graphics_job *job, NGL_state &state, uint32_t num_vbos, uint32_t *vbos);
	ID3D12PipelineState *GetPipelineState(D3D12_GRAPHICS_PIPELINE_STATE_DESC &pso_desc_stub);
	void FillPipelineStateShaderFields(D3D12_GRAPHICS_PIPELINE_STATE_DESC &pso_desc);
	void InputLayoutReflection(ID3DBlob *vs_shader, uint32_t num_vbos, uint32_t *vbos, std::vector<D3D12_INPUT_ELEMENT_DESC> &input_elements);

	std::vector<D3D12_INPUT_ELEMENT_DESC> m_input_elements;
	D3D12_GRAPHICS_PIPELINE_STATE_DESC m_pso_desc_complete;
	PSOMap m_psos;
};


struct D3D12_compute_renderer : public D3D12_renderer
{
	typedef std::map<D3D12_COMPUTE_PIPELINE_STATE_DESC, ID3D12PipelineState*, D3D12_pipeline_state_comparator<D3D12_COMPUTE_PIPELINE_STATE_DESC> > PSOMap;

	D3D12_compute_renderer(D3D12_compute_job *job);
	virtual ~D3D12_compute_renderer();
	ID3D12PipelineState *GetPipelineState(D3D12_COMPUTE_PIPELINE_STATE_DESC &pso_desc_stub);
	void FillPipelineStateShaderFields(D3D12_COMPUTE_PIPELINE_STATE_DESC &pso_desc);

	D3D12_COMPUTE_PIPELINE_STATE_DESC m_pso_desc_complete;
	PSOMap m_psos;
};

#endif
