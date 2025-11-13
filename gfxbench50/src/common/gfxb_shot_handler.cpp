/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "gfxb_shot_handler.h"

#include <kcl_io.h>
#include <kcl_animation4.h>
#include <kcl_scene_handler.h>

using namespace GFXB;

ShotHandler::ShotHandler()
{
	m_camera_shot_track = nullptr;

	m_current_id = 0;
	m_camera_shot_changed = false;

	m_first_shot = true;
}


ShotHandler::~ShotHandler()
{
	delete m_camera_shot_track;
}


KCL::KCL_Status ShotHandler::Init(KCL::SceneHandler *scene)
{
	KCL::AssetFile track_file("animations/cc_track");
	if (!track_file.GetLastError())
	{
		KCL::_key_node::Read(m_camera_shot_track, track_file);
	}
	else
	{
		// Create a single shot for the whole scene
		CameraShot full_shot;
		full_shot.m_id = 0;
		full_shot.m_start_time = 0;
		full_shot.m_length = scene->m_play_time;
		m_shots.push_back(full_shot);

		INFO("ERROR: Can not load cc_track!");
		return KCL::KCL_TESTERROR_FILE_NOT_FOUND;
	}

	// Read the animation track
	CameraShot shot;
	shot.m_id = 0;
	shot.m_start_time = 0;
	for (KCL::uint32 anim_time = 0; anim_time < scene->m_play_time; anim_time++)
	{
		// Read the shot according to the animation time
		float time = float(anim_time) * 0.001f;
		float time_base = 0.0f;

		KCL::Vector4D r;
		KCL::_key_node::Get(r, m_camera_shot_track, time, time_base);
		KCL::uint32 id = KCL::uint32(r.x);

		if (id != shot.m_id)
		{
			// Finish the current shot
			shot.m_length = anim_time - shot.m_start_time;
			m_shots.push_back(shot);

			// Start a new one
			shot.m_id = id;
			shot.m_start_time = anim_time;
		}
	}

	// Finish the last shot
	shot.m_length = scene->m_play_time - shot.m_start_time;
	m_shots.push_back(shot);

	return KCL::KCL_TESTERROR_NOERROR;
}


void ShotHandler::Animate(KCL::uint32 animation_time)
{
	// Detect camera clip changes
	KCL::uint32 current_id = 0;
	if (m_camera_shot_track)
	{
		KCL::Vector4D r;
		float time = animation_time * 0.001f;
		float time_base = 0.0f;

		KCL::_key_node::Get(r, m_camera_shot_track, time, time_base);
		current_id = KCL::uint32(r.x + 0.5f);
	}

	if (m_first_shot || current_id != m_current_id)
	{
		m_camera_shot_changed = true;
	}
	else
	{
		m_camera_shot_changed = false;
	}

	m_first_shot = false;
	m_current_id = current_id;
}


void ShotHandler::Reset()
{
	m_first_shot = true;
}


bool ShotHandler::IsCameraShotChanged() const
{
	return m_camera_shot_changed;
}


const ShotHandler::CameraShot &ShotHandler::GetCurrentCameraShot() const
{
	assert( !(m_shots.size() <= m_current_id));
	return m_shots[m_current_id];
}


const ShotHandler::CameraShot &ShotHandler::GetCameraShot(KCL::uint32 index) const
{
	if (size_t(index) >= m_shots.size())
	{
		INFO("ShotHandler - Invalid camera shot index: %d !!!", index);
		assert(0);
	}

	return m_shots[index];
}


void ShotHandler::GetCameraShots(std::vector<ShotHandler::CameraShot> &shots) const
{
	shots = m_shots;
}


KCL::uint32 ShotHandler::GetCameraShotCount() const
{
	return KCL::uint32(m_shots.size());
}


KCL::uint32 ShotHandler::GetCurrentCameraShotIndex() const
{
	return m_current_id;
}


KCL::uint32 ShotHandler::GetShotId(KCL::uint32 anim_time) const
{
	for (size_t i = 0; i < m_shots.size(); i++)
	{
		const CameraShot &shot = m_shots[i];

		if (shot.m_start_time <= anim_time && (i == m_shots.size() - 1 || anim_time < shot.m_start_time + shot.m_length))
		{
			return shot.m_id;
		}
	}

	// Shot id not found!!
	assert(0);
	return (KCL::uint32)-1;
}