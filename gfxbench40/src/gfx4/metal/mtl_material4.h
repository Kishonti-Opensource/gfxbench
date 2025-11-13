/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef MTL_MATERIAL4_H
#define MTL_MATERIAL4_H

#include "metal/mtl_material.h"
//#include "opengl/glb_shader2.h"
#include "krl_material.h"
#include "krl_scene.h"

#include <string>
#include <set>

class MTL_Scene_40;

namespace GFXB4
{
    class Mesh3;
}


struct RenderInfo
{
    MetalRender::Pipeline* renderPipeline = nullptr;
    MetalRender::Pipeline* preparePipeline = nullptr;
    
    id<MTLDepthStencilState> depthStencil;
    MTLCullMode cullMode;
    bool tessellate = false;
    bool useConstantOne;
    bool occlusionCull;
    unsigned lod;
    
    
    id<MTLTexture> fragmentTextures[22] = {nil};
    id<MTLSamplerState> fragmentSamplers[16] = {nil};
    
    NSUInteger instanceDataOffset = 0;
    NSUInteger vertexOffsets[4] = {0};
    NSUInteger fragmentOffsets[4] = {0};
    
    GFXB4::Mesh3* glbMesh3;
    KCL::Mesh* sm;
};

namespace MetalRender
{

class Material4 : public KRL::Material
{
public:
	static const KCL::uint32 INSTANCE_BUFFER_BINDING = 0;

	struct PassType
	{
		enum Enum
		{
			NORMAL = KRL_Scene::PassType::NORMAL,
			REFLECTION = KRL_Scene::PassType::REFLECTION,
			SHADOW = KRL_Scene::PassType::SHADOW,
			NORMAL_PREPARE = 3,
			SHADOW_PREPARE = 4,
			PASS_TYPE_COUNT_MTL40 = 5
		};
	};
	
	Material4( const char *name);

	virtual void SetDefaults();	

    KCL::KCL_Status InitShaders(MTL_Scene_40 *scene);
	Shader* CreateShader(const char* vsfile, const char* fsfile, const std::set<std::string> *defines, KCL::KCL_Status& error);

    void SetPipelineAndTextures(RenderInfo& info, KCL::uint32 pass_type, KCL::uint32 shader_variant, KCL::uint32 & texture_num);
	KCL::int32 m_sort_order;

    static void SetTextureOverrides(KCL::Texture* ovrds[]);

    // Old API from KRL::Material, should be never called
	virtual KCL::KCL_Status InitShaders( const char* path,  const std::string &max_joint_num_per_mesh);
	virtual void preInit( KCL::uint32 &texture_num, int type, int pass_type);
	virtual void postInit();

private:
    void ResolveDefines(std::set<std::string> resolved_defines[PassType::PASS_TYPE_COUNT_MTL40]) const;
	void ClearShaders();

	void SetSortOrder();

    static KCL::Texture* m_texture_overrides[KCL::Material::MAX_IMAGE_TYPE];
	Pipeline * m_pipelines[PassType::PASS_TYPE_COUNT_MTL40][KRL_Scene::ShaderVariant::SHADER_VARIANT_COUNT];

	id<MTLDevice> m_device;
	id<MTLDepthStencilState> m_depth_state[PassType::PASS_TYPE_COUNT_MTL40];
	MTLCullMode	m_cullmode[PassType::PASS_TYPE_COUNT_MTL40];
	MTL_Scene_40 * m_scene;
};

}
#endif //MTL_MATERIAL4_H
