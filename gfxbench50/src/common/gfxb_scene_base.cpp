/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "gfxb_scene_base.h"
#include "gfxb_scheduler.h"
#include "gfxb_shot_handler.h"
#include "gfxb_environment.h"
#include "gfxb_light.h"
#include "gfxb_lightshaft.h"
#include "gfxb_frustum_cull.h"
#include "gfxb_warmup_helper.h"
#include "kcl_camera2.h"

using namespace GFXB;

SceneBase::SceneBase(KCL::SceneVersion scene_version)
{
	m_scene_version = scene_version;

	FrustumCull::ResetInstanceCounter();

	m_ap = new KCL::AnimateParameter(KCL::Matrix4x4(), 0);
	m_prev_ap = new KCL::AnimateParameter(KCL::Matrix4x4(), 0);

	m_animate_actors = true;
	m_animate_camera = true;

	m_is_inited = false;
	m_is_portrait = false;
	m_is_warmup = false;
	m_editor_mode = false;
	m_scheduler = nullptr;
	m_shot_handler = nullptr;
	m_environment = nullptr;
	m_animated_camera = new KCL::Camera2();
	m_warmup_helper = nullptr;

	m_force_highp = false;

	m_active_backbuffer_id = 0;
	m_backbuffers.push_back(0);

	m_single_frame = -1;
}


SceneBase::~SceneBase()
{
	delete m_ap;
	delete m_prev_ap;

	delete m_scheduler;
	delete m_shot_handler;
	delete m_environment;
	delete m_warmup_helper;
}


KCL::KCL_Status SceneBase::Init()
{
	INFO("Set viewport: %dx%d,%dx%d", m_viewport_x, m_viewport_y, m_viewport_width, m_viewport_height);
	ResizeCameras(m_viewport_width, m_viewport_height);

	m_scheduler = CreateScheduler();

	// Load the camera shots
	m_shot_handler = new ShotHandler();
	m_shot_handler->Init(this);

	m_environment = new Environment(m_shot_handler->GetCameraShotCount());
	m_environment->LoadParameters();

	m_warmup_helper = new WarmupHelper;
	m_warmup_helper->Init(m_workgroup_sizes);

	return KCL::KCL_TESTERROR_NOERROR;
}


void SceneBase::FinalizeTasks()
{
	m_scheduler->FinalizeTasks();
}


void SceneBase::Animate()
{
	//SceneHandler::Animate();

	m_frame++;

	// Detect camera clip changes
	m_shot_handler->Animate(m_animation_time);

	m_ap->parent.identity();
	m_ap->time = int( float(m_animation_time) * m_animation_multiplier);

	m_prev_ap->parent.identity();
	m_prev_ap->time = int ( float(m_animation_time) * m_animation_multiplier - float(PREV_MVP_INTERVAL));

	if (m_animate_camera)
	{
		AnimateCamera();
	}

	if (m_animate_actors)
	{
		AnimateActors();
	}

	{
		KCL::Vector2D nearfar;

		nearfar.x = 0.1f;
		nearfar.y = 2048.0f;

		m_active_camera->SetNearFar(nearfar.x, nearfar.y);

		m_active_camera->Update();
	}

	m_environment->Update(m_shot_handler->GetCurrentCameraShot().m_id);
}


void SceneBase::AnimateCamera()
{
	KCL::Vector3D eye, ref, up;

	if (m_free_camera)
	{
		m_fps_camera->LookAt(m_camera_position, m_camera_ref, KCL::Vector3D(0, 1, 0));
		m_active_camera = m_fps_camera;

		m_camera_focus_distance = 10.0f;
		m_prev_vp = m_fps_camera->GetViewProjection();
	}
	else
	{
		{
			float fov;
			KCL::Matrix4x4 m;
			KCL::Vector3D fp;

			KCL::SceneHandler::AnimateCamera(m_ap->time, m, fov, fp);
			eye.set(m.v[12], m.v[13], m.v[14]);
			ref.set(-m.v[8], -m.v[9], -m.v[10]);
			ref += eye;
			up.set(m.v[4], m.v[5], m.v[6]);

			m_animated_camera->Perspective(fov, m_viewport_width, m_viewport_height, m_CullNear, m_CullFar);
			m_animated_camera->LookAt(eye, ref, up);
			m_active_camera = m_animated_camera;

			KCL::Vector3D r = mult4x3(m_active_camera->GetView(), fp);
			m_camera_focus_distance = -r.z;
		}

		{
			float fov;
			KCL::Matrix4x4 m;
			KCL::Vector3D fp;

			KCL::SceneHandler::AnimateCamera(m_prev_ap->time, m, fov, fp);
			eye.set(m.v[12], m.v[13], m.v[14]);
			ref.set(-m.v[8], -m.v[9], -m.v[10]);
			ref += eye;
			up.set(m.v[4], m.v[5], m.v[6]);

			KCL::Camera2 c;
			c.Perspective(fov, m_viewport_width, m_viewport_height, m_CullNear, m_CullFar);
			c.LookAt(eye, ref, up);
			c.Update();
			m_prev_vp = c.GetViewProjection();
		}
	}

	m_active_camera->SetNearFar(m_CullNear, m_CullFar);
	m_active_camera->Update();
}


