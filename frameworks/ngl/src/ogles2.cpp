/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "ngl.h"
#include "ngl_internal.h"
#ifdef PLATFORM_IS_WINDOWS
#include "glew.h"
#else
#include "GLES2\gl2.h"
#include "GLES2\gl2ext.h"
#endif

#include <map>

#ifndef GL_DEPTH_COMPONENT24_OES
#define GL_DEPTH_COMPONENT24_OES GL_DEPTH_COMPONENT24
#endif

namespace OGLES2
{
#include "ogl.h"

NGL_renderer* OGL_job::CreateRenderer(NGL_shader_hash sh, uint32_t num_vbos, uint32_t *vbos)
{
	uint32_t p;
	uint32_t shaders[NGL_NUM_SHADER_TYPES];
	NGL_shader_source_descriptor ssd[7];
	std::vector<NGL_shader_uniform> application_uniforms;

	p = glCreateProgram();

	m_descriptor.m_load_shader_callback(m_descriptor, sh.m_shader_code, ssd, application_uniforms);

	for (uint32_t shader_type = NGL_VERTEX_SHADER; shader_type< NGL_NUM_SHADER_TYPES; shader_type++)
	{
		if (ssd[shader_type].m_source_data.length())
		{
			uint32_t GL_shader_type;

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
				//error
			}
			else if (shader_type == NGL_TESS_EVALUATION_SHADER)
			{
				//error
			}
			else if (shader_type == NGL_GEOMETRY_SHADER)
			{
				//error
			}
			else if (shader_type == NGL_COMPUTE_SHADER)
			{
				//error
			}

			shaders[shader_type] = OGL_renderer::CompileShader(GL_shader_type, ssd[shader_type].m_source_data.c_str());

			glAttachShader(p, shaders[shader_type]);
		}
	}

	OGL_renderer *renderer = new OGL_renderer;

	renderer->m_hash.X = sh.X;
	renderer->m_p = p;
	renderer->LinkShader();
	if (!m_descriptor.m_is_compute)
	{
		bool shader_matches_with_mesh = renderer->GetActiveAttribs(num_vbos, vbos);
		if (!shader_matches_with_mesh)
		{
			_logf("shader_matches_with_mesh\n");
		}
	}
	renderer->GetActiveResources(0);

	for (size_t j = 0; j<renderer->m_used_uniforms.size(); j++)
	{
		for (size_t i = 0; i < application_uniforms.size(); i++)
		{
			if (application_uniforms[i].m_name == renderer->m_used_uniforms[j].m_uniform.m_name)
			{
				renderer->m_used_uniforms[j].m_application_location = i;
				break;
			}
		}
		if (renderer->m_used_uniforms[j].m_application_location == -1)
		{
			_logf("not set uniform: %s in %s\n", renderer->m_used_uniforms[j].m_uniform.m_name.c_str(), m_descriptor.m_name.c_str());
		}
	}

	m_renderers.push_back(renderer);

	return renderer;
}


uint32_t OGL_renderer::CompileShader(uint32_t shader_type, const char *str)
{
	const char *sources[1] =
	{
		str
	};
	GLint status = 0;
	uint32_t s = glCreateShader(shader_type);

	glShaderSource(s, 1, sources, 0);

	glCompileShader(s);

	glGetShaderiv(s, GL_COMPILE_STATUS, &status);

	if (!status)
	{
		int len;
		std::string log;

		glGetShaderiv(s, GL_INFO_LOG_LENGTH, &len);
		log.resize(len);

		glGetShaderInfoLog(s, len, 0, &log[0]);

		_logf("%s\n", log.c_str());
	}
	else
	{
		//_logf(".info(%s): compiled.\n", io.Filename());
	}

	return s;
}


void OGL_renderer::LinkShader()
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
	}
	else
	{
		//_logf(".info(program): linked.\n");
	}
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
		uint32_t attrib_size;

		glGetActiveAttrib(m_p, index, bufSize, &length, &size, &type, name);
		location = glGetAttribLocation(m_p, name);

		if (location == -1)
		{
			continue;
		}

		attrib_semantic_name = name;
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
			_logf("Unsupported vertex attrib type!\n");
		}

		bool found;
		OGL_vertex_buffer *vb;
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
		case NGL_R8_B8_UINT:
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


