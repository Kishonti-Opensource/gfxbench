/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
extern PFNLOGF _logf;

struct NGL_texture
{
	NGL_texture_descriptor m_texture_descriptor;
	bool m_is_color;

	NGL_texture()
		: m_is_color(true)
	{
	}

	virtual ~NGL_texture() {}
};

struct NGL_vertex_buffer
{
	NGL_vertex_descriptor m_vertex_descriptor;
	uint32_t m_datasize;
	uint32_t m_hash;
	size_t m_memory_pool_offset;

	virtual ~NGL_vertex_buffer() {}
};


struct NGL_index_buffer
{
	NGL_format m_format;
	uint32_t m_num_indices;
	uint32_t m_datasize;
	size_t m_memory_pool_offset;

	virtual ~NGL_index_buffer() {}
};


enum NGL_state_mask
{
	NGL_SHADER_MASK = (1 << 0),
	NGL_SUBPASS_MASK = (1 << 1),
	NGL_PRIMITIVE_TYPE_MASK = (1 << 2),
	NGL_CULL_MODE_MASK = (1 << 3),
	NGL_COLOR_BLEND_FUNCS_MASK = (1 << 4),
	NGL_COLOR_MASKS_MASK = (1 << 5),
	NGL_DEPTH_FUNC_MASK = (1 << 6),
	NGL_DEPTH_MASK_MASK = (1 << 7),
	NGL_VIEWPORT_MASK = (1 << 8),
	NGL_SCISSOR_MASK = (1 << 9),
	NGL_HEAVY_STATES = NGL_SHADER_MASK | NGL_SUBPASS_MASK | NGL_PRIMITIVE_TYPE_MASK | NGL_CULL_MODE_MASK | NGL_COLOR_BLEND_FUNCS_MASK | NGL_COLOR_MASKS_MASK | NGL_DEPTH_FUNC_MASK | NGL_DEPTH_MASK_MASK
};


struct NGL_state
{
	struct
	{
		uint32_t m_vbo_hash;
		uint32_t m_shader_code;
	}m_shader;
	uint32_t m_subpass;
	NGL_primitive_type m_primitive_type;
	NGL_cull_mode m_cull_mode;
	struct
	{
		NGL_blend_func m_funcs[8];
		NGL_color_channel_mask m_masks[8];
	}m_blend_state;
	struct
	{
		NGL_depth_func m_func;
		bool m_mask;
	}m_depth_state;
	int32_t m_viewport[4];
	int32_t m_scissor[4];

	static uint32_t ChangedMask(const NGL_state &A, const NGL_state &B)
	{
		uint32_t result = 0;

		result |= (memcmp(&A.m_shader, &B.m_shader, sizeof(uint32_t) * 2) != 0) * NGL_SHADER_MASK;
		result |= (memcmp(&A.m_subpass, &B.m_subpass, sizeof(uint32_t)) != 0) * NGL_SUBPASS_MASK;
		result |= (A.m_primitive_type != B.m_primitive_type) * NGL_PRIMITIVE_TYPE_MASK;
		result |= (A.m_cull_mode != B.m_cull_mode) * NGL_CULL_MODE_MASK;
		result |= (memcmp(&A.m_blend_state.m_funcs, &B.m_blend_state.m_funcs, sizeof(NGL_blend_func) * 8) != 0) * NGL_COLOR_BLEND_FUNCS_MASK;
		result |= (memcmp(&A.m_blend_state.m_masks, &B.m_blend_state.m_masks, sizeof(NGL_color_channel_mask) * 8) != 0) * NGL_COLOR_MASKS_MASK;
		result |= (A.m_depth_state.m_func != B.m_depth_state.m_func) * NGL_DEPTH_FUNC_MASK;
		result |= (A.m_depth_state.m_mask != B.m_depth_state.m_mask) * NGL_DEPTH_MASK_MASK;
		result |= (memcmp(&A.m_viewport, &B.m_viewport, sizeof(int32_t) * 4) != 0) * NGL_VIEWPORT_MASK;
		result |= (memcmp(&A.m_scissor, &B.m_scissor, sizeof(int32_t) * 4) != 0) * NGL_SCISSOR_MASK;

		return result;
	}
	NGL_state()
	{
		memset(this, 0, sizeof(NGL_state));
		m_cull_mode = NGL_TWO_SIDED;
		for (int i = 0; i < 8; i++)
		{
			m_blend_state.m_masks[i] = NGL_CHANNEL_ALL;
		}
		m_depth_state.m_mask = true;
	}
};


