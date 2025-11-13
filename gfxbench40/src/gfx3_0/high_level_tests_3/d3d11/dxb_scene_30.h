/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef DXB_SCENE_30_H
#define DXB_SCENE_30_H

#include "dxb_scene.h"
#include <kcl_io.h>
#include <kcl_particlesystem2.h>
#include <kcl_texture.h>
#include "d3d11/dxb_texture.h"
#include "d3d11/dxb_buffer.h"

struct Filter
{
private:
	Shader* m_shader;
	ID3D11Texture2D*			m_outputTexture;
	ID3D11RenderTargetView*		m_outputRenderTarget;
	ID3D11ShaderResourceView*	m_outputShaderResource;
	CD3D11_VIEWPORT				m_viewPort;
	const KCL::Camera2*				m_camera;
	int							m_dir;
	bool						m_mipmapped;
	float						m_camera_focus_distance;

public:
	Filter();
	~Filter();

	void Create(KCL::uint32 w, KCL::uint32 h, bool onscreen, bool mipmap, int dir);
	void SetRenderTarget(ID3D11DepthStencilView* depthStencilView = NULL);
	void Render(ID3D11ShaderResourceView* input, KCL::ConstantBuffer* constantBuffer);
	void SetShader(Shader* shader)			{ m_shader = shader; }
	void SetCamera(const KCL::Camera2* camera)	{ m_camera = camera; }
	void SetFocusDistance(float focusDistance)	{ m_camera_focus_distance = focusDistance; }
	ID3D11ShaderResourceView*const* GetOutput()	{ return &m_outputShaderResource; }
};

struct PP
{
private:
	// Array containing all render target views. The 0th is the final buffer
	UINT m_gBufferSize;
	UINT m_renderTargetCount;

	ID3D11Texture2D**			m_gBufferTextures;
	ID3D11RenderTargetView**	m_gBufferRenderTargets;
	ID3D11ShaderResourceView**	m_gBufferShaderResources;
	ID3D11Texture2D*			m_depthStencilTexture;
	ID3D11DepthStencilView*		m_depthStencilView;
	ID3D11DepthStencilView*		m_depthStencilReadOnlyView;
	ID3D11ShaderResourceView*	m_depthStencilResource;

public:
	PP(KCL::uint32 width, KCL::uint32 height, UINT gBufferCount);

	~PP();

	inline UINT GetGBufferCount() const	{ return m_gBufferSize; }

	inline ID3D11ShaderResourceView*const* GetGBufferShaderResource(int layer = 0) const	{ return m_gBufferShaderResources + (1 + layer); }

	inline ID3D11ShaderResourceView*const* GetFinalBufferShaderResource() const	{ return m_gBufferShaderResources; }

	inline ID3D11ShaderResourceView*const* GetDepthStencilResource() const	{ return &m_depthStencilResource; }

	inline ID3D11DepthStencilView* GetDepthStencilView() const	{ return m_depthStencilView; }

	inline ID3D11DepthStencilView* GetReadOnlyDepthStencilView() const	{ return m_depthStencilReadOnlyView; }

	inline void BindGBuffer()
	{
		DX::getContext()->OMSetRenderTargets(m_gBufferSize, m_gBufferRenderTargets + 1, m_depthStencilView);
	}

	inline void UnbindGBuffer()
	{
	}

	// Binds the final buffer as render target and the GBuffer as shader resource
	inline void BindForLightPass()
	{
		// Set the render targets
		DX::getContext()->OMSetRenderTargets(1, m_gBufferRenderTargets, m_depthStencilReadOnlyView);
		DX::getContext()->PSSetShaderResources(0, m_gBufferSize, m_gBufferShaderResources + 1);
		DX::getContext()->PSSetShaderResources(m_gBufferSize, 1, &m_depthStencilResource);
	}

	// Unbinds the GBuffer from shader resource
	inline void UnbindShaderResources()
	{
		ID3D11ShaderResourceView* nullPtr = NULL;
		for (UINT i = 0; i <= m_gBufferSize; i++)
		{
			DX::getContext()->PSSetShaderResources(i, 1, &nullPtr);
		}
	}

	inline ID3D11ShaderResourceView*const* GetFinalBuffer()
	{
		return m_gBufferShaderResources;
	}

	inline void ClearGBuffer(float r = 0.0f, float g = 0.0f, float b = 0.0f, float a = 1.0f)
	{
		const float clearColor[] = { r, g, b, a};
		for (UINT i = 1; i <= m_gBufferSize; i++)
		{
			DX::getContext()->ClearRenderTargetView(m_gBufferRenderTargets[i], clearColor);
		}
	}

	inline void ClearFinalBuffer(float r = 0.0f, float g = 0.0f, float b = 0.0f, float a = 1.0f)
	{
		const float clearColor[] = { r, g, b, a};
		DX::getContext()->ClearRenderTargetView(m_gBufferRenderTargets[0], clearColor);
	}

	inline void ClearDepth()
	{
		DX::getContext()->ClearDepthStencilView(m_depthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);
	}
};

class DXB_Scene_30 : public DXB_Scene
{
protected:
	PP* m_pp;
	Filter m_filters[16];
	Shader* m_copyShader;

    ID3D11DepthStencilState* m_depthLessEqual;
	ID3D11DepthStencilState* m_depthDisabled;
	ID3D11DepthStencilState* m_depthLessNoWrite;
    ID3D11DepthStencilState* m_depthLessEqualNoWrite;
	ID3D11DepthStencilState* m_depthGreaterNoWrite;
	ID3D11RasterizerState* m_lightCullRasterizerState;
    ID3D11RasterizerState* m_noCullRasterizerState;
    ID3D11BlendState* m_blendStateDisableColorWrite;
	ID3D11BlendState* m_blendStateAdd;
	ID3D11BlendState* m_blendStateAlpha;
    ID3D11BlendState* m_blendStateDarken;

	KCL::ConstantBuffer* m_constantBufferFilter;
	KCL::ConstantBuffer* m_constantBufferEmitter;
	KCL::VertexBuffer* m_sphere_vbo;
	KCL::IndexBuffer* m_sphere_ebo;
	KCL::VertexBuffer* m_cone_vbo;
	KCL::IndexBuffer* m_cone_ebo;
	KCL::VertexBuffer* m_lightshaft_vbo;
	KCL::IndexBuffer* m_lightshaft_ebo;

	DXB::DXBTexture* m_volumeLightTexture;
	DXB::DXBTexture* m_fireTexture;
	DXB::DXBTexture* m_fogTexture;
	DXB::DXBTexture* m_lightTexture;

public:
	DXB_Scene_30();
	virtual ~DXB_Scene_30();
	virtual void Render();
    virtual void Render( KCL::Camera2* camera, std::vector<KCL::Mesh*> &visible_meshes, KCL::Material *_override_material, KCL::PlanarMap* pm, KCL::uint32 lod, KCL::Light* light) override;
	virtual KCL::KCL_Status Process_GL( GLB::FBO_COLORMODE color_mode, GLB::FBO_DEPTHMODE depth_mode, int samples);

protected:
	void Release_GLResources();
	void RenderLight(KCL::Light* light);
	void RenderEmitter(KCL::_emitter* emitter);
	void RenderLightShaft(KCL::Light* light, UINT indexCount, UINT indexOffset);
    
    void CollectInstances( std::vector<KCL::Mesh*> &visible_meshes);
};

#endif