/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef DXB_SCENE_H
#define DXB_SCENE_H

#include "dxb_emitter.h"
#include "dxb_light.h"
#include "krl_scene.h"
#include "d3d11/dxb_image.h"
#include "d3d11/dxb_texture.h"
#include "d3d11/dxb_buffer.h"
#include "dxb_mesh.h"
#include "d3d11/fbo3.h"

class CubeEnvMap;
class ShadowMap;

class DXB_Scene : public KRL_Scene
{
protected:
	Microsoft::WRL::ComPtr<ID3D11InputLayout> m_fullscreen_inputLayout;

	KCL::ConstantBuffer* m_constantBuffer;
	KCL::ConstantBuffer* m_constantBufferParticle;
	KCL::ConstantBuffer* m_constantBufferMBlur;

	Microsoft::WRL::ComPtr<ID3D11SamplerState> m_linearSampler;
	Microsoft::WRL::ComPtr<ID3D11SamplerState> m_linearSamplerClamp;

    Microsoft::WRL::ComPtr<ID3D11SamplerState> m_pointSamplerClamp;
    Microsoft::WRL::ComPtr<ID3D11SamplerState> m_shadowCmpSampler;

	KCL::uint32 m_fullscreen_vbo;
	KCL::uint32 m_fullscreen_ebo;
	KCL::uint32 m_particles_vbo;         //for per particle data
	KCL::uint32 m_fullscreen_quad_vbo;
	GLB::FBO *m_main_fbo;
	GLB::FBO *m_mblur_fbo;

	void CreateMirror( const char* name);

public:
	DXB_Scene();
	~DXB_Scene();

	virtual KCL::KCL_Status Process_GL( GLB::FBO_COLORMODE color_mode, GLB::FBO_DEPTHMODE depth_mode, int samples);
	virtual void Render() = 0;
	KCL::KCL_Status Init(KCL::uint32 w, KCL::uint32 h, float cullNear = 0.01f, float cullFar = 10000.0f);

	void InitFactories();

	virtual KCL::TextureFactory &TextureFactory()
	{
		return textureFactory;
	}

	virtual KCL::Mesh3Factory &Mesh3Factory()
	{
		return meshFactory;
	}

	virtual KCL::AnimatedEmitterFactory &EmitterFactory()
	{
		return m_emitter_factory;
	}

	virtual KCL::LightFactory &LightFactory()
	{
		return m_light_factory;
	}

	virtual void DeleteVBOPool() {};
    virtual void DeleteShaders() {};
    virtual void CreateVBOPool() {};
    virtual KCL::KCL_Status reloadShaders();

protected:
	DXB::DXBMesh3Factory meshFactory;
	DXB::DXBTextureFactory textureFactory;
	DXB::DXBAnimatedEmitterFactory m_emitter_factory;
	DXB::DXBLightFactory m_light_factory;
	KCL::AnimatedEmitterFactory m_animated_factory;
	KCL::FactoryBase* m_materialFactory;

	virtual void Release_GLResources();
	void OcclusionCullLights();
	void RenderShadow( ShadowMap* sm);
	void RenderPlanar( KCL::PlanarMap* pm);
	void RenderPrepass( KCL::Camera2* camera, std::vector<KCL::Mesh*> &visible_meshes, KCL::Material *_override_material, KCL::PlanarMap* pm, KCL::uint32 lod, KCL::Light* light);
	virtual void Render( KCL::Camera2* camera, std::vector<KCL::Mesh*> &visible_meshes, KCL::Material *_override_material, KCL::PlanarMap* pm, KCL::uint32 lod, KCL::Light* light);
	void Render( KCL::Camera2* camera, std::vector<KCL::Mesh*> &visible_meshes, KCL::Material *_override_material, KCL::PlanarMap* pm, KCL::uint32 lod, KCL::Light* light, int pass_type);
	void RenderVisibleAABBs( bool actors, bool rooms, bool room_meshes);
	void RenderPortals();
	void renderSkeleton( KCL::Node* node);

	KCL::uint32 LoadTexture3D( const std::string &name, bool clamp);
	KCL::uint32 Create3DTexture( std::vector<DXB::Image2D*> &images, bool clamp);

	CubeEnvMap *CreateEnvMap( const KCL::Vector3D &pos, KCL::uint32 idx);
	void GeneratePVS();
};

#endif