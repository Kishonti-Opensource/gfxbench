/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef GFXB5_SCENE_TOOLS_H
#define GFXB5_SCENE_TOOLS_H

#include "gfxb_scene5.h"

#include <kcl_base.h>
#include <string>

class JsonSerializer;

namespace GFXB
{
	class Scene5Tools
	{
	public:
		static bool LoadRenderFlags(KCL::uint32 &flags);
		static void SaveRenderFlags(KCL::uint32 flags);

		static TierLevel ResolveTierLevel(const std::string &name);

		static KCL::uint32 CreateDebugMipmapTexture();

		static KCL::uint32 CaptureEnvironmentCube(KCL::uint32 command_buffer, const char *name, Shapes *shapes, KCL::uint32 size, std::vector<KCL::Mesh*> &meshes);

		static void SaveMemoryStatistics(const NGLStatistic &stats);

		static void DumpPipelineStatistics(const NGLStatistic &stats);

	private:
		static void SerializeRenderFlags(JsonSerializer& s, KCL::uint32 flags);
	};
}

#endif