/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#if defined HAVE_GLES3 || defined HAVE_GLEW

#include "glb_opengl_limits.h"
#include "platform.h"
#include "opengl/ext.h"
#include "glb_discard_functions.h"
#include <ng/log.h>

using namespace GLB;

bool ContextLimits::s_initialized = false;
ContextLimits ContextLimits::s_es31_limits;
ContextLimits ContextLimits::s_es31_aep_limits;
ContextLimits ContextLimits::s_gl43_limits;

ContextLimits* ContextLimits::GetES31Limits()
{
	Init();		
	return &s_es31_limits;
}

ContextLimits* ContextLimits::GetES31_AEP_Limits()
{
	Init();	
	return &s_es31_aep_limits;
}

ContextLimits* ContextLimits::GetLimits(GraphicsContext *ctx)
{
	Init();	

	if (ctx->type() == GraphicsContext::GLES)
	{
		if (ctx->versionMajor() == 3 && ctx->versionMinor() == 1)
		{
			if (g_extension->hasExtension(GLBEXT_extension_pack_es31a))
			{
				return &s_es31_aep_limits;
			}
		}		
	}
	return &s_es31_limits;
}

KCL::KCL_Status ContextLimits::ShaderConformanceTest(GraphicsContext *ctx, KCL::uint32 program)
{	
	KCL::KCL_Status result = KCL::KCL_TESTERROR_NOERROR;
	if (!ctx)
	{
		return result;
	}
	std::stringstream sstream;

	int ctx_major_version = ctx->versionMajor();
	int ctx_minor_version = ctx->versionMinor();

	bool es_31 = g_extension->isES() && ctx_major_version == 3 && ctx_minor_version == 1;
	bool gl_43 = !g_extension->isES() && ctx_major_version == 4 && ctx_minor_version >= 3;

	if (!es_31 && !gl_43)
	{
		// Program interface query reqires gles 3.1 or gl 4.3
		return result;
	}

	bool has_aep = g_extension->hasExtension(GLBEXT_extension_pack_es31a);
	bool has_geometry_shader = (gl_43 || has_aep) ? true : g_extension->hasExtension(GLBEXT_geometry_shader);
	bool has_tesselation_shader = (gl_43 || has_aep) ? true : g_extension->hasExtension(GLBEXT_tessellation_shader);
		
	// Query the active resource counts for the program interfaces
	GLint uniform_count = 0;
	GLint uniform_block_count = 0;
	GLint atomic_counter_buffer_count = 0;
	GLint shader_storage_block_count = 0;
	glGetProgramInterfaceivProc(program, GL_UNIFORM, GL_ACTIVE_RESOURCES, &uniform_count);
	glGetProgramInterfaceivProc(program, GL_UNIFORM_BLOCK, GL_ACTIVE_RESOURCES, &uniform_block_count);
	glGetProgramInterfaceivProc(program, GL_ATOMIC_COUNTER_BUFFER, GL_ACTIVE_RESOURCES, &atomic_counter_buffer_count);
	glGetProgramInterfaceivProc(program, GL_SHADER_STORAGE_BLOCK, GL_ACTIVE_RESOURCES, &shader_storage_block_count);

	// Check the combined values
	KCL::uint32 max = s_es31_limits.m_max_combinded_resource[RES_UNIFORM_BLOCK];
	if (uniform_block_count > max)
	{
		sstream << "Too many uniform blocks in program: " << uniform_block_count << " limit is: " << max << std::endl;
		result = KCL::KCL_TESTERROR_SHADER_ERROR;
	}
	max = s_es31_limits.m_max_combinded_resource[RES_ATOMIC_COUNTER_BUFFER];
	if (atomic_counter_buffer_count > max)
	{
		sstream << "Too many atomic counter buffers in program: " << atomic_counter_buffer_count << " limit is: " << max << std::endl;
		result = KCL::KCL_TESTERROR_SHADER_ERROR;
	}
	max = s_es31_limits.m_max_combinded_resource[RES_SHADER_STORAGE_BLOCK];
	if (shader_storage_block_count > max)
	{
		sstream << "Too many storage buffers in program: " << shader_storage_block_count << " limit is: " << max << std::endl;
		result = KCL::KCL_TESTERROR_SHADER_ERROR;
	}

	// Check the compute shader
	if (HasComputeShader(program))
	{
        /*
		GLint sizes[3];
		glGetProgramiv(program, GL_COMPUTE_WORK_GROUP_SIZE, sizes);
		if (sizes[0] > m_max_compute_work_group_size_x || sizes[1] > m_max_compute_work_group_size_y || sizes[2] > m_max_compute_work_group_size_z)
		{
			sstream << "Invalid work group size: " <<
				sizes[0] << "x" << sizes[1] << "x" << sizes[2] <<
				" limit is: " << m_max_compute_work_group_size_x << "x" << m_max_compute_work_group_size_y << "x" << m_max_compute_work_group_size_z << std::endl;
			result = KCL::KCL_TESTERROR_SHADER_ERROR;
		}
		KCL::uint32 invocations = sizes[0] * sizes[1] * sizes[2];
		if (invocations > m_max_compute_work_group_invocations)
		{
			sstream << "Invalid work group invocations: " << invocations << " limit is: " << m_max_compute_work_group_invocations <<  std::endl;
			result = KCL::KCL_TESTERROR_SHADER_ERROR;
		}
        */
	}

	// Check the uniforms blocks
	InteraceQuery query_result;
	QueryInteface(program, GL_UNIFORM_BLOCK, uniform_block_count, has_tesselation_shader, has_geometry_shader, query_result);
	for (KCL::uint32 i = 0; i < uniform_block_count; i++)
	{
		if (query_result.m_buffer_sizes[i] > m_max_uniform_block_size)
		{
			sstream << "Uniform block " << GetResourceName(program, GL_UNIFORM_BLOCK, i) << " is too large: " << query_result.m_buffer_sizes[i] << " limit is: " << m_max_uniform_block_size << std::endl;
			result = KCL::KCL_TESTERROR_SHADER_ERROR;
		}
		if (query_result.m_bindings[i] > m_max_uniform_block_binding)
		{
			sstream << "Uniform block " << GetResourceName(program, GL_UNIFORM_BLOCK, i) << " binding is invalid: " << query_result.m_bindings[i] << " max is: " << m_max_uniform_block_binding << std::endl;
			result = KCL::KCL_TESTERROR_SHADER_ERROR;
		}
	}	
	CheckResourceReferences(RES_UNIFORM_BLOCK, query_result, result, sstream);

	// Check the atomic counters
	QueryInteface(program, GL_ATOMIC_COUNTER_BUFFER, atomic_counter_buffer_count, has_tesselation_shader, has_geometry_shader, query_result);
	for (KCL::uint32 i = 0; i < atomic_counter_buffer_count; i++)
	{
		// TODO: Check atomic counter buffer size

		if (query_result.m_bindings[i] > m_max_atomic_counter_binding)
		{
			sstream << "Atomic counter buffer binding is invalid: " << query_result.m_bindings[i] << " max is: " << m_max_atomic_counter_binding << std::endl;
			result = KCL::KCL_TESTERROR_SHADER_ERROR;
		}
	}
	CheckResourceReferences(RES_ATOMIC_COUNTER_BUFFER, query_result, result, sstream);

	// Check the shader storage blocks	
	QueryInteface(program, GL_SHADER_STORAGE_BLOCK, shader_storage_block_count, has_tesselation_shader, has_geometry_shader, query_result);
	for (KCL::uint32 i = 0; i < shader_storage_block_count; i++)
	{
		if (query_result.m_bindings[i] > m_max_shader_storage_block_binding)
		{
			sstream << "Shader storage block " << GetResourceName(program, GL_SHADER_STORAGE_BLOCK, i) << " binding is invalid: " << query_result.m_bindings[i] << " max is: " << m_max_shader_storage_block_binding << std::endl;
			result = KCL::KCL_TESTERROR_SHADER_ERROR;
		}
	}
	CheckResourceReferences(RES_SHADER_STORAGE_BLOCK, query_result, result, sstream);
	
	// Query the unifrom variables(images and textures)
	InteraceQuery texture_result;
	InteraceQuery image_result;
	QueryUniforms(program, uniform_count, has_tesselation_shader, has_geometry_shader, texture_result, image_result);

	// Check the texture samplers
	for (KCL::uint32 i = 0; i < texture_result.m_bindings.size(); i++)
	{
		if (texture_result.m_bindings[i] > m_max_texture_binding)
		{
            sstream << "Sampler " << GetResourceName(program, GL_UNIFORM, texture_result.m_resource_indices[i]) << " binding is invalid: " << texture_result.m_bindings[i] << " max is: " << m_max_texture_binding << std::endl;
			result = KCL::KCL_TESTERROR_SHADER_ERROR;
		}
    }
    CheckResourceReferences(RES_TEXTURE_UNIT, texture_result, result, sstream);


	// Check the image unifroms
	for (KCL::uint32 i = 0; i < image_result.m_bindings.size(); i++)
	{
		if (image_result.m_bindings[i] > m_max_image_binding)
		{
			sstream << "Image uniform " << GetResourceName(program, GL_UNIFORM,  image_result.m_resource_indices[i]) << " binding is invalid: " << image_result.m_bindings[i] << " max is: " << m_max_image_binding << std::endl;
			result = KCL::KCL_TESTERROR_SHADER_ERROR;
		}
	}
	CheckResourceReferences(RES_IMAGE_UNIFORM, image_result, result, sstream);
	
	if (result != KCL::KCL_TESTERROR_NOERROR)
	{
		NGLOG_ERROR("\nShader conformance error! Profile: %s\n%s", name, sstream.str());
	}
	return result;
}

