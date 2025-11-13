/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */

#include "mtl_globals.h"
#include "mtl_mesh.h"
#include "mtl_factories.h"

#include "platform.h"
#include "vbopool.h"
#include "misc2.h"

#include "mtl_types.h"
#import "graphics/metalgraphicscontext.h"

using namespace MetalRender;

#ifndef EPSILON
#define EPSILON 0.0001f
#endif


GFXVertexLayout::GFXVertexLayout()
{
    m_vertex_descriptor = nil ;
}


GFXVertexLayout::~GFXVertexLayout()
{
    if (m_vertex_descriptor != nil)
    {
        releaseObj(m_vertex_descriptor) ;
    }
}


KCL::uint32 GFXVertexLayout::AttrSizeOf(KCL::uint32 type)
{
    switch (type)
    {
        case GL_FLOAT:
            return sizeof(float);
        case GL_UNSIGNED_BYTE:
            return sizeof(KCL::uint8);
        case GL_UNSIGNED_INT:
            return sizeof(KCL::uint32);
        default:
            assert(0);
            return -1 ;
    }
}


bool GFXVertexLayout::attrIsSkeletal(int attr)
{
    return (attr == BONE_INDEX) || (attr == BONE_WEIGHT) ;
}


std::string GFXVertexLayout::AttrName(int attr)
{
    switch (attr) {
        case POSITION:
            return "POSITION";
        case TEXTURE_COORD0:
            return "TEXTURE_COORD0" ;
        case TEXTURE_COORD1:
            return "TEXTURE_COORD1" ;
        case NORMAL:
            return "NORMAL";
        case TANGENT:
            return "TANGENT";
        case BONE_INDEX:
            return "BONE INDEX";
        case BONE_WEIGHT:
            return "BONE_WEIGTH";
            
        default:
            assert(0) ;
            return "Unkwown attribute";
    }
}

GFXVertexLayout Mesh3::s_vertex_layout ;
GFXVertexLayout Mesh3::s_skeletal_vertex_layout ;

const KCL::uint32 GFXVertexLayout::m_attribute_order[] = { POSITION, TEXTURE_COORD0, TEXTURE_COORD1, NORMAL, TANGENT, BONE_WEIGHT, BONE_INDEX } ;
//const KCL::uint32 GFXVertexLayout::m_attribute_order[] = { BONE_INDEX,  TANGENT, POSITION, NORMAL, TEXTURE_COORDINATE, BONE_WEIGHT} ;

const KCL::uint32 GFXVertexLayout::m_shader_order[] = { POSITION, TEXTURE_COORD0, TEXTURE_COORD1, NORMAL, TANGENT, BONE_WEIGHT, BONE_INDEX } ;

