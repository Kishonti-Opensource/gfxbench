/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "ngl_d3d12_renderer.h"

#include "ngl_d3d12.h"
#include "ngl_d3d12_memory.h"
#include "ngl_d3d12_job.h"
#include "ngl_d3d12_util.h"
#include "ngl_d3d12_action.h"
#include <cassert>
#include <string>
#include <map>


// D3D12_renderer ///////////////////////////////////////////////////////////////////////////////

D3D12_renderer::D3D12_renderer(D3D12_job *job)
{
	m_job = job;
	m_backend = job->m_backend;
	memset(m_shader_blobs, 0, sizeof(m_shader_blobs));
	has_shaders = false;
}


D3D12_renderer::~D3D12_renderer()
{

}


bool D3D12_renderer::Init(D3D12_job *job, NGL_state &state)
{
	NGL_shader_source_descriptor ssd[7];
	ID3DBlob *shader_blobs[NGL_NUM_SHADER_TYPES];
	memset(shader_blobs, 0, sizeof(shader_blobs));
	
	//std::vector<NGL_shader_uniform> application_uniforms;
	m_application_uniforms.clear();
	std::vector<NGL_shader_uniform> &application_uniforms = m_application_uniforms;

	// Load shader

	job->m_descriptor.m_load_shader_callback(job->m_descriptor, job->m_current_subpass, state.m_shader.m_shader_code, ssd, application_uniforms);

	for (uint32_t shader_type = NGL_VERTEX_SHADER; shader_type < NGL_NUM_SHADER_TYPES; shader_type++)
	{
		if (ssd[shader_type].m_source_data.empty())
		{
			continue;
		}

		// Sanity checks
		if (job->m_descriptor.m_is_compute)
		{
			assert(shader_type == NGL_COMPUTE_SHADER);
		}
		else
		{
			assert(shader_type != NGL_COMPUTE_SHADER);
		}

		// Compile the shader
		shader_blobs[shader_type] = m_backend->m_shader_cache->CompileShader(ssd[shader_type].m_source_data, ssd[shader_type].m_entry_point, ssd[shader_type].m_version);
		has_shaders = true;

		if (shader_blobs[shader_type] == nullptr)
		{
			_logf("DX12 - Shader compilation failed! (Cannot create blob)");
			has_shaders = false;
			return false;
		}
	}

	if (!has_shaders)
	{
		_logf("DX12 - Shader compilation failed! (No source data)");
		return false;
	}

	if (!job->m_is_compute && shader_blobs[NGL_VERTEX_SHADER] == nullptr)
	{
		_logf("DX12 - No vertex shader attached in graphics job!");
		assert(false);
	}

	if (job->m_is_compute && shader_blobs[NGL_COMPUTE_SHADER] == nullptr)
	{
		_logf("DX12 - No compute shader attached in compute job!");
		assert(false);
	}

	memcpy(m_shader_blobs, shader_blobs, sizeof(m_shader_blobs));

#ifdef NGL_DX12_PRINT_SHADERS
	PrintShaders(ssd);
#endif

	// Create the root signature with shader reflection

	m_root_signature.Init(this, application_uniforms);

	if (m_used_uniforms[NGL_GROUP_PER_RENDERER_CHANGE].size() 
		+ m_used_uniforms[NGL_GROUP_PER_DRAW].size() 
		+ m_used_uniforms[NGL_GROUP_MANUAL].size()
		== 0)
	{
		_logf("Warning: No uniforms in renderer of job (%s)", job->m_descriptor.m_subpasses[job->m_current_subpass].m_name.c_str());
	}

	return true;
}


// D3D12_graphics_renderer ///////////////////////////////////////////////////////////////////////////////

D3D12_graphics_renderer::D3D12_graphics_renderer(D3D12_graphics_job *job)
	: D3D12_renderer(job)
{

}


D3D12_graphics_renderer::~D3D12_graphics_renderer()
{
	for (PSOMap::iterator i = m_psos.begin(); i != m_psos.end(); i++)
	{
		SAFE_RELEASE(i->second);
	}
	m_psos.clear();
}

bool D3D12_graphics_renderer::Init(D3D12_graphics_job *job, NGL_state &state, uint32_t num_vbos, uint32_t *vbos)
{
	bool is_init_successful = D3D12_renderer::Init(job, state);
	if (!is_init_successful)
	{
		return false;
	}

	// Extract input layout

	InputLayoutReflection(m_shader_blobs[NGL_VERTEX_SHADER], num_vbos, vbos, m_input_elements);

	return true;
}