void ContextLimits::QueryInteface(KCL::uint32 program, KCL::uint32 prg_interface, KCL::uint32 idx_count, bool has_tess_shader, bool has_geometry_shader, InteraceQuery &result)
{
	result.clear();
	if (!idx_count)
	{
		return;
	}

	std::vector<GLenum> props;
	props.push_back(GL_BUFFER_DATA_SIZE); // 0
	props.push_back(GL_BUFFER_BINDING); // 1
	props.push_back(GL_REFERENCED_BY_VERTEX_SHADER); // 2
	props.push_back(GL_REFERENCED_BY_FRAGMENT_SHADER); // 3
	props.push_back(GL_REFERENCED_BY_COMPUTE_SHADER); // 4
	// Note: geometry shader extension is required by tesselation shader extension 
	if (has_geometry_shader)
	{
		props.push_back(GL_REFERENCED_BY_GEOMETRY_SHADER); // 5
	}
	if (has_tess_shader)
	{
		props.push_back(GL_REFERENCED_BY_TESS_CONTROL_SHADER); // 6
		props.push_back(GL_REFERENCED_BY_TESS_EVALUATION_SHADER); // 7
	}	

	GLsizei result_length;
	GLint results[8];
	for (int i = 0; i < idx_count; i++)
	{
		memset(results, 0, sizeof(results));

		glGetProgramResourceivProc(program, prg_interface, i, props.size(), props.data(), 8, &result_length, results);	
        result.m_resource_indices.push_back(i);
		result.m_buffer_sizes.push_back(results[0]);
		result.m_bindings.push_back(results[1]);

		result.m_vertex_shader_ref_count += results[2];
		result.m_fragment_shader_ref_count += results[3];
		result.m_compute_shader_ref_count += results[4];
		result.m_geometry_shader_ref_count += results[5];
		result.m_tess_control_ref_count += results[6];
		result.m_tess_eval_shader_ref_count += results[7];
	}
}