struct NGL_used_uniform
{
	uint32_t m_index;
	NGL_shader_uniform m_uniform;
	uint32_t m_binding_type;
	int32_t m_shader_location[NGL_NUM_SHADER_TYPES];
	int32_t m_application_location;
	NGL_used_uniform()
	{
		m_index = 0;
		m_binding_type = 0;
		m_application_location = -1;
		for (int i = 0; i<NGL_NUM_SHADER_TYPES; i++) m_shader_location[i] = -1;
	}
};


struct NGL_renderer
{
	NGL_state m_my_state;
	std::vector<NGL_used_uniform> m_used_uniforms[4];

	virtual ~NGL_renderer() {};
};


struct NGL_job
{
	uint32_t m_idx;
	NGL_job_descriptor m_descriptor;
	std::vector<NGL_renderer*> m_renderers;
	NGL_state m_previous_state;
	NGL_state m_current_state;
	NGL_renderer *m_active_renderer;
	uint32_t m_cmd_buffer_idx;
	bool m_onscreen;

	NGL_job() : m_idx(0), m_active_renderer(0), m_cmd_buffer_idx(~0), m_onscreen(false)
	{
	}
	virtual ~NGL_job()
	{
		DeleteRenderers();
	}

	virtual NGL_renderer* CreateRenderer(NGL_state &sh, uint32_t num_vbos, uint32_t *vbos) = 0;

	void DeleteRenderers()
	{
		for (size_t i = 0; i < m_renderers.size(); i++)
		{
			delete m_renderers[i];
		}

		m_renderers.clear();
	}
};


template<typename T> struct NGL_memory
{
	T m_memory;
	size_t m_size;
	size_t m_offset;

	NGL_memory() : m_size(0), m_offset(0)
	{
	}

	bool TestAllocate(size_t size, size_t alignment)
	{
		size_t o = (m_offset + alignment - 1) & ~(alignment - 1);
		o += size;
		return o < m_size;
	}
	size_t Allocate(size_t size, size_t alignment)
	{
		m_offset = (m_offset + alignment - 1) & ~(alignment - 1);
		size_t o = m_offset;

		m_offset += size;


		return o;
	}
};


struct NGL_instance
{
	std::string m_propertiess[NGL_NUM_PROPERTIES];
	int32_t m_propertiesi[NGL_NUM_PROPERTIES];
	NGL_context_descriptor m_context_descriptor;
	 
	virtual ~NGL_instance()
	{
	}
};


inline bool SearchAttribBySemanticAndSize(NGL_vertex_descriptor &vd, uint32_t &idx, const std::string &semantic_name, uint32_t size)
{
	for (size_t i = 0; i<vd.m_attribs.size(); i++)
	{
		uint32_t current_idx = (uint32_t)i;

		if (vd.m_attribs[i].m_semantic == semantic_name)
		{
			switch (size)
			{
			case 1:
			{
				if (vd.m_attribs[i].m_format == NGL_R32_FLOAT)
				{
					idx = current_idx;
					return true;
				}
				return false;
			}
			case 2:
			{
				if (vd.m_attribs[i].m_format == NGL_R32_G32_FLOAT)
				{
					idx = current_idx;
					return true;
				}
				return false;
			}
			case 3:
			{
				if (vd.m_attribs[i].m_format == NGL_R32_G32_B32_FLOAT)
				{
					idx = current_idx;
					return true;
				}
				return false;
			}
			case 4:
			{
				if (vd.m_attribs[i].m_format == NGL_R32_G32_B32_A32_FLOAT)
				{
					idx = current_idx;
					return true;
				}
				if (vd.m_attribs[i].m_format == NGL_R8_G8_B8_A8_UINT)
				{
					idx = current_idx;
					return true;
				}
				return false;
			}
			}
		}
	}

	return false;
}


inline uint32_t GenerateHash(NGL_vertex_descriptor &vd)
{
	uint32_t hash = vd.m_stride;

	for (size_t i = 0; i<vd.m_attribs.size(); i++)
	{
		NGL_vertex_attrib &a = vd.m_attribs[i];

		hash += a.m_offset;
		hash += a.m_format;
	}

	return hash;
}


