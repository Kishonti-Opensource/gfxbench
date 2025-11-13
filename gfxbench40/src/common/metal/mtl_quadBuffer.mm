/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "mtl_quadBuffer.h"
#include "mtl_globals.h"

#include "mtl_types.h"
#import "graphics/metalgraphicscontext.h"

using namespace MetalRender;

QuadBuffer::QuadBuffer(BlitQuadMode blitQuadMode) :
m_buffer(nil),
m_Device(MetalRender::GetContext()->getDevice())
{
    static const float portraitQuadVerts[16] =
    {
        // Pos     //Texcoords
        -1.0,-1.0, 0.0, 0.0,
        1.0,-1.0, 0.0, 1.0,
        -1.0, 1.0, 1.0, 0.0,
        1.0, 1.0, 1.0, 1.0,
    };
    
    static const float landscapeQuadVerts[16] =
    {
        // Pos     //Texcoords
        -1.0,-1.0, 0.0, 1.0,
        1.0,-1.0, 1.0, 1.0,
        -1.0, 1.0, 0.0, 0.0,
        1.0, 1.0, 1.0, 0.0,
    };
    
    assert(sizeof(landscapeQuadVerts) == sizeof(portraitQuadVerts));
    
#if !TARGET_OS_EMBEDDED
    m_buffer = [m_Device newBufferWithLength:sizeof(portraitQuadVerts) options: MTLResourceStorageModeManaged];
#else
    m_buffer = [m_Device newBufferWithLength:sizeof(portraitQuadVerts) options:MTLResourceOptionCPUCacheModeDefault];
#endif
    
    if(blitQuadMode == kBlitQuadLandscape)
    {
        memcpy([m_buffer contents], landscapeQuadVerts, sizeof(landscapeQuadVerts));
    }
    else
    {
        memcpy([m_buffer contents], portraitQuadVerts, sizeof(portraitQuadVerts));
    }
    
#if !TARGET_OS_EMBEDDED
    [m_buffer didModifyRange:NSMakeRange(0, sizeof(portraitQuadVerts))];
#endif
}


MTLVertexDescriptor* QuadBuffer::GetVertexLayout()
{
    NSUInteger interleavedBufferStride = 4 * sizeof(float);
    MTLVertexDescriptor* vertexDesc = [[MTLVertexDescriptor alloc] init];
    
    
    vertexDesc.attributes[0].format = MTLVertexFormatFloat2;
    vertexDesc.attributes[0].bufferIndex = 0;
    vertexDesc.attributes[0].offset = 0;
    vertexDesc.attributes[1].format = MTLVertexFormatFloat2;
    vertexDesc.attributes[1].bufferIndex = 0;
    vertexDesc.attributes[1].offset = 2 * sizeof(float);  // 8 bytes
    
    vertexDesc.layouts[0].stride = interleavedBufferStride ;
    vertexDesc.layouts[0].stepFunction = MTLVertexStepFunctionPerVertex;
    
    return vertexDesc ;
}


QuadBuffer::~QuadBuffer()
{
    releaseObj(m_buffer);
}


void QuadBuffer::Draw(id<MTLRenderCommandEncoder> renderEncoder)
{
    [renderEncoder setVertexBuffer:m_buffer offset:0 atIndex:0];

    [renderEncoder drawPrimitives:MTLPrimitiveTypeTriangleStrip vertexStart:0 vertexCount:4 instanceCount:1];
}