void ContextLimits::QueryUniforms(KCL::uint32 program, KCL::uint32 uniform_count, bool has_tess_shader, bool has_geometry_shader, InteraceQuery &texture_result, InteraceQuery &image_result)
{
	texture_result.clear();
	image_result.clear();
	if (!uniform_count)
	{
		return;
	}

	std::vector<GLenum> props;
	props.push_back(GL_TYPE); // 0
	props.push_back(GL_LOCATION); // 1
	props.push_back(GL_REFERENCED_BY_VERTEX_SHADER); // 2
	props.push_back(GL_REFERENCED_BY_FRAGMENT_SHADER); // 3
	props.push_back(GL_REFERENCED_BY_COMPUTE_SHADER); // 4
	// Note: geometry shader extension is required by tesselation shader extension 
	if (has_geometry_shader)
	{
		props.push_back(GL_REFERENCED_BY_GEOMETRY_SHADER); // 5
	}
	if (has_tess_shader)
	{
		props.push_back(GL_REFERENCED_BY_TESS_CONTROL_SHADER); // 6
		props.push_back(GL_REFERENCED_BY_TESS_EVALUATION_SHADER); // 7
	}	

	GLsizei result_length;
	GLint results[8];
	for (int i = 0; i < uniform_count; i++)
	{
		memset(results, 0, sizeof(results));

		glGetProgramResourceivProc(program, GL_UNIFORM, i, props.size(), props.data(), 7, &result_length, results);	
		
		InteraceQuery *result = NULL;           
		if (IsImageType(results[0]))
		{
            result = &image_result;
		}
		else if (IsSamplerType(results[0]))
		{
            result = &texture_result;
		}
        else
        {
            continue;
        }

        result->m_resource_indices.push_back(i);

		GLint binding = 0;
		glGetUniformiv(program, results[1], &binding);				
		result->m_bindings.push_back(binding);

		result->m_vertex_shader_ref_count += results[2];
		result->m_fragment_shader_ref_count += results[3];
		result->m_compute_shader_ref_count += results[4];
		result->m_geometry_shader_ref_count += results[5];
		result->m_tess_control_ref_count += results[6];
		result->m_tess_eval_shader_ref_count += results[7];
	}
}

