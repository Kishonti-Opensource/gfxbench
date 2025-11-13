/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef MTL_MATERIAL_H
#define MTL_MATERIAL_H

#include "mtl_ogg_decoder.h"
#include "mtl_pipeline.h"
#include "mtl_planarmap.h"
#include "mtl_texture.h"
#include "mtl_pipeline.h"
#include "mtl_types.h"
#include "mtl_shader_constant_layouts_30.h"

#include <string>
#include <vector>
#include "render_statistics_defines.h"
#include "mtl_factories.h"

#include <kcl_base.h>
#include <kcl_object.h>
#include <kcl_material.h>

#include "krl_material.h"

#include <set>


#define ERROR_AT_UNBINDED_TEXTURES 0


class MTL_ogg_decoder;

namespace MetalRender
{
	class Image2D;

	class Material : public KRL::Material
	{
        friend class MaterialFactory;
    
    private:
        static const unsigned int MAX_PASS_COUNT = 3 ;
        static const int MIN_PASS_ID = -1 ;
        static const int MAX_PASS_ID =  1 ;
        
	public:

        PlanarMap *m_planar_map;
		
        KCL::KCL_Status InitShaders(const char* path, uint32_t maxJointsPerMesh, const std::set<MetalRender::ShaderType> & required_shader_types, bool forceHighP);

		virtual void LoadVideo( const char *filename);
		virtual void PlayVideo( float time_in_sec);
		virtual void DecodeVideo();
        
        virtual void DecodeMipMapVideo() ;

		~Material();

        inline void Set(id <MTLRenderCommandEncoder> renderEncoder, int pass_type, char meshType, MetalRender::ShaderType shader_type)
        {
            assert( (MIN_PASS_ID <= pass_type) && (pass_type <= MAX_PASS_ID) ) ;
            
			if(m_textures[EMISSION])
			{
				assert(m_textures[EMISSION] != nil);
			}
			
			//__builtin_printf("%s %u %u\n", m_name.c_str(), shaderBank, meshType);
            [renderEncoder setDepthStencilState:m_depthState[pass_type+1]];
            [renderEncoder setCullMode:m_cullMode[pass_type+1]];

            SetPipelineAndTextures<false,true>(renderEncoder, pass_type, meshType, shader_type);

            if(m_texture_array)
            {
                MetalRender::Texture* mtl_texture_array = static_cast<MetalRender::Texture*>(m_texture_array) ;
                mtl_texture_array->Set(renderEncoder, ARRAY_TEXTURE_SLOT);
            }
        }

        inline bool HasVideoTexture()
        {
            return m_mtlOggDecoder;
        }
        
        template<bool SetInVertexStage=false, bool SetInFragmentStage=true>
        inline void SetPipelineAndTextures(id <MTLRenderCommandEncoder> encoder, int pass_type, char meshtype, MetalRender::ShaderType shader_type)
        {
            assert( (MIN_PASS_ID <= pass_type) && (pass_type <= MAX_PASS_ID) ) ;
            
            // Set pipeline
            m_pipelines[pass_type+1][meshtype][shader_type]->Set(encoder);
            
            // Set textures
            for(int i = 0; i < MAX_IMAGE_TYPE; i++)
            {
#if ERROR_AT_UNBINDED_TEXTURES
                [encoder setFragmentTexture:nil atIndex:i] ;
                [encoder setVertexTexture:nil atIndex:i] ;
#endif
                
                if(i == 0 && m_mtlOggDecoder)
                {
                    m_mtlOggDecoder->Set(encoder, 0);
				}
                else if(m_textures[i])
                {
                    MetalRender::Texture* mtl_texture = static_cast<MetalRender::Texture*>(m_textures[i]) ;
        
                    mtl_texture->Set<SetInVertexStage, SetInFragmentStage>(encoder, i);
                    assert(i != ARRAY_TEXTURE_SLOT && i != ENVMAP_0_SLOT && i != ENVMAP_1_SLOT);
                }
            }
            
        }
		
		bool m_usesColorParam; //TODO: better name for this. means we need to set mesh color in slot 3
	//protected:
		Material(const char *name);
        
        virtual Shader* CreateShader(const char* vsfile, const char* fsfile, const std::set<std::string> *defines, KCL::KCL_Status& error) ;
        
	private:
		Material(const Material&);
		Material& operator=(const Material&);
        
        void preInit( int type) {}
		float m_video_time_in_sec;

        id<MTLDepthStencilState> m_depthState[MAX_PASS_COUNT];
        MTLCullMode m_cullMode[MAX_PASS_COUNT];
        
        Pipeline* m_pipelines[MAX_PASS_COUNT][3][MetalRender::SINGLE_SHADER_TYPE_COUNT];
        
        MTL_ogg_decoder* m_mtlOggDecoder;
        
        id <MTLDevice> m_Device ;
	};
    class MaterialFactory : public KCL::MaterialFactory
    {
    public:
        MaterialFactory(int scene_version) : m_scene_version(scene_version) {}
        virtual KCL::Material *Create(const std::string& material_name, KCL::Node *parent, KCL::Object *owner);
        
    private:
        int m_scene_version;
    };

} //namespace MetalRender

#endif // MTL_MATERIAL_H
