/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "ngl_metal.h"


static uint32_t NGLGroupToUniformIntefaceId(NGL_shader_uniform_group ngl_group)
{
    switch (ngl_group)
    {
        case NGL_GROUP_PER_DRAW: return 0;
        case NGL_GROUP_PER_RENDERER_CHANGE: return 1;
        case NGL_GROUP_MANUAL: return 2;
        default:
            break;
    }
    
    assert(0);
    return 0;
}


static bool IsUniformFormat(NGL_shader_uniform_format format)
{
    switch (format)
    {
        case NGL_FLOAT16:
        case NGL_FLOAT4:
        case NGL_FLOAT2:
        case NGL_FLOAT:
        case NGL_INT:
        case NGL_INT2:
        case NGL_INT4:
        case NGL_UINT:
        case NGL_UINT2:
        case NGL_UINT4:
            return true;
            
        case NGL_TEXTURE:
        case NGL_TEXTURE_SUBRESOURCE:
        case NGL_BUFFER:
        case NGL_BUFFER_SUBRESOURCE:
            return false;
            
        default:
            break;
    }
    
    assert(0);
    return false;
}


NGL_renderer* Metal_job::CreateRenderer(NGL_state &st, uint32_t num_vbos, uint32_t *vbos)
{
	id<MTLFunction> funcs[NGL_NUM_CLASSIC_SHADER_TYPES];
	NGL_shader_source_descriptor ssd[NGL_NUM_SHADER_TYPES];
	
	std::vector<NGL_shader_uniform> application_uniforms;
	
	m_descriptor.m_load_shader_callback(m_descriptor, m_current_subpass, st.m_shader.m_shader_code, ssd, application_uniforms);
	
	Metal_renderer *renderer = new Metal_renderer;
	renderer->m_my_state = st;
	
	
	if (m_descriptor.m_is_compute)
	{
		id <MTLFunction> m_shader = nil;
		if (ssd[NGL_COMPUTE_SHADER].m_source_data.size())
		{
			m_shader = Metal_renderer::CompileShader(Metal_instance::This->device, ssd[NGL_COMPUTE_SHADER].m_source_data.c_str(), ssd[NGL_COMPUTE_SHADER].m_entry_point.c_str());
		}
		
		if (m_shader == nil)
		{
			return nullptr;
		}
		
		renderer->m_work_group_size[0] = ssd[NGL_COMPUTE_SHADER].m_work_group_size[0];
		renderer->m_work_group_size[1] = ssd[NGL_COMPUTE_SHADER].m_work_group_size[1];
		renderer->m_work_group_size[2] = ssd[NGL_COMPUTE_SHADER].m_work_group_size[2];
		
		renderer->threads_per_workgroup = renderer->m_work_group_size[0]*renderer->m_work_group_size[1]*renderer->m_work_group_size[2];
		
		renderer->GetUsedComputeResources(Metal_instance::This->device, m_shader);
		
		NSError* error = nil;
		renderer->m_compute_pipeline = [Metal_instance::This->device newComputePipelineStateWithFunction:m_shader error:&error];
		
		if( renderer->m_compute_pipeline == nil )
		{
			printf("Compute pipeline create error: %s\n", [[error localizedDescription] UTF8String]);
			return nullptr;
		}
		
		uint32_t max_threads_per_group = (uint32_t)renderer->m_compute_pipeline.maxTotalThreadsPerThreadgroup;
		
		if (max_threads_per_group < renderer->threads_per_workgroup)
		{
			printf("Error: %s computepass: desired worgroup size (%d) exceeds the maximum (%d)\n",m_descriptor.m_subpasses[0].m_name.c_str(),renderer->threads_per_workgroup,max_threads_per_group);
		}
		
		if(renderer->m_compute_pipeline == nil)
		{
			NSLog(@"Error creating pipeline %@", [error localizedDescription]);
		}
		
		assert(renderer->m_compute_pipeline);
	}
	else
	{
		// !m_descriptor.m_is_compute
		
		
		for( uint32_t shader_type=NGL_VERTEX_SHADER; shader_type<NGL_NUM_CLASSIC_SHADER_TYPES; shader_type++)
		{
			funcs[shader_type] = nil ;
			
			if( ssd[shader_type].m_source_data.size())
			{
				funcs[shader_type] = Metal_renderer::CompileShader(Metal_instance::This->device, ssd[shader_type].m_source_data.c_str(), ssd[shader_type].m_entry_point.c_str());
			}
		}
		
		MTLRenderPipelineDescriptor *pipelineDescriptor = [[MTLRenderPipelineDescriptor alloc] init];
		
		pipelineDescriptor.vertexFunction = funcs[NGL_VERTEX_SHADER];
		pipelineDescriptor.fragmentFunction = funcs[NGL_FRAGMENT_SHADER];
		
		for( size_t i=0; i< m_perpass_attachments[m_current_subpass].size(); i++)
		{
			if (m_perpass_attachments[m_current_subpass][i] == nullptr)
			{
				continue;
			}
			
			NGL_attachment_descriptor &atd = *m_perpass_attachments[m_current_subpass][i];
			Metal_texture &t = Metal_instance::This->m_textures[atd.m_attachment.m_idx];
			
			size_t sp_id = m_current_subpass;
			NGL_resource_state usage = m_descriptor.m_subpasses[sp_id].m_usages[i];
			
			if (!t.m_is_color)
			{
				uint32_t attachment = (atd.m_attachment.m_idx > 0) ? atd.m_attachment.m_idx : Metal_instance::This->system_depth;
				Metal_texture &t = Metal_instance::This->m_textures[attachment];
				
				pipelineDescriptor.depthAttachmentPixelFormat = t.pixel_format;
				continue;
			}
			
			bool at_used = false;
			at_used |= usage == NGL_COLOR_ATTACHMENT;
			at_used |= usage == NGL_COLOR_ATTACHMENT_AND_INPUT_ATTACHMENT_AND_SHADER_RESOURCE;
			pipelineDescriptor.colorAttachments[i].pixelFormat = t.pixel_format;
			
			if (!at_used)
			{
				pipelineDescriptor.colorAttachments[i].writeMask = 0;
				pipelineDescriptor.colorAttachments[i].blendingEnabled = false;
				continue;
			}
			
			pipelineDescriptor.colorAttachments[i].writeMask = st.m_blend_state.m_masks[i];
			
			switch( st.m_blend_state.m_funcs[i])
			{
				case NGL_BLEND_DISABLED:
				{
					pipelineDescriptor.colorAttachments[i].blendingEnabled = false;
					break;
				}
				case NGL_BLEND_ADDITIVE:
				{
					pipelineDescriptor.colorAttachments[i].blendingEnabled = TRUE;
					pipelineDescriptor.colorAttachments[i].sourceRGBBlendFactor = MTLBlendFactorOne;
					pipelineDescriptor.colorAttachments[i].sourceAlphaBlendFactor = MTLBlendFactorOne;
					pipelineDescriptor.colorAttachments[i].destinationRGBBlendFactor = MTLBlendFactorOne;
					pipelineDescriptor.colorAttachments[i].destinationAlphaBlendFactor = MTLBlendFactorOne;
					break;
				}
				case NGL_BLEND_ALFA:
				{
					pipelineDescriptor.colorAttachments[i].blendingEnabled = TRUE;
					pipelineDescriptor.colorAttachments[i].sourceRGBBlendFactor = MTLBlendFactorSourceAlpha;
					pipelineDescriptor.colorAttachments[i].sourceAlphaBlendFactor = MTLBlendFactorSourceAlpha;
					pipelineDescriptor.colorAttachments[i].destinationRGBBlendFactor = MTLBlendFactorOneMinusSourceAlpha;
					pipelineDescriptor.colorAttachments[i].destinationAlphaBlendFactor = MTLBlendFactorOneMinusSourceAlpha;
					break;
				}
				case NGL_BLEND_ADDITIVE_ALFA:
				{
					pipelineDescriptor.colorAttachments[i].blendingEnabled = TRUE;
					pipelineDescriptor.colorAttachments[i].sourceRGBBlendFactor = MTLBlendFactorOne;
					pipelineDescriptor.colorAttachments[i].sourceAlphaBlendFactor = MTLBlendFactorOne;
					pipelineDescriptor.colorAttachments[i].destinationRGBBlendFactor = MTLBlendFactorSourceAlpha;
					pipelineDescriptor.colorAttachments[i].destinationAlphaBlendFactor = MTLBlendFactorSourceAlpha;
					break;
				}
                case NGL_BLEND_MODULATIVE:
                {
                    pipelineDescriptor.colorAttachments[i].blendingEnabled = TRUE;
                    pipelineDescriptor.colorAttachments[i].sourceRGBBlendFactor = MTLBlendFactorDestinationColor;
                    pipelineDescriptor.colorAttachments[i].sourceAlphaBlendFactor = MTLBlendFactorDestinationAlpha;
                    pipelineDescriptor.colorAttachments[i].destinationRGBBlendFactor = MTLBlendFactorZero;
                    pipelineDescriptor.colorAttachments[i].destinationAlphaBlendFactor = MTLBlendFactorZero;
                    break;
                }
				case NGL_BLEND_ADDITIVE_INVERSE_ALFA:
				{
					pipelineDescriptor.colorAttachments[i].blendingEnabled = TRUE;
					pipelineDescriptor.colorAttachments[i].sourceRGBBlendFactor = MTLBlendFactorOne;
					pipelineDescriptor.colorAttachments[i].sourceAlphaBlendFactor = MTLBlendFactorOne;
					pipelineDescriptor.colorAttachments[i].destinationRGBBlendFactor = MTLBlendFactorOneMinusSourceAlpha;
					pipelineDescriptor.colorAttachments[i].destinationAlphaBlendFactor = MTLBlendFactorOneMinusSourceAlpha;
					break;
				}
				case NGL_BLEND_TRANSPARENT_ACCUMULATION:
				{
					pipelineDescriptor.colorAttachments[i].blendingEnabled = TRUE;
					pipelineDescriptor.colorAttachments[i].sourceRGBBlendFactor = MTLBlendFactorOne;
					pipelineDescriptor.colorAttachments[i].sourceAlphaBlendFactor = MTLBlendFactorZero;
					pipelineDescriptor.colorAttachments[i].destinationRGBBlendFactor = MTLBlendFactorOneMinusSourceAlpha;
					pipelineDescriptor.colorAttachments[i].destinationAlphaBlendFactor = MTLBlendFactorOneMinusSourceAlpha;
					break;
				}
				default:
				{
					printf("Warning! Unhandled blend mode!!\n");
                    assert(0);
				}
			}
		}
		
		renderer->GetUsedRenderResources(pipelineDescriptor, num_vbos, vbos, ssd[NGL_VERTEX_SHADER]);
		
		renderer->m_render_pipeline = [Metal_instance::This->device newRenderPipelineStateWithDescriptor:pipelineDescriptor error:NULL];
		
		// !m_descriptor.m_is_compute
	}
	
    
    // the used uniforms collected to the zero array
    // seperate them to groups based on application uniforms
    
    std::vector<NGL_used_uniform> used_uniforms = renderer->m_used_uniforms[0];
    renderer->m_used_uniforms[0].clear();
    
	for (size_t j = 0; j<used_uniforms.size(); j++)
	{
		NGL_used_uniform &uu = used_uniforms[j];
		
		for (size_t i = 0; i < application_uniforms.size(); i++)
		{
			if (application_uniforms[i].m_name == uu.m_uniform.m_name)
			{
				uu.m_application_location = (uint32_t)i;
				
                uint32_t group_id = NGLGroupToUniformIntefaceId(application_uniforms[i].m_group);
                
                if (IsUniformFormat(uu.m_uniform.m_format))
                {
                    for( uint32_t shader_type=0; shader_type<NGL_NUM_SHADER_TYPES; shader_type++)
                    {
                        renderer->m_has_ubo[shader_type][group_id] |= uu.m_shader_location[shader_type] != -1;
                    }
                }
                
				if (application_uniforms[i].m_format == NGL_TEXTURE_SUBRESOURCE)
				{
					assert(uu.m_uniform.m_format == NGL_TEXTURE);
					uu.m_uniform.m_format = NGL_TEXTURE_SUBRESOURCE;
				}
				
				if (application_uniforms[i].m_format == NGL_BUFFER_SUBRESOURCE)
				{
					assert(uu.m_uniform.m_format == NGL_BUFFER);
					uu.m_uniform.m_format = NGL_BUFFER_SUBRESOURCE;
				}
                
                renderer->m_used_uniforms[group_id].push_back(uu);
				
				break;
			}
		}
		
		if (uu.m_application_location == -1)
		{
			_logf("not set uniform: %s in %s\n", uu.m_uniform.m_name.c_str(), m_descriptor.m_subpasses[0].m_name.c_str());
		}
	}
	
	m_renderers.push_back( renderer);
	
	return renderer;
}


