/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef __GFXB_FONT_H__
#define __GFXB_FONT_H__

#include <stdint.h>
#include <map>
#include <vector>
#include <string>



struct _kerning
{
	uint32_t first;
	uint32_t second;
	int16_t amount;
};


struct _character
{
	int x, y, w, h;
	int x_ofs, y_ofs;
	unsigned int x_advance,	page;
	std::map<uint32_t, int16_t> kernings;

	float m_pos[2];
	float m_size[2];
	float m_tc[2];
	int16_t Kerning( uint32_t i)
	{
		std::map<uint32_t, int16_t>::iterator iter = kernings.find( i);
		if( iter != kernings.end())
		{
			return iter->second;
		}
		else
		{
			return 0;
		}
	}
};


struct _font
{
	void AddText(float *font_screenboxes, float *font_uvs_colorindexs, int pos_x, int pos_y, int w, int h, int alignment, const std::string &str);
	void Load2(const std::string &fnt);

	std::string m_texture_filename;
	uint32_t m_texture;
	float m_inv_texture_resolution[4];

private:
	unsigned int base, line_h, scaleW, scaleH, pages;

	std::map<uint32_t, _character> chars;
};

#endif  // __GFXB_FONT_H__

