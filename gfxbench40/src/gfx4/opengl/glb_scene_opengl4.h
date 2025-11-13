/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef GLB_SCENE_OPENGL4_H
#define GLB_SCENE_OPENGL4_H

#include <memory> //shared_ptr

#include "kcl_base.h"
#include "krl_scene.h"

#include "glb_scene_.h"

#include "opengl/glb_scene_opengl4_support.h"
#include "opengl/glb_discard_functions.h"
#include "opengl/glb_instance_manager.h"
#include "opengl/glb_filter.h"
#include "opengl/shader.h"
#include "opengl/cubemap.h"
#include "opengl/shadowmap.h"
#include "gfxb4_mesh.h"

#include "opengl/ubo_cpp_defines.h"
#include "opengl/ubo_manager.h"
#include "ubo_frame.h"

#define ENABLE_CUBEMAP_FP_RENDER_TARGETS        0
#define ENABLE_LIGHTCOMBINE_FP_RENDER_TARGET    0
#define RENDER_MATERIAL_ID                      1

class GUIInterface;
class GlobalTestEnvironment;

namespace GLB
{
	class GLBTexture;
	class GLBShader2;

	class Material4;
	class Lensflare;
	class FragmentBlur;
	class StrideBlur;
    class ParticleSystem4;
	class OcclusionCull;
	class CascadedShadowMap;

	class ComputeMotionBlur;
	class ComputeEmitter;
    class ComputeHDR;
}

namespace GFXB4
{
    class GBuffer;
};

class GLB_Scene4 : public GLB_Scene_ES2_
{
public:
    enum WireframeMode
    {
        WireframePolygon,
        WireframeGS
    };

	GLB_Scene4(const GlobalTestEnvironment *gte);
	virtual ~GLB_Scene4();
	virtual KCL::KCL_Status  Process_GL( GLB::FBO_COLORMODE color_mode, GLB::FBO_DEPTHMODE depth_mode, int samples);
	virtual void Animate();
	virtual void Render();

	virtual KCL::KCL_Status reloadShaders();
	virtual void LoadHDR();
	virtual void ReloadHDR();
	virtual void ReloadExportedMeshes(const std::vector<std::string> & filenames);
    virtual void ReloadExportedMaterials(const std::vector<std::string> & filenames);

    virtual void SetWireframeRenderEnabled(bool value);
    WireframeMode GetWireframeMode() const { return m_wireframe_mode; }

    void Render(KCL::Camera2* camera, std::vector<KCL::Mesh*> &visible_meshes, KCL::Material *_override_material, KCL::PlanarMap* pm, KCL::uint32 lod, KCL::Light* light, PassType::Enum pass_type, bool occlusion_cull, KCL::uint32 slice_id = 0);

    void CaptureEnvmap( const KCL::Vector3D &pos, KCL::uint32 idx);
    KCL::uint32 GetVelocityBuffer() const;
    GLB::ComputeMotionBlur *GetComputeMotionBlur() const;
    GLB::InstanceManager<GLB::Mesh3::InstanceData> *GetInstanceManager() const;

    KCL::Texture* GetTopdownShadowMap() const;
    KCL::uint32 GetTopdownShadowSampler() const;

protected:
	virtual KCL::Mesh3Factory &MeshFactory()
	{
		return m_mesh3_factory;
	}

private:
	const GlobalTestEnvironment *m_gte;
    WarmUpHelper *m_warmup_helper;

	WireframeMode m_wireframe_mode;

    GFXB4::GBuffer *m_gbuffer;

    KCL::int32 m_last_animation_time;

	bool m_occlusion_cull_enabled;
	bool m_cull_non_instanced_meshes_enabled;
    bool m_render_occluders_enabled;
    bool m_compute_bright_pass_enabled;
	bool m_is_compute_warmup;

    float m_tessellation_viewport_scale;

	KCL::uint32 m_fullscreen_quad_vbo;
	KCL::uint32 m_fullscreen_quad_vao;
	FboEnvMap* m_fboEnvMap;

    ParaboloidCulling *m_paraboloid_culling;
	ParaboloidEnvMap* m_dynamic_cubemaps[2];

	GLB::GLBTexture *m_logo;
	KCL::uint32 m_logo_vbo;
	KCL::uint32 m_logo_vao;
	
	GLB::GLBShader2* m_logo_shader;

    bool cubemaps_inited;
    KCL::Actor* m_carActor_hero;
    KCL::Actor* m_carActor_evil;
    KCL::Material* m_sky_mat_paraboloid;

