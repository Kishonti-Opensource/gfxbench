/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "mtl_material.h"
//#include "mtl_image.h"

#include "mtl_ogg_decoder.h"
#import "mtl_pipeline.h"
#ifdef OPT_TEST_GFX40
#include "metal/mtl_material4.h"
#endif
#include "shader.h"

#include "mtl_types.h"
#include "mtl_mesh.h"
#import "graphics/metalgraphicscontext.h"

using namespace MetalRender;


Material::Material(const char *name) :
	KRL::Material(name),
	m_planar_map(0),
	m_mtlOggDecoder(0),
    m_usesColorParam(NO),
    m_Device(MetalRender::GetContext()->getDevice())
{
	for( KCL::uint32 i = 0; i < MAX_PASS_COUNT; i++)
	{
		for( KCL::uint32 j = 0; j < 3; j++)
		{
            for (KCL::uint32 l = 0 ; l < SINGLE_SHADER_TYPE_COUNT ; l++)
            {
                m_pipelines[i][j][l] = nullptr;
            }
		}
	}
    
    for( KCL::uint32 i = 0; i < 2; i++)
    {
        for( KCL::uint32 j = 0; j < 3; j++)
        {
            m_shaders[i][j] = nullptr ;
        }
    }
}

Material::~Material()
{
    for( KCL::uint32 i=0; i<MAX_PASS_COUNT; i++)
	{
        releaseObj(m_depthState[i]);
        
        for( KCL::uint32 j=0; j<3; j++)
        {
            for (KCL::uint32 l = 0 ; l < SINGLE_SHADER_TYPE_COUNT ; l++)
            {
                // delete m_pipelines[i][j]; // need to delete s_pipelineCache elsewhere
                // HACK HACK HACK TODO leaking for now
                m_pipelines[i][j][l]  = NULL;
            }
        }
    }
    
    for( KCL::uint32 i=0; i<2; i++)
    {
        for( KCL::uint32 j=0; j<3; j++)
        {
            delete m_shaders[i][j] ;
            m_shaders[i][j] = nullptr ;
        }
    }

	delete m_mtlOggDecoder;


    // DON'T delete m_textureAlias or m_TextureArrayAlias since those are
    // simply aliases to instances managed by base class
}


void Material::LoadVideo( const char *filename)
{
	m_mtlOggDecoder = new MTL_ogg_decoder(filename);

}


void Material::PlayVideo( float time_in_sec)
{
	m_video_time_in_sec = time_in_sec;

	m_mtlOggDecoder->Play( m_video_time_in_sec);
}

void Material::DecodeVideo()
{
    assert(false) ;
	//m_mtlOggDecoder->DecodeDirect(m_video_time_in_sec);
}

void Material::DecodeMipMapVideo()
{
    m_mtlOggDecoder->DecodeDirect(m_video_time_in_sec) ;
}


