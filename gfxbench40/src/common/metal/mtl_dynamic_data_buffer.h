/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef MTL_DYNAMICDATABUFFER_H
#define MTL_DYNAMICDATABUFFER_H
#include <Metal/Metal.h>
#include "mtl_types.h"

#import "graphics/metalgraphicscontext.h"

namespace MetalRender
{


#if TARGET_OS_IPHONE
#define DATA_BUFFER_PADDING 16
#else
#define DATA_BUFFER_PADDING 256
#endif


class DynamicDataBufferPool ;

    
class DynamicDataBuffer
{
public:
    static const unsigned int MAX_SLOT_COUNT = 128 ;
    
    friend class DynamicDataBufferPool ;
    
    inline size_t ReserveAndGetOffset(size_t size)
    {
        assert((m_currentOffset + size) <= [m_buffer[m_currentSlot] length]);
        size_t retOffset = m_currentOffset;
        
        if(size % DATA_BUFFER_PADDING)
        {
            size += DATA_BUFFER_PADDING - (size % DATA_BUFFER_PADDING);
        }
        
        m_currentOffset += size;
        assert(m_currentOffset%DATA_BUFFER_PADDING == 0);
        return retOffset;
    }
    
	template<bool SetVertexBuffer, bool SetFragmentBuffer>
    inline size_t WriteAndSetData(id <MTLRenderCommandEncoder> renderEncoder, int index, const void* data, size_t size)
    {
        // Assert if there is not enough space in the buffer to store the data
        assert((m_currentOffset + size) <= [m_buffer[m_currentSlot] length]);

        char* dst = (char*)[m_buffer[m_currentSlot] contents] + m_currentOffset;

        memcpy(dst, data, size);
        
        if(index >= 0)
        {
			if(SetVertexBuffer)
			{
	            [renderEncoder setVertexBuffer:m_buffer[m_currentSlot] offset:m_currentOffset atIndex:index];
			}
			if(SetFragmentBuffer)
			{
	            [renderEncoder setFragmentBuffer:m_buffer[m_currentSlot] offset:m_currentOffset atIndex:index];
			}
        }

        // Offset must always be 256 byte aligned according to docs (and debug wrapper)
        if(size % DATA_BUFFER_PADDING)
        {
            size += DATA_BUFFER_PADDING - (size % DATA_BUFFER_PADDING);
        }

#if !TARGET_OS_EMBEDDED
        if(m_buffer[m_currentSlot].storageMode & MTLStorageModeManaged)
		{
			size_t end = std::min<size_t>([m_buffer[m_currentSlot] length],size);
			[m_buffer[m_currentSlot] didModifyRange:NSMakeRange(m_currentOffset, end)];
		}
#endif

        m_currentOffset += size;

        assert(m_currentOffset%DATA_BUFFER_PADDING == 0);
        
        return size;
    }

    inline size_t WriteDataAndGetOffset(id <MTLRenderCommandEncoder> renderEncoder,
                                               const void* data, size_t size)
    {
        // Assert if there is not enough space in the buffer to store the data
        assert((m_currentOffset + size) <= [m_buffer[m_currentSlot] length]);
        size_t retOffset = m_currentOffset;
        char* dst = (char*)[m_buffer[m_currentSlot] contents] + m_currentOffset;

        memcpy(dst, data, size);

        // Offset must always be 256 byte aligned according to docs (and debug wrapper)
        if(size % DATA_BUFFER_PADDING)
        {
            size += DATA_BUFFER_PADDING - (size % DATA_BUFFER_PADDING);
        }

#if !TARGET_OS_EMBEDDED
        if(m_buffer[m_currentSlot].storageMode & MTLStorageModeManaged)
		{
			size_t end = std::min<size_t>([m_buffer[m_currentSlot] length],size);
            [m_buffer[m_currentSlot] didModifyRange:NSMakeRange(m_currentOffset, end)];
		}
#endif

        m_currentOffset += size;

        assert(m_currentOffset%DATA_BUFFER_PADDING == 0);

        return retOffset;
    }

    inline void DrawWithIndicesAtOffset(id <MTLRenderCommandEncoder> renderEncoder, size_t offset, uint32_t indexCount, MTLIndexType indexType)
    {
        [renderEncoder drawIndexedPrimitives:MTLPrimitiveTypeTriangle
                                  indexCount:indexCount
                                   indexType:indexType
                                 indexBuffer:m_buffer[m_currentSlot]
                           indexBufferOffset:offset];
    }
    
    id <MTLBuffer> GetCurrentBuffer()
    {
        return m_buffer[m_currentSlot];
    }

protected:
    
    DynamicDataBuffer(unsigned int slot_count, size_t buffer_size) ;
    virtual ~DynamicDataBuffer();
	DynamicDataBuffer(unsigned int slot_count, size_t buffer_size, MTLResourceOptions rsc_options);
	
	void Init(unsigned int slot_count, size_t buffer_size, MTLResourceOptions rsc_options);
    
    void SetNewSlot(unsigned int slot_id)
    {
        assert(slot_id <= MAX_SLOT_COUNT) ;
        
        m_currentSlot = slot_id ;
        m_currentOffset = 0 ;
    }


    id <MTLBuffer> m_buffer[MAX_SLOT_COUNT] ;
    
    unsigned char m_currentSlot;
    size_t m_currentOffset;
    
    id <MTLDevice> m_Device ;
};
    
    
    
class DynamicDataBufferPool
{
public:
    DynamicDataBufferPool(unsigned int slot_count) ;
    virtual ~DynamicDataBufferPool() ;
    
    void InitFrame() ;
    
    DynamicDataBuffer* GetNewBuffer(size_t buffer_size) ;
    DynamicDataBuffer* GetNewBuffer(size_t buffer_size, MTLResourceOptions rsc_options);
    
    
    inline void MarkSlotUnused(unsigned char slot)
    {
        assert(slot < m_slot_count) ;
        
        m_slotInUse[slot] = false;
#if TARGET_OS_EMBEDDED
        dispatch_semaphore_signal(m_semaphore);
#endif
    }

    inline unsigned char GetCurrentSlot() const
    {
        return m_currentSlot;
    }
    
    
private:
    unsigned int m_slot_count;
    unsigned int m_currentSlot;
    
    volatile bool* m_slotInUse ;
    
    std::vector<DynamicDataBuffer*> m_data_buffers ;
    
#if TARGET_OS_EMBEDDED
    dispatch_semaphore_t m_semaphore;
#endif
};

}

#endif //MTL_DYNAMICDATABUFFER_H
