/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include <d3d11_1.h>
#include "d3d11/dxb_planarmap.h"
#include "d3d11/dxb_emitter.h"
#include "d3d11/dxb_scene_30.h"
#include "d3d11/dxb_material.h"
#include "d3d11/dxb_texture.h"
#include "d3d11/dxb_light.h"
#include "d3d11/DXUtils.h"
#include "d3d11/DX.h"
#include "d3d11/vbopool.h"
#include "d3d11/shader.h"
#include <algorithm> //std::sort

#include "d3d11/shadowmap.h"
#include "d3d11/dxb_mesh.h"

//#define PERF_MARKERS
#if defined(_DEBUG) && defined(PERF_MARKERS)
    #include "atlbase.h" //for CComPtr, perf markers
#endif

#define HALF_SIZED_LIGHTING
#define SAFE_DELETE(x)	if ((x)) { delete (x); (x) = NULL; }
#define SAFE_RELEASE(x)	if ((x)) { (x)->Release(); (x) = NULL; }

DXB::DXBTexture* CreateTexture2D(const char* fileName)
{
	KCL::Image image;
	image.load(fileName);
	if (image.getFormat() == KCL::Image_RGB888)
	{
		image.convertTo(KCL::Image_RGBA8888);
	}

	DXB::DXBTexture* texture = new DXB::DXBTexture(&image);
	texture->commit();
	return texture;
}

DXB::DXBTexture* CreateTexture3D(const char* fileName)
{
	KCL::Image image;
	image.load3D(fileName);
	if (image.getFormat() == KCL::Image_RGB888)
	{
		image.convertTo(KCL::Image_RGBA8888);
	}

	DXB::DXBTexture* texture = new DXB::DXBTexture(&image);
	texture->commit();
	return texture;
}

Filter::Filter() : 
	m_outputTexture(NULL),
	m_outputRenderTarget(NULL),
	m_outputShaderResource(NULL),
	m_camera(NULL)
{
}

void Filter::Create(KCL::uint32 width, KCL::uint32 height, bool onscreen, bool mipmap, int dir)
{
	m_dir = dir;
	m_mipmapped = mipmap;

	m_viewPort.Height = (float)height;
	m_viewPort.Width = (float)width;
	m_viewPort.TopLeftX = m_viewPort.TopLeftY = 0.0f;
	m_viewPort.MinDepth = 0.0f;
	m_viewPort.MaxDepth = 1.0f;

	if (onscreen)
	{
		m_outputTexture = NULL;
		m_outputRenderTarget = NULL;
		m_outputShaderResource = NULL;
	}
	else
	{
		D3D11_TEXTURE2D_DESC textureDesc;
		ZeroMemory(&textureDesc, sizeof(textureDesc));
		textureDesc.Width = width;
		textureDesc.Height = height;
		textureDesc.MipLevels = mipmap ? 0 : 1;
		textureDesc.ArraySize = 1;
		textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		textureDesc.SampleDesc.Count = 1;
		textureDesc.Usage = D3D11_USAGE_DEFAULT;
		textureDesc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;
		textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;

		D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc;
		ZeroMemory(&renderTargetViewDesc, sizeof(renderTargetViewDesc));
		renderTargetViewDesc.Format = textureDesc.Format;
		renderTargetViewDesc.Texture2D.MipSlice = 0;
		renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;

		D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;
		ZeroMemory(&shaderResourceViewDesc, sizeof(shaderResourceViewDesc));
		shaderResourceViewDesc.Format = textureDesc.Format;
		shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
		shaderResourceViewDesc.Texture2D.MipLevels = mipmap ? -1 : 1;
		shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;

		DX_THROW_IF_FAILED(
			DX::getDevice()->CreateTexture2D(&textureDesc, NULL, &m_outputTexture));
		DX_THROW_IF_FAILED(
			DX::getDevice()->CreateRenderTargetView(m_outputTexture, &renderTargetViewDesc, &m_outputRenderTarget));
		DX_THROW_IF_FAILED(
			DX::getDevice()->CreateShaderResourceView(m_outputTexture, &shaderResourceViewDesc, &m_outputShaderResource));
	}
}

Filter::~Filter()
{
	SAFE_RELEASE(m_outputTexture);
	SAFE_RELEASE(m_outputRenderTarget);
	SAFE_RELEASE(m_outputShaderResource);
}

void Filter::SetRenderTarget(ID3D11DepthStencilView* depthStencilView)
{
	if (m_outputRenderTarget)
	{
		DX::getContext()->OMSetRenderTargets(1, &m_outputRenderTarget, depthStencilView);
	}
	else
	{
		GLB::FBO::bind(NULL);
	}
}

void Filter::Render(ID3D11ShaderResourceView* input, KCL::ConstantBuffer* constantBuffer)
{
	//DX::getContext()->RSSetViewports(1, &m_viewPort);
    DX::getStateManager()->SetViewport(m_viewPort);

	m_shader->Bind();
	if (input != NULL)
	{
		DX::getContext()->PSSetShaderResources(0, 1, &input);
	}

	ConstantBufferFilter *buf = (ConstantBufferFilter*)constantBuffer->map();
	buf->screen_resolution = DirectX::XMFLOAT2(m_viewPort.Width, m_viewPort.Height);
	buf->inv_screen_resolution = DirectX::XMFLOAT2(1.0f / m_viewPort.Width, 1.0f / m_viewPort.Height);
	if (m_camera)
	{
		buf->camera_focus = m_camera_focus_distance;
		buf->depth_parameters = DirectX::XMFLOAT4(m_camera->m_depth_linearize_factors.v);
		buf->dof_strength = m_camera->GetFov();
	}

	if( m_dir)
	{
		buf->offset_2d = DirectX::XMFLOAT2(buf->inv_screen_resolution.x, 0.0f);
	}
	else
	{
		buf->offset_2d = DirectX::XMFLOAT2(0.0f, buf->inv_screen_resolution.y);
	}

	constantBuffer->unmap();

/*
	VboPool::Instance()->BindBuffer(m_fullscreen_vbo);
	IndexBufferPool::Instance()->BindBuffer(m_fullscreen_ebo);
	DX::getContext()->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	DX::getContext()->IASetInputLayout( m_fullscreen_inputLayout.Get());
*/
		
	DX::getStateManager()->ApplyStates();
	DX::getContext()->DrawIndexed( 6, 0, 0);
}

