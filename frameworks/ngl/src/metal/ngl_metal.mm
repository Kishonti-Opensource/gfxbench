/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifdef ENABLE_METAL_IMPLEMENTATION
#include "ngl_metal.h"

#include <algorithm>


#if 0
#define NGL_LOG(...) printf (__VA_ARGS__); printf("\n");
#else
#define NGL_LOG(...) ;
#endif


#define LOG_UNIFORM_UPLOAD 0

#if LOG_UNIFORM_UPLOAD
void INIT_UNIFORM_LOG();
void LOG_SET_BYTES(uint32_t group_id, uint32_t count);
void DUMP_UNIFORM_LOG();
#else
#define INIT_UNIFORM_LOG(...);
#define LOG_SET_BYTES(...);
#define DUMP_UNIFORM_LOG(...);
#endif


#define LOG_ALLOCATIONS 0

#if LOG_ALLOCATIONS
void DUMP_ALLOCATIONS();
void LOG_TEXTURE(uint32 num_data_layouts, Metal_resource_manager::_texture_data_layout *data_layouts, uint32 stride, uint32 bytes_per_image, bool compressed);
void LOG_BUFFER(Metal_resource_manager::_vertex_layout vertex_layout, uint32 num);
#else
#define DUMP_ALLOCATIONS() ;
#define LOG_TEXTURE(...) ;
#define LOG_BUFFER(...) ;
#endif


const uint32_t MAX_UNIFORM_BYTES = 4096;


void reorder_attachments(std::vector<NGL_attachment_descriptor*> &v)
{
	std::vector<NGL_attachment_descriptor*> t = v;
	v.clear();
	
	NGL_attachment_descriptor* depth_attachment = nullptr;
	
	for (size_t i = 0; i < t.size(); i++)
	{
		if (t[i] == nullptr) continue;
		
		Metal_texture &tex = Metal_instance::This->m_textures[t[i]->m_attachment.m_idx];
		
		if (!tex.m_is_color)
		{
			depth_attachment = t[i];
			continue;
		}
		
		v.push_back(t[i]);
	}
	
	if (depth_attachment != nullptr)
	{
		v.push_back(depth_attachment);
	}
}


uint32_t mtl_GenJob(NGL_job_descriptor &descriptor)
{    
    Metal_job *job = new Metal_job;
    
    job->m_descriptor = descriptor;
	
	uint32_t pass_count = (Metal_instance::This->m_use_subpass)?1:(uint32_t)descriptor.m_subpasses.size();
	
	job->passDescriptors.resize(pass_count);
	job->m_backbuffer_attachment_ids.resize(pass_count);
	
	job->m_has_depth_attachment.resize(descriptor.m_subpasses.size());
	job->m_perpass_attachments.resize(descriptor.m_subpasses.size());
    
    Metal_instance::This->m_jobs.push_back(job);

	if (job->m_descriptor.m_is_compute)
	{
		return (uint32_t)Metal_instance::This->m_jobs.size() - 1;
	}
	
	job->viewport.znear   = 0.0;
	job->viewport.zfar    = 1.0;
	
	if (job->m_descriptor.m_attachments.size() > 0)
	{
		NGL_attachment_descriptor *atd = &job->m_descriptor.m_attachments[0];
		Metal_texture* t = &Metal_instance::This->m_textures[atd->m_attachment.m_idx];
		
		job->m_current_state.m_viewport[0] = 0;
		job->m_current_state.m_viewport[1] = 0;
		job->m_current_state.m_viewport[2] = t->m_texture_descriptor.m_size[0] / (1 << atd->m_attachment.m_level);
		job->m_current_state.m_viewport[3] = t->m_texture_descriptor.m_size[1] / (1 << atd->m_attachment.m_level);
		
		job->m_current_state.m_scissor[0] = 0;
		job->m_current_state.m_scissor[1] = 0;
		job->m_current_state.m_scissor[2] = job->m_current_state.m_viewport[2];
		job->m_current_state.m_scissor[3] = job->m_current_state.m_viewport[3];
	}
	else
	{
		printf("ERROR: unable to set default viewport\n");
	}
	
	
	if (Metal_instance::This->m_use_subpass)
	{
		for (uint32_t j = 0; j < descriptor.m_subpasses.size(); j++)
		{
			job->m_perpass_attachments[j].resize(descriptor.m_attachments.size());
			job->m_has_depth_attachment[j] = false;
			
			for(uint32_t i = 0; i < descriptor.m_attachments.size(); i++)
			{
				job->m_perpass_attachments[j][i] = new NGL_attachment_descriptor();
				*job->m_perpass_attachments[j][i] = job->m_descriptor.m_attachments[i];
				
				Metal_texture &t = Metal_instance::This->m_textures[job->m_descriptor.m_attachments[i].m_attachment.m_idx];
				if (!t.m_is_color)
				{
					job->m_has_depth_attachment[j] = true;
				}
			}
		}
	}
	else
	{
		std::vector<bool> loaded;
		loaded.resize(descriptor.m_attachments.size(),false);
		
		for (uint32_t j = 0; j < descriptor.m_subpasses.size(); j++)
		{
			job->m_perpass_attachments[j].resize(descriptor.m_attachments.size());
			job->m_has_depth_attachment[j] = false;
			
			for(uint32_t i = 0; i < descriptor.m_attachments.size(); i++)
			{
				bool used = false;
				used |= descriptor.m_subpasses[j].m_usages[i] == NGL_COLOR_ATTACHMENT;
				used |= descriptor.m_subpasses[j].m_usages[i] == NGL_DEPTH_ATTACHMENT;
				used |= descriptor.m_subpasses[j].m_usages[i] == NGL_READ_ONLY_DEPTH_ATTACHMENT;
				used |= descriptor.m_subpasses[j].m_usages[i] == NGL_READ_ONLY_DEPTH_ATTACHMENT_AND_SHADER_RESOURCE;
				
				if (!used)
				{
					job->m_perpass_attachments[j][i] = nullptr;
					continue;
				}
				
				job->m_perpass_attachments[j][i] = new NGL_attachment_descriptor();
				*job->m_perpass_attachments[j][i] = job->m_descriptor.m_attachments[i];
				
				if (loaded[i])
				{
					job->m_perpass_attachments[j][i]->m_attachment_load_op = NGL_LOAD_OP_LOAD;
				}
				else
				{
					loaded[i] = true;
				}
				job->m_perpass_attachments[j][i]->m_attachment_store_op = NGL_STORE_OP_DONT_CARE;
				
				
				Metal_texture &t = Metal_instance::This->m_textures[job->m_descriptor.m_attachments[i].m_attachment.m_idx];
				if (!t.m_is_color)
				{
					job->m_has_depth_attachment[j] = true;
				}
			}
		}
		
		std::vector<bool> need_store;
		need_store.resize(descriptor.m_attachments.size(), false);
		
		for(uint32_t i = 0; i < descriptor.m_attachments.size(); i++)
		{
			need_store[i] = descriptor.m_attachments[i].m_attachment_store_op == NGL_STORE_OP_STORE;
		}
		
		for (uint32_t j = (uint32_t)descriptor.m_subpasses.size()-1; true; j--)
		{
			for(uint32_t i = 0; i < descriptor.m_attachments.size(); i++)
			{
				if (job->m_perpass_attachments[j][i] != nullptr)
				{
					job->m_perpass_attachments[j][i]->m_attachment_store_op = (need_store[i])?NGL_STORE_OP_STORE:NGL_STORE_OP_DONT_CARE;
					need_store[i] = false;
				}
				
				bool ns = false;
				ns |= descriptor.m_subpasses[j].m_usages[i] == NGL_COLOR_ATTACHMENT;
				ns |= descriptor.m_subpasses[j].m_usages[i] == NGL_COLOR_ATTACHMENT_AND_PRESERVED_ATTACHMENT;
				ns |= descriptor.m_subpasses[j].m_usages[i] == NGL_COLOR_ATTACHMENT_AND_INPUT_ATTACHMENT_AND_SHADER_RESOURCE;

				ns |= descriptor.m_subpasses[j].m_usages[i] == NGL_DEPTH_ATTACHMENT;
				ns |= descriptor.m_subpasses[j].m_usages[i] == NGL_READ_ONLY_DEPTH_ATTACHMENT;
				ns |= descriptor.m_subpasses[j].m_usages[i] == NGL_READ_ONLY_DEPTH_ATTACHMENT_AND_SHADER_RESOURCE;

				if (ns)
				{
					need_store[i] = true;
				}
			}
			
			if (j == 0) break;
		}
		
		
		// debug load store operations
#if 0
		for (uint32_t j = 0; j < (uint32_t)descriptor.m_subpasses.size(); j++)
		{
			printf("%s\n", descriptor.m_subpasses[j].m_name.c_str());
			for(uint32_t i = 0; i < descriptor.m_attachments.size(); i++)
			{
				if (job->m_perpass_attachments[j][i] == nullptr)
				{
					printf("uu:");
					continue;
				}
				
				switch (job->m_perpass_attachments[j][i]->m_attachment_load_op) {
					case NGL_LOAD_OP_DONT_CARE: printf("d"); break;
					case NGL_LOAD_OP_CLEAR: printf("c"); break;
					case NGL_LOAD_OP_LOAD: printf("l"); break;
					default: assert(0); break;
				}
				
				switch (job->m_perpass_attachments[j][i]->m_attachment_store_op) {
					case NGL_STORE_OP_DONT_CARE: printf("d"); break;
					case NGL_STORE_OP_STORE: printf("s"); break;
					default: assert(0); break;
				}
				
				printf(":");
			}
			printf("\n");
		}
#endif
		
		
		for (uint32_t j = 0; j < descriptor.m_subpasses.size(); j++)
		{
			reorder_attachments(job->m_perpass_attachments[j]);
		}
	}
	
	
	for(uint32_t i = 0; i < pass_count; i++)
	{
		job->InitRenderPassDescriptor(i);
	}
  
    return (uint32_t)Metal_instance::This->m_jobs.size() - 1;
}