void GFXVertexLayout::InitVertexLayout(bool skeletal, KCL::SceneVersion scene_version)
{
    KCL::uint32 attribute_padding_in_bytes = -1 ;
    KCL::uint32 layout_padding_in_bytes = -1 ;
    
    KCL::uint32 normal_type = -1 ;
    
    if (scene_version == KCL::SV_27)
    {
        attribute_padding_in_bytes = 4 ;
        layout_padding_in_bytes = 4 ;
        
        normal_type = GL_UNSIGNED_BYTE ;
    }
    else if ( (scene_version == KCL::SV_30) || (scene_version == KCL::SV_31) )
    {
        // MTL_TODO
        //
        // Normals doesn't fetch if attribute padding is 1
        //
        
        attribute_padding_in_bytes = 4 ;
        layout_padding_in_bytes = 4 ;
        
        normal_type = GL_UNSIGNED_BYTE ;
    }
    else
    {
        assert(0) ;
    }
    
    
    m_skeletal = skeletal ;
    
    m_attributes[POSITION].size = 3 ;
    m_attributes[POSITION].type = GL_FLOAT ;
    
    m_attributes[TEXTURE_COORD0].size = 2 ;
    m_attributes[TEXTURE_COORD0].type = GL_FLOAT ;
    
    m_attributes[TEXTURE_COORD1].size = 2 ;
    m_attributes[TEXTURE_COORD1].type = GL_FLOAT ;
    
    m_attributes[NORMAL].size = 3 ;
    m_attributes[NORMAL].type = normal_type ;
    m_attributes[NORMAL].normalized = true ;
    
    
    m_attributes[TANGENT].size = 3 ;
    m_attributes[TANGENT].type = normal_type ;
    m_attributes[TANGENT].normalized = true ;
    
    
    m_attributes[BONE_WEIGHT].size = 4 ;
    m_attributes[BONE_WEIGHT].type = GL_FLOAT ;
    
    m_attributes[BONE_INDEX].size = 4 ;
    m_attributes[BONE_INDEX].type = GL_UNSIGNED_BYTE ;
    m_attributes[BONE_INDEX].normalized = false ;
    
    m_num_attributes = (skeletal)? 7 : 5 ;
    
    
    KCL::uint32 actual_offset = 0 ;
    
    for (int i = 0 ; i < ATTRIBUTE_COUNT ; i++)
    {
        int act_attr = m_attribute_order[ i ] ;
        
        if ( !m_skeletal && attrIsSkeletal(act_attr) ) continue ;
        
        if (actual_offset % attribute_padding_in_bytes != 0)
        {
            KCL::uint32 t = actual_offset/attribute_padding_in_bytes ;
            
            actual_offset = (t+1)*attribute_padding_in_bytes ;
        }
        
        m_attributes[act_attr].offset = actual_offset ;
        m_attributes[act_attr].size_in_bytes = m_attributes[act_attr].size * AttrSizeOf(m_attributes[act_attr].type) ;
        
        actual_offset += m_attributes[act_attr].size_in_bytes ;
    
    }
    
    m_stride = actual_offset ;
    
    if (m_stride % layout_padding_in_bytes != 0)
    {
        KCL::uint32 t = m_stride/layout_padding_in_bytes ;
        
        m_stride = (t+1)*layout_padding_in_bytes ;
    }
    
    //
    // setup shader id-s
    //
    for (int i = 0 ; i < ATTRIBUTE_COUNT ; i++)
    {
        m_attributes [ m_shader_order[i] ].shader_id = i ;
    }
    
    InitVertexDescriptor() ;
}


void GFXVertexLayout::InitVertexDescriptor()
{
    m_vertex_descriptor = [[MTLVertexDescriptor alloc] init];
    
    for (int i = 0 ; i < m_num_attributes; i++)
    {
        MTLVertexFormat format = MTLVertexFormatInvalid ;
        
        if      ( (m_attributes[i].size == 2) && (m_attributes[i].type == GL_FLOAT) )
        {
            format = MTLVertexFormatFloat2 ;
        }
        else if ( (m_attributes[i].size == 3) && (m_attributes[i].type == GL_FLOAT) )
        {
            format = MTLVertexFormatFloat3 ;
        }
        else if ( (m_attributes[i].size == 4) && (m_attributes[i].type == GL_FLOAT) )
        {
            format = MTLVertexFormatFloat4 ;
        }
        else if ( (m_attributes[i].size == 4) && (m_attributes[i].type == GL_UNSIGNED_BYTE) && !m_attributes[i].normalized )
        {
            format = MTLVertexFormatUChar4 ;
        }
        else if ( (m_attributes[i].size == 3) && (m_attributes[i].type == GL_UNSIGNED_BYTE) && m_attributes[i].normalized )
        {
            format = MTLVertexFormatChar3Normalized ;
        }
        else if ( (m_attributes[i].size == 4) && (m_attributes[i].type == GL_UNSIGNED_INT) && !m_attributes[i].normalized )
        {
            format = MTLVertexFormatInt4 ;
        }
        else
        {
            assert(0) ;
        }
        
        m_vertex_descriptor.attributes[i].format = format;
        m_vertex_descriptor.attributes[i].bufferIndex = 0;
        m_vertex_descriptor.attributes[i].offset = m_attributes[i].offset;
    }
    
    m_vertex_descriptor.layouts[0].stride = m_stride ;
    m_vertex_descriptor.layouts[0].stepFunction = MTLVertexStepFunctionPerVertex;
}


