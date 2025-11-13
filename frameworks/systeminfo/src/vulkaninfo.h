/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef VULKANINFO_H
#define VULKANINFO_H

#include <string>
#include <vector>
#include <sstream>
#include <iomanip>

namespace sysinf
{

class VulkanDeviceInfo
{
public:
	uint32_t vendorID;
	uint32_t deviceID;
	uint64_t luid;

	std::string major; // major string on UI card name
	std::string minor; // minor string on UI version string
	uint32_t driverVersion;
	std::string deviceType;
	std::string card_name;

	int32_t major_vulkan_version;
	int32_t minor_vulkan_version;
	std::string apiVersion;

	bool supportsASTC;
	bool supportsETC2;
	bool supportsDXT5;

	std::vector< std::pair<std::string, int> > limits;
	std::vector< std::pair<std::string, uint32_t> > limits_uint32;
	std::vector< std::pair<std::string, int64_t> > limits64;
	std::vector< std::pair<std::string, float> > limitsf;
	std::vector< std::pair<std::string, bool> > features;
	std::vector<std::string> instance_extensions;
	std::vector<std::string> device_extensions;


	VulkanDeviceInfo()
        : vendorID(0)
        , deviceID(0)
        , luid(0)
        , driverVersion(0)
        , major_vulkan_version(0)
        , minor_vulkan_version(0)
		, supportsASTC(false)
		, supportsETC2(false)
		, supportsDXT5(false)
    { }

private:
	std::string IntToHexString(uint32_t value) const
	{
		std::stringstream s;
		s << "0x" << std::uppercase << std::setfill('0') << std::setw(8) << std::hex << value;
		return s.str();
	}

public:
	template<class F> void applyVisitor(F visitor) const
	{
		visitor("major", major);
		visitor("minor", minor);

		visitor("apiVersion", apiVersion);
		visitor("deviceName", card_name);
		visitor("deviceType", deviceType);
		visitor("driverVersion", driverVersion);

		visitor("vendorID", vendorID);
		visitor("deviceID", deviceID);
		visitor("luid", luid);

		visitor("Instance Extensions", instance_extensions);
		visitor("Device Extensions", device_extensions);

		for (const auto &feature : features)
		{
			visitor(feature.first, feature.second);
		}

		for (const auto &limit : limits)
		{
			visitor(limit.first, limit.second);
		}

		for (const auto &limit : limits_uint32)
		{
			visitor(limit.first, limit.second);
		}

		for (const auto &limit : limits64)
		{
			visitor(limit.first, limit.second);
		}

		for (const auto &limit : limitsf)
		{
			visitor(limit.first, limit.second);
		}
	}
};


class VulkanInfo
{
public:
	std::vector<VulkanDeviceInfo> devices;


	VulkanInfo() {}

	template<class F> void applyVisitor(F visitor) const
	{
		visitor("devices", devices);
	}
};



}

#endif  // VULKANINFO_H