Metal_job::Metal_job()
: renderCommandEncoder(nil)
, computeCommandEncoder(nil)
, m_commandBuffer(nil)
, m_status(NGL_JOB_ENDED)
{
    
}


Metal_job::~Metal_job()
{
    @autoreleasepool {
        renderCommandEncoder = nil;
		computeCommandEncoder = nil;
    }
	
	for (uint32_t j = 0; j < m_descriptor.m_subpasses.size(); j++)
	{
		for(uint32_t i = 0; i < m_perpass_attachments[j].size(); i++)
		{
			delete m_perpass_attachments[j][i];
		}
	}
}


void Metal_job::InitRenderPassDescriptor(uint32_t idx)
{
	MTLRenderPassDescriptor* passDescriptor = [MTLRenderPassDescriptor renderPassDescriptor];
	m_backbuffer_attachment_ids[idx] = -1;
	
	for( size_t i=0; i < m_perpass_attachments[idx].size(); i++)
	{
		if (m_perpass_attachments[idx][i] == nullptr)
		{
			continue;
		}
		
		NGL_attachment_descriptor &atd = *m_perpass_attachments[idx][i];
		Metal_texture &t = Metal_instance::This->m_textures[atd.m_attachment.m_idx];
		
		if( !t.m_texture_descriptor.m_is_renderable)
		{
			printf("Error: attachment not renderable\n");
		}
		
		uint32_t slice = 0;
		switch (t.m_texture_descriptor.m_type)
		{
			case NGL_TEXTURE_2D: slice = 0;
				break;
				
			case NGL_TEXTURE_2D_ARRAY: slice = atd.m_attachment.m_layer;
				break;
				
			case NGL_TEXTURE_CUBE: slice = atd.m_attachment.m_face;
				break;
				
			default:
				assert(0);
				break;
		}
		
		if (t.m_is_color)
		{
			if( atd.m_attachment.m_idx > 0)
			{
				passDescriptor.colorAttachments[i].level = atd.m_attachment.m_level;
				passDescriptor.colorAttachments[i].texture = t.texture;
				passDescriptor.colorAttachments[i].slice = slice;
			}
			else
			{
				m_backbuffer_attachment_ids[idx] = (uint32_t)i;
			}
			
			passDescriptor.colorAttachments[i].loadAction = NGLLoadToMTLLoad(atd.m_attachment_load_op);
			passDescriptor.colorAttachments[i].storeAction = NGLStoreToMTLStore(atd.m_attachment_store_op);
			passDescriptor.colorAttachments[i].clearColor = MTLClearColorMake(t.m_texture_descriptor.m_clear_value[0],
																			  t.m_texture_descriptor.m_clear_value[1],
																			  t.m_texture_descriptor.m_clear_value[2],
																			  t.m_texture_descriptor.m_clear_value[3]);
		}
		else
		{
			m_has_depth_attachment[idx] = true;
			
			if( atd.m_attachment.m_idx > 0)
			{
				passDescriptor.depthAttachment.level = atd.m_attachment.m_level;
				passDescriptor.depthAttachment.texture = t.texture;
				passDescriptor.depthAttachment.slice = slice;
			}
			else
			{
				Metal_texture &system_depth = Metal_instance::This->m_textures[Metal_instance::This->system_depth];
				
				passDescriptor.depthAttachment.texture = system_depth.texture;
			}
			
			passDescriptor.depthAttachment.loadAction = NGLLoadToMTLLoad(atd.m_attachment_load_op);
			passDescriptor.depthAttachment.storeAction = NGLStoreToMTLStore(atd.m_attachment_store_op);
			passDescriptor.depthAttachment.clearDepth = t.m_texture_descriptor.m_clear_value[0];
		}
	}
	
	passDescriptors[idx] = passDescriptor;
}