void GFXVertexLayout::DumpLayout()
{
    return;
    
    INFO("\n=========================================\n") ;
    INFO("Vertex Layout\n") ;
    
    INFO("attribute count: %d",m_num_attributes) ;
    INFO("is skeletal:     %s",m_skeletal?"true":"false") ;
    INFO("stride:          %d",m_stride) ;
    
    INFO("\nAttributes:") ;
    
    for (int i = 0 ; i < ATTRIBUTE_COUNT ; i++)
    {
        int act_attr = m_attribute_order[ i ] ;
        
        if ( !m_skeletal && attrIsSkeletal(act_attr) ) continue ;
        
        INFO("name:          %s",AttrName(act_attr).c_str()) ;
        INFO("size:          %d",m_attributes[act_attr].size) ;
        INFO("size in bytes: %d",m_attributes[act_attr].size_in_bytes) ;
        INFO("offset:        %d\n",m_attributes[act_attr].offset) ;
        
    }
    INFO("\n=========================================\n") ;
}


KCL::Mesh3* Mesh3Factory::Create(const char *name)
{
    return new MetalRender::Mesh3(name);
}


Mesh3::Mesh3(const char *name) :
KRL::Mesh3(name),
m_vertexBuffer(nil),
m_indexBuffer(),
m_indices_from_mio(false),
m_Device(MetalRender::GetContext()->getDevice())
{
    m_index_counts[0] = 0;
    m_index_counts[1] = 0;
    
    m_mio_offsets[0] = -1;
    m_mio_offsets[1] = -1;

    
#if MULTI_LOCK_MUTEX
    for (int i = 0 ; i< MAX_LOCK_COUNT ; i++ )
    {
        m_locks[i] = false ;
    }
#else
    m_lock = 0 ;
    
    m_InflightSemaphore = dispatch_semaphore_create(1);
#endif
    

}

Mesh3::~Mesh3()
{
    releaseObj(m_vertexBuffer);

	for(unsigned int i=0; i < NUM_INDEX_BUFFERS; i++)
    {
        releaseObj(m_indexBuffer[i]);
    }
    
#if !MULTI_LOCK_MUTEX
    releaseObj(m_InflightSemaphore) ;
#endif
}


void Mesh3::InitVertexLayouts(KCL::SceneVersion scene_version)
{
    s_vertex_layout.InitVertexLayout(false, scene_version) ;
    s_vertex_layout.DumpLayout() ;
    
    s_skeletal_vertex_layout.InitVertexLayout(true, scene_version) ;
    s_skeletal_vertex_layout.DumpLayout() ;
}