PP::PP(KCL::uint32 width, KCL::uint32 height, UINT gBufferCount) : 
	m_gBufferTextures(NULL),
	m_gBufferRenderTargets(NULL),
	m_gBufferShaderResources(NULL),
	m_depthStencilTexture(NULL),
	m_depthStencilView(NULL),
	m_depthStencilResource(NULL),
	m_gBufferSize(gBufferCount),
	m_renderTargetCount(gBufferCount + 1)
{
	D3D11_TEXTURE2D_DESC textureDesc;
	ZeroMemory(&textureDesc, sizeof(textureDesc));
	textureDesc.Width = width;
	textureDesc.Height = height;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = 0;

	D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc;
	renderTargetViewDesc.Format = textureDesc.Format;
	renderTargetViewDesc.Texture2D.MipSlice = 0;
	renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;

	D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;
	shaderResourceViewDesc.Format = textureDesc.Format;
	shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
	shaderResourceViewDesc.Texture2D.MipLevels = 1;
	shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;

	m_gBufferTextures = new ID3D11Texture2D*[m_renderTargetCount];
	m_gBufferRenderTargets = new ID3D11RenderTargetView*[m_renderTargetCount];
	m_gBufferShaderResources = new ID3D11ShaderResourceView*[m_renderTargetCount];
	ZeroMemory(m_gBufferTextures, sizeof(ID3D11Texture2D*) * m_renderTargetCount);
	ZeroMemory(m_gBufferRenderTargets, sizeof(ID3D11RenderTargetView*) * m_renderTargetCount);
	ZeroMemory(m_gBufferShaderResources, sizeof(ID3D11ShaderResourceView*) * m_renderTargetCount);

	for (UINT i = 0; i < m_renderTargetCount; i++)
	{
		DX_THROW_IF_FAILED(
			DX::getDevice()->CreateTexture2D(&textureDesc, NULL, m_gBufferTextures + i));

		DX_THROW_IF_FAILED(
			DX::getDevice()->CreateRenderTargetView(m_gBufferTextures[i], &renderTargetViewDesc, m_gBufferRenderTargets + i));

		DX_THROW_IF_FAILED(
			DX::getDevice()->CreateShaderResourceView(m_gBufferTextures[i], &shaderResourceViewDesc, m_gBufferShaderResources + i));
	}

	CD3D11_TEXTURE2D_DESC descDepth(DXGI_FORMAT_R24G8_TYPELESS, width, height, 1, 1, D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE);
	descDepth.SampleDesc.Count = 1;
	descDepth.SampleDesc.Quality = textureDesc.SampleDesc.Quality;
	DX_THROW_IF_FAILED(
		DX::getDevice()->CreateTexture2D(&descDepth, nullptr, &m_depthStencilTexture));

	D3D11_DEPTH_STENCIL_VIEW_DESC descDSV;
	ZeroMemory(&descDSV, sizeof(descDSV));
	descDSV.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	descDSV.Texture2D.MipSlice = 0;

	DX_THROW_IF_FAILED(
		DX::getDevice()->CreateDepthStencilView(m_depthStencilTexture, &descDSV, &m_depthStencilView));

	descDSV.Flags = D3D11_DSV_READ_ONLY_DEPTH | D3D11_DSV_READ_ONLY_STENCIL;
	DX_THROW_IF_FAILED(
		DX::getDevice()->CreateDepthStencilView(m_depthStencilTexture, &descDSV, &m_depthStencilReadOnlyView));

	D3D11_SHADER_RESOURCE_VIEW_DESC depthResourceViewDesc;
	depthResourceViewDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
	depthResourceViewDesc.Texture2D.MostDetailedMip = 0;
	depthResourceViewDesc.Texture2D.MipLevels = 1;
	depthResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;

	DX_THROW_IF_FAILED(
		DX::getDevice()->CreateShaderResourceView(m_depthStencilTexture, &depthResourceViewDesc, &m_depthStencilResource));
}

PP::~PP()
{
	if (m_gBufferRenderTargets)
	{
		// We could use separate loops for each array but they get initialized/released together
		for (UINT i = 0; i < m_renderTargetCount;i++)
		{
			SAFE_RELEASE(m_gBufferRenderTargets[i]);
			SAFE_RELEASE(m_gBufferShaderResources[i]);
			SAFE_RELEASE(m_gBufferTextures[i]);
		}

		delete m_gBufferRenderTargets;
	}

	SAFE_RELEASE(m_depthStencilTexture);
	SAFE_RELEASE(m_depthStencilView);
	SAFE_RELEASE(m_depthStencilReadOnlyView);
	SAFE_RELEASE(m_depthStencilResource);
}

DXB_Scene_30::DXB_Scene_30() :
	m_pp(NULL),
	m_lightCullRasterizerState(NULL),
    m_noCullRasterizerState(NULL),
	m_blendStateAdd(NULL),
	m_blendStateAlpha(NULL),
    m_blendStateDarken(NULL),
    m_blendStateDisableColorWrite(NULL),
    m_depthLessEqual(NULL),
	m_depthDisabled(NULL),
	m_depthLessNoWrite(NULL),
	m_depthGreaterNoWrite(NULL),
	m_constantBufferFilter(NULL),
	m_constantBufferEmitter(NULL),
	m_sphere_vbo(NULL),
	m_sphere_ebo(NULL),
	m_cone_vbo(NULL),
	m_cone_ebo(NULL),
	m_lightshaft_vbo(NULL),
	m_lightshaft_ebo(NULL)
{
}

DXB_Scene_30::~DXB_Scene_30()
{
	SAFE_DELETE(m_pp);
	SAFE_DELETE(m_volumeLightTexture);
	SAFE_DELETE(m_fireTexture);
	SAFE_DELETE(m_fogTexture);
	SAFE_DELETE(m_lightTexture);

	SAFE_DELETE(m_constantBufferFilter);
	SAFE_DELETE(m_constantBufferEmitter);
	SAFE_DELETE(m_sphere_vbo);
	SAFE_DELETE(m_sphere_ebo);
	SAFE_DELETE(m_cone_vbo);
	SAFE_DELETE(m_cone_ebo);
	SAFE_DELETE(m_lightshaft_vbo);
	SAFE_DELETE(m_lightshaft_ebo);

	SAFE_RELEASE(m_lightCullRasterizerState);
    SAFE_RELEASE(m_noCullRasterizerState);
	SAFE_RELEASE(m_blendStateAdd);
    SAFE_RELEASE(m_blendStateDisableColorWrite);
	SAFE_RELEASE(m_blendStateAlpha);
    SAFE_RELEASE(m_blendStateDarken);
    SAFE_RELEASE(m_depthLessEqual);
	SAFE_RELEASE(m_depthDisabled);
	SAFE_RELEASE(m_depthGreaterNoWrite);
	SAFE_RELEASE(m_depthLessNoWrite);
    SAFE_RELEASE(m_depthLessEqualNoWrite);
}