std::string ContextLimits::GetResourceName(GLuint program, KCL::uint32 prg_interface, GLuint idx)
{
	static char buffer[256];
	GLint length = 0;
	glGetProgramResourceNameProc(program, prg_interface, idx, 256, &length, buffer);
	return length ? buffer : "Unknown";	
}

void ContextLimits::CheckResourceReferences(KCL::uint32 res_type, InteraceQuery &query, KCL::KCL_Status &result, std::stringstream &sstream)
{
	if (query.m_vertex_shader_ref_count > m_vertex_shader_limits.m_max_resource[res_type])
	{
		sstream << "Vertex shader uses too many "
			<< GetResourceTypeName(res_type) << "s: " << query.m_vertex_shader_ref_count
			<< " Limit is: " << m_vertex_shader_limits.m_max_resource[res_type] << std::endl;	
		result = KCL::KCL_TESTERROR_SHADER_ERROR;
	}
	if (query.m_tess_control_ref_count > m_tess_control_shader_limits.m_max_resource[res_type])
	{
		sstream << "Tesselation control shader uses too many "
			<< GetResourceTypeName(res_type) << "s: " << query.m_tess_control_ref_count
			<< " Limit is: " << m_tess_control_shader_limits.m_max_resource[res_type] << std::endl;	
		result = KCL::KCL_TESTERROR_SHADER_ERROR;
	}
	if (query.m_tess_eval_shader_ref_count > m_tess_eval_shader_limits.m_max_resource[res_type])
	{
		sstream << "Tesselation evaluation  uses too many "
			<< GetResourceTypeName(res_type) <<"s: " << query.m_tess_eval_shader_ref_count
			<< " Limit is: " << m_tess_eval_shader_limits.m_max_resource[res_type] << std::endl;	
		result = KCL::KCL_TESTERROR_SHADER_ERROR;
	}
	if (query.m_geometry_shader_ref_count > m_geometry_shader_limits.m_max_resource[res_type])
	{
		sstream << "Geometry shader uses too many "
			<< GetResourceTypeName(res_type) <<"s: " << query.m_geometry_shader_ref_count
			<< " Limit is: " << m_geometry_shader_limits.m_max_resource[res_type] << std::endl;		
		result = KCL::KCL_TESTERROR_SHADER_ERROR;
	}
	if (query.m_fragment_shader_ref_count > m_fragment_shader_limits.m_max_resource[res_type])
	{
		sstream << "Fragment shader uses too many "
			<< GetResourceTypeName(res_type) <<"s: " << query.m_fragment_shader_ref_count
			<< " Limit is: " << m_fragment_shader_limits.m_max_resource[res_type] << std::endl;	
		result = KCL::KCL_TESTERROR_SHADER_ERROR;
	}
	if (query.m_compute_shader_ref_count > m_compute_shader_limits.m_max_resource[res_type])
	{
		sstream << "Compute shader uses too many "
			<< GetResourceTypeName(res_type) <<"s: " << query.m_compute_shader_ref_count
			<< " Limit is: " << m_compute_shader_limits.m_max_resource[res_type] << std::endl;	
		result = KCL::KCL_TESTERROR_SHADER_ERROR;
	}
}

const char * ContextLimits::GetResourceTypeName(KCL::uint32 type)
{
	switch (type)
	{
	case RES_ATOMIC_COUNTER_BUFFER:
		return "atomic counter buffer";
	case RES_IMAGE_UNIFORM:
		return "image uniform";
	case RES_SHADER_STORAGE_BLOCK:
		return "shader storage block";
	case RES_TEXTURE_UNIT:
		return "texture unit";
	case RES_UNIFORM_BLOCK:
		return "uniform block";
	default:
		return "?unknown resource type?";
	}
}