void Metal_job::BeginRenderPass(uint32_t idx)
{
	if (m_backbuffer_attachment_ids[idx] >= 0)
	{
		passDescriptors[idx].colorAttachments[m_backbuffer_attachment_ids[idx]].texture = Metal_instance::This->backbuffer_provider->GetBackBuffer();
	}
	
	renderCommandEncoder = [m_commandBuffer renderCommandEncoderWithDescriptor:passDescriptors[idx]];
	
	[renderCommandEncoder setViewport:viewport] ;
	[renderCommandEncoder setFrontFacingWinding:MTLWindingCounterClockwise];
}


void mtl_Begin(uint32_t job_, uint32_t idx)
{
	@autoreleasepool {
    Metal_job *job = Metal_instance::This->m_jobs[job_];
	
	job->renderCommandEncoder = nil;
	job->m_active_renderer = nullptr;
	job->computeCommandEncoder = nil;
	job->m_current_subpass = 0;
	
	if (job->m_status != NGL_JOB_ENDED)
	{
		printf("Warning!!! Unended job started: %s\n",job->m_descriptor.m_subpasses[0].m_name.c_str());
	}
	job->m_status = NGL_JOB_STARTED;
    
	job->m_commandBuffer = Metal_instance::This->m_command_buffers[idx];
	
	assert(job->m_commandBuffer != nil);
	assert(Metal_instance::This->m_command_buffer_status[idx] == NGL_COMMAND_BUFFER_STARTED);
	
	if(job->m_descriptor.m_is_compute)
	{
		job->computeCommandEncoder = [job->m_commandBuffer computeCommandEncoder];
		return;
	}
		
	job->BeginRenderPass(job->m_current_subpass);
}
}


void mtl_End(uint32_t job_)
{
@autoreleasepool {
    Metal_job *job = Metal_instance::This->m_jobs[job_];
	
	if (job->m_status != NGL_JOB_STARTED)
	{
		printf("Warning!!! Unstarted job finished: %s\n",job->m_descriptor.m_subpasses[0].m_name.c_str());
	}
	job->m_status = NGL_JOB_ENDED;
	job->m_commandBuffer = nil;
    
	if (job->m_descriptor.m_is_compute)
	{
		[job->computeCommandEncoder endEncoding];
		job->computeCommandEncoder = nil;
		job->m_active_renderer = nullptr;
	}
    else
	{
		[job->renderCommandEncoder endEncoding];
		job->renderCommandEncoder = nil;
		job->m_active_renderer = nullptr;
	}
}
}