KCL::KCL_Status DXB_Scene_30::Process_GL( GLB::FBO_COLORMODE color_mode, GLB::FBO_DEPTHMODE depth_mode, int samples)
{
	if (DX::getDevice()->GetFeatureLevel() < D3D_FEATURE_LEVEL_10_1)
	{
		return KCL::KCL_TESTERROR_DX_FEATURE_LEVEL;
	}

	KCL::KCL_Status result = DXB_Scene::Process_GL(color_mode, depth_mode, samples);
	if (result != KCL::KCL_TESTERROR_NOERROR)
	{
		return result;
	}

	m_lightTexture = ::CreateTexture2D("common/light.png");
	m_fireTexture = ::CreateTexture3D((ImagesDirectory() + std::string("anims/fire")).c_str());
	m_fogTexture = ::CreateTexture3D((ImagesDirectory() + std::string("anims/fog")).c_str());

    std::set< std::string> defines;
    defines.insert("SV_30");
	m_copyShader = Shader::CreateShader( "pp.vs", "texture_copy.fs", &defines, result);

	if (result != KCL::KCL_TESTERROR_NOERROR)
	{
		return result;
	}

	std::vector<KCL::Vector3D> sphere_vertices;
	std::vector<KCL::Vector2D> sphere_tcs;
	std::vector<KCL::uint16> sphere_indices;
	KCL::Mesh3::CreateSphere(sphere_vertices, sphere_tcs, sphere_indices, 5, 5);
	for (uint16 i = 0; i < sphere_indices.size(); i += 3)
	{
		uint16 tmp = sphere_indices[i];
		sphere_indices[i] = sphere_indices[i + 1];
		sphere_indices[i + 1] = tmp;
	}
	
	m_sphere_vbo = KCL::VertexBuffer::factory->CreateBuffer<KCL::Vector3D>(&sphere_vertices[0], sphere_vertices.size());
	m_sphere_vbo->commit();
	m_sphere_ebo = KCL::IndexBuffer::factory->CreateBuffer(&sphere_indices[0], sphere_indices.size());
	m_sphere_ebo->commit();

	std::vector<KCL::Vector3D> cone_vertices;
	std::vector<KCL::Vector2D> cone_tcs;
	std::vector<KCL::uint16> cone_indices;
	KCL::Mesh3::CreateCone(cone_vertices, cone_tcs, cone_indices, 15, 1);
	for (uint16 i = 0; i < cone_indices.size(); i += 3)
	{
		uint16 tmp = cone_indices[i];
		cone_indices[i] = cone_indices[i + 1];
		cone_indices[i + 1] = tmp;
	}

	m_cone_vbo = KCL::VertexBuffer::factory->CreateBuffer<KCL::Vector3D>(&cone_vertices[0], cone_indices.size());
	m_cone_vbo->commit();
	m_cone_ebo = KCL::IndexBuffer::factory->CreateBuffer(&cone_indices[0], cone_indices.size());
	m_cone_ebo->commit();

	m_lightshaft_vbo = KCL::VertexBuffer::factory->CreateBuffer<KCL::Vector3D>(1024, true);
	m_lightshaft_vbo->commit();
	m_lightshaft_ebo = KCL::IndexBuffer::factory->CreateBuffer((USHORT*)NULL, 1024, false, true);
	m_lightshaft_ebo->commit();

	m_constantBufferFilter = KCL::ConstantBuffer::factory->CreateBuffer<ConstantBufferFilter>();
	m_constantBufferFilter->commit();

	m_constantBufferEmitter = KCL::ConstantBuffer::factory->CreateBuffer<ConstantBufferEmitter>();
	m_constantBufferEmitter->commit();

	m_pp = new PP(m_viewport_width, m_viewport_height, 4);

	D3D11_BLEND_DESC blendDesc;
	ZeroMemory(&blendDesc, sizeof(blendDesc));
	blendDesc.IndependentBlendEnable = FALSE;

	D3D11_RENDER_TARGET_BLEND_DESC& rtbd = blendDesc.RenderTarget[0];
    ZeroMemory(&rtbd, sizeof(rtbd));
	rtbd.BlendEnable = TRUE;
	rtbd.SrcBlend = D3D11_BLEND_ONE;
	rtbd.DestBlend = D3D11_BLEND_ONE;
	rtbd.BlendOp = D3D11_BLEND_OP_ADD;
	rtbd.SrcBlendAlpha = D3D11_BLEND_ONE;
	rtbd.DestBlendAlpha = D3D11_BLEND_ONE;
	rtbd.BlendOpAlpha = D3D11_BLEND_OP_ADD;
	rtbd.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	DX_THROW_IF_FAILED(DX::getDevice()->CreateBlendState(&blendDesc, &m_blendStateAdd));

	rtbd.SrcBlend = D3D11_BLEND_SRC_ALPHA;
	rtbd.DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	rtbd.SrcBlendAlpha = D3D11_BLEND_SRC_ALPHA;
	rtbd.DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
	DX_THROW_IF_FAILED(DX::getDevice()->CreateBlendState(&blendDesc, &m_blendStateAlpha));

	rtbd.SrcBlend = D3D11_BLEND_DEST_COLOR;
	rtbd.DestBlend = D3D11_BLEND_ZERO;
	rtbd.SrcBlendAlpha = D3D11_BLEND_ZERO;
	rtbd.DestBlendAlpha = D3D11_BLEND_ONE;
	DX_THROW_IF_FAILED(DX::getDevice()->CreateBlendState(&blendDesc, &m_blendStateDarken));

    rtbd.BlendEnable = FALSE;
	rtbd.SrcBlend = D3D11_BLEND_ONE;
	rtbd.DestBlend = D3D11_BLEND_ZERO;
	rtbd.BlendOp = D3D11_BLEND_OP_ADD;
	rtbd.SrcBlendAlpha = D3D11_BLEND_ONE;
	rtbd.DestBlendAlpha = D3D11_BLEND_ZERO;
	rtbd.BlendOpAlpha = D3D11_BLEND_OP_ADD;
	rtbd.RenderTargetWriteMask = 0;
	DX_THROW_IF_FAILED(DX::getDevice()->CreateBlendState(&blendDesc, &m_blendStateDisableColorWrite));

	D3D11_RASTERIZER_DESC rasterDesc;
	ZeroMemory(&rasterDesc, sizeof(rasterDesc));
	rasterDesc.CullMode = D3D11_CULL_FRONT;
	rasterDesc.FillMode = D3D11_FILL_SOLID;
	rasterDesc.DepthClipEnable = TRUE;
	DX::getDevice()->CreateRasterizerState(&rasterDesc, &m_lightCullRasterizerState);

    rasterDesc.CullMode = D3D11_CULL_NONE;
	rasterDesc.FrontCounterClockwise = TRUE;
	DX::getDevice()->CreateRasterizerState(&rasterDesc, &m_noCullRasterizerState);

	D3D11_DEPTH_STENCIL_DESC dsDesc;
	ZeroMemory(&dsDesc, sizeof(dsDesc));

	dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	dsDesc.DepthEnable = TRUE;
	dsDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
	DX_THROW_IF_FAILED(
		DX::getDevice()->CreateDepthStencilState(&dsDesc, &m_depthLessEqual));

	dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	dsDesc.DepthEnable = FALSE;
	dsDesc.DepthFunc = D3D11_COMPARISON_ALWAYS;
	DX_THROW_IF_FAILED(
		DX::getDevice()->CreateDepthStencilState(&dsDesc, &m_depthDisabled));

	dsDesc.DepthEnable = TRUE;
	dsDesc.DepthFunc = D3D11_COMPARISON_LESS;
	DX_THROW_IF_FAILED(
		DX::getDevice()->CreateDepthStencilState(&dsDesc, &m_depthLessNoWrite));

	dsDesc.DepthFunc = D3D11_COMPARISON_GREATER_EQUAL;
	DX_THROW_IF_FAILED(
		DX::getDevice()->CreateDepthStencilState(&dsDesc, &m_depthGreaterNoWrite));

    dsDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
    DX_THROW_IF_FAILED(
		DX::getDevice()->CreateDepthStencilState(&dsDesc, &m_depthLessEqualNoWrite));

	m_filters[0].Create( m_viewport_width, m_viewport_height, true, false, 0);
	m_filters[1].Create( m_viewport_width / 2.0f, m_viewport_height / 2.0f, false, false, 0);
	m_filters[2].Create( m_viewport_width / 2.0f, m_viewport_height / 2.0f, false, false, 1);
	m_filters[3].Create( m_viewport_width / 4.0f, m_viewport_height / 4.0f, false, false, 0);
	m_filters[4].Create( m_viewport_width / 4.0f, m_viewport_height / 4.0f, false, false, 1);
	m_filters[5].Create( m_viewport_width / 8.0f, m_viewport_height / 8.0f, false, false, 0);
	m_filters[6].Create( m_viewport_width / 8.0f, m_viewport_height / 8.0f, false, false, 1);
	m_filters[7].Create( m_viewport_width / 16.0f, m_viewport_height / 16.0f, false, false, 0);
	m_filters[8].Create( m_viewport_width / 16.0f, m_viewport_height / 16.0f, false, false, 1);
	m_filters[9].Create( 1024, 1024, false, true, 1);
	m_filters[10].Create( m_viewport_width, m_viewport_height, false, true, 0);

	//fill volume light data with static lights
	AABB lightBounds;
	std::vector<KCL::Light*> staticLights;
	{
		for(size_t i=0; i<m_actors.size(); ++i)
		{
			for(size_t j=0; j<m_actors[i]->m_lights.size(); ++j)
			{
				if (m_actors[i]->m_lights[j]->m_intensity_track == NULL)
				{
					KCL::Light* l = m_actors[i]->m_lights[j];
					staticLights.push_back(l);

					KCL::Vector3D lpos(l->m_world_pom.getTranslation());
					KCL::Vector3D lrad(l->m_radius, l->m_radius, l->m_radius);
					AABB lbounds(lpos - lrad, lpos + lrad);
					lightBounds.Merge(lpos); //do we have a valid world pom here???
				}
			}
		}
	}

	KCL::Vector3D volumeLightMin = lightBounds.GetMinVertex();
	KCL::Vector3D volumeLightExtents = lightBounds.GetMaxVertex() - volumeLightMin;
	
	float maxDim = std::max<float>(volumeLightExtents.x, volumeLightExtents.z);
	volumeLightExtents = KCL::Vector3D(maxDim,0.0f,maxDim);

	float tileSize = std::min<float>(volumeLightExtents.x, volumeLightExtents.z) / 64.0f;

	KCL::Image volumeLightImage(64, 64,  KCL::Image_RGBA8888);
	KCL::uint32 bytesPerPixel = 4;
	KCL::uint8* imgPtr = (KCL::uint8*)volumeLightImage.getData();
	memset(imgPtr, 0x0, volumeLightImage.getDataSize());

	for(int y = 0; y < 64; y++, imgPtr += volumeLightImage.getLinePitch())
	{
		KCL::uint8* rowPtr = imgPtr;
		for(int x = 0; x < 64; x++, rowPtr += bytesPerPixel)
		{
			KCL::Vector3D tileCenter = volumeLightMin;
			tileCenter.x += float(x)/64.0f*volumeLightExtents.x;
			tileCenter.z += float(y)/64.0f*volumeLightExtents.z;

			for(size_t l=0; l<staticLights.size(); ++l)
			{
				if( (staticLights[l]->m_radius > tileSize * 2.0f) && (staticLights[l]->m_light_type == Light::OMNI) )
				{
					KCL::Vector3D lightPos = staticLights[l]->m_world_pom.getTranslation();
					KCL::Vector3D color = staticLights[l]->m_diffuse_color / 2.0f;//.normalize();
					//float intensity = staticLights[l]->m_diffuse_color.length();
					float dist = (lightPos - tileCenter).length();
					if( dist < staticLights[l]->m_radius) //TODO attenuation
					{
						float atten = 1.0f - sqrtf(dist / staticLights[l]->m_radius);
						*(rowPtr + 0) = std::min<KCL::uint8>(255, *(rowPtr + 0) + int(color.x * atten * 255.0f));
						*(rowPtr + 1) = std::min<KCL::uint8>(255, *(rowPtr + 1) + int(color.y * atten * 255.0f));
						*(rowPtr + 2) = std::min<KCL::uint8>(255, *(rowPtr + 2) + int(color.z * atten * 255.0f));
					}
				}
			}
		}
	}

	m_volumeLightTexture = new DXB::DXBTexture(&volumeLightImage);
	m_volumeLightTexture->commit();

	//create buffers for TF_emitters
	for(std::vector<KCL::Actor*>::size_type i=0; i<m_actors.size(); ++i)
	{
		for(std::vector<KCL::AnimatedEmitter*>::size_type j=0; j<m_actors[i]->m_emitters.size(); ++j)
		{
			KCL::_emitter* em = static_cast<KCL::_emitter*>(m_actors[i]->m_emitters[j]);
			em->Process();

			DXB::DXBEmitter* dxbem = dynamic_cast<DXB::DXBEmitter*>(em);
			if (dxbem)
			{
				dxbem->setConstantBuffer(m_constantBufferEmitter);
			}
		}
	}

    //create occlusion query objects
    CD3D11_QUERY_DESC queryDesc(D3D11_QUERY_OCCLUSION_PREDICATE);
    ID3D11Query* pQueries[DXB::Light::QUERY_COUNT];

    for(size_t i=0; i<m_actors.size(); i++)
	{
		Actor* a = m_actors[i];
		for(size_t j=0; j<a->m_lights.size(); j++)
		{
			DXB::Light* l = static_cast<DXB::Light*>(a->m_lights[j]); //factory will create DXB::Light so this is fine

			if (l->m_has_lensflare)
			{
                for(int i=0; i<4; ++i)
                {
                    DX::getDevice()->CreateQuery(&queryDesc, &pQueries[i]);
                }
                l->SetQueryObjects(pQueries);
			}
		}
	}
	return KCL::KCL_TESTERROR_NOERROR;
}

