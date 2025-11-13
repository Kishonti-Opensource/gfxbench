/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef GLB_TEXTUREFLIPBOOK_H
#define GLB_TEXTUREFLIPBOOK_H

#include <kcl_base.h>
#include <kcl_math3d.h>
#include <kcl_node.h>
#include <vector>

// cell

// TODO: what should signal that cellID is OOB?

namespace GFXB
{
	class TextureFlipbook
	{
	private:
		KCL::uint32 m_rows;
		KCL::uint32 m_cols;
		KCL::uint32 m_start_cell;
		KCL::uint32 m_end_cell;

		KCL::uint32 m_frameCount;
		float m_framerate;
		std::vector<KCL::Vector2D> m_cells;

	public:
		TextureFlipbook();
		TextureFlipbook(KCL::uint32 rows, KCL::uint32 cols, float framerate = 1.0f);
		TextureFlipbook(KCL::uint32 rows, KCL::uint32 cols, KCL::uint32 start_cell, KCL::uint32 end_cell, float framerate = 1.0f);
		~TextureFlipbook();

		void Init(KCL::uint32 rows, KCL::uint32 cols, float framerate = 1.0f);
		void Init(KCL::uint32 rows, KCL::uint32 cols, KCL::uint32 start_cell, KCL::uint32 end_cell, float framerate = 1.0f);
		KCL::Vector2D *GetFrame(KCL::uint32 frame_index);
		void GetAnimation(KCL::uint32 start_frame, float delta_time, KCL::uint32 &begin_frame, KCL::uint32 &end_frame, float &blend_factor);

		inline KCL::uint32 GetRows() { return m_rows; }
		inline KCL::uint32 GetCols() { return m_cols; }
		inline KCL::uint32 GetStartCell() { return m_start_cell; }
		inline KCL::uint32 GetEndCell() { return m_end_cell; }
		inline KCL::uint32 GetFrameCount() { return m_frameCount; }
		inline float GetFramerate() { return m_framerate; }
	};
}

#endif