void mtl_BindUniformGroup(Metal_renderer* renderer, uint32_t group_id, id <MTLCommandEncoder> commandEncoder, const void **parameters)
{
    const std::vector<NGL_used_uniform> *used_uniforms = renderer->m_used_uniforms;
    
	uint8_t buffer[2][MAX_UNIFORM_BYTES];
	uint32_t per_stage_memory_usage[] = { 0, 0 };
	
	memset(buffer[0], 0, MAX_UNIFORM_BYTES);
	memset(buffer[1], 0, MAX_UNIFORM_BYTES);
	
	for( size_t i=0; i<used_uniforms[group_id].size(); i++)
	{
		const NGL_used_uniform &uu = used_uniforms[group_id][i];
		
		if (uu.m_application_location <= -1)
		{
			continue;
		}
		
		const void* ptr = parameters[uu.m_application_location];
		
		uint32_t format_size = 0;
		
		switch( uu.m_uniform.m_format)
		{
			case NGL_FLOAT:   format_size = 1 * 4; break;
			case NGL_FLOAT2:  format_size = 2 * 4; break;
			case NGL_FLOAT4:  format_size = 4 * 4; break;
			case NGL_FLOAT16: format_size = 16 * 4; break;
				
			case NGL_INT:  format_size = 1 * 4; break;
			case NGL_INT2: format_size = 2 * 4; break;
			case NGL_INT4: format_size = 4 * 4; break;
				
            case NGL_UINT:  format_size = 1 * 4; break;
            case NGL_UINT2: format_size = 2 * 4; break;
            case NGL_UINT4: format_size = 4 * 4; break;
				
			case NGL_TEXTURE:
			case NGL_TEXTURE_SUBRESOURCE:
			{
				uint32_t texture_id = 0;
				if (uu.m_uniform.m_format == NGL_TEXTURE)
				{
					texture_id = *(uint32_t*)ptr;
				}
				else
				{
					texture_id = ((NGL_texture_subresource*)ptr)->m_idx;
				}
				
				Metal_texture &texture = Metal_instance::This->m_textures[texture_id];
				
				if( uu.m_shader_location[NGL_VERTEX_SHADER] > -1)
				{
					[ (id <MTLRenderCommandEncoder>)commandEncoder setVertexTexture:texture.texture atIndex:uu.m_shader_location[NGL_VERTEX_SHADER]];
					[ (id <MTLRenderCommandEncoder>)commandEncoder setVertexSamplerState:texture.sampler atIndex:uu.m_shader_location[NGL_VERTEX_SHADER]];
				}
				if( uu.m_shader_location[NGL_FRAGMENT_SHADER] > -1)
				{
					[ (id <MTLRenderCommandEncoder>)commandEncoder setFragmentTexture:texture.texture atIndex:uu.m_shader_location[NGL_FRAGMENT_SHADER]];
					if (texture.sampler != nil)
					{
						[ (id <MTLRenderCommandEncoder>)commandEncoder setFragmentSamplerState:texture.sampler atIndex:uu.m_shader_location[NGL_FRAGMENT_SHADER]];
					}
				}
				if( uu.m_shader_location[NGL_COMPUTE_SHADER] > -1)
				{
					[ (id <MTLComputeCommandEncoder>)commandEncoder setTexture:texture.texture atIndex:uu.m_shader_location[NGL_COMPUTE_SHADER]];
					if (texture.sampler != nil)
					{
						[ (id <MTLComputeCommandEncoder>)commandEncoder setSamplerState:texture.sampler atIndex:uu.m_shader_location[NGL_COMPUTE_SHADER]];
					}
				}
				break;
			}
				
			case NGL_BUFFER_SUBRESOURCE:
			case NGL_BUFFER:
			{
				uint32_t offset = 0;
				uint32_t buffer_id = 0;
				
				if (uu.m_uniform.m_format == NGL_BUFFER_SUBRESOURCE)
				{
					NGL_buffer_subresource* buffer_subresource = (NGL_buffer_subresource*)ptr;
					
					buffer_id = buffer_subresource->m_buffer;
					offset = buffer_subresource->m_offset;
				}
				else
				{
					buffer_id = *(uint32_t*)ptr;
				}
				
				Metal_vertex_buffer &buffer = Metal_instance::This->m_vertex_buffers[buffer_id];
				
				if( uu.m_shader_location[NGL_VERTEX_SHADER] > -1)
				{
					[ (id <MTLRenderCommandEncoder>)commandEncoder setVertexBuffer:buffer.buffer offset:offset atIndex:uu.m_shader_location[NGL_VERTEX_SHADER]];
				}
				if( uu.m_shader_location[NGL_FRAGMENT_SHADER] > -1)
				{
					[ (id <MTLRenderCommandEncoder>)commandEncoder setFragmentBuffer:buffer.buffer offset:offset atIndex:uu.m_shader_location[NGL_FRAGMENT_SHADER]];
				}
				if( uu.m_shader_location[NGL_COMPUTE_SHADER] > -1)
				{
					[ (id <MTLComputeCommandEncoder>)commandEncoder setBuffer:buffer.buffer offset:offset atIndex:uu.m_shader_location[NGL_COMPUTE_SHADER]];
				}
				break;
			}
			default:
			{
				assert(0);
				printf("WARNING!! Try to upload unhandled uniform format!\n") ;
			}
		}
		
		if (format_size == 0)
		{
			continue;
		}
		
		NGL_shader_type stages[] = { NGL_VERTEX_SHADER, NGL_FRAGMENT_SHADER, NGL_COMPUTE_SHADER };
			
		for (uint32_t i = 0; i < 3; i++)
		{
			NGL_shader_type stage = stages[i];
				
			if(uu.m_shader_location[stage] <= -1) continue;
				
			uint32_t buffer_id = i % 2;
			
			uint32_t max_usage = uu.m_shader_location[stage] + format_size * uu.m_uniform.m_size;
			
			if (max_usage <= MAX_UNIFORM_BYTES)
			{
				memcpy( buffer[buffer_id] + uu.m_shader_location[stage], ptr, format_size * uu.m_uniform.m_size);
				per_stage_memory_usage[buffer_id] = std::max(per_stage_memory_usage[buffer_id],max_usage);
			}
			else
			{
				printf("ERROR: to many per stage uniform data\n");
				assert(0);
			}
		}
	}
	
	const int BUFFER_PADDING = 16;
	for (int i = 0; i < 2; i++)
	{
		if (per_stage_memory_usage[i] % BUFFER_PADDING != 0)
		{
			per_stage_memory_usage[i] = (per_stage_memory_usage[i]/BUFFER_PADDING+1)*BUFFER_PADDING;
		}
	}
	
	if( renderer->m_has_ubo[NGL_VERTEX_SHADER][group_id] )
	{
		[(id <MTLRenderCommandEncoder>)commandEncoder setVertexBytes:buffer[NGL_VERTEX_SHADER]
										   length:per_stage_memory_usage[NGL_VERTEX_SHADER]
										  atIndex:group_id+1] ;
        
        LOG_SET_BYTES(group_id, per_stage_memory_usage[NGL_VERTEX_SHADER]);
	}
	if( renderer->m_has_ubo[NGL_FRAGMENT_SHADER][group_id] )
	{
		[(id <MTLRenderCommandEncoder>)commandEncoder setFragmentBytes:buffer[NGL_FRAGMENT_SHADER]
											 length:per_stage_memory_usage[NGL_FRAGMENT_SHADER]
											atIndex:group_id+1] ;
        
        LOG_SET_BYTES(group_id, per_stage_memory_usage[NGL_FRAGMENT_SHADER]);
	}
	if( renderer->m_has_ubo[NGL_COMPUTE_SHADER][group_id] )
	{
		[(id <MTLComputeCommandEncoder>)commandEncoder setBytes:buffer[0]
									  length:per_stage_memory_usage[0]
									 atIndex:group_id+1] ;
        
        LOG_SET_BYTES(group_id, per_stage_memory_usage[0]);
	}
}


void mtl_BindUniforms(Metal_renderer* renderer, uint32_t change_mask, id <MTLCommandEncoder> commandEncoder, const void **parameters)
{
    for (int i = 0; i < NGL_METAL_NUM_UNIFORM_GROUPS; i++)
    {
        if ( (change_mask & (1 << i)) != 0)
        {
            mtl_BindUniformGroup(renderer, i, commandEncoder, parameters);
        }
    }
}


bool ComparePipeLine(const NGL_state &st1, const NGL_state &st2)
{
	if (st1.m_shader.m_shader_code != st2.m_shader.m_shader_code) return false;
	if (st1.m_shader.m_vbo_hash != st2.m_shader.m_vbo_hash) return false;
	if (memcmp(st1.m_blend_state.m_funcs, st2.m_blend_state.m_funcs, sizeof(NGL_blend_func) * 8) != 0) return false;
	if (memcmp(st1.m_blend_state.m_masks, st2.m_blend_state.m_masks, sizeof(NGL_color_channel_mask) * 8) != 0) return false;
	return true;
}