bool g_ShadowDepthRender = true;
bool g_GBufferSolids = true;
bool g_LensFlareQuery = true;
bool g_Lighting = true;
bool g_Sky = true;
bool g_RE = true;
bool g_ShadowDecal = true;
bool g_Decals = true;
bool g_Particles = true;
bool g_LightShafts = true;
bool g_Transparents = true;
bool g_LensFlares = true;
bool g_Post = true;

struct mesh_depth_sort_info
{
	KCL::Mesh *mesh;
	KCL::Vector2D extents;
    float vertexCenterDist;
};

bool depth_compare(const mesh_depth_sort_info &A, const mesh_depth_sort_info &B)
{
    return A.vertexCenterDist < B.vertexCenterDist;
}

static void sort_vis_list(std::vector<KCL::Mesh*> &visible_meshes, KCL::Camera2 *camera)
{
	std::vector<mesh_depth_sort_info> sorted_visible_meshes;

	for (uint32 i = 0; i < visible_meshes.size(); i++)
	{
		mesh_depth_sort_info mesh_info;

		mesh_info.mesh = visible_meshes[i];
		mesh_info.extents = visible_meshes[i]->m_aabb.DistanceFromPlane(camera->GetCullPlane(KCL::CULLPLANE_NEAR));
        mesh_info.vertexCenterDist = KCL::Vector4D::dot( KCL::Vector4D(visible_meshes[i]->m_vertexCenter), camera->GetCullPlane(KCL::CULLPLANE_NEAR));

		// force objects with infinite bounds (actors) to draw first 
		if (mesh_info.extents.x < -FLT_MAX || mesh_info.extents.y > FLT_MAX)
		{
			mesh_info.extents.x = 0.0f;
			mesh_info.extents.y = 0.0f;
            mesh_info.vertexCenterDist = 0.0f;
		}
		sorted_visible_meshes.push_back(mesh_info);
	}

	// depth sort
	std::sort (sorted_visible_meshes.begin(), sorted_visible_meshes.end(), &depth_compare);

	// remap original visible meshes
	for (uint32 i = 0; i < visible_meshes.size(); i++)
		visible_meshes[i] = sorted_visible_meshes[i].mesh;
}


