/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "mtl_material4.h"

#include <set>
#include "metal/mtl_scene_40.h"
#include "metal/mtl_pipeline_builder.h"
#include "ng/log.h"
#include "mtl_tessellator.h"

using namespace MetalRender;

KCL::Texture* Material4::m_texture_overrides[KCL::Material::MAX_IMAGE_TYPE];

Material4::Material4(const char *name) : 
	KRL::Material(name),
    m_device(GetContext()->getDevice())
{	
	SetDefaults();
}

void Material4::SetDefaults()
{
	KRL::Material::SetDefaults();	
	ClearShaders();
	m_sort_order = 0;
}

void Material4::ClearShaders()
{
	for (KCL::uint32 i = 0; i < PassType::PASS_TYPE_COUNT_MTL40; i++)
	{
		for (KCL::uint32 j = 0; j < KRL_Scene::ShaderVariant::SHADER_VARIANT_COUNT; j++)	
		{
			// m_shaders4[i][j] = NULL;
		}
	}
}

KCL::KCL_Status Material4::InitShaders(MTL_Scene_40 *scene)
{
    m_scene = scene;

    ClearShaders();

	// Set the default shaders for the techniques
	std::string default_shaders[PassType::PASS_TYPE_COUNT_MTL40];
	default_shaders[KRL_Scene::PassType::NORMAL] = "default.shader";
	default_shaders[KRL_Scene::PassType::REFLECTION] = "simple_lightmapped.shader";
    default_shaders[KRL_Scene::PassType::SHADOW] = "shadow_caster.shader";
    
    default_shaders[PassType::NORMAL_PREPARE] = "default_tc.shader";
//    default_shaders[KRL_Scene::PassType::REFLECTION_PREPARE] = "simple_lightmapped.shader";
    default_shaders[PassType::SHADOW_PREPARE] = "shadow_caster_tc.shader";

    // Collect the defines
    std::set<std::string> resolved_defines[PassType::PASS_TYPE_COUNT_MTL40];
    resolved_defines[KRL_Scene::PassType::NORMAL] = m_shader_defs[KRL_Scene::PassType::NORMAL];
    resolved_defines[KRL_Scene::PassType::REFLECTION] = m_shader_defs[KRL_Scene::PassType::REFLECTION];
    resolved_defines[KRL_Scene::PassType::SHADOW] = m_shader_defs[KRL_Scene::PassType::SHADOW];
	
	// m_shader_defs for these passes are allways empty
	resolved_defines[PassType::NORMAL_PREPARE].clear(); // = m_shader_defs[MTL_Scene_40::PassType::NORMAL_PREPARE];
	resolved_defines[PassType::SHADOW_PREPARE].clear(); // = m_shader_defs[MTL_Scene_40::PassType::SHADOW_PREPARE];
    ResolveDefines(resolved_defines);

	std::string shader_name;
	KCL::KCL_Status result = KCL::KCL_TESTERROR_NOERROR;

	// NORMAL REFLETCTION SHADOW
	for (KCL::uint32 pass_type = 0; pass_type < PassType::PASS_TYPE_COUNT_MTL40; pass_type++)
	{
		// Set the name of the shader		
		if( (pass_type >= KRL_Scene::PassType::PASS_TYPE_COUNT) || m_shader_names[pass_type].empty())
		{
			shader_name = default_shaders[pass_type];			
		}
		else
		{
			shader_name = m_shader_names[pass_type];
		}

		// Hard code the shadow caster shader NOTE: We could remove this if we only use cascaded shadows
		if (m_name.find("shadow_caster_depth_depth") != std::string::npos)
		{
			m_material_type = SHADOWCASTER1;
			shader_name = "shadow_caster0.shader";
            return result;
		}

        std::set<std::string> defines = resolved_defines[pass_type];

		MTLDepthStencilDescriptor * depthStateDesc = [[MTLDepthStencilDescriptor alloc] init];

		depthStateDesc.depthWriteEnabled = YES;
		depthStateDesc.depthCompareFunction = MTLCompareFunctionAlways;

		MTLCullMode base_cullmode = MTLCullModeNone;

		Pipeline::BlendType blendType = Pipeline::DISABLED;

		switch (m_material_type)
		{
			case SKY:
			{
				depthStateDesc.depthWriteEnabled = NO;
				depthStateDesc.depthCompareFunction = MTLCompareFunctionLessEqual;
				break;
			}
			case WATER:
			{
				depthStateDesc.depthWriteEnabled = YES;
				depthStateDesc.depthCompareFunction = MTLCompareFunctionLess;
				blendType = Pipeline::ALPHA_X_ONE_MINUS_SOURCE_ALPHA;
				break;
			}
			case GLASS:
			{
				depthStateDesc.depthWriteEnabled = YES;
				depthStateDesc.depthCompareFunction = MTLCompareFunctionLess;

#if ENABLE_LIGHTCOMBINE_FP_RENDER_TARGET
				blendType = Pipeline::ALPHA_X_ONE_MINUS_SOURCE_ALPHA;
#else
				// Special blending for offscreen target: premultiplied alpha
				blendType = Pipeline::ONE_X_ONE_MINUS_SOURCE_ALPHA;
#endif
				break;
			}
			case LIGHTSHAFT:
			{
				depthStateDesc.depthWriteEnabled = NO;
				depthStateDesc.depthCompareFunction = MTLCompareFunctionLess;
				blendType = Pipeline::ONE_X_ONE;
				break;
			}
			case FOLIAGE:
			{
				depthStateDesc.depthWriteEnabled = YES;
				depthStateDesc.depthCompareFunction = MTLCompareFunctionLess;
				break;
			}
			case FLAME:
			{
				depthStateDesc.depthWriteEnabled = NO;
				depthStateDesc.depthCompareFunction = MTLCompareFunctionLess;
				blendType = Pipeline::ONE_X_ONE;
				break;
			}
			case FIRE:
			{
				depthStateDesc.depthWriteEnabled = NO;
				depthStateDesc.depthCompareFunction = MTLCompareFunctionLess;
				blendType = Pipeline::ONE_X_ONE_MINUS_SOURCE_ALPHA;
				break;
			}
			case SMOKE:
			{
				depthStateDesc.depthWriteEnabled = NO;
				depthStateDesc.depthCompareFunction = MTLCompareFunctionLess;
				blendType = Pipeline::ALPHA_X_ONE_MINUS_SOURCE_ALPHA;
				break;
			}
			case STEAM:
			{
				depthStateDesc.depthWriteEnabled = NO;
				depthStateDesc.depthCompareFunction = MTLCompareFunctionLess;
				blendType = Pipeline::ONE_X_ONE_MINUS_SOURCE_ALPHA;
				break;
			}
			case DECAL:
			{
				depthStateDesc.depthWriteEnabled = NO;
				depthStateDesc.depthCompareFunction = MTLCompareFunctionLess;
				blendType = Pipeline::COLOR_ALPHA_X_COLOR_ALPHA;
				break;
			}
			case OMNILIGHT:
			{
				break;
			}
			case SHADOWCASTER0:
			{
				break;
			}
			case SHADOWRECEIVER0:
			{
				depthStateDesc.depthWriteEnabled = NO;
				depthStateDesc.depthCompareFunction = MTLCompareFunctionEqual;
				blendType = Pipeline::COLOR_ALPHA_X_ZERO;
				break;
			}
			case SHADOWCASTER1:
			{
				depthStateDesc.depthWriteEnabled = YES;
				depthStateDesc.depthCompareFunction = MTLCompareFunctionLess;
				base_cullmode = MTLCullModeFront;
				break;
			}
			case SHADOWRECEIVER1:
			{
				depthStateDesc.depthWriteEnabled = NO;
				depthStateDesc.depthCompareFunction = MTLCompareFunctionEqual;
				blendType = Pipeline::COLOR_ALPHA_X_ZERO;
				break;
			}
			case PLANAR_REFLECTION:
			{
				depthStateDesc.depthWriteEnabled = NO;
				depthStateDesc.depthCompareFunction = MTLCompareFunctionEqual;
				blendType = Pipeline::ALPHA_X_ONE_MINUS_SOURCE_ALPHA;
				break;
			}
			default:
			{
				depthStateDesc.depthWriteEnabled = YES;
				depthStateDesc.depthCompareFunction = MTLCompareFunctionLess;
				base_cullmode = MTLCullModeBack;
			}
		}

		if(m_is_two_sided)
		{
			base_cullmode = MTLCullModeNone;
		}

		for (KCL::uint32 shader_variant = 0; shader_variant < KRL_Scene::ShaderVariant::SHADER_VARIANT_COUNT; shader_variant++)
		{
			MTLPipeLineBuilder sb;

            // GFXBench 4.0 does not use skeletal now, so it is pointless to compile the shaders
            if (shader_variant == KRL_Scene::ShaderVariant::SKELETAL)
            {
                continue;
            }

			sb.AddDefines(&resolved_defines[pass_type]);

          	if (shader_variant == KRL_Scene::ShaderVariant::INSTANCED)
			{
				sb.AddDefine("INSTANCING");
			}

#ifdef _DEBUG
            if(scene->GetWireframeRenderEnabled() && scene->GetWireframeMode() == GLB_Scene4::WireframeGS)
            {
                sb.AddDefine("USE_GEOMSHADER");
            }
#endif

			MetalRender::ShaderType shader_type = kShaderTypeUnknown;

			if (pass_type == KRL_Scene::PassType::NORMAL)
			{
				if (m_is_transparent)
					shader_type = kShaderTypeSingleRGBA8Default;
				else
					shader_type = kShaderTypeCarChaseGBuffer;
			}
			else if (pass_type == KRL_Scene::PassType::REFLECTION)
			{
				shader_type = kShaderTypeSingleRGBA8Default;
			}
			else if (pass_type == KRL_Scene::PassType::SHADOW)
			{
				shader_type = kShaderTypeCarChaseShadow;
			}
            
            MTLVertexDescriptor* vertexDescriptor = nil;
            if(m_is_tesselated &&
               ((pass_type == KRL_Scene::PassType::NORMAL) || (pass_type == KRL_Scene::PassType::SHADOW)))
            {
                vertexDescriptor = [[MTLVertexDescriptor alloc] init];
                
                MTLRenderPipelineDescriptor* desc = [[MTLRenderPipelineDescriptor alloc] init];
            
                
                desc.tessellationPartitionMode = MTLTessellationPartitionModeFractionalOdd;
                if(pass_type == KRL_Scene::PassType::SHADOW)
                    desc.maxTessellationFactor = 2;
                else
                    desc.maxTessellationFactor = 16;

                desc.tessellationFactorScaleEnabled = false;
                desc.tessellationOutputWindingOrder = MTLWindingCounterClockwise;
                
                if(Tessellator::UseStageInControlPoints() && m_displacement_mode == DISPLACEMENT_LOCAL)
                {
                    desc.tessellationControlPointIndexType = MTLTessellationControlPointIndexTypeUInt16;
                    vertexDescriptor = Tessellator::GetVertexDescriptor();
                }
                
                if(pass_type == KRL_Scene::PassType::SHADOW)
                    desc.tessellationFactorStepFunction = MTLTessellationFactorStepFunctionConstant;
                else
                    desc.tessellationFactorStepFunction = MTLTessellationFactorStepFunctionPerPatchAndPerInstance;
                
                sb.SetBaseDescriptor(desc);
            }
            else
            {
                vertexDescriptor = GFXB4::Mesh3::GetVertexDescriptor();
            }            

			
			bool force_highp = false;
			// the tessellation kernel (NORMAL_PREPARE and SHADOW_PREPARE) pass is allways highp
			force_highp = (pass_type == PassType::NORMAL_PREPARE) || (pass_type == PassType::SHADOW_PREPARE);
			
			m_pipelines[pass_type][shader_variant] = sb
			.ShaderFile(shader_name.c_str())
			.SetType(shader_type)
			.HasDepth(true)
			.SetBlendType(blendType)
			.SetVertexLayout(vertexDescriptor)
            .SetAlwaysTGSize(m_is_tesselated)
			.ForceHighp(force_highp)
			.Build(result);

			if (result != KCL::KCL_TESTERROR_NOERROR)
			{
				return result;
			}
		}
		m_cullmode[pass_type] = base_cullmode;
		m_depth_state[pass_type] = [m_device newDepthStencilStateWithDescriptor:depthStateDesc];

		releaseObj(depthStateDesc);		
	}

	SetSortOrder();

	return KCL::KCL_TESTERROR_NOERROR;
}

