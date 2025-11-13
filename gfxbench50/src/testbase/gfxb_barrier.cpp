/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "gfxb_barrier.h"

#include <kcl_os.h>
#include <cassert>

using namespace GFXB;

Transitions *Transitions::instance = nullptr;


Transitions &Transitions::Get()
{
	if (instance == nullptr)
	{
		instance = new Transitions();
	}

	return *instance;
}


void Transitions::Release()
{
	delete instance;
	instance = nullptr;
}


Transitions::Transitions()
{
}


Transitions::~Transitions()
{
}



Transitions &Transitions::Register(KCL::uint32 texture, const NGL_texture_descriptor &descriptor)
{
	if (texture == 0)
	{
		return *this;
	}

	NGL_resource_state begin_state;

	if (descriptor.m_unordered_access)
	{
		begin_state = NGL_SHADER_RESOURCE_AND_UNORDERED_ACCESS_AND_NON_FRAGMENT_SHADER_RESOURCE;
	}
	else if (descriptor.m_is_renderable)
	{
		begin_state = IsDepthTexture(descriptor.m_format) ? NGL_DEPTH_ATTACHMENT : NGL_COLOR_ATTACHMENT;
	}
	else
	{
		assert(false);
		return *this;
	}

	KCL::uint32 faces = descriptor.m_type == NGL_TEXTURE_CUBE ? 6 : 1;

	for (KCL::uint32 level = 0; level < descriptor.m_num_levels; level++)
	{
		for (KCL::uint32 layer = 0; layer < descriptor.m_num_array; layer++)
		{
			for (KCL::uint32 face = 0; face < faces; face++)
			{
				NGL_texture_subresource subresource(texture, level, layer, face);

				assert(m_textures.find(subresource) == m_textures.end());

				m_textures[subresource] = begin_state;

#ifdef _DEBUG
				m_texture_names[subresource] = descriptor.m_name;
#endif
			}
		}
	}

	return *this;
}


Transitions &Transitions::Register(KCL::uint32 buffer, const NGL_vertex_descriptor &descriptor)
{
	assert(m_buffers.find(buffer) == m_buffers.end());

	if (descriptor.m_unordered_access)
	{
		m_buffers[buffer] = NGL_SHADER_RESOURCE_AND_UNORDERED_ACCESS_AND_NON_FRAGMENT_SHADER_RESOURCE;
	}
	else
	{
		assert(false);
	}

	return *this;
}


Transitions &Transitions::TextureBarrier(const NGL_texture_subresource &subresource, NGL_resource_state new_state)
{
	if (subresource.m_idx == 0)
	{
		// NGL backend handles the state of the backbuffer
		return *this;
	}

	assert(m_textures.find(subresource) != m_textures.end());

	NGL_resource_state &old_state = m_textures[subresource];
	if (old_state != new_state || new_state == NGL_SHADER_RESOURCE_AND_UNORDERED_ACCESS || new_state == NGL_SHADER_RESOURCE_AND_UNORDERED_ACCESS_AND_NON_FRAGMENT_SHADER_RESOURCE)
	{
		NGL_texture_subresource_transition barrier(subresource, old_state, new_state);
		m_texture_barriers.push_back(barrier);

		old_state = new_state;
	}

	return *this;
}


Transitions &Transitions::TextureBarrier(KCL::uint32 texture, NGL_resource_state new_state)
{
	NGL_texture_subresource subresource(texture);

	return TextureBarrier(subresource, new_state);
}


Transitions &Transitions::TextureMipLevelBarrier(KCL::uint32 texture, KCL::uint32 level, NGL_resource_state new_state)
{
	NGL_texture_subresource subresource(texture, level);

	return TextureBarrier(subresource, new_state);
}


Transitions &Transitions::TextureBarriers(const std::vector<NGL_texture_subresource_transition> &barriers)
{
	for (size_t i = 0; i < barriers.size(); i++)
	{
		const NGL_texture_subresource_transition &barrier = barriers[i];
		TextureBarrier(barrier.m_texture, barrier.m_new_state);
	}

	return *this;
}


Transitions &Transitions::TextureBarriers(const std::vector<NGL_texture_subresource> &textures, NGL_resource_state new_state)
{
	for (size_t i = 0; i < textures.size(); i++)
	{
		TextureBarrier(textures[i], new_state);
	}

	return *this;
}


Transitions &Transitions::BufferBarrier(KCL::uint32 buffer, NGL_resource_state new_state)
{
	assert(m_buffers.find(buffer) != m_buffers.end());

	NGL_resource_state &old_state = m_buffers[buffer];
	if (old_state != new_state || new_state == NGL_SHADER_RESOURCE_AND_UNORDERED_ACCESS || new_state == NGL_SHADER_RESOURCE_AND_UNORDERED_ACCESS_AND_NON_FRAGMENT_SHADER_RESOURCE)
	{
		NGL_buffer_transition barrier(buffer, old_state, new_state);
		m_buffer_barriers.push_back(barrier);

		old_state = new_state;
	}

	return *this;
}


