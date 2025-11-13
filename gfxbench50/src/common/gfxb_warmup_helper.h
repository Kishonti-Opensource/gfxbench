/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef GFXB_WARMUP_HELPER_H
#define GFXB_WARMUP_HELPER_H

#include <kcl_base.h>

#include <string>
#include <map>


namespace GFXB
{
	struct WorkGroupSize
	{
		WorkGroupSize()
			: x(0), y(0), z(0)
		{
		}
		
		KCL::uint32 x, y, z;
	};

	class WarmupHelper
	{
	public:
		WarmupHelper();

		virtual ~WarmupHelper()
		{
		}

		bool Init(const std::string &wgs_str);

		WorkGroupSize &At(const std::string &type)
		{
			return m_work_group_sizes[type];
		}

		void BeginTimer();
		KCL::uint64 EndTimer();

		bool ValidateWorkGroupSize(KCL::uint32 size_x, KCL::uint32 size_y = 1, KCL::uint32 size_z = 1) const;

	private: 
		std::map<std::string, WorkGroupSize> m_work_group_sizes;

		KCL::uint64 m_start_time;

		KCL::uint32 m_max_ws_size_x;
		KCL::uint32 m_max_ws_size_y;
		KCL::uint32 m_max_ws_size_z;

		KCL::uint32 m_max_invocations;

		KCL::uint32 m_max_shared_memory;
	};
}

#endif  // GFXB_WARMUP_HELPER_H

