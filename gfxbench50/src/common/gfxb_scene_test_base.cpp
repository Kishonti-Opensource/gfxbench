/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "gfxb_scene_test_base.h"

#include <ngl.h>
#include <components/time_component.h>
#include <components/input_component.h>

#include "common/gfxb_scene_base.h"

using namespace GFXB;


SceneTestBase::SceneTestBase()
{
	m_scene = nullptr;

	m_time_component = nullptr;
	m_input_component = nullptr;

	m_mouse_sensitivity = 3.0f;
	m_mouse_fast_multiplier = 1.0f;
	m_mouse_slow_multiplier = 0.1f;

	m_movement_velocity = 20.0f;
	m_movement_fast_multiplier = 1.0f;
	m_movement_slow_multiplier = 0.05f;

	m_last_animation_time = 0;
	m_animation_multiplier = 1.0f;
	m_animation_paused = false;

	m_last_shader_reload_time = 0.0;
	m_last_screenshot_time = 0.0;
}


SceneTestBase::~SceneTestBase()
{
	terminate();
}


bool SceneTestBase::terminate()
{
	delete m_scene;
	m_scene = NULL;
	return TestBaseGFX::terminate();
}


KCL::KCL_Status SceneTestBase::Init()
{
	m_time_component = dynamic_cast<GLB::TimeComponent*>(GetTestComponent(GLB::TimeComponent::NAME));
	m_input_component = dynamic_cast<GLB::InputComponent*>(GetTestComponent(GLB::InputComponent::NAME));

	const bool is_portrait = m_test_height > m_test_width;
	m_scene->SetSize(0, 0, !is_portrait ? m_test_width : m_test_height, !is_portrait ? m_test_height : m_test_width);
	m_scene->SetIsPortrait(is_portrait);
	m_scene->SetForceHighp(GetTestDescriptor().m_force_highp);
	m_scene->SetSingleFrame(GetTestDescriptor().m_single_frame);
	m_scene->m_workgroup_sizes = GetTestDescriptor().m_workgroup_sizes;
	m_scene->setTextureCompressionType(ChooseTextureType());
	m_scene->m_backbuffers = m_screen_manager_component->GetBackbuffers();

	m_scene->m_particle_save_frames.insert(
		m_scene->m_particle_save_frames.end(),
		GetTestDescriptor().m_particle_save_frames.begin(),
		GetTestDescriptor().m_particle_save_frames.end());

	m_scene->m_gi_use_texture_sh_atlas = GetTestDescriptor().m_gi_use_texture_sh_atlas;

	m_scene->m_hdr_adaptation_mode = GetTestDescriptor().m_adaptation_mode;
	m_scene->m_hdr_save_luminance_values = GetTestDescriptor().m_hdr_save_luminance_values;

	const char *scene_file = GetTestDescriptor().m_scenefile.c_str();

	INFO("Loading scene: %s", scene_file);
	KCL::KCL_Status status = m_scene->Process_Load(~0, scene_file);
	if (status != KCL::KCL_TESTERROR_NOERROR)
	{
		return status;
	}

	INFO("Init scene...");
	status = m_scene->Init();
	if (status != KCL::KCL_TESTERROR_NOERROR)
	{
		return status;
	}

	return KCL::KCL_TESTERROR_NOERROR;
}


KCL::KCL_Status SceneTestBase::Warmup()
{
	m_scene->SetWarmup(true);

	KCL::KCL_Status status = m_scene->Warmup();

	m_scene->SetWarmup(false);

	m_scene->m_animation_time = 0;

	return status;
}


void SceneTestBase::Animate()
{
	// Auto shader reload for debugging
	if (AUTO_SHADER_RELOAD_TIME > 0)
	{
		if (m_last_shader_reload_time == 0.0)
		{
			m_last_shader_reload_time = KCL::g_os->GetTimeMilliSec();
		}
		else if (KCL::g_os->GetTimeMilliSec() - m_last_shader_reload_time > (double)AUTO_SHADER_RELOAD_TIME)
		{
			ReloadShaders();
		}
	}
	if (AUTO_SCREENSHOT_TIME > 0)
	{
		if (m_last_screenshot_time == 0.0)
		{
			m_last_screenshot_time = KCL::g_os->GetTimeMilliSec();
		}
		else if (KCL::g_os->GetTimeMilliSec() - m_last_screenshot_time > (double)AUTO_SCREENSHOT_TIME)
		{
			CreateScreenshot("auto_screenshot");
			m_last_screenshot_time = KCL::g_os->GetTimeMilliSec();
		}
	}

	// Handle the user input
	if (m_input_component && m_input_component->IsUserInputEnabled())
	{
		HandleUserInput(m_input_component, (float)m_time_component->GetFrameRenderTime() / 1000.0f);
	}

	if (m_animation_paused == false)
	{
		if (m_last_animation_time > GetAnimationTime())
		{
			// Handle the case when the animation is resumed after the real animation already time restarted
			m_scene->m_animation_time = GetAnimationTime();
		}
		else
		{
			// Calculate the animation time
			float dt = float(GetAnimationTime() - m_last_animation_time);

			float animation_time = KCL::Max(float(m_scene->m_animation_time) + dt * m_animation_multiplier, 0.0f);

			m_scene->m_animation_time = KCL::uint32(animation_time);
		}
	}

	m_last_animation_time = GetAnimationTime();

	m_scene->Animate();
}


void SceneTestBase::Render()
{
	GetScene()->m_active_backbuffer_id = m_screen_manager_component->GetActiveBackbufferId();
	GetScene()->Render();
}