void Material4::ResolveDefines(std::set<std::string> resolved_defines[PassType::PASS_TYPE_COUNT_MTL40]) const
{
	resolved_defines[KRL_Scene::PassType::NORMAL].insert("UBYTE_NORMAL_TANGENT");
	resolved_defines[KRL_Scene::PassType::REFLECTION].insert("UBYTE_NORMAL_TANGENT");
	resolved_defines[KRL_Scene::PassType::SHADOW].insert("UBYTE_NORMAL_TANGENT");

    resolved_defines[KRL_Scene::PassType::NORMAL].insert("VELOCITY_BUFFER");

    if(!m_is_transparent)
    {
        resolved_defines[KRL_Scene::PassType::NORMAL].insert("G_BUFFER_PASS");
    }
    
    if (m_has_emissive_channel)
    {
        resolved_defines[KRL_Scene::PassType::NORMAL].insert("HAS_LOCAL_EMISSIVE");
        resolved_defines[KRL_Scene::PassType::REFLECTION].insert("HAS_LOCAL_EMISSIVE");
    }

    if (m_opacity_mode == ALPHA_TEST)
    {
         resolved_defines[KRL_Scene::PassType::NORMAL].insert("HAS_LOCAL_OPACITY");
         resolved_defines[KRL_Scene::PassType::NORMAL].insert("ALPHA_TEST");

		 resolved_defines[KRL_Scene::PassType::SHADOW].insert("HAS_LOCAL_OPACITY");
         resolved_defines[KRL_Scene::PassType::SHADOW].insert("ALPHA_TEST");
    }

    if (m_opacity_mode == ALPHA_BLEND)
    {
         resolved_defines[KRL_Scene::PassType::NORMAL].insert("HAS_LOCAL_OPACITY");
    }

    if (m_is_tesselated)
    {    
        if(Tessellator::UseStageInControlPoints())
        {
            resolved_defines[KRL_Scene::PassType::NORMAL].insert("USE_STAGE_IN_CONTROL_POINTS");
            
            resolved_defines[PassType::NORMAL_PREPARE].insert("USE_STAGE_IN_CONTROL_POINTS");
            
            resolved_defines[PassType::SHADOW_PREPARE].insert("USE_STAGE_IN_CONTROL_POINTS");
            
            resolved_defines[KRL_Scene::PassType::SHADOW].insert("USE_STAGE_IN_CONTROL_POINTS");
        }
        
        if (m_displacement_mode == DISPLACEMENT_LOCAL)
        {
            resolved_defines[KRL_Scene::PassType::NORMAL].insert("HAS_LOCAL_DISPLACEMENT");
            resolved_defines[KRL_Scene::PassType::NORMAL].insert("USE_TESSELLATION");
            
            resolved_defines[PassType::NORMAL_PREPARE].insert("HAS_LOCAL_DISPLACEMENT");
            resolved_defines[PassType::NORMAL_PREPARE].insert("USE_TESSELLATION");
            
            resolved_defines[PassType::SHADOW_PREPARE].insert("HAS_LOCAL_DISPLACEMENT");
            resolved_defines[PassType::SHADOW_PREPARE].insert("USE_TESSELLATION");

            resolved_defines[KRL_Scene::PassType::SHADOW].insert("HAS_LOCAL_DISPLACEMENT");
            resolved_defines[KRL_Scene::PassType::SHADOW].insert("USE_TESSELLATION");
        }

        if (m_displacement_mode == DISPLACEMENT_ABS)
        {
            resolved_defines[KRL_Scene::PassType::NORMAL].insert("HAS_ABS_DISPLACEMENT");
            resolved_defines[KRL_Scene::PassType::NORMAL].insert("USE_TESSELLATION");
            
            resolved_defines[PassType::NORMAL_PREPARE].insert("HAS_ABS_DISPLACEMENT");
            resolved_defines[PassType::NORMAL_PREPARE].insert("USE_TESSELLATION");
            
            resolved_defines[PassType::SHADOW_PREPARE].insert("HAS_ABS_DISPLACEMENT");
            resolved_defines[PassType::SHADOW_PREPARE].insert("USE_TESSELLATION");
            
            resolved_defines[KRL_Scene::PassType::SHADOW].insert("HAS_ABS_DISPLACEMENT");
            resolved_defines[KRL_Scene::PassType::SHADOW].insert("USE_TESSELLATION");
        }
    }    

    if (m_is_billboard)
    {
        resolved_defines[KRL_Scene::PassType::NORMAL].insert("IS_BILLBOARD");
        resolved_defines[KRL_Scene::PassType::SHADOW].insert("IS_BILLBOARD");
    }

    if (m_has_car_ao)
    {
        resolved_defines[KRL_Scene::PassType::NORMAL].insert("HAS_CAR_AO");
    }

    if (m_is_car_paint)
    {
        resolved_defines[KRL_Scene::PassType::NORMAL].insert("HAS_CAR_PAINT");
    }

    if (m_is_two_sided)
    {
        resolved_defines[KRL_Scene::PassType::NORMAL].insert("IS_TWO_SIDED");
    }

    if(m_use_world_ao)
    {
        resolved_defines[KRL_Scene::PassType::NORMAL].insert("USE_WORLD_AO");
    }
}

