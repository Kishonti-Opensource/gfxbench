/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef __XX__SHADOWMAP_H__
#define __XX__SHADOWMAP_H__

#include "krl_shadowmap.h"
#include <string>

class Shader;

class ShadowMap : public KRL_ShadowMap
{

public:
	~ShadowMap();

	void Bind();	
	void Unbind();
	void Clear();
	void BindShadowMap(unsigned int texture_unit_id) ;

	const int Size() const
	{
		return m_size;
	}

	const bool IsBlurEnabled() const
	{
		return m_blur;
	}
	
	static ShadowMap *Create( int size, const std::string &m, KCL::uint32 &fullscreen_quad_vbo, bool is_blur_enabled, bool is_immutable);

private:
	ShadowMap( int size, const std::string &m, KCL::uint32 &fullscreen_quad_vbo, bool is_blur_enabled, bool is_immutable);
	ShadowMap(const ShadowMap&);
	ShadowMap& operator=(const ShadowMap&);
	void ApplyBlur();

	const int m_size;
	unsigned int m_tboid;
	unsigned int m_fboid;
	unsigned int m_rboid;
	unsigned int m_sampler_id ;

	int m_clear_mask;
	std::string m_desc;

	
	unsigned int GetTextureId() const
	{
		if(IsBlurEnabled())
		{
			return m_blur->m_aux_texture[1];
		}

		return m_tboid;
	}

	
	struct Blur
	{
		Blur( KCL::uint32 &blur_fullscreen_quad_vbo, int size, bool is_immutable);
		~Blur();

		KCL::uint32 m_aux_fbo[2];
		KCL::uint32 m_aux_texture[2];
		KCL::uint32 &m_fullscreen_quad_vbo;
		Shader *m_shader;
	};
	Blur *m_blur;
};


#endif //__XX__CUBEMAP_H__