void OGL_renderer::GetActiveResources(uint32_t programInterface)
{
	int32_t num_active_uniforms = 0;

	glGetProgramiv(m_p, GL_ACTIVE_UNIFORMS, &num_active_uniforms);

	for (int32_t i = 0; i<num_active_uniforms; i++)
	{
		GLuint index = i;
		GLsizei bufSize = 512;
		GLsizei length = 0;
		GLint size = 0;
		GLenum type;
		GLchar name[512];
		int32_t location;

		glGetActiveUniform(m_p, index, bufSize, &length, &size, &type, name);
		location = glGetUniformLocation(m_p, name);

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
		case GL_SAMPLER_2D:
		{
			uu.m_uniform.m_format = NGL_TEXTURE;
			uu.m_binding_type = GL_SAMPLER_2D;
			break;
		}
		case GL_SAMPLER_CUBE:
		{
			uu.m_uniform.m_format = NGL_TEXTURE;
			uu.m_binding_type = GL_SAMPLER_CUBE;
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
		default:
			_logf("Warning!! unhandled reflection case!!\n");
		}
		{
			//http://stackoverflow.com/questions/10532384/how-to-remove-a-particular-substring-from-a-string
			size_t array_remark = uu.m_uniform.m_name.find("[0]");
			if (array_remark != std::string::npos)
			{
				uu.m_uniform.m_name.erase(array_remark, 3);
			}
		}

		m_used_uniforms.push_back(uu);
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
	{
		OGL_texture &texture = OGL_instance::This->m_textures[*(uint32_t*)ptr];

		if (uu.m_binding_type == GL_SAMPLER_2D || uu.m_binding_type == GL_SAMPLER_CUBE)
		{
			glActiveTexture(GL_TEXTURE0 + m_used_texture_slots);
			glBindTexture(texture.m_target, texture.m_object);
			glUniform1i(uu.m_shader_location[0], m_used_texture_slots);
			m_used_texture_slots++;
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
	default:
	{
		_logf("Warning!! Unhandled bind uniform!!\n");
	}
	}
}


bool GenTexture(uint32_t &buffer, NGL_texture_descriptor &texture_layout, std::vector<std::vector<uint8_t> > *datas)
{
	if (texture_layout.m_size[0] > OGL_instance::This->m_GL_MAX_TEXTURE_SIZE || texture_layout.m_size[1] > OGL_instance::This->m_GL_MAX_TEXTURE_SIZE)
	{
		//error
		return 0;
	}
	if (texture_layout.m_is_renderable && datas)
	{
		//error;
		return 0;
	}
	if (!texture_layout.m_is_renderable && !datas)
	{
		//error;
		return 0;
	}

	LOGGLERROR();

	if (buffer && buffer >= OGL_instance::This->m_textures.size())
	{
		buffer = 0xBADF00D;
		return false;
	}

	if (!buffer)
	{
		OGL_texture texture;

		OGL_instance::This->m_textures.push_back(texture);
		buffer = OGL_instance::This->m_textures.size() - 1;
	}

	OGL_texture &texture = OGL_instance::This->m_textures[buffer];

	texture.m_texture_descriptor = texture_layout;
	texture.m_is_color = true;

	if (texture_layout.m_type == NGL_RENDERBUFFER)
	{
		texture.m_target = GL_RENDERBUFFER;

		glGenRenderbuffers(1, &texture.m_object);

		glBindRenderbuffer(GL_RENDERBUFFER, texture.m_object);

		switch (texture_layout.m_format)
		{
		case NGL_D16_UNORM:
		{
			glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, texture_layout.m_size[0], texture_layout.m_size[1]);
			texture.m_is_color = false;
			break;
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
		case NGL_TEXTURE_CUBE:
		{
			texture.m_target = GL_TEXTURE_CUBE_MAP;
			break;
		}
		}

		glGenTextures(1, &texture.m_object);

		glBindTexture(texture.m_target, texture.m_object);

		bool is_compressed = false;

		switch (texture_layout.m_format)
		{
		case NGL_R8_G8_B8_UNORM:
		{
			texture.m_GLinternalformat = GL_RGB;
			texture.m_GLformat = GL_RGB;
			texture.m_GLtype = GL_UNSIGNED_BYTE;
			break;
		}
		case NGL_R8_G8_B8_A8_UNORM:
		{
			texture.m_GLinternalformat = GL_RGBA;
			texture.m_GLformat = GL_RGBA;
			texture.m_GLtype = GL_UNSIGNED_BYTE;
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
			texture.m_GLinternalformat = GL_DEPTH_COMPONENT24_OES;
			texture.m_GLformat = GL_DEPTH_COMPONENT;
			texture.m_GLtype = GL_UNSIGNED_INT;
			texture.m_is_color = false;
			break;
		}
#if 0

		// compressed
		case _texture_layout::DXT5:
		{
			is_compressed = true;
			texture.m_GLinternalformat = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
			texture.m_GLformat = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
			break;
		}
		case _texture_layout::DXT5_SRGB:
		{
			is_compressed = true;
			texture.m_GLinternalformat = GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT;
			texture.m_GLformat = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
			break;
		}

		case _texture_layout::ETC2_RGB888:
		{
			is_compressed = true;
			texture.m_GLinternalformat = GL_COMPRESSED_RGB8_ETC2;
			texture.m_GLformat = texture.m_GLinternalformat;
			break;
		}
		case _texture_layout::ETC2_SRBG888:
		{
			is_compressed = true;
			texture.m_GLinternalformat = GL_COMPRESSED_SRGB8_ETC2;
			texture.m_GLformat = texture.m_GLinternalformat;
			break;
		}

		case _texture_layout::ETC2_RGBA8888:
		{
			is_compressed = true;
			texture.m_GLinternalformat = GL_COMPRESSED_RGBA8_ETC2_EAC;
			texture.m_GLformat = texture.m_GLinternalformat;
			break;
		}
		case _texture_layout::ETC2_SRGB888_ALPHA8:
		{
			is_compressed = true;
			texture.m_GLinternalformat = GL_COMPRESSED_SRGB8_ALPHA8_ETC2_EAC;
			texture.m_GLformat = texture.m_GLinternalformat;
			break;
		}

		case _texture_layout::ASTC_4x4_RGBA8888:
		{
			is_compressed = true;
			texture.m_GLinternalformat = GL_COMPRESSED_RGBA_ASTC_4x4_KHR;
			texture.m_GLformat = texture.m_GLinternalformat;
			break;
		}
		case _texture_layout::ASTC_4x4_SRGB888_ALPHA8:
		{
			is_compressed = true;
			texture.m_GLinternalformat = GL_COMPRESSED_SRGB8_ALPHA8_ASTC_4x4_KHR;
			texture.m_GLformat = texture.m_GLinternalformat;
			break;
		}

		case _texture_layout::ASTC_5x5_RGBA8888:
		{
			is_compressed = true;
			texture.m_GLinternalformat = GL_COMPRESSED_RGBA_ASTC_5x5_KHR;
			texture.m_GLformat = texture.m_GLinternalformat;
			break;
		}
		case _texture_layout::ASTC_5x5_SRGB888_ALPHA8:
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
		default:
		{
			_logf("ERROR: unhandled texture format!!\n");
		}
		}

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
		case NGL_ANISO:
		{
			glTexParameteri(texture.m_target, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glTexParameteri(texture.m_target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(texture.m_target, GL_TEXTURE_MAX_ANISOTROPY_EXT, 16);
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

		bool has_enough_mipmap_data = true;

		if (texture_layout.m_type == NGL_TEXTURE_2D)
		{
			uint32_t w = texture_layout.m_size[0];
			uint32_t h = texture_layout.m_size[1];

			if (datas)
			{
				for (size_t i = 0; i<datas->size(); i++)
				{
					if (is_compressed)
					{
						glCompressedTexImage2D(GL_TEXTURE_2D, i, texture.m_GLinternalformat, w, h, 0, (*datas)[i].size(), &(*datas)[i][0]);
					}
					else
					{
						glTexImage2D(GL_TEXTURE_2D, i, texture.m_GLinternalformat, w, h, 0, texture.m_GLformat, texture.m_GLtype, &(*datas)[i][0]);
					}
				}

				has_enough_mipmap_data = false;
			}
			else
			{
				glTexImage2D(GL_TEXTURE_2D, 0, texture.m_GLinternalformat, w, h, 0, texture.m_GLformat, texture.m_GLtype, 0);

				has_enough_mipmap_data = false;
			}
		}
		else if (texture_layout.m_type == NGL_TEXTURE_CUBE)
		{
			uint32_t w = texture_layout.m_size[0];
			uint32_t h = texture_layout.m_size[1];

			if (datas)
			{
				for (size_t i = 0; i < datas->size(); i++)
				{
					uint32_t level = i / 6;
					uint32_t face = i % 6;
					if (is_compressed)
					{
						glCompressedTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, level, texture.m_GLinternalformat, w, h, 0, (*datas)[i].size(), &(*datas)[i][0]);
					}
					else
					{
						glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, level, texture.m_GLinternalformat, w, h, 0, texture.m_GLformat, texture.m_GLtype, &(*datas)[i][0]);
					}
				}

				has_enough_mipmap_data = false;
			}
			else
			{
				for (uint32_t j = 0; j < 6; j++)
				{
					glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + j, 0, texture.m_GLinternalformat, w, h, 0, texture.m_GLformat, texture.m_GLtype, 0);
				}

				has_enough_mipmap_data = false;
			}
		}


		if (!has_enough_mipmap_data && texture_layout.m_filter > 1)
		{
			glGenerateMipmap(texture.m_target);
		}

		glBindTexture(texture.m_target, 0);
	}

	LOGGLERROR();

	return true;
}


bool GenVertexBuffer(uint32_t &buffer, NGL_vertex_descriptor &vertex_layout, uint32_t num, void *data)
{
	if (buffer && buffer >= OGL_instance::This->m_vertex_buffers.size())
	{
		buffer = 0xBADF00D;
		return false;
	}

	if (!buffer)
	{
		OGL_vertex_buffer vb;

		OGL_instance::This->m_vertex_buffers.push_back(vb);
		buffer = OGL_instance::This->m_vertex_buffers.size() - 1;
	}
	
	OGL_vertex_buffer &vb = OGL_instance::This->m_vertex_buffers[buffer];

	vb.m_hash = GenerateHash(vertex_layout);
	vb.m_vertex_descriptor = vertex_layout;
	vb.m_datasize = num * vertex_layout.m_stride;

	glGenBuffers(1, &vb.m_GL_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vb.m_GL_vbo);
	glBufferData(GL_ARRAY_BUFFER, vb.m_datasize, data, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	return true;
}


bool GenIndexBuffer(uint32_t &buffer, NGL_format format, uint32_t num, void *data)
{
	if (buffer && buffer >= OGL_instance::This->m_index_buffers.size())
	{
		buffer = 0xBADF00D;
		return false;
	}

	if (!buffer)
	{
		OGL_index_buffer ib;

		OGL_instance::This->m_index_buffers.push_back(ib);
		buffer = OGL_instance::This->m_index_buffers.size() - 1;
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

	glGenBuffers(1, &ib.m_ebo);
	glBindBuffer(GL_ARRAY_BUFFER, ib.m_ebo);
	glBufferData(GL_ARRAY_BUFFER, num * stride, data, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	return true;
}


uint32_t GenJob(NGL_job_descriptor &descriptor)
{
	bool use_default_fbo = true;
	
	OGL_job *job = new OGL_job;

	job->m_descriptor = descriptor;

	glGenFramebuffers(1, &job->m_fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, job->m_fbo);

	for (size_t i = 0; i<descriptor.m_color_attachments.size(); i++)
	{
		NGL_attachment_descriptor &atd = descriptor.m_color_attachments[i];
		OGL_texture &t = OGL_instance::This->m_textures[atd.m_attachment];

		if (t.m_texture_descriptor.m_format > NGL_UNDEFINED)
		{
			if (!t.m_texture_descriptor.m_is_renderable)
			{
				_logf("not renderable\n");
			}
			else if (!t.m_is_color)
			{
				_logf("type mismatch\n");
			}
			else
			{
				use_default_fbo = false;

				if (t.m_texture_descriptor.m_type == NGL_TEXTURE_2D)
				{
					glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, t.m_object, atd.m_attachment_level);
				}
				else if (t.m_texture_descriptor.m_type == NGL_TEXTURE_CUBE)
				{
					glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_CUBE_MAP_POSITIVE_X + atd.m_attachment_face, t.m_object, atd.m_attachment_level);
				}
				else if (t.m_texture_descriptor.m_type == NGL_RENDERBUFFER)
				{
					//error
				}
			}
		}
	}

	if (descriptor.m_depth_attachment.size())
	{
		NGL_attachment_descriptor &atd = descriptor.m_depth_attachment[0];
		OGL_texture &t = OGL_instance::This->m_textures[atd.m_attachment];

		if (t.m_texture_descriptor.m_format > NGL_UNDEFINED)
		{
			if (!t.m_texture_descriptor.m_is_renderable)
			{
				_logf("not renderable\n");
			}
			else if (t.m_is_color)
			{
				_logf("type mismatch\n");
			}
			else
			{
				use_default_fbo = false;

				if (t.m_texture_descriptor.m_type == NGL_TEXTURE_2D)
				{
					glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, t.m_object, atd.m_attachment_level);
				}
				else if (t.m_texture_descriptor.m_type == NGL_TEXTURE_CUBE)
				{
					glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_CUBE_MAP_POSITIVE_X + atd.m_attachment_face, t.m_object, atd.m_attachment_level);
				}
				else if (t.m_texture_descriptor.m_type == NGL_RENDERBUFFER)
				{
					glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, t.m_object);
				}
			}
		}
	}

	if (!use_default_fbo)
	{
		GLenum r = glCheckFramebufferStatus(GL_FRAMEBUFFER);
		if (r != GL_FRAMEBUFFER_COMPLETE)
		{
			LOGGLFBOERROR();
		}
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	if (use_default_fbo)
	{
		glDeleteFramebuffers(1, &job->m_fbo);
		job->m_fbo = 0;
	}

	switch (descriptor.m_primitive_type)
	{
	case NGL_POINTS:
	{
		job->m_primitive_GL_type = GL_POINTS;
		break;
	}
	case NGL_LINES:
	{
		job->m_primitive_GL_type = GL_LINES;
		break;
	}
	case NGL_TRIANGLES:
	{
		job->m_primitive_GL_type = GL_TRIANGLES;
		break;
	}
	case NGL_PATCH1:
	case NGL_PATCH2:
	case NGL_PATCH3:
	case NGL_PATCH4:
	{
		_logf("tessellation not supported");
		break;
	}
	default:
		_logf("unknown primitive type\n");
	}

	OGL_instance::This->m_jobs.push_back(job);

	return OGL_instance::This->m_jobs.size() - 1;
}


void Begin(uint32_t job_)
{
	if (job_ == NGL_FRAME)
		return;

	OGL_job *job = OGL_instance::This->m_jobs[job_];
	const NGL_job_descriptor &descriptor = job->m_descriptor;

	glBindFramebuffer(GL_FRAMEBUFFER, job->m_fbo);

	if (descriptor.m_color_attachments.size() > 1)
	{
		_logf("MRT not supported");
	}

	glViewport(descriptor.m_viewport[0], descriptor.m_viewport[1], descriptor.m_viewport[2], descriptor.m_viewport[3]);

	switch (descriptor.m_scissor_mode)
	{
	case NGL_SCISSOR_DISABLED:
	{
		glDisable(GL_SCISSOR_TEST);
		break;
	}
	case NGL_SCISSOR_PER_TASK:
	{
		glEnable(GL_SCISSOR_TEST);
		break;
	}
	case NGL_SCISSOR_PER_JOB:
	{
		glEnable(GL_SCISSOR_TEST);
		glScissor(descriptor.m_scissor[0], descriptor.m_scissor[1], descriptor.m_scissor[2], descriptor.m_scissor[3]);
		break;
	}
	}

	GLenum clear_flag = 0;

	for (size_t i = 0; i<descriptor.m_color_attachments.size(); i++)
	{
		if (descriptor.m_color_attachments[i].m_clear_flag)
		{
			clear_flag += GL_COLOR_BUFFER_BIT;
			glClearColor(
				descriptor.m_color_attachments[i].m_clear_value[0],
				descriptor.m_color_attachments[i].m_clear_value[1],
				descriptor.m_color_attachments[i].m_clear_value[2],
				descriptor.m_color_attachments[i].m_clear_value[3]
				);
			break;
		}
	}

	if (descriptor.m_depth_attachment.size() && descriptor.m_depth_attachment[0].m_clear_flag)
	{
		glClearDepthf(descriptor.m_depth_attachment[0].m_clear_value[0]);
		clear_flag += GL_DEPTH_BUFFER_BIT;
	}

	if (clear_flag)
	{
		glClear(clear_flag);
	}

	if (descriptor.m_depth_attachment.size())
	{
		switch (descriptor.m_depth_attachment[0].m_func)
		{
		case NGL_NONE:
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
			glDepthFunc(GL_EQUAL);
			glDepthRangef(1, 1);
			break;
		}
		case NGL_DEPTH_LESS_WITH_OFFSET:
		{
			glEnable(GL_DEPTH_TEST);
			glDepthFunc(GL_LESS);
			glEnable(GL_POLYGON_OFFSET_FILL);
			glPolygonOffset(1, 1);
			break;
		}
		}

		glDepthMask(descriptor.m_depth_attachment[0].m_mask[0] > 0);
	}

	for (size_t i = 0; i<descriptor.m_color_attachments.size(); i++)
	{
		switch (descriptor.m_color_attachments[i].m_func)
		{
		case NGL_NONE:
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
			glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
			break;
		}
		}

		glColorMask(
			descriptor.m_color_attachments[i].m_mask[0] > 0,
			descriptor.m_color_attachments[i].m_mask[1] > 0,
			descriptor.m_color_attachments[i].m_mask[2] > 0,
			descriptor.m_color_attachments[i].m_mask[3] > 0);
	}

	job->m_prev_renderer = 0;
}