void SceneTestBase::HandleUserInput(GLB::InputComponent *input, float frame_time)
{
	SceneBase *scene = GetScene();

	const bool fast_mode = input->IsKeyDown(340); //lshift
	const bool slow_mode = input->IsKeyDown(341); //lctrl

	bool moved = false;
	float velocity = m_movement_velocity * frame_time;
	if (fast_mode)
	{
		velocity *= m_movement_fast_multiplier;
	}
	if (slow_mode)
	{
		velocity *= m_movement_slow_multiplier;
	}

	//w
	if (input->IsKeyDown(87))
	{
		scene->Move(velocity);
		moved = true;
	}
	//s
	if (input->IsKeyDown(83))
	{
		scene->Move(-velocity);
		moved = true;
	}
	//a
	if (input->IsKeyDown(65))
	{
		scene->Strafe(-velocity);
		moved = true;
	}
	//d
	if (input->IsKeyDown(68))
	{
		scene->Strafe(velocity);
		moved = true;
	}
	//q
	if (input->IsKeyDown(81))
	{
		scene->Elevate(velocity);
		moved = true;
	}
	//e
	if (input->IsKeyDown(69))
	{
		scene->Elevate(-velocity);
		moved = true;
	}


	if (input->IsMouseButtonDown(1))
	{
		int dx = input->GetMouseX() - input->GetMouseOldX();
		int dy = input->GetMouseY() - input->GetMouseOldY();

		static double last_elapsed = KCL::g_os->GetTimeMilliSec();

		double elapsed = KCL::g_os->GetTimeMilliSec();

		double diff = 0;

		if( last_elapsed > 0 )
			diff = elapsed - last_elapsed;

		//diff in sec
		diff *= 0.001;

		last_elapsed = elapsed;

		float rotatex = (float)dx * (float)diff * m_mouse_sensitivity;
		float rotatey = (float)dy * (float)diff * m_mouse_sensitivity;

		if (moved == false)
		{
			if (fast_mode)
			{
				rotatex *= m_mouse_fast_multiplier;
				rotatey *= m_mouse_fast_multiplier;
			}
			if (slow_mode)
			{
				rotatex *= m_mouse_slow_multiplier;
				rotatey *= m_mouse_slow_multiplier;
			}
		}

		if (fabs(rotatex) > 0.1 && dx != 0)
		{
			scene->Rotate(rotatex*360/160.f);
		}
		if (fabs(rotatey) > 0.1 && dy != 0)
		{
			scene->Tilt(rotatey*360/160.f);
		}
	}


	//r
	if (input->IsKeyPressed(82))
	{
		ReloadShaders();
	}
	//tab
	if (input->IsKeyPressed(258))
	{
		scene->SelectNextCamera();
	}
	if (input->IsKeyPressed(293)) // F4
	{
		m_animation_multiplier = -2.0f;
	}
	if (input->IsKeyPressed(294))  // F5
	{
		m_animation_multiplier -= 0.25f;
	}
	if (input->IsKeyPressed(295))  // F6
	{
		m_animation_paused = !m_animation_paused;
	}
	if (input->IsKeyPressed(296))  // F7
	{
		m_animation_multiplier = 1.0f;
	}
	if (input->IsKeyPressed(297))  // F8
	{
		m_animation_multiplier += 0.25f;
	}
	if (input->IsKeyPressed(298))  // F9
	{
		m_animation_multiplier = 2.0f;
	}
}


std::string SceneTestBase::ChooseTextureType()
{
	const std::string &texture_type = GetTestDescriptor().GetTextureType();

	if (texture_type != "Auto")
	{
		return texture_type;
	}

	bool astc_supported = nglGetInteger(NGL_TEXTURE_COMPRESSION_ASTC) > 0;
	bool etc2_supported = nglGetInteger(NGL_TEXTURE_COMPRESSION_ETC2) > 0;
	bool dxt5_supported = nglGetInteger(NGL_TEXTURE_COMPRESSION_DXT5) > 0;

	switch (nglGetApi())
	{
	case NGL_OPENGL:
	case NGL_DIRECT3D_11:
	case NGL_DIRECT3D_12:
	case NGL_METAL_MACOS:
		if (dxt5_supported)
		{
			return "DXT5";
		}
		return "888";

	case NGL_OPENGL_ES:
	case NGL_METAL_IOS:
	{
		if (astc_supported)
		{
			return "ASTC";
		}
		if (etc2_supported)
		{
			return "ETC2";
		}
		return "888";
	}

	default:
		return "888";
	}
}


void SceneTestBase::ReloadShaders()
{
	m_scene->ReloadShaders();

	m_last_shader_reload_time = KCL::g_os->GetTimeMilliSec();
}


SceneBase *SceneTestBase::GetScene()
{
	return m_scene;
}


NGLStatistic& SceneTestBase::GetFrameStatistics()
{
	return m_scene->GetNGLStatistics();
}


ShotHandler* SceneTestBase::GetCurrentShotIndex()
{
	return m_scene->GetCameraShotHandler();
}


void SceneTestBase::GetCommandBufferConfiguration(KCL::uint32 &buffers_in_frame, KCL::uint32 &prerendered_frame_count)
{
	m_scene = CreateScene();
	m_scene->GetCommandBufferConfiguration(buffers_in_frame, prerendered_frame_count);
}


void SceneTestBase::SetCommandBuffers(const std::vector<KCL::uint32> &buffers)
{
	m_scene->SetCommandBuffers(buffers);
}


KCL::uint32 SceneTestBase::GetLastCommandBuffer()
{
	return m_scene->GetLastCommandBuffer();
}