    std::shared_ptr<GLB::ParticleSystem4> m_psys;

    KCL::Camera2 m_car_ao_cam;

	GLB::ComputeHDR *m_compute_hdr;	
    GLB::FragmentBlur *m_dof_blur;
	GLB::Lensflare *m_lensflare;
    ////////////////////////////////////
    //          GUI
    ////////////////////////////////////
	friend class GFXGui;
	std::unique_ptr<GUIInterface> m_gui;
    virtual void UpdateGUI(KCL::uint32 cursorX, KCL::uint32 cursorY, bool mouseLPressed, bool mouseLClicked, bool mouseRPressed, bool mouseRClicked, const bool *downKeys);

    ////////////////////////////////////
    //          END OF GUI
    ////////////////////////////////////

    std::shared_ptr<UBOManager> m_ubo_manager;
    
    UBO_Container<UBlockFrame> m_ubo_frame;

    GLB::OcclusionCull *m_occlusion_cull;
	GLB::StrideBlur *m_ao_shadow_blur;
    GLB::CascadedShadowMap *m_cascaded_shadow_map;
    GLB::ComputeMotionBlur *m_compute_motion_blur;
	
	GLB::Material4 *m_sky_lightcombine_pass;
	GLB::Material4 *m_shadow_material;
	GLB::Material4 *m_transparent_shadow_material;
    GLB::Material4 *m_transparent_billboard_shadow_material;
	GLB::Material4 *m_tessellated_shadow_material;

	KCL::Texture *m_static_cubemaps;

    GLB::InstanceManager<GLB::Mesh3::InstanceData> *m_instance_manager;
	std::vector<GLB::Mesh3::InstanceData> m_instance_data;

	float m_sunColorStrength;
	float m_fogColorStrength;
	KCL::Vector3D m_fogColor;

    enum FilterNames
    {
        HDR_FINAL_PASS,
        DOF,
        MOTION_BLUR,
        SSAO,
		SSDS,
        LIGHT_COMBINE,
        FILTER_COUNT
    };

	GLB::GLBFilter filters[FILTER_COUNT];

    KCL::KCL_Status InitEmitters();
    void InitPostProcessPipeline();

    void SetRenderMaterial(KCL::Mesh *mesh, KCL::Material *override_material, PassType::Enum pass_type, KCL::uint32 slice_id = 0);
    void CollectRenderMaterials( std::vector<KCL::Mesh*> &meshes, KCL::Material *override_material, PassType::Enum pass_type, KCL::uint32 slice_id = 0);
    void CollectRenderMaterials( std::vector<KCL::MeshInstanceOwner2*> &visible_mios, KCL::Material *override_material, PassType::Enum pass_type, KCL::uint32 slice_id = 0);

    void SimulateParticles();
    void RenderParticles();
	void RenderShadow( ShadowMap* sm);
    void RenderCascadedShadow();
        
	void UpdateEnvmap( const KCL::Vector3D &pos, KCL::uint32 idx);
	CubeEnvMap* CreateEnvmap( const KCL::Vector3D &pos, KCL::uint32 idx);

    KCL::int32 GetCameraClipId(KCL::int32 time);

    static void CollectInstances( std::vector<GLB::Mesh3::InstanceData> &instance_data, std::vector<KCL::Mesh*> &instances, const std::vector< std::vector<KCL::Mesh*> > &visible_instances);
    static void CollectInstances( std::vector<GLB::Mesh3::InstanceData> &instance_data, std::vector<KCL::Mesh*> &instances, const std::vector<KCL::MeshInstanceOwner2*> &visible_mios);

    void WarmUpShaders();

	GLB::GLBShader2 *m_billboard_point_gs;
	const static int NUM_OF_EMITTERS = 4;
	GLB::ComputeEmitter *m_emitters[NUM_OF_EMITTERS];

	//#define ENABLE_TEST_EMITTER
	GLB::ComputeEmitter *m_test_emitter;//debug only

	KCL::_key_node *m_emitter_rate_anim[2];
    KCL::_key_node *m_camera_cut_track;
    KCL::int32 m_prev_camera_clip;
    bool m_camera_clip_changed;
    bool m_prev_mvp_valid;
	KCL::Texture *m_topdown_shadow;
	KCL::uint32 m_topdown_shadow_sampler;

	GFXB4::Mesh3Factory m_mesh3_factory;

	bool m_is_main_pass;
};

#endif
