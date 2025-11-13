/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef MTL_SCENE_40_H
#define MTL_SCENE_40_H

#include "kcl_base.h"
#include "krl_scene.h"
#include "mtl_scene_base.h"
#include "mtl_mesh4.h"
#include "mtl_scene_40_support.h"
#include "mtl_tessellator.h"

#include "glb_kcl_adapter.h"

#include "mtl_dynamic_data_buffer.h"
#include "graphics/metalgraphicscontext.h"

#define ENABLE_CUBEMAP_FP_RENDER_TARGETS        0
#define ENABLE_LIGHTCOMBINE_FP_RENDER_TARGET    0
#define RENDER_MATERIAL_ID                      1

#include "mtl_shader_constant_layouts_40.h"


class GUIInterface;
class GlobalTestEnvironment;


namespace MetalRender
{
	class Material4;
	class Lensflare;
	class FragmentBlur;
	class StrideBlur;
//	class ParticleSystem4;
	class OcclusionCull;
	class CascadedShadowMap;
	class QuadBuffer;

	class ComputeMotionBlur;
	class ComputeEmitter;
}

class ComputeHDR40;

namespace GFXB4
{
	class GBuffer;
};


class MTL_Scene_40 : public MTL_Scene_Base
{
public:
	static const KCL::uint32 MAX_INSTANCES = 128;

	static MTLPixelFormat GetShadowMapFormat()
	{
#if TARGET_OS_EMBEDDED
		return MTLPixelFormatDepth32Float;
#else
		return MTLPixelFormatDepth16Unorm;
#endif
	}

	MTL_Scene_40(const GlobalTestEnvironment* const gte);
	virtual ~MTL_Scene_40();
	KCL::KCL_Status Process_GL(GLB::FBO_COLORMODE color_mode, GLB::FBO_DEPTHMODE depth_mode, int samples);
	virtual void Animate();
	virtual void Render();

	virtual KCL::KCL_Status reloadShaders();
	virtual void LoadHDR();
	virtual void ReloadHDR();
//	virtual void ReloadExportedMeshes(const std::vector<std::string> & filenames);
//	virtual void ReloadExportedMaterials(const std::vector<std::string> & filenames);

    void BeginEncodeRender();
    void EncodeRender(KCL::Camera2* camera,
                    std::vector<Mesh*> &visible_meshes,
                    KCL::Material *_override_material,
                    KCL::PlanarMap * pm,
                    KCL::uint32 lod,
                    KCL::Light* light,
                    PassType::Enum pass_type,
                    bool occlusion_cull,
                    KCL::uint32 slice_id = 0);
    void EndEncodeRender(__strong id<MTLCommandBuffer>& command_buffer,
                         std::function<id<MTLRenderCommandEncoder>(id<MTLCommandBuffer>)> createEncoder,
                         bool occlusion_cull, bool tessellation);

	size_t GetFrameConstsOffset() { return m_frame_consts_offset; }

	id<MTLSamplerState> GetTopdownShadowSampler() const;
	KCL::Texture * GetTopdownShadowMap() const;
	id<MTLTexture> GetVelocityBuffer() const;

	id<MTLCommandQueue> GetCommandQueue() const { return m_command_queue; }
	
	virtual void Resize(KCL::uint32 w, KCL::uint32 h);

protected:
	virtual KCL::Mesh3Factory &Mesh3Factory()
	{
		return m_mesh3_factory;
	}

private:
#if !TARGET_OS_EMBEDDED
	static const KCL::uint32 NUM_STATIC_ENVMAPS = 1;
#else
	static const KCL::uint32 NUM_STATIC_ENVMAPS = 9;
#endif

	const GlobalTestEnvironment *m_gte;
	WarmUpHelper *m_warmup_helper;
//	WireframeMode m_wireframe_mode;

	GFXB4::GBuffer * m_gbuffer;

	KCL::int32 m_last_animation_time;

	bool m_occlusion_cull_indirect_enabled;
	bool m_occlusion_cull_tessellated_enabled;
	
	bool m_cull_non_instanced_meshes_enabled;
	bool m_render_occluders_enabled;
	bool m_compute_bright_pass_enabled;
	bool m_is_compute_warmup;

	float m_tessellation_viewport_scale;

	FboEnvMap * m_fboEnvMap;

	ParaboloidCulling * m_paraboloid_culling;
	ParaboloidEnvMap * m_dynamic_cubemaps[2];

	bool cubemaps_inited;
	KCL::Actor* m_carActor_hero;
	KCL::Actor* m_carActor_evil;
	KCL::Material* m_sky_mat_paraboloid;

	KCL::Camera2 m_car_ao_cam;	

	ComputeHDR40 * m_compute_hdr;
	MetalRender::FragmentBlur *m_dof_blur;
	MetalRender::Lensflare *m_lensflare;
	////////////////////////////////////
	//          GUI
	////////////////////////////////////
	friend class GFXGui;
	std::unique_ptr<GUIInterface> m_gui;
	virtual void UpdateGUI(KCL::uint32 cursorX, KCL::uint32 cursorY, bool mouseLPressed, bool mouseLClicked, bool mouseRPressed, bool mouseRClicked, const bool *downKeys);

	////////////////////////////////////
	//          END OF GUI
	////////////////////////////////////

	size_t					m_frame_consts_offset;

	FrameConstants			m_frame_consts;
	VertexConstants			m_vert_consts;
	TessellationConstants	m_tess_consts;
	FragmentConstants		m_frag_consts;

