/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef GFXB_MATERIAL_H
#define GFXB_MATERIAL_H

#include "kcl_material.h"

namespace GFXB
{
	class ShaderProgramContainer;
	class Texture;

	class Material : public KCL::Material
	{
	public:
		enum ShaderVariant
		{
			SHADER_NORMAL = 0,
			SHADER_SKELETAL = 1,
			SHADER_INSTANCED = 2,
			SHADER_VARIANT_COUNT = 3
		};

		Material(const char *name);
		~Material();

		virtual void SetDefaults() override;

		KCL::KCL_Status Init();

		Texture* GetTexture(KCL::uint32 idx) const;

		void SetShaderCode(ShaderVariant variant, KCL::uint32 shader_code);
		KCL::uint32 GetShaderCode(ShaderVariant variant) const;

	private:
		KCL::uint32 m_shader_codes[SHADER_VARIANT_COUNT];

		ShaderProgramContainer* m_shader_programs;
	};
}

#endif