void DXB_Scene_30::CollectInstances( std::vector<KCL::Mesh*> &visible_meshes)
{
    KRL_Scene::CollectInstances(visible_meshes);

    for( uint32 j=0; j<visible_meshes.size(); j++)
	{
		KCL::Mesh* m = visible_meshes[j];
        DXB::Mesh3 *gm = static_cast<DXB::Mesh3*>(m->m_mesh);

        size_t instanceVBOelemCount = gm->m_instances[m->m_material].size();
        std::vector<DirectX::XMFLOAT4X4A> mats;
        for(int mtxCnt=0; mtxCnt<instanceVBOelemCount; ++mtxCnt)
        {
            mats.push_back(DX::Float4x4toXMFloat4x4ATransposed(gm->m_instances[m->m_material][mtxCnt].mv.v));
			mats.push_back(DirectX::XMFLOAT4X4A(gm->m_instances[m->m_material][mtxCnt].inv_mv.v));
        }
        gm->UpdateInstanceVBO(mats[0].m[0], instanceVBOelemCount);
	}
}

void DXB_Scene_30::Render( KCL::Camera2* camera, std::vector<Mesh*> &visible_meshes, KCL::Material *_override_material, KCL::PlanarMap* pm, KCL::uint32 lod, KCL::Light* light)
{
    //sort by depth here
    if(visible_meshes.size())
    {
        sort_vis_list(visible_meshes, camera);
    }
    if( &visible_meshes == &m_visible_meshes[0])
	{
        CollectInstances( visible_meshes);
    }

    DXB_Scene::Render(camera, visible_meshes, _override_material, pm, lod, light);
}

void DXB_Scene_30::Render()
{
#if defined(_DEBUG) && defined(PERF_MARKERS)
    CComPtr<ID3DUserDefinedAnnotation> pPerf;
    HRESULT hr = DX::getContext()->QueryInterface( __uuidof(pPerf), reinterpret_cast<void**>(&pPerf) );
    if ( FAILED( hr ) ) 
        return;

#define PBB(NAME) pPerf->BeginEvent( L#NAME)
#define PBE pPerf->EndEvent()

#else

#define PBB
#define PBE 

#endif

	ID3D11ShaderResourceView* nullShaderResource[] = { NULL, NULL, NULL, NULL, NULL };

	DX::getContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	m_constantBuffer->bind(0);
	m_constantBufferParticle->bind(1);
	m_constantBufferMBlur->bind(2);

	m_constantBufferFilter->bind(3);
	m_constantBufferEmitter->bind(4);

	DX::getContext()->PSSetSamplers(0, 1, m_linearSampler.GetAddressOf());
	DX::getContext()->PSSetSamplers(1, 1, m_linearSamplerClamp.GetAddressOf());

	m_num_draw_calls = 0;
	m_num_triangles = 0;
	m_num_vertices = 0;

    // Render shadow maps

    PBB("Shadow render Begins");
if(g_ShadowDepthRender)
{
    for( KCL::uint32 i=0; i<m_num_shadow_maps; i++)
    {
    	if (m_global_shadowmaps[i])
    	{
    		RenderShadow((ShadowMap*)m_global_shadowmaps[i]);
    	}
    }
}
    PBE;

	// Render scene to G-Buffer
	m_pp->BindGBuffer();
	m_pp->ClearGBuffer(0, 0, 0, 0);
	m_pp->ClearDepth();

    PBB("G-Buffer Solids");
if(g_GBufferSolids)
{
    DX::getContext()->OMSetDepthStencilState(m_depthLessEqual, 0);
	DXB_Scene_30::Render(m_active_camera, m_visible_meshes[0], 0, 0, 0, 0);
}
    PBE;

    ID3D11BlendState* defaultBlendState;
	FLOAT blendFactor[4];
	UINT sampleMask;
	DX::getContext()->OMGetBlendState(&defaultBlendState, blendFactor, &sampleMask);

    PBB("LensFlareQuery");
if(g_LensFlareQuery)
{
    DX::getContext()->OMSetDepthStencilState(m_depthLessNoWrite, 0);
    DX::getContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
    DX::getContext()->RSSetState(m_noCullRasterizerState);
    DX::getContext()->OMSetBlendState(m_blendStateDisableColorWrite, NULL, 0xffffffff);

    m_occlusion_query_shader->Bind();

    for( KCL::uint32 i=0; i<m_visible_lights.size(); i++)
	{
		DXB::Light *l = static_cast<DXB::Light*>(m_visible_lights[i]);

		if( !l->m_has_lensflare)
		{
			continue;
		}

		if( l->m_light_type == Light::AMBIENT || l->m_light_type == Light::DIRECTIONAL || l->m_diffuse_color == KCL::Vector3D() )
		{
			continue;
		}

		l->NextQueryObject();

        DX::getContext()->Begin(l->GetCurrentQueryObject());

        ConstantBuffer *buf = (ConstantBuffer*)m_constantBuffer->map();
	    KCL::Matrix4x4 mvp = m_active_camera->GetViewProjection();
	    buf->mvp = DX::Float4x4toXMFloat4x4ATransposed(mvp.v);

        KCL::Vector3D light_pos(l->m_world_pom.v[12], l->m_world_pom.v[13], l->m_world_pom.v[14]);
		buf->light_pos = DirectX::XMFLOAT4(light_pos.x, light_pos.y, light_pos.z, 1);

        buf->inv_screen_resolution = DirectX::XMFLOAT2(1.0f / m_viewport_width, 1.0f / m_viewport_height);

        m_constantBuffer->unmap();
        m_constantBuffer->bind(0);

        UINT null = 0;
        ID3D11Buffer* nullB[D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT] = {0};
        DX::getContext()->IASetVertexBuffers(0,D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT,nullB,&null,&null);
        DX::getContext()->IASetInputLayout(NULL);
        DX::getContext()->IASetIndexBuffer(nullB[0], DXGI_FORMAT_R16_UINT, 0);
        
        DX::getContext()->Draw(4, 0); //vs will generate vertex data
	
        DX::getContext()->End(l->GetCurrentQueryObject());
	}
}
    PBE;

//LIGHTS

    // Generate final buffer from G-Buffer
	m_pp->UnbindShaderResources();
	m_pp->BindForLightPass();
	m_pp->ClearFinalBuffer(0, 0, 0, 0);

    ID3D11RasterizerState* defaultRasterizerState;
	DX::getContext()->RSGetState(&defaultRasterizerState);
    ID3D11DepthStencilState* defaultDepthState;
	UINT stencilValue;
	DX::getContext()->OMGetDepthStencilState(&defaultDepthState, &stencilValue);

    PBB("Lights");
if(g_Lighting)
{

	DX::getContext()->PSSetShaderResources(0, 1, m_pp->GetGBufferShaderResource(0));
	DX::getContext()->PSSetSamplers(0, 1, m_linearSampler.GetAddressOf());
	DX::getContext()->IASetInputLayout(m_fullscreen_inputLayout.Get());
	DX::getContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	DX::getContext()->OMSetBlendState(m_blendStateAdd, blendFactor, sampleMask);


	DX::getContext()->RSSetState(m_lightCullRasterizerState);


	DX::getContext()->OMSetDepthStencilState(m_depthGreaterNoWrite, 0);

	CD3D11_VIEWPORT vpFullScreen;
	UINT vpNum = 1;
	DX::getContext()->RSGetViewports(&vpNum, &vpFullScreen);

	for (KCL::uint32 i=0; i<m_visible_lights.size(); i++)
	{
		Light* light = m_visible_lights[i];
		RenderLight(light);
	}

	DX::getContext()->OMSetBlendState(defaultBlendState, blendFactor, sampleMask);
}
    PBE;

//SKY
    PBB("Sky");
if(g_Sky)
{
    DX::getContext()->OMSetDepthStencilState(m_depthLessEqualNoWrite, 0);
    DXB_Scene::Render(m_active_camera, m_sky_mesh, 0, 0, 0, 0);
}
    PBE;

	m_pp->UnbindShaderResources();

	// Combine G-Buffer components
	VboPool::Instance()->BindBuffer(m_fullscreen_vbo);
	IndexBufferPool::Instance()->BindBuffer(m_fullscreen_ebo);

    //re
    PBB("RE");
if(g_RE)
{
    DX::getContext()->OMSetDepthStencilState(m_depthDisabled, 0);
    m_filters[10].SetRenderTarget();
	DX::getContext()->PSSetShaderResources(0, 3, m_pp->GetGBufferShaderResource());
	DX::getContext()->PSSetShaderResources(3, 1, m_pp->GetFinalBufferShaderResource());
	DX::getContext()->PSSetShaderResources(4, 1, m_pp->GetDepthStencilResource());
	DX::getContext()->PSSetSamplers(0, 1, m_linearSampler.GetAddressOf());
	DX::getContext()->IASetInputLayout( m_fullscreen_inputLayout.Get());
	DX::getContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);	
	m_filters[10].SetShader(m_reflection_emission_shader);
	m_filters[10].Render(NULL, m_constantBufferFilter);
}	
    PBE;

