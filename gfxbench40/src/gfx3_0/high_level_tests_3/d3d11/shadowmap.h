/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef __XX__SHADOWMAP_H__
#define __XX__SHADOWMAP_H__

#include <kcl_base.h>
#include <kcl_math3d.h>
#include <kcl_camera2.h>
#include "glb_kcl_adapter.h"

#include <float.h>
#include <kcl_os.h>
#include <vector>
#include <d3d11_1.h>
#include "d3d11/DXUtils.h"
#include "d3d11/DX.h"
#include "constant_buffer.h"
#include "d3d11/fbo3.h"
#include "krl_shadowmap.h"

using namespace GLB;


class Shader;


class ShadowMap : public KRL_ShadowMap
{

public:
	~ShadowMap();

	void Bind();	
	void Unbind();
	void Clear();

	const int Size() const
	{
		return m_size;
	}

	const bool IsBlurEnabled() const
	{
		return m_blur;
	}

    virtual unsigned int GetTextureId() const {return 0; }

	ID3D11ShaderResourceView* GetD3DTextureId() const
	{
		if(IsBlurEnabled())
		{
			return m_blur->m_aux_fbo2->Get();
		}

		return m_FBO->Get();;
	}

	static ShadowMap *Create( int size, const std::string &m, uint32 &fullscreen_quad_vbo, bool is_blur_enabled);

private:
	ShadowMap( int size, const std::string &m, uint32 &fullscreen_quad_vbo, bool is_blur_enabled);
	ShadowMap(const ShadowMap&);
	ShadowMap& operator=(const ShadowMap&);
	void ApplyBlur();

	const int m_size;
	unsigned int m_tboid;
	unsigned int m_fboid;
	unsigned int m_rboid;
	
	FBO *m_FBO;
		
	struct Blur
	{
		Blur(KCL::uint32 &blur_fullscreen_quad_vbo, int size);
		~Blur();

		KCL::uint32 m_aux_fbo[2];
		KCL::uint32 m_aux_texture[2];
		KCL::uint32 &m_fullscreen_quad_vbo;
		Shader *m_shader;

		Microsoft::WRL::ComPtr<ID3D11InputLayout> m_inputLayout;

		FBO *m_aux_fbo1;
		FBO *m_aux_fbo2;

		uint32 m_vbo;
		uint32 m_ebo;

		Microsoft::WRL::ComPtr<ID3D11Buffer> m_constantBuffer;
		ConstantBuffer m_constantBufferData;

	};
public:
	Blur *m_blur;
};


#endif //__XX__CUBEMAP_H__