inline size_t ValidateSubpassses(NGL_job_descriptor &d)
{
	size_t num_subpasses = 0;

#if 0
	if (d.m_color_attachments.size())
	{
		num_subpasses = d.m_color_attachments[0].m_attachment_usages.size();
	}
	else if (d.m_depth_attachment.size())
	{
		num_subpasses = d.m_depth_attachment[0].m_attachment_usages.size();
	}

	if (!num_subpasses)
	{
		_logf("zero number of subpass in %s\n", d.m_name.c_str());
		return 0;
	}

	for (size_t i = 0; i < d.m_color_attachments.size(); i++)
	{
		if (d.m_color_attachments[i].m_attachment_usages.size() != num_subpasses)
		{
			_logf("subpass setup mismatch (color-%u) in %s\n", i, d.m_name.c_str());
			return 0;
		}
	}
	for (size_t i = 0; i < d.m_depth_attachment.size(); i++)
	{
		if (d.m_depth_attachment[i].m_attachment_usages.size() != num_subpasses)
		{
			_logf("subpass setup mismatch (depth-%u) in %s\n", i, d.m_name.c_str());
			return 0;
		}
	}
#endif

	return num_subpasses;
}


struct NGL_texture_subresource_transition_range : public NGL_texture_subresource_transition
{
	uint32_t m_base_level;
	uint32_t m_base_face;
	uint32_t m_num_levels;
	uint32_t m_num_faces;

	NGL_texture_subresource_transition_range(const NGL_texture_subresource_transition &transition, uint32_t base_level, uint32_t base_face)
		: NGL_texture_subresource_transition(transition), m_base_level(base_level), m_base_face(base_face), m_num_levels(1), m_num_faces(1)
	{ }
};


template <typename T> struct NGL_texture_barrier_merger
{
	std::vector<NGL_texture_subresource_transition_range> m_ranges;

	NGL_texture_barrier_merger(std::vector<NGL_texture_subresource_transition> &texture_barriers, std::vector<T> &textures)
	{
		m_ranges.reserve(texture_barriers.size());

		for (size_t i = 0; i < texture_barriers.size(); i++)
		{
			const NGL_texture_subresource_transition &transition = texture_barriers[i];
			T &texture = textures[transition.m_texture.m_idx];

			uint32_t face = transition.m_texture.m_layer;
			if (texture.m_texture_descriptor.m_type == NGL_TEXTURE_CUBE)
			{
				face = transition.m_texture.m_layer * 6 + transition.m_texture.m_face;
			}

			bool start_new_range = true;

			if (m_ranges.size() > 0)
			{
				NGL_texture_subresource_transition_range &last_range = m_ranges.back();

				if (transition.m_texture.m_idx == last_range.m_texture.m_idx
					&& transition.m_old_state == last_range.m_old_state
					&& transition.m_new_state == last_range.m_new_state)
				{
					if (last_range.m_base_face == face
						&& last_range.m_num_faces == 1
						&& last_range.m_base_level + last_range.m_num_levels == transition.m_texture.m_level)
					{
						last_range.m_num_levels++;
						start_new_range = false;
					}
					else if (last_range.m_base_level == transition.m_texture.m_level
						&& last_range.m_num_levels == 1
						&& last_range.m_base_face + last_range.m_num_faces == face)
					{
						last_range.m_num_faces++;
						start_new_range = false;
					}
				}
			}

			if (start_new_range)
			{
				m_ranges.push_back(NGL_texture_subresource_transition_range(transition, transition.m_texture.m_level, face));
			}
		}
	}
};


inline uint8_t* RGB888toRGBA8888(uint32_t width, uint32_t height, uint8_t *data)
{
	uint8_t *tmp = new uint8_t[width*height * 4];

	for (uint32_t i = 0; i < width*height; i++)
	{
		tmp[i * 4 + 0] = data[i * 3 + 0];
		tmp[i * 4 + 1] = data[i * 3 + 1];
		tmp[i * 4 + 2] = data[i * 3 + 2];
		tmp[i * 4 + 3] = 255;
	}

	return tmp;
}