Metal_renderer::Metal_renderer()
{
    for (int i = 0; i < NGL_NUM_SHADER_TYPES; i++)
    {
        for (int j = 0; j < NGL_METAL_NUM_UNIFORM_GROUPS; j++)
        {
            m_has_ubo[i][j] = false;
        }
    }
}


NGL_shader_uniform_format GetNGLTypeFromMetalType(MTLDataType type)
{
	switch(type)
	{
		case MTLDataTypeFloat:    return NGL_FLOAT;
		case MTLDataTypeFloat2:   return NGL_FLOAT2;
		case MTLDataTypeFloat4:   return NGL_FLOAT4;
		case MTLDataTypeFloat4x4: return NGL_FLOAT16;
		case MTLDataTypeInt:      return NGL_INT;
		case MTLDataTypeInt2:     return NGL_INT2;
		case MTLDataTypeInt4:     return NGL_INT4;
        case MTLDataTypeUInt:     return NGL_UINT;
        case MTLDataTypeUInt2:    return NGL_UINT2;
        case MTLDataTypeUInt4:    return NGL_UINT4;
		default:
			assert(0);
			printf("\nERROR: unimplmented reflection case\n\n");
	}
	return NGL_FLOAT;
}


//#define DUMP_ARG(...) printf (__VA_ARGS__)
#define DUMP_ARG(...) ;