Transitions &Transitions::UpdateTextureStates(const std::vector<NGL_texture_subresource_transition> &states)
{
	for (size_t i = 0; i < states.size(); i++)
	{
		const NGL_texture_subresource_transition &state = states[i];

		assert(m_textures.find(state.m_texture) != m_textures.end());
		assert(m_textures[state.m_texture] == state.m_old_state);

		m_textures[state.m_texture] = state.m_new_state;
	}

	return *this;
}


void Transitions::Execute(KCL::uint32 command_buffer)
{
	if (m_texture_barriers.empty() && m_buffer_barriers.empty())
	{
		return;
	}

	nglBarrier(command_buffer, m_texture_barriers, m_buffer_barriers);

	m_texture_barriers.clear();
	m_buffer_barriers.clear();
}


NGL_resource_state Transitions::GetTextureState(const NGL_texture_subresource &subresource) const
{
	if (subresource.m_idx == 0) // system attachment
	{
		return NGL_COLOR_ATTACHMENT;
	}
	else
	{
		return m_textures.at(subresource);
	}
}


NGL_resource_state Transitions::GetTextureState(KCL::uint32 texture) const
{
	NGL_texture_subresource subresource(texture);

	return GetTextureState(subresource);
}


NGL_resource_state Transitions::GetBufferState(KCL::uint32 buffer) const
{
	return m_buffers.at(buffer);
}


bool Transitions::IsDepthTexture(NGL_format format)
{
	switch (format)
	{
	case NGL_D16_UNORM:
	case NGL_D24_UNORM:
	case NGL_D32_UNORM:
		return true;

	default:
		return false;
	}
}


void Transitions::DumpResourceStates()
{
	INFO("Textures:");
	for (auto it = m_textures.begin(); it != m_textures.end(); it++)
	{
		INFO("%d (%s): %s", it->first.m_idx, m_texture_names[it->first].c_str(), GetStateName(it->second));
	}

	INFO("Buffers:");
	for (auto it = m_buffers.begin(); it != m_buffers.end(); it++)
	{
		INFO("%d: %s", it->first, GetStateName(it->second));
	}
}


const char *Transitions::GetStateName(NGL_resource_state state)
{
	switch (state)
	{
	case NGL_COLOR_ATTACHMENT:
		return "NGL_COLOR_ATTACHMENT";
	case NGL_COLOR_ATTACHMENT_AND_PRESERVED_ATTACHMENT:
		return "NGL_COLOR_ATTACHMENT_AND_PRESERVED_ATTACHMENT";
	case NGL_COLOR_ATTACHMENT_AND_INPUT_ATTACHMENT_AND_SHADER_RESOURCE:
		return "NGL_COLOR_ATTACHMENT_AND_INPUT_ATTACHMENT_AND_SHADER_RESOURCE";

	case NGL_DEPTH_ATTACHMENT:
		return "NGL_DEPTH_ATTACHMENT";
	case NGL_READ_ONLY_DEPTH_ATTACHMENT:
		return "NGL_READ_ONLY_DEPTH_ATTACHMENT";
	case NGL_READ_ONLY_DEPTH_ATTACHMENT_AND_SHADER_RESOURCE:
		return "NGL_READ_ONLY_DEPTH_ATTACHMENT_AND_SHADER_RESOURCE";

	case NGL_SHADER_RESOURCE:
		return "NGL_SHADER_RESOURCE";
	case NGL_SHADER_RESOURCE_AND_UNORDERED_ACCESS:
		return "NGL_SHADER_RESOURCE_AND_UNORDERED_ACCESS";
	case NGL_SHADER_RESOURCE_AND_NON_FRAGMENT_SHADER_RESOURCE:
		return "NGL_SHADER_RESOURCE_AND_NON_FRAGMENT_SHADER_RESOURCE";
	case NGL_SHADER_RESOURCE_AND_UNORDERED_ACCESS_AND_NON_FRAGMENT_SHADER_RESOURCE:
		return "NGL_SHADER_RESOURCE_AND_UNORDERED_ACCESS_AND_NON_FRAGMENT_SHADER_RESOURCE";

	default:
		return "UNKNOWN";
	}
}


void Transitions::CreateSubpassTransitions(const NGL_job_descriptor &descriptor,
	std::vector<NGL_texture_subresource_transition> &begin_transitions,
	std::vector<NGL_texture_subresource_transition> &end_transitions)
{
	begin_transitions.clear();
	end_transitions.clear();

	for (size_t i = 0; i < descriptor.m_attachments.size(); i++)
	{
		const NGL_attachment_descriptor &attachment = descriptor.m_attachments[i];

		const NGL_subpass &first_subpass = descriptor.m_subpasses.front();
		const NGL_subpass &last_subpass = descriptor.m_subpasses.back();

		// Transitions before the first subpass (old state is provided by Transitions class)
		NGL_texture_subresource_transition begin_transition(NGL_texture_subresource(attachment.m_attachment), NGL_resource_state(~0), first_subpass.m_usages[i]);
		begin_transitions.push_back(begin_transition);

		if (descriptor.m_subpasses.size() > 1)
		{
			NGL_texture_subresource_transition end_transition(NGL_texture_subresource(attachment.m_attachment), first_subpass.m_usages[i], last_subpass.m_usages[i]);
			end_transitions.push_back(end_transition);
		}
	}
}
