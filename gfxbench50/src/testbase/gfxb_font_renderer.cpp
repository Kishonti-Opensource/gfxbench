/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "gfxb_font_renderer.h"

#include <vector>
#include <cstdint>
#include <math.h>

#include "ngl.h"


#define MULTI_LINE_STRING(a) #a


namespace GFXB
{


enum FontUniforms
{
	UNIFORM_FONT_SCREENBOXES,
	UNIFORM_FONT_UVS_COLORINDEXS,
	UNIFORM_FONT_COLORS,
	UNIFORM_FONT_INV_TEXTURE_RES,
	UNIFORM_FONT_INV_SCREEN_RES,
	UNIFORM_FONT_TEXTURE,

	UNIFORM_MAX
};


FontRenderer::FontRenderer()
{
	m_font = nullptr;
}


FontRenderer::~FontRenderer()
{
	delete m_font;
}


static void LoadShader(NGL_job_descriptor &jd, uint32_t pass, uint32_t shader_code, NGL_shader_source_descriptor ssd[NGL_NUM_SHADER_TYPES], std::vector<NGL_shader_uniform> &application_uniforms)
{
	NGL_api api = nglGetApi();

	std::string header;

	if (api == NGL_VULKAN)
	{
		header += "#version 430 core\n";
	}
	else if (api == NGL_OPENGL)
	{
		header += "#version 430 core\n";
	}
	else
	{
		header += "#version 300 es\n";
		header += "precision highp float;\n";
	}

	ssd[NGL_VERTEX_SHADER].m_source_data = header;
	ssd[NGL_FRAGMENT_SHADER].m_source_data = header;

	if (api == NGL_VULKAN)
	{
		ssd[NGL_VERTEX_SHADER].m_source_data += MULTI_LINE_STRING(
			layout(std140, binding = 0) uniform uniformObject0
			{
				uniform highp vec4 font_screenboxes[20];
				uniform highp vec4 font_uvs_colorindexs[20];
				//uniform highp vec4 font_colors[16];
				uniform highp vec4 inv_texture_resolution;
				uniform highp vec4 inv_screen_resolution;
			};

			layout(location = 0) in highp vec3 vertex_encoded_id;

			layout(location = 0) out highp vec2 texcoord;
			//layout(location = 1) out highp vec4 color;
		);

		ssd[NGL_FRAGMENT_SHADER].m_source_data += MULTI_LINE_STRING(
			layout(binding = 1) uniform highp sampler2D font_texture;

			layout(location = 0) in highp vec2 texcoord;
			//layout(location = 1) in highp vec4 color;

			layout(location = 0) out highp vec4 res;
		);
	}
	else
	{
		ssd[NGL_VERTEX_SHADER].m_source_data += MULTI_LINE_STRING(
			uniform vec4 font_screenboxes[20];
			uniform vec4 font_uvs_colorindexs[20];
			//uniform vec4 font_colors[16];
			uniform vec4 inv_texture_resolution;
			uniform vec4 inv_screen_resolution;

			in vec3 vertex_encoded_id;

			out vec2 texcoord;
			//varying vec4 color;
		);

		ssd[NGL_FRAGMENT_SHADER].m_source_data += MULTI_LINE_STRING(
			uniform sampler2D font_texture;

			in vec2 texcoord;
			//varying vec4 color;

			out vec4 res;
		);
	}


	// Add body
	ssd[NGL_VERTEX_SHADER].m_source_data += MULTI_LINE_STRING(
		void main()
		{
			vec2 pos;
			vec2 size;

			int font_id = int(vertex_encoded_id.x);
			int color_index = int(font_uvs_colorindexs[font_id].z);
			size = font_screenboxes[font_id].zw * vertex_encoded_id.yz;
			pos = font_screenboxes[font_id].xy + size;
			pos *= inv_screen_resolution.xy;
			pos = pos * vec2(2.0, -2.0) + vec2(-1.0, 1.0);
			texcoord = font_uvs_colorindexs[font_id].xy + size;
			texcoord *= inv_texture_resolution.xy;
			//color = font_colors[color_index];
			gl_Position = vec4(pos, 0.0, 1.0);
		}
	);


	ssd[NGL_FRAGMENT_SHADER].m_source_data += MULTI_LINE_STRING(
		void main()
		{
			res = texture(font_texture, texcoord); // * color;
		}
	);


	{
		NGL_shader_uniform u;

		application_uniforms.resize(UNIFORM_MAX);

		application_uniforms[UNIFORM_FONT_SCREENBOXES].m_name = "font_screenboxes";
		application_uniforms[UNIFORM_FONT_UVS_COLORINDEXS].m_name = "font_uvs_colorindexs";
		application_uniforms[UNIFORM_FONT_COLORS].m_name = "font_colors";
		application_uniforms[UNIFORM_FONT_INV_TEXTURE_RES].m_name = "inv_texture_resolution";
		application_uniforms[UNIFORM_FONT_INV_SCREEN_RES].m_name = "inv_screen_resolution";
		application_uniforms[UNIFORM_FONT_TEXTURE].m_name = "font_texture";
	}
}


struct _vertex_attrib2
{
	float font_id;
	float corner_factor[2];
};


void FontRenderer::Init(const char* fnt, uint32_t target, uint32_t m_vp_width, uint32_t m_vp_height)
{
	m_viewport[0] = 0;
	m_viewport[1] = 0;
	m_viewport[2] = m_vp_width;
	m_viewport[3] = m_vp_height;

	std::vector<_vertex_attrib2> vertices;
	std::vector<uint16_t> indices;

	//   3-----2
	//   |    /|
	//   |   / |
	//   |  /  |
	//   | /   |
	//   0_____1

	for (int i = 0; i < 20; i++)
	{
		_vertex_attrib2 v;

		v.font_id = (float)i;
		v.corner_factor[0] = 0;
		v.corner_factor[1] = 0;
		vertices.push_back(v);
		v.corner_factor[0] = 1;
		v.corner_factor[1] = 0;
		vertices.push_back(v);
		v.corner_factor[0] = 1;
		v.corner_factor[1] = 1;
		vertices.push_back(v);
		v.corner_factor[0] = 0;
		v.corner_factor[1] = 1;
		vertices.push_back(v);
	}
	for (int i = 0; i < 20; i++)
	{
		indices.push_back(i * 4 + 0);
		indices.push_back(i * 4 + 1);
		indices.push_back(i * 4 + 2);
		indices.push_back(i * 4 + 0);
		indices.push_back(i * 4 + 2);
		indices.push_back(i * 4 + 3);
	}

	{
		NGL_vertex_descriptor vl;
		NGL_vertex_attrib vla;

		vla.m_semantic = "vertex_encoded_id";
		vla.m_format = NGL_R32_G32_B32_FLOAT;
		vla.m_offset = 0;
		vl.m_attribs.push_back(vla);
		vl.m_stride = sizeof(_vertex_attrib2);

		m_font_vbo = 0;
		nglGenVertexBuffer(m_font_vbo, vl, (uint32_t)vertices.size(), &vertices[0]);
	}

	{
		m_font_ebo = 0;
		nglGenIndexBuffer(m_font_ebo, NGL_R16_UINT, (uint32_t)indices.size(), &indices[0]);
	}

	{
		m_font = new _font();
		m_font->Load2(fnt);
	}


	{
		NGL_job_descriptor rrd;
		{
			NGL_attachment_descriptor ad;
			ad.m_attachment.m_idx = target;
			ad.m_attachment_load_op = NGL_LOAD_OP_LOAD;
			ad.m_attachment_store_op = NGL_STORE_OP_STORE;
			rrd.m_attachments.push_back(ad);
		}
		{
			NGL_subpass sp;
			sp.m_name = "font_render";
			sp.m_usages.push_back(NGL_COLOR_ATTACHMENT);
			rrd.m_subpasses.push_back(sp);
		}

		rrd.m_load_shader_callback = LoadShader;
		rrd.m_user_data = this;

		m_font_render = nglGenJob(rrd);

		nglViewportScissor(m_font_render, m_viewport, m_viewport);
	}
}


void FontRenderer::SetTexture(uint32_t t, uint32_t width, uint32_t height)
{
	m_font->m_texture = t;

	m_font->m_inv_texture_resolution[0] = 1.0f / width;
	m_font->m_inv_texture_resolution[1] = 1.0f / height;
}


void FontRenderer::Render(uint32_t command_buffer, const std::string &text, int32_t *vp)
{
	nglBegin(m_font_render, command_buffer);

	const uint32_t max_text_length = 2000;
	const uint32_t font_batch_size = 20;
	const uint32_t batch_count = max_text_length / font_batch_size + 1;

	float font_screenboxes[batch_count * font_batch_size * 4];
	float font_uvs_colorindexs[batch_count * font_batch_size * 4];

	uint32_t n = (uint32_t)ceil(text.length() / (float)font_batch_size);

	memset(font_screenboxes, 0, n * sizeof(float) * font_batch_size * 4);
	memset(font_uvs_colorindexs, 0, n * sizeof(float) * font_batch_size * 4);

	m_font->AddText(font_screenboxes, font_uvs_colorindexs, vp[0], vp[1], vp[2], vp[3], 0, text);

	for (uint32_t i = 0; i < n; i++)
	{
		const void *p[UNIFORM_MAX];

		p[UNIFORM_FONT_SCREENBOXES] = &font_screenboxes[i * font_batch_size * 4];
		p[UNIFORM_FONT_UVS_COLORINDEXS] = &font_uvs_colorindexs[i * font_batch_size * 4];
		p[UNIFORM_FONT_INV_TEXTURE_RES] = &m_font->m_inv_texture_resolution;
		p[UNIFORM_FONT_COLORS] = nullptr;

		float isr[4] = { 1.0f / m_viewport[2], 1.0f / m_viewport[3], 0.0f, 0.0f };
		p[UNIFORM_FONT_INV_SCREEN_RES] = isr;
		p[UNIFORM_FONT_TEXTURE] = &m_font->m_texture;

		nglBlendState(m_font_render, 0, NGL_BLEND_ALFA, NGL_CHANNEL_ALL);

		nglViewportScissor(m_font_render, 0, 0);
		nglDrawTwoSided(m_font_render, m_font_shader, m_font_vbo, m_font_ebo, p);
	}

	nglEnd(m_font_render);
}


void FontRenderer::Render(uint32_t command_buffer, const std::string &text, int32_t x, int32_t y, int32_t w, int32_t h)
{
	int32_t vp[4] = { x, y, w, h };
	Render(command_buffer, text, vp);
}


std::string FontRenderer::GetTextureFilename()
{
	return m_font->m_texture_filename;
}


}