void Metal_renderer::GetUsedArguments(NSArray *arguments, uint32_t shader_type)
{
	for (MTLArgument *arg in arguments)
	{
		if( !arg.active)
		{
			continue;
		}
		
		DUMP_ARG("arg.name: %s\n", [[arg name] UTF8String]);
		DUMP_ARG("arg.index: %u\n", (unsigned int)arg.index);
		
		switch( arg.type)
		{
			case MTLArgumentTypeTexture:
			{
				DUMP_ARG("arg.textureType: %u\n", (unsigned int)arg.textureType);
				
				NGL_used_uniform uu;
				
				uu.m_uniform.m_name = [[arg name] UTF8String];
				uu.m_shader_location[shader_type] = (int32_t)arg.index;
				uu.m_uniform.m_format = NGL_TEXTURE;
				uu.m_uniform.m_size = 1;
				
				m_used_uniforms[0].push_back( uu);
				break;
			}
			case MTLArgumentTypeBuffer:
			{
				DUMP_ARG("arg.bufferDataSize: %u\n", (unsigned int)arg.bufferDataSize);
				
				if (arg.index < 4)
				{
					if( arg.bufferDataType == MTLDataTypeStruct)
					{
						// Uniform buffer
						for (MTLStructMember *member in [arg bufferStructType].members)
						{
							NGL_used_uniform uu;
							
							uu.m_uniform.m_name = [[member name] UTF8String];
							uu.m_shader_location[shader_type] = (int32_t)[member offset];
							
							if ([member dataType] == MTLDataTypeArray)
							{
								MTLArrayType *arrayinfo = [member arrayType];
								
								uu.m_uniform.m_format = GetNGLTypeFromMetalType([arrayinfo elementType]);
								uu.m_uniform.m_size = (uint32_t)[arrayinfo arrayLength];
							}
							else
							{
								uu.m_uniform.m_format = GetNGLTypeFromMetalType([member dataType]);
								uu.m_uniform.m_size = 1;
							}
							
							m_used_uniforms[0].push_back( uu);
						}
					}
					else
					{
						printf("Warning!! Non struct data at uniform buffer binding point!\n") ;
					}
				}
				else
				{
					// buffer binding
					NGL_used_uniform uu;
					
					uu.m_uniform.m_name = [[arg name] UTF8String];
					uu.m_shader_location[shader_type] = (int32_t)arg.index;
					uu.m_uniform.m_format = NGL_BUFFER;
					uu.m_uniform.m_size = 1;
					
					m_used_uniforms[0].push_back( uu);;
					
					DUMP_ARG("arg.bufferDataType: %u\n", (unsigned int)arg.bufferDataType);
				}
				break;
			}
			case MTLArgumentTypeSampler:
				// sampler handled explicitly
				break;
			default:
				printf("Warning unhandled argument type!!\n");
				printf("arg.type: %u\n", (unsigned int)arg.type);
		}
	}
}


