/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef KRL_SCENE_H
#define KRL_SCENE_H

#include <kcl_base.h>
#include <kcl_scene_handler.h>
#include <kcl_aabb.h>
#include <kcl_light2.h>
#include <kcl_room.h>
#include <kcl_actor.h>

#include "krl_cubemap.h"
#include "krl_shadowmap.h"
#include "fbo_enums.h"


#include <string>
#include <vector>
#include <map>

class Shader;



namespace KCL
{
	class MeshInstanceOwner;
}

struct LightShaft
{
	KCL::Vector4D m_corners[8];
	std::vector<KCL::Vector3D> m_vertices;
	std::vector<KCL::uint16> m_indices;

	std::vector<KCL::uint32> m_index_offsets;
	std::vector<KCL::uint32> m_num_indices;

	void CreateSlices( KCL::Vector4D plane, bool isCameraShaft);
	void CreateCone( const KCL::Matrix4x4& transform, KCL::Vector4D plane, bool isCameraShaft);
};


class KRL_Scene : public KCL::SceneHandler
{
	bool m_use_zprepass;
protected:
    bool m_force_highp;

	void OnLoaded();
	void CreateMirror( const char* name);
	void CollectInstances( std::vector<KCL::Mesh*> &visible_meshes);

public:
	struct PassType
	{
		enum Enum
		{
			NORMAL = 0,
			REFLECTION = 1,
            SHADOW = 2,
			PASS_TYPE_COUNT = 3,
		};
	};
	struct ShaderVariant
	{
		enum Enum
		{
			NORMAL = 0,
			SKELETAL = 1,
			INSTANCED = 2,
			SHADER_VARIANT_COUNT = 3,
		};
	};
    struct KeyCode
    {
        enum Enum
        {
            KEY_Y               = 89,
            KEY_Z               = 90,
            KEY_LEFT_CONTROL    = 341,
            KEY_LEFT_ALT        = 342
        };
    };

    bool m_featureToggle;

	KRL_Scene();
	~KRL_Scene();

	void ResetMIOs();
	void Animate();
    virtual void UpdateGUI(KCL::uint32 cursorX, KCL::uint32 cursorY, bool leftMousePressed, bool leftMouseClicked, bool rightMousePressed, bool rightMouseClicked, const bool *keyDown) {};

	//exlude list: when rendering to dynamic cubemaps, some actors can be visible
	void FrustumCull( KCL::Camera2*camera, std::vector<KCL::PlanarMap*> &visible_planar_maps, std::vector<KCL::Mesh*> visible_meshes[3], std::vector< std::vector<KCL::Mesh*> > &visible_instances, std::vector<KCL::Mesh*> &meshes_to_blur, bool **pvs, KCL::Vector2D &nearfar, KCL::XRoom *r, bool include_sky, bool include_actors, std::vector<KCL::Actor*>* exlude_list = NULL, KCL::CullingAlgorithm *culling_algorithm = NULL);
	virtual void Resize(KCL::uint32 w, KCL::uint32 h);
	void PrepareShadow( KCL::Camera2* camera, KRL_ShadowMap *sm, KCL::Vector3D &light_dir);
	void PrepareReflection( KCL::Camera2* camera, KCL::PlanarMap *pm);
	void SetZPrePass(bool b) { m_use_zprepass = b; }
	bool UseZPrePass() const
	{
		return m_use_zprepass;
	}

    void SetForceHighp(bool forceHighp) { m_force_highp = forceHighp; }
	bool IsForceHighp() { return m_force_highp; }

	void Get2Closest(const KCL::Vector3D &pos, KRL_CubeEnvMap *&firstClosest, KRL_CubeEnvMap *&secondClosest, float &lambda);

	virtual void DeleteVBOPool() = 0;
	virtual void DeleteShaders() = 0;
	virtual void CreateVBOPool() = 0;
	virtual void Render() = 0;
	virtual KCL::KCL_Status Process_GL( GLB::FBO_COLORMODE color_mode, GLB::FBO_DEPTHMODE depth_mode, int samples) = 0;
	virtual KCL::KCL_Status reloadShaders() = 0;
	virtual void InitFactories() = 0;
	KCL::KCL_Status reloadLights();
	KCL::KCL_Status reloadEmitters();

