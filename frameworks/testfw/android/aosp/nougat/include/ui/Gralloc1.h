/*
 * Copyright 2016 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef ANDROID_UI_GRALLOC1_H
#define ANDROID_UI_GRALLOC1_H

#define GRALLOC1_LOG_TAG "Gralloc1"

#include <ui/Gralloc1On0Adapter.h>

#include <unordered_set>

namespace std {
    template <>
    struct hash<gralloc1_capability_t> {
        size_t operator()(gralloc1_capability_t capability) const {
            return std::hash<int32_t>()(static_cast<int32_t>(capability));
        }
    };
}

namespace android {

class Fence;
class GraphicBuffer;

namespace Gralloc1 {

class Device;

class Descriptor {
public:
    Descriptor(Device& device, gralloc1_buffer_descriptor_t deviceId)
      : mShimDevice(device),
        mDeviceId(deviceId),
        mWidth(0),
        mHeight(0),
        mFormat(static_cast<android_pixel_format_t>(0)),
        mProducerUsage(GRALLOC1_PRODUCER_USAGE_NONE),
        mConsumerUsage(GRALLOC1_CONSUMER_USAGE_NONE) {}

    ~Descriptor();

    gralloc1_buffer_descriptor_t getDeviceId() const { return mDeviceId; }

    gralloc1_error_t setDimensions(uint32_t width, uint32_t height);
    gralloc1_error_t setFormat(android_pixel_format_t format);
    gralloc1_error_t setProducerUsage(gralloc1_producer_usage_t usage);
    gralloc1_error_t setConsumerUsage(gralloc1_consumer_usage_t usage);

private:
    Device& mShimDevice;
    const gralloc1_buffer_descriptor_t mDeviceId;

    uint32_t mWidth;
    uint32_t mHeight;
    android_pixel_format_t mFormat;
    gralloc1_producer_usage_t mProducerUsage;
    gralloc1_consumer_usage_t mConsumerUsage;

}; // Descriptor

class Device {
    friend class Gralloc1::Descriptor;

public:
    Device(gralloc1_device_t* device);

    bool hasCapability(gralloc1_capability_t capability) const;

    std::string dump();

    std::shared_ptr<Descriptor> createDescriptor();

    gralloc1_error_t getStride(buffer_handle_t buffer, uint32_t* outStride);

    gralloc1_error_t allocate(
            const std::vector<std::shared_ptr<const Descriptor>>& descriptors,
            std::vector<buffer_handle_t>* outBuffers);
    gralloc1_error_t allocate(
            const std::shared_ptr<const Descriptor>& descriptor,
            gralloc1_backing_store_t id, buffer_handle_t* outBuffer);

    gralloc1_error_t retain(buffer_handle_t buffer);
    gralloc1_error_t retain(const GraphicBuffer* buffer);

    gralloc1_error_t release(buffer_handle_t buffer);

    gralloc1_error_t getNumFlexPlanes(buffer_handle_t buffer,
            uint32_t* outNumPlanes);

    gralloc1_error_t lock(buffer_handle_t buffer,
            gralloc1_producer_usage_t producerUsage,
            gralloc1_consumer_usage_t consumerUsage,
            const gralloc1_rect_t* accessRegion, void** outData,
            const sp<Fence>& acquireFence);
    gralloc1_error_t lockFlex(buffer_handle_t buffer,
            gralloc1_producer_usage_t producerUsage,
            gralloc1_consumer_usage_t consumerUsage,
            const gralloc1_rect_t* accessRegion,
            struct android_flex_layout* outData, const sp<Fence>& acquireFence);
    gralloc1_error_t lockYCbCr(buffer_handle_t buffer,
            gralloc1_producer_usage_t producerUsage,
            gralloc1_consumer_usage_t consumerUsage,
            const gralloc1_rect_t* accessRegion, struct android_ycbcr* outData,
            const sp<Fence>& acquireFence);

    gralloc1_error_t unlock(buffer_handle_t buffer, sp<Fence>* outFence);

private:
    std::unordered_set<gralloc1_capability_t> loadCapabilities();

    bool loadFunctions();

    template <typename LockType, typename OutType>
    gralloc1_error_t lockHelper(LockType pfn, buffer_handle_t buffer,
            gralloc1_producer_usage_t producerUsage,
            gralloc1_consumer_usage_t consumerUsage,
            const gralloc1_rect_t* accessRegion, OutType* outData,
            const sp<Fence>& acquireFence) {
        int32_t intError = pfn(mDevice, buffer,
                static_cast<uint64_t>(producerUsage),
                static_cast<uint64_t>(consumerUsage), accessRegion, outData,
                acquireFence->dup());
        return static_cast<gralloc1_error_t>(intError);
    }

    gralloc1_device_t* const mDevice;

    const std::unordered_set<gralloc1_capability_t> mCapabilities;

    template <typename PFN, gralloc1_function_descriptor_t descriptor>
    struct FunctionLoader {
        FunctionLoader() : pfn(nullptr) {}

        bool load(gralloc1_device_t* device, bool errorIfNull) {
            gralloc1_function_pointer_t rawPointer =
                    device->getFunction(device, descriptor);
            pfn = reinterpret_cast<PFN>(rawPointer);
            if (errorIfNull && !rawPointer) {
                ALOG(LOG_ERROR, GRALLOC1_LOG_TAG,
                        "Failed to load function pointer %d", descriptor);
            }
            return rawPointer != nullptr;
        }

        template <typename ...Args>
        typename std::result_of<PFN(Args...)>::type operator()(Args... args) {
            return pfn(args...);
        }

        PFN pfn;
    };

    // Function pointers
    struct Functions {
        FunctionLoader<GRALLOC1_PFN_DUMP, GRALLOC1_FUNCTION_DUMP> dump;
        FunctionLoader<GRALLOC1_PFN_CREATE_DESCRIPTOR,
                GRALLOC1_FUNCTION_CREATE_DESCRIPTOR> createDescriptor;
        FunctionLoader<GRALLOC1_PFN_DESTROY_DESCRIPTOR,
                GRALLOC1_FUNCTION_DESTROY_DESCRIPTOR> destroyDescriptor;
        FunctionLoader<GRALLOC1_PFN_SET_CONSUMER_USAGE,
                GRALLOC1_FUNCTION_SET_CONSUMER_USAGE> setConsumerUsage;
        FunctionLoader<GRALLOC1_PFN_SET_DIMENSIONS,
                GRALLOC1_FUNCTION_SET_DIMENSIONS> setDimensions;
        FunctionLoader<GRALLOC1_PFN_SET_FORMAT,
                GRALLOC1_FUNCTION_SET_FORMAT> setFormat;
        FunctionLoader<GRALLOC1_PFN_SET_PRODUCER_USAGE,
                GRALLOC1_FUNCTION_SET_PRODUCER_USAGE> setProducerUsage;
        FunctionLoader<GRALLOC1_PFN_GET_BACKING_STORE,
                GRALLOC1_FUNCTION_GET_BACKING_STORE> getBackingStore;
        FunctionLoader<GRALLOC1_PFN_GET_CONSUMER_USAGE,
                GRALLOC1_FUNCTION_GET_CONSUMER_USAGE> getConsumerUsage;
        FunctionLoader<GRALLOC1_PFN_GET_DIMENSIONS,
                GRALLOC1_FUNCTION_GET_DIMENSIONS> getDimensions;
        FunctionLoader<GRALLOC1_PFN_GET_FORMAT,
                GRALLOC1_FUNCTION_GET_FORMAT> getFormat;
        FunctionLoader<GRALLOC1_PFN_GET_PRODUCER_USAGE,
                GRALLOC1_FUNCTION_GET_PRODUCER_USAGE> getProducerUsage;
        FunctionLoader<GRALLOC1_PFN_GET_STRIDE,
                GRALLOC1_FUNCTION_GET_STRIDE> getStride;
        FunctionLoader<GRALLOC1_PFN_ALLOCATE,
                GRALLOC1_FUNCTION_ALLOCATE> allocate;
        FunctionLoader<GRALLOC1_PFN_RETAIN,
                GRALLOC1_FUNCTION_RETAIN> retain;
        FunctionLoader<GRALLOC1_PFN_RELEASE,
                GRALLOC1_FUNCTION_RELEASE> release;
        FunctionLoader<GRALLOC1_PFN_GET_NUM_FLEX_PLANES,
                GRALLOC1_FUNCTION_GET_NUM_FLEX_PLANES> getNumFlexPlanes;
        FunctionLoader<GRALLOC1_PFN_LOCK,
                GRALLOC1_FUNCTION_LOCK> lock;
        FunctionLoader<GRALLOC1_PFN_LOCK_FLEX,
                GRALLOC1_FUNCTION_LOCK_FLEX> lockFlex;
        FunctionLoader<GRALLOC1_PFN_LOCK_YCBCR,
                GRALLOC1_FUNCTION_LOCK_YCBCR> lockYCbCr;
        FunctionLoader<GRALLOC1_PFN_UNLOCK,
                GRALLOC1_FUNCTION_UNLOCK> unlock;

        // Adapter-only functions
        FunctionLoader<GRALLOC1_PFN_RETAIN_GRAPHIC_BUFFER,
                GRALLOC1_FUNCTION_RETAIN_GRAPHIC_BUFFER> retainGraphicBuffer;
        FunctionLoader<GRALLOC1_PFN_ALLOCATE_WITH_ID,
                GRALLOC1_FUNCTION_ALLOCATE_WITH_ID> allocateWithId;
    } mFunctions;

}; // class android::Gralloc1::Device

class Loader
{
public:
    Loader();
    ~Loader();

    std::unique_ptr<Device> getDevice();

private:
    static std::unique_ptr<Gralloc1On0Adapter> mAdapter;
    std::unique_ptr<Device> mDevice;
};

} // namespace android::Gralloc1

} // namespace android

#endif