//SHADOW
    PBB("Shadow decal Begins");
if(g_ShadowDecal)
{
    if(m_num_shadow_maps && m_global_shadowmaps[0])
    {
        DX::getContext()->OMSetBlendState(m_blendStateDarken, blendFactor, sampleMask);
        
        DX::getContext()->PSSetShaderResources(0, 1, m_pp->GetDepthStencilResource());
        DX::getContext()->PSSetSamplers(0, 1, m_pointSamplerClamp.GetAddressOf());

	    ID3D11ShaderResourceView* textureView = ((ShadowMap*)m_global_shadowmaps[0])->GetD3DTextureId();
        DX::getContext()->PSSetShaderResources(1, 1, &textureView);
        DX::getContext()->PSSetSamplers(1, 1, m_shadowCmpSampler.GetAddressOf());

		Light* ll = LightFactory().Create("", 0, 0);
	    ll->m_light_type = Light::SHADOW_DECAL;
	    ll->m_radius = 1000.0f;
	    RenderLight( ll);
        delete ll;
    	
        DX::getContext()->OMSetBlendState(defaultBlendState, blendFactor, sampleMask);
        DX::getContext()->PSSetSamplers(0, 1, m_linearSampler.GetAddressOf());
        DX::getContext()->PSSetSamplers(1, 1, m_linearSamplerClamp.GetAddressOf());
    }
}
    PBE;
    

//DECALS
    PBB("Decals");
if(g_Decals)
{
    DX::getContext()->PSSetShaderResources(0, 5, nullShaderResource);

	// Use the original depth buffer in read-only mode
	m_filters[10].SetRenderTarget(m_pp->GetReadOnlyDepthStencilView());
	DXB_Scene::Render(m_active_camera, m_visible_meshes[2], 0, 0, 0, 0);
}
    PBE;

//PARTICLES
    PBB("Particles");
if(g_Particles)
{
	DX::getContext()->OMSetBlendState(m_blendStateAdd, blendFactor, sampleMask);
	DX::getContext()->OMSetDepthStencilState(m_depthLessNoWrite, 0);

	for( KCL::uint32 i=0; i<m_actors.size(); i++)
	{
		Actor *actor = m_actors[i];
		for( KCL::uint32 i=0; i<actor->m_emitters.size(); i++)
		{
			KCL::_emitter* emitter = (KCL::_emitter *)actor->m_emitters[i];
			RenderEmitter(emitter);
		}
	}
}
    PBE;

//LIGHTSHAFTS
    PBB("LightShafts");
if(g_LightShafts)
{
	// Unbind input streams
	ID3D11Buffer* nulls[] = { NULL, NULL }; 
	UINT offsetsAndStrides[] = { 0, 0 };
	DX::getContext()->IASetVertexBuffers(0, 2, nulls, offsetsAndStrides, offsetsAndStrides);

	DX::getContext()->IASetInputLayout( m_fullscreen_inputLayout.Get());

	LightShaft ls;
	for (uint32 j=0; j< m_lightshafts.size(); j++)
	{
		Light *l = m_lightshafts[j];		
		for( uint32 i=0; i<8; i++)
		{
			ls.m_corners[i].set( l->m_frustum_vertices[i].v);
		}

		bool isCamShaft = false; //(j == m_lightshafts.size() - 1);

		KCL::Matrix4x4 m2 = l->m_inv_light_projection * l->m_world_pom;
		ls.CreateCone( m2, m_active_camera->GetCullPlane( KCL::CULLPLANE_NEAR), isCamShaft);
	}

	if (ls.m_vertices.size() && ls.m_indices.size())
	{
		m_lightshaft_vbo->updateData((Vector3D*)ls.m_vertices[0].v, ls.m_vertices.size());
		m_lightshaft_ebo->updateData(&ls.m_indices[0], ls.m_indices.size()); 

		DX::getContext()->OMSetBlendState(m_blendStateAlpha, blendFactor, sampleMask);
		m_lightshaft_vbo->bind(0);
		m_lightshaft_ebo->bind(0);

		for (uint32 j=0; j < m_lightshafts.size(); j++)
		{
			Light* light = m_lightshafts[j];
			RenderLightShaft(light, ls.m_num_indices[j], ls.m_index_offsets[j]);
		}

		m_lightshafts.clear();
	}
}
    PBE;

//TRANSPARENTS
    PBB("Transparents");
if(g_Transparents)
{
	// Render transparent meshes
	m_filters[10].SetRenderTarget(m_pp->GetReadOnlyDepthStencilView());
	DX::getContext()->RSSetState(defaultRasterizerState);
	DX::getContext()->OMSetBlendState(defaultBlendState, blendFactor, sampleMask);
	DX::getContext()->OMSetDepthStencilState(defaultDepthState, stencilValue);
	DXB_Scene::Render(m_active_camera, m_visible_meshes[1], 0, 0, 0, 0);
}
    PBE;

//LENS FLARES
    PBB("LensFlares");
if(g_LensFlares)
{
    for( KCL::uint32 i=0; i<m_visible_lights.size(); i++)
	{
		DXB::Light *l = static_cast<DXB::Light*>(m_visible_lights[i]);

		if (l->m_has_lensflare)
		{
			BOOL visible = false;
				
			if( l->IsPreviousQueryObjectInitialized())
			{			
                while( S_OK != DX::getContext()->GetData(l->GetPreviousQueryObject(), &visible, sizeof(BOOL), 0) )
                {
                }
			}

			if(visible)
			{
				l->visible_meshes[0].push_back( m_lens_flare_mesh);

				DXB_Scene::Render(m_active_camera, l->visible_meshes[0], 0, 0, 0, l);

				l->visible_meshes[0].clear();
			}
		}
	}
}
    PBE;

//POST

    PBB("Post");
