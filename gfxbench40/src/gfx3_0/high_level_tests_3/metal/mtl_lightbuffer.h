/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef MTL_LIGHTBUFFER_H
#define MTL_LIGHTBUFFER_H
#include <Metal/Metal.h>

namespace MetalRender
{

class LightBuffer
{
public:

    typedef enum
    {
        SPHERE,
        CONE,
        NUM_SHAPES
    } Shape;

    LightBuffer(Shape shape);
    virtual ~LightBuffer();

    void Draw(id <MTLRenderCommandEncoder> renderEncoder)
    {
        [renderEncoder setVertexBuffer:m_vertexBuffer offset:0 atIndex:0];

        [renderEncoder drawIndexedPrimitives:MTLPrimitiveTypeTriangle
                                  indexCount:m_numIndices
                                   indexType:MTLIndexTypeUInt16
                                 indexBuffer:m_indexBuffer
                           indexBufferOffset:0
                               instanceCount:1];

    }
    
    void DrawInstanced(id <MTLRenderCommandEncoder> renderEncoder, unsigned int instanceCount)
    {
        [renderEncoder setVertexBuffer:m_vertexBuffer offset:0 atIndex:0];
        
        [renderEncoder drawIndexedPrimitives:MTLPrimitiveTypeTriangle
                                  indexCount:m_numIndices
                                   indexType:MTLIndexTypeUInt16
                                 indexBuffer:m_indexBuffer
                           indexBufferOffset:0
                               instanceCount:instanceCount];
        
    }
    
protected:
	id <MTLBuffer> m_vertexBuffer;
	id <MTLBuffer> m_indexBuffer;
	NSUInteger m_numIndices;
    
    id <MTLDevice> m_Device ;
};

}

#endif // MTL_LIGHTBUFFER_H