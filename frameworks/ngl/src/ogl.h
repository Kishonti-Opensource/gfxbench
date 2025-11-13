/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
struct OGL_texture : public NGL_texture
{
	uint32_t m_object;
	uint32_t m_target;
	uint32_t m_GLinternalformat;
	uint32_t m_GLformat;
	uint32_t m_GLtype;
	bool m_shadow_sampler;

	OGL_texture()
	{
		m_object = 0;
		m_target = 0;
		m_GLinternalformat = 0;
		m_GLformat = 0;
		m_GLtype = 0;
		m_shadow_sampler = false;
	}
};


struct OGL_vertex_buffer : public NGL_vertex_buffer
{
	uint32_t m_GL_vbo;

	OGL_vertex_buffer()
	{
		m_GL_vbo = 0;
	}
};


struct OGL_index_buffer : public NGL_index_buffer
{
	uint32_t m_ebo;
	uint32_t m_GL_data_type;

	OGL_index_buffer()
	{
		m_ebo = 0;
		m_GL_data_type = 0;
	}
};


struct OGL_renderer : public NGL_renderer
{
	struct _GL_layout
	{
		uint32_t loc;
		uint32_t size;
		uint32_t type;
		uint32_t normalized;
		uint32_t offset;
	};

	struct _used_vertex_buffer
	{
		uint32_t m_buffer_idx;
		std::vector<_GL_layout> m_GL_layouts;
	};

	uint32_t m_p;
	std::vector<_used_vertex_buffer> m_used_vbs;
	std::vector<uint32_t> m_used_sampler_slots;
	std::vector<uint32_t> m_used_image_slots;
	std::vector<bool> m_used_slot_readonly;
	GLbitfield m_barriers;

	OGL_renderer();
	~OGL_renderer();

	static uint32_t CompileShader(uint32_t shader_type, const char *str, std::string &info_string);
	bool LinkShader();
	bool GetActiveAttribs(uint32_t num_vbos, uint32_t *vbos);
	void GetActiveResources(std::vector<NGL_shader_uniform> &application_uniforms, NGL_shader_source_descriptor ssd[NGL_NUM_SHADER_TYPES]);
	void BindUniform(const NGL_used_uniform &uu, const void *ptr);
};


struct OGL_subpass
{
	struct _clear_data
	{
		GLint m_attachment_index;
		float m_value[4];
		_clear_data(const uint32_t attachment_index) : m_attachment_index(attachment_index)
		{
		}
	};

	uint32_t m_fbo;
	std::string m_name;
	std::vector<uint32_t> m_color_attachments_remap;
	std::vector<uint32_t> m_depth_attachments_remap;
	std::vector<NGL_attachment_descriptor> m_color_attachments;
	std::vector<NGL_attachment_descriptor> m_depth_attachments;
	std::vector<_clear_data> m_color_clears;
	std::vector<_clear_data> m_depth_clear;
	std::vector<GLenum> m_load_discards;
	std::vector<GLenum> m_store_discards;
	bool m_independent_clear;

	OGL_subpass() : m_fbo(0), m_independent_clear(false)
	{
	}

	void ClearAttachments() const;
	void DiscardAttachments() const;
};


struct OGL_job : public NGL_job
{
	bool m_is_started;
	std::vector<OGL_subpass> m_subpasses;

	OGL_job() : m_is_started(false)
	{
	}

	NGL_renderer* CreateRenderer(NGL_state &sh, uint32_t num_vbos, uint32_t *vbos);
	