void Mesh3::InitVertexAttribs()
{
	assert(!m_vbo);  // Don't use m_vbo in MTL

	CalculateTangents();
    
    bool skeletal = m_vertex_matrix_indices.size() != 0 ;
    
    
    GFXVertexLayout & vertex_layout = (skeletal)?s_skeletal_vertex_layout:s_vertex_layout  ;
    
    
    //
    //  Position
    //
    m_vertex_attribs[0].m_size = vertex_layout.m_attributes[GFXVertexLayout::POSITION].size ;
    m_vertex_attribs[0].m_type = vertex_layout.m_attributes[GFXVertexLayout::POSITION].type ;
    m_vertex_attribs[0].m_data = reinterpret_cast<const void*>(vertex_layout.m_attributes[GFXVertexLayout::POSITION].offset) ;
    
    
    //
    // Texcoord 0 bumpmap & colormap texcoords
	//
    m_vertex_attribs[1].m_size = 2;
    m_vertex_attribs[1].m_type = vertex_layout.m_attributes[GFXVertexLayout::TEXTURE_COORD0].type;
    m_vertex_attribs[1].m_data = reinterpret_cast<const void*>(vertex_layout.m_attributes[GFXVertexLayout::TEXTURE_COORD0].offset);
    
    
    //
    // Texcoord 1
	//
    m_vertex_attribs[2].m_size = 2;
    m_vertex_attribs[2].m_type = vertex_layout.m_attributes[GFXVertexLayout::TEXTURE_COORD1].type;
    m_vertex_attribs[2].m_data = reinterpret_cast<const void*>(vertex_layout.m_attributes[GFXVertexLayout::TEXTURE_COORD1].offset);
    
    
    //
    //  Normal
    //
    m_vertex_attribs[3].m_size = vertex_layout.m_attributes[GFXVertexLayout::NORMAL].size ;
    m_vertex_attribs[3].m_type = vertex_layout.m_attributes[GFXVertexLayout::NORMAL].type ;
    m_vertex_attribs[3].m_data = reinterpret_cast<const void*>(vertex_layout.m_attributes[GFXVertexLayout::NORMAL].offset) ;
    
    
    //
    //  Tangent
    //
    m_vertex_attribs[4].m_size = vertex_layout.m_attributes[GFXVertexLayout::TANGENT].size ;
    m_vertex_attribs[4].m_type = vertex_layout.m_attributes[GFXVertexLayout::TANGENT].type ;
    m_vertex_attribs[4].m_data = reinterpret_cast<const void*>(vertex_layout.m_attributes[GFXVertexLayout::TANGENT].offset) ;
    
    
    if (skeletal)
    {
        //
        //  Bone Weights
        //
        m_vertex_attribs[5].m_size = vertex_layout.m_attributes[GFXVertexLayout::BONE_WEIGHT].size ;
        m_vertex_attribs[5].m_type = vertex_layout.m_attributes[GFXVertexLayout::BONE_WEIGHT].type ;
        m_vertex_attribs[5].m_data = reinterpret_cast<const void*>(vertex_layout.m_attributes[GFXVertexLayout::BONE_WEIGHT].offset) ;
        
        
        //
        //  Bone Indexes
        //
        m_vertex_attribs[6].m_size = vertex_layout.m_attributes[GFXVertexLayout::BONE_INDEX].size;
        m_vertex_attribs[6].m_type = vertex_layout.m_attributes[GFXVertexLayout::BONE_INDEX].type;
        m_vertex_attribs[6].m_data = reinterpret_cast<const void*>(vertex_layout.m_attributes[GFXVertexLayout::BONE_INDEX].offset);
    }
    
    
    for(int attribute = 0; attribute < vertex_layout.m_num_attributes; attribute++)
    {
        m_vertex_attribs[attribute].m_stride = vertex_layout.m_stride;
    }

    
    size_t vertex_stride = vertex_layout.m_stride;
    size_t numVerts =  m_vertex_attribs3[0].size();
    size_t bufferSize = numVerts * vertex_stride;

#if !TARGET_OS_EMBEDDED
    m_vertexBuffer = [m_Device newBufferWithLength:bufferSize options: MTLResourceStorageModeManaged];
#else
    m_vertexBuffer = [m_Device newBufferWithLength:bufferSize
                                              options:MTLResourceOptionCPUCacheModeDefault];
#endif


	KCL::int8* data = (KCL::int8*)[m_vertexBuffer contents];
    
	
    KCL::Vector3D default_vector_3d = KCL::Vector3D(0.0f,0.0f,0.0f) ;
    KCL::int8* default_vector_3d_ptr = (KCL::int8*)(&default_vector_3d) ;
    
    KCL::Vector2D default_vector_2d = KCL::Vector2D(0.0f,0.0f) ;
    KCL::int8* default_vector_2d_ptr = (KCL::int8*)(&default_vector_2d) ;
    

	for(unsigned int i=0; i < numVerts; ++i)
	{
        KCL::int8* vert_dst = data + i * vertex_stride;
    
         
        KCL::int8* src_ptrs[GFXVertexLayout::ATTRIBUTE_COUNT] ;
        
        src_ptrs[GFXVertexLayout::POSITION]           = (m_vertex_attribs3[0].size() != 0)?(KCL::int8*)(&m_vertex_attribs3[0][i]):nullptr ;
        src_ptrs[GFXVertexLayout::TEXTURE_COORD0]     = (m_vertex_attribs2[0].size() != 0)?(KCL::int8*)(&m_vertex_attribs2[0][i]):default_vector_2d_ptr ;
        src_ptrs[GFXVertexLayout::TEXTURE_COORD1]     = (m_vertex_attribs2[1].size() != 0)?(KCL::int8*)(&m_vertex_attribs2[1][i]):default_vector_2d_ptr ;
        src_ptrs[GFXVertexLayout::NORMAL]             = (m_vertex_attribs3[1].size() != 0)?(KCL::int8*)(&m_vertex_attribs3[1][i]):default_vector_3d_ptr ;
        src_ptrs[GFXVertexLayout::TANGENT]            = (m_vertex_attribs3[2].size() != 0)?(KCL::int8*)(&m_vertex_attribs3[2][i]):default_vector_3d_ptr ;
        src_ptrs[GFXVertexLayout::BONE_WEIGHT]        = nullptr ;
        src_ptrs[GFXVertexLayout::BONE_INDEX]         = nullptr ;
        
        if (vertex_layout.m_skeletal)
        {
            src_ptrs[GFXVertexLayout::BONE_WEIGHT] = (m_vertex_attribs4[0].size() != 0)?(KCL::int8*)(&m_vertex_attribs4[0][i]):nullptr ;
            src_ptrs[GFXVertexLayout::BONE_INDEX]  = (m_vertex_matrix_indices.size() != 0) ? (KCL::int8*)(&m_vertex_matrix_indices[0]) + i * 4 * sizeof(KCL::uint8) : nullptr ;
        }
        
        
        for (int j = 0 ; j < vertex_layout.m_num_attributes; j++)
        {
            enum {
                SIMPLE_COPY,
                FLOAT_3_TO_BYTE,
                UNKNOWN
            } ;
            
            int copy_method = UNKNOWN ;
            
            switch (j) {
                case GFXVertexLayout::NORMAL:
                case GFXVertexLayout::TANGENT:
                    copy_method = (vertex_layout.m_attributes[j].type == GL_FLOAT)?SIMPLE_COPY:FLOAT_3_TO_BYTE ;
                    break;
                    
                default:
                    copy_method = SIMPLE_COPY ;
                    break;
            }
            
            
            KCL::int8* attr_dst = vert_dst + vertex_layout.m_attributes[j].offset ;
            
            if (copy_method == SIMPLE_COPY)
            {
                
                if (src_ptrs[j] == nullptr) assert(0) ;
                memcpy(attr_dst, src_ptrs[j], vertex_layout.m_attributes[j].size_in_bytes);
            }
            else if (copy_method == FLOAT_3_TO_BYTE)
            {
                if (src_ptrs[j] == nullptr) assert(0) ;
                
                KCL::Vector3D* v = (KCL::Vector3D*)(src_ptrs[j]);
                
                attr_dst[0] = (KCL::int8)(v->x * 127.0f);
                attr_dst[1] = (KCL::int8)(v->y * 127.0f);
                attr_dst[2] = (KCL::int8)(v->z * 127.0f);
            }
            else
            {
                assert(0) ;
            }
            
        }
    }

#if !TARGET_OS_EMBEDDED
    [m_vertexBuffer didModifyRange: NSMakeRange(0, bufferSize)];
#endif
    
	DeleteVertexAttribs();

	for(unsigned int i=0; i < NUM_INDEX_BUFFERS; i++)
    {
        assert(!m_ebo[i].m_buffer);

       // KCL::uint32 index_count = static_cast<KCL::uint32>(m_vertex_indices[i].size());
        assert(m_vertex_indices[i].size() % 3 == 0);
        
        bool was_invalid_triangle = false ;
        size_t triangle_count = m_vertex_indices[i].size()/3 ;
        
        //
        //  Collect invalid triangles
        //
        for (int j = 0; j < triangle_count ; j++ )
        {
            KCL::uint16 id1 = m_vertex_indices[i][3*j+0] ;
            KCL::uint16 id2 = m_vertex_indices[i][3*j+1] ;
            KCL::uint16 id3 = m_vertex_indices[i][3*j+2] ;
            
            bool id1_inv = id1 >= numVerts;
            bool id2_inv = id2 >= numVerts;
            bool id3_inv = id3 >= numVerts;
            
            bool invalid = id1_inv || id2_inv || id3_inv ;
            
            was_invalid_triangle |= invalid ;
        }
        
        
        //
        //  use new indices buffer if needed
        //
        if (was_invalid_triangle)
        {
            //printf("ERROR!! invalid triangles found: %s\n", m_name.c_str()) ;
        }
        
        
        bufferSize = m_vertex_indices[i].size() * sizeof(KCL::uint16);

#if !TARGET_OS_EMBEDDED
        m_indexBuffer[i] = [m_Device newBufferWithLength:bufferSize options: MTLResourceStorageModeManaged];
#else
        m_indexBuffer[i] = [m_Device newBufferWithLength:bufferSize
                                                    options:MTLResourceOptionCPUCacheModeDefault];
#endif
        
        memcpy([m_indexBuffer[i] contents], (const void*)(&m_vertex_indices[i][0]), bufferSize);

		m_index_counts[i] = static_cast<uint32_t>(m_vertex_indices[i].size());
        //printf("Mesh %s indexBuffer %u, %lu verts, %d indices\n", m_name.c_str(), i, numVerts, m_index_counts[i]);
        m_vertex_indices[i].clear();
        
#if !TARGET_OS_EMBEDDED
        [m_indexBuffer[i] didModifyRange: NSMakeRange(0, bufferSize)];
#endif
    }
}

