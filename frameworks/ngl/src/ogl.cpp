/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "ngl.h"
#include "ngl_internal.h"
#include "ngl_gl_adapter_interface.h"

#include "oglx/gl.h"
#include "ogl_gles31_functions.h"
#include "ogl_gles30_functions.h"

#include <map>
#include <set>
#include <sstream>
#include <cassert>

//HACK: glext macros
#ifndef GL_COMPRESSED_RGB_S3TC_DXT1_EXT
#define GL_COMPRESSED_RGB_S3TC_DXT1_EXT 0x83F0
#endif

#ifndef GL_COMPRESSED_RGBA_S3TC_DXT1_EXT
#define GL_COMPRESSED_RGBA_S3TC_DXT1_EXT 0x83F1
#endif

#ifndef GL_COMPRESSED_RGBA_S3TC_DXT3_EXT
#define GL_COMPRESSED_RGBA_S3TC_DXT3_EXT 0x83F2
#endif

#ifndef GL_COMPRESSED_RGBA_S3TC_DXT5_EXT
#define GL_COMPRESSED_RGBA_S3TC_DXT5_EXT 0x83F3
#endif


#ifndef GL_COMPRESSED_SRGB_S3TC_DXT1_EXT
#define GL_COMPRESSED_SRGB_S3TC_DXT1_EXT 0x8C4C
#endif

#ifndef GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT
#define GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT 0x8C4D
#endif


#ifndef GL_TEXTURE_MAX_ANISOTROPY_EXT
#define GL_TEXTURE_MAX_ANISOTROPY_EXT 0x84FE
#define GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT 0x84FF
#endif

#ifndef GL_DEPTH_COMPONENT32
#define GL_DEPTH_COMPONENT32 0x81A7
#endif

#ifndef GL_UNSIGNED_INT_2_10_10_10_REV
#define GL_UNSIGNED_INT_2_10_10_10_REV 0x8368
#endif


#ifndef GL_COMPRESSED_RGBA_ASTC_4x4_KHR
#define GL_COMPRESSED_RGBA_ASTC_4x4_KHR 0x93B0
#define GL_COMPRESSED_RGBA_ASTC_5x5_KHR 0x93B2
#define GL_COMPRESSED_RGBA_ASTC_6x6_KHR 0x93B4
#define GL_COMPRESSED_RGBA_ASTC_8x8_KHR 0x93B7
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_4x4_KHR 0x93D0
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x5_KHR 0x93D2
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x6_KHR 0x93D4
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x8_KHR 0x93D7
#endif


