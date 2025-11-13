/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "statistics_component.h"
#include "test_base_gfx.h"
#include "common/gfxb_scene_test_base.h"
#include "render_statistics.h"
#include "kcl_io.h"

using namespace GLB;

const char* StatisticsComponent::NAME = "StatisticsComponent";

StatisticsComponent::StatisticsComponent(TestBaseGFX *test, int statistic_mode) : TestComponent(test, NAME)
{
	m_statistic_mode = statistic_mode;
	m_render_statistics = nullptr;
	m_timed_render_statistics = nullptr;
	m_per_frame_render_time_statistics = nullptr;
	m_render_job_statistics = nullptr;
}


StatisticsComponent::~StatisticsComponent()
{
	delete m_render_statistics;
	delete m_timed_render_statistics;
	delete m_per_frame_render_time_statistics;
}


bool StatisticsComponent::Init()
{
	if (m_statistic_mode & STATISTICS_MODE_RENDER_STATISTIC)
	{
		m_render_statistics = new RenderStatistics(m_test->GetTestWidth(), m_test->GetTestHeight());
		m_timed_render_statistics = new TimedRenderStatistics(m_test->GetTestWidth(), m_test->GetTestHeight());
	}
	if (m_statistic_mode & STATISTICS_MODE_PER_FRAME_RENDER_TIME)
	{
		m_per_frame_render_time_statistics = new PerFrameRenderTimeStatistics();
	}
	if (m_statistic_mode & STATISTICS_MODE_RENDER_JOB)
	{
		m_render_job_statistics = new RenderJobStatistics();
	}

	return true;
}


void StatisticsComponent::BeginFrame()
{
	if (m_statistic_mode & STATISTICS_MODE_RENDER_STATISTIC)
	{
		m_test->GetFrameStatistics().queries_enabled[NGL_STATISTIC_PRIMITIVES_SUBMITED] = true;
		m_test->GetFrameStatistics().queries_enabled[NGL_STATISTIC_VERTICES_SUBMITED] = true;
		m_test->GetFrameStatistics().queries_enabled[NGL_STATISTIC_VS_INVOCATIONS] = true;
		m_test->GetFrameStatistics().queries_enabled[NGL_STATISTIC_TCS_PATCHES] = true;
		m_test->GetFrameStatistics().queries_enabled[NGL_STATISTIC_TES_INVOCATIONS] = true;
		m_test->GetFrameStatistics().queries_enabled[NGL_STATISTIC_GS_INVOCATIONS] = true;
		m_test->GetFrameStatistics().queries_enabled[NGL_STATISTIC_GS_PRIMITIVES_EMITTED] = true;
		m_test->GetFrameStatistics().queries_enabled[NGL_STATISTIC_FS_INVOCATIONS] = true;
		m_test->GetFrameStatistics().queries_enabled[NGL_STATISTIC_CLIPPING_INPUT_PRIMITIVES] = true;
		m_test->GetFrameStatistics().queries_enabled[NGL_STATISTIC_CLIPPING_OUTPUT_PRIMITIVES] = true;
		m_test->GetFrameStatistics().queries_enabled[NGL_STATISTIC_PRIMITIVES_GENERATED] = false; // TODO
		m_test->GetFrameStatistics().queries_enabled[NGL_STATISTIC_SAMPLES_PASSED] = true;
		m_test->GetFrameStatistics().queries_enabled[NGL_STATISTIC_TIME_ELAPSED] = true;
		m_timed_render_statistics->StartGlobalTimer();
	}
	if (m_statistic_mode & STATISTICS_MODE_PER_FRAME_RENDER_TIME)
	{
		m_test->GetFrameStatistics().queries_enabled[NGL_STATISTIC_TIME_ELAPSED] = true;
	}
	if (m_statistic_mode & STATISTICS_MODE_RENDER_JOB)
	{
		m_test->GetFrameStatistics().queries_enabled[NGL_STATISTIC_TIME_ELAPSED] = true;
	}
}


void StatisticsComponent::EndFrame()
{
	if (m_statistic_mode != STATISTICS_MODE_NONE)
	{
		// Get the result
		nglFinish();
		nglGetStatistic();
	}

	if (m_render_statistics)
	{
		m_render_statistics->AddFrame(m_test->GetFrameStatistics());
	}

	if (m_timed_render_statistics)
	{
		m_timed_render_statistics->AddFrame(m_test->GetFrameStatistics(), m_test->GetAnimationTime(), m_test->GetCurrentShotIndex());
	}

	if (m_per_frame_render_time_statistics)
	{
		m_per_frame_render_time_statistics->AddFrame(m_test->GetFrameStatistics(), m_test->GetAnimationTime());
	}

	if (m_render_job_statistics)
	{
		m_render_job_statistics->AddFrame(m_test->GetFrameStatistics(), m_test->GetAnimationTime());
	}
}


void StatisticsComponent::CreateResult(tfw::ResultGroup &result_group)
{
	INFO("Statistic save path = %s", KCL::File::GetDataRWPath().c_str());
	if (m_render_statistics)
	{
		m_render_statistics->CalculateStatistics();
		m_render_statistics->ExportStatistics((KCL::File::GetDataRWPath() + "pipeline_statistics.csv").c_str() );
	}

	if (m_timed_render_statistics)
	{
		m_timed_render_statistics->CalculateStatistics();
		m_timed_render_statistics->ExportStatistics((KCL::File::GetDataRWPath() + "timed_pipeline_statistics.csv").c_str());
	}

	if (m_per_frame_render_time_statistics)
	{
		m_per_frame_render_time_statistics->CalculateStatistics();
		m_per_frame_render_time_statistics->ExportStatistics((KCL::File::GetDataRWPath() + "per_frame_statistics.csv").c_str());
	}

	if (m_render_job_statistics)
	{
		m_render_job_statistics->CalculateStatistics();
		m_render_job_statistics->ExportStatistics((KCL::File::GetDataRWPath() + "render_job_statistics.csv").c_str());
	}
}