bool ContextLimits::IsSamplerType(KCL::int32 type)
{
	switch(type)
	{
	case GL_SAMPLER_2D:
	case GL_SAMPLER_3D:
	case GL_SAMPLER_CUBE:
	case GL_SAMPLER_2D_SHADOW:
	case GL_SAMPLER_2D_ARRAY:
	case GL_SAMPLER_2D_ARRAY_SHADOW:
	case GL_SAMPLER_2D_MULTISAMPLE:	
	case GL_SAMPLER_CUBE_SHADOW:
	case GL_SAMPLER_CUBE_MAP_ARRAY: // ARB_texture_cube_map_array
	case GL_SAMPLER_CUBE_MAP_ARRAY_SHADOW:

	case GL_INT_SAMPLER_2D:
	case GL_INT_SAMPLER_3D:
	case GL_INT_SAMPLER_CUBE:
	case GL_INT_SAMPLER_2D_ARRAY:
	case GL_INT_SAMPLER_2D_MULTISAMPLE:
	case GL_INT_SAMPLER_CUBE_MAP_ARRAY: // ARB_texture_cube_map_array

	case GL_UNSIGNED_INT_SAMPLER_2D:
	case GL_UNSIGNED_INT_SAMPLER_3D:
	case GL_UNSIGNED_INT_SAMPLER_CUBE:
	case GL_UNSIGNED_INT_SAMPLER_2D_ARRAY:
	case GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE:
	case GL_UNSIGNED_INT_SAMPLER_CUBE_MAP_ARRAY: // ARB_texture_cube_map_array
		return true;

	default:
		return false;
	}
}
bool ContextLimits::IsImageType(KCL::int32 type)
{
	switch (type)
	{
	case GL_IMAGE_2D:
	case GL_IMAGE_3D:
	case GL_IMAGE_CUBE:
	case GL_IMAGE_2D_ARRAY:
	case GL_IMAGE_CUBE_MAP_ARRAY: // ARB_texture_cube_map_array

	case GL_INT_IMAGE_2D:
	case GL_INT_IMAGE_3D:
	case GL_INT_IMAGE_CUBE:
	case GL_INT_IMAGE_2D_ARRAY:
	case GL_INT_IMAGE_CUBE_MAP_ARRAY: // ARB_texture_cube_map_array

	case GL_UNSIGNED_INT_IMAGE_2D:
	case GL_UNSIGNED_INT_IMAGE_3D:
	case GL_UNSIGNED_INT_IMAGE_CUBE:
	case GL_UNSIGNED_INT_IMAGE_2D_ARRAY: // ARB_texture_cube_map_array
		return true;

	default:
		return false;
	}
}

bool ContextLimits::HasComputeShader(KCL::uint32 program)
{
	GLsizei length = 0;
	GLuint shaders[16];
	glGetAttachedShaders(program, 16, &length, shaders);
	for (GLuint i = 0; i < length; i++)
	{
		GLint type = 0;
		glGetShaderiv(shaders[i], GL_SHADER_TYPE, &type);
		if (type == GL_COMPUTE_SHADER)
		{
			return true;
		}
	}
	return false;
}