void SceneBase::AnimateActors()
{
	for (size_t i = 0; i < m_actors.size(); i++)
	{
		KCL::Actor *actor = m_actors[i];

		if (!actor->m_root)
		{
			continue;
		}

		if (actor->m_shader_track)
		{
			KCL::Vector4D v;
			float t = m_ap->time / 1000.0f;
			float tb = 0.0f;

			KCL::_key_node::Get(v, actor->m_shader_track, t, tb);

			actor->material_idx = 1 - int(v.x);

			for (size_t j = 0; j < actor->m_meshes.size(); j++)
			{
				KCL::Mesh* m = actor->m_meshes[j];
				m->SetActiveMaterial(actor->material_idx);
			}
		}

		actor->m_root->animate(false, *m_ap);
		actor->m_root->animate(true, *m_prev_ap);
		actor->CalculateAABB();

		for (size_t j = 0; j < actor->m_meshes.size(); j++)
		{
			KCL::Mesh* m = actor->m_meshes[j];
			if (m->m_mesh->m_vertex_matrix_indices.size())
			{
				m->m_mesh->UpdateNodeMatrices();
				m->m_world_pom = actor->m_root->m_world_pom;
			}
		}

		for (size_t j = 0; j < actor->m_lights.size(); j++)
		{
			Light *light = (Light*)actor->m_lights[j];
			light->Animate(m_animation_time);
			if (light->m_has_lightshaft)
			{
				light->GetLightshaft()->Animate(m_shot_handler->GetCurrentCameraShotIndex());
			}
		}
	}
}


void SceneBase::Render()
{
	m_scheduler->Execute();
}


void SceneBase::RenderAndClose()
{
	Render();

	KCL::uint32 last_command_buffer = GetLastCommandBuffer();
	nglEndCommandBuffer(last_command_buffer);
	nglSubmitCommandBuffer(last_command_buffer);
}


KCL::KCL_Status SceneBase::Warmup()
{
	return KCL::KCL_TESTERROR_NOERROR;
}


void  SceneBase::SetWarmup(bool value)
{
	m_is_warmup = value;
}


bool SceneBase::IsWarmup() const
{
	return m_is_warmup;
}


void SceneBase::SetIsPortrait(bool x)
{
	m_is_portrait = x;
}


void SceneBase::SetSize(KCL::uint32 x, KCL::uint32 y, KCL::uint32 width, KCL::uint32 height)
{
	INFO("Set viewport size: %dx%d-%dx%d", x, y, x + width, y + height);
	INFO("Portrait mode: %d", m_is_portrait);

	m_viewport_x = x;
	m_viewport_y = y;
	m_viewport_width = width;
	m_viewport_height = height;
}


void SceneBase::Resize(KCL::uint32 x, KCL::uint32 y, KCL::uint32 width, KCL::uint32 height)
{
	SetSize(x, y, width, height);
	ResizeCameras(width, height);
}


void SceneBase::ResizeCameras(KCL::uint32 width, KCL::uint32 height)
{
	ResizeCamera(m_fps_camera, width, height);
	ResizeCamera(m_animated_camera, width, height);
}


void SceneBase::ResizeCamera(KCL::Camera2 *camera, KCL::uint32 width, KCL::uint32 height)
{
	if (camera != nullptr)
	{
		camera->Perspective(camera->GetFov(), width, height, camera->GetNear(), camera->GetFar());
		camera->Update();
	}
}


ShotHandler *SceneBase::GetCameraShotHandler() const
{
	return m_shot_handler;
}


Environment *SceneBase::GetEnvironment() const
{
	return m_environment;
}


void SceneBase::SetAnimateActors(bool value)
{
	m_animate_actors = value;
}


void SceneBase::SetAnimateCamera(bool value)
{
	m_animate_camera = value;
}


void SceneBase::SetEditorMode(bool editor_mode)
{
	m_editor_mode = editor_mode;

	if (m_editor_mode)
	{
		INFO("Editor mode enabled");
	}
}


bool SceneBase::IsEditorMode() const
{
	return m_editor_mode;
}


void SceneBase::SetInitialized(bool value)
{
	m_is_inited = value;
}


bool SceneBase::IsInitialized() const
{
	return m_is_inited;
}


bool SceneBase::ForceHighp() const
{
	return m_force_highp;
}


void SceneBase::SetForceHighp(bool value)
{
	m_force_highp = value;

	if (IsInitialized())
	{
		ReloadShaders();
	}
}


NGLStatistic &SceneBase::GetNGLStatistics()
{
	return m_ngl_statistics;
}