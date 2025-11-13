/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "kcl_mesh.h"
#include "mtl_lightbuffer.h"
#include "mtl_globals.h"
#include <string.h>

#include "mtl_types.h"
#import "graphics/metalgraphicscontext.h"

using namespace MetalRender;

LightBuffer::LightBuffer(Shape shape) :
m_Device(MetalRender::GetContext()->getDevice())
{
    std::vector<KCL::Vector3D> vertices;
    std::vector<KCL::Vector2D> texcoords;
    std::vector<KCL::uint16> indices;
    switch (shape) {
        case CONE:
        {
            printf("CONE ");
            KCL::Mesh3::CreateCone( vertices, texcoords, indices, 15, 1);
            break;
        }
        case SPHERE:
        {
            printf("SHERE ");
            KCL::Mesh3::CreateSphere( vertices, texcoords, indices, 10, 10);
            break;
        }
        default:
            assert(!"Unsupported LBO shape");
            break;
    }


    NSUInteger vertexBufferSize = sizeof(KCL::Vector3D) * vertices.size();
#if !TARGET_OS_EMBEDDED
    m_vertexBuffer = [m_Device newBufferWithLength:vertexBufferSize options: MTLResourceStorageModeManaged];
#else
    m_vertexBuffer = [m_Device newBufferWithLength:vertexBufferSize
                                              options:MTLResourceOptionCPUCacheModeDefault];
#endif

    memcpy([m_vertexBuffer contents], vertices[0].v, vertexBufferSize);
#if !TARGET_OS_EMBEDDED
    [m_vertexBuffer didModifyRange:NSMakeRange(0, vertexBufferSize)];
#endif

    NSUInteger indexBufferSize = sizeof(KCL::uint16) * indices.size();
#if !TARGET_OS_EMBEDDED
    m_indexBuffer = [m_Device newBufferWithLength:indexBufferSize options: MTLResourceStorageModeManaged];
#else
    m_indexBuffer = [m_Device newBufferWithLength:indexBufferSize
                                             options:MTLResourceOptionCPUCacheModeDefault];
#endif

    KCL::uint16 maxVal = 0;
    for(int i = 0; i < indices.size(); i++)
    {
        maxVal = (indices[i] > maxVal) ? indices[i]: maxVal;
    }
    printf("NumVerts:%lu NunIndices %lu MaxIndexValue %d\n", vertices.size(), indices.size(), maxVal);

    memcpy([m_indexBuffer contents], &indices[0], indexBufferSize);

#if !TARGET_OS_EMBEDDED
    [m_indexBuffer didModifyRange:NSMakeRange(0, indexBufferSize)];
#endif
    
    m_numIndices = indices.size();
}


LightBuffer::~LightBuffer()
{
    releaseObj(m_vertexBuffer);
    releaseObj(m_indexBuffer);
}