void Material4::SetPipelineAndTextures(RenderInfo& info, KCL::uint32 pass_type, KCL::uint32 shader_variant, KCL::uint32 & texture_num)
{
    Pipeline * pipeline = m_pipelines[pass_type][shader_variant];
    
    info.renderPipeline = pipeline;
    
    switch (pass_type)
    {
        case KRL_Scene::PassType::NORMAL:
            info.preparePipeline = m_pipelines[PassType::NORMAL_PREPARE][shader_variant];
            break;
        case KRL_Scene::PassType::SHADOW:
            info.preparePipeline = m_pipelines[PassType::SHADOW_PREPARE][shader_variant];
            break;
        default:
            info.preparePipeline = nullptr;
            break;
    }
    
    info.depthStencil = m_depth_state[pass_type];
    info.cullMode = m_cullmode[pass_type];
    
    for (KCL::uint32 i = 0; i < 8; ++i)
    {
        if (m_textures[i])
        {
            if (m_texture_overrides[i])
            {
                MetalRender::Texture * mtl_texture = static_cast<MetalRender::Texture*>(m_texture_overrides[i]) ;
                
                info.fragmentTextures[i] = mtl_texture->GetTexture();
                info.fragmentSamplers[i] = mtl_texture->GetSampler();
                
                if (m_texture_overrides[i] == m_scene->GetTopdownShadowMap())
                {
                    info.fragmentSamplers[i] = m_scene->GetTopdownShadowSampler();
                }
            }
            else
            {
                MetalRender::Texture * mtl_texture = static_cast<MetalRender::Texture*>(m_textures[i]) ;
                
                info.fragmentTextures[i] = mtl_texture->GetTexture();
                info.fragmentSamplers[i] = mtl_texture->GetSampler();
            }
        }
    }
}