void End(uint32_t job_)
{
	if (job_ == NGL_FRAME)
		return;

	OGL_job *job = OGL_instance::This->m_jobs[job_];
	const NGL_job_descriptor &descriptor = job->m_descriptor;

#ifdef USE_VAO
	glBindVertexArray(0);
#endif

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	glDisable(GL_POLYGON_OFFSET_FILL);
	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);
	glDisable(GL_SCISSOR_TEST);
	glColorMask(1, 1, 1, 1);
	glDepthMask(1);
	glDepthFunc(GL_LESS);
	glDepthRangef(0, 1);
}


void Draw(uint32_t job_, uint32_t shader_code, uint32_t num_vbos, uint32_t *vbos, uint32_t ebo, NGL_cull_mode cull_type, int32_t scissor[4], void** parameters)
{
	OGL_job *job = OGL_instance::This->m_jobs[job_];
	const NGL_job_descriptor &descriptor = job->m_descriptor;
	NGL_shader_hash sh;

	sh.m_shader_code = shader_code;
	sh.m_vbo_hash = 0;
	for (uint32_t i = 0; i<num_vbos; i++)
	{
		sh.m_vbo_hash += OGL_instance::This->m_vertex_buffers[vbos[i]].m_hash;
	}

	OGL_renderer *renderer = (OGL_renderer *)job->FindRenderer(sh, num_vbos, vbos);

	glUseProgram(renderer->m_p);

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

	renderer->m_used_texture_slots = 0;
	renderer->m_used_image_slots = 0;

	for (size_t i = 0; i<renderer->m_used_uniforms.size(); i++)
	{
		const NGL_used_uniform &uu = renderer->m_used_uniforms[i];

		if (uu.m_application_location > -1 && parameters[uu.m_application_location])
		{
			renderer->BindUniform(uu, parameters[uu.m_application_location]);
		}
	}

#ifdef USE_VAO
	glBindVertexArray(RM->m_global_vao);
#endif
	OGL_index_buffer &ib = OGL_instance::This->m_index_buffers[ebo];

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ib.m_ebo);

	for (size_t j = 0; j<renderer->m_used_vbs.size(); j++)
	{
		OGL_renderer::_used_vertex_buffer &uvb = renderer->m_used_vbs[j];
		OGL_vertex_buffer &vb = OGL_instance::This->m_vertex_buffers[vbos[uvb.m_buffer_idx]];

		glBindBuffer(GL_ARRAY_BUFFER, vb.m_GL_vbo);

		for (size_t i = 0; i<uvb.m_GL_layouts.size(); i++)
		{
			OGL_renderer::_GL_layout &l = uvb.m_GL_layouts[i];

			glEnableVertexAttribArray(l.loc);
			glVertexAttribPointer(
				l.loc,
				l.size,
				l.type,
				l.normalized,
				vb.m_vertex_descriptor.m_stride,
				(void*)l.offset
				);
		}
	}

	if (descriptor.m_scissor_mode == NGL_SCISSOR_PER_TASK)
	{
		glScissor(scissor[0], scissor[1], scissor[2], scissor[3]);
	}

	glDrawElements(job->m_primitive_GL_type, ib.m_num_indices, ib.m_GL_data_type, 0);

	for (size_t j = 0; j<renderer->m_used_vbs.size(); j++)
	{
		OGL_renderer::_used_vertex_buffer &uvb = renderer->m_used_vbs[j];

		for (size_t i = 0; i<uvb.m_GL_layouts.size(); i++)
		{
			OGL_renderer::_GL_layout &l = uvb.m_GL_layouts[i];

			glDisableVertexAttribArray(l.loc);
		}
	}

	job->m_prev_renderer = renderer;
}


