/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef GFXB_SHOT_HANDLER_H
#define GFXB_SHOT_HANDLER_H

#include <vector>
#include <kcl_base.h>

#include <assert.h>

namespace GFXB
{
	class ShotHandler
	{
	public:
		struct CameraShot
		{
			KCL::uint32 m_id;

			KCL::uint32 m_start_time;
			KCL::uint32 m_length;

			CameraShot()
			{
				m_id = 0;
				m_start_time = 0;
				m_length = 0;
			}
		};

		ShotHandler();
		~ShotHandler();

		KCL::KCL_Status Init(KCL::SceneHandler *scene);
		void Animate(KCL::uint32 animation_time);

		void Reset();

		bool IsCameraShotChanged() const;
		const CameraShot &GetCurrentCameraShot() const;
		const CameraShot &GetCameraShot(KCL::uint32 index) const;
		void GetCameraShots(std::vector<CameraShot> &shots) const;
		KCL::uint32 GetCameraShotCount() const;

		KCL::uint32 GetCurrentCameraShotIndex() const;

		KCL::uint32 GetShotId(KCL::uint32 anim_time) const;

	private:
		KCL::_key_node *m_camera_shot_track;

		std::vector<CameraShot> m_shots;

		KCL::uint32 m_current_id;
		bool m_camera_shot_changed;
		bool m_first_shot;
	};
}

#endif