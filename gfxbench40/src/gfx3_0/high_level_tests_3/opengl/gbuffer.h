/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef GBUFFER_H
#define GBUFFER_H

#include "glb_kcl_adapter.h"
#include <cstring>

struct PP
{
public:
	KCL::uint32 m_fbo;
	KCL::uint32 m_msaa_fbo;

	KCL::uint32 m_color_map;
	KCL::uint32 m_normal_map;
	KCL::uint32 m_reflection_map;
	KCL::uint32 m_param_map;
	KCL::uint32 m_depth_texture;

	KCL::uint32 m_color_map2;
	KCL::uint32 m_normal_map2;
	KCL::uint32 m_reflection_map2;
	KCL::uint32 m_param_map2;
	KCL::uint32 m_depth_texture2;

	KCL::uint32 m_final_texture;

	KCL::uint32 m_samples;
	KCL::uint32 m_viewport_width;
	KCL::uint32 m_viewport_height;

	PP() 
	{
		memset( this, 0, sizeof( PP));
	}
	bool Init( KCL::uint32 w, KCL::uint32 h, KCL::uint32 samples, bool final_texture_fp);
	void Clear();
	void BindGBuffer();
	void BindFinalBuffer();

	static KCL::uint32 Create2DTexture( KCL::uint32 max_mipmaps, bool linear, KCL::uint32 w, KCL::uint32 h, int format);
	static KCL::uint32 CreateRenderbuffer( int samples, KCL::uint32 w, KCL::uint32 h, int format);
};

#endif