void ContextLimits::Init()
{
	if (s_initialized)
	{
		return;
	}

	s_es31_limits.name = "OpenGL ES 3.1";

	s_es31_limits.m_vertex_shader_limits.m_max_resource[RES_UNIFORM_BLOCK] = 12;
	s_es31_limits.m_vertex_shader_limits.m_max_resource[RES_TEXTURE_UNIT] = 16;
	s_es31_limits.m_vertex_shader_limits.m_max_resource[RES_ATOMIC_COUNTER_BUFFER] = 0;
	s_es31_limits.m_vertex_shader_limits.m_max_resource[RES_IMAGE_UNIFORM] = 0;
	s_es31_limits.m_vertex_shader_limits.m_max_resource[RES_SHADER_STORAGE_BLOCK] = 0;

	// Only used if EXT_tessellation_shader is avaible
	s_es31_limits.m_tess_control_shader_limits.m_max_resource[RES_UNIFORM_BLOCK] = 12;
	s_es31_limits.m_tess_control_shader_limits.m_max_resource[RES_TEXTURE_UNIT] = 16;
	s_es31_limits.m_tess_control_shader_limits.m_max_resource[RES_ATOMIC_COUNTER_BUFFER] = 0;
	s_es31_limits.m_tess_control_shader_limits.m_max_resource[RES_IMAGE_UNIFORM] = 0;
	s_es31_limits.m_tess_control_shader_limits.m_max_resource[RES_SHADER_STORAGE_BLOCK] = 0;
				
	// Only used if EXT_tessellation_shader is avaible
	s_es31_limits.m_tess_eval_shader_limits.m_max_resource[RES_UNIFORM_BLOCK] = 12;
	s_es31_limits.m_tess_eval_shader_limits.m_max_resource[RES_TEXTURE_UNIT] = 16;
	s_es31_limits.m_tess_eval_shader_limits.m_max_resource[RES_ATOMIC_COUNTER_BUFFER] = 0;
	s_es31_limits.m_tess_eval_shader_limits.m_max_resource[RES_IMAGE_UNIFORM] = 0;
	s_es31_limits.m_tess_eval_shader_limits.m_max_resource[RES_SHADER_STORAGE_BLOCK] = 0;
				
	// Only used if EXT_geometry_shader is avaible
	s_es31_limits.m_geometry_shader_limits.m_max_resource[RES_UNIFORM_BLOCK] = 12;
	s_es31_limits.m_geometry_shader_limits.m_max_resource[RES_TEXTURE_UNIT] = 16;
	s_es31_limits.m_geometry_shader_limits.m_max_resource[RES_ATOMIC_COUNTER_BUFFER] = 0;
	s_es31_limits.m_geometry_shader_limits.m_max_resource[RES_IMAGE_UNIFORM] = 0;
	s_es31_limits.m_geometry_shader_limits.m_max_resource[RES_SHADER_STORAGE_BLOCK] = 0;

	s_es31_limits.m_fragment_shader_limits.m_max_resource[RES_UNIFORM_BLOCK] = 12;
	s_es31_limits.m_fragment_shader_limits.m_max_resource[RES_TEXTURE_UNIT] = 16;
	s_es31_limits.m_fragment_shader_limits.m_max_resource[RES_ATOMIC_COUNTER_BUFFER] = 0;
	s_es31_limits.m_fragment_shader_limits.m_max_resource[RES_IMAGE_UNIFORM] = 0;
	s_es31_limits.m_fragment_shader_limits.m_max_resource[RES_SHADER_STORAGE_BLOCK] = 0;

	s_es31_limits.m_compute_shader_limits.m_max_resource[RES_UNIFORM_BLOCK] = 12;
	s_es31_limits.m_compute_shader_limits.m_max_resource[RES_TEXTURE_UNIT] = 16;
	s_es31_limits.m_compute_shader_limits.m_max_resource[RES_ATOMIC_COUNTER_BUFFER] = 8;
	s_es31_limits.m_compute_shader_limits.m_max_resource[RES_IMAGE_UNIFORM] = 4;
	s_es31_limits.m_compute_shader_limits.m_max_resource[RES_SHADER_STORAGE_BLOCK] = 4;
		
	// Global limits		
	s_es31_limits.m_max_combinded_resource[RES_UNIFORM_BLOCK] = 24;
	s_es31_limits.m_max_combinded_resource[RES_TEXTURE_UNIT] = 48;
	s_es31_limits.m_max_combinded_resource[RES_ATOMIC_COUNTER_BUFFER] = 1;
	s_es31_limits.m_max_combinded_resource[RES_IMAGE_UNIFORM] = 4;
	s_es31_limits.m_max_combinded_resource[RES_SHADER_STORAGE_BLOCK] = 4;

	s_es31_limits.m_max_uniform_block_binding = 35;
	s_es31_limits.m_max_shader_storage_block_binding = 3;
	s_es31_limits.m_max_atomic_counter_binding = 0;
	s_es31_limits.m_max_texture_binding = 15;
	s_es31_limits.m_max_image_binding = 3;

	s_es31_limits.m_max_uniform_block_size = 16384;

	s_es31_limits.m_max_compute_work_group_size_x = 128;
	s_es31_limits.m_max_compute_work_group_size_y = 128;
	s_es31_limits.m_max_compute_work_group_size_z = 64;
	s_es31_limits.m_max_compute_work_group_invocations = 128;

	// ANDROID_extension_pack_es31a
	s_es31_aep_limits = s_es31_limits;

	s_es31_aep_limits.name = "OpenGL ES 3.1 + AEP";
	s_es31_aep_limits.m_fragment_shader_limits.m_max_resource[RES_ATOMIC_COUNTER_BUFFER] = 1;
	s_es31_aep_limits.m_fragment_shader_limits.m_max_resource[RES_IMAGE_UNIFORM] = 4;
	s_es31_aep_limits.m_fragment_shader_limits.m_max_resource[RES_SHADER_STORAGE_BLOCK] = 4;

	s_initialized = true;
}
#endif