	MetalRender::OcclusionCull *m_occlusion_cull;
	MetalRender::StrideBlur *m_ao_shadow_blur;
	MetalRender::CascadedShadowMap *m_cascaded_shadow_map;
	MetalRender::ComputeMotionBlur *m_compute_motion_blur;

	MetalRender::Material4 *m_sky_lightcombine_pass;
	MetalRender::Material4 *m_shadow_material;
	MetalRender::Material4 *m_transparent_shadow_material;
	MetalRender::Material4 *m_transparent_billboard_shadow_material;
	MetalRender::Material4 *m_tessellated_shadow_material;

	KCL::Texture * m_static_cubemaps[NUM_STATIC_ENVMAPS];

	float m_sunColorStrength;
	float m_fogColorStrength;
	KCL::Vector3D m_fogColor;

	KCL::KCL_Status InitEmitters();
	void InitPostProcessPipeline();
	void CreateRenderPassDescriptors();

	void CollectRenderMaterials( std::vector<KCL::Mesh*> &meshes, KCL::Material *override_material, PassType::Enum pass_type, KCL::uint32 slice_id = 0);
	void CollectRenderMaterials( std::vector<KCL::MeshInstanceOwner2*> &visible_mios, KCL::Material *override_material, PassType::Enum pass_type, KCL::uint32 slice_id = 0);

	void SimulateParticles(id<MTLCommandBuffer> command_buffer);
	void RenderParticles(id<MTLCommandBuffer> command_buffer);

	void ExecuteSSAOPass(id<MTLCommandBuffer> command_buffer);
	void ExecuteSSDSPass(id<MTLCommandBuffer> command_buffer);
	void ExecuteLightCombineSkyPass(id<MTLCommandBuffer> command_buffer);
	void ExecuteTransparentPass(id<MTLCommandBuffer> command_buffer);
	void ExecuteMotionBlurPass(id<MTLCommandBuffer> command_buffer, bool enable_motion_blur);
	void ExecuteDOFPass(id<MTLCommandBuffer> command_buffer, bool enable_motion_blur);
	void ExecuteToneMapBloomPass(id<MTLCommandBuffer> command_buffer);
    void RenderCascadedShadow(__strong id<MTLCommandBuffer>& command_buffer);

	void UpdateEnvmap(id<MTLCommandBuffer> command_buffer, const GLB::Vector3D &pos, KCL::uint32 idx);

	KCL::int32 GetCameraClipId(KCL::int32 time);
    
    
    enum CollectionFlags
    {
        CollectOpaque = 1,
        CollectTransparent = 2
    };

	static void CollectInstances(unsigned flags, std::vector<GFXB4::Mesh3::InstanceData> &instance_data, std::vector<KCL::Mesh*> &instances, const std::vector< std::vector<KCL::Mesh*> > &visible_instances);
	static void CollectInstances(unsigned flags, std::vector<GFXB4::Mesh3::InstanceData> &instance_data, std::vector<KCL::Mesh*> &instances, const std::vector<KCL::MeshInstanceOwner2*> &visible_mios);

	void WarmUpShaders();

	const static int NUM_OF_EMITTERS = 4;
	MetalRender::ComputeEmitter *m_emitters[NUM_OF_EMITTERS];

	KCL::_key_node *m_emitter_rate_anim[2];
	KCL::_key_node *m_camera_cut_track;
	KCL::int32 m_prev_camera_clip;
	bool m_camera_clip_changed;
	bool m_prev_mvp_valid;
	KCL::Texture *m_topdown_shadow;
	id<MTLSamplerState> m_topdown_shadow_sampler;
	id<MTLSamplerState> m_cubemap_sampler;

	GFXB4::Mesh3Factory m_mesh3_factory;

	MetalGraphicsContext * m_context;

	MetalRender::DynamicDataBufferPool * m_dynamic_buffer_pool;
	MetalRender::DynamicDataBuffer * m_dynamic_data_buffer;

	MetalRender::QuadBuffer * m_quad_buffer;

	id <MTLDevice> m_device;
    id <MTLCommandQueue> m_command_queue;

    MetalRender::Tessellator* m_tessellator;

	id <MTLTexture> m_hdr_output;
	id <MTLTexture> m_dof_output;
	id <MTLTexture> m_mb_output;
	id <MTLTexture> m_ssao_output;
	id <MTLTexture> m_ssds_output;
	id <MTLTexture> m_lightcombine_output;
	id <MTLTexture> m_tonemap_output;
	id <MTLTexture> m_motion_blur_output;

	MTLRenderPassDescriptor * m_ssao_pass_desc;
	MTLRenderPassDescriptor * m_ssds_pass_desc;
	MTLRenderPassDescriptor * m_lightcombine_pass_desc;
	MTLRenderPassDescriptor * m_motion_blur_pass_desc;
	MTLRenderPassDescriptor * m_tonemap_pass_desc;
	MTLRenderPassDescriptor * m_particles_pass_desc;
	MTLRenderPassDescriptor * m_dof_pass_desc;

	MetalRender::Pipeline * m_hdr_pipeline;
	MetalRender::Pipeline * m_ssao_pipeline;
	MetalRender::Pipeline * m_dof_pipeline;
	MetalRender::Pipeline * m_ssds_pipeline;
	MetalRender::Pipeline * m_lightcombine_pipeline;
    MetalRender::Pipeline * m_billboard_point_pipline;
    
    id<MTLDepthStencilState> m_billboard_point_depth_stencil;
	
	bool m_portrait_mode;
};


#endif
