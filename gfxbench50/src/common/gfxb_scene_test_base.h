/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef GFXB_SCENE_TEST_BASE_H
#define GFXB_SCENE_TEST_BASE_H

#include "test_base_gfx.h"

/*
Base class of the tests based on GFXB::SceneBase (aka high level tests)
Handles common tasks like initialization and user input handling.

TODO: screen shot and statistics support
*/
struct NGLStatistic;
namespace GLB
{
	class TimeComponent;
	class InputComponent;
}

namespace GFXB
{
	class ShotHandler;
	class SceneBase;

	class SceneTestBase : public GLB::TestBaseGFX
	{
	private:
		static const int AUTO_SHADER_RELOAD_TIME = 0;
		static const int AUTO_SCREENSHOT_TIME = 0;

	public:
		SceneTestBase();
		virtual ~SceneTestBase();

		virtual bool terminate() override;

		NGLStatistic& GetFrameStatistics() override;
		GFXB::ShotHandler* GetCurrentShotIndex() override;

	protected:
		virtual KCL::KCL_Status Init() override;
		virtual KCL::KCL_Status Warmup() override;
		virtual void Animate() override;
		virtual void Render() override;
		virtual void HandleUserInput(GLB::InputComponent *input_component, float frame_time_secs) override;

		virtual void GetCommandBufferConfiguration(KCL::uint32 &buffers_in_frame, KCL::uint32 &prerendered_frame_count) override;
		virtual void SetCommandBuffers(const std::vector<KCL::uint32> &buffers) override;
		virtual KCL::uint32 GetLastCommandBuffer() override;

		virtual SceneBase *CreateScene() = 0;
		virtual SceneBase *GetScene();

		virtual std::string ChooseTextureType();

		float m_mouse_sensitivity;
		float m_mouse_fast_multiplier;
		float m_mouse_slow_multiplier;

		float m_movement_velocity;
		float m_movement_fast_multiplier;
		float m_movement_slow_multiplier;

	private:
		SceneBase *m_scene;

		GLB::TimeComponent *m_time_component;
		GLB::InputComponent *m_input_component;

		bool m_animation_paused;
		float m_animation_multiplier;
		KCL::uint32 m_last_animation_time;
		double m_last_shader_reload_time;
		double m_last_screenshot_time;

		void ReloadShaders();
	};
}

#endif
