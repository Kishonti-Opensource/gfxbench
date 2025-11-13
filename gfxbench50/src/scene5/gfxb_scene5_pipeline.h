/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef GFXB_SCENE5_PIPELINE_H
#define GFXB_SCENE5_PIPELINE_H

#include "gfxb_scene5.h"

namespace GFXB
{
	class Scene5Pipeline : public Scene5
	{
	public:
		Scene5Pipeline();
		~Scene5Pipeline();

		KCL::KCL_Status Init() override;
		KCL::KCL_Status ReloadShaders() override;

		void RenderPipeline() override;
		void Resize(KCL::uint32 x, KCL::uint32 y, KCL::uint32 w, KCL::uint32 h) override;

		void GetCommandBufferConfiguration(KCL::uint32 &buffers_in_frame, KCL::uint32 &prerendered_frame_count) override;
		void SetCommandBuffers(const std::vector<KCL::uint32> &buffers) override;
		KCL::uint32 GetLastCommandBuffer() override;

	protected:
		void InitShaders() override;
		void InitShaderFactory(ShaderFactory *shader_factory) override;
		
		virtual void BindGBuffer(const void **p) override;

	private:
		// Command buffers
		KCL::uint32 m_command_buffer_shadow;
		KCL::uint32 m_command_buffer_gi;
		KCL::uint32 m_command_buffer_main;

		// Render jobs
		KCL::uint32 m_deferred_render;
		KCL::uint32 m_forward_render;

		// Render target textures
		KCL::uint32 m_gbuffer_color_depth_texture;

		KCL::uint32 m_gbuffer_color_depth_attachment_id;
		KCL::uint32 m_lighting_attachment_id;
		KCL::uint32 m_lighting_weight_attachment_id;

		std::vector<NGL_texture_subresource_transition> m_subpass_begin_transitions;
		std::vector<NGL_texture_subresource_transition> m_subpass_end_transitions;
		
		bool m_deffered_render_run;

		void InitSubpassDefines();
	};
}

#endif
