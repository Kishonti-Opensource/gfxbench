/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef MTL_TEXTURE_H
#define MTL_TEXTURE_H

#include "kcl_texture.h"
#include "platform.h"

#include <Metal/Metal.h>

namespace MetalRender
{
	class Texture: public KCL::Texture
	{
    public:
		Texture();
		Texture(const KCL::Image* image, bool releaseUponCommit = false);
		Texture(const KCL::Image* image, KCL::TextureType type, bool releaseUponCommit = false);
		virtual ~Texture(void);
        
        template<bool setInVertexStage=false,bool setInFragmentStage=true>
        inline void Set(id <MTLRenderCommandEncoder> renderEncoder, char slot)
        {
            assert(m_isCommitted);

			if(setInFragmentStage)
			{
				[renderEncoder setFragmentTexture:m_texture atIndex:slot];
				[renderEncoder setFragmentSamplerState:m_sampler atIndex:slot];
			}
			if(setInVertexStage)
			{
				[renderEncoder setVertexTexture:m_texture atIndex:slot];
				[renderEncoder setVertexSamplerState:m_sampler atIndex:slot];
			}
        }
        
        inline id <MTLTexture> GetTexture()
        {
            return m_texture ;
        }
        
        inline id <MTLSamplerState> GetSampler()
        {
            return m_sampler;
        }

		virtual long bind(KCL::uint32 slotId);
		virtual long commit();
		virtual long release();
		virtual long setVideoTime(float time);
        
        virtual KCL::uint32 textureObject() { return -1 ; }

	protected:

        id<MTLSamplerState> m_sampler;
        id<MTLTexture> m_texture;
        long commit2D();
        long commit3D();
        long commitCube();
        long commitArray();
        long commitCubeArray();
        void GenerateMipmaps();
		void commitETC2D(void* data, uint32_t mipmapCount);

        void setupSampler();
        
        id <MTLDevice> m_Device ;
    
    private:
        long commit2DNew();
        long commitCubeNew();
        long commitArrayNew();
        long commitCubeArrayNew();
        
	};
    
    class TextureFactory : public KCL::TextureFactory
    {
    public:
        virtual KCL::Texture* CreateTexture(const KCL::Image* img, bool releaseUponCommit = false) ;
    };

}

#endif  // MTL_TEXTURE_H
