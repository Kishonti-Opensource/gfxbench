/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef MTL_SHADOWMAP
#define MTL_SHADOWMAP
#import <Metal/Metal.h>

#include "krl_shadowmap.h"

namespace MetalRender
{

class ShadowMap : public KRL_ShadowMap
{
private:
    
    enum ShadowMethod
    {
        METHOD_DEPTH_MAP_DEPTH,
        METHOD_DEPTH_MAP_COLOR,
        METHOD_UNKNOWN
    };
    
public:
    ShadowMap(unsigned int width, unsigned int height, const std::string &method_str);
    virtual ~ShadowMap();

    inline id <MTLRenderCommandEncoder> SetAsTargetAndGetEncoder(id <MTLCommandBuffer> commandBuffer)
    {
        return [commandBuffer renderCommandEncoderWithDescriptor:m_framebuffer];
    }

    inline void SetAsTexture(id <MTLRenderCommandEncoder> renderEncoder, unsigned char slot)
    {
        if (m_method == ShadowMap::METHOD_DEPTH_MAP_DEPTH)
        {
            // compare function can be specified from shader code only
            [renderEncoder setFragmentTexture:m_depthTexture atIndex:slot];
        }
        else if (m_method == ShadowMap::METHOD_DEPTH_MAP_COLOR)
        {
            [renderEncoder setFragmentSamplerState:m_colorSampler atIndex:slot];
            [renderEncoder setFragmentTexture:m_colorTexture atIndex:slot];
        }
        else
        {
            assert(0) ;
        }
    }
    
    virtual void Bind() { ; }
    virtual void Unbind() { ; }
    virtual void Clear() { ; }
    virtual const int Size() const { return m_width ; }
    virtual unsigned int GetTextureId() const { return -1 ; }

protected:
    const unsigned int m_width;
    const unsigned int m_height;
    
    ShadowMethod m_method ;

    id <MTLTexture> m_depthTexture;
    id <MTLTexture> m_colorTexture;
    
    id <MTLSamplerState> m_colorSampler;
    
    MTLRenderPassDescriptor* m_framebuffer;
    
    id <MTLDevice> m_Device ;
};
	
}
#endif // MTL_SHADOWMAP