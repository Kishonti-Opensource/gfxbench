/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef GFXB_BARRIER_H
#define GFXB_BARRIER_H

#include <ngl.h>
#include <kcl_base.h>
#include <map>

namespace GFXB
{
	class Transitions
	{
	public:
		static Transitions &Get();
		static void Release();

		Transitions &Register(KCL::uint32 texture, const NGL_texture_descriptor &descriptor);
		Transitions &Register(KCL::uint32 buffer, const NGL_vertex_descriptor &descriptor);

		Transitions &TextureBarrier(const NGL_texture_subresource &subresource, NGL_resource_state new_state);
		Transitions &TextureBarrier(KCL::uint32 texture, NGL_resource_state new_state);
		Transitions &TextureMipLevelBarrier(KCL::uint32 texture, KCL::uint32 level, NGL_resource_state new_state);
		Transitions &TextureBarriers(const std::vector<NGL_texture_subresource_transition> &barriers);
		Transitions &TextureBarriers(const std::vector<NGL_texture_subresource> &textures, NGL_resource_state new_state);
		Transitions &BufferBarrier(KCL::uint32 buffer, NGL_resource_state new_state);

		Transitions &UpdateTextureStates(const std::vector<NGL_texture_subresource_transition> &states);

		void Execute(KCL::uint32 command_buffer);

		NGL_resource_state GetTextureState(const NGL_texture_subresource &subresource) const;
		NGL_resource_state GetTextureState(KCL::uint32 texture) const;
		NGL_resource_state GetBufferState(KCL::uint32 buffer) const;

		void DumpResourceStates();

		static const char *GetStateName(NGL_resource_state state);

		static void CreateSubpassTransitions(const NGL_job_descriptor &descriptor,
			std::vector<NGL_texture_subresource_transition> &begin_transitions,
			std::vector<NGL_texture_subresource_transition> &end_transitions);

	private:
		static Transitions *instance;

		struct SubresourceComparator
		{
			bool operator()(const NGL_texture_subresource &a, const NGL_texture_subresource &b) const
			{
				if (a.m_idx != b.m_idx)
				{
					return a.m_idx < b.m_idx;
				}

				if (a.m_level != b.m_level)
				{
					return a.m_level < b.m_level;
				}

				if (a.m_face != b.m_face)
				{
					return a.m_face < b.m_face;
				}

				return a.m_layer < b.m_layer;
			}
		};

		std::map<NGL_texture_subresource, NGL_resource_state, SubresourceComparator> m_textures;
		std::map<KCL::uint32, NGL_resource_state> m_buffers;

		std::vector<NGL_texture_subresource_transition> m_texture_barriers;
		std::vector<NGL_buffer_transition> m_buffer_barriers;

		// For debugging
		std::map<NGL_texture_subresource, std::string, SubresourceComparator> m_texture_names;

		Transitions();
		Transitions(const Transitions &other);
		Transitions &operator=(const Transitions &);
		virtual ~Transitions();


		static bool IsDepthTexture(NGL_format format);
	};
}

#endif