if(g_Post)
{
	// Bloom...
	VboPool::Instance()->BindBuffer(m_fullscreen_vbo);
	IndexBufferPool::Instance()->BindBuffer(m_fullscreen_ebo);
	DX::getContext()->PSSetSamplers(0, 1, m_linearSamplerClamp.GetAddressOf());
	DX::getContext()->IASetInputLayout( m_fullscreen_inputLayout.Get());
	DX::getContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);	

	DX::getContext()->GenerateMips(*m_filters[10].GetOutput());

	m_filters[1].SetRenderTarget();
	m_filters[1].SetShader(m_pp_shaders[2]);
	DX::getContext()->PSSetShaderResources(0, 1, m_filters[10].GetOutput());
	DX::getContext()->PSSetShaderResources(1, 1, m_filters[9].GetOutput());
	m_filters[1].Render(NULL, m_constantBufferFilter);

	for( KCL::uint32 i=2; i<=8; i++)
	{
		m_filters[i].SetRenderTarget();
		m_filters[i].SetShader(m_pp_shaders[1]);
		m_filters[i].Render(*m_filters[i - 1].GetOutput(), m_constantBufferFilter);
	}

	// Depth-of-field...
	m_filters[0].SetRenderTarget();
	m_filters[0].SetFocusDistance(m_camera_focus_distance);
	m_filters[0].SetCamera(m_active_camera);
	m_filters[0].SetShader(m_pp_shaders[0]);

    //pp
	DX::getContext()->PSSetShaderResources(0, 1, m_filters[10].GetOutput());
	DX::getContext()->PSSetShaderResources(1, 1, m_filters[8].GetOutput());
	DX::getContext()->PSSetShaderResources(2, 1, m_pp->GetDepthStencilResource());
	m_filters[0].Render(NULL, m_constantBufferFilter);

	// Release the final and depth buffer
	DX::getContext()->PSSetShaderResources(0, 5, nullShaderResource);
}

	ID3D11RenderTargetView* nullViews[D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT] = {nullptr};
    DX::getContext()->OMSetRenderTargets(ARRAYSIZE(nullViews), nullViews, nullptr);

    PBE;

	if (defaultBlendState!=NULL) defaultBlendState->Release();
	if (defaultDepthState!=NULL) defaultDepthState->Release();
	if (defaultRasterizerState!=NULL) defaultRasterizerState->Release();
}

void DXB_Scene_30::RenderLight(Light* light)
{
	ConstantBuffer *buf = (ConstantBuffer*)m_constantBuffer->map();

	Matrix4x4 model;
	int lightShaderIndex = 3;
	UINT indexCount = 0;

	buf->view_pos = DirectX::XMFLOAT4(m_active_camera->GetEye().v);

	buf->screen_resolution = DirectX::XMFLOAT2(m_viewport_width, m_viewport_height);
	buf->inv_screen_resolution = DirectX::XMFLOAT2(1.0f / m_viewport_width, 1.0f / m_viewport_height);

	buf->shadow_matrix0 = DX::Float4x4toXMFloat4x4ATransposed(m_global_shadowmaps[0]->m_matrix.v);
	buf->depth_parameters =  DirectX::XMFLOAT4(m_active_camera->m_depth_linearize_factors.v);
	buf->view_dir = DirectX::XMFLOAT4(
		-m_active_camera->GetView().v[2],
		-m_active_camera->GetView().v[6],
		-m_active_camera->GetView().v[10],
		1.0f);

	float i = 1.0f;
	if (light->m_intensity_track)
	{
		Vector4D v;
		light->t = m_animation_time / 1000.0f;
		_key_node::Get( v, light->m_intensity_track, light->t, light->tb, true);
		//i = v.x / l->m_intensity;
		i = 0.01 * v.x;// / light->m_intensity;
	}

	buf->light_color = DirectX::XMFLOAT4(
		light->m_diffuse_color.x * i, 
		light->m_diffuse_color.y * i, 
		light->m_diffuse_color.z * i, 1);

	switch (light->m_light_type)
	{
	case Light::OMNI:
		{
			lightShaderIndex = 1;

			Vector3D light_pos(light->m_world_pom.v[12], light->m_world_pom.v[13], light->m_world_pom.v[14]);
			buf->light_pos = DirectX::XMFLOAT4(light_pos.x, light_pos.y, light_pos.z, 1);
			buf->attenuation_parameter = -1.0f / (light->m_radius * light->m_radius);
			buf->spot_cos = DirectX::XMFLOAT2(-1, 1) ;

			model.zero();
			model.v11 =
			model.v22 =
			model.v33 = light->m_radius * 1.25f;
			model.v41 = light_pos.x;
			model.v42 = light_pos.y;
			model.v43 = light_pos.z;
			model.v44 = 1;
			m_sphere_vbo->bind(0);
			m_sphere_ebo->bind(0);
			indexCount = m_sphere_ebo->getIndexCount();
			break;
		}

	case Light::SPOT:
		{
			lightShaderIndex = 2;

			Vector3D light_pos(light->m_world_pom.v[12], light->m_world_pom.v[13], light->m_world_pom.v[14]);
			buf->light_pos = DirectX::XMFLOAT4(light_pos.x, light_pos.y, light_pos.z, 1);
			buf->attenuation_parameter = -1.0f / (light->m_radius * light->m_radius);

			Vector3D light_dir( -light->m_world_pom.v[8], -light->m_world_pom.v[9], -light->m_world_pom.v[10]);
			light_dir.normalize();
			buf->light_x = DirectX::XMFLOAT4(light_dir.x, light_dir.y, light_dir.z, light->m_world_pom[14]);

			Vector2D spot_sin;
			float halfSpotAngle = Math::Rad(0.5f * light->m_spotAngle);
			spot_sin.x = cosf(halfSpotAngle);
			spot_sin.y = 1.0f / (1.0f - spot_sin.x);
			buf->spot_cos = DirectX::XMFLOAT2(spot_sin.x, spot_sin.y);
	
			model.zero();
			model.v33 = light->m_radius;
			model.v11 =
			model.v22 = model.v33 * tanf(halfSpotAngle);
			model.v43 = -model.v33;	// Translate so the top is at the origo
			model.v44 = 1;
			model *= light->m_world_pom;
			m_cone_ebo->bind(0);
			m_cone_vbo->bind(0);
			indexCount = m_cone_ebo->getIndexCount();
			break;
		}
    case Light::SHADOW_DECAL:
        {
            lightShaderIndex = 15;

            Vector3D light_pos(light->m_world_pom.v[12], light->m_world_pom.v[13], light->m_world_pom.v[14]);
			buf->light_pos = DirectX::XMFLOAT4(light_pos.x, light_pos.y, light_pos.z, 1);
			buf->attenuation_parameter = -1.0f / (light->m_radius * light->m_radius);
			buf->spot_cos = DirectX::XMFLOAT2(-1, 1) ;

			model.zero();
			model.v11 =
			model.v22 =
			model.v33 = light->m_radius * 1.25f;
			model.v41 = light_pos.x;
			model.v42 = light_pos.y;
			model.v43 = light_pos.z;
			model.v44 = 1;
			m_sphere_vbo->bind(0);
			m_sphere_ebo->bind(0);
			indexCount = m_sphere_ebo->getIndexCount();
			break;
        }

	case Light::SSAO:
	case Light::DIRECTIONAL:
	default:
		{
			m_constantBuffer->unmap();
			return;

			////m_active_camera->GetBillboardTranform(model);
			////VboPool::Instance()->BindBuffer(m_fullscreen_vbo);
			////IndexBufferPool::Instance()->BindBuffer(m_fullscreen_ebo);
			////indexCount = 6;
			////break;
		}
	}

	buf->model = DX::Float4x4toXMFloat4x4ATransposed(model.v);

	Matrix4x4 mv = model * m_active_camera->GetView();
	buf->mv = DX::Float4x4toXMFloat4x4ATransposed(mv.v);

	Matrix4x4 mvp = model * m_active_camera->GetViewProjection();
	buf->mvp = DX::Float4x4toXMFloat4x4ATransposed(mvp.v);

	Matrix4x4 inv_mv;
	Matrix4x4::Invert4x4(mv, inv_mv);
	buf->inv_mv = DX::Float4x4toXMFloat4x4ATransposed(inv_mv.v);

	m_constantBuffer->unmap();

	m_lighting_shaders[lightShaderIndex]->Bind();
	DX::getContext()->DrawIndexed(indexCount, 0, 0);
}

