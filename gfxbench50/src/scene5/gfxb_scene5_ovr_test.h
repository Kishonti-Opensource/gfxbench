/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef GFXB_5_OVR_TEST_H
#define GFXB_5_OVR_TEST_H

#include "common/gfxb_scene_test_base.h"

#include "gfxb_5_ovr_ngl_adapter.h"

namespace GFXB
{
	class Gfxb5OvrTest : public SceneTestBase
	{
	public:
		Gfxb5OvrTest();
		virtual ~Gfxb5OvrTest();

		virtual KCL::KCL_Status Init() override;
		virtual void Animate() override;
		virtual void Render() override;
	protected:
		virtual SceneBase *CreateScene() override;
		virtual std::string ChooseTextureType() override;
		virtual void HandleUserInput(GLB::InputComponent *input_component, float frame_time_secs) override;
		virtual double GetScore() override;
		bool InitRenderAPI() override;
	private:
		OVRGraphicsContext *m_ovr_graphics_context;

		OvrNGLAdapter *m_adapter;

		KCL::uint32 m_ovr_texture_width;
		KCL::uint32 m_ovr_texture_height;

		float m_ovr_fov_x;
		float m_ovr_fov_y;

		ovrHeadModelParms m_ovr_head_model_params;
		ovrTracking m_ovr_base_tracking;
		ovrTracking m_ovr_tracking;
		ovrFrameParms m_ovr_frame_params;

		ovrMatrix4f m_ovr_projection_matrix;
		ovrMatrix4f m_ovr_center_eye_view_matrix;

		ovrMatrix4f m_ovr_eye_view_matrix[VRAPI_FRAME_LAYER_EYE_MAX];
		ovrTextureSwapChain *m_ovr_swap_chain[VRAPI_FRAME_LAYER_EYE_MAX];
		std::vector<KCL::uint32> m_render_textures[VRAPI_FRAME_LAYER_EYE_MAX];

		void InitOvr();
	};
}

#endif
