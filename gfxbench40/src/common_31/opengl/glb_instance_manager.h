/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef GLB_INSTANCE_MANAGER_H
#define GLB_INSTANCE_MANAGER_H

#include "platform.h"
#include <kcl_math3d.h>
#include <vector>

namespace GLB
{
    // Currently Uniform block size equals with UBO size. We could optimize it but handling aligment could be very complicated 
    template <typename T>
    class InstanceManager
    {
    public:
        static const KCL::uint32 DEFAULT_UBLOCK_SIZE = 16384; // Min guaranteed value by ES 3.x 

        InstanceManager(KCL::uint32 max_ublock_size = DEFAULT_UBLOCK_SIZE);
        ~InstanceManager();
    
        void PreallocateBuffers(KCL::uint32 instance_count);
        void UploadInstanceData(const std::vector<T> &data);
        void UploadInstanceData(const T *data, const KCL::uint32 data_count);
        void BindInstanceBuffer(KCL::uint32 binding_point, KCL::uint32 instance_count, KCL::uint32 &offset, KCL::uint32 &max_instance_count);

        KCL::uint32 GetMaxInstanceCount() const
        {
            return m_max_instance_count;
        }
        KCL::uint32 GetInstanceBlockSize() const
        {
            return m_instance_block_size;
        }

    private:            
        const KCL::uint32 m_max_ublock_size;
        const KCL::uint32 m_max_instance_count;
        const KCL::uint32 m_instance_block_size;

        KCL::uint32 m_instance_counter;
        KCL::uint32 m_ubo_counter;

        KCL::uint32 m_current_ubo_bind;

        std::vector<KCL::uint32> m_instance_buffers;
    };

    template <typename T>
    InstanceManager<T>::InstanceManager(KCL::uint32 max_ublock_size) :
        m_max_ublock_size(max_ublock_size),
        m_max_instance_count(max_ublock_size / sizeof(T)),
        m_instance_block_size(m_max_instance_count * sizeof(T))
    {                  
        m_instance_counter = 0;
        m_ubo_counter = 0;   
        m_current_ubo_bind = 0;     
    }

    template <typename T>
    InstanceManager<T>::~InstanceManager()
    {
        for (KCL::uint32 i = 0; i < m_instance_buffers.size(); i++)
        {
            glDeleteBuffers(1, &m_instance_buffers[i]);
        }
    }

    template <typename T>
    void InstanceManager<T>::PreallocateBuffers(KCL::uint32 instance_count)
    {
        KCL::uint32 data_size = instance_count * sizeof(T);
        KCL::uint32 ubo_count = (data_size + m_max_ublock_size - 1) / m_max_ublock_size;
        KCL::uint32 current_ubo_count = m_instance_buffers.size();
        if (current_ubo_count < ubo_count)
        {
            KCL::uint32 ubo = 0;
            for (KCL::uint32 i = 0; i < ubo_count - current_ubo_count; i++)
            {
                glGenBuffers(1, &ubo);  
                glBindBuffer(GL_UNIFORM_BUFFER, ubo);
                glBufferData(GL_UNIFORM_BUFFER, m_max_ublock_size, NULL, GL_DYNAMIC_COPY);               
                m_instance_buffers.push_back(ubo);          
            }
            glBindBuffer(GL_UNIFORM_BUFFER, 0);
        }
    }

    template <typename T>
    void InstanceManager<T>::UploadInstanceData(const T *data, const KCL::uint32 data_count)
    {
        m_instance_counter = 0;
        m_ubo_counter = 0;
        m_current_ubo_bind = 0; // Reset the binding point for safety
          
        if (!data_count)
        {
            return;
        }

        KCL::uint32 ubo_counter = 0;
        KCL::uint32 current_ubo_count = m_instance_buffers.size();
        KCL::uint32 ubo = 0;
        for (KCL::uint32 offset = 0; offset < data_count; offset += m_max_instance_count)
        {            
            if (ubo_counter >= current_ubo_count)
            {
                glGenBuffers(1, &ubo);  
                glBindBuffer(GL_UNIFORM_BUFFER, ubo);
                glBufferData(GL_UNIFORM_BUFFER, m_max_ublock_size, NULL, GL_DYNAMIC_DRAW);               
                m_instance_buffers.push_back(ubo);          
            }
            else
            {
                ubo = m_instance_buffers[ubo_counter++];
                glBindBuffer(GL_UNIFORM_BUFFER, ubo);
            }

            KCL::uint32 batch_size = KCL::Min(m_max_instance_count, data_count - offset);
            glBufferSubData(GL_UNIFORM_BUFFER, 0, batch_size * sizeof(T), &data[offset]);
        }
        glBindBuffer(GL_UNIFORM_BUFFER, 0);
    }

    template <typename T>
    void InstanceManager<T>::UploadInstanceData(const std::vector<T> &data)
    {
        UploadInstanceData(data.data(), data.size());        
    }

    template <typename T>
    void InstanceManager<T>::BindInstanceBuffer(KCL::uint32 binding_point, KCL::uint32 instance_count, KCL::uint32 &offset, KCL::uint32 &max_instance_count)
    {
        KCL::uint32 ubo_id = m_instance_buffers[m_ubo_counter];
        if (m_current_ubo_bind != ubo_id)
        {
            glBindBufferBase(GL_UNIFORM_BUFFER, binding_point, ubo_id);
            m_current_ubo_bind = ubo_id;
        }

        offset = m_instance_counter;
        max_instance_count = KCL::Min(instance_count, m_max_instance_count - m_instance_counter);

        m_instance_counter += max_instance_count;
        if (m_instance_counter == m_max_instance_count)
        {
            m_instance_counter = 0;
            m_ubo_counter++;
        }
    }
} // namespace GLB

#endif