#if MULTI_LOCK_MUTEX

void Mesh3::LockIndexBuffer(int lock_id)
{
    assert( !m_locks[lock_id] ) ;
    
    m_locks[lock_id] = true ;
}

void Mesh3::UnlockIndexBuffer(int lock_id)
{
    assert( m_locks[lock_id] ) ;
    
    m_locks[lock_id] = false ;
}

bool Mesh3::isLocked()
{
    for (int i = 0 ; i < MAX_LOCK_COUNT; i++)
    {
        if (m_locks[i]) return true ;
    }
    return false ;
}

void Mesh3::UpdateIndexBuffer(size_t size, const void* data, int buffer_id)
{
    
    while (isLocked()) {
        //NSLog(@"Wait for indexbuffer unlock, multi lock mutex") ;
    }
    
    memcpy([m_indexBuffer[buffer_id] contents], data, size);
    
    assert(!isLocked()) ;
}


#else

void Mesh3::LockIndexBuffer(int )
{
    dispatch_semaphore_wait(m_InflightSemaphore, DISPATCH_TIME_FOREVER);
    
    assert(m_lock >= 0) ;
    m_lock++ ;
    
    dispatch_semaphore_signal(m_InflightSemaphore);
}

void Mesh3::UnlockIndexBuffer(int )
{
    dispatch_semaphore_wait(m_InflightSemaphore, DISPATCH_TIME_FOREVER);
    
    assert(m_lock > 0) ;
    m_lock-- ;
    
    dispatch_semaphore_signal(m_InflightSemaphore);
}

void Mesh3::UpdateIndexBuffer(size_t size, const void* data, int buffer_id)
{
    while(true)
    {
        dispatch_semaphore_wait(m_InflightSemaphore, DISPATCH_TIME_FOREVER);
        
        assert(m_lock >= 0) ;
        
        if (m_lock == 0)
        {
            memcpy([m_indexBuffer[buffer_id] contents], data, size);
        
            assert(m_lock == 0) ;
            dispatch_semaphore_signal(m_InflightSemaphore);
            break ;
        }
        
        //NSLog(@"Wait for indexbuffer unlock, semaphore mutex") ;
        dispatch_semaphore_signal(m_InflightSemaphore);
    }
    
}

#endif