void mtl_Draw(uint32_t job_, NGL_primitive_type primitive_type, uint32_t shader_code, uint32_t num_vbos, uint32_t *vbos, uint32_t ebo, NGL_cull_mode cull_type, const void** parameters)
{
    Metal_job *job = Metal_instance::This->m_jobs[job_];
    
	job->m_current_state.m_cull_mode = cull_type;
	job->m_current_state.m_primitive_type = primitive_type;
	job->m_current_state.m_shader.m_shader_code = shader_code;
	job->m_current_state.m_shader.m_vbo_hash = 0;
    for (uint32_t i = 0; i < num_vbos; i++)
    {
        job->m_current_state.m_shader.m_vbo_hash += Metal_instance::This->m_vertex_buffers[vbos[i]].m_hash;
    }
	
	uint32_t changed_mask = NGL_state::ChangedMask(job->m_current_state, job->m_previous_state);
    uint32_t uniform_change_mask = NGL_GROUP_PER_DRAW;
	
	job->m_previous_state = job->m_current_state;
	
	
	// setup pipeline
	{
		bool pipeline_changed = changed_mask & (NGL_SHADER_MASK | NGL_COLOR_MASKS_MASK | NGL_COLOR_BLEND_FUNCS_MASK);
		pipeline_changed |= (job->m_active_renderer == nullptr);
		
		if (pipeline_changed)
		{
			NGL_renderer *ngl_renderer = nullptr;
			
			for (size_t j = 0; j < job->m_renderers.size(); j++)
			{
				if (ComparePipeLine( job->m_renderers[j]->m_my_state, job->m_current_state))
				{
					ngl_renderer = job->m_renderers[j];
					break;
				}
			}
			
			if (ngl_renderer == nullptr)
			{
				NGL_LOG("create render pipeline");
				ngl_renderer = job->CreateRenderer(job->m_current_state, num_vbos, vbos);
			}
			
			job->m_active_renderer = ngl_renderer;
            
            uniform_change_mask |= NGL_GROUP_MANUAL | NGL_GROUP_PER_RENDERER_CHANGE;
		}
	}
	
	Metal_renderer* renderer = (Metal_renderer*)(job->m_active_renderer);
	[job->renderCommandEncoder setRenderPipelineState:renderer->m_render_pipeline];
	
	// setup depth stencil state
	if (job->m_has_depth_attachment[job->m_current_subpass])
	{
		MTLCompareFunction depth_compare_key;
		
		bool depth_mask = job->m_current_state.m_depth_state.m_mask;
		
		job->viewport.znear   = 0.0;
		job->viewport.zfar    = 1.0;
		
		switch( job->m_current_state.m_depth_state.m_func)
		{
			case NGL_DEPTH_DISABLED:
			{
				depth_compare_key = MTLCompareFunctionAlways;
				depth_mask = false;
				break;
			}
			case NGL_DEPTH_LESS:
			{
				depth_compare_key = MTLCompareFunctionLess;
				break;
			}
			case NGL_DEPTH_LEQUAL:
			{
				depth_compare_key = MTLCompareFunctionLessEqual;
				break;
			}
			case NGL_DEPTH_EQUAL:
			{
				depth_compare_key = MTLCompareFunctionEqual;
				break;
			}
			case NGL_DEPTH_GREATER:
			{
				depth_compare_key = MTLCompareFunctionGreater;
				break;
			}
			case NGL_DEPTH_TO_FAR:
			{
				depth_compare_key = MTLCompareFunctionLessEqual;
				
				job->viewport.znear   = 1.0;
				job->viewport.zfar    = 1.0;
				break;
			}
			case NGL_DEPTH_LESS_WITH_OFFSET:
			{
				depth_compare_key = MTLCompareFunctionLess;
				
				//
				//	note:
				//  GL: glPolygonOffset(x, y):
				//  metal: [renderEncoder setDepthBiad:y slopScale:x clamp:0]
				//
				
				[job->renderCommandEncoder setDepthBias:200 slopeScale:1 clamp:0];
				break;
			}
			case NGL_DEPTH_ALWAYS:
			{
				depth_compare_key = MTLCompareFunctionAlways;
				break;
			}
			default:
			{
				assert(0);
			}
		}
		
		auto it = job->m_depth_stencil_states[depth_mask].find(depth_compare_key);
		
		id<MTLDepthStencilState> depthStencilState = nil;
		
		if (it == job->m_depth_stencil_states[depth_mask].end())
		{
			MTLDepthStencilDescriptor *depthStencilDescriptor = [[MTLDepthStencilDescriptor alloc] init];

			depthStencilDescriptor.depthCompareFunction = depth_compare_key;
			depthStencilDescriptor.depthWriteEnabled = depth_mask;
			
			depthStencilState = [Metal_instance::This->device newDepthStencilStateWithDescriptor:depthStencilDescriptor];
			
			job->m_depth_stencil_states[depth_mask][depth_compare_key] = depthStencilState;
		}
		else
		{
			depthStencilState = it->second;
		}
		
		[job->renderCommandEncoder setDepthStencilState:depthStencilState];
	}
	
	
	// setup cull type
    switch(cull_type)
    {
        case NGL_FRONT_SIDED:
        {
            [job->renderCommandEncoder setCullMode:MTLCullModeBack];
            break;
        }
        case NGL_TWO_SIDED:
        {
            [job->renderCommandEncoder setCullMode:MTLCullModeNone];
            break;
        }
        case NGL_BACK_SIDED:
        {
            [job->renderCommandEncoder setCullMode:MTLCullModeFront];
            break;
        }
    }
    
	
	// setup viewport
	job->viewport.originX = job->m_current_state.m_viewport[0] ;
	job->viewport.originY = job->m_current_state.m_viewport[1] ;
	job->viewport.width   = job->m_current_state.m_viewport[2] ;
	job->viewport.height  = job->m_current_state.m_viewport[3] ;
	
	[job->renderCommandEncoder setViewport:job->viewport];
	
	
    MTLPrimitiveType mtl_primitive_type = MTLPrimitiveTypeTriangle;
    switch(primitive_type)
    {
        case NGL_LINES:     mtl_primitive_type = MTLPrimitiveTypeLine;     break;
        case NGL_POINTS:    mtl_primitive_type = MTLPrimitiveTypePoint;    break;
        case NGL_TRIANGLES: mtl_primitive_type = MTLPrimitiveTypeTriangle; break;
        default: assert(0); break ;
    }
   
	mtl_BindUniforms(renderer, uniform_change_mask, job->renderCommandEncoder, parameters);
   
	assert(num_vbos <= 1);
	
    for (uint32_t j = 0; j < num_vbos; j++)
    {
        Metal_vertex_buffer &vb = Metal_instance::This->m_vertex_buffers[vbos[j]];
        [job->renderCommandEncoder setVertexBuffer:vb.buffer offset:0 atIndex:j];
    }
    
    Metal_index_buffer &ib = Metal_instance::This->m_index_buffers[ebo];
    
    [job->renderCommandEncoder
     drawIndexedPrimitives:mtl_primitive_type
     indexCount:ib.m_num_indices
     indexType:ib.m_Metal_data_type
     indexBuffer:ib.buffer
     indexBufferOffset:0];
}


