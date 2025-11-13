/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "gfxb_font.h"
#include <stdio.h>
#include <string.h>

void _font::AddText(float *font_screenboxes, float *font_uvs_colorindexs, int pos_x, int pos_y, int w, int h, int alignment, const std::string &str)
{
	if (!chars.size())
	{
		return;
	}

	float *last_font_screenbox = (float*)font_screenboxes;
	float *last_font_uvs_colorindex = (float*)font_uvs_colorindexs;
	float x = 0.0f;
	float y = (float)line_h;
	float m_calculated_width = 0.0f;
	float m_calculated_height = 0.0f;

	for (size_t i = 0; i < str.length(); i++)
	{
		const uint8_t &c = str[i];
		_character& chr = chars[c];

		int newline_mode = 0;

		if (c == '\n')
		{
			newline_mode = 1;
		}
		else if ((x + chr.x_advance) > w)
		{
			newline_mode = 2;
		}

		if (newline_mode)
		{
			if (x > m_calculated_width)
			{
				m_calculated_width = x;
			}

			x = 0;
			y += (float)line_h;

			if (y > h)
			{
				break;
			}

			if (newline_mode == 1)
			{
				continue;
			}
		}

		last_font_screenbox[0] = x + chr.m_pos[0];
		last_font_screenbox[1] = y + chr.m_pos[1];
		last_font_screenbox[2] = chr.m_size[0];
		last_font_screenbox[3] = chr.m_size[1];
		last_font_uvs_colorindex[0] = chr.m_tc[0];
		last_font_uvs_colorindex[1] = chr.m_tc[1];
		last_font_uvs_colorindex[2] = 0;

		last_font_screenbox += 4;
		last_font_uvs_colorindex += 4;

		x += (float)chr.x_advance;

		const uint8_t &c2 = str[i + 1];
		_character& chr2 = chars[c2];
		x += (float)chr2.Kerning(c);
	}

	if (x > m_calculated_width)
	{
		m_calculated_width = x;
	}
	m_calculated_height = y + line_h - base;

	last_font_screenbox = (float*)font_screenboxes;

	//alignment:
	//0 - 1 - 2
	//3 - 4 - 5
	//6 - 7 - 8
	const float weights[9][2] = 
	{
		{ 0.0f, 0.0f }, { 0.5f, 0.0f }, { 1.0f, 0.0f },
		{ 0.0f, 0.5f }, { 0.5f, 0.5f }, { 1.0f, 0.5f },
		{ 0.0f, 1.0f }, { 0.5f, 1.0f }, { 1.0f, 1.0f },
	};

	//NOTE: kerekites miatt int
	int x_anchor = (int)((w - m_calculated_width) * weights[alignment][0]);
	int y_anchor = (int)((h - m_calculated_height) * weights[alignment][1]);

	for (size_t i = 0; i < str.length(); i++)
	{
		last_font_screenbox[0] += x_anchor + pos_x;
		last_font_screenbox[1] += y_anchor + pos_y;
		last_font_screenbox += 4;
	}
}

void _font::Load2(const std::string &fnt)
{
	bool finished = false;
	size_t idx = 0;

	while (!finished)
	{
		int i;
		int r;
		char spaces[256];
		_character temp_char;
		_kerning temp_kerning;
		char buffer[512] = { 0 };

		for (size_t i = 0; i < 512; i++)
		{
			if (idx == fnt.length())
			{
				finished = true;
				break;
			}

			buffer[i] = fnt[idx];

			idx++;

			if (fnt[idx] == '\n')
			{
				idx++;
				break;
			}
		}

		r = sscanf(buffer, "common lineHeight=%d base=%d scaleW=%d scaleH=%d pages=%d\n", &line_h, &base, &scaleW, &scaleH, &pages);
		if (r == 5)
		{
			continue;
		}

		r = sscanf(buffer, "page id=%d file=%s\n", &i, spaces);
		if (r == 2)
		{
			spaces[strlen(spaces) - 1] = 0;
			m_texture_filename = spaces + 1;
			continue;
		}

		r = sscanf(
			buffer, "char id=%d%[ ]x=%d%[ ]y=%d%[ ]width=%d%[ ]height=%d%[ ]xoffset=%d%[ ]yoffset=%d%[ ]xadvance=%d%[ ]page=%d%[ ]\n",
			&i, spaces,
			&temp_char.x, spaces,
			&temp_char.y, spaces,
			&temp_char.w, spaces,
			&temp_char.h, spaces,
			&temp_char.x_ofs, spaces,
			&temp_char.y_ofs, spaces,
			&temp_char.x_advance, spaces,
			&temp_char.page, spaces);
		if (r == 9 * 2)
		{
			_character &c = temp_char;

#if 0
			c.m_pos[0] = (float)c.x_ofs;
			c.m_pos[1] = -c.y_ofs - c.h + (float)base;
			c.m_size[0] = (float)c.w;
			c.m_size[1] = -(float)c.h;
			c.m_tc[0] = (float)c.x;
			c.m_tc[1] = (float)c.y + c.h;
#else
			c.m_pos[0] = (float)c.x_ofs;
			c.m_pos[1] = (float)c.y_ofs - (float)base;
			c.m_size[0] = (float)c.w;
			c.m_size[1] = (float)c.h;
			c.m_tc[0] = (float)c.x;
			c.m_tc[1] = (float)c.y;
#endif
			chars[i] = c;
			continue;
		}

		r = sscanf(buffer, "kerning first=%d%*[ ]second=%d%*[ ]amount=%hd%*[ \n\r]", &temp_kerning.first, &temp_kerning.second, &temp_kerning.amount);
		if (r == 3)
		{
			chars[temp_kerning.first].kernings[temp_kerning.second] = temp_kerning.amount;
		}
	}
}
