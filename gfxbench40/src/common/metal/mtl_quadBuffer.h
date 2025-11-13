/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef MTL_QUADBUFFER
#define MTL_QUADBUFFER

#include <Metal/Metal.h>

namespace MetalRender
{

class QuadBuffer
{
public:
    typedef enum
    {
        kBlitQuadPortrait,
        kBlitQuadLandscape
    } BlitQuadMode;

    QuadBuffer(BlitQuadMode blitQuadMode);
    virtual ~QuadBuffer();
    
    static MTLVertexDescriptor* GetVertexLayout() ;

    void Draw(id<MTLRenderCommandEncoder> renderEncoder);
protected:

    id <MTLBuffer> m_buffer;
    id <MTLDevice> m_Device ;
};
    
}

#endif // MTL_QUADBUFFER