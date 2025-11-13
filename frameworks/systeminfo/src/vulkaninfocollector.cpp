/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifdef ANDROID
#include "vulkan_wrapper.h"
#endif

#include "vulkaninfocollector.h"
#include <vulkan/vulkan.h>
#include <cstring>
#include <cstdlib>
#include "ng/log.h"
#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif


#define ADD_FEATURE(device_info, field_name) AddFeature(device_info, #field_name, field_name)
#define ADD_LIMIT(device_info, field_name) AddLimit(device_info, #field_name, field_name)
#define CASE_RETURN_STRING(c) case c: return #c

static const char *GetResultString(VkResult result)
{
	switch (result)
	{
		CASE_RETURN_STRING(VK_SUCCESS);
		CASE_RETURN_STRING(VK_NOT_READY);
		CASE_RETURN_STRING(VK_TIMEOUT);
		CASE_RETURN_STRING(VK_EVENT_SET);
		CASE_RETURN_STRING(VK_EVENT_RESET);
		CASE_RETURN_STRING(VK_INCOMPLETE);
		CASE_RETURN_STRING(VK_ERROR_OUT_OF_HOST_MEMORY);
		CASE_RETURN_STRING(VK_ERROR_OUT_OF_DEVICE_MEMORY);
		CASE_RETURN_STRING(VK_ERROR_INITIALIZATION_FAILED);
		CASE_RETURN_STRING(VK_ERROR_DEVICE_LOST);
		CASE_RETURN_STRING(VK_ERROR_MEMORY_MAP_FAILED);
		CASE_RETURN_STRING(VK_ERROR_LAYER_NOT_PRESENT);
		CASE_RETURN_STRING(VK_ERROR_EXTENSION_NOT_PRESENT);
		CASE_RETURN_STRING(VK_ERROR_FEATURE_NOT_PRESENT);
		CASE_RETURN_STRING(VK_ERROR_INCOMPATIBLE_DRIVER);
		CASE_RETURN_STRING(VK_ERROR_TOO_MANY_OBJECTS);
		CASE_RETURN_STRING(VK_ERROR_FORMAT_NOT_SUPPORTED);
		//CASE_RETURN_STRING(VK_ERROR_FRAGMENTED_POOL);
		CASE_RETURN_STRING(VK_ERROR_SURFACE_LOST_KHR);
		CASE_RETURN_STRING(VK_ERROR_NATIVE_WINDOW_IN_USE_KHR);
		CASE_RETURN_STRING(VK_SUBOPTIMAL_KHR);
		CASE_RETURN_STRING(VK_ERROR_OUT_OF_DATE_KHR);
		CASE_RETURN_STRING(VK_ERROR_INCOMPATIBLE_DISPLAY_KHR);
		CASE_RETURN_STRING(VK_ERROR_VALIDATION_FAILED_EXT);
		//CASE_RETURN_STRING(VK_ERROR_INVALID_SHADER_NV);
		//CASE_RETURN_STRING(VK_ERROR_OUT_OF_POOL_MEMORY_KHR);
		//CASE_RETURN_STRING(VK_ERROR_INVALID_EXTERNAL_HANDLE_KHR);
	default:
		return "UNKNOWN_VKRESULT_VALUE";
	}
}

static const char *GetDeviceTypeString(VkPhysicalDeviceType type) {
	switch (type) {
#define STR(r)                        \
	case VK_PHYSICAL_DEVICE_TYPE_##r: \
		return #r
		STR(OTHER);
		STR(INTEGRATED_GPU);
		STR(DISCRETE_GPU);
		STR(VIRTUAL_GPU);
		STR(CPU);
#undef STR
	default:
		return "UNKNOWN_DEVICE";
	}
}

static inline void AddFeature(sysinf::VulkanDeviceInfo &device_info, const char *name, VkBool32 value)
{
	device_info.features.push_back(std::pair<std::string, bool>(name, value == VK_TRUE));
}