KCL::KCL_Status Material::InitShaders(const char* path, uint32_t maxJointsPerMesh, const std::set<MetalRender::ShaderType> & required_shader_types, bool forceHighP)
{
    char max_joint_num_per_mesh_str[64];
    sprintf( max_joint_num_per_mesh_str, " %d ", maxJointsPerMesh);
    
    std::string max_joint_num_per_mesh(max_joint_num_per_mesh_str) ;
    
    
    KCL::KCL_Status result = KRL::Material::InitShaders(path, max_joint_num_per_mesh) ;
    
    if (result != KCL::KCL_TESTERROR_NOERROR)
    {
        return result ;
    }
    
	if( strstr( m_name.c_str(), "sky"))
	{
        m_material_type = SKY;
	}
	else if( strstr( m_name.c_str(), "glow"))
	{
		m_usesColorParam = YES;
	}

    MTLDepthStencilDescriptor *depthStateDesc = [[MTLDepthStencilDescriptor alloc] init];
    
    //
    //  "postInit()"
    //
    
    //OpenGLStateManager::GlDisable( GL_DEPTH_TEST);
    //OpenGLStateManager::GlDepthFunc( GL_LESS);
    //OpenGLStateManager::GlDepthMask( 1);
    depthStateDesc.depthCompareFunction = MTLCompareFunctionAlways;
    depthStateDesc.depthWriteEnabled = NO;
    
    //OpenGLStateManager::GlCullFace( GL_BACK);
    //OpenGLStateManager::GlDisable( GL_CULL_FACE);
    MTLCullMode baseCullMode = MTLCullModeNone;
    
    
    //OpenGLStateManager::GlDisable( GL_BLEND);
    Pipeline::BlendType blendType = Pipeline::DISABLED;

    
	switch(m_material_type)
	{
        case SKY:
		{
            depthStateDesc.depthWriteEnabled = NO;
            depthStateDesc.depthCompareFunction = MTLCompareFunctionLessEqual;
			break;
		}
        case WATER:
		{
            blendType = Pipeline::ALPHA_X_ONE_MINUS_SOURCE_ALPHA;
			depthStateDesc.depthCompareFunction = MTLCompareFunctionLess;
            depthStateDesc.depthWriteEnabled = YES;
			break;
		}
        case GLASS:
		{
            blendType = Pipeline::ALPHA_X_ONE_MINUS_SOURCE_ALPHA;
            depthStateDesc.depthWriteEnabled = NO;
			depthStateDesc.depthCompareFunction = MTLCompareFunctionLess;
			break;
		}
        case LIGHTSHAFT:
		{
            blendType = Pipeline::ONE_X_ONE;
            depthStateDesc.depthWriteEnabled = NO;
			depthStateDesc.depthCompareFunction = MTLCompareFunctionLess;
			break;
		}
        case FOLIAGE:
		{
            depthStateDesc.depthCompareFunction = MTLCompareFunctionLess;
            depthStateDesc.depthWriteEnabled = YES;
			break;
		}
        case FLAME:
		{
            blendType = Pipeline::ONE_X_ONE;
            depthStateDesc.depthWriteEnabled = NO;
			depthStateDesc.depthCompareFunction = MTLCompareFunctionLess;
			break;
		}
        case FIRE:
		{
            blendType = Pipeline::ONE_X_ONE_MINUS_SOURCE_ALPHA;
            depthStateDesc.depthWriteEnabled = NO;
			depthStateDesc.depthCompareFunction = MTLCompareFunctionLess;
			break;
		}
        case SMOKE:
		{
            blendType = Pipeline::ALPHA_X_ONE_MINUS_SOURCE_ALPHA;
            depthStateDesc.depthWriteEnabled = NO;
			depthStateDesc.depthCompareFunction = MTLCompareFunctionLess;
			break;
		}
        case STEAM:
		{
            blendType = Pipeline::ONE_X_ONE_MINUS_SOURCE_ALPHA;
            depthStateDesc.depthWriteEnabled = NO;
			depthStateDesc.depthCompareFunction = MTLCompareFunctionLess;
			break;
		}
        case DECAL:
		{
            blendType = Pipeline::COLOR_ALPHA_X_COLOR_ALPHA;
            depthStateDesc.depthWriteEnabled = NO;
			depthStateDesc.depthCompareFunction = MTLCompareFunctionLess;
			break;
		}
        case OMNILIGHT:
		{
            //depthStateDesc.depthCompareFunction = MTLCompareFunctionLess;
            //depthStateDesc.depthWriteEnabled = NO;
			break;
		}
        case SHADOWCASTER0:
		{
            //depthStateDesc.depthCompareFunction = MTLCompareFunctionLess;
            //depthStateDesc.depthWriteEnabled = NO;
			break;
		}
        case SHADOWRECEIVER0:
		{
            blendType = Pipeline::COLOR_ALPHA_X_ZERO;
            depthStateDesc.depthWriteEnabled = NO;
            depthStateDesc.depthCompareFunction = MTLCompareFunctionEqual;
			break;
		}
        case SHADOWCASTER1:
		{
			//depth test on, normal depth test, backface culling
            depthStateDesc.depthCompareFunction = MTLCompareFunctionLess;
            depthStateDesc.depthWriteEnabled = YES;
            baseCullMode = MTLCullModeBack;
			break;
		}
        case SHADOWRECEIVER1:
		{
            blendType = Pipeline::COLOR_ALPHA_X_ZERO;
            depthStateDesc.depthWriteEnabled = NO;
            depthStateDesc.depthCompareFunction = MTLCompareFunctionEqual;
			break;
		}
        case PLANAR_REFLECTION:
		{
            blendType = Pipeline::ALPHA_X_ONE_MINUS_SOURCE_ALPHA;
            depthStateDesc.depthWriteEnabled = NO;
            depthStateDesc.depthCompareFunction = MTLCompareFunctionEqual;
			break;
		}
        default:
		{
            baseCullMode = MTLCullModeBack;
            depthStateDesc.depthCompareFunction = MTLCompareFunctionLess;
            depthStateDesc.depthWriteEnabled = YES;
		}
	}
    
    //
    //  Create Different Passes
    //
    for( KCL::uint32 i=0; i<MAX_PASS_COUNT; i++)
    {
        m_cullMode[i] = baseCullMode ;
        MTLDepthStencilDescriptor *perPassDepthStateDesc = [depthStateDesc copy];
        Pipeline::BlendType perPassBlendType = blendType ;
        
        int pass_type = i - 1 ;
        int shader_bank = (pass_type == -1) ? 1 : 0 ;
        
        
        switch (pass_type)
        {
            /* depth prepass */
            case -1:
                /* NOTE: colour writes are disabled outside the state manager! */
                switch(m_material_type)
                {
                    case FOLIAGE:
                        //OpenGLStateManager::GlEnable(GL_DEPTH_TEST);
                        //OpenGLStateManager::GlDepthFunc(GL_LESS);
                        
                        perPassDepthStateDesc.depthCompareFunction = MTLCompareFunctionLess;
                        perPassDepthStateDesc.depthWriteEnabled = YES;
                        break;
                        
                    case DEFAULT:
                        //OpenGLStateManager::GlEnable(GL_CULL_FACE);
                        //OpenGLStateManager::GlCullFace(GL_BACK);
                        //OpenGLStateManager::GlEnable(GL_DEPTH_TEST);
                        //OpenGLStateManager::GlDepthFunc(GL_LESS);
                        
                        m_cullMode[i] = MTLCullModeBack;
                        perPassDepthStateDesc.depthCompareFunction = MTLCompareFunctionLess;
                        perPassDepthStateDesc.depthWriteEnabled = YES;
                        break;
                    default:
                        break;
                }
                perPassBlendType = Pipeline::NO_COLOR_WRITES;
                break;
                
                /* shade pass */
            case +1:
                switch(m_material_type)
                {
                    case FOLIAGE:
                        //OpenGLStateManager::GlEnable(GL_DEPTH_TEST);
                        //OpenGLStateManager::GlDepthMask(0);
                        //OpenGLStateManager::GlDepthFunc(GL_EQUAL);
                        
                        perPassDepthStateDesc.depthCompareFunction = MTLCompareFunctionEqual;
                        perPassDepthStateDesc.depthWriteEnabled = NO;
                        break;
                        
                    case DEFAULT:
                        //OpenGLStateManager::GlEnable(GL_CULL_FACE);
                        //OpenGLStateManager::GlCullFace(GL_BACK);
                        //OpenGLStateManager::GlEnable(GL_DEPTH_TEST);
                        //OpenGLStateManager::GlDepthMask(0);
                        //OpenGLStateManager::GlDepthFunc(GL_LEQUAL);
                        
                        perPassDepthStateDesc.depthCompareFunction = MTLCompareFunctionLessEqual;
                        perPassDepthStateDesc.depthWriteEnabled = NO;
                        m_cullMode[i] = MTLCullModeBack;
                        
                        break;
                    default:
                        break;
                }
                break;
            default:
                break;
        }
        
        for( KCL::uint32 j=0; j<3; j++)
        {
            if (m_shaders[shader_bank][j] == nullptr)
            {
                continue ;
            }
            
            if( strstr( path, "SV_27") && strstr(m_shaders[shader_bank][j]->getFsFile().c_str(),"smoke.fs" ) )
            {
                m_shaders[shader_bank][j]->AddDefine("USE_SMOKE");
            }
            
            if( strstr( path, "SV_27") && strstr(m_shaders[shader_bank][j]->getFsFile().c_str(),"steam.fs" ) )
            {
                m_shaders[shader_bank][j]->AddDefine("USE_STEAM");
            }
            
            for ( int l = 0 ; l < SINGLE_SHADER_TYPE_COUNT; l++)
            {
                
                ShaderType shaderType = (ShaderType)l ;
                
                if (required_shader_types.find(shaderType) == required_shader_types.end())
                {
                    m_pipelines[i][j][l] = nullptr ;
                    continue ;
                }
            
                //
                //
                //  Special shade typer override
                //
                //
                
                if( (m_shaders[shader_bank][j]->getVsFile() == "gbuffer.vs") && (m_shaders[shader_bank][j]->getFsFile() == "gbuffer.fs") )
                {
                    shaderType = kShaderTypeManhattanGBuffer;
                }
                
                // In T-rex the shadowmap has RGBA8 hardware format
                if( strstr( path, "SV_27") && strstr(m_shaders[shader_bank][j]->getFsFile().c_str(),"shadow_caster" ) )
                {
                    shaderType = kShaderTypeSingleRGBA8Default ;
                }
                
                // In manhattan the shadowmap has no color attachment
                if( strstr( path, "SV_30") && strstr(m_shaders[shader_bank][j]->getFsFile().c_str(),"shadow_caster0" ) )
                {
                    shaderType = kShaderTypeNoColorAttachment ;
                }
                if( strstr( path, "SV_31") && strstr(m_shaders[shader_bank][j]->getFsFile().c_str(),"shadow_caster0" ) )
                {
                    shaderType = kShaderTypeNoColorAttachment ;
                }
                

                Pipeline::GFXPipelineDescriptor pipelineDescriptor(perPassBlendType,
                            shaderType,
                            (j==1)?(Mesh3::s_skeletal_vertex_layout.m_vertex_descriptor):(Mesh3::s_vertex_layout.m_vertex_descriptor),
                            YES,
                            forceHighP) ;
                
                m_pipelines[i][j][l] = Pipeline::CreatePipeline(m_shaders[shader_bank][j],
                                                                pipelineDescriptor,
                                                                result);
                
                
                if (result != KCL::KCL_TESTERROR_NOERROR)
                {
                    return result ;
                }
            }
        }
        
        
        m_depthState[i] = [m_Device newDepthStencilStateWithDescriptor:perPassDepthStateDesc];
        releaseObj(perPassDepthStateDesc);
    }
   
    releaseObj(depthStateDesc);


	return result;
}


Shader* Material::CreateShader(const char* vsfile, const char* fsfile, const std::set<std::string> *defines, KCL::KCL_Status& error)
{
    return Shader::CreateShader(vsfile,fsfile,defines,error) ;
}


KCL::Material *MetalRender::MaterialFactory::Create(const std::string& material_name, KCL::Node *parent, KCL::Object *owner)
{
    assert(m_scene_version != KCL::SV_INVALID);
#ifdef OPT_TEST_GFX40
    if (m_scene_version >= KCL::SV_40)
	{
		return new MetalRender::Material4(material_name.c_str());
	}
	else
#endif
	{
		return new MetalRender::Material(material_name.c_str());
	}
}

