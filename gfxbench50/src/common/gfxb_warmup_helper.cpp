/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "gfxb_warmup_helper.h"

#include "ngl.h"
#include "kcl_os.h"

#include <sstream>

using namespace GFXB;


WarmupHelper::WarmupHelper()
{
	m_max_ws_size_x = nglGetInteger(NGL_MAX_COMPUTE_WORK_GROUP_SIZE_X);
	m_max_ws_size_y = nglGetInteger(NGL_MAX_COMPUTE_WORK_GROUP_SIZE_Y);
	m_max_ws_size_z = nglGetInteger(NGL_MAX_COMPUTE_WORK_GROUP_SIZE_Z);

	m_max_invocations = nglGetInteger(NGL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS);

	m_max_shared_memory = nglGetInteger(NGL_MAX_COMPUTE_SHARED_MEMORY_SIZE);
}


bool WarmupHelper::Init(const std::string &wgs_str)
{
	// Parse the workgroup size string
	std::stringstream ss(wgs_str);
	std::string values;
	while (std::getline(ss, values, ','))
	{
		std::stringstream ss2(values);
		std::string name;
		KCL::uint32 wg_size;

		if (!(ss2 >> name))
		{
			continue;
		}

		if (!(ss2 >> wg_size))
		{
			m_work_group_sizes[name].x = 0;
			continue;
		}
		m_work_group_sizes[name].x = wg_size;

		if (!(ss2 >> wg_size))
		{
			m_work_group_sizes[name].y = 0;
			continue;
		}
		m_work_group_sizes[name].y = wg_size;

		if (!(ss2 >> wg_size))
		{
			m_work_group_sizes[name].z = 0;
			continue;
		}
		m_work_group_sizes[name].z = wg_size;
	}

	return true;
}


void WarmupHelper::BeginTimer()
{
	nglFinish();
	m_start_time = KCL::uint64(KCL::g_os->GetTimeMilliSec());
}


KCL::uint64 WarmupHelper::EndTimer()
{
	nglFinish();
	KCL::uint64 now = KCL::uint64(KCL::g_os->GetTimeMilliSec());
	return now - m_start_time;
}


bool WarmupHelper::ValidateWorkGroupSize(KCL::uint32 size_x, KCL::uint32 size_y, KCL::uint32 size_z) const
{
	if (size_x > m_max_ws_size_x)
	{
		return false;
	}
	if (size_y > m_max_ws_size_y)
	{
		return false;
	}
	if (size_z > m_max_ws_size_z)
	{
		return false;
	}

	return size_x * size_y * size_z <= m_max_invocations;
}