void Dispatch(uint32_t job_, uint32_t shader_code, uint32_t x, uint32_t y, uint32_t z, void** parameters)
{
	_logf("compute not supported");
}


void UpdateViewport(uint32_t job_, int32_t vp[4])
{
	OGL_job *job = OGL_instance::This->m_jobs[job_];
	NGL_job_descriptor &descriptor = job->m_descriptor;

	memcpy(descriptor.m_viewport, vp, 4 * 4);
}


void UpdateScissor(uint32_t job_, int32_t vp[4])
{
	OGL_job *job = OGL_instance::This->m_jobs[job_];
	NGL_job_descriptor &descriptor = job->m_descriptor;

	memcpy(descriptor.m_scissor, vp, 4 * 4);
}


NGL_api GetApi()
{
	return NGL_OPENGL_ES2;
}


void DeletePipelines(uint32_t job_)
{
	OGL_job *job = OGL_instance::This->m_jobs[job_];
	
	job->DeleteRenderers();
}


void CustomAction(uint32_t job_, uint32_t parameter)
{
	if (parameter == 100)
	{
		glUseProgram(0);
		glActiveTexture(GL_TEXTURE0);
	}
	if (parameter == 101)
	{
		glFinish();
	}
}


void SetLineWidth(uint32_t job_, float width)
{
	glLineWidth(width);
}


