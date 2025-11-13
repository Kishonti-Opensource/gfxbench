/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef __GFXB_FONT_RENDERER_H__
#define __GFXB_FONT_RENDERER_H__

#include "gfxb_font.h"

namespace GFXB
{


class FontRenderer
{
public:
	FontRenderer();
	virtual ~FontRenderer();

	void Init(const char* fnt, uint32_t target, uint32_t m_vp_width, uint32_t m_vp_height);
	std::string GetTextureFilename();
	void SetTexture(uint32_t t, uint32_t width, uint32_t height);

	void Render(uint32_t command_buffer, const std::string &text, int32_t *vp);
	void Render(uint32_t command_buffer, const std::string &text, int32_t x, int32_t y, int32_t w, int32_t h);

private:
	uint32_t m_font_ebo;
	uint32_t m_font_vbo;

	_font *m_font;

	uint32_t m_font_shader;

	uint32_t m_font_render;
	int32_t m_viewport[4];
};


};


#endif //__GFXB_FONT_RENDERER_H__

