/*
**
** Copyright 2009, The Android Open Source Project
**
** Licensed under the Apache License, Version 2.0 (the "License");
** you may not use this file except in compliance with the License.
** You may obtain a copy of the License at
**
**     http://www.apache.org/licenses/LICENSE-2.0
**
** Unless required by applicable law or agreed to in writing, software
** distributed under the License is distributed on an "AS IS" BASIS,
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
** See the License for the specific language governing permissions and
** limitations under the License.
*/

#ifndef ANDROID_BUFFER_ALLOCATOR_H
#define ANDROID_BUFFER_ALLOCATOR_H

#include <stdint.h>

#include <cutils/native_handle.h>

#include <utils/Errors.h>
#include <utils/KeyedVector.h>
#include <utils/threads.h>
#include <utils/Singleton.h>

#include <ui/Gralloc1.h>
#include <ui/PixelFormat.h>

namespace android {

class Gralloc1Loader;
class String8;

class GraphicBufferAllocator : public Singleton<GraphicBufferAllocator>
{
public:
    enum {
        USAGE_SW_READ_NEVER     = GRALLOC1_CONSUMER_USAGE_CPU_READ_NEVER,
        USAGE_SW_READ_RARELY    = GRALLOC1_CONSUMER_USAGE_CPU_READ,
        USAGE_SW_READ_OFTEN     = GRALLOC1_CONSUMER_USAGE_CPU_READ_OFTEN,
        USAGE_SW_READ_MASK      = GRALLOC1_CONSUMER_USAGE_CPU_READ_OFTEN,

        USAGE_SW_WRITE_NEVER    = GRALLOC1_PRODUCER_USAGE_CPU_WRITE_NEVER,
        USAGE_SW_WRITE_RARELY   = GRALLOC1_PRODUCER_USAGE_CPU_WRITE,
        USAGE_SW_WRITE_OFTEN    = GRALLOC1_PRODUCER_USAGE_CPU_WRITE_OFTEN,
        USAGE_SW_WRITE_MASK     = GRALLOC1_PRODUCER_USAGE_CPU_WRITE_OFTEN,

        USAGE_SOFTWARE_MASK     = USAGE_SW_READ_MASK|USAGE_SW_WRITE_MASK,

        USAGE_HW_TEXTURE        = GRALLOC1_CONSUMER_USAGE_GPU_TEXTURE,
        USAGE_HW_RENDER         = GRALLOC1_PRODUCER_USAGE_GPU_RENDER_TARGET,
        USAGE_HW_2D             = 0x00000400, // Deprecated
        USAGE_HW_MASK           = 0x00071F00, // Deprecated
    };

    static inline GraphicBufferAllocator& get() { return getInstance(); }

    status_t allocate(uint32_t w, uint32_t h, PixelFormat format,
            uint32_t usage, buffer_handle_t* handle, uint32_t* stride,
            uint64_t graphicBufferId, std::string requestorName);

    status_t free(buffer_handle_t handle);

    void dump(String8& res) const;
    static void dumpToSystemLog();

private:
    struct alloc_rec_t {
        uint32_t width;
        uint32_t height;
        uint32_t stride;
        PixelFormat format;
        uint32_t usage;
        size_t size;
        std::string requestorName;
    };

    static Mutex sLock;
    static KeyedVector<buffer_handle_t, alloc_rec_t> sAllocList;

    friend class Singleton<GraphicBufferAllocator>;
    GraphicBufferAllocator();
    ~GraphicBufferAllocator();

    std::unique_ptr<Gralloc1::Loader> mLoader;
    std::unique_ptr<Gralloc1::Device> mDevice;
};

// ---------------------------------------------------------------------------
}; // namespace android

#endif // ANDROID_BUFFER_ALLOCATOR_H