bool GetTextureContent(uint32_t texture_, uint32_t level, uint32_t layer, uint32_t face, NGL_format format, uint32_t &width, uint32_t &height, std::vector<uint8_t> &data)
{
	if (texture_)
	{
		_logf("oglGetTextureContent not supported");
	}
	else
	{
		int vp[4];

		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		glGetIntegerv(GL_VIEWPORT, vp);

		width = vp[2] - vp[0];
		height = vp[3] - vp[1];
		data.resize(width * height * 4);

		glReadPixels(vp[0], vp[1], vp[2], vp[3], GL_RGBA, GL_UNSIGNED_BYTE, (void*)&data[0]);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	return true;
}

OGL_instance *OGL_instance::This = 0;


OGL_instance::OGL_instance()
{
	This = this;

	nglGenJob = GenJob;
	nglBegin = Begin;
	nglEnd = End;
	nglDraw = Draw;
	nglDispatch = Dispatch;
	nglGenTexture = GenTexture;
	nglGenVertexBuffer = GenVertexBuffer;
	nglGenIndexBuffer = GenIndexBuffer;
	nglUpdateViewport = UpdateViewport;
	nglUpdateScissor = UpdateScissor;
	nglGetApi = GetApi;
	nglDeletePipelines = DeletePipelines;
	nglCustomAction = CustomAction;
	nglSetLineWidth = SetLineWidth;
	nglGetTextureContent = GetTextureContent;

	{
		//a nullas attachment invalid, mert a m_format == 0
		OGL_texture t;
		memset(&t, 0, sizeof(OGL_texture));
		m_textures.push_back(t);
	}
}


void OGL_instance::Init()
{
#ifdef USE_VAO
	glGenVertexArrays(1, &m_global_vao);
#endif

#ifdef GL_TEXTURE_CUBE_MAP_SEAMLESS
	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
#endif
#ifdef GL_VERTEX_PROGRAM_POINT_SIZE
	glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
#endif
#ifdef GL_PERSPECTIVE_CORRECTION_HINT
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
#endif
#ifdef GL_FRAGMENT_SHADER_DERIVATIVE_HINT
	glHint(GL_FRAGMENT_SHADER_DERIVATIVE_HINT, GL_NICEST);
#endif
#ifdef GL_GENERATE_MIPMAP_HINT
	glHint(GL_GENERATE_MIPMAP_HINT, GL_NICEST);
#endif

	glLineWidth(1);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glPixelStorei(GL_PACK_ALIGNMENT, 1);

#ifdef PLATFORM_IS_WINDOWS
	glewInit();
#endif
}
}


void nglCreateContextOGLES2(std::vector<void*> *user_data)
{
	new OGLES2::OGL_instance;

	if (user_data)
	{
		OGLES2::OGL_instance::This->m_user_data = *user_data;
	}

	OGLES2::OGL_instance::This->Init();
}