ID3D12PipelineState *D3D12_graphics_renderer::GetPipelineState(D3D12_GRAPHICS_PIPELINE_STATE_DESC &pso_desc_stub)
{
	auto item = m_psos.find(pso_desc_stub);
	if (item != m_psos.end())
	{
		return item->second;
	}
	else
	{
		memcpy(&m_pso_desc_complete, &pso_desc_stub, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
		FillPipelineStateShaderFields(m_pso_desc_complete);

		ID3D12PipelineState *new_pso = nullptr;
		HRESULT result = m_backend->m_device->CreateGraphicsPipelineState(&m_pso_desc_complete, IID_PPV_ARGS(&new_pso));
		ThrowIfFailed(result);

#ifdef NGL_DX12_DEBUG_PSO_CREATION
		_logf("Created graphics PSO: %s", m_job->m_descriptor.m_subpasses[m_job->m_current_subpass].m_name.c_str());
#endif

		m_psos.insert(std::pair<D3D12_GRAPHICS_PIPELINE_STATE_DESC, ID3D12PipelineState*>(pso_desc_stub, new_pso));
		return new_pso;
	}
}


void D3D12_graphics_renderer::FillPipelineStateShaderFields(D3D12_GRAPHICS_PIPELINE_STATE_DESC &pso_desc)
{
	// Set root signature

	pso_desc.pRootSignature = m_root_signature.m_root_signature;

	// Set shader stages

	D3D12_SHADER_BYTECODE shaders[NGL_NUM_SHADER_TYPES];
	memset(shaders, 0, sizeof(shaders));
	for (uint32_t shader_type = NGL_VERTEX_SHADER; shader_type < NGL_NUM_SHADER_TYPES; shader_type++)
	{
		ID3DBlob *blob = m_shader_blobs[shader_type];
		if (blob != nullptr)
		{
			shaders[shader_type].pShaderBytecode = reinterpret_cast<BYTE*>(blob->GetBufferPointer());
			shaders[shader_type].BytecodeLength = blob->GetBufferSize();
		}
	}

	pso_desc.VS = shaders[NGL_VERTEX_SHADER];
	pso_desc.HS = shaders[NGL_TESS_CONTROL_SHADER];
	pso_desc.DS = shaders[NGL_TESS_EVALUATION_SHADER];
	pso_desc.GS = shaders[NGL_GEOMETRY_SHADER];
	pso_desc.PS = shaders[NGL_FRAGMENT_SHADER];
	
	// Set input layout

	pso_desc.InputLayout.NumElements = (uint32_t)m_input_elements.size();
	pso_desc.InputLayout.pInputElementDescs = m_input_elements.size() ? m_input_elements.data() : nullptr;
}


void D3D12_graphics_renderer::InputLayoutReflection(ID3DBlob *vs_shader, uint32_t num_vbos, uint32_t *vbos, std::vector<D3D12_INPUT_ELEMENT_DESC> &input_elements)
{
	// Reflect the vertex shader and query the vertex attributes
	ID3D12ShaderReflection *reflection = nullptr;
	ThrowIfFailed(D3DReflect(vs_shader->GetBufferPointer(), vs_shader->GetBufferSize(), IID_PPV_ARGS(&reflection)));

	D3D12_SHADER_DESC shader_desc;
	ThrowIfFailed(reflection->GetDesc(&shader_desc));

	input_elements.clear();

	D3D12_SIGNATURE_PARAMETER_DESC input_desc;
	for (uint32_t i = 0; i < shader_desc.InputParameters; i++)
	{
		ThrowIfFailed(reflection->GetInputParameterDesc(i, &input_desc));

		if (input_desc.SystemValueType != D3D_NAME_UNDEFINED)
		{
			// Skip system values like SV_VertexID, SV_PrimitiveID...
			continue;
		}

		// Matrix is not yet supported
		assert(input_desc.SemanticIndex == 0);

		// Check if the format is supported
		if (input_desc.ComponentType == D3D_REGISTER_COMPONENT_UNKNOWN)
		{
			_logf("DX12 - InputLayoutReflection: Unknown input parameter type: %s %d", input_desc.SemanticName, (int32_t)input_desc.ComponentType);
			assert(0);
			continue;
		}

		// Determinate the element count of the attribute
		uint32_t element_count = 0;
		BYTE mask = input_desc.Mask;
		while (mask)
		{
			if (mask & 0x01)
			{
				element_count++;
			}
			mask = mask >> 1;
		}

		assert(element_count);

		// Find the corresponing VBO and element
		D3D12_vertex_buffer *vb = nullptr;
		uint32_t attrib_index = 0;
		uint32_t buffer_index = (uint32_t)-1;
		for (uint32_t vbo_index = 0; vbo_index < num_vbos; vbo_index++)
		{
			vb = m_backend->m_resource_mgr->m_vertex_buffers[vbos[vbo_index]];

			if (SearchAttribBySemanticAndSize(vb->m_ngl_vertex_buffer.m_vertex_descriptor, attrib_index, input_desc.SemanticName, element_count))
			{
				buffer_index = vbo_index;
				break;
			}
		}

		if (buffer_index == (uint32_t)-1)
		{
			_logf("DX12 - InputLayoutReflection: Can not match vertex attrib %s with vertex input!\n", input_desc.SemanticName);
			assert(false);
			continue;
		}

		// Create and store the input element
		// NOTE: Instancing is not yet supported
		D3D12_INPUT_ELEMENT_DESC input_element_desc;
		memset(&input_element_desc, 0, sizeof(input_element_desc));
		input_element_desc.SemanticName = vb->m_ngl_vertex_buffer.m_vertex_descriptor.m_attribs[attrib_index].m_semantic.c_str();
		input_element_desc.SemanticIndex = 0;
		input_element_desc.Format = GetDXGIVertexFormat(vb->m_ngl_vertex_buffer.m_vertex_descriptor.m_attribs[attrib_index].m_format);
		input_element_desc.InputSlot = (uint32_t)buffer_index;
		input_element_desc.AlignedByteOffset = vb->m_ngl_vertex_buffer.m_vertex_descriptor.m_attribs[attrib_index].m_offset;
		input_element_desc.InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
		input_element_desc.InstanceDataStepRate = 0;
		input_elements.push_back(input_element_desc);
	}

	reflection->Release();
}


// D3D12_compute_renderer ///////////////////////////////////////////////////////////////////////////////

D3D12_compute_renderer::D3D12_compute_renderer(D3D12_compute_job *job)
	: D3D12_renderer(job)
{

}


D3D12_compute_renderer::~D3D12_compute_renderer()
{
	for (PSOMap::iterator i = m_psos.begin(); i != m_psos.end(); i++)
	{
		SAFE_RELEASE(i->second);
	}
	m_psos.clear();
}


ID3D12PipelineState* D3D12_compute_renderer::GetPipelineState(D3D12_COMPUTE_PIPELINE_STATE_DESC &pso_desc_stub)
{
	auto item = m_psos.find(pso_desc_stub);
	if (item != m_psos.end())
	{
		return item->second;
	}
	else
	{
		memcpy(&m_pso_desc_complete, &pso_desc_stub, sizeof(D3D12_COMPUTE_PIPELINE_STATE_DESC));
		FillPipelineStateShaderFields(m_pso_desc_complete);

		ID3D12PipelineState *new_pso = nullptr;
		HRESULT result = m_backend->m_device->CreateComputePipelineState(&m_pso_desc_complete, IID_PPV_ARGS(&new_pso));
		ThrowIfFailed(result);

#ifdef NGL_DX12_DEBUG_PSO_CREATION
		_logf("Created compute PSO: %s", m_job->m_descriptor.m_subpasses[m_job->m_current_subpass].m_name.c_str());
#endif
		
		m_psos.insert(std::pair<D3D12_COMPUTE_PIPELINE_STATE_DESC, ID3D12PipelineState*>(pso_desc_stub, new_pso));
		return new_pso;
	}
}


void D3D12_compute_renderer::FillPipelineStateShaderFields(D3D12_COMPUTE_PIPELINE_STATE_DESC &pso_desc)
{	
	// Set root signature

	pso_desc.pRootSignature = m_root_signature.m_root_signature;

	// Set shader stages

	D3D12_SHADER_BYTECODE shaders[NGL_NUM_SHADER_TYPES];
	memset(shaders, 0, sizeof(shaders));
	ID3DBlob *blob = m_shader_blobs[NGL_COMPUTE_SHADER];
	if (blob != nullptr)
	{
		shaders[NGL_COMPUTE_SHADER].pShaderBytecode = reinterpret_cast<BYTE*>(blob->GetBufferPointer());
		shaders[NGL_COMPUTE_SHADER].BytecodeLength = blob->GetBufferSize();
	}

	pso_desc.CS = shaders[NGL_COMPUTE_SHADER];
}
