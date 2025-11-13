/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef GFXB_5_OVR_NGL_ADAPTER_H
#define GFXB_5_OVR_NGL_ADAPTER_H

#ifdef WITH_OVR_SDK
#include <graphics/ovrgraphicscontext.h>
#else
#include <graphics/ovrgraphicscontext_dummy.h>
#include "gfxb_vrapi_dummy.h"
#endif

#include <ngl_gl_adapter_interface.h>
#include <kcl_math3d.h>
#include <vector>

namespace GFXB
{
	class OvrNGLAdapter : public NGL_gl_adapter_interface
	{
	public:
		OvrNGLAdapter(const std::vector<KCL::uint32> &textures)
		{
			m_textures = textures;
			m_current_index = 0;
		}

		void SetCurrentTextureIndex(KCL::uint32 index)
		{
			m_current_index = index;
		}

		virtual void GetDefaultFramebufferTextures(std::vector<KCL::uint32> &ids) override
		{
			ids = m_textures;
		}
		virtual KCL::uint32 GetDefaultFramebufferTextureIndex() override
		{
			return m_current_index;
		}

	private:
		std::vector<uint32_t> m_textures;
		KCL::uint32 m_current_index;
	};
}

#endif