static inline void AddLimit(sysinf::VulkanDeviceInfo &device_info, const char *name, int32_t value)
{
	device_info.limits.push_back(std::pair<std::string, int32_t>(name, value));
}

static inline void AddLimit(sysinf::VulkanDeviceInfo &device_info, const char *name, uint32_t value)
{
	device_info.limits_uint32.push_back(std::pair<std::string, uint32_t>(name, value));
}

static inline void AddLimit(sysinf::VulkanDeviceInfo &device_info, const char *name, uint64_t value)
{
	device_info.limits64.push_back(std::pair<std::string, uint64_t>(name, value));
}

static inline void AddLimit(sysinf::VulkanDeviceInfo &device_info, const char *name, float value)
{
	device_info.limitsf.push_back(std::pair<std::string, float>(name, value));
}


void sysinf::collectVulkanInfo(SystemInfo& systemInfo)
{
	NGLOG_INFO("Initializing Vulkan");

#ifdef ANDROID
	if(InitVulkan()==0)
	{
		return;
	}
#endif

#ifdef WIN32
	HMODULE vk_module = LoadLibrary(TEXT("vulkan-1.dll"));
	if (vk_module)
	{
		FreeLibrary(vk_module);
	}
	else
	{
		NGLOG_INFO("vulkan-1.dll missing!");
		return;
	}
#endif

	VkApplicationInfo app_info = {};
	app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	app_info.pApplicationName = "sysinfo";
	app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 3);
	app_info.engineVersion = VK_MAKE_VERSION(1, 0, 3);
	app_info.apiVersion = VK_MAKE_VERSION(1, 0, 3);

	VkInstanceCreateInfo instance_info = {};
	instance_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instance_info.pApplicationInfo = &app_info;
	instance_info.enabledLayerCount = 0;
	instance_info.ppEnabledLayerNames = nullptr;
	instance_info.enabledExtensionCount = 0;
	instance_info.ppEnabledExtensionNames = nullptr;

	VkInstance vulkan_instance;
	VkResult result;
	result = vkCreateInstance(&instance_info, nullptr, &vulkan_instance);
	if (result != VK_SUCCESS)
	{
		NGLOG_WARN("Couldn't create Vulkan instance: %s (%d)\n", GetResultString(result), (int)result);
		return;
	}

	uint32_t deviceCount = 0;
	result = vkEnumeratePhysicalDevices(vulkan_instance, &deviceCount, NULL);
	if (result != VK_SUCCESS)
	{
		NGLOG_WARN("Failed to query the number of physical devices present: %s (%d)\n", GetResultString(result), (int)result);
		return;
	}

	if (deviceCount == 0)
	{
		NGLOG_WARN("Couldn't detect any device present with Vulkan support: %s (%d)\n", GetResultString(result), (int)result);
		return;
	}

	std::vector<VkPhysicalDevice> physicalDevices(deviceCount);
	result = vkEnumeratePhysicalDevices(vulkan_instance, &deviceCount, &physicalDevices[0]);
	if (result != VK_SUCCESS)
	{
		NGLOG_WARN("Failed to enumerate physical devices present: %s (%d)\n", GetResultString(result), (int)result);
		return;
	}

	// Enumerate all physical devices
	VkPhysicalDeviceProperties properties;
	VkPhysicalDeviceFeatures features;


	for (uint32_t i = 0; i < deviceCount; i++)
	{
		VulkanDeviceInfo device;

		memset(&properties, 0, sizeof(properties));
		vkGetPhysicalDeviceProperties(physicalDevices[i], &properties);

		memset(&features, 0, sizeof(features));
		vkGetPhysicalDeviceFeatures(physicalDevices[i], &features);

		device.major_vulkan_version = VK_VERSION_MAJOR(properties.apiVersion);
		device.minor_vulkan_version = VK_VERSION_MINOR(properties.apiVersion);

		std::stringstream oss;
		oss << device.major_vulkan_version << "." << device.minor_vulkan_version << "." << VK_VERSION_PATCH(properties.apiVersion);
		device.apiVersion = oss.str();

		device.major = properties.deviceName;
		oss.str("");
		oss.clear();
		oss << "Vulkan " << device.apiVersion;
		device.minor = oss.str();

		device.driverVersion = properties.driverVersion;
		device.deviceType = GetDeviceTypeString(properties.deviceType);
		device.card_name = properties.deviceName;
		device.vendorID = properties.vendorID;
		device.deviceID = properties.deviceID;
		device.luid = i;

		device.supportsASTC = features.textureCompressionASTC_LDR == VK_TRUE;
		device.supportsETC2 = features.textureCompressionETC2 == VK_TRUE;
		device.supportsDXT5 = features.textureCompressionBC == VK_TRUE;

		ADD_FEATURE(device, features.robustBufferAccess);
		ADD_FEATURE(device, features.fullDrawIndexUint32);
		ADD_FEATURE(device, features.imageCubeArray);
		ADD_FEATURE(device, features.independentBlend);
		ADD_FEATURE(device, features.geometryShader);
		ADD_FEATURE(device, features.tessellationShader);
		ADD_FEATURE(device, features.sampleRateShading);
		ADD_FEATURE(device, features.dualSrcBlend);
		ADD_FEATURE(device, features.logicOp);
		ADD_FEATURE(device, features.multiDrawIndirect);
		ADD_FEATURE(device, features.drawIndirectFirstInstance);
		ADD_FEATURE(device, features.depthClamp);
		ADD_FEATURE(device, features.depthBiasClamp);
		ADD_FEATURE(device, features.fillModeNonSolid);
		ADD_FEATURE(device, features.depthBounds);
		ADD_FEATURE(device, features.wideLines);
		ADD_FEATURE(device, features.largePoints);
		ADD_FEATURE(device, features.alphaToOne);
		ADD_FEATURE(device, features.multiViewport);
		ADD_FEATURE(device, features.samplerAnisotropy);
		ADD_FEATURE(device, features.textureCompressionETC2);
		ADD_FEATURE(device, features.textureCompressionASTC_LDR);
		ADD_FEATURE(device, features.textureCompressionBC);
		ADD_FEATURE(device, features.occlusionQueryPrecise);
		ADD_FEATURE(device, features.pipelineStatisticsQuery);
		ADD_FEATURE(device, features.vertexPipelineStoresAndAtomics);
		ADD_FEATURE(device, features.fragmentStoresAndAtomics);
		ADD_FEATURE(device, features.shaderTessellationAndGeometryPointSize);
		ADD_FEATURE(device, features.shaderImageGatherExtended);
		ADD_FEATURE(device, features.shaderStorageImageExtendedFormats);
		ADD_FEATURE(device, features.shaderStorageImageMultisample);
		ADD_FEATURE(device, features.shaderStorageImageReadWithoutFormat);
		ADD_FEATURE(device, features.shaderStorageImageWriteWithoutFormat);
		ADD_FEATURE(device, features.shaderUniformBufferArrayDynamicIndexing);
		ADD_FEATURE(device, features.shaderSampledImageArrayDynamicIndexing);
		ADD_FEATURE(device, features.shaderStorageBufferArrayDynamicIndexing);
		ADD_FEATURE(device, features.shaderStorageImageArrayDynamicIndexing);
		ADD_FEATURE(device, features.shaderClipDistance);
		ADD_FEATURE(device, features.shaderCullDistance);
		ADD_FEATURE(device, features.shaderFloat64);
		ADD_FEATURE(device, features.shaderInt64);
		ADD_FEATURE(device, features.shaderInt16);
		ADD_FEATURE(device, features.shaderResourceResidency);
		ADD_FEATURE(device, features.shaderResourceMinLod);
		ADD_FEATURE(device, features.sparseBinding);
		ADD_FEATURE(device, features.sparseResidencyBuffer);
		ADD_FEATURE(device, features.sparseResidencyImage2D);
		ADD_FEATURE(device, features.sparseResidencyImage3D);
		ADD_FEATURE(device, features.sparseResidency2Samples);
		ADD_FEATURE(device, features.sparseResidency4Samples);
		ADD_FEATURE(device, features.sparseResidency8Samples);
		ADD_FEATURE(device, features.sparseResidency16Samples);
		ADD_FEATURE(device, features.sparseResidencyAliased);
		ADD_FEATURE(device, features.variableMultisampleRate);
		ADD_FEATURE(device, features.inheritedQueries);

		const VkPhysicalDeviceLimits &limits = properties.limits;
		if (properties.limits.maxPushConstantsSize < 128)
		{
			properties.limits.maxPushConstantsSize = 128;
		}

		ADD_LIMIT(device, limits.maxImageDimension1D);
		ADD_LIMIT(device, limits.maxImageDimension2D);
		ADD_LIMIT(device, limits.maxImageDimension3D);
		ADD_LIMIT(device, limits.maxImageDimensionCube);
		ADD_LIMIT(device, limits.maxImageArrayLayers);
		ADD_LIMIT(device, limits.maxTexelBufferElements);
		ADD_LIMIT(device, limits.maxUniformBufferRange);
		ADD_LIMIT(device, limits.maxStorageBufferRange);
		ADD_LIMIT(device, limits.maxPushConstantsSize);
		ADD_LIMIT(device, limits.maxMemoryAllocationCount);
		ADD_LIMIT(device, limits.maxSamplerAllocationCount);
		ADD_LIMIT(device, limits.bufferImageGranularity);
		ADD_LIMIT(device, limits.sparseAddressSpaceSize);
		ADD_LIMIT(device, limits.maxBoundDescriptorSets);
		ADD_LIMIT(device, limits.maxPerStageDescriptorSamplers);
		ADD_LIMIT(device, limits.maxPerStageDescriptorUniformBuffers);
		ADD_LIMIT(device, limits.maxPerStageDescriptorStorageBuffers);
		ADD_LIMIT(device, limits.maxPerStageDescriptorSampledImages);
		ADD_LIMIT(device, limits.maxPerStageDescriptorStorageImages);
		ADD_LIMIT(device, limits.maxPerStageDescriptorInputAttachments);
		ADD_LIMIT(device, limits.maxPerStageResources);
		ADD_LIMIT(device, limits.maxDescriptorSetSamplers);
		ADD_LIMIT(device, limits.maxDescriptorSetUniformBuffers);
		ADD_LIMIT(device, limits.maxDescriptorSetUniformBuffersDynamic);
		ADD_LIMIT(device, limits.maxDescriptorSetStorageBuffers);
		ADD_LIMIT(device, limits.maxDescriptorSetStorageBuffersDynamic);
		ADD_LIMIT(device, limits.maxDescriptorSetSampledImages);
		ADD_LIMIT(device, limits.maxDescriptorSetStorageImages);
		ADD_LIMIT(device, limits.maxDescriptorSetInputAttachments);
		ADD_LIMIT(device, limits.maxVertexInputAttributes);
		ADD_LIMIT(device, limits.maxVertexInputBindings);
		ADD_LIMIT(device, limits.maxVertexInputAttributeOffset);
		ADD_LIMIT(device, limits.maxVertexInputBindingStride);
		ADD_LIMIT(device, limits.maxVertexOutputComponents);
		ADD_LIMIT(device, limits.maxTessellationGenerationLevel);
		ADD_LIMIT(device, limits.maxTessellationPatchSize);
		ADD_LIMIT(device, limits.maxTessellationControlPerVertexInputComponents);
		ADD_LIMIT(device, limits.maxTessellationControlPerVertexOutputComponents);
		ADD_LIMIT(device, limits.maxTessellationControlPerPatchOutputComponents);
		ADD_LIMIT(device, limits.maxTessellationControlTotalOutputComponents);
		ADD_LIMIT(device, limits.maxTessellationEvaluationInputComponents);
		ADD_LIMIT(device, limits.maxTessellationEvaluationOutputComponents);
		ADD_LIMIT(device, limits.maxGeometryShaderInvocations);
		ADD_LIMIT(device, limits.maxGeometryInputComponents);
		ADD_LIMIT(device, limits.maxGeometryOutputComponents);
		ADD_LIMIT(device, limits.maxGeometryOutputVertices);
		ADD_LIMIT(device, limits.maxGeometryTotalOutputComponents);
		ADD_LIMIT(device, limits.maxFragmentInputComponents);
		ADD_LIMIT(device, limits.maxFragmentOutputAttachments);
		ADD_LIMIT(device, limits.maxFragmentDualSrcAttachments);
		ADD_LIMIT(device, limits.maxFragmentCombinedOutputResources);
		ADD_LIMIT(device, limits.maxComputeSharedMemorySize);
		ADD_LIMIT(device, limits.maxComputeWorkGroupCount[0]);
		ADD_LIMIT(device, limits.maxComputeWorkGroupCount[1]);
		ADD_LIMIT(device, limits.maxComputeWorkGroupCount[2]);
		ADD_LIMIT(device, limits.maxComputeWorkGroupInvocations);
		ADD_LIMIT(device, limits.maxComputeWorkGroupSize[0]);
		ADD_LIMIT(device, limits.maxComputeWorkGroupSize[1]);
		ADD_LIMIT(device, limits.maxComputeWorkGroupSize[2]);
		ADD_LIMIT(device, limits.subPixelPrecisionBits);
		ADD_LIMIT(device, limits.subTexelPrecisionBits);
		ADD_LIMIT(device, limits.mipmapPrecisionBits);
		ADD_LIMIT(device, limits.maxDrawIndexedIndexValue);
		ADD_LIMIT(device, limits.maxDrawIndirectCount);
		ADD_LIMIT(device, limits.maxSamplerLodBias);
		ADD_LIMIT(device, limits.maxSamplerAnisotropy);
		ADD_LIMIT(device, limits.maxViewports);
		ADD_LIMIT(device, limits.maxViewportDimensions[0]);
		ADD_LIMIT(device, limits.maxViewportDimensions[1]);
		ADD_LIMIT(device, limits.viewportBoundsRange[0]);
		ADD_LIMIT(device, limits.viewportBoundsRange[1]);
		ADD_LIMIT(device, limits.viewportSubPixelBits);
		ADD_LIMIT(device, limits.minMemoryMapAlignment);
		ADD_LIMIT(device, limits.minTexelBufferOffsetAlignment);
		ADD_LIMIT(device, limits.minUniformBufferOffsetAlignment);
		ADD_LIMIT(device, limits.minStorageBufferOffsetAlignment);
		ADD_LIMIT(device, limits.minTexelOffset);
		ADD_LIMIT(device, limits.maxTexelOffset);
		ADD_LIMIT(device, limits.minTexelGatherOffset);
		ADD_LIMIT(device, limits.maxTexelGatherOffset);
		ADD_LIMIT(device, limits.minInterpolationOffset);
		ADD_LIMIT(device, limits.maxInterpolationOffset);
		ADD_LIMIT(device, limits.subPixelInterpolationOffsetBits);
		ADD_LIMIT(device, limits.maxFramebufferWidth);
		ADD_LIMIT(device, limits.maxFramebufferHeight);
		ADD_LIMIT(device, limits.maxFramebufferLayers);
		ADD_LIMIT(device, limits.framebufferColorSampleCounts);
		ADD_LIMIT(device, limits.framebufferDepthSampleCounts);
		ADD_LIMIT(device, limits.framebufferStencilSampleCounts);
		ADD_LIMIT(device, limits.framebufferNoAttachmentsSampleCounts);
		ADD_LIMIT(device, limits.maxColorAttachments);
		ADD_LIMIT(device, limits.sampledImageColorSampleCounts);
		ADD_LIMIT(device, limits.sampledImageIntegerSampleCounts);
		ADD_LIMIT(device, limits.sampledImageDepthSampleCounts);
		ADD_LIMIT(device, limits.sampledImageStencilSampleCounts);
		ADD_LIMIT(device, limits.storageImageSampleCounts);
		ADD_LIMIT(device, limits.maxSampleMaskWords);
		ADD_LIMIT(device, limits.timestampComputeAndGraphics);
		ADD_LIMIT(device, limits.timestampPeriod);
		ADD_LIMIT(device, limits.maxClipDistances);
		ADD_LIMIT(device, limits.maxCullDistances);
		ADD_LIMIT(device, limits.maxCombinedClipAndCullDistances);
		ADD_LIMIT(device, limits.discreteQueuePriorities);
		ADD_LIMIT(device, limits.pointSizeRange[0]);
		ADD_LIMIT(device, limits.pointSizeRange[1]);
		ADD_LIMIT(device, limits.lineWidthRange[0]);
		ADD_LIMIT(device, limits.lineWidthRange[1]);
		ADD_LIMIT(device, limits.pointSizeGranularity);
		ADD_LIMIT(device, limits.lineWidthGranularity);
		ADD_LIMIT(device, limits.strictLines);
		ADD_LIMIT(device, limits.standardSampleLocations);
		ADD_LIMIT(device, limits.optimalBufferCopyOffsetAlignment);
		ADD_LIMIT(device, limits.optimalBufferCopyRowPitchAlignment);
		ADD_LIMIT(device, limits.nonCoherentAtomSize);

		const VkPhysicalDeviceSparseProperties &sparseProperties = properties.sparseProperties;
		ADD_LIMIT(device, sparseProperties.residencyStandard2DBlockShape);
		ADD_LIMIT(device, sparseProperties.residencyStandard2DMultisampleBlockShape);
		ADD_LIMIT(device, sparseProperties.residencyStandard3DBlockShape);
		ADD_LIMIT(device, sparseProperties.residencyAlignedMipSize);
		ADD_LIMIT(device, sparseProperties.residencyNonResidentStrict);

		// These aren't per device, but they will be displayed with the device
		std::vector<VkExtensionProperties> instance_extensions;
		uint32_t num_instance_extensions = 0;
		result = vkEnumerateInstanceExtensionProperties(nullptr, &num_instance_extensions, nullptr);
		if (num_instance_extensions > 0 && result == VK_SUCCESS)
		{
			instance_extensions.resize(num_instance_extensions);
			result = vkEnumerateInstanceExtensionProperties(nullptr, &num_instance_extensions, instance_extensions.data());

			if (result != VK_SUCCESS && result != VK_INCOMPLETE)
			{
				instance_extensions.clear();
			}
			else if (num_instance_extensions > 0
				&& static_cast<size_t>(num_instance_extensions) < instance_extensions.size())
			{
				instance_extensions.resize(num_instance_extensions);
			}
		}

		for (const auto &extension : instance_extensions)
		{
			oss.str("");
			oss.clear();
			oss << extension.extensionName << " (ver. " << extension.specVersion << ")";
			device.instance_extensions.push_back(oss.str());
		}

		std::vector<VkExtensionProperties> device_extensions;
		uint32_t num_device_extensions = 0;
		result = vkEnumerateDeviceExtensionProperties(physicalDevices[i], nullptr, &num_device_extensions, nullptr);
		if (num_device_extensions > 0 && result == VK_SUCCESS)
		{
			device_extensions.resize(num_device_extensions);
			result = vkEnumerateDeviceExtensionProperties(physicalDevices[i], nullptr, &num_device_extensions, device_extensions.data());

			if (result != VK_SUCCESS && result != VK_INCOMPLETE)
			{
				device_extensions.clear();
			}
			else if (num_device_extensions > 0
				&& static_cast<size_t>(num_device_extensions) < device_extensions.size())
			{
				device_extensions.resize(num_device_extensions);
			}
		}

		for (const auto &extension : device_extensions)
		{
			oss.str("");
			oss.clear();
			oss << extension.extensionName << " (ver. " << extension.specVersion << ")";
			device.device_extensions.push_back(oss.str());
		}

		systemInfo.vulkanInfo.devices.push_back(device);
	}

	systemInfo.hasVulkan = true;
}
