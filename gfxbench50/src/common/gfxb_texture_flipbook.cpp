/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "gfxb_texture_flipbook.h"

#include "gfxb_mesh_shape.h"

#include <vector>
#include <iostream>

using namespace GFXB;

GFXB::TextureFlipbook::TextureFlipbook()
{
	Init(0, 0);
}

GFXB::TextureFlipbook::TextureFlipbook(KCL::uint32 rows, KCL::uint32 cols, float framerate)
{
	Init(rows, cols, framerate);
}

GFXB::TextureFlipbook::TextureFlipbook(KCL::uint32 rows, KCL::uint32 cols, KCL::uint32 start_cell, KCL::uint32 end_cell, float framerate)
{
	Init(rows, cols, start_cell, end_cell, framerate);
}

GFXB::TextureFlipbook::~TextureFlipbook()
{
}

void GFXB::TextureFlipbook::Init(KCL::uint32 rows, KCL::uint32 cols, float framerate)
{
	KCL::uint32 cellCount = rows * cols;
	Init(rows, cols, 0, cellCount == 0 ? 0 : cellCount - 1, framerate);
}

void GFXB::TextureFlipbook::Init(KCL::uint32 rows, KCL::uint32 cols, KCL::uint32 start_cell, KCL::uint32 end_cell, float framerate)
{
	m_rows = rows;
	m_cols = cols;
	KCL::uint32 cellCount = rows * cols;
	if (start_cell > end_cell)
		end_cell = start_cell;
	m_start_cell = start_cell;
	m_end_cell = end_cell;
	m_frameCount = end_cell - start_cell + 1;
	m_framerate = framerate;

	m_cells.clear();
	if (cellCount == 0)
		return;

	m_cells.reserve(m_frameCount * 2); // 2 corner coord per frame
	float cellWidth = 1.0f / cols;
	float cellHeight = 1.0f / rows;

	for (KCL::uint32 i = start_cell; i <= end_cell; i++)
	{
		KCL::uint32 rowIndex = (i % cellCount) / cols;
		KCL::uint32 colIndex = (i % cellCount) % cols;

		m_cells.push_back(KCL::Vector2D(cellWidth * colIndex, cellHeight * rowIndex));
		m_cells.push_back(KCL::Vector2D(cellWidth, cellHeight));
	}
}

KCL::Vector2D * GFXB::TextureFlipbook::GetFrame(KCL::uint32 frame_index)
{
	return &m_cells[(frame_index % m_frameCount) * 2];
}

void GFXB::TextureFlipbook::GetAnimation(KCL::uint32 start_frame, float delta_time, KCL::uint32 & begin_frame, KCL::uint32 & end_frame, float & blend_factor)
{
	KCL::uint32 passedFrameCount = (KCL::uint32)(m_framerate * delta_time);
	begin_frame = (start_frame + passedFrameCount) % m_frameCount;
	end_frame = (begin_frame + 1) % m_frameCount;
	blend_factor = m_framerate * delta_time - delta_time;
}


