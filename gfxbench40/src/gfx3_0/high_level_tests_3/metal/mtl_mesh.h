/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef MTL_MESH_H
#define MTL_MESH_H

#include <krl_mesh.h>
#include <kcl_base.h>
#include <kcl_mesh.h>
#include <kcl_material.h>
#include <kcl_node.h>
#include <kcl_aabb.h>
#include <kcl_scene_version.h>

#include <string>
#include <vector>
#include <set>

#include "platform.h"
#include <Metal/Metal.h>

#include <string>
#include <vector>
#include <set>

#include "platform.h"
#include "mtl_globals.h"


#define MULTI_LOCK_MUTEX 1

namespace MetalRender
{
    
    
    class GFXAttribute
    {
    public:
        KCL::uint32 size ;
        KCL::uint32 size_in_bytes ;
        KCL::uint32 offset ;
        KCL::uint32 type ;
        bool normalized ;
        KCL::uint32 shader_id ;
        
        GFXAttribute() : size(0), offset(0), type(0), normalized(false), shader_id(0) { } ;
    };
    
    
    class GFXVertexLayout
    {
    public:
        GFXVertexLayout() ;
        ~GFXVertexLayout() ;
        
        enum
        {
            POSITION = 0,
            TEXTURE_COORD0,
            TEXTURE_COORD1,
            NORMAL,
            TANGENT,
            BONE_WEIGHT,
            BONE_INDEX,
            ATTRIBUTE_COUNT
        };
        
        GFXAttribute m_attributes[ATTRIBUTE_COUNT] ;
        
        KCL::uint32 m_stride ;
        KCL::uint32 m_num_attributes ;
        
        bool m_skeletal ;
        
        static const KCL::uint32 m_attribute_order[] ;
        static const KCL::uint32 m_shader_order[]  ;
        
        MTLVertexDescriptor* m_vertex_descriptor ;
        
        void InitVertexLayout(bool skeletal, KCL::SceneVersion scene_version) ;
        void DumpLayout() ;
        void InitVertexDescriptor() ;
        
        static KCL::uint32 AttrSizeOf(KCL::uint32 type) ;
        static std::string AttrName(int attr) ;
        static bool attrIsSkeletal(int attr) ;
    };
    
    
	class Mesh3 : public KRL::Mesh3
	{
		friend class Factory;

        enum { NUM_INDEX_BUFFERS = 2 };

	public:

#if MULTI_LOCK_MUTEX
        static const int MAX_LOCK_COUNT = 1024 ;
#endif
        
        Mesh3(const char *name);
        virtual ~Mesh3();

        void InitVertexAttribs();

        inline void SetVertexBuffer(id <MTLRenderCommandEncoder> renderEncoder)
        {
            [renderEncoder setVertexBuffer:m_vertexBuffer offset:0 atIndex:0];
        }

        
        inline void Draw(id <MTLRenderCommandEncoder> renderEncoder, char lod, unsigned long num_instances, KCL::uint32 indexCount)
        {
            assert(lod < NUM_INDEX_BUFFERS);
            
            [renderEncoder drawIndexedPrimitives:MTLPrimitiveTypeTriangle
                                      indexCount:indexCount
                                       indexType:MTLIndexTypeUInt16
                                     indexBuffer:m_indexBuffer[lod]
                               indexBufferOffset:0
                                   instanceCount:num_instances];
        }
        
        
        inline void Draw(id <MTLRenderCommandEncoder> renderEncoder, char lod, unsigned long num_instances)
        {
            Draw(renderEncoder,lod,num_instances, getIndexCount(lod)) ;
        }
        
        void UpdateIndexBuffer(size_t size, const void* data, int buffer_id) ;

        
        void LockIndexBuffer(int lock_id) ;
        void UnlockIndexBuffer(int lock_id) ;

#if MULTI_LOCK_MUTEX
        bool isLocked() ;
#endif
        
        bool m_indices_from_mio ;
        size_t m_mio_offsets[NUM_INDEX_BUFFERS] ;

        static void InitVertexLayouts(KCL::SceneVersion scene_version) ;
        
        static GFXVertexLayout s_skeletal_vertex_layout ;
        static GFXVertexLayout s_vertex_layout ;
        
    protected:
        id <MTLBuffer> m_vertexBuffer;
        id <MTLBuffer> m_indexBuffer[NUM_INDEX_BUFFERS];
        
        id <MTLDevice> m_Device ;
        
#if MULTI_LOCK_MUTEX
        volatile bool m_locks[MAX_LOCK_COUNT+1] ;
#else
        volatile int m_lock ;
        dispatch_semaphore_t m_InflightSemaphore;
#endif
        
	};
}

#endif // MTL_MESH_H