bool mtl_Dispatch(uint32_t job_, uint32_t shader_code, uint32_t x, uint32_t y, uint32_t z, const void** parameters)
{
	Metal_job *job = Metal_instance::This->m_jobs[job_];
	
	job->m_current_state.m_shader.m_shader_code = shader_code;
	job->m_current_state.m_shader.m_vbo_hash = 0;
	
	uint32_t changed_mask = NGL_state::ChangedMask(job->m_current_state, job->m_previous_state);
	
	job->m_previous_state = job->m_current_state;
    
    uint32_t uniform_change_mask = NGL_GROUP_PER_DRAW;
	
	bool pipeline_changed = (changed_mask & NGL_SHADER_MASK) || (job->m_active_renderer == nullptr);
	if (pipeline_changed)
	{
		NGL_renderer *ngl_renderer = nullptr;
		for (size_t j = 0; j < job->m_renderers.size(); j++)
		{
			if (memcmp(&job->m_renderers[j]->m_my_state.m_shader, &job->m_current_state.m_shader, sizeof(uint32_t) * 2) == 0)
			{
				ngl_renderer = job->m_renderers[j];
				break;
			}
		}
		
		if (ngl_renderer == nullptr)
		{
			NGL_LOG("create compute pipeline");
			ngl_renderer = job->CreateRenderer(job->m_current_state, 0, 0);
		}
		
		job->m_active_renderer = ngl_renderer;
		
		if(ngl_renderer == nullptr)
		{
			return false;
		}
        
        uniform_change_mask |= NGL_GROUP_MANUAL | NGL_GROUP_PER_RENDERER_CHANGE;
	}
	
    @autoreleasepool {
		Metal_renderer* renderer = (Metal_renderer*)(job->m_active_renderer);
		
		if ([renderer->m_compute_pipeline maxTotalThreadsPerThreadgroup] < renderer->threads_per_workgroup)
		{
			return false;
		}
		
		[job->computeCommandEncoder setComputePipelineState:renderer->m_compute_pipeline] ;
		
		mtl_BindUniforms(renderer, uniform_change_mask, job->computeCommandEncoder, parameters);

        MTLSize threadsPerGroup = { renderer->m_work_group_size[0], renderer->m_work_group_size[1], renderer->m_work_group_size[2] };
        MTLSize numThreadgroups = { x, y, z};
        
        [job->computeCommandEncoder dispatchThreadgroups:numThreadgroups
                        threadsPerThreadgroup:threadsPerGroup] ;
    }
	
	return true;
}


void mtl_BeginCommandBuffer(uint32_t idx)
{
@autoreleasepool {
	Metal_instance* m = Metal_instance::This;
	if (m->m_command_buffer_status[idx] != NGL_COMMAND_BUFFER_SUBMITTED)
	{
		printf("Error: unsubmitted commandbuffer started\n");
	}
	
	m->m_command_buffers[idx] = [m->commandQueue commandBuffer];
	m->m_command_buffer_status[idx] = NGL_COMMAND_BUFFER_STARTED;
}
}


void mtl_EndCommandBuffer(uint32_t idx)
{
	Metal_instance* m = Metal_instance::This;
	if (m->m_command_buffer_status[idx] != NGL_COMMAND_BUFFER_STARTED)
	{
		printf("Error: unstarted commandbuffer ended\n");
	}
	m->m_command_buffer_status[idx] = NGL_COMMAND_BUFFER_ENDED;
	
#if 0
	[Metal_instance::This->commandBuffer addScheduledHandler:^(id <MTLCommandBuffer> schelduledCommandBuffer)
	 {
		 if ([schelduledCommandBuffer error] != nil)
		 {
			 NSLog(@"%@\n",[schelduledCommandBuffer error]);
		 }
	 }];
	
	[Metal_instance::This->commandBuffer addCompletedHandler:^(id <MTLCommandBuffer> completedCommandBuffer)
	 {
		 if ([completedCommandBuffer error] != nil)
		 {
			 NSLog(@"%@\n",[completedCommandBuffer error]);
		 }
	 }];
#endif
}


void mtl_SubmitCommandBuffer(uint32_t idx)
{
@autoreleasepool {
	Metal_instance* m = Metal_instance::This;
	if (m->m_command_buffer_status[idx] != NGL_COMMAND_BUFFER_ENDED)
	{
		printf("Error: unended commandbuffer submitted\n");
	}

	[m->m_command_buffers[idx] commit];
	m->m_command_buffer_status[idx] = NGL_COMMAND_BUFFER_SUBMITTED;
	
#if 0
	id <MTLCommandBuffer> cb = m->m_command_buffers[idx];

	[cb waitUntilScheduled];
	if ([cb error] != nil)
	{
		NSLog(@"%@\n",[cb error]); assert(0);
	}
	
	[cb waitUntilCompleted];
	if ([cb error] != nil)
	{
		NSLog(@"%@\n",[cb error]); assert(0);
	}
#endif
		
	m->m_command_buffers[idx] = nil ;
}
}


void mtl_Barrier(uint32_t cmd_buffer, std::vector<NGL_texture_subresource_transition> &texture_barriers, std::vector<NGL_buffer_transition> &buffer_barriers)
{
}


int32_t mtl_GetIntegerColorFormat()
{
    MTLPixelFormat p = Metal_instance::This->backbuffer_provider->GetBackBufferPixelFormat();
    
    switch (p)
    {
        case MTLPixelFormatRGBA8Unorm_sRGB:
        case MTLPixelFormatBGRA8Unorm_sRGB:
        case MTLPixelFormatBGRA8Unorm:
        case MTLPixelFormatRGBA8Unorm: return 8888;
            
        default:
            break;
    }
    
    return -1;
}


