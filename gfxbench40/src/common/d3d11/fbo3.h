/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef FBO_H
#define FBO_H

#include <d3d11_1.h>
#include "kcl_image.h"
#include "DXUtils.h"
#include "DX.h"
#include <kcl_base.h>
#include "fbo_enums.h"

namespace GLB
{
	class FBO
	{
	CD3D11_VIEWPORT	m_viewport;
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView>   m_renderTargetView;
    Microsoft::WRL::ComPtr<ID3D11DepthStencilView>   m_depthStencilView;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_d3d11_tid;
	Microsoft::WRL::ComPtr<ID3D11Texture2D> m_d3d11_texture_msaa;
	bool m_bUseMsaa;

	public:
		FBO();

		bool init (KCL::uint32 width, KCL::uint32 height, FBO_COLORMODE color_mode, FBO_DEPTHMODE depth_mode, int msaa=1);
        bool initDepthOnly (KCL::uint32 width, KCL::uint32 height); //no msaa
		void set_viewport( float x, float y, float w, float h, float mind=0.0f, float maxd=1.0f);

		static void bind( FBO *fbo);
		static void clear( FBO *fbo, float r = 1, float g = 1, float b = 1, float a = 1);
		static void discardDepth( FBO *fbo);
		static void discard( FBO *fbo );
		static void SetGlobalFBO( FBO *fbo );
		static FBO* GetGlobalFBO( );

		static KCL::Image* GetScreenshotImage();
		static KCL::uint32 GetScreenshotImage(KCL::Image& img);

		ID3D11ShaderResourceView* Get();
	private:
		static FBO* m_currentGlobal;
	};
}

#endif