void Material4::SetTextureOverrides(KCL::Texture* ovrds[])
{
    memcpy(&m_texture_overrides[0], &ovrds[0], sizeof(m_texture_overrides));
}


KCL::KCL_Status Material4::InitShaders( const char* path,  const std::string &max_joint_num_per_mesh)
{		
	INFO("Warning! GLB::Material4::InitShaders is not implemented! Use InitShader(GLB_Scene4*)");
    return KCL::KCL_TESTERROR_NOERROR;
}

void Material4::preInit( KCL::uint32 &texture_num, int type, int pass_type)
{
	INFO("Warning! GLB::Material4::preInit4 is not implemented! Use Bind()");
}

void Material4::postInit()
{
	INFO("Warning! GLB::Material4::postInit is not implemented! Use Unbind()");
}

void Material4::SetSortOrder()
{
	bool tesselated = m_displacement_mode != KCL::Material::NO_DISPLACEMENT;
	bool alpha_tested = m_opacity_mode == ALPHA_TEST;
	bool alpha_blended = m_opacity_mode == ALPHA_BLEND;
	bool opaque = !alpha_tested  && !alpha_blended;
	
	if (m_material_type == SKY)
	{
		// Render the sky last
		m_sort_order = 8;
	}
	else if (tesselated)
	{
		m_sort_order = 0;
	}
	else
	{
		if (opaque)
		{
			m_sort_order = 1;
		}
		else if (alpha_tested)
		{
			m_sort_order = 2;
		}
		else if (alpha_blended)
		{
			m_sort_order = 4;
		}
		else
		{
			INFO("Can not define sort order for material: %s", m_name.c_str());
			m_sort_order = 0;
		}
	}
}

Shader* Material4::CreateShader(const char* vsfile, const char* fsfile, const std::set<std::string> *defines, KCL::KCL_Status& error)
{
	return Shader::CreateShader(vsfile, fsfile, defines, error);
}