    void SaveFreeCamTransform();
    KCL::uint32 LoadFreeCamTransform();

	virtual void ReloadExportedData(bool revert = false);
	virtual void ReloadExportedMeshes(const std::vector<std::string> & filenames, std::vector<KCL::Mesh3*> &uploaded_meshes);
	virtual void ReloadExportedTextures(const std::vector<std::string> & filenames);
	virtual void ReloadExportedMaterials(const std::vector<std::string> & filenames);
	virtual void ReloadHDR() {}

	virtual void UpdateMesh(KCL::Mesh3* mesh_) { }

    virtual void SetWireframeRenderEnabled(bool value);
    virtual bool GetWireframeRenderEnabled() const { return m_wireframe_render; }

	std::vector<KCL::Mesh*> m_visible_meshes[3];
    std::vector<KCL::Actor*> m_visible_actors;
	std::vector<KCL::Mesh*> m_shadowStaticReceiverMeshes;
	std::vector<KCL::Light*> m_visible_lights;

	std::vector<KCL::PlanarMap*> m_planarmaps;
	std::vector<KCL::PlanarMap*> m_visible_planar_maps;

	bool m_do_instancing;
	std::vector<KCL::MeshInstanceOwner*> m_mios;
	std::vector<KCL::Mesh*> m_motion_blurred_meshes;
	std::vector<KCL::Light*> m_lightshafts;
	std::vector< std::vector<KCL::Mesh*> > m_visible_instances;

	std::vector<KRL_CubeEnvMap*> m_cubemaps;

    KCL::Camera2 m_dpcam;

	KRL_ShadowMap* m_global_shadowmaps[2];

	Shader *m_shader;
	Shader *m_pp_shaders[3];
	Shader *m_lighting_shader;
	Shader *m_lighting_shaders[16];
	Shader *m_reflection_emission_shader;
	Shader *m_hud_shader;
	Shader *m_blur_shader;
	Shader *m_occlusion_query_shader;
	Shader *m_fog_shader;
	Shader *m_camera_fog_shader;
	Shader* m_particleAdvect_shader;

    float m_azimuth;
    float m_altitude;

	KCL::Vector4D m_particle_data[10000];
	KCL::Vector4D m_particle_color[10000];
	KCL::Vector4D m_box_vertices[8];

	KCL::uint32 getRenderedVerticesCount() const;
	KCL::uint32 getRenderedTrianglesCount() const;
	KCL::uint32 getDrawCalls() const;
	KCL::int32 getUsedTextureCount() const;
	KCL::int32 getSamplesPassed() const;
    float getPixelCoverage() const;
	KCL::int32 getInstructionCount() const;

	bool m_wireframe_render;
	bool m_framebuffer_gamma_correction ;
	bool m_bloom ;

    KCL::uint32 m_disabledRenderBits; //combination of flags - the selected render passes will be disabled, defaults to 0 - bits are Scene dependent
    KCL::uint32 m_adaptation_mode;
    bool m_is_warmup;

    bool m_force_cast_shadows;
    bool m_force_cast_reflections;

	std::string m_test_id;

	void SetTestId(const std::string &test_id)
	{
		m_test_id = test_id;
	}
};


inline KCL::uint32 KRL_Scene::getRenderedVerticesCount() const
{
	return m_num_vertices;
}


inline KCL::uint32 KRL_Scene::getRenderedTrianglesCount() const
{
	return m_num_triangles;
}


inline KCL::uint32 KRL_Scene::getDrawCalls() const
{
	return m_num_draw_calls;
}


inline KCL::int32 KRL_Scene::getUsedTextureCount() const
{
	return m_num_used_texture;
}


inline KCL::int32 KRL_Scene::getSamplesPassed() const
{
	return m_num_samples_passed;
}

inline float KRL_Scene::getPixelCoverage() const
{
    if(!m_pixel_coverage_sampleCount)
        return 0.0f;
    return float(m_pixel_coverage_sampleCount) / float(m_pixel_coverage_primCount);
}

inline KCL::int32 KRL_Scene::getInstructionCount() const
{
	return m_num_instruction;
}

#endif // KRL_SCENE_H