namespace OGL4
{
#include "ogl.h"


void OGL_subpass::ClearAttachments() const
{
	if (m_color_attachments.size() == 1 && m_color_attachments[0].m_attachment.m_idx == 0)
	{
		GLenum clear_flag = 0;

		if (m_color_attachments[0].m_attachment_load_op == NGL_LOAD_OP_CLEAR)
		{
			glClearColor(
				OGL_instance::This->m_textures[0].m_texture_descriptor.m_clear_value[0],
				OGL_instance::This->m_textures[0].m_texture_descriptor.m_clear_value[1],
				OGL_instance::This->m_textures[0].m_texture_descriptor.m_clear_value[2],
				OGL_instance::This->m_textures[0].m_texture_descriptor.m_clear_value[3]
				);
			clear_flag |= GL_COLOR_BUFFER_BIT;
		}

#if 0
		if (m_depth_attachment.size() && m_depth_attachment[0].m_attachment_load_op == NGL_LOAD_OP_CLEAR)
		{
			glClearDepthf(1.0f);
			clear_flag |= GL_DEPTH_BUFFER_BIT;
		}
#endif

		if (clear_flag)
		{
			glClear(clear_flag);
		}

		return;
	}

	if (m_independent_clear)
	{
		for (size_t i = 0; i < m_color_clears.size(); i++)
		{
			glClearBufferfv(GL_COLOR, m_color_clears[i].m_attachment_index, m_color_clears[i].m_value);
		}

		if (m_depth_clear.size())
		{
			glClearBufferfv(GL_DEPTH, m_depth_clear[0].m_attachment_index, m_depth_clear[0].m_value);
		}
	}
	else
	{
		GLenum clear_flag = 0;

		if (m_color_clears.size())
		{
			glClearColor(m_color_clears[0].m_value[0], m_color_clears[0].m_value[1], m_color_clears[0].m_value[2], m_color_clears[0].m_value[3]);

			clear_flag |= GL_COLOR_BUFFER_BIT;
		}

		if (m_depth_clear.size())
		{
			glClearDepthf(m_depth_clear[0].m_value[0]);

			clear_flag |= GL_DEPTH_BUFFER_BIT;
		}

		if (clear_flag)
		{
			glClear(clear_flag);
		}
	}

	if (m_load_discards.size())
	{
		glInvalidateFramebuffer(GL_FRAMEBUFFER, (uint32_t)m_load_discards.size(), m_load_discards.data());
	}
}


void OGL_subpass::DiscardAttachments() const
{
	if (m_store_discards.size())
	{
		glInvalidateFramebuffer(GL_FRAMEBUFFER, (uint32_t)m_store_discards.size(), m_store_discards.data());
	}
}


NGL_renderer* OGL_job::CreateRenderer(NGL_state &sh, uint32_t num_vbos, uint32_t *vbos)
{
	uint32_t p;
	uint32_t shaders[NGL_NUM_SHADER_TYPES];
	NGL_shader_source_descriptor ssd[NGL_NUM_SHADER_TYPES];
	std::vector<NGL_shader_uniform> application_uniforms;

	memset(shaders, 0, sizeof(shaders));

	p = glCreateProgram();

	OGL_instance::This->SetDebugLabel(GL_PROGRAM, p, m_subpasses[sh.m_subpass].m_name + "_gl_program");
	m_descriptor.m_load_shader_callback(m_descriptor, sh.m_subpass, sh.m_shader.m_shader_code, ssd, application_uniforms);

	//_logf("%s", ssd[0].m_source_data.c_str());
	//_logf("%s", ssd[1].m_source_data.c_str());

	for (uint32_t shader_type = NGL_VERTEX_SHADER; shader_type < NGL_NUM_SHADER_TYPES; shader_type++)
	{
		if (ssd[shader_type].m_source_data.length())
		{
			uint32_t GL_shader_type = 0;

			if (shader_type == NGL_VERTEX_SHADER)
			{
				GL_shader_type = GL_VERTEX_SHADER;
			}
			else if (shader_type == NGL_FRAGMENT_SHADER)
			{
				GL_shader_type = GL_FRAGMENT_SHADER;
			}
			else if (shader_type == NGL_TESS_CONTROL_SHADER)
			{
				GL_shader_type = GL_TESS_CONTROL_SHADER;
			}
			else if (shader_type == NGL_TESS_EVALUATION_SHADER)
			{
				GL_shader_type = GL_TESS_EVALUATION_SHADER;
			}
			else if (shader_type == NGL_GEOMETRY_SHADER)
			{
				GL_shader_type = GL_GEOMETRY_SHADER;
			}
			else if (shader_type == NGL_COMPUTE_SHADER)
			{
				GL_shader_type = GL_COMPUTE_SHADER;

				if (!m_descriptor.m_is_compute)
				{
					_logf("descriptor and compute shader mismatch!\n");
				}
			}
			else
			{
				_logf("Unsupported shader type\n");
			}

			shaders[shader_type] = OGL_renderer::CompileShader(GL_shader_type, ssd[shader_type].m_source_data.c_str(), ssd[shader_type].m_info_string);

			glAttachShader(p, shaders[shader_type]);
		}
	}

	OGL_renderer *renderer = new OGL_renderer;

	renderer->m_my_state = sh;
	renderer->m_barriers = 0;
	renderer->m_p = p;

	bool s = renderer->LinkShader();

	for (size_t i = 0; i < NGL_NUM_SHADER_TYPES; i++)
	{
		if (shaders[i])
		{
			glDetachShader(p, shaders[i]);
			glDeleteShader(shaders[i]);
		}
	}

	if (!s)
	{
		delete renderer;
		return NULL;
	}
	if (!m_descriptor.m_is_compute)
	{
		bool shader_matches_with_mesh = renderer->GetActiveAttribs(num_vbos, vbos);
		if (!shader_matches_with_mesh)
		{
			_logf("Error: shader-mesh mismatch in job (%s)\n", G().m_name.c_str());
		}
	}
	renderer->GetActiveResources(application_uniforms, ssd);
	
	if (!renderer->m_used_uniforms[0].size() && !renderer->m_used_uniforms[1].size() && !renderer->m_used_uniforms[1].size() && !renderer->m_used_uniforms[2].size())
	{
		_logf("Warning: no uniforms in renderer of pass %s\n", G().m_name.c_str());
	}

	m_renderers.push_back(renderer);

	return renderer;
}


OGL_renderer::OGL_renderer()
{
	m_p = 0;
	m_barriers = 0;
}


OGL_renderer::~OGL_renderer()
{
	if (m_p)
	{
		glDeleteProgram(m_p);
	}
}


uint32_t OGL_renderer::CompileShader(uint32_t shader_type, const char *str, std::string &info_string)
{
	const char *sources[1] =
	{
		str
	};
	GLint status = 0;
	uint32_t s = glCreateShader(shader_type);

	OGL_instance::This->SetDebugLabel(GL_SHADER, s, info_string);

	glShaderSource(s, 1, sources, 0);

	glCompileShader(s);

	glGetShaderiv(s, GL_COMPILE_STATUS, &status);

	if (!status)
	{
		std::stringstream sstream(str);
		std::string line;
		int counter = 1;
		while (std::getline(sstream, line))
		{
			_logf("%d: %s", counter++, line.c_str());
		}

		int len;
		std::string log;

		glGetShaderiv(s, GL_INFO_LOG_LENGTH, &len);
		log.resize(len);

		glGetShaderInfoLog(s, len, 0, &log[0]);

		_logf("shader log: %s\napp. info: %s", log.c_str(), info_string.c_str());
	}
	else
	{
		//_logf(".info(%s): compiled.\n", io.Filename());
	}

	return s;
}


bool OGL_renderer::LinkShader()
{
	glLinkProgram(m_p);

	GLint status;
	glGetProgramiv(m_p, GL_LINK_STATUS, &status);

	if (!status)
	{
		GLint len;
		std::string log;

		glGetProgramiv(m_p, GL_INFO_LOG_LENGTH, &len);
		log.resize(len);

		glGetProgramInfoLog(m_p, len, 0, &log[0]);

		_logf("%s\n", log.c_str());

		return false;
	}
	else
	{
		//_logf(".info(program): linked.\n");
	}

	return true;
}


bool OGL_renderer::GetActiveAttribs(uint32_t num_vbos, uint32_t *vbos)
{
	int32_t num_active_attributes = 0;
	std::map<uint32_t, _used_vertex_buffer> used_layouts;

	glGetProgramiv(m_p, GL_ACTIVE_ATTRIBUTES, &num_active_attributes);

	for (int32_t i = 0; i<num_active_attributes; i++)
	{
		GLuint index = i;
		GLsizei bufSize = 512;
		GLsizei length = 0;
		GLint size = 0;
		GLenum type;
		GLchar name[512];
		int32_t location;
		std::string attrib_semantic_name;
		uint32_t attrib_size = 0;

		glGetActiveAttrib(m_p, index, bufSize, &length, &size, &type, name);
		location = glGetAttribLocation(m_p, name);

		if (location == -1)
		{
			continue;
		}

		attrib_semantic_name = name;

		//WA - would it be valid anyway?
		if (attrib_semantic_name == "gl_VertexID")
		{
			continue;
		}

		switch (type)
		{
		case GL_FLOAT:
		{
			attrib_size = 1;
			break;
		}
		case GL_FLOAT_VEC2:
		{
			attrib_size = 2;
			break;
		}
		case GL_FLOAT_VEC3:
		{
			attrib_size = 3;
			break;
		}
		case GL_FLOAT_VEC4:
		{
			attrib_size = 4;
			break;
		}
		default:
		{
			_logf("Unsupported vertex attrib type: %s - %x!\n", name, type);
			continue;
		}
		}

		bool found;
		OGL_vertex_buffer *vb = NULL;
		uint32_t idx = 0;
		uint32_t buffer_idx;

		for (buffer_idx = 0; buffer_idx<num_vbos; buffer_idx++)
		{
			vb = &OGL_instance::This->m_vertex_buffers[vbos[buffer_idx]];
			found = SearchAttribBySemanticAndSize(vb->m_vertex_descriptor, idx, attrib_semantic_name, attrib_size);
			if (found)
			{
				break;
			}
		}

		if (!found)
		{
			return false;
		}

		OGL_renderer::_GL_layout l;

		l.loc = location;
		l.offset = vb->m_vertex_descriptor.m_attribs[idx].m_offset;
		switch (vb->m_vertex_descriptor.m_attribs[idx].m_format)
		{
		case NGL_R32_FLOAT:
		{
			l.size = 1;
			l.type = GL_FLOAT;
			l.normalized = 0;
			break;
		}
		case NGL_R32_G32_FLOAT:
		{
			l.size = 2;
			l.type = GL_FLOAT;
			l.normalized = 0;
			break;
		}
		case NGL_R32_G32_B32_FLOAT:
		{
			l.size = 3;
			l.type = GL_FLOAT;
			l.normalized = 0;
			break;
		}
		case NGL_R32_G32_B32_A32_FLOAT:
		{
			l.size = 4;
			l.type = GL_FLOAT;
			l.normalized = 0;
			break;
		}
		case NGL_R8_UINT:
		{
			l.size = 1;
			l.type = GL_UNSIGNED_BYTE;
			l.normalized = 0;
			break;
		}
		case NGL_R8_G8_UINT:
		{
			l.size = 2;
			l.type = GL_UNSIGNED_BYTE;
			l.normalized = 0;
			break;
		}
		case NGL_R8_G8_B8_A8_UINT:
		{
			l.size = 4;
			l.type = GL_UNSIGNED_BYTE;
			l.normalized = 0;
			break;
		}
		default:
		{
			_logf("Unhandled attrib format!\n");
		}
		}

		used_layouts[buffer_idx].m_buffer_idx = buffer_idx;
		used_layouts[buffer_idx].m_GL_layouts.push_back(l);
	}
	{
		std::map<uint32_t, _used_vertex_buffer>::iterator i = used_layouts.begin();

		while (i != used_layouts.end())
		{
			m_used_vbs.push_back(i->second);

			i++;
		}
	}

	return true;
}


static bool IsReadonlyBuffer(NGL_shader_source_descriptor ssd[NGL_NUM_SHADER_TYPES], const std::string &name)
{
	for (size_t i = 0; i < NGL_NUM_SHADER_TYPES; i++)
	{
		for (size_t j = 0; j < ssd[i].m_used_readonly_buffers.size(); j++)
		{
			if (ssd[i].m_used_readonly_buffers[j].m_name == name)
			{
				return true;
			}
		}
	}

	return false;
}


static bool IsReadonlyImage(NGL_shader_source_descriptor ssd[NGL_NUM_SHADER_TYPES], const std::string &name)
{
	for (size_t i = 0; i < NGL_NUM_SHADER_TYPES; i++)
	{
		for (size_t j = 0; j < ssd[i].m_used_readonly_images.size(); j++)
		{
			if (ssd[i].m_used_readonly_images[j].m_name == name)
			{
				return true;
			}
		}
	}

	return false;
}


void OGL_renderer::GetActiveResources(std::vector<NGL_shader_uniform> &application_uniforms, NGL_shader_source_descriptor ssd[NGL_NUM_SHADER_TYPES])
{
	int32_t num_active_resources = 0;
	GLenum programInterfaces[] =
	{
		GL_UNIFORM_BLOCK,
		GL_UNIFORM,
		GL_SHADER_STORAGE_BLOCK,
		GL_ATOMIC_COUNTER_BUFFER,
		0
	};
	uint32_t uniform_index = 0;
	uint32_t last_used_sampler_slot = 0;
	uint32_t last_used_image_slot = 0;

	for (size_t i = 0;; i++)
	{
		GLenum programInterface = programInterfaces[i];

		if (!programInterface) break;

		glGetProgramInterfaceivProc(m_p, programInterface, GL_ACTIVE_RESOURCES, &num_active_resources);

		for (int32_t i = 0; i < num_active_resources; i++)
		{
			GLsizei length = 0;
			GLint size = 0;
			GLenum type = 0;
			GLchar name[512];
			int32_t location = -1;
#if 0
			the following variables will have an effective location of - 1:
			*uniforms declared as atomic counters;
			*members of a uniform block;
			*built - in inputs, outputs, and uniforms(starting with "gl_"); and
				* inputs or outputs not declared with a "location" layout qualifier, except for vertex shader inputs and fragment shader outputs.
#endif

			if (programInterface == GL_UNIFORM)
			{
				GLenum props[512] = { GL_LOCATION, GL_TYPE, GL_ARRAY_SIZE };
				int32_t params[512];

				glGetProgramResourceNameProc(m_p, programInterface, i, 512, &length, name);

				glGetProgramResourceivProc(m_p, programInterface, i, 3, props, 512, &length, params);

				location = params[0];
				type = params[1];
				size = params[2];

				if (location == -1)
				{
					continue;
				}
			}
			else if (programInterface == GL_SHADER_STORAGE_BLOCK || programInterface == GL_UNIFORM_BLOCK || programInterface == GL_ATOMIC_COUNTER_BUFFER)
			{
				GLenum props[512] = { GL_BUFFER_BINDING };
				int32_t params[512];

				glGetProgramResourceNameProc(m_p, programInterface, i, 512, &length, name);

				glGetProgramResourceivProc(m_p, programInterface, i, 1, props, 512, &length, params);

				location = params[0];
				type = programInterface;
				size = 1;
			}
			else
			{
				_logf("Warning!! unsupported program interface!!\n");
				continue;
			}

			NGL_used_uniform uu;

			uu.m_uniform.m_name = name;
			uu.m_shader_location[0] = location;
			uu.m_uniform.m_size = size;
			uu.m_binding_type = 0;

			switch (type)
			{
			case GL_FLOAT_MAT4:
			{
				uu.m_uniform.m_format = NGL_FLOAT16;
				break;
			}
			case GL_FLOAT_VEC4:
			{
				uu.m_uniform.m_format = NGL_FLOAT4;
				break;
			}
			case GL_SAMPLER_2D_ARRAY:
			{
				uu.m_uniform.m_format = NGL_TEXTURE;
				uu.m_binding_type = GL_SAMPLER_2D_ARRAY;
				break;
			}
			case GL_SAMPLER_2D_ARRAY_SHADOW:
			{
				uu.m_uniform.m_format = NGL_TEXTURE;
				uu.m_binding_type = GL_SAMPLER_2D_ARRAY_SHADOW;
				break;
			}
			case GL_SAMPLER_2D:
			{
				uu.m_uniform.m_format = NGL_TEXTURE;
				uu.m_binding_type = GL_SAMPLER_2D;
				break;
			}
			case GL_SAMPLER_2D_SHADOW:
			{
				uu.m_uniform.m_format = NGL_TEXTURE;
				uu.m_binding_type = GL_SAMPLER_2D_SHADOW;
				break;
			}
			case GL_SAMPLER_CUBE:
			{
				uu.m_uniform.m_format = NGL_TEXTURE;
				uu.m_binding_type = GL_SAMPLER_CUBE;
				break;
			}
			case GL_SAMPLER_CUBE_SHADOW:
			{
				uu.m_uniform.m_format = NGL_TEXTURE;
				uu.m_binding_type = GL_SAMPLER_CUBE_SHADOW;
				break;
			}
			case GL_FLOAT_VEC2:
			{
				uu.m_uniform.m_format = NGL_FLOAT2;
				break;
			}
			case GL_FLOAT:
			{
				uu.m_uniform.m_format = NGL_FLOAT;
				break;
			}
			case GL_INT:
			{
				uu.m_uniform.m_format = NGL_INT;
				break;
			}
			case GL_INT_VEC2:
			{
				uu.m_uniform.m_format = NGL_INT2;
				break;
			}
			case GL_INT_VEC4:
			{
				uu.m_uniform.m_format = NGL_INT4;
				break;
			}
			case GL_UNSIGNED_INT:
			{
				uu.m_uniform.m_format = NGL_UINT;
				break;
			}
			case GL_UNSIGNED_INT_VEC2:
			{
				uu.m_uniform.m_format = NGL_UINT2;
				break;
			}
			case GL_UNSIGNED_INT_VEC4:
			{
				uu.m_uniform.m_format = NGL_UINT4;
				break;
			}
			case GL_IMAGE_2D:
			{
				uu.m_uniform.m_format = NGL_TEXTURE;
				uu.m_binding_type = GL_IMAGE_2D;
				break;
			}
			case GL_SHADER_STORAGE_BLOCK:
			{
				uu.m_uniform.m_format = NGL_BUFFER;
				uu.m_binding_type = GL_SHADER_STORAGE_BUFFER;
				break;
			}
			case GL_UNIFORM_BLOCK:
			{
				uu.m_uniform.m_format = NGL_BUFFER;
				uu.m_binding_type = GL_UNIFORM_BUFFER;
				break;
			}
			case GL_ATOMIC_COUNTER_BUFFER:
			{
				uu.m_uniform.m_format = NGL_BUFFER;
				uu.m_binding_type = GL_ATOMIC_COUNTER_BUFFER;
				break;
			}
			default:
				_logf("Warning!! unhandled reflection case: %x!\n", type);
			}
			{
				//http://stackoverflow.com/questions/10532384/how-to-remove-a-particular-substring-from-a-string
				size_t array_remark = uu.m_uniform.m_name.find("[0]");
				if (array_remark != std::string::npos)
				{
					uu.m_uniform.m_name.erase(array_remark, 3);
				}
			}

			for (size_t i = 0; i < application_uniforms.size(); i++)
			{
				if (application_uniforms[i].m_name == uu.m_uniform.m_name)
				{
					uu.m_index = uniform_index;
					uu.m_application_location = (int32_t)i;

					if (application_uniforms[i].m_format == NGL_TEXTURE_SUBRESOURCE)
					{
						uu.m_uniform.m_format = NGL_TEXTURE_SUBRESOURCE;
					}

					if (application_uniforms[i].m_format == NGL_BUFFER_SUBRESOURCE)
					{
						uu.m_uniform.m_format = NGL_BUFFER_SUBRESOURCE;
					}

					for (uint32_t g = 0; g < 4; g++)
					{
						if (application_uniforms[i].m_group & (1 << g))
						{
							m_used_uniforms[g].push_back(uu);
						}
					}

					m_used_sampler_slots.push_back(last_used_sampler_slot);
					m_used_image_slots.push_back(last_used_image_slot);
					m_used_slot_readonly.push_back(true);

					if (uu.m_binding_type == GL_SAMPLER_2D || 
						uu.m_binding_type == GL_SAMPLER_2D_ARRAY || 
						uu.m_binding_type == GL_SAMPLER_2D_ARRAY_SHADOW || 
						uu.m_binding_type == GL_SAMPLER_2D_SHADOW || 
						uu.m_binding_type == GL_SAMPLER_CUBE ||
						uu.m_binding_type == GL_SAMPLER_CUBE_SHADOW
						)
					{
						last_used_sampler_slot++;
					}
					else if (uu.m_binding_type == GL_IMAGE_2D)
					{
						m_used_slot_readonly[uu.m_index] = IsReadonlyImage(ssd, uu.m_uniform.m_name);
						last_used_image_slot++;
					}
					else if (uu.m_binding_type == GL_SHADER_STORAGE_BUFFER)
					{
						m_used_slot_readonly[uu.m_index] = IsReadonlyBuffer(ssd, uu.m_uniform.m_name);
					}

					uniform_index++;

					break;
				}
			}
		}
	}
}


void OGL_renderer::BindUniform(const NGL_used_uniform &uu, const void *ptr)
{
	switch (uu.m_uniform.m_format)
	{
	case NGL_FLOAT16:
	{
		glUniformMatrix4fv(uu.m_shader_location[0], uu.m_uniform.m_size, 0, (float*)ptr);
		break;
	}
	case NGL_FLOAT4:
	{
		glUniform4fv(uu.m_shader_location[0], uu.m_uniform.m_size, (float*)ptr);
		break;
	}
	case NGL_TEXTURE:
	case NGL_TEXTURE_SUBRESOURCE:
	{
		uint32_t texture_id;
		if (uu.m_uniform.m_format == NGL_TEXTURE_SUBRESOURCE)
		{
			NGL_texture_subresource *subresource = (NGL_texture_subresource*)ptr;
			texture_id = subresource->m_idx;
		}
		else
		{
			texture_id = *(uint32_t*)ptr;
		}

		OGL_texture &texture = OGL_instance::This->m_textures[texture_id];

		if (uu.m_binding_type == GL_SAMPLER_2D || uu.m_binding_type == GL_SAMPLER_2D_ARRAY || uu.m_binding_type == GL_SAMPLER_CUBE)
		{
			uint32_t slot = m_used_sampler_slots[uu.m_index];

			glActiveTexture(GL_TEXTURE0 + slot);
			glBindTexture(texture.m_target, texture.m_object);
			glUniform1i(uu.m_shader_location[0], slot);
			if (!texture.m_is_color && texture.m_shadow_sampler)
			{
				glTexParameteri(texture.m_target, GL_TEXTURE_COMPARE_MODE, GL_NONE);
				if (texture.m_texture_descriptor.m_shadow_filter == NGL_LINEAR)
				{
					glTexParameteri(texture.m_target, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
					glTexParameteri(texture.m_target, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
				}
				texture.m_shadow_sampler = false;
			}
		}
		else if (uu.m_binding_type == GL_SAMPLER_2D_SHADOW || uu.m_binding_type == GL_SAMPLER_2D_ARRAY_SHADOW || uu.m_binding_type == GL_SAMPLER_CUBE_SHADOW)
		{
			uint32_t slot = m_used_sampler_slots[uu.m_index];

			glActiveTexture(GL_TEXTURE0 + slot);
			glBindTexture(texture.m_target, texture.m_object);
			if (!texture.m_shadow_sampler)
			{
				glTexParameteri(texture.m_target, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
				if (texture.m_texture_descriptor.m_shadow_filter == NGL_LINEAR)
				{
					glTexParameteri(texture.m_target, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
					glTexParameteri(texture.m_target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				}
				texture.m_shadow_sampler = true;
			}
			glUniform1i(uu.m_shader_location[0], slot);
		}
		else if (uu.m_binding_type == GL_IMAGE_2D)
		{
			uint32_t slot = m_used_image_slots[uu.m_index];

			GLenum access = m_used_slot_readonly[uu.m_index] ? GL_READ_ONLY : GL_WRITE_ONLY;
			GLenum format = texture.m_GLinternalformat;

			glBindImageTextureProc(slot, texture.m_object, 0, 0, 0, access, format);

			if (access == GL_WRITE_ONLY)
			{
				m_barriers |= GL_TEXTURE_FETCH_BARRIER_BIT;
			}
		}
		break;
	}
	case NGL_FLOAT:
	{
		glUniform1fv(uu.m_shader_location[0], uu.m_uniform.m_size, (float*)ptr);
		break;
	}
	case NGL_FLOAT2:
	{
		glUniform2fv(uu.m_shader_location[0], uu.m_uniform.m_size, (float*)ptr);
		break;
	}
	case NGL_INT:
	{
		glUniform1iv(uu.m_shader_location[0], uu.m_uniform.m_size, (int*)ptr);
		break;
	}
	case NGL_INT2:
	{
		glUniform2iv(uu.m_shader_location[0], uu.m_uniform.m_size, (int*)ptr);
		break;
	}
	case NGL_INT4:
	{
		glUniform4iv(uu.m_shader_location[0], uu.m_uniform.m_size, (int*)ptr);
		break;
	}
	case NGL_UINT:
	{
		glUniform1uiv(uu.m_shader_location[0], uu.m_uniform.m_size, (uint32_t*)ptr);
		break;
	}
	case NGL_UINT2:
	{
		glUniform2uiv(uu.m_shader_location[0], uu.m_uniform.m_size, (uint32_t*)ptr);
		break;
	}
	case NGL_UINT4:
	{
		glUniform4uiv(uu.m_shader_location[0], uu.m_uniform.m_size, (uint32_t*)ptr);
		break;
	}
	case NGL_BUFFER:
	{
		OGL_vertex_buffer &v = OGL_instance::This->m_vertex_buffers[*(uint32_t*)ptr];

		glBindBufferBase(uu.m_binding_type, uu.m_shader_location[0], v.m_GL_vbo);

		if (m_used_slot_readonly[uu.m_index] == false)
		{
			m_barriers |= GL_SHADER_STORAGE_BARRIER_BIT;
			m_barriers |= GL_UNIFORM_BARRIER_BIT;
		}

		break;
	}
	case NGL_BUFFER_SUBRESOURCE:
	{
		NGL_buffer_subresource *bsr = (NGL_buffer_subresource*)ptr;
		OGL_vertex_buffer &v = OGL_instance::This->m_vertex_buffers[bsr->m_buffer];

		glBindBufferRange(uu.m_binding_type, uu.m_shader_location[0], v.m_GL_vbo, (GLintptr)bsr->m_offset, (GLsizeiptr)(bsr->m_size == (uint32_t)~0 ? v.m_datasize : bsr->m_size));

		if (m_used_slot_readonly[uu.m_index] == false)
		{
			m_barriers |= GL_SHADER_STORAGE_BARRIER_BIT;
			m_barriers |= GL_UNIFORM_BARRIER_BIT;
		}

		break;
	}
	default:
	{
		_logf("Warning!! Unhandled bind uniform!!\n");
	}
	}
}


bool GenTexture(uint32_t &buffer, NGL_texture_descriptor &texture_layout, std::vector<std::vector<uint8_t> > *datas)
{
	if (texture_layout.m_size[0] > (uint32_t)OGL_instance::This->m_propertiesi[NGL_TEXTURE_MAX_SIZE_2D] || texture_layout.m_size[1] > (uint32_t)OGL_instance::This->m_propertiesi[NGL_TEXTURE_MAX_SIZE_2D])
	{
		_logf("OGL - GenTexture: Too big texture size: %dx%d. Max supported is: %d", texture_layout.m_size[0], texture_layout.m_size[1], OGL_instance::This->m_propertiesi[NGL_TEXTURE_MAX_SIZE_2D]);
		buffer = 0xBADF00D;
		return 0;
	}
	if (texture_layout.m_is_renderable && datas)
	{
		_logf("OGL - GenTexture: Can not create renderable texture with pixel data!\n");
		buffer = 0xBADF00D;
		return 0;
	}
	if (!texture_layout.m_is_renderable && !texture_layout.m_unordered_access && !datas)
	{
		_logf("OGL - GenTexture: No pixel data!\n");
		buffer = 0xBADF00D;
		return 0;
	}

	LOGGLERROR();

	if (buffer && buffer >= OGL_instance::This->m_textures.size())
	{
		_logf("OGL - GenTexture: Illegal texture id: %s, %d!\n", texture_layout.m_name.c_str(), buffer);
		buffer = 0xBADF00D;
		return false;
	}

	if (!buffer)
	{
		OGL_texture texture;

		OGL_instance::This->m_textures.push_back(texture);
		buffer = (uint32_t)OGL_instance::This->m_textures.size() - 1;
	}

	OGL_texture &texture = OGL_instance::This->m_textures[buffer];

	texture.m_texture_descriptor = texture_layout;
	texture.m_is_color = true;

	if (texture_layout.m_type == NGL_RENDERBUFFER)
	{
		texture.m_target = GL_RENDERBUFFER;

		glGenRenderbuffers(1, &texture.m_object);

		glBindRenderbuffer(GL_RENDERBUFFER, texture.m_object);

		OGL_instance::This->SetDebugLabel(GL_RENDERBUFFER, texture.m_object, texture.m_texture_descriptor.m_name);

		switch (texture_layout.m_format)
		{
		case NGL_D16_UNORM:
		{
			if (texture.m_texture_descriptor.samples < 2)
			{
				glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, texture_layout.m_size[0], texture_layout.m_size[1]);
			}
			else
			{
				glRenderbufferStorageMultisample(GL_RENDERBUFFER, texture.m_texture_descriptor.samples, GL_DEPTH_COMPONENT16, texture_layout.m_size[0], texture_layout.m_size[1]);
			}
			texture.m_is_color = false;
			break;
		}
		case NGL_R8_G8_B8_A8_UNORM:
		{
			if (texture.m_texture_descriptor.samples < 2)
			{
				glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, texture_layout.m_size[0], texture_layout.m_size[1]);
			}
			else
			{
				glRenderbufferStorageMultisample(GL_RENDERBUFFER, texture.m_texture_descriptor.samples, GL_RGBA8, texture_layout.m_size[0], texture_layout.m_size[1]);
			}
			texture.m_is_color = true;
			break;
		}
		case NGL_D24_UNORM:
		{
			if (texture.m_texture_descriptor.samples < 2)
			{
				glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, texture_layout.m_size[0], texture_layout.m_size[1]);
			}
			else
			{
				glRenderbufferStorageMultisample(GL_RENDERBUFFER, texture.m_texture_descriptor.samples, GL_DEPTH_COMPONENT24, texture_layout.m_size[0], texture_layout.m_size[1]);
			}
			texture.m_is_color = false;
			break;
		}
		case NGL_D32_UNORM:
		{
			if (texture.m_texture_descriptor.samples < 2)
			{
				glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT32, texture_layout.m_size[0], texture_layout.m_size[1]);
			}
			else
			{
				glRenderbufferStorageMultisample(GL_RENDERBUFFER, texture.m_texture_descriptor.samples, GL_DEPTH_COMPONENT32, texture_layout.m_size[0], texture_layout.m_size[1]);
			}
			texture.m_is_color = false;
			break;
		}
		default:
		{
			_logf("Unhandled depth format\n");
		}
		}

		glBindRenderbuffer(GL_RENDERBUFFER, 0);
	}
	else
	{
		switch (texture_layout.m_type)
		{
		case NGL_TEXTURE_2D:
		{
			texture.m_target = GL_TEXTURE_2D;
			break;
		}
		case NGL_TEXTURE_2D_ARRAY:
		{
			texture.m_target = GL_TEXTURE_2D_ARRAY;
			break;
		}
		case NGL_TEXTURE_CUBE:
		{
			texture.m_target = GL_TEXTURE_CUBE_MAP;
			break;
		}
		default:
		{
			_logf("Unhandled texture type\n");
		}
		}

		glGenTextures(1, &texture.m_object);

		glBindTexture(texture.m_target, texture.m_object);

		OGL_instance::This->SetDebugLabel(GL_TEXTURE, texture.m_object, texture.m_texture_descriptor.m_name);

		bool is_compressed = false;

		switch (texture_layout.m_format)
		{
		case NGL_R8_UNORM:
		{
			texture.m_GLinternalformat = GL_R8;
			texture.m_GLformat = GL_RED;
			texture.m_GLtype = GL_UNSIGNED_BYTE;
			break;
		}
		case NGL_R8_G8_UNORM:
		{
			texture.m_GLinternalformat = GL_RG8;
			texture.m_GLformat = GL_RG;
			texture.m_GLtype = GL_UNSIGNED_BYTE;
			break;
		}
		case NGL_R8_G8_B8_UNORM:
		{
			texture.m_GLinternalformat = GL_RGB8;
			texture.m_GLformat = GL_RGB;
			texture.m_GLtype = GL_UNSIGNED_BYTE;
			break;
		}
		case NGL_R8_G8_B8_UNORM_SRGB:
		{
			texture.m_GLinternalformat = GL_SRGB8;
			texture.m_GLformat = GL_RGB;
			texture.m_GLtype = GL_UNSIGNED_BYTE;
			break;
		}
		case NGL_R8_G8_B8_A8_UNORM:
		{
			texture.m_GLinternalformat = GL_RGBA8;
			texture.m_GLformat = GL_RGBA;
			texture.m_GLtype = GL_UNSIGNED_BYTE;
			break;
		}
		case NGL_R8_G8_B8_A8_UNORM_SRGB:
		{
			texture.m_GLinternalformat = GL_SRGB8_ALPHA8;
			texture.m_GLformat = GL_RGBA;
			texture.m_GLtype = GL_UNSIGNED_BYTE;
			break;
		}
		case NGL_R10_G10_B10_A2_UNORM:
		{
			texture.m_GLinternalformat = GL_RGB10_A2;
			texture.m_GLformat = GL_RGBA;
			texture.m_GLtype = GL_UNSIGNED_INT_2_10_10_10_REV;
			break;
		}
		case NGL_R32_FLOAT:
		{
			texture.m_GLinternalformat = GL_R32F;
			texture.m_GLformat = GL_RED;
			texture.m_GLtype = GL_FLOAT;
			break;
		}
		case NGL_R16_FLOAT:
		{
			texture.m_GLinternalformat = GL_R16F;
			texture.m_GLformat = GL_RED;
			texture.m_GLtype = GL_FLOAT;
			break;
		}
		case NGL_D16_UNORM:
		{
			texture.m_GLinternalformat = GL_DEPTH_COMPONENT16;
			texture.m_GLformat = GL_DEPTH_COMPONENT;
			texture.m_GLtype = GL_UNSIGNED_SHORT;
			texture.m_is_color = false;
			break;
		}
		case NGL_D24_UNORM:
		{
			texture.m_GLinternalformat = GL_DEPTH_COMPONENT24;
			texture.m_GLformat = GL_DEPTH_COMPONENT;
			texture.m_GLtype = GL_UNSIGNED_INT;
			texture.m_is_color = false;
			break;
		}
		case NGL_D32_UNORM:
		{
			texture.m_GLinternalformat = GL_DEPTH_COMPONENT32;
			texture.m_GLformat = GL_DEPTH_COMPONENT;
			texture.m_GLtype = GL_UNSIGNED_INT;
			texture.m_is_color = false;
			break;
		}
		case NGL_R32_G32_B32_FLOAT:
		{
			texture.m_GLinternalformat = GL_RGB32F;
			texture.m_GLformat = GL_RGB;
			texture.m_GLtype = GL_FLOAT;
			break;
		}
		case NGL_R32_G32_B32_A32_FLOAT:
		{
			texture.m_GLinternalformat = GL_RGBA32F;
			texture.m_GLformat = GL_RGBA;
			texture.m_GLtype = GL_FLOAT;
			break;
		}
		case NGL_R16_G16_FLOAT:
		{
			texture.m_GLinternalformat = GL_RG16F;
			texture.m_GLformat = GL_RG;
			texture.m_GLtype = GL_FLOAT;
			break;
		}
		case NGL_R16_G16_B16_FLOAT:
		{
			texture.m_GLinternalformat = GL_RGB16F;
			texture.m_GLformat = GL_RGB;
			texture.m_GLtype = GL_FLOAT;
			break;
		}
		case NGL_R16_G16_B16_A16_FLOAT:
		{
			texture.m_GLinternalformat = GL_RGBA16F;
			texture.m_GLformat = GL_RGBA;
			texture.m_GLtype = GL_FLOAT;
			break;
		}
		case NGL_R11_B11_B10_FLOAT:
		{
			texture.m_GLinternalformat = GL_R11F_G11F_B10F;
			texture.m_GLformat = GL_RGB;
			texture.m_GLtype = GL_FLOAT;
			break;
		}

		// compressed
		case NGL_R8_G8_B8_DXT1_UNORM:
		{
			is_compressed = true;
			texture.m_GLinternalformat = GL_COMPRESSED_RGB_S3TC_DXT1_EXT;
			texture.m_GLformat = GL_COMPRESSED_RGB_S3TC_DXT1_EXT;
			break;
		}
		case NGL_R8_G8_B8_A1_DXT1_UNORM:
		{
			is_compressed = true;
			texture.m_GLinternalformat = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
			texture.m_GLformat = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
			break;
		}
		case NGL_R8_G8_B8_DXT1_UNORM_SRGB:
		{
			is_compressed = true;
			texture.m_GLinternalformat = GL_COMPRESSED_SRGB_S3TC_DXT1_EXT;
			texture.m_GLformat = GL_COMPRESSED_SRGB_S3TC_DXT1_EXT;
			break;
		}
		case NGL_R8_G8_B8_A1_DXT1_UNORM_SRGB:
		{
			is_compressed = true;
			texture.m_GLinternalformat = GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT;
			texture.m_GLformat = GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT;
			break;
		}
		case NGL_R8_G8_B8_A8_DXT5_UNORM:
		{
			is_compressed = true;
			texture.m_GLinternalformat = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
			texture.m_GLformat = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
			break;
		}
		case NGL_R8_G8_B8_A8_DXT5_UNORM_SRGB:
		{
			is_compressed = true;
			texture.m_GLinternalformat = GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT;
			texture.m_GLformat = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
			break;
		}

		case NGL_R8_G8_B8_ETC2_UNORM:
		{
			is_compressed = true;
			texture.m_GLinternalformat = GL_COMPRESSED_RGB8_ETC2;
			texture.m_GLformat = texture.m_GLinternalformat;
			break;
		}
		case  NGL_R8_G8_B8_ETC2_UNORM_SRGB:
		{
			is_compressed = true;
			texture.m_GLinternalformat = GL_COMPRESSED_SRGB8_ETC2;
			texture.m_GLformat = texture.m_GLinternalformat;
			break;
		}
		case NGL_R8_G8_B8_A1_ETC2_UNORM:
		{
			is_compressed = true;
			texture.m_GLinternalformat = GL_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2;
			texture.m_GLformat = texture.m_GLinternalformat;
			break;
		}
		case NGL_R8_G8_B8_A1_ETC2_UNORM_SRGB:
		{
			is_compressed = true;
			texture.m_GLinternalformat = GL_COMPRESSED_SRGB8_PUNCHTHROUGH_ALPHA1_ETC2;
			texture.m_GLformat = texture.m_GLinternalformat;
			break;
		}
		case NGL_R8_G8_B8_A8_ETC2_UNORM:
		{
			is_compressed = true;
			texture.m_GLinternalformat = GL_COMPRESSED_RGBA8_ETC2_EAC;
			texture.m_GLformat = texture.m_GLinternalformat;
			break;
		}
		case NGL_R8_G8_B8_A8_ETC2_UNORM_SRGB:
		{
			is_compressed = true;
			texture.m_GLinternalformat = GL_COMPRESSED_SRGB8_ALPHA8_ETC2_EAC;
			texture.m_GLformat = texture.m_GLinternalformat;
			break;
		}

#if 1
		case NGL_R8_G8_B8_A8_ASTC_4x4_UNORM:
		{
			is_compressed = true;
			texture.m_GLinternalformat = GL_COMPRESSED_RGBA_ASTC_4x4_KHR;
			texture.m_GLformat = texture.m_GLinternalformat;
			break;
		}
		case NGL_R8_G8_B8_A8_ASTC_4x4_UNORM_SRGB:
		{
			is_compressed = true;
			texture.m_GLinternalformat = GL_COMPRESSED_SRGB8_ALPHA8_ASTC_4x4_KHR;
			texture.m_GLformat = texture.m_GLinternalformat;
			break;
		}
		case NGL_R8_G8_B8_A8_ASTC_6x6_UNORM:
		{
			is_compressed = true;
			texture.m_GLinternalformat = GL_COMPRESSED_RGBA_ASTC_6x6_KHR;
			texture.m_GLformat = texture.m_GLinternalformat;
			break;
		}
		case NGL_R8_G8_B8_A8_ASTC_6x6_UNORM_SRGB:
		{
			is_compressed = true;
			texture.m_GLinternalformat = GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x6_KHR;
			texture.m_GLformat = texture.m_GLinternalformat;
			break;
		}
#endif

#if 0
		case NGL_R8_G8_B8_A8_ASTC_5x5_UNORM:
		{
			is_compressed = true;
			texture.m_GLinternalformat = GL_COMPRESSED_RGBA_ASTC_5x5_KHR;
			texture.m_GLformat = texture.m_GLinternalformat;
			break;
		}
		case NGL_R8_G8_B8_A8_ASTC_5x5_UNORM_SRGB:
		{
			is_compressed = true;
			texture.m_GLinternalformat = GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x5_KHR;
			texture.m_GLformat = texture.m_GLinternalformat;
			break;
		}

		case _texture_layout::ASTC_8x8_RGBA8888:
		{
			is_compressed = true;
			texture.m_GLinternalformat = GL_COMPRESSED_RGBA_ASTC_8x8_KHR;
			texture.m_GLformat = texture.m_GLinternalformat;
			break;
		}
		case _texture_layout::ASTC_8x8_SRGB888_ALPHA8:
		{
			is_compressed = true;
			texture.m_GLinternalformat = GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x8_KHR;
			texture.m_GLformat = texture.m_GLinternalformat;
			break;
		}
#endif
		case NGL_R9_G9_B9_E5_SHAREDEXP:
		{
			texture.m_GLinternalformat = GL_RGB9_E5;
			texture.m_GLformat = GL_RGB;
			texture.m_GLtype = GL_UNSIGNED_INT_5_9_9_9_REV;
			break;
		}
		case NGL_R8_G8_B8_A8_UINT:
		{
			texture.m_GLinternalformat = GL_RGBA8UI;
			texture.m_GLformat = GL_RGBA;
			texture.m_GLtype = GL_UNSIGNED_BYTE;
			break;
		}

		default:
		{
			_logf("ERROR: unhandled texture format!!\n");
		}
		}

		texture.m_shadow_sampler = false;

		switch (texture_layout.m_filter)
		{
		case NGL_NEAREST:
		{
			glTexParameteri(texture.m_target, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(texture.m_target, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			break;
		}
		case NGL_LINEAR:
		{
			glTexParameteri(texture.m_target, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(texture.m_target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			break;
		}
		case NGL_NEAREST_MIPMAPPED:
		{
			glTexParameteri(texture.m_target, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
			glTexParameteri(texture.m_target, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			break;
		}
		case NGL_LINEAR_MIPMAPPED:
		{
			glTexParameteri(texture.m_target, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glTexParameteri(texture.m_target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			break;
		}
		case NGL_ANISO_4:
		{
			glTexParameteri(texture.m_target, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glTexParameteri(texture.m_target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(texture.m_target, GL_TEXTURE_MAX_ANISOTROPY_EXT, 4);
			break;
		}
		}

		switch (texture_layout.m_wrap_mode)
		{
		case NGL_REPEAT_ALL:
		{
			glTexParameteri(texture.m_target, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(texture.m_target, GL_TEXTURE_WRAP_T, GL_REPEAT);
#ifdef GL_TEXTURE_WRAP_R
			glTexParameteri(texture.m_target, GL_TEXTURE_WRAP_R, GL_REPEAT);
#endif
			break;
		}
		case NGL_CLAMP_ALL:
		{
			glTexParameteri(texture.m_target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(texture.m_target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
#ifdef GL_TEXTURE_WRAP_R
			glTexParameteri(texture.m_target, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
#endif
			break;
		}
		}

		size_t max_uploaded_level = 0;

		if (texture_layout.m_type == NGL_TEXTURE_2D)
		{
			uint32_t w = texture_layout.m_size[0];
			uint32_t h = texture_layout.m_size[1];
			glTexStorage2D(GL_TEXTURE_2D, texture_layout.m_num_levels, texture.m_GLinternalformat, texture_layout.m_size[0], texture_layout.m_size[1]);
			if (datas)
			{
				for (size_t i = 0; i<datas->size(); i++)
				{
					// Calculate the dimensions of the mipmap
					uint32_t mipmap_width = w / (1 << i);
					uint32_t mipmap_height = h / (1 << i);
					mipmap_width = mipmap_width ? mipmap_width : 1;
					mipmap_height = mipmap_height ? mipmap_height : 1;

					if (is_compressed)
					{
						glCompressedTexSubImage2D(GL_TEXTURE_2D, (GLint)i, 0, 0, mipmap_width, mipmap_height, texture.m_GLinternalformat, (GLsizei)(*datas)[i].size(), &(*datas)[i][0]);
					}
					else
					{
						glTexSubImage2D(GL_TEXTURE_2D, (GLint)i, 0, 0, mipmap_width, mipmap_height, texture.m_GLformat, texture.m_GLtype, &(*datas)[i][0]);
					}
				}

				max_uploaded_level = datas->size();
			}
		}
		if (texture_layout.m_type == NGL_TEXTURE_2D_ARRAY)
		{
			uint32_t w = texture_layout.m_size[0];
			uint32_t h = texture_layout.m_size[1];

			glTexStorage3D(GL_TEXTURE_2D_ARRAY, texture_layout.m_num_levels, texture.m_GLinternalformat, w, h, texture_layout.m_num_array);
			if (datas)
			{
				for (size_t i = 0; i < datas->size(); i++)
				{
					size_t level = i / texture_layout.m_num_array;
					size_t surface = i % texture_layout.m_num_array;

					// Calculate the dimensions of the mipmap
					uint32_t mipmap_width = w / (1 << level);
					uint32_t mipmap_height = h / (1 << level);
					mipmap_width = mipmap_width ? mipmap_width : 1;
					mipmap_height = mipmap_height ? mipmap_height : 1;

					if (is_compressed)
					{
						glCompressedTexSubImage3D(GL_TEXTURE_2D_ARRAY, (GLint)level, 0, 0, (GLint)surface, mipmap_width, mipmap_height, 1, texture.m_GLinternalformat, (GLsizei)(*datas)[i].size(), &(*datas)[i][0]);
					}
					else
					{
						glTexSubImage3D(GL_TEXTURE_2D_ARRAY, (GLint)level, 0, 0, (GLint)surface, mipmap_width, mipmap_height, 1, texture.m_GLformat, texture.m_GLtype, &(*datas)[i][0]);
					}

					max_uploaded_level = level > max_uploaded_level ? level : max_uploaded_level;
				}
			}
		}
		else if (texture_layout.m_type == NGL_TEXTURE_CUBE)
		{
			uint32_t w = texture_layout.m_size[0];
			uint32_t h = texture_layout.m_size[1];

			glTexStorage2D(GL_TEXTURE_CUBE_MAP, texture_layout.m_num_levels, texture.m_GLinternalformat, w, h);

			if (datas)
			{
				for (size_t i = 0; i < datas->size(); i++)
				{
					size_t level = i / 6;
					size_t face = i % 6;

					// Calculate the dimensions of the mipmap
					uint32_t mipmap_width = w / (1 << level);
					uint32_t mipmap_height = h / (1 << level);
					mipmap_width = mipmap_width ? mipmap_width : 1;
					mipmap_height = mipmap_height ? mipmap_height : 1;

					if (is_compressed)
					{
						glCompressedTexSubImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + (GLenum)face, (GLint)level, 0, 0, mipmap_width, mipmap_height, texture.m_GLinternalformat, (GLsizei)(*datas)[i].size(), &(*datas)[i][0]);
					}
					else
					{
						glTexSubImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + (GLenum)face, (GLint)level, 0, 0, mipmap_width, mipmap_height, texture.m_GLformat, texture.m_GLtype, &(*datas)[i][0]);
					}

					max_uploaded_level = level > max_uploaded_level ? level : max_uploaded_level;
				}
			}
		}

		if (texture_layout.m_filter > 1 && !is_compressed && texture_layout.m_num_levels > max_uploaded_level+1)
		{
			glGenerateMipmap(texture.m_target);
		}

		glBindTexture(texture.m_target, 0);
	}

	StoreMemoryStatistics(OGL_instance::This->m_statistic_vector, texture_layout, datas);

	LOGGLERROR();

	return true;
}


bool GenVertexBuffer(uint32_t &buffer, NGL_vertex_descriptor &vertex_layout, uint32_t num, void *data)
{
	if (buffer && buffer >= OGL_instance::This->m_vertex_buffers.size())
	{
		_logf("OGL - GenVertexBuffer: Illegal vertex buffer id: %d!\n", buffer);
		buffer = 0xBADF00D;
		return false;
	}

	if (!buffer)
	{
		OGL_vertex_buffer vb;

		glGenBuffers(1, &vb.m_GL_vbo);

		OGL_instance::This->m_vertex_buffers.push_back(vb);
		buffer = (uint32_t)OGL_instance::This->m_vertex_buffers.size() - 1;
	}

	OGL_vertex_buffer &vb = OGL_instance::This->m_vertex_buffers[buffer];

	vb.m_hash = GenerateHash(vertex_layout);
	vb.m_vertex_descriptor = vertex_layout;
	vb.m_datasize = num * vertex_layout.m_stride;

	glBindBuffer(GL_ARRAY_BUFFER, vb.m_GL_vbo);
	glBufferData(GL_ARRAY_BUFFER, vb.m_datasize, data, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	OGL_instance::This->SetDebugLabel(GL_BUFFER, vb.m_GL_vbo, "vertex_buffer");

	StoreMemoryStatistics(OGL_instance::This->m_statistic_vector, vertex_layout, num);

	return true;
}


bool GenIndexBuffer(uint32_t &buffer, NGL_format format, uint32_t num, void *data)
{
	if (buffer && buffer >= OGL_instance::This->m_index_buffers.size())
	{
		_logf("OGL - GenIndexBuffer: Illegal index buffer id: %d!\n", buffer);
		buffer = 0xBADF00D;
		return false;
	}

	if (!buffer)
	{
		OGL_index_buffer ib;

		glGenBuffers(1, &ib.m_ebo);

		OGL_instance::This->m_index_buffers.push_back(ib);
		buffer = (uint32_t)OGL_instance::This->m_index_buffers.size() - 1;
	}

	OGL_index_buffer &ib = OGL_instance::This->m_index_buffers[buffer];

	uint32_t stride = 0;

	ib.m_format = format;
	ib.m_num_indices = num;

	switch (format)
	{
	case NGL_R16_UINT:
	{
		stride = 2;
		ib.m_GL_data_type = GL_UNSIGNED_SHORT;
		break;
	}
	case NGL_R32_UINT:
	{
		stride = 4;
		ib.m_GL_data_type = GL_UNSIGNED_INT;
		break;
	default:
		//error
		;
	}
	}

	glBindBuffer(GL_ARRAY_BUFFER, ib.m_ebo);
	glBufferData(GL_ARRAY_BUFFER, num * stride, data, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	OGL_instance::This->SetDebugLabel(GL_BUFFER, ib.m_ebo, "index_buffer");

	StoreMemoryStatistics(OGL_instance::This->m_statistic_vector, format, num);

	return true;
}


uint32_t GenJob(NGL_job_descriptor &descriptor)
{
	OGL_job *job = new OGL_job;

	if (!descriptor.m_load_shader_callback)
	{
		_logf("!!!error: m_load_shader_callback is not set for %s.\n", descriptor.m_subpasses[0].m_name.c_str());
	}

	job->m_descriptor = descriptor;

	job->CreatePasses();

	for (size_t i = 0; i < job->m_subpasses.size(); i++)
	{
		OGL_instance::This->GetPassClearValues(job->m_subpasses[i]);
	}

	CreateJobFBO(job);

	OGL_instance::This->m_jobs.push_back(job);

	return (uint32_t)OGL_instance::This->m_jobs.size() - 1;
}


void ViewportScissor(uint32_t job_, int32_t viewport[4], int32_t scissor[4])
{
	OGL_job *job = OGL_instance::This->m_jobs[job_];
	
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
	//OGL_job *job = OGL_instance::This->m_jobs[job_];

	glLineWidth(width);
}


void BlendState(uint32_t job_, uint32_t attachment, NGL_blend_func func, NGL_color_channel_mask mask)
{
	OGL_job *job = OGL_instance::This->m_jobs[job_];

	job->m_current_state.m_blend_state.m_funcs[attachment] = func;
	job->m_current_state.m_blend_state.m_masks[attachment] = mask;
}


void DepthState(uint32_t job_, NGL_depth_func func, bool mask)
{
	OGL_job *job = OGL_instance::This->m_jobs[job_];

	job->m_current_state.m_depth_state.m_func = func;
	job->m_current_state.m_depth_state.m_mask = mask;
}


void Begin(uint32_t job_, uint32_t idx)
{
	OGL_job *job = OGL_instance::This->m_jobs[job_];
	const NGL_job_descriptor &descriptor = job->m_descriptor;

	job->m_cmd_buffer_idx = idx;

	if (job->m_is_started)
	{
		_logf("Error: job (%s) has already started!", job->m_subpasses[0].m_name.c_str());
	}

	job->m_is_started = true;

	memset(&job->m_previous_state, ~0, sizeof(NGL_state));

	if (descriptor.m_is_compute)
	{
		return;
	}

	job->m_current_state.m_subpass = 0;

	const OGL_subpass &first_subpass = job->m_subpasses.front();

	if( first_subpass.m_color_attachments.size() == 1 && first_subpass.m_color_attachments[0].m_attachment.m_idx == 0 )
	{
		if( OGL_instance::This->m_ngl_adapter )
		{
			OGL_instance::This->m_default_fbo = OGL_instance::This->m_default_fbos[OGL_instance::This->m_ngl_adapter->GetDefaultFramebufferTextureIndex()];
		}
	}

	glBindFramebuffer(GL_FRAMEBUFFER, first_subpass.m_fbo ? first_subpass.m_fbo : OGL_instance::This->m_default_fbo);
	//glBindFramebuffer(GL_FRAMEBUFFER, OGL_instance::This->m_default_fbo);

	first_subpass.ClearAttachments();

	//glEnable(GL_SCISSOR_TEST);
}


void NextSubpass(uint32_t job_)
{
	OGL_job *job = OGL_instance::This->m_jobs[job_];

	job->m_current_state.m_subpass++;

	const OGL_subpass &prev_subpass = job->F();
	const OGL_subpass &current_subpass = job->G();
	
	prev_subpass.DiscardAttachments();

	glBindFramebuffer(GL_FRAMEBUFFER, current_subpass.m_fbo);
	
	current_subpass.ClearAttachments();
}


void End(uint32_t job_)
{
	OGL_job *job = OGL_instance::This->m_jobs[job_];
	const NGL_job_descriptor &descriptor = job->m_descriptor;

	if (!job->m_is_started)
	{
		_logf("Error: job (%s) hasn't started!", job->m_subpasses[0].m_name.c_str());
	}

	job->m_is_started = false;

	if (descriptor.m_is_compute)
	{
		return;
	}

#ifdef USE_VAO
	glBindVertexArray(0);
#endif

	const OGL_subpass &last_subpass = job->G();

	last_subpass.DiscardAttachments();

	//glBindFramebuffer(GL_FRAMEBUFFER, last_subpass.m_fbo ? last_subpass.m_fbo : OGL_instance::This->m_default_fbo);
	/*
	glBindFramebuffer(GL_FRAMEBUFFER, OGL_instance::This->m_default_fbo);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	*/

	glDisable(GL_POLYGON_OFFSET_FILL);
	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);
	glDisable(GL_SCISSOR_TEST);
	glColorMask(1, 1, 1, 1);
	glDepthMask(1);
	glDepthFunc(GL_LESS);
	glDepthRangef(0.0, 1.0);
}


inline bool Predraw(OGL_job *job, NGL_primitive_type primitive_type, uint32_t shader_code, uint32_t num_vbos, uint32_t *vbos, NGL_cull_mode cull_type, const void** parameters, uint32_t &primitive_GL_type)
{
	//const NGL_job_descriptor &descriptor = job->m_descriptor;
	const OGL_subpass &current_subpass = job->G();
	uint32_t uniform_update_mask = NGL_GROUP_PER_DRAW;
	primitive_GL_type = 0;

	if (!job->m_is_started)
	{
		_logf("Error: job (%s) hasn't started!", job->m_subpasses[0].m_name.c_str());
	}

	job->m_current_state.m_cull_mode = cull_type;
	job->m_current_state.m_primitive_type = primitive_type;
	job->m_current_state.m_shader.m_shader_code = shader_code;
	job->m_current_state.m_shader.m_vbo_hash = 0;
	for (uint32_t i = 0; i < num_vbos; i++)
	{
		job->m_current_state.m_shader.m_vbo_hash += OGL_instance::This->m_vertex_buffers[vbos[i]].m_hash;
	}

	uint32_t changed_mask = NGL_state::ChangedMask(job->m_current_state, job->m_previous_state);

	if (changed_mask & NGL_VIEWPORT_MASK)
	{
		glViewport(job->m_current_state.m_viewport[0], job->m_current_state.m_viewport[1], job->m_current_state.m_viewport[2], job->m_current_state.m_viewport[3]);
	}
	if (changed_mask & NGL_SCISSOR_MASK)
	{
		glScissor(job->m_current_state.m_scissor[0], job->m_current_state.m_scissor[1], job->m_current_state.m_scissor[2], job->m_current_state.m_scissor[3]);
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

			if (!renderer)
			{
				return false;
			}
		}

		job->m_active_renderer = renderer;

		glUseProgram(((OGL_renderer*)renderer)->m_p);

		uniform_update_mask = NGL_GROUP_PER_DRAW | NGL_GROUP_PER_RENDERER_CHANGE | NGL_GROUP_MANUAL;
	}
	if (changed_mask & NGL_CULL_MODE_MASK)
	{
		switch (cull_type)
		{
		case NGL_TWO_SIDED:
		{
			glDisable(GL_CULL_FACE);
			break;
		}
		case NGL_FRONT_SIDED:
		{
			glEnable(GL_CULL_FACE);
			glCullFace(GL_BACK);
			break;
		}
		case NGL_BACK_SIDED:
		{
			glEnable(GL_CULL_FACE);
			glCullFace(GL_FRONT);
			break;
		}
		}
	}
	if (changed_mask & NGL_COLOR_BLEND_FUNCS_MASK)
	{
		for (size_t i = 0; i < current_subpass.m_color_attachments_remap.size(); i++)
		{
			const uint32_t &reference = current_subpass.m_color_attachments_remap[i];

			switch (job->m_current_state.m_blend_state.m_funcs[reference])
			{
			case NGL_BLEND_DISABLED:
			{
				glDisable(GL_BLEND);
				break;
			}
			case NGL_BLEND_ADDITIVE:
			{
				glEnable(GL_BLEND);
				glBlendFunc(1, 1);
				break;
			}
			case NGL_BLEND_ALFA:
			{
				glEnable(GL_BLEND);
				glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
				break;
			}
			case NGL_BLEND_ADDITIVE_ALFA:
			{
				glEnable(GL_BLEND);
				glBlendFunc(GL_ONE, GL_SRC_ALPHA);
				break;
			}
			case NGL_BLEND_ADDITIVE_INVERSE_ALFA:
			{
				glEnable(GL_BLEND);
				glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
				break;
			}
			case NGL_BLEND_DECAL:
			{
				glEnable(GL_BLEND);
				glBlendFunc(GL_DST_COLOR, GL_SRC_COLOR);
				break;
			}
			case NGL_BLEND_MODULATIVE:
			{
				glEnable(GL_BLEND);
				glBlendFunc(GL_DST_COLOR, GL_ZERO);
				break;
			}
			case NGL_BLEND_TRANSPARENT_ACCUMULATION:
			{
				glEnable(GL_BLEND);
				glBlendFuncSeparate(GL_ONE, GL_ONE_MINUS_SRC_ALPHA, GL_ZERO, GL_ONE_MINUS_SRC_ALPHA);
				break;
			}
			default:
			{
				_logf("ERROR: Unhandled blend state!");
				break;
			}
			}
		}
	}
	if (changed_mask & NGL_COLOR_MASKS_MASK)
	{
		for (size_t i = 0; i < current_subpass.m_color_attachments_remap.size(); i++)
		{
			const uint32_t &reference = current_subpass.m_color_attachments_remap[i];

			glColorMask(
				(job->m_current_state.m_blend_state.m_masks[reference] | NGL_CHANNEL_R) > 0,
				(job->m_current_state.m_blend_state.m_masks[reference] | NGL_CHANNEL_G) > 0,
				(job->m_current_state.m_blend_state.m_masks[reference] | NGL_CHANNEL_B) > 0,
				(job->m_current_state.m_blend_state.m_masks[reference] | NGL_CHANNEL_A) > 0);
		}
	}
	if (changed_mask & NGL_DEPTH_FUNC_MASK)
	{
		if (job->m_previous_state.m_depth_state.m_func > NGL_DEPTH_DISABLED)
		{
			if (job->m_previous_state.m_depth_state.m_func == NGL_DEPTH_TO_FAR)
			{
				glDepthRangef(0.0f, 1.0f);
			}
			if (job->m_previous_state.m_depth_state.m_func == NGL_DEPTH_LESS_WITH_OFFSET)
			{
				glDisable(GL_POLYGON_OFFSET_FILL);
			}
			if (!current_subpass.m_depth_attachments.size())
			{
				glDisable(GL_DEPTH_TEST);
			}
		}

		if (current_subpass.m_depth_attachments.size())
		{
			switch (job->m_current_state.m_depth_state.m_func)
			{
			case NGL_DEPTH_DISABLED:
			{
				glDisable(GL_DEPTH_TEST);
				break;
			}
			case NGL_DEPTH_LESS:
			{
				glEnable(GL_DEPTH_TEST);
				glDepthFunc(GL_LESS);
				break;
			}
			case NGL_DEPTH_GREATER:
			{
				glEnable(GL_DEPTH_TEST);
				glDepthFunc(GL_GREATER);
				break;
			}
			case NGL_DEPTH_TO_FAR:
			{
				glEnable(GL_DEPTH_TEST);
				glDepthFunc(GL_LEQUAL);
				glDepthRangef(1, 1);
				break;
			}
			case NGL_DEPTH_LESS_WITH_OFFSET:
			{
				glEnable(GL_DEPTH_TEST);
				glDepthFunc(GL_LESS);
				glEnable(GL_POLYGON_OFFSET_FILL);
				glPolygonOffset(1, 200);
				break;
			}
			case NGL_DEPTH_ALWAYS:
			{
				glEnable(GL_DEPTH_TEST);
				glDepthFunc(GL_ALWAYS);
				break;
			}
			case NGL_DEPTH_EQUAL:
			{
				glEnable(GL_DEPTH_TEST);
				glDepthFunc(GL_EQUAL);
				break;
			}
			case NGL_DEPTH_LEQUAL:
			{
				glEnable(GL_DEPTH_TEST);
				glDepthFunc(GL_LEQUAL);
				break;
			}
			}
		}
	}
	if (changed_mask & NGL_DEPTH_MASK_MASK)
	{
		if (current_subpass.m_depth_attachments.size())
		{
			glDepthMask(job->m_current_state.m_depth_state.m_mask);
		}
	}
	switch (primitive_type)
	{
	case NGL_POINTS:
	{
		primitive_GL_type = GL_POINTS;
		break;
	}
	case NGL_LINES:
	{
		primitive_GL_type = GL_LINES;
		break;
	}
	case NGL_TRIANGLES:
	{
		primitive_GL_type = GL_TRIANGLES;
		break;
	}
	case NGL_PATCH3:
	{
		primitive_GL_type = GL_PATCHES;
		if (changed_mask & NGL_PRIMITIVE_TYPE_MASK)
		{
			glPatchParameteriProc(GL_PATCH_VERTICES, 3);
		}
		break;
	}
	case NGL_PATCH4:
	{
		primitive_GL_type = GL_PATCHES;
		if (changed_mask & NGL_PRIMITIVE_TYPE_MASK)
		{
			glPatchParameteriProc(GL_PATCH_VERTICES, 4);
		}
		break;
	}
	case NGL_PATCH16:
	{
		primitive_GL_type = GL_PATCHES;
		if (changed_mask & NGL_PRIMITIVE_TYPE_MASK)
		{
			glPatchParameteriProc(GL_PATCH_VERTICES, 16);
		}
		break;
	}
	default:
		_logf("unknown primitive type\n");
	}

	OGL_renderer *renderer = (OGL_renderer *)job->m_active_renderer;

	renderer->m_barriers = 0;

	for (uint32_t g = 0; g < 4; g++)
	{
		if (uniform_update_mask & (1 << g))
		{
			for (size_t i = 0; i<renderer->m_used_uniforms[g].size(); i++)
			{
				const NGL_used_uniform &uu = renderer->m_used_uniforms[g][i];

				if (uu.m_application_location > -1)
				{
					if (parameters[uu.m_application_location])
					{
						renderer->BindUniform(uu, parameters[uu.m_application_location]);
					}
					else
					{
						_logf("not set uniform: %s in %s\n", uu.m_uniform.m_name.c_str(), current_subpass.m_name.c_str());
					}
				}
			}
		}
	}

#ifdef USE_VAO
	glBindVertexArray(RM->m_global_vao);
#endif

	for (size_t j = 0; j < renderer->m_used_vbs.size(); j++)
	{
		OGL_renderer::_used_vertex_buffer &uvb = renderer->m_used_vbs[j];
		OGL_vertex_buffer &vb = OGL_instance::This->m_vertex_buffers[vbos[uvb.m_buffer_idx]];

		glBindBuffer(GL_ARRAY_BUFFER, vb.m_GL_vbo);

		for (size_t i = 0; i < uvb.m_GL_layouts.size(); i++)
		{
			OGL_renderer::_GL_layout &l = uvb.m_GL_layouts[i];

			glEnableVertexAttribArray(l.loc);
			glVertexAttribPointer(
				l.loc,
				l.size,
				l.type,
				l.normalized,
				vb.m_vertex_descriptor.m_stride,
				(void*)(intptr_t)l.offset
				);
		}
	}

	job->m_previous_state = job->m_current_state;

	return true;
}


inline void Postdraw(OGL_job *job)
{
	OGL_renderer *renderer = (OGL_renderer *)job->m_active_renderer;

	for (size_t j = 0; j<renderer->m_used_vbs.size(); j++)
	{
		OGL_renderer::_used_vertex_buffer &uvb = renderer->m_used_vbs[j];

		for (size_t i = 0; i<uvb.m_GL_layouts.size(); i++)
		{
			OGL_renderer::_GL_layout &l = uvb.m_GL_layouts[i];

			glDisableVertexAttribArray(l.loc);
		}
	}
}


void Draw(uint32_t job_, NGL_primitive_type primitive_type, uint32_t shader_code, uint32_t num_vbos, uint32_t *vbos, uint32_t ebo, NGL_cull_mode cull_type, const void** parameters)
{
	OGL_job *job = OGL_instance::This->m_jobs[job_];

	uint32_t primitive_GL_type;

	bool s = Predraw(job, primitive_type, shader_code, num_vbos, vbos, cull_type, parameters, primitive_GL_type);
	if (!s) return;

	OGL_index_buffer &ib = OGL_instance::This->m_index_buffers[ebo];

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ib.m_ebo);

	glDrawElements(primitive_GL_type, ib.m_num_indices, ib.m_GL_data_type, 0);

	Postdraw(job);
}


void BeginQuery(uint32_t job_)
{
	OGL_job *job = OGL_instance::This->m_jobs[job_];

	NGLStatistic* ngl_stat = OGL_instance::This->m_statistic_vector;
	if (job_ >= ngl_stat->jobs.size())
	{
		return;
	}

	job_statistics &job_stat = ngl_stat->jobs[job_];
	job_stat.m_is_active = true;

	job_stat.m_sub_pass[job->m_current_state.m_subpass].m_draw_calls.push_back(draw_call_statistics());

	draw_call_statistics *draw_stat = &job_stat.m_sub_pass[job->m_current_state.m_subpass].m_draw_calls.back();

	if (ngl_stat->queries_enabled[NGL_STATISTIC_PRIMITIVES_SUBMITED])
	{
		draw_stat->m_query_targets[NGL_STATISTIC_PRIMITIVES_SUBMITED] = GL_PRIMITIVES_SUBMITTED_ARB;
	}
	if (ngl_stat->queries_enabled[NGL_STATISTIC_VERTICES_SUBMITED])
	{
		draw_stat->m_query_targets[NGL_STATISTIC_VERTICES_SUBMITED] = GL_VERTICES_SUBMITTED_ARB;
	}
	if (ngl_stat->queries_enabled[NGL_STATISTIC_VS_INVOCATIONS])
	{
		draw_stat->m_query_targets[NGL_STATISTIC_VS_INVOCATIONS] = GL_VERTEX_SHADER_INVOCATIONS_ARB;
	}
	if (ngl_stat->queries_enabled[NGL_STATISTIC_TCS_PATCHES])
	{
		draw_stat->m_query_targets[NGL_STATISTIC_TCS_PATCHES] = GL_TESS_CONTROL_SHADER_PATCHES_ARB;
	}
	if (ngl_stat->queries_enabled[NGL_STATISTIC_TES_INVOCATIONS])
	{
		draw_stat->m_query_targets[NGL_STATISTIC_TES_INVOCATIONS] = GL_TESS_EVALUATION_SHADER_INVOCATIONS_ARB;
	}
	if (ngl_stat->queries_enabled[NGL_STATISTIC_GS_INVOCATIONS])
	{
		draw_stat->m_query_targets[NGL_STATISTIC_GS_INVOCATIONS] = GL_GEOMETRY_SHADER_INVOCATIONS;
	}
	if (ngl_stat->queries_enabled[NGL_STATISTIC_GS_PRIMITIVES_EMITTED])
	{
		draw_stat->m_query_targets[NGL_STATISTIC_GS_PRIMITIVES_EMITTED] = GL_GEOMETRY_SHADER_PRIMITIVES_EMITTED_ARB;
	}
	if (ngl_stat->queries_enabled[NGL_STATISTIC_FS_INVOCATIONS])
	{
		draw_stat->m_query_targets[NGL_STATISTIC_FS_INVOCATIONS] = GL_FRAGMENT_SHADER_INVOCATIONS_ARB;
	}
	if (ngl_stat->queries_enabled[NGL_STATISTIC_CLIPPING_INPUT_PRIMITIVES])
	{
		draw_stat->m_query_targets[NGL_STATISTIC_CLIPPING_INPUT_PRIMITIVES] = GL_CLIPPING_INPUT_PRIMITIVES_ARB;
	}
	if (ngl_stat->queries_enabled[NGL_STATISTIC_CLIPPING_OUTPUT_PRIMITIVES])
	{
		draw_stat->m_query_targets[NGL_STATISTIC_CLIPPING_OUTPUT_PRIMITIVES] = GL_CLIPPING_OUTPUT_PRIMITIVES_ARB;
	}
	if (ngl_stat->queries_enabled[NGL_STATISTIC_PRIMITIVES_GENERATED])
	{
		draw_stat->m_query_targets[NGL_STATISTIC_PRIMITIVES_GENERATED] = GL_PRIMITIVES_GENERATED;
	}
	if (ngl_stat->queries_enabled[NGL_STATISTIC_SAMPLES_PASSED])
	{
		draw_stat->m_query_targets[NGL_STATISTIC_SAMPLES_PASSED] = GL_SAMPLES_PASSED;
	}
	if (ngl_stat->queries_enabled[NGL_STATISTIC_TIME_ELAPSED])
	{
		draw_stat->m_query_targets[NGL_STATISTIC_TIME_ELAPSED] = GL_TIME_ELAPSED;
	}

	for (uint32_t i = 0; i < NGL_STATISTIC_COUNT; i++)
	{
		if (!ngl_stat->queries_enabled[i])
		{
			continue;
		}

		GLuint &query_object = draw_stat->m_query_objects[i];

		if (!query_object)
		{
			glGenQueries(1, &query_object);

			//OGL_instance::This->SetDebugLabel(GL_QUERY, query_object, "statistics_query");
		}

		glBeginQuery(draw_stat->m_query_targets[i], query_object);
	}
}


void EndQuery(uint32_t job_)
{
	OGL_job *job = OGL_instance::This->m_jobs[job_];

	NGLStatistic* ngl_stat = OGL_instance::This->m_statistic_vector;
	if (job_ >= ngl_stat->jobs.size())
	{
		return;
	}

	job_statistics &job_stat = (*OGL_instance::This->m_statistic_vector).jobs[job_];
	draw_call_statistics *draw_stat = &job_stat.m_sub_pass[job->m_current_state.m_subpass].m_draw_calls.back();

	for (uint32_t i = 0; i < NGL_STATISTIC_COUNT; i++)
	{
		if (!ngl_stat->queries_enabled[i])
		{
			continue;
		}

		glEndQuery(draw_stat->m_query_targets[i]);
	}
}


void DrawWithStatistic(uint32_t job_, NGL_primitive_type primitive_type, uint32_t shader_code, uint32_t num_vbos, uint32_t *vbos, uint32_t ebo, NGL_cull_mode cull_type, const void** parameters)
{
	BeginQuery(job_);
	Draw(job_, primitive_type, shader_code, num_vbos, vbos, ebo, cull_type, parameters);
	EndQuery(job_);

	OGL_job *job = OGL_instance::This->m_jobs[job_];
	OGL_renderer *renderer = (OGL_renderer*)job->m_active_renderer;
	for (size_t g = 0; g < 3; g++)
	{
		for (size_t i = 0; i < renderer->m_used_uniforms[g].size(); i++)
		{
			const NGL_used_uniform &uu = renderer->m_used_uniforms[g][i];
			const void* ptr = parameters[uu.m_application_location];
			if (uu.m_application_location > -1)
			{
				switch (uu.m_uniform.m_format)
				{
				case NGL_TEXTURE:
				{
					OGL_texture &texture = OGL_instance::This->m_textures[*(uint32_t*)ptr];
					
					if (uu.m_binding_type == GL_SAMPLER_2D || uu.m_binding_type == GL_SAMPLER_2D_ARRAY || uu.m_binding_type == GL_SAMPLER_CUBE)
					{
						OGL_instance::This->m_statistic_vector->m_used_textures[*(uint32_t*)ptr] = texture.m_texture_descriptor.m_name;
					}
					else if (uu.m_binding_type == GL_SAMPLER_2D_SHADOW || uu.m_binding_type == GL_SAMPLER_2D_ARRAY_SHADOW)
					{
						OGL_instance::This->m_statistic_vector->m_used_textures[*(uint32_t*)ptr] = texture.m_texture_descriptor.m_name;
					}
				}
				default:;
				}
			}
		}
	}
}


bool Dispatch(uint32_t job_, uint32_t shader_code, uint32_t x, uint32_t y, uint32_t z, const void** parameters);
bool DispatchWithStatistic(uint32_t job_, uint32_t shader_code, uint32_t x, uint32_t y, uint32_t z, const void** parameters)
{
	NGLStatistic* ngl_stat = OGL_instance::This->m_statistic_vector;
	job_statistics &job_stat = (*OGL_instance::This->m_statistic_vector).jobs[job_];

	uint32_t &timer = ngl_stat->jobs[job_].m_timer_query;
	glGenQueries(1, &timer);

	//OGL_instance::This->SetDebugLabel(GL_QUERY, timer, "timer_query");

	glBeginQuery(GL_TIME_ELAPSED, timer);
	bool s = Dispatch(job_, shader_code, x, y, z, parameters);
	glEndQuery(GL_TIME_ELAPSED);

	GLuint result_ready = GL_FALSE;
	while (!result_ready)
	{
		glGetQueryObjectuiv(timer, GL_QUERY_RESULT_AVAILABLE, &result_ready);
	}

	GLuint64 elapsed_time = 0;
	glGetQueryObjectui64vProc(timer, GL_QUERY_RESULT, &elapsed_time);
	glDeleteQueries(1, &timer);
	timer = 0;

	job_stat.m_dispatch_elapsed_time += elapsed_time;;
	job_stat.m_dispatch_count++;
	job_stat.m_is_active = true;

	return s;
}


void BeginStatistic(NGLStatistic& statistic_vector)
{
	if (OGL_instance::This->m_propertiesi[NGL_PIPELINE_STATISTICS] == 0)
	{
		return;
	}

	NGLStatistic* ngl_stat = &statistic_vector;
	OGL_instance::This->m_statistic_vector = ngl_stat;

	size_t job_count = OGL_instance::This->m_jobs.size();
	ngl_stat->jobs.resize(job_count);
	for (size_t i = 0; i < job_count; i++)
	{
		ngl_stat->jobs[i].m_sub_pass.resize(OGL_instance::This->m_jobs[i]->m_subpasses.size());
		for (size_t j = 0; j < OGL_instance::This->m_jobs[i]->m_subpasses.size(); j++)
		{
			ngl_stat->jobs[i].m_sub_pass[j].m_name = OGL_instance::This->m_jobs[i]->m_subpasses[j].m_name;
		}

		ngl_stat->jobs[i].Clear();
	}

	ngl_stat->m_used_textures.clear();

	nglDraw = OGL4::DrawWithStatistic;
	nglDispatch = OGL4::DispatchWithStatistic;
}

void EndStatistic()
{
	if (OGL_instance::This->m_propertiesi[NGL_PIPELINE_STATISTICS] == 0)
	{
		return;
	}

	nglDraw = OGL4::Draw;
	nglDispatch = OGL4::Dispatch;
}


void GetStatistic()
{
	if (OGL_instance::This->m_propertiesi[NGL_PIPELINE_STATISTICS] == 0)
	{
		return;
	}

	NGLStatistic* ngl_stat = OGL_instance::This->m_statistic_vector;
	for (size_t h = 0; h < ngl_stat->jobs.size();++h)
	{
		job_statistics& job_stat = ngl_stat->jobs[h];
		if (!job_stat.m_is_active)
		{
			continue;
		}

		for (size_t k = 0; k < job_stat.m_sub_pass.size(); k++)
		{
			for (size_t j = 0; j < job_stat.m_sub_pass[k].m_draw_calls.size(); j++)
			{
				draw_call_statistics *draw_stat = &job_stat.m_sub_pass[k].m_draw_calls[j];

				for (uint32_t i = 0; i < NGL_STATISTIC_COUNT; i++)
				{
					if (!ngl_stat->queries_enabled[i])
					{
						continue;
					}

					GLuint &query_object = draw_stat->m_query_objects[i];

					GLuint result_ready = GL_FALSE;
					while (!result_ready)
					{
						glGetQueryObjectuiv(query_object, GL_QUERY_RESULT_AVAILABLE, &result_ready);
					}

					GLuint64 result = 0;
					glGetQueryObjectui64vProc(query_object, GL_QUERY_RESULT, &result);
					draw_stat->m_query_results[i] = result;

					glDeleteQueries(1, &query_object);
					query_object = 0;
				}
			}
		}
	}
}


void DrawInstanced(uint32_t job_, NGL_primitive_type primitive_type, uint32_t shader_code, uint32_t num_vbos, uint32_t *vbos, uint32_t ebo, NGL_cull_mode cull_type, const void** parameters, uint32_t instance_count)
{
	OGL_job *job = OGL_instance::This->m_jobs[job_];

	uint32_t primitive_GL_type;

	bool s = Predraw(job, primitive_type, shader_code, num_vbos, vbos, cull_type, parameters, primitive_GL_type);
	if (!s) return;

	OGL_index_buffer &ib = OGL_instance::This->m_index_buffers[ebo];

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ib.m_ebo);

	glDrawElementsInstanced(primitive_GL_type, ib.m_num_indices, ib.m_GL_data_type, 0, instance_count);

	Postdraw(job);
}


void DrawIndirect(uint32_t job_, NGL_primitive_type primitive_type, uint32_t shader_code, uint32_t num_vbos, uint32_t *vbos, uint32_t ebo, NGL_cull_mode cull_type, const void** parameters, uint32_t indirect_buffer, void* indirect_buffer_offset)
{
	OGL_job *job = OGL_instance::This->m_jobs[job_];

	uint32_t primitive_GL_type;

	bool s = Predraw(job, primitive_type, shader_code, num_vbos, vbos, cull_type, parameters, primitive_GL_type);
	if (!s) return;

	OGL_index_buffer &ib = OGL_instance::This->m_index_buffers[ebo];
	OGL_vertex_buffer &vb = OGL_instance::This->m_vertex_buffers[indirect_buffer];

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ib.m_ebo);
	glBindBuffer(GL_DRAW_INDIRECT_BUFFER, vb.m_GL_vbo);

	glDrawElementsIndirectProc(primitive_GL_type, ib.m_GL_data_type, indirect_buffer_offset);

	glBindBuffer(GL_DRAW_INDIRECT_BUFFER, 0);

	Postdraw(job);
}


bool Predispatch(uint32_t job_, uint32_t shader_code, const void** parameters)
{
	OGL_job *job = OGL_instance::This->m_jobs[job_];
	//const NGL_job_descriptor &descriptor = job->m_descriptor;
	uint32_t uniform_update_mask = NGL_GROUP_PER_DRAW;

	if (!job->m_is_started)
	{
		_logf("Error: job (%s) hasn't started!", job->m_subpasses[0].m_name.c_str());
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

		job->m_active_renderer = renderer;

		glUseProgram(((OGL_renderer*)renderer)->m_p);
	
		uniform_update_mask = NGL_GROUP_PER_DRAW | NGL_GROUP_PER_RENDERER_CHANGE | NGL_GROUP_MANUAL;
	}

	OGL_renderer *renderer = (OGL_renderer *)job->m_active_renderer;

	renderer->m_barriers = 0;

	for (uint32_t g = 0; g < 4; g++)
	{
		if (uniform_update_mask & (1 << g))
		{
			for (size_t i = 0; i<renderer->m_used_uniforms[g].size(); i++)
			{
				const NGL_used_uniform &uu = renderer->m_used_uniforms[g][i];

				if (uu.m_application_location > -1)
				{
					if (parameters[uu.m_application_location])
					{
						renderer->BindUniform(uu, parameters[uu.m_application_location]);
					}
					else
					{
						_logf("not set uniform: %s in %s\n", uu.m_uniform.m_name.c_str(), job->G().m_name.c_str());
					}
				}
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

	glDispatchComputeProc(x, y, z);

	OGL_renderer *renderer = (OGL_renderer*)OGL_instance::This->m_jobs[job_]->m_active_renderer;
	if (renderer->m_barriers)
	{
		glMemoryBarrierProc(renderer->m_barriers);
	}

	return true;
}


bool DispatchIndirect(uint32_t job_, uint32_t shader_code, uint32_t x, uint32_t y, uint32_t z, const void** parameters, uint32_t indirect_buffer, void* indirect_buffer_offset)
{
	bool s = Predispatch(job_, shader_code, parameters);
	if (!s) return false;

	OGL_vertex_buffer &vb = OGL_instance::This->m_vertex_buffers[indirect_buffer];

	glBindBuffer(GL_DISPATCH_INDIRECT_BUFFER, vb.m_GL_vbo);

	glDispatchComputeIndirectProc((GLintptr)indirect_buffer_offset);

	glBindBuffer(GL_DISPATCH_INDIRECT_BUFFER, 0);

	OGL_renderer *renderer = (OGL_renderer*)OGL_instance::This->m_jobs[job_]->m_active_renderer;
	if (renderer->m_barriers)
	{
		glMemoryBarrierProc(renderer->m_barriers);
	}

	return true;
}


const char* GetString(NGL_backend_property prop)
{
	return OGL_instance::This->m_propertiess[prop].c_str();
}


int32_t GetInteger(NGL_backend_property prop)
{
	return OGL_instance::This->m_propertiesi[prop];
}


void DeletePipelines(uint32_t job_)
{
	OGL_job *job = OGL_instance::This->m_jobs[job_];
	
	job->DeleteRenderers();
}


void CustomAction(uint32_t job_, uint32_t parameter)
{
	if (parameter == NGL_CUSTOM_ACTION_WAIT_FINISH)
	{
		GLubyte values[NGL_WAIT_FINISH_RT_WIDTH * NGL_WAIT_FINISH_RT_HEIGHT * 4];
		glReadPixels(0, 0, NGL_WAIT_FINISH_RT_WIDTH, NGL_WAIT_FINISH_RT_HEIGHT, GL_RGBA, GL_UNSIGNED_BYTE, values);
	}
	if (parameter == 100)
	{
		glUseProgram(0);
		glActiveTexture(GL_TEXTURE0);
	}
	if (parameter == 101)
	{
		glFinish();
	}
	if (parameter == 102)
	{
		OGL_texture &texture = OGL_instance::This->m_textures[job_];

		glBindTexture(texture.m_target, texture.m_object);
		glGenerateMipmap(texture.m_target);
		glBindTexture(texture.m_target, 0);
	}
	if (parameter == 200)
	{
		OGL_instance::This->m_textures[0].m_texture_descriptor.m_clear_value[0] = 1.0f;
		OGL_instance::This->m_textures[0].m_texture_descriptor.m_clear_value[1] = 0.0f;
		OGL_instance::This->m_textures[0].m_texture_descriptor.m_clear_value[2] = 0.0f;
	}
	if (parameter == 201)
	{
		OGL_instance::This->m_textures[0].m_texture_descriptor.m_clear_value[0] = 0.0f;
		OGL_instance::This->m_textures[0].m_texture_descriptor.m_clear_value[1] = 1.0f;
		OGL_instance::This->m_textures[0].m_texture_descriptor.m_clear_value[2] = 0.0f;
	}
	if (parameter == 202)
	{
		OGL_instance::This->m_textures[0].m_texture_descriptor.m_clear_value[0] = 0.0f;
		OGL_instance::This->m_textures[0].m_texture_descriptor.m_clear_value[1] = 0.0f;
		OGL_instance::This->m_textures[0].m_texture_descriptor.m_clear_value[2] = 1.0f;
	}
#ifdef HAVE_GLEW
	if (parameter == 300)
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	}
	if (parameter == 301)
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}
#endif
}


void SetLineWidth(uint32_t job_, float width)
{
	glLineWidth(width);
}


bool GetTextureContent(uint32_t texture_, uint32_t level, uint32_t layer, uint32_t face, NGL_format format, NGL_resource_state state, uint32_t &width, uint32_t &height, std::vector<uint8_t> &data)
{
	GLenum GLformat;
	GLenum GLtype;
	uint32_t stride = 0;

	switch (format)
	{
	case NGL_R8_G8_B8_UNORM:
	{
		stride = 3;
		GLformat = GL_RGB;
		GLtype = GL_UNSIGNED_BYTE;
		break;
	}
	case NGL_R8_G8_B8_A8_UNORM:
	{
		stride = 4;
		GLformat = GL_RGBA;
		GLtype = GL_UNSIGNED_BYTE;
		break;
	}
	case NGL_R16_G16_B16_A16_FLOAT:
	{
		stride = 4 * 4;
		GLformat = GL_RGBA;
		GLtype = GL_FLOAT;
		break;
	}
	case NGL_R32_G32_B32_A32_FLOAT:
	{
		stride = 4 * 4;
		GLformat = GL_RGBA;
		GLtype = GL_FLOAT;
		break;
	}
	case NGL_R32_G32_B32_FLOAT:
	{
		stride = 3 * 4;
		GLformat = GL_RGB;
		GLtype = GL_FLOAT;
		break;
	}
	default:
	{
		_logf("GetTextureContent with non-supported format.\n");
		return false;
	}
	}	if (texture_)
	{
		OGL_texture &texture = OGL_instance::This->m_textures[texture_];

#if HAVE_GLEW

		int32_t w = 0;
		int32_t h = 0;
		GLenum target0 = 0;
		GLenum target1 = 0;

		if (texture.m_texture_descriptor.m_type == NGL_TEXTURE_2D)
		{
			target0 = GL_TEXTURE_2D;
			target1 = GL_TEXTURE_2D;
		}
		else if (texture.m_texture_descriptor.m_type == NGL_TEXTURE_CUBE)
		{
			target0 = GL_TEXTURE_CUBE_MAP;
			target1 = GL_TEXTURE_CUBE_MAP_POSITIVE_X + face;
		}

		glBindTexture(target0, texture.m_object);

		glGetTexLevelParameteriv(target1, level, GL_TEXTURE_WIDTH, &w);
		glGetTexLevelParameteriv(target1, level, GL_TEXTURE_HEIGHT, &h);

		width = w;
		height = h;

		if (w && h)
		{
			data.resize(w*h*stride);

			glGetTexImage(target1, level, GLformat, GLtype, &data[0]);
		}

		glBindTexture(target0, 0);
#else

		GLuint m_readback_fbo;
		glGenFramebuffers(1, &m_readback_fbo);

		glBindFramebuffer(GL_READ_FRAMEBUFFER, m_readback_fbo);

		if (texture.m_texture_descriptor.m_type == NGL_TEXTURE_2D)
		{
			glFramebufferTexture2D( GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture.m_object, level);
		}
		else if (texture.m_texture_descriptor.m_type == NGL_TEXTURE_2D_ARRAY)
		{
			glFramebufferTextureLayer( GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, texture.m_object, level, layer);
		}
		else if (texture.m_texture_descriptor.m_type == NGL_TEXTURE_CUBE)
		{
			glFramebufferTexture2D( GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, texture.m_object, level);
		}
		else
		{
			_logf("OGL - GetTextureContent: Invalid texture type");
			assert(0);
		}

		width = texture.m_texture_descriptor.m_size[0] / (1 << level);
		height = texture.m_texture_descriptor.m_size[1] / (1 << level);
		data.resize(width*height*stride);

		glViewport(0, 0, width, height);
		glReadBuffer(GL_COLOR_ATTACHMENT0);

		glReadPixels(0, 0, width, height, GLformat, GLtype, (void*)&data[0]);

		glDeleteFramebuffers(1, &m_readback_fbo);

#endif
	}
	else
	{
		int vp[4];

		glBindFramebuffer(GL_FRAMEBUFFER, OGL_instance::This->m_default_fbo);

		glGetIntegerv(GL_VIEWPORT, vp);

		width = vp[2] - vp[0];
		height = vp[3] - vp[1];
		data.resize(width * height * stride);

		glReadPixels(vp[0], vp[1], vp[2], vp[3], GLformat, GLtype, (void*)&data[0]);

		glBindFramebuffer(GL_FRAMEBUFFER, OGL_instance::This->m_default_fbo);
	}

	return true;
}


bool GetVertexBufferContent(uint32_t buffer_id, NGL_resource_state state, std::vector<uint8_t> &data)
{
	if (buffer_id >= OGL_instance::This->m_vertex_buffers.size())
	{
		_logf("OGL - GetVertexBufferContent: Illegal vertex buffer id: %d!\n", buffer_id);
		return false;
	}

	const OGL_vertex_buffer &buffer = OGL_instance::This->m_vertex_buffers[buffer_id];
	data.resize(buffer.m_datasize);

	if (!buffer.m_datasize)
	{
		_logf("OGL - GetVertexBufferContent: Warning! Buffer (%d) size is zero!: %d!\n", buffer_id);
		return true;
	}

	// This function should be only used for debugging, so we can stop the pipeline here
	glMemoryBarrierProc(GL_ALL_BARRIER_BITS);
	glFinish();

	// Map the buffer and copy the contents
	glBindBuffer(GL_ARRAY_BUFFER, buffer.m_GL_vbo);
	void *src_ptr = glMapBufferRange(GL_ARRAY_BUFFER, 0, buffer.m_datasize, GL_MAP_READ_BIT);
	if (src_ptr)
	{
		memcpy(data.data(), src_ptr, buffer.m_datasize);
		glUnmapBuffer(GL_ARRAY_BUFFER);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		return true;
	}
	else
	{
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		_logf("OGL - GetVertexBufferContent: Can not map buffer: %d GL id: %d!\n", buffer_id, buffer.m_GL_vbo);
		return false;
	}
}


bool ResizeTextures(uint32_t num_textures, uint32_t *textures, uint32_t size[3])
{
	std::set<OGL_job*> affected_jobs;

	for (uint32_t i = 0; i < num_textures; i++)
	{
		uint32_t texture_ = textures[i];

		if (!texture_ || texture_ >= OGL_instance::This->m_textures.size())
		{
			_logf("OGL - ResizeTexture: Illegal texture id: %d!\n", texture_);
			continue;
		}
		OGL_texture &texture = OGL_instance::This->m_textures[texture_];
		if (texture.m_texture_descriptor.m_type == NGL_TEXTURE_2D)
		{
			NGL_texture_descriptor &texture_layout = texture.m_texture_descriptor;
			memcpy(texture_layout.m_size, size, sizeof(texture_layout.m_size));

			glDeleteTextures(1, &texture.m_object);

			if (nglGenTexture(texture_, texture_layout, nullptr) == false)
			{
				_logf("OGL - ResizeTexture: Can not resize texture: %d!\n", texture_);

				continue;
			}
		}
		else
		{
			//assert(0);
		}

		for (size_t j = 0; j < OGL_instance::This->m_jobs.size(); j++)
		{
			OGL_job *job = OGL_instance::This->m_jobs[j];

			for (size_t j = 0; j < job->m_subpasses.size(); j++)
			{
				OGL_subpass &sp = job->m_subpasses[j];

				for (size_t i = 0; i < sp.m_color_attachments.size(); i++)
				{
					const NGL_attachment_descriptor &atd = sp.m_color_attachments[i];
					if (atd.m_attachment.m_idx == texture_)
					{
						affected_jobs.insert(job);
					}
				}

				for (size_t i = 0; i < sp.m_depth_attachments.size(); i++)
				{
					const NGL_attachment_descriptor &atd = sp.m_depth_attachments[i];
					if (atd.m_attachment.m_idx == texture_)
					{
						affected_jobs.insert(job);
					}
				}
			}
		}
	}

	for (std::set<OGL_job*>::iterator i = affected_jobs.begin(); i != affected_jobs.end(); i++)
	{
		bool b = CreateJobFBO(*i);
		if( !b)
		{
			_logf("OGL - ResizeTexture: Can not rebind attachments for job: %s!\n", (*i)->m_subpasses[0].m_name.c_str());
		}
	}

	return true;
}


void BeginCommandBuffer(uint32_t idx)
{
}


void EndCommandBuffer(uint32_t idx)
{
}


void SubmitCommandBuffer(uint32_t idx)
{
}


void Flush()
{
	glFlush();
}


void Finish()
{
	glFinish();
}


void Barrier(uint32_t cmd_buffer, std::vector<NGL_texture_subresource_transition> &texture_barriers, std::vector<NGL_buffer_transition> &buffer_barriers)
{
	// NOP
}


void DestroyContext()
{	
	delete OGL_instance::This;
	OGL_instance::This = 0;
}


OGL_instance *OGL_instance::This = 0;


OGL_instance::OGL_instance()
{
	This = this;

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
	nglBeginStatistic = BeginStatistic;
	nglEndStatistic = EndStatistic;
	nglGetStatistic = GetStatistic;
	nglDestroyContext = DestroyContext;
	nglBarrier = Barrier;

	{
		//a nullas attachment invalid, mert a m_format == 0
		OGL_texture t;
		m_textures.push_back(t);
	}
	m_statistic_vector = 0;
	m_global_vao = 0;
	m_default_fbo = 0;
	m_ngl_adapter = 0;
	m_validation_layer_enabled = false;
}


OGL_instance::~OGL_instance()
{
	for (size_t i = 0; i < m_jobs.size(); i++)
	{
		m_jobs[i]->DeleteRenderers();

		delete m_jobs[i];
	}
	m_jobs.clear();

	for (size_t i = 0; i < m_textures.size(); i++)
	{
		if (m_textures[i].m_texture_descriptor.m_type == NGL_RENDERBUFFER)
		{
			glDeleteRenderbuffers(1, &m_textures[i].m_object);
		}
		else
		{
			glDeleteTextures(1, &m_textures[i].m_object);
		}
	}

	for (size_t i = 0; i < m_vertex_buffers.size(); i++)
	{
		glDeleteBuffers(1, &m_vertex_buffers[i].m_GL_vbo);
	}

	for (size_t i = 0; i < m_index_buffers.size(); i++)
	{
		glDeleteBuffers(1, &m_index_buffers[i].m_ebo);
	}

	if (m_global_vao)
	{
		glDeleteVertexArrays(1, &m_global_vao);
	}		

	for (size_t i = 0; i < m_default_fbos.size(); i++)
	{
		if (m_default_fbos[i])
		{
			glDeleteFramebuffers(1, &m_default_fbos[i]);
		}
	}	
}


void OGL_instance::Init(NGL_api api, uint32_t major_version, uint32_t minor_version, bool enable_validation)
{
	if (api == NGL_OPENGL_ES && (major_version == 3 && minor_version == 0))
	{
		getES30ProcAddresses();
	}
	else
	{
		getES31ProcAddresses();
	}

	m_textures[0].m_texture_descriptor.m_size[0] = m_context_descriptor.m_display_width;
	m_textures[0].m_texture_descriptor.m_size[1] = m_context_descriptor.m_display_height;

	if (m_context_descriptor.m_user_data.size() >= 1)
	{
		m_ngl_adapter = (NGL_gl_adapter_interface*)m_context_descriptor.m_user_data[0];

		std::vector<uint32_t> default_textures;
		m_ngl_adapter->GetDefaultFramebufferTextures(default_textures);

		if (default_textures.empty())
		{
			_logf("Zero default texture provided by NGL_gl_adapter_interface!");
			assert(0);
		}

		for (size_t i = 0; i < default_textures.size(); i++)
		{
			GLuint fbo = 0;

			if (default_textures[i])
			{
				glGenFramebuffers(1, &fbo);
				glBindFramebuffer(GL_FRAMEBUFFER, fbo);
				glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, default_textures[i], 0);

				SetDebugLabel(GL_FRAMEBUFFER, fbo, "default_fbo");
			}

			m_default_fbos.push_back(fbo);
		}

		m_default_fbo = m_default_fbos[m_ngl_adapter->GetDefaultFramebufferTextureIndex()];
		glBindFramebuffer(GL_FRAMEBUFFER, m_default_fbo);
	}

#ifdef USE_VAO
	glGenVertexArrays(1, &m_global_vao);
#endif

	if (api == NGL_OPENGL)
	{
#ifdef GL_TEXTURE_CUBE_MAP_SEAMLESS
		glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
#endif
	}

	if (api == NGL_OPENGL)
	{
#ifdef GL_VERTEX_PROGRAM_POINT_SIZE
		glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
#endif

	}

#ifdef GL_FRAGMENT_SHADER_DERIVATIVE_HINT
	glHint(GL_FRAGMENT_SHADER_DERIVATIVE_HINT, GL_NICEST);
#endif


	glLineWidth(1);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glPixelStorei(GL_PACK_ALIGNMENT, 1);

	glGenVertexArrays(1, &m_global_vao);
	glBindVertexArray(m_global_vao);

#if 0
	if (wglSwapIntervalEXT)
	{
		//wglSwapIntervalEXT(0);
	}
#endif

	LOGGLERROR();
	const GLubyte *ext_str = glGetString(GL_EXTENSIONS); // Generates GL_ERROR on core profile
	glGetError();
	if (ext_str)
	{
		m_extension_list = (const char*)glGetString(GL_EXTENSIONS);
	}
	else
	{
#ifdef GL_NUM_EXTENSIONS
		int numext = 0;
		glGetIntegerv(GL_NUM_EXTENSIONS, &numext);
		for (int i = 0; i<numext; i++)
		{
			const char *s = (const char*)glGetStringi(GL_EXTENSIONS, i);
			if (s)
			{
				m_extension_list += std::string((char*)s) + " ";
			}
		}
#endif
	}

	if (enable_validation)
	{
		// Enable validation if supported
		bool validation_supported = m_extension_list.find("ARB_debug_output") != std::string::npos;
		validation_supported |= m_extension_list.find("KHR_debug") != std::string::npos;
		if (validation_supported)
		{
			_logf("Enable GL validation layer");

			glDebugMessageCallbackProc(OGL_instance::DebugCallback, NULL);
			glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);

			// Disable notification messages
			glDebugMessageControlProc(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_NOTIFICATION, 0, nullptr, GL_FALSE);
			
			m_validation_layer_enabled = true;
		}
		else
		{
			_logf("GL validation layer is not supported!");
		}
	}

	const char *vendor = (const char*) glGetString(GL_VENDOR);
	const char *renderer = (const char*) glGetString(GL_RENDERER);
	const char *version = (const char*) glGetString(GL_VERSION);

	m_propertiess[NGL_VENDOR] = vendor ? vendor : "null";
	m_propertiess[NGL_RENDERER] = renderer ? renderer : "null";
	m_propertiess[NGL_VERSION] = version ? version : "null";
	for (size_t i = 0; i < NGL_NUM_PROPERTIES; i++)
	{
		m_propertiesi[i] = 0;
	}
	m_propertiesi[NGL_API] = api;
	m_propertiesi[NGL_MAJOR_VERSION] = major_version;
	m_propertiesi[NGL_MINOR_VERSION] = minor_version;
	m_propertiesi[NGL_RASTERIZATION_CONTROL_MODE] = NGL_ORIGIN_LOWER_LEFT;
	m_propertiesi[NGL_DEPTH_MODE] = NGL_NEGATIVE_ONE_TO_ONE;
	m_propertiesi[NGL_NEED_SWAPBUFFERS] = 1;
	m_propertiesi[NGL_TEXTURE_COMPRESSION_DXT5] = api == NGL_OPENGL_ES ? 0 : 1;
	m_propertiesi[NGL_TEXTURE_COMPRESSION_ETC2] = api == NGL_OPENGL_ES ? 1 : 0;
	m_propertiesi[NGL_TEXTURE_COMPRESSION_ASTC] = m_extension_list.find("texture_compression_astc_ldr") != std::string::npos ? 1 : 0;
	m_propertiesi[NGL_TESSELLATION] = api == NGL_OPENGL_ES ? 0 : 1;
	m_propertiesi[NGL_PIPELINE_STATISTICS] = m_extension_list.find("pipeline_statistics_query") != std::string::npos ? 1 : 0;
	//just to be sure
	if (api == NGL_OPENGL_ES)
	{
		m_propertiesi[NGL_FLOATING_POINT_RENDERTARGET] = m_extension_list.find("texture_float") != std::string::npos ? 1 : 0;
	}
	else
	{
		m_propertiesi[NGL_FLOATING_POINT_RENDERTARGET] = 1;
	}

	if (m_extension_list.find("texture_filter_anisotropic") != std::string::npos)
	{
		glGetIntegerv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &m_propertiesi[NGL_TEXTURE_MAX_ANISOTROPY]);
	}

	glGetIntegerv(GL_MAX_TEXTURE_SIZE, &m_propertiesi[NGL_TEXTURE_MAX_SIZE_2D]);
	glGetIntegerv(GL_MAX_CUBE_MAP_TEXTURE_SIZE, &m_propertiesi[NGL_TEXTURE_MAX_SIZE_CUBE]);
	glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &m_GL_MAX_VERTEX_ATTRIBS);
#ifdef GL_MAX_VERTEX_UNIFORM_VECTORS
	glGetIntegerv(GL_MAX_VERTEX_UNIFORM_VECTORS, &m_GL_MAX_VERTEX_UNIFORM_VECTORS);
#endif
	glGetIntegerv(GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS, &m_GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS);
#ifdef GL_MAX_VARYING_VECTORS
	glGetIntegerv(GL_MAX_VARYING_VECTORS, &m_GL_MAX_VARYING_VECTORS);
#endif
#ifdef GL_MAX_FRAGMENT_UNIFORM_VECTORS
	glGetIntegerv(GL_MAX_FRAGMENT_UNIFORM_VECTORS, &m_GL_MAX_FRAGMENT_UNIFORM_VECTORS);
#endif
	glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &m_GL_MAX_TEXTURE_IMAGE_UNITS);

	// compute shader properties
	{
		m_propertiesi[NGL_MAX_COMPUTE_WORK_GROUP_SIZE_X] = 0;
		m_propertiesi[NGL_MAX_COMPUTE_WORK_GROUP_SIZE_Y] = 0;
		m_propertiesi[NGL_MAX_COMPUTE_WORK_GROUP_SIZE_Z] = 0;
		glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 0, &m_propertiesi[NGL_MAX_COMPUTE_WORK_GROUP_SIZE_X]);
		glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 1, &m_propertiesi[NGL_MAX_COMPUTE_WORK_GROUP_SIZE_Y]);
		glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 2, &m_propertiesi[NGL_MAX_COMPUTE_WORK_GROUP_SIZE_Z]);

		m_propertiesi[NGL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS] = 0;
		glGetIntegerv(GL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS, &m_propertiesi[NGL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS]);

		m_propertiesi[NGL_MAX_COMPUTE_SHARED_MEMORY_SIZE] = 0;
		glGetIntegerv(GL_MAX_COMPUTE_SHARED_MEMORY_SIZE, &m_propertiesi[NGL_MAX_COMPUTE_SHARED_MEMORY_SIZE]);
	}

	m_propertiesi[NGL_D16_LINEAR_SHADOW_FILTER] = 1;
	m_propertiesi[NGL_D24_LINEAR_SHADOW_FILTER] = 1;
	m_propertiesi[NGL_PIPELINE_MAX_PUSH_CONSTANT_SIZE] = 0;
		
	{
		GLint object_type = 0;

		glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, GL_DEPTH, GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE, &object_type);

		if (object_type)
		{
			GLint depth_bits = 0;
			GLint type = GL_UNSIGNED_NORMALIZED;
			//GLint encoding;

			glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, GL_DEPTH, GL_FRAMEBUFFER_ATTACHMENT_DEPTH_SIZE, &depth_bits);
			glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, GL_DEPTH, GL_FRAMEBUFFER_ATTACHMENT_COMPONENT_TYPE, &type);
			//glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, GL_DEPTH, GL_FRAMEBUFFER_ATTACHMENT_COLOR_ENCODING, &encoding);

			if (depth_bits == 16)
			{
				m_propertiesi[NGL_SWAPCHAIN_DEPTH_FORMAT] = 16;

				if (type == GL_UNSIGNED_NORMALIZED)
				{
					m_propertiess[NGL_SWAPCHAIN_DEPTH_FORMAT] = "unorm";
				}
			}
			else if (depth_bits == 24)
			{
				m_propertiesi[NGL_SWAPCHAIN_DEPTH_FORMAT] = 24;

				if (type == GL_UNSIGNED_NORMALIZED)
				{
					m_propertiess[NGL_SWAPCHAIN_DEPTH_FORMAT] = "unorm";
				}
			}
			else if (depth_bits == 32)
			{
				m_propertiesi[NGL_SWAPCHAIN_DEPTH_FORMAT] = 32;

				if (type == GL_UNSIGNED_NORMALIZED)
				{
					m_propertiess[NGL_SWAPCHAIN_DEPTH_FORMAT] = "unorm";
				}
			}
			else
			{
				std::stringstream s;
				s << depth_bits;

				m_propertiesi[NGL_SWAPCHAIN_DEPTH_FORMAT] = -1;
				m_propertiess[NGL_SWAPCHAIN_DEPTH_FORMAT] = s.str();
			}
		}
	}
	{
		GLint object_type = 0;

		glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, GL_BACK, GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE, &object_type);

		if (object_type)
		{
			GLint rgba_bits[4] = { 0, 0, 0, 0 };
			GLint type = GL_UNSIGNED_NORMALIZED;
			GLint encoding = GL_LINEAR;

			glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, GL_BACK, GL_FRAMEBUFFER_ATTACHMENT_RED_SIZE, &rgba_bits[0]);
			glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, GL_BACK, GL_FRAMEBUFFER_ATTACHMENT_GREEN_SIZE, &rgba_bits[1]);
			glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, GL_BACK, GL_FRAMEBUFFER_ATTACHMENT_BLUE_SIZE, &rgba_bits[2]);
			glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, GL_BACK, GL_FRAMEBUFFER_ATTACHMENT_ALPHA_SIZE, &rgba_bits[3]);
			glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, GL_BACK, GL_FRAMEBUFFER_ATTACHMENT_COMPONENT_TYPE, &type);
			glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, GL_BACK, GL_FRAMEBUFFER_ATTACHMENT_COLOR_ENCODING, &encoding);

			if (rgba_bits[0] == 8 && rgba_bits[1] == 8 && rgba_bits[2] == 8 && rgba_bits[3] == 8)
			{
				m_propertiesi[NGL_SWAPCHAIN_COLOR_FORMAT] = 8888;

				if (type == GL_UNSIGNED_NORMALIZED)
				{
					m_propertiess[NGL_SWAPCHAIN_COLOR_FORMAT] = "unorm";
				}
				if (encoding == GL_SRGB)
				{
					m_propertiess[NGL_SWAPCHAIN_COLOR_FORMAT] += "_srgba";
				}
				else
				{
					m_propertiess[NGL_SWAPCHAIN_COLOR_FORMAT] += "_rgba";
				}
			}
			else if (rgba_bits[0] == 8 && rgba_bits[1] == 8 && rgba_bits[2] == 8 && rgba_bits[3] == 0)
			{
				m_propertiesi[NGL_SWAPCHAIN_COLOR_FORMAT] = 888;

				if (type == GL_UNSIGNED_NORMALIZED)
				{
					m_propertiess[NGL_SWAPCHAIN_COLOR_FORMAT] = "unorm";
				}
				if (encoding == GL_SRGB)
				{
					m_propertiess[NGL_SWAPCHAIN_COLOR_FORMAT] += "_srgb";
				}
				else
				{
					m_propertiess[NGL_SWAPCHAIN_COLOR_FORMAT] += "_rgb";
				}
			}
			else
			{
				std::stringstream s;
				s << rgba_bits[0] << rgba_bits[1] << rgba_bits[2] << rgba_bits[3];

				m_propertiesi[NGL_SWAPCHAIN_COLOR_FORMAT] = -1;
				m_propertiess[NGL_SWAPCHAIN_COLOR_FORMAT] = s.str();
			}
		}
	}

	_logf("GL_RENDERER: %s\n", m_propertiess[NGL_RENDERER].c_str());
	_logf("GL_VERSION: %s\n", m_propertiess[NGL_VERSION].c_str());
	_logf("GL max. aniso: %d\n", m_propertiesi[NGL_TEXTURE_MAX_ANISOTROPY]);
	_logf("GL max. 2D texture size: %d\n", m_propertiesi[NGL_TEXTURE_MAX_SIZE_2D]);
	_logf("GL max. cube texture size: %d\n", m_propertiesi[NGL_TEXTURE_MAX_SIZE_CUBE]);

	_logf("GL max. vertex shader attribs: %d\n", m_GL_MAX_VERTEX_ATTRIBS);
	_logf("GL max. vertex shader uniforms: %d\n", m_GL_MAX_VERTEX_UNIFORM_VECTORS);
	_logf("GL max. vertex shader images: %d\n", m_GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS);

	_logf("GL max. shader varyings: %d\n", m_GL_MAX_VARYING_VECTORS);
	_logf("GL max. fragment shader uniforms: %d\n", m_GL_MAX_FRAGMENT_UNIFORM_VECTORS);
	_logf("GL max. fragment shader images: %d\n", m_GL_MAX_TEXTURE_IMAGE_UNITS);
}