	void CreatePasses()
	{
		std::vector<bool> is_cleared(m_descriptor.m_attachments.size(), false);
		std::vector<bool> is_discarded(m_descriptor.m_attachments.size(), false);

		m_subpasses.resize(m_descriptor.m_subpasses.size());

		for (size_t sp_idx = 0; sp_idx < m_subpasses.size(); sp_idx++)
		{
			NGL_subpass &src_sp = m_descriptor.m_subpasses[sp_idx];
			OGL_subpass &dst_sp = m_subpasses[sp_idx];
			bool is_last_pass = true;
	
			if (sp_idx < m_subpasses.size() - 1)
			{
				is_last_pass = false;
			}

			dst_sp.m_name = src_sp.m_name;

			for (size_t reference = 0; reference < src_sp.m_usages.size(); reference++)
			{
				const NGL_resource_state &f = src_sp.m_usages[reference];
				const NGL_attachment_descriptor &atd = m_descriptor.m_attachments[reference];

				if (f == NGL_COLOR_ATTACHMENT)
				{
					uint32_t j = (uint32_t)dst_sp.m_color_attachments.size();

					if (atd.m_attachment_load_op == NGL_LOAD_OP_CLEAR)
					{
						if (!is_cleared[reference])
						{
							dst_sp.m_color_clears.push_back(OGL_subpass::_clear_data(j));
							is_cleared[reference] = true;
						}
					}
					else if (atd.m_attachment_load_op == NGL_LOAD_OP_DONT_CARE)
					{
						if (!is_discarded[reference])
						{
							dst_sp.m_load_discards.push_back(GL_COLOR_ATTACHMENT0 + j);
							is_discarded[reference] = true;
						}
					}

					if (atd.m_attachment_store_op == NGL_STORE_OP_DONT_CARE)
					{
						if (is_last_pass)
						{
							dst_sp.m_store_discards.push_back(GL_COLOR_ATTACHMENT0 + j);
						}
					}

					dst_sp.m_color_attachments.push_back(atd);
					dst_sp.m_color_attachments_remap.push_back((uint32_t)reference);
				}
				else if (f == NGL_DEPTH_ATTACHMENT || f == NGL_READ_ONLY_DEPTH_ATTACHMENT || f == NGL_READ_ONLY_DEPTH_ATTACHMENT_AND_SHADER_RESOURCE)
				{
					uint32_t j = (uint32_t)dst_sp.m_depth_attachments.size();

					if (atd.m_attachment_load_op == NGL_LOAD_OP_CLEAR)
					{
						if (!is_cleared[reference])
						{
							dst_sp.m_depth_clear.push_back(OGL_subpass::_clear_data(j));
							is_cleared[reference] = true;
						}
					}
					else if (atd.m_attachment_load_op == NGL_LOAD_OP_DONT_CARE)
					{
						if (!is_discarded[reference])
						{
							dst_sp.m_load_discards.push_back(GL_DEPTH_ATTACHMENT);
							is_discarded[reference] = true;
						}
					}
					if (atd.m_attachment_store_op == NGL_STORE_OP_DONT_CARE)
					{
						if (is_last_pass)
						{
							dst_sp.m_store_discards.push_back(GL_DEPTH_ATTACHMENT);
						}
					}

					dst_sp.m_depth_attachments.push_back(atd);
					dst_sp.m_depth_attachments_remap.push_back((uint32_t)reference);
				}
			}
		}
	}

	OGL_subpass& F()
	{
		return m_subpasses[m_current_state.m_subpass - 1];
	}
	OGL_subpass& G()
	{
		return m_subpasses[m_current_state.m_subpass];
	}
};


struct OGL_instance : public NGL_instance
{
	std::vector<OGL_vertex_buffer> m_vertex_buffers;
	std::vector<OGL_index_buffer> m_index_buffers;
	std::vector<OGL_texture> m_textures;
	std::vector<OGL_job*> m_jobs;
	std::vector<uint32_t> m_default_fbos;

	NGL_gl_adapter_interface *m_ngl_adapter;
	int32_t m_GL_MAX_VERTEX_ATTRIBS;
	int32_t m_GL_MAX_VERTEX_UNIFORM_VECTORS;
	int32_t m_GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS;
	int32_t m_GL_MAX_VARYING_VECTORS;
	int32_t m_GL_MAX_FRAGMENT_UNIFORM_VECTORS;
	int32_t m_GL_MAX_TEXTURE_IMAGE_UNITS;
	std::string m_extension_list;
	uint32_t m_global_vao;
	uint32_t m_default_fbo;
	bool m_validation_layer_enabled;

