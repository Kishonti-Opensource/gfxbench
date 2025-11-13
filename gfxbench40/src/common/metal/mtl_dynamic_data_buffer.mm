/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "mtl_dynamic_data_buffer.h"
#include "mtl_globals.h"

using namespace MetalRender;

DynamicDataBuffer::DynamicDataBuffer(unsigned int slot_count, size_t buffer_size) :
m_buffer(),
m_currentSlot(0),
m_currentOffset(0),
m_Device(MetalRender::GetContext()->getDevice())
{
#if !TARGET_OS_EMBEDDED
	Init(slot_count, buffer_size, MTLResourceStorageModeManaged);
#else
	Init(slot_count, buffer_size, MTLResourceOptionCPUCacheModeDefault);
#endif
}


DynamicDataBuffer::DynamicDataBuffer(unsigned int slot_count, size_t buffer_size, MTLResourceOptions rsc_options) :
m_buffer(),
m_currentSlot(0),
m_currentOffset(0),
m_Device(MetalRender::GetContext()->getDevice())
{
	Init(slot_count, buffer_size, rsc_options);
}


void DynamicDataBuffer::Init(unsigned int slot_count, size_t buffer_size, MTLResourceOptions rsc_options)
{
	for(int i = 0; i < MAX_SLOT_COUNT; i++)
	{
		m_buffer[i] = nil ;
	}
	
	for(int bufNum = 0; bufNum < slot_count; bufNum++)
	{
		m_buffer[bufNum] = [m_Device newBufferWithLength:buffer_size options:rsc_options];
	}
}


DynamicDataBuffer::~DynamicDataBuffer()
{
    for(int bufNum = 0; bufNum < MAX_SLOT_COUNT; bufNum++)
    {
        if (m_buffer[bufNum] != nil) releaseObj(m_buffer[bufNum]);
    }
}



//
//
//  Dynamic Data Buffer Pool
//
//


DynamicDataBufferPool::DynamicDataBufferPool(unsigned int slot_count) : m_slot_count(slot_count), m_currentSlot(0)
{
    assert(slot_count <= DynamicDataBuffer::MAX_SLOT_COUNT) ;
    
    m_data_buffers.clear() ;
    
    m_slotInUse = new bool[slot_count] ;
    
    for (int i = 0 ; i < slot_count; i++) m_slotInUse[i] = false ;
    
#if TARGET_OS_EMBEDDED
    m_semaphore = dispatch_semaphore_create(slot_count);
#endif
}


DynamicDataBufferPool::~DynamicDataBufferPool()
{
    for (auto it = m_data_buffers.begin(); it != m_data_buffers.end() ; it++)
    {
        delete *it ;
    }
    
    m_data_buffers.clear() ;
    
    delete[] m_slotInUse ;
}


DynamicDataBuffer* DynamicDataBufferPool::GetNewBuffer(size_t buffer_size)
{
    DynamicDataBuffer* r = new DynamicDataBuffer(m_slot_count, buffer_size) ;
	m_data_buffers.push_back(r) ;
	return r ;
}


DynamicDataBuffer* DynamicDataBufferPool::GetNewBuffer(size_t buffer_size, MTLResourceOptions rsc_options)
{
    DynamicDataBuffer* r = new DynamicDataBuffer(m_slot_count, buffer_size, rsc_options) ;
    
    m_data_buffers.push_back(r) ;
    
    return r ;
}


void DynamicDataBufferPool::InitFrame()
{
#if TARGET_OS_EMBEDDED
    dispatch_semaphore_wait(m_semaphore, DISPATCH_TIME_FOREVER);
#endif
    
	m_currentSlot = (m_currentSlot+1) % m_slot_count;
    
#if !TARGET_OS_EMBEDDED
	while(m_slotInUse[m_currentSlot])
	{
		usleep(10);
	}
#endif

    if(m_slotInUse[m_currentSlot])
    {
        printf("ERROR!! Next slot is still used!");
        assert(0);
    }

    m_slotInUse[m_currentSlot] = true;
	for (auto it = m_data_buffers.begin() ; it != m_data_buffers.end() ; it++)
	{
		(*it)->SetNewSlot(m_currentSlot) ;
	}
}

