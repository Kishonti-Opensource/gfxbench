/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef TIME_COMPONENT_H
#define TIME_COMPONENT_H

#include "test_component.h"

#include <vector>

#include <ng/timer.h>
#include <kcl_base.h>
#include <inttypes.h>

namespace GLB
{

class TimeComponent : public TestComponent
{
public:
	static const char *NAME;

	TimeComponent(TestBaseGFX *test);
	virtual ~TimeComponent();

	virtual bool Init() override;

	virtual void BeginFrame() override;
	virtual void EndFrame() override;

	virtual void BeginTest() override;

	void AddTimeEvent(int64_t time);
	void StopTimer();

	int64_t GetFrameRenderTime() const;
	int64_t GetElapsedTime() const;
	float GetFPS() const;

	virtual void CreateResult(tfw::ResultGroup &result_group) override;

private:
	ng::cpu_timer m_timer;

	int64_t m_frame_start_time;
	int64_t m_frame_time;

	int64_t m_animation_time;

	int64_t m_start_animation_time;

	int64_t m_min_frame_time;
	bool m_frame_time_limit_enabled;

	float m_fps;
	int64_t m_fps_frames;
	int64_t m_fps_time;

	std::vector<int64_t> m_time_events;
};

}

#endif
