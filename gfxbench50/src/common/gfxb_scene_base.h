/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef GFXB_SCENE_H
#define GFXB_SCENE_H

#include <ngl.h>
#include <kcl_scene_handler.h>

#include "gfxb_scheduler.h"

namespace GFXB
{
	class Scheduler;
	class ShotHandler;
	class Environment;
	class WarmupHelper;
	class SceneBase : public KCL::SceneHandler
	{
	public:
		static const KCL::uint32 PREV_MVP_INTERVAL = 5;

		SceneBase(KCL::SceneVersion scene_version);
		virtual	~SceneBase();

		virtual KCL::KCL_Status Init();
		virtual KCL::KCL_Status ReloadShaders() = 0;

		virtual void Animate() override;
		virtual void Render();
		virtual void RenderAndClose();
		virtual void SetIsPortrait(bool x);
		virtual void SetSize(KCL::uint32 x, KCL::uint32 y, KCL::uint32 width, KCL::uint32 height);
		virtual void Resize(KCL::uint32 x, KCL::uint32 y, KCL::uint32 width, KCL::uint32 height);

		virtual void GetCommandBufferConfiguration(KCL::uint32 &buffers_in_frame, KCL::uint32 &prerendered_frame_count) = 0;
		virtual void SetCommandBuffers(const std::vector<KCL::uint32> &buffers) = 0;
		virtual KCL::uint32 GetLastCommandBuffer() = 0;

		void SetSingleFrame( int ssf )
		{
			m_single_frame = ssf;
		}

		int GetSingleFrame()
		{
			return m_single_frame;
		}

		void ScheduleCPUTask(std::function<void(void)> fn, KCL::uint32 build_order)
		{
			Task *task = new Task(fn, build_order);
			m_scheduler->ScheduleTask(task);
		}
		void ScheduleRenderTask(std::function<KCL::uint32(void)> fn, KCL::uint32 build_order, KCL::uint32 submit_order)
		{
			Task *task = new Task(fn, build_order, submit_order);
			m_scheduler->ScheduleTask(task);
		}

		void FinalizeTasks();

		ShotHandler *GetCameraShotHandler() const;
		Environment *GetEnvironment() const;

		void SetAnimateActors(bool value);
		void SetAnimateCamera(bool value);

		void SetEditorMode(bool editor_mode);
		bool IsEditorMode() const;
		bool IsInitialized() const;
		bool ForceHighp() const;
		void SetForceHighp(bool value);
		NGLStatistic &GetNGLStatistics();

		virtual KCL::KCL_Status Warmup();
		void SetWarmup(bool value);
		bool IsWarmup() const;

		KCL::uint32 m_active_backbuffer_id;
		std::vector<KCL::uint32> m_backbuffers;

		std::string m_workgroup_sizes;
		WarmupHelper* m_warmup_helper;

		std::vector<int> m_particle_save_frames;
		bool m_gi_use_texture_sh_atlas;
		int m_hdr_adaptation_mode;
		bool m_hdr_save_luminance_values;

	protected:
		void AnimateActors();
		void AnimateCamera();

		virtual void ResizeCameras(KCL::uint32 width, KCL::uint32 height);
		virtual void ResizeCamera(KCL::Camera2 *camera, KCL::uint32 width, KCL::uint32 height);

		void SetInitialized(bool value);

		virtual Scheduler *CreateScheduler() = 0;

		NGLStatistic m_ngl_statistics;

	private:
		Scheduler *m_scheduler;
		ShotHandler *m_shot_handler;
		Environment *m_environment;

		int m_single_frame;

		bool m_is_inited;
		bool m_is_warmup;
		bool m_editor_mode;

		bool m_force_highp;

		bool m_animate_camera;
		bool m_animate_actors;

		KCL::AnimateParameter *m_ap;
		KCL::AnimateParameter *m_prev_ap;
	};
};

#endif