int32_t mtl_GetInteger(NGL_backend_property prop)
{
	switch (prop)
	{
		case NGL_RASTERIZATION_CONTROL_MODE: return NGL_ORIGIN_UPPER_LEFT_AND_NDC_FLIP;
		case NGL_DEPTH_MODE: return NGL_ZERO_TO_ONE;
		case NGL_NEED_SWAPBUFFERS: return 1;
		case NGL_SUBPASS_ENABLED: return Metal_instance::This->m_use_subpass?1:0;
		case NGL_D24_LINEAR_SHADOW_FILTER: return 1;
        case NGL_SWAPCHAIN_COLOR_FORMAT: return mtl_GetIntegerColorFormat();
        case NGL_SWAPCHAIN_DEPTH_FORMAT: return 0;
#if TARGET_OS_IPHONE
		case NGL_API: return NGL_METAL_IOS;
		case NGL_TEXTURE_COMPRESSION_ETC2: return 1;
        case NGL_TEXTURE_COMPRESSION_ASTC: return [Metal_instance::This->device supportsFeatureSet:MTLFeatureSet_iOS_GPUFamily2_v1] ? 1 : 0;
		case NGL_TEXTURE_COMPRESSION_DXT5: return 0;
			
		case NGL_MAX_COMPUTE_WORK_GROUP_SIZE_X:
		case NGL_MAX_COMPUTE_WORK_GROUP_SIZE_Y:
		case NGL_MAX_COMPUTE_WORK_GROUP_SIZE_Z:
		case NGL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS:
			return 512;
			
		case NGL_MAX_COMPUTE_SHARED_MEMORY_SIZE:
			return 16*1024;
#else
#if defined(__arm64__)    // Apple Silicon Mac interface presents itself as ios device, NGL_METAL_MACOS reserved for intel based macs
    case NGL_API: return NGL_METAL_IOS;
#else
    case NGL_API: return NGL_METAL_MACOS;
#endif
      
		case NGL_TEXTURE_COMPRESSION_ETC2: return 0;
        case NGL_TEXTURE_COMPRESSION_ASTC:
            if (@available(macOS 10.15, *)) {
                    return [Metal_instance::This->device supportsFamily:MTLGPUFamilyApple5] ? 1 : 0;
            } else {
                return 0;
            }
		case NGL_TEXTURE_COMPRESSION_DXT5: return 1;
			
		case NGL_MAX_COMPUTE_WORK_GROUP_SIZE_X:
		case NGL_MAX_COMPUTE_WORK_GROUP_SIZE_Y:
		case NGL_MAX_COMPUTE_WORK_GROUP_SIZE_Z:
		case NGL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS:
			return 1024;
			
		case NGL_MAX_COMPUTE_SHARED_MEMORY_SIZE:
			return 32*1024;
#endif
		default:
		{
			assert(0);
			return false;
		}
	}
}


std::string mtl_GetStringColorFormat()
{
    switch (Metal_instance::This->backbuffer_provider->GetBackBufferPixelFormat())
    {
        case MTLPixelFormatRGBA8Unorm_sRGB: return "unorm_srgba";
        case MTLPixelFormatRGBA8Unorm: return "unorm_rgba";
            
        case MTLPixelFormatBGRA8Unorm_sRGB: return "unorm_sbgra";
        case MTLPixelFormatBGRA8Unorm: return "unorm_bgra";
            
        default:
            break;
    }
    
    return "";
}


const char* mtl_GetString(NGL_backend_property prop)
{
    static std::map<NGL_backend_property, std::string> sprops;
    
	switch (prop) {
        case NGL_VENDOR:                 sprops[prop] = "Apple"; break;
        case NGL_RENDERER:               sprops[prop] = [Metal_instance::This->device.name UTF8String]; break;
        case NGL_SWAPCHAIN_COLOR_FORMAT: sprops[prop] = mtl_GetStringColorFormat(); break;
        case NGL_SWAPCHAIN_DEPTH_FORMAT: sprops[prop] = ""; break;
		
		default:
		{
			assert(0);
			return NULL;
		}
	}
    
    return sprops.at(prop).c_str();
}


void mtl_DepthState(uint32_t job_, NGL_depth_func func, bool mask)
{
	Metal_job *job = Metal_instance::This->m_jobs[job_];
	
	job->m_current_state.m_depth_state.m_func = func;
	job->m_current_state.m_depth_state.m_mask = mask;
}


void mtl_BlendState(uint32_t job_, uint32_t attachment, NGL_blend_func func, NGL_color_channel_mask mask)
{
	Metal_job *job = Metal_instance::This->m_jobs[job_];
	
	job->m_current_state.m_blend_state.m_funcs[attachment] = func;
	job->m_current_state.m_blend_state.m_masks[attachment] = mask;
}


void mtl_ViewportScissor(uint32_t job_, int32_t viewport[4], int32_t scissor[4])
{
	Metal_job *job = Metal_instance::This->m_jobs[job_];
	
	if (viewport)
	{
		memcpy(job->m_current_state.m_viewport, viewport, sizeof(int32_t) * 4);
	}
	if (scissor)
	{
		memcpy(job->m_current_state.m_scissor, scissor, sizeof(int32_t) * 4);
	}
}


void mtl_DeletePipelines(uint32_t job_)
{
	Metal_job *job = Metal_instance::This->m_jobs[job_];
	
	job->DeleteRenderers();
}


void mtl_CustomAction(uint32_t job_, uint32_t parameter)
{
	
}


void mtl_Flush()
{
	//NOP
}


void mtl_Finish()
{
	id <MTLCommandBuffer> commandBuffer = [Metal_instance::This->commandQueue commandBuffer];
	[commandBuffer commit];
	[commandBuffer waitUntilCompleted];
	commandBuffer = nil;
}


void mtl_NextSubpass(uint32_t job_)
{
	Metal_job *job = Metal_instance::This->m_jobs[job_];
	job->m_current_subpass++;

	if (Metal_instance::This->m_use_subpass)
	{
#if !TARGET_OS_IPHONE && !defined(__arm64__)
		[job->renderCommandEncoder textureBarrier];
#endif
	}
	else
	{
		[job->renderCommandEncoder endEncoding];
		job->BeginRenderPass(job->m_current_subpass);
	}
}


Metal_instance *Metal_instance::This = 0;


void mtl_DestroyContext()
{
	delete Metal_instance::This;
	Metal_instance::This = nullptr;
}


