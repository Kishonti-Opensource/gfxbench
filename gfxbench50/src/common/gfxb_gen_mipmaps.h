/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef GFXB_GEN_MIPMAPS_H
#define GFXB_GEN_MIPMAPS_H

#include <kcl_base.h>
#include <vector>
#include <string>

namespace GFXB
{
	class Shapes;

	class GenMipmaps
	{
	public:
		GenMipmaps();
		~GenMipmaps();

		void Init(const char *name, Shapes *shapes, KCL::uint32 input_texture, KCL::uint32 width, KCL::uint32 height, KCL::uint32 levels);
		void Init(const char *name, Shapes *shapes, std::vector<KCL::uint32> textures, KCL::uint32 width, KCL::uint32 height);
		void Resize(KCL::uint32 width, KCL::uint32 height);
		void DeletePipelines();

		KCL::uint32 GenerateMipmaps(KCL::uint32 command_buffer, KCL::uint32 input_level);

		KCL::uint32 GetLevels() const;
		KCL::uint32 GetTexture(KCL::uint32 level = 0) const;

	private:
		std::string m_name;
		Shapes *m_shapes;
		KCL::uint32 m_shader;
		KCL::uint32 m_width;
		KCL::uint32 m_height;
		KCL::uint32 m_levels;
		std::vector<KCL::uint32> m_textures;
		std::vector<KCL::uint32> m_jobs;
		bool m_lod_mode;

		void InitRenderers();
	};
}

#endif