	OGL_instance();
	~OGL_instance();
	static OGL_instance *This;
	NGLStatistic* m_statistic_vector;

	void Init(NGL_api api, uint32_t major_version, uint32_t minor_version, bool enable_validation);

	void GetPassClearValues(OGL_subpass &pass)
	{
		for (size_t i = 0; i < pass.m_color_clears.size(); i++)
		{
			OGL_subpass::_clear_data &cd = pass.m_color_clears[i];
			const NGL_attachment_descriptor &atd = pass.m_color_attachments[cd.m_attachment_index];
			OGL_texture &texture = m_textures[atd.m_attachment.m_idx];

			cd.m_value[0] = texture.m_texture_descriptor.m_clear_value[0];
			cd.m_value[1] = texture.m_texture_descriptor.m_clear_value[1];
			cd.m_value[2] = texture.m_texture_descriptor.m_clear_value[2];
			cd.m_value[3] = texture.m_texture_descriptor.m_clear_value[3];
		}
		
		for (size_t i = 0; i < pass.m_depth_clear.size(); i++)
		{
			OGL_subpass::_clear_data &cd = pass.m_depth_clear[i];
			const NGL_attachment_descriptor &atd = pass.m_depth_attachments[cd.m_attachment_index];
			OGL_texture &texture = m_textures[atd.m_attachment.m_idx];

			cd.m_value[0] = texture.m_texture_descriptor.m_clear_value[0];
			cd.m_value[1] = texture.m_texture_descriptor.m_clear_value[1];
			cd.m_value[2] = texture.m_texture_descriptor.m_clear_value[2];
			cd.m_value[3] = texture.m_texture_descriptor.m_clear_value[3];
		}

		if (pass.m_color_clears.size())
		{
			if (pass.m_color_clears.size() == pass.m_color_attachments.size())
			{
				for (size_t j = 1; j < pass.m_color_clears.size(); j++)
				{
					if (memcmp(pass.m_color_clears[0].m_value, pass.m_color_clears[j].m_value, sizeof(float) * 4))
					{
						pass.m_independent_clear = true;
						break;
					}
				}
			}
			else
			{
				pass.m_independent_clear = true;
			}
		}
	}

	void SetDebugLabel(GLenum object_type, GLuint object_id, const std::string &name);
	static void GFXB_APIENTRY DebugCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam);
};


#ifndef GL_STACK_OVERFLOW
#define GL_STACK_OVERFLOW 0x0503
#endif
#ifndef GL_STACK_UNDERFLOW
#define GL_STACK_UNDERFLOW 0x0504
#endif
#ifndef GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS
#define GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS 0x8CD9
#endif

#define LOGGLERROR() LogGlError( __FUNCTION__, __LINE__)
#define LOGGLFBOERROR() LogGLFBOError( __FUNCTION__, __LINE__)


inline bool LogGlError(const char* fn_name, int line)
{
	GLenum e;
	const char *msg = "";

	e = glGetError();

	switch (e)
	{
	case GL_NO_ERROR:
	{
		return false;
	}
	case GL_INVALID_ENUM:
	{
		msg = "GL_INVALID_ENUM";
		break;
	}
	case GL_INVALID_VALUE:
	{
		msg = "GL_INVALID_VALUE";
		break;
	}
	case GL_INVALID_OPERATION:
	{
		msg = "GL_INVALID_OPERATION";
		break;
	}
	case GL_STACK_OVERFLOW:
	{
		msg = "GL_STACK_OVERFLOW";
		break;
	}
	case GL_STACK_UNDERFLOW:
	{
		msg = "GL_STACK_UNDERFLOW";
		break;
	}
	case GL_OUT_OF_MEMORY:
	{
		msg = "GL_OUT_OF_MEMORY";
		break;
	}
	case GL_INVALID_FRAMEBUFFER_OPERATION:
	{
		msg = "GL_INVALID_FRAMEBUFFER_OPERATION";
		break;
	}
	default:
	{
		msg = "unknown GL error";
	}
	}

	_logf("glGetError: %s - %d, %s", msg, line, fn_name);

	return true;
}