void Metal_renderer::GetUsedRenderResources(MTLRenderPipelineDescriptor *pipelineDescriptor, uint32_t num_vbos, uint32_t *vbos, const NGL_shader_source_descriptor &ssd)
{
	pipelineDescriptor.vertexDescriptor = [MTLVertexDescriptor vertexDescriptor];
	
	for( size_t j=0; j<num_vbos; j++)
	{
        Metal_vertex_buffer &vb = Metal_instance::This->m_vertex_buffers[vbos[j]];
		
		for( size_t i=0; i<vb.m_vertex_descriptor.m_attribs.size(); i++)
		{
			NGL_vertex_attrib &a = vb.m_vertex_descriptor.m_attribs[i];
			
			size_t k = 0;
			for( ; k < ssd.m_used_vertex_attribs.size(); k++)
			{
				if (a.m_semantic == ssd.m_used_vertex_attribs[k].m_semantic) break ;
			}
			
			if (k >= ssd.m_used_vertex_attribs.size())
			{
				continue;
			}
			
			MTLVertexFormat format = MTLVertexFormatFloat4;
			
			switch( a.m_format)
			{
				case NGL_R32_FLOAT:
				{
					format = MTLVertexFormatFloat;
					break;
				}
				case NGL_R32_G32_FLOAT:
				{
					format = MTLVertexFormatFloat2;
					break;
				}
				case NGL_R32_G32_B32_FLOAT:
				{
					format = MTLVertexFormatFloat3;
					break;
				}
				case NGL_R32_G32_B32_A32_FLOAT:
				{
					format = MTLVertexFormatFloat4;
					break;
				}
				default:
				{
					printf("WARNING!! Try to setup unhandled vertex attribute format!") ;
				}
			}
			pipelineDescriptor.vertexDescriptor.attributes[k].format = format;
			pipelineDescriptor.vertexDescriptor.attributes[k].bufferIndex = j;
			pipelineDescriptor.vertexDescriptor.attributes[k].offset = a.m_offset;
		}
		
		pipelineDescriptor.vertexDescriptor.layouts[j].stride = vb.m_vertex_descriptor.m_stride;
		pipelineDescriptor.vertexDescriptor.layouts[j].stepFunction = MTLVertexStepFunctionPerVertex;
	}
	
	
	NSError *error = nil;
	MTLRenderPipelineReflection* reflection = nil;
	
    [Metal_instance::This->device
	 newRenderPipelineStateWithDescriptor:pipelineDescriptor
	 options:MTLPipelineOptionArgumentInfo|MTLPipelineOptionBufferTypeInfo
	 reflection:&reflection error:&error];
	
	if (error != nil)
	{
		NSString *compiler_error_string = [error localizedDescription];
		
		const char *compiler_error_cstring = [compiler_error_string cStringUsingEncoding:NSUTF8StringEncoding];
		
		if( compiler_error_cstring)
		{
			printf( "%s", compiler_error_cstring);
		}
		
	}
	NSArray *arguments[NGL_NUM_CLASSIC_SHADER_TYPES];
	
	arguments[NGL_VERTEX_SHADER]   = [reflection vertexArguments];
	arguments[NGL_FRAGMENT_SHADER] = [reflection fragmentArguments];
	
	for( uint32_t shader_type=0; shader_type<NGL_NUM_CLASSIC_SHADER_TYPES; shader_type++)
	{
		GetUsedArguments(arguments[shader_type],shader_type);
	}
}