void OGL_instance::SetDebugLabel(GLenum object_type, GLuint object_id, const std::string &name)
{
	if (m_validation_layer_enabled && name.length())
	{
		glObjectLabelProc(object_type, object_id, (GLsizei)name.length(), name.c_str());
	}
}


void GFXB_APIENTRY OGL_instance::DebugCallback(
	GLenum source,
	GLenum type,
	GLuint id,
	GLenum severity,
	GLsizei length,
	const GLchar* message,
	const void* userParam)
{
	std::string source_str;
	switch (source)
	{
	case GL_DEBUG_SOURCE_API:					source_str = "API"; break;
	case GL_DEBUG_SOURCE_WINDOW_SYSTEM:			source_str = "WINDOW_SYSTEM"; break;
	case GL_DEBUG_SOURCE_SHADER_COMPILER:		source_str = "SHADER_COMPILER"; break;
	case GL_DEBUG_SOURCE_THIRD_PARTY:			source_str = "THIRD_PARTY"; break;
	case GL_DEBUG_SOURCE_APPLICATION:			source_str = "APPLICATION"; break;
	case GL_DEBUG_SOURCE_OTHER:					source_str = "OTHER"; break;
	default:									source_str = "UNDEFINED"; break;
	}

	std::string type_str;
	switch (type)
	{
	case GL_DEBUG_TYPE_ERROR:					type_str = "ERROR"; break;
	case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:		type_str = "DEPRECATED_BEHAVIOR"; break;
	case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:		type_str = "UNDEFINED_BEHAVIOR"; break;
	case GL_DEBUG_TYPE_PORTABILITY:				type_str = "PORTABILITY"; break;
	case GL_DEBUG_TYPE_PERFORMANCE:				type_str = "PERFORMANCE"; break;
	case GL_DEBUG_TYPE_OTHER:					type_str = "OTHER"; break;
	default:									type_str = "UNDEFINED"; break;
	}

	std::string severity_str;
	switch (severity)
	{
	case GL_DEBUG_SEVERITY_HIGH:			severity_str = "HIGH"; break;
	case GL_DEBUG_SEVERITY_MEDIUM:			severity_str = "MEDIUM"; break;
	case GL_DEBUG_SEVERITY_LOW:				severity_str = "LOW"; break;
	case GL_DEBUG_SEVERITY_NOTIFICATION:	severity_str = "NOTIFICATION"; break;
	default:								severity_str = "UNDEFINED"; break;
	}

	_logf("GL Debug: %s [source=%s type=%s severity=%s id=%u]", message, source_str.c_str(), type_str.c_str(), severity_str.c_str(), id);
}


}//!namespace OGL


void nglCreateContextOGL(NGL_context_descriptor& descriptor)
{
	new OGL4::OGL_instance();

	OGL4::OGL_instance::This->m_context_descriptor = descriptor;

#ifdef HAVE_GLEW
	glewExperimental = GL_TRUE;
	glewInit();
	glGetError();
#endif

	OGL4::OGL_instance::This->Init(descriptor.m_api, descriptor.m_major_version, descriptor.m_minor_version, descriptor.m_enable_validation);
}
