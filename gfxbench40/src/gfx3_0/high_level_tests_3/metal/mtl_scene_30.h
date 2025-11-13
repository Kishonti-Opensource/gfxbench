/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef MTL_SCENE_30_H
#define MTL_SCENE_30_H

#include "mtl_framebuffer_30.h"
#include "mtl_lensflare_30.h"

#include "krl_scene.h"
#include "fbo.h"
#include "mtl_lightbuffer.h"
#include "mtl_globals.h"
#include "mtl_factories.h"
#include "mtl_particlesystem.h"
#include "mtl_texture.h"
#include "mtl_scene_base.h"
#include "mtl_dynamic_data_buffer.h"
#include "mtl_light.h"

#import "graphics/metalgraphicscontext.h"

namespace MetalRender
{
	class Pipeline;
    class Framebuffer;
    class ShadowMap;
    class Texture;
    class DynamicDataBuffer;
    class QuadBuffer;
}

class CubeEnvMap_Metal ;

class MTL_Scene_30 : public MTL_Scene_Base
{
public:
	MTL_Scene_30(const GlobalTestEnvironment* const gte);
	virtual ~MTL_Scene_30();


    KCL::KCL_Status Process_GL( GLB::FBO_COLORMODE color_mode, GLB::FBO_DEPTHMODE depth_mode, int samples);
    virtual KCL::KCL_Status reloadShaders();

	void Render();

    typedef enum
    {
        LIGHT_POINT,
        LIGHT_SPOT,
        LIGHT_SHADOW_DECAL,
        LIGHT_POINT_NOBLEND,
        LIGHT_SPOT_NOBLEND,
        NUM_LIGHT_TYPES
    } LightPipelineType;
    
protected:

	MetalRender::Pipeline* m_pipeline;
	MetalRender::Pipeline* m_lighting_pipelines[NUM_LIGHT_TYPES];
	MetalRender::Pipeline* m_blurFilter_pipeline;
	MetalRender::Pipeline* m_subFilter_pipeline;
	MetalRender::Pipeline* m_depthOfField_pipeline;
	MetalRender::Pipeline* m_occlusion_query_pipeline;
	MetalRender::Pipeline* m_fog_pipeline;
	MetalRender::Pipeline* m_particleAdvect_pipeline;
	MetalRender::Pipeline* m_reflection_emission_pipeline;
    
    void RenderWithCamera(KCL::Camera2* camera,
                          id <MTLRenderCommandEncoder> renderEncoder,
                          std::vector<KCL::Mesh*> &visible_meshes,
                          KCL::Material *kcl_override_material,
                          KCL::PlanarMap* pm,
                          KCL::uint32 lod,
                          KCL::Light* light,
                          int pass_type);

    void RenderShadow(MetalRender::ShadowMap* shadowMap, id<MTLCommandBuffer> commandBuffer);
    void RenderLight(KCL::Light *l, id <MTLRenderCommandEncoder> renderEncoder, bool blend);
    void RenderLightShafts(id<MTLRenderCommandEncoder> renderEncoder);
    
    virtual void DoLightingPass(id <MTLRenderCommandEncoder> render_encoder) = 0;

    virtual void RunPostProcess(id <MTLCommandBuffer> commandBuffer) = 0;
    virtual void RenderPostProcess(id <MTLCommandBuffer> commandBuffer) = 0;
    
    virtual void RunLightningEffectPass1(id <MTLCommandBuffer> commandBuffer) = 0;
    virtual void RunLightningEffectPass2(id <MTLCommandBuffer> commandBuffer) = 0;
    virtual void RenderLightningEffect(id <MTLRenderCommandEncoder> render_encoder) = 0;
    
    void MoveParticles(id <MTLCommandBuffer> commandBuffer);
	void RenderParticles(id <MTLRenderCommandEncoder> encoder);
	
	void IssueQueries(id <MTLCommandBuffer> cmdBuf);
	void RenderLensFlares(id <MTLRenderCommandEncoder> encoder);
    
	template<bool SetVertex, bool SetFragment>
    void WriteAndSetFrameConsts(KCL::Camera2* camera,
                                id <MTLRenderCommandEncoder> renderEncoder);

	void Release_MetalResources();
	
    MetalRender::Framebuffer* m_framebuffer;

    enum { NUM_SHADOW_MAPS = 2 };
    MetalRender::ShadowMap* m_shadowMaps[NUM_SHADOW_MAPS];

    MetalRender::DynamicDataBufferPool* m_dynamicDataBufferPool ;
    
    MetalRender::DynamicDataBuffer* m_dynamicDataBuffer;
	MetalRender::DynamicDataBuffer* m_lensFlareDataBuffer;
    
    MetalRender::Texture* m_envMapTextures[2];

    MetalRender::Texture* m_light_noise;

	
	//TODO initialize this stuff!
	id <MTLTexture> m_TransformFeedbackDummyTexture;
	MTLRenderPassDescriptor* m_TransformFeedbackFramebuffer;
	KCL::Texture* m_noiseTexture;
	
	KCL::Texture* m_fireTexture;
	
	id <MTLSamplerState> m_pointSampler;
	id <MTLSamplerState> m_linearSampler;
	
    MetalRender::LightBuffer* m_lbos[MetalRender::LightBuffer::NUM_SHAPES];

    id <MTLCommandQueue> m_commandQueue;

    id <MTLDepthStencilState> m_blitDepthState;
    id <MTLDepthStencilState> m_lightDepthState;
    id <MTLDepthStencilState> m_lightShaftDepthState;

    MTLVertexDescriptor* m_quadBufferVertexLayout;
    MetalRender::QuadBuffer* m_quadBuffer;
    MetalRender::QuadBuffer* m_finalBlitQuadBuffer;
	
	//occlusion queries
    MetalRender::OcclusionQueryBuffer m_occlusionQueries[MetalRender::Light::QUERY_COUNT];
    uint32_t m_currentQueryBufferIndex;
    uint32_t m_lastQueryBufferIndex;
    void CloseQueryBuffers(id <MTLCommandBuffer> commandBuffer);
     
	id <MTLDepthStencilState> m_particleDepthStencilState;
	id <MTLDepthStencilState> m_DepthTestOffDepthWritesOff;

    uint64_t m_totalDuration;
    
    MetalGraphicsContext* m_Context ;
    id <MTLDevice> m_Device ;
    
    MetalRender::ShaderType m_default_shader_type ;
};

#endif // MTL_SCENE_30_H