void Metal_renderer::GetUsedComputeResources(id<MTLDevice> device, id<MTLFunction> function)
{
	NSError *error = nil;
	MTLComputePipelineReflection* reflection = nil;
	
	[device
	 newComputePipelineStateWithFunction:function
	 options:MTLPipelineOptionArgumentInfo|MTLPipelineOptionBufferTypeInfo
	 reflection:&reflection error:&error];
	
	if (error != nil)
	{
		NSString *compiler_error_string = [error localizedDescription];
		
		const char *compiler_error_cstring = [compiler_error_string cStringUsingEncoding:NSUTF8StringEncoding];
		
		if( compiler_error_cstring)
		{
			printf( "%s", compiler_error_cstring);
		}
		
	}
	NSArray *arguments;
	
	arguments = [reflection arguments];
	
	GetUsedArguments(arguments,NGL_COMPUTE_SHADER);
}


id<MTLFunction> Metal_renderer::CompileShader( id<MTLDevice> device, const char *str, const char *ep)
{
	NSString *shader_source = [NSString stringWithCString:str encoding:NSUTF8StringEncoding];
	NSString *entry_point = [NSString stringWithCString:ep encoding:NSUTF8StringEncoding];
	
	MTLCompileOptions *compile_options = nil ;
	NSError *compile_error = nil;
	
	id<MTLFunction> function = nil;
	id<MTLLibrary> library = [device newLibraryWithSource:shader_source options:compile_options error:&compile_error];
	
	if( library == nil )
	{
		printf("Compile shader error: %s\n", [[compile_error localizedDescription] UTF8String]);
	}
	else
	{
		function = [library newFunctionWithName:entry_point];
	}
	
	return function;
}

