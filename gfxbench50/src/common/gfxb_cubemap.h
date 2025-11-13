/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef GFXB5_CUBEMAP_H
#define GFXB5_CUBEMAP_H

#include "kcl_base.h"
#include "ngl.h"

namespace GFXB
{
	class Cubemap
	{
	public:
		virtual ~Cubemap();

		static Cubemap *Create(KCL::uint32 width, KCL::uint32 height, NGL_format format);
		static Cubemap *Load(const char *filename, bool rgbe = true);

		KCL::KCL_Status SaveTGA(const char *filename);
		KCL::uint32 GetTexture() const;
		KCL::uint32 *GetTexturePtr();
		KCL::uint32 GetLevels() const;
		KCL::uint32 GetWidth() const;
		KCL::uint32 GetHeight() const;
		bool IsRGBE() const;

	private:
		Cubemap();

		KCL::uint32 m_texture;

		NGL_format m_format;
		bool m_is_rgbe;
		KCL::uint32 m_levels;
		KCL::uint32 m_width;
		KCL::uint32 m_height;
	};

	uint32_t CaptureCubemap(uint32_t command_buffer, const KCL::Vector3D &pos, const std::string &sky_format, const std::string &encode_format, std::vector<KCL::Mesh*> &sky, uint32_t cube_vbid, uint32_t cube_ibid, uint32_t size[2]);

	uint32_t CreateIntegrateBRDF_LUT(uint32_t command_buffer, uint32_t quad_vbid, uint32_t quad_ibid, uint32_t size);

	void ConvertCubemapToRGB9E5(uint32_t input_texture, uint32_t width, uint32_t height, uint32_t levels, std::vector<std::vector<uint8_t>> &pixel_data);
	uint32_t ConvertCubemapToRGB9E5(uint32_t input_texture, uint32_t width, uint32_t height, uint32_t levels);

	KCL::KCL_Status LoadCubemapRGB9E5(const char* filename, uint32_t &texture, uint32_t &size);
	KCL::KCL_Status SaveCubemapRGB9E5(const char* filaname, uint32_t input_texture, uint32_t width, uint32_t height, uint32_t levels);


}

#endif