void DXB_Scene_30::RenderEmitter(KCL::_emitter* emitter)
{
	KCL::uint32 texture_num = 0;
	DXB::Material *material = NULL;
	if( emitter->m_name.find( "smoke") != std::string::npos)
	{
		material = dynamic_cast<DXB::Material*>(m_instanced_smoke_material);
	}
	if( emitter->m_name.find( "fire") != std::string::npos)
	{
		material = dynamic_cast<DXB::Material*>(m_instanced_fire_material);
	}
	if( emitter->m_name.find( "spark") != std::string::npos)
	{
		material = dynamic_cast<DXB::Material*>(m_instanced_spark_material);
	}
	if( emitter->m_name.find( "soot") != std::string::npos)
	{
		material = dynamic_cast<DXB::Material*>(m_instanced_spark_material);
	}
	else
	{
		material = dynamic_cast<DXB::Material*>(m_instanced_fire_material);
	}

	material->preInit( texture_num, 0, 0);
	Shader* shader = material->m_shaders[0][0];
	DXB::DXBEmitter* dxbemitter = static_cast<DXB::DXBEmitter*>(emitter);
	dxbemitter->setRenderShader(shader);
	
	if (shader->m_ps.m_name.find( "fire") != std::string::npos)
	{
		DX::getContext()->PSSetShaderResources(0, 1, m_fireTexture->getShaderResourceView());
	}

	shader->Bind();

	ConstantBufferEmitter* buf = (ConstantBufferEmitter*)m_constantBufferEmitter->map();
	dxbemitter->setConsts(buf);

	Matrix4x4 mv = m_active_camera->GetView();
	buf->mv = DX::Float4x4toXMFloat4x4ATransposed(mv.v);

	Matrix4x4 mvp = m_active_camera->GetViewProjection();
	buf->mvp = DX::Float4x4toXMFloat4x4ATransposed(mvp.v);

	m_constantBufferEmitter->unmap();

	dxbemitter->bindBuffers();

	DX::getContext()->DrawInstanced(6, emitter->VisibleParticleCount(), 0, 0);
		
	material->postInit();
}

void DXB_Scene_30::RenderLightShaft(Light* light, UINT indexCount, UINT indexOffset)
{
	if (indexCount <= 0)
	{
		return;
	}

	float normalized_time = (float) (m_animation_time % 100000) / 100000.0f;

	ConstantBuffer *buf = (ConstantBuffer*)m_constantBuffer->map();

	Matrix4x4 mv = m_active_camera->GetView();
	buf->mv = DX::Float4x4toXMFloat4x4ATransposed(mv.v);

	Matrix4x4 mvp = m_active_camera->GetViewProjection();
	buf->mvp = DX::Float4x4toXMFloat4x4ATransposed(mvp.v);

	buf->view_pos = DirectX::XMFLOAT4(m_active_camera->GetEye().v);
	buf->background_color = DirectX::XMFLOAT4(0, 0, 0, 0);
	buf->time = normalized_time;

	buf->screen_resolution = DirectX::XMFLOAT2(m_viewport_width, m_viewport_height);
	buf->inv_screen_resolution = DirectX::XMFLOAT2(1.0f / m_viewport_width, 1.0f / m_viewport_height);

	static const Matrix4x4 shadowM (
		0.5f,  0,    0,    0,
		0,     0.5f, 0,    0,
		0,     0,    0.5f, 0,
		0.5f,  0.5f, 0.5f, 1);

	KCL::Matrix4x4 m0;
	KCL::Matrix4x4::Invert4x4(light->m_world_pom, m0);
	KCL::Matrix4x4 m = m0 * light->m_light_projection * shadowM;
	buf->shadow_matrix0 = DX::Float4x4toXMFloat4x4ATransposed(m.v);
	buf->depth_parameters =  DirectX::XMFLOAT4(m_active_camera->m_depth_linearize_factors.v);

	buf->view_dir = DirectX::XMFLOAT4(
		-m_active_camera->GetView().v[2],
		-m_active_camera->GetView().v[6],
		-m_active_camera->GetView().v[10],
		1.0f);

	buf->attenuation_parameter = -1.0f / (light->m_radius * light->m_radius);

	float i = 1.0f;
	if (light->m_intensity_track)
	{
		Vector4D v;
		light->t = m_animation_time / 1000.0f;

		_key_node::Get(v, light->m_intensity_track, light->t, light->tb, true);
		i = v.x / light->m_intensity;
	}

	buf->light_color = DirectX::XMFLOAT4(
		light->m_diffuse_color.x * i, 
		light->m_diffuse_color.y * i, 
		light->m_diffuse_color.z * i, 1);

	Vector3D light_pos(light->m_world_pom.v[12], light->m_world_pom.v[13], light->m_world_pom.v[14]);
	buf->light_pos = DirectX::XMFLOAT4(light_pos.x, light_pos.y, light_pos.z, 1);
	buf->attenuation_parameter = -1.0f / (light->m_radius * light->m_radius);

	Vector3D light_dir( -light->m_world_pom.v[8], -light->m_world_pom.v[9], -light->m_world_pom.v[10]);
	light_dir.normalize();
	buf->light_x = DirectX::XMFLOAT4(light_dir.x, light_dir.y, light_dir.z, light->m_world_pom[14]);

	Vector2D spot_sin;
	float halfSpotAngle = Math::Rad(0.5f * light->m_spotAngle);
	spot_sin.x = cosf(halfSpotAngle);
	spot_sin.y = 1.0f / (1.0f - spot_sin.x);
	buf->spot_cos = DirectX::XMFLOAT2(spot_sin.x, spot_sin.y);

	bool isCamShaft = false;
	if (isCamShaft)
	{
		ID3D11ShaderResourceView* textures[] =
		{
			*m_pp->GetDepthStencilResource(),
			*m_volumeLightTexture->getShaderResourceView(),
			*m_fogTexture->getShaderResourceView()
		};

		DX::getContext()->PSSetShaderResources(0, 3, textures);
		m_camera_fog_shader->Bind();
	}
	else
	{
		ID3D11ShaderResourceView* textures[] =
		{
			*m_lightTexture->getShaderResourceView(),
			*m_pp->GetDepthStencilResource(),
		};

		DX::getContext()->PSSetShaderResources(0, 2, textures);
		m_fog_shader->Bind();
	}

	m_constantBuffer->unmap();

	DX::getContext()->DrawIndexed(indexCount, indexOffset, 0);
}

void DXB_Scene_30::Release_GLResources()
{
	DXB_Scene::Release_GLResources();
}