inline bool LogGLFBOError(const char* fn_name, int line)
{
	GLenum status;
	const char *msg = "";

	status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

	switch (status)
	{
	case GL_FRAMEBUFFER_COMPLETE:
	{
		return false;
	}
	case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
	{
		msg = "GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT";
		break;
	}
	case GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS:
	{
		msg = "GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS";
		break;
	}
	case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
	{
		msg = "GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT";
		break;
	}
	case GL_FRAMEBUFFER_UNSUPPORTED:
	{
		msg = "GL_FRAMEBUFFER_UNSUPPORTED";
		break;
	}
	default:
	{
		msg = "unknown FBO GL error";
		break;
	}
	}

	_logf("glCheckFramebufferStatus: %s - %d, %s", msg, line, fn_name);

	return true;
}


inline bool CreateJobFBO(OGL_job *job)
{
	bool success = true;

	for (size_t j = 0; j < job->m_subpasses.size(); j++)
	{
		OGL_subpass &g = job->m_subpasses[j];

		bool use_default_fbo = true;

		if (g.m_fbo)
		{
			glDeleteFramebuffers(1, &g.m_fbo);
		}

		glGenFramebuffers(1, &g.m_fbo);
		glBindFramebuffer(GL_FRAMEBUFFER, g.m_fbo);

		OGL_instance::This->SetDebugLabel(GL_FRAMEBUFFER, g.m_fbo, g.m_name + "_fbo");
		for (size_t i = 0; i<g.m_color_attachments.size(); i++)
		{
			const NGL_attachment_descriptor &atd = g.m_color_attachments[i];
			OGL_texture &t = OGL_instance::This->m_textures[atd.m_attachment.m_idx];

			if (t.m_texture_descriptor.m_format > NGL_UNDEFINED)
			{
				if (!t.m_texture_descriptor.m_is_renderable)
				{
					_logf("not renderable\n");
					success = false;
				}
				else if (!t.m_is_color)
				{
					_logf("type mismatch\n");
					success = false;
				}
				else
				{
					use_default_fbo = false;

					if (t.m_texture_descriptor.m_type == NGL_TEXTURE_2D)
					{
						glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + (uint32_t)i, GL_TEXTURE_2D, t.m_object, atd.m_attachment.m_level);
					}
					else if (t.m_texture_descriptor.m_type == NGL_TEXTURE_2D_ARRAY)
					{
						glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + (uint32_t)i, t.m_object, atd.m_attachment.m_level, atd.m_attachment.m_layer);
					}
					else if (t.m_texture_descriptor.m_type == NGL_TEXTURE_CUBE)
					{
						glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + (uint32_t)i, GL_TEXTURE_CUBE_MAP_POSITIVE_X + atd.m_attachment.m_face, t.m_object, atd.m_attachment.m_level);
					}
					else if (t.m_texture_descriptor.m_type == NGL_RENDERBUFFER)
					{
						//error
						assert(0);
					}
					else
					{
						assert(0);
					}
				}

				job->m_current_state.m_viewport[0] = 0;
				job->m_current_state.m_viewport[1] = 0;
				job->m_current_state.m_viewport[2] = t.m_texture_descriptor.m_size[0] / (1 << atd.m_attachment.m_level);
				job->m_current_state.m_viewport[3] = t.m_texture_descriptor.m_size[1] / (1 << atd.m_attachment.m_level);

				job->m_current_state.m_scissor[0] = 0;
				job->m_current_state.m_scissor[1] = 0;
				job->m_current_state.m_scissor[2] = t.m_texture_descriptor.m_size[0] / (1 << atd.m_attachment.m_level);
				job->m_current_state.m_scissor[3] = t.m_texture_descriptor.m_size[1] / (1 << atd.m_attachment.m_level);
			}
		}

		for (size_t i = 0; i<g.m_depth_attachments.size(); i++)
		{
			const NGL_attachment_descriptor &atd = g.m_depth_attachments[i];
			OGL_texture &t = OGL_instance::This->m_textures[atd.m_attachment.m_idx];

			if (t.m_texture_descriptor.m_format > NGL_UNDEFINED)
			{
				if (!t.m_texture_descriptor.m_is_renderable)
				{
					_logf("not renderable\n");
					success = false;
				}
				else if (t.m_is_color)
				{
					_logf("type mismatch\n");
					success = false;
				}
				else
				{
					use_default_fbo = false;

					if (t.m_texture_descriptor.m_type == NGL_TEXTURE_2D)
					{
						glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, t.m_object, atd.m_attachment.m_level);
					}
					else if (t.m_texture_descriptor.m_type == NGL_TEXTURE_2D_ARRAY)
					{
						glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, t.m_object, atd.m_attachment.m_level, atd.m_attachment.m_layer);
					}
					else if (t.m_texture_descriptor.m_type == NGL_TEXTURE_CUBE)
					{
						glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_CUBE_MAP_POSITIVE_X + atd.m_attachment.m_face, t.m_object, atd.m_attachment.m_level);
					}
					else if (t.m_texture_descriptor.m_type == NGL_RENDERBUFFER)
					{
						glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, t.m_object);
					}
					else
					{
						_logf("Unhandled depth texture type: %d", t.m_texture_descriptor.m_type);
						assert(0);
					}
				}

				job->m_current_state.m_viewport[0] = 0;
				job->m_current_state.m_viewport[1] = 0;
				job->m_current_state.m_viewport[2] = t.m_texture_descriptor.m_size[0] / (1 << atd.m_attachment.m_level);
				job->m_current_state.m_viewport[3] = t.m_texture_descriptor.m_size[1] / (1 << atd.m_attachment.m_level);

				job->m_current_state.m_scissor[0] = 0;
				job->m_current_state.m_scissor[1] = 0;
				job->m_current_state.m_scissor[2] = t.m_texture_descriptor.m_size[0] / (1 << atd.m_attachment.m_level);
				job->m_current_state.m_scissor[3] = t.m_texture_descriptor.m_size[1] / (1 << atd.m_attachment.m_level);
			}
			else
			{
				if (!use_default_fbo)
				{
					assert(0);
				}
			}
		}

		if (!use_default_fbo)
		{
			GLenum r = glCheckFramebufferStatus(GL_FRAMEBUFFER);
			if (r != GL_FRAMEBUFFER_COMPLETE)
			{
				LOGGLFBOERROR();
				_logf("FBO error in %s\n", g.m_name.c_str());
			}

			if (g.m_color_attachments.size() > 1)
			{
#ifdef GL_COLOR_ATTACHMENT1
				GLenum bufs[] =
				{
					GL_COLOR_ATTACHMENT0,
					GL_COLOR_ATTACHMENT1,
					GL_COLOR_ATTACHMENT2,
					GL_COLOR_ATTACHMENT3,
					GL_COLOR_ATTACHMENT4,
					GL_COLOR_ATTACHMENT5,
					GL_COLOR_ATTACHMENT6,
					GL_COLOR_ATTACHMENT7
				};
				glDrawBuffers((uint32_t)g.m_color_attachments.size(), bufs);
#endif
			}
		}

		glBindFramebuffer(GL_FRAMEBUFFER, OGL_instance::This->m_default_fbo);

		if (use_default_fbo)
		{
			glDeleteFramebuffers(1, &g.m_fbo);
			g.m_fbo = 0;
		}
	}

	return success;
}