inline void RGB888toRGBA8888(uint32_t width, uint32_t height, const uint8_t *data, uint8_t *data_out)
{
	for (uint32_t i = 0; i < width * height; i++)
	{
		data_out[i * 4 + 0] = data[i * 3 + 0];
		data_out[i * 4 + 1] = data[i * 3 + 1];
		data_out[i * 4 + 2] = data[i * 3 + 2];
		data_out[i * 4 + 3] = 255;
	}
}


inline bool FindUniform(uint32_t &group, int32_t &application_location, const std::vector<NGL_shader_uniform> &application_uniforms, const std::string &name)
{
	group = 0;
	application_location = -1;

	for (size_t k = 0; k < application_uniforms.size(); k++)
	{
		if (application_uniforms[k].m_name == name)
		{
			application_location = (int32_t)k;

			if (application_uniforms[k].m_group == NGL_GROUP_PER_DRAW)
			{
				group = 0;
			}
			if (application_uniforms[k].m_group == NGL_GROUP_PER_RENDERER_CHANGE)
			{
				group = 1;
			}
			return true;
		}
	}

	return false;
}


inline void StoreMemoryStatistics(NGLStatistic *stats, NGL_vertex_descriptor &vertex_layout, uint32_t num)
{
	if (!stats)
	{
		return;
	}

	NGLMemoryItem item;
	item.m_size = vertex_layout.m_stride * num;

	if (vertex_layout.m_unordered_access)
	{
		stats->m_memory_statistics.m_storage_buffers.push_back(item);
	}
	else
	{
		stats->m_memory_statistics.m_vertex_buffers.push_back(item);
	}
}


inline void StoreMemoryStatistics(NGLStatistic *stats, NGL_format format, uint32_t num)
{
	if (!stats)
	{
		return;
	}

	NGLMemoryItem item;
	item.m_size = format == NGL_R16_UINT ? 2 : 4;
	item.m_size *= num;

	stats->m_memory_statistics.m_index_buffers.push_back(item);
}


inline void StoreMemoryStatistics(NGLStatistic *stats, NGL_texture_descriptor &texture_layout, std::vector<std::vector<uint8_t> > *datas)
{
	if (!stats)
	{
		return;
	}

	NGLMemoryItem item;
	item.m_name = texture_layout.m_name;
	if (datas)
	{
		for (size_t i = 0; i < datas->size(); i++)
		{
			item.m_size += (*datas)[i].size();
		}
	}
	else if (texture_layout.m_format == NGL_R8_UNORM)
	{
		item.m_size = texture_layout.m_size[0] * texture_layout.m_size[1] * 1;

	}
	else if (texture_layout.m_format == NGL_R32_FLOAT)
	{
		item.m_size = texture_layout.m_size[0] * texture_layout.m_size[1] * 4;
	}
	else if (texture_layout.m_format == NGL_R16_G16_B16_FLOAT)
	{
		item.m_size = texture_layout.m_size[0] * texture_layout.m_size[1] * 6;
	}
	else if (texture_layout.m_format == NGL_R16_G16_B16_A16_FLOAT)
	{
		item.m_size = texture_layout.m_size[0] * texture_layout.m_size[1] * 8;
	}
	else if (texture_layout.m_format == NGL_R32_G32_B32_FLOAT)
	{
		item.m_size = texture_layout.m_size[0] * texture_layout.m_size[1] * 12;
	}
	else if (texture_layout.m_format == NGL_R32_G32_B32_A32_FLOAT)
	{
		item.m_size = texture_layout.m_size[0] * texture_layout.m_size[1] * 16;
	}
	else
	{
		item.m_size = texture_layout.m_size[0] * texture_layout.m_size[1] * 4;
	}

	if (!datas)
	{
		if (texture_layout.m_type == NGL_TEXTURE_CUBE)
		{
			item.m_size *= 6;
		}
		if (texture_layout.m_type == NGL_TEXTURE_2D_ARRAY)
		{
			item.m_size *= texture_layout.m_num_array;
		}
	}

	if (texture_layout.m_is_renderable)
	{
		stats->m_memory_statistics.m_render_targets.push_back(item);
	}
	else if (texture_layout.m_unordered_access)
	{
		stats->m_memory_statistics.m_images.push_back(item);
	}
	else
	{
		stats->m_memory_statistics.m_textures.push_back(item);
	}
}