Metal_instance::Metal_instance()
{
	nglGenJob = mtl_GenJob;
    nglBegin = mtl_Begin;
    nglEnd = mtl_End;
    nglDraw = mtl_Draw;
    nglDispatch = mtl_Dispatch;
    nglGenTexture = mtl_GenTexture;
    nglGenVertexBuffer = mtl_GenVertexBuffer;
    nglGenIndexBuffer = mtl_GenIndexBuffer;
	nglDepthState = mtl_DepthState;
	nglViewportScissor = mtl_ViewportScissor;
	nglGetInteger = mtl_GetInteger;
	nglGetString = mtl_GetString;
	nglBlendState = mtl_BlendState;
	nglCustomAction = mtl_CustomAction;
	nglDeletePipelines = mtl_DeletePipelines;
	nglFlush = mtl_Flush;
	nglFinish = mtl_Finish;
	nglDestroyContext = mtl_DestroyContext;
	nglNextSubpass = mtl_NextSubpass;
	nglBeginCommandBuffer = mtl_BeginCommandBuffer;
	nglEndCommandBuffer = mtl_EndCommandBuffer;
	nglSubmitCommandBuffer = mtl_SubmitCommandBuffer;
	nglBarrier = mtl_Barrier;
	nglGetTextureContent = mtl_GetTextureContent;
	nglGetVertexBufferContent = mtl_GetVertexBufferContent;
	
	for (uint32_t i = 0; i < MAX_COMMAND_BUFFER_COUNT; i++)
	{
		m_command_buffer_status[i] = NGL_COMMAND_BUFFER_SUBMITTED;
	}
	
	m_use_subpass = true;
    
    INIT_UNIFORM_LOG();
}


Metal_instance::~Metal_instance()
{
	DUMP_ALLOCATIONS();
    DUMP_UNIFORM_LOG();
	
	mtl_Finish();
	
	for (size_t i = 0; i < m_jobs.size(); i++)
	{
		delete m_jobs[i];
	}
	m_jobs.clear();
	
	delete backbuffer_provider;
	backbuffer_provider = nullptr;
}


void Metal_instance::Init(NGL_metal_adapter_interface* metal_adapter, uint32_t width, uint32_t height)
{
	{
		//a nullas attachment invalid, mert a m_format == 0
		Metal_texture t;
		t.m_is_color = true;
		t.pixel_format = metal_adapter->GetBackBufferPixelFormat();
		t.m_texture_descriptor.m_size[0] = width;
		t.m_texture_descriptor.m_size[1] = height;
		t.m_texture_descriptor.m_is_renderable = true;
		
		m_textures.push_back( t);
	}
	
	{
		// default nil system depth
		Metal_texture t;
		t.m_is_color = false;
		t.pixel_format = MTLPixelFormatInvalid;
		m_textures.push_back( t);
		t.m_texture_descriptor.m_size[0] = width;
		t.m_texture_descriptor.m_size[1] = height;
		t.m_texture_descriptor.m_is_renderable = true;
		
		system_depth = (uint32_t)m_textures.size()-1;
	}
	
    device = metal_adapter->device;
    commandQueue = metal_adapter->commandQueue;
    backbuffer_provider = metal_adapter;
    if (metal_adapter->system_depth != 0)
    {
        system_depth = metal_adapter->system_depth;
    }
	
	{
		m_use_subpass = true;

#if TARGET_OS_IPHONE
    m_use_subpass = [device supportsFeatureSet:MTLFeatureSet_iOS_GPUFamily2_v1];
#else
    if (@available(macOS 10.15, *)) {
      m_use_subpass =  [Metal_instance::This->device supportsFamily:MTLGPUFamilyApple5] ? 1 : 0;
    } else {
      m_use_subpass = metal_adapter->macos_use_subpass;
    }
#endif
	
		printf("Using subpass: %s\n",(m_use_subpass)?"true":"false");
	}
}


void nglCreateContextMetal(NGL_context_descriptor &descriptor)
{
	assert(Metal_instance::This == nullptr);
	Metal_instance::This = new Metal_instance;
    
    Metal_instance::This->m_context_descriptor = descriptor;
    
    uint32_t width = descriptor.m_display_width;
    uint32_t height = descriptor.m_display_height;
    NGL_metal_adapter_interface* p_metal_adapter = (NGL_metal_adapter_interface*)(descriptor.m_user_data)[0];
    
    Metal_instance::This->Init(p_metal_adapter, width, height);
}


#if LOG_ALLOCATIONS

unsigned long g_render_targets = 0;
unsigned long g_textures = 0;
unsigned long g_compressed_textures = 0;
unsigned long g_buffers = 0;

void DUMP_ALLOCATIONS()
{
    unsigned long total = g_render_targets + g_textures + g_compressed_textures + g_buffers;

    printf("\n\n");
    printf("render targets: %fMb\n",static_cast<double>(g_render_targets     )/(1024.0*1024.0));
    printf("textures:       %fMb\n",static_cast<double>(g_textures           )/(1024.0*1024.0));
    printf("compressed:     %fMb\n",static_cast<double>(g_compressed_textures)/(1024.0*1024.0));
    printf("buffers:        %fMb\n",static_cast<double>(g_buffers            )/(1024.0*1024.0));
    printf("\n");
    printf("total:          %fMb\n",static_cast<double>(total                )/(1024.0*1024.0));
    printf("\n\n");
}

void LOG_TEXTURE(uint32 num_data_layouts, Metal_resource_manager::_texture_data_layout *data_layouts, uint32 stride, uint32 bytes_per_image, bool compressed)
{
    // its just a heuristic
    bool render_target = (num_data_layouts == 1) && (data_layouts[0].m_data == nullptr) ;
    if (render_target)
    {
        g_render_targets += data_layouts[0].m_width*data_layouts[0].m_height*stride;
    }
    else
    {
        if (compressed)
        {
            g_compressed_textures += bytes_per_image;
        }
        else
        {
            g_textures += bytes_per_image;
        }
    }
}

void LOG_BUFFER(Metal_resource_manager::_vertex_layout vertex_layout, uint32 num)
{
    g_buffers += vertex_layout.m_stride*num;
}

#endif


#endif


#if LOG_UNIFORM_UPLOAD

std::map<uint32_t, uint32_t> g_uniform_amount;

void LOG_SET_BYTES(uint32_t group_id, uint32_t count)
{
    g_uniform_amount[group_id] += count;
}

void INIT_UNIFORM_LOG()
{
    for (int i = 0; i < NGL_METAL_NUM_UNIFORM_GROUPS; i++) g_uniform_amount[i] = 0;
}

void DUMP_UNIFORM_LOG()
{
    printf("\n\nUniform upload:\n");
    uint32_t total = 0;
    for (int i = 0; i < NGL_METAL_NUM_UNIFORM_GROUPS; i++)
    {
        uint32_t t = g_uniform_amount[i];
        printf("  %d. group: %d bytes (%f kbytes, %f mbytes)\n", i, t, (float)t/1024.0f, (float)t/(1024.0f*1024.0f));
        total += t;
    }
    printf("  total: %d bytes (%f kbytes, %f mbytes)\n", total, (float)total/1024.0f, (float)total/(1024.0f*1024.0f));
    printf("\n\n");
}

#else
#define LogUniformUpload(...);
#endif
