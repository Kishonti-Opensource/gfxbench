/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef DIRECTXINFO_H
#define DIRECTXINFO_H

#include <string>
#include <vector>

namespace sysinf
{



class DirectxDeviceInfo
{
public:
	uint32_t vendorID;
	uint32_t deviceID;
	uint64_t luid;

	std::string name;

	// feature level: major.minor
	int32_t majorVersion;
	int32_t minorVersion;

	std::vector< std::pair<std::string, std::string> > features_string;
	std::vector< std::pair<std::string, bool> > features_bool;
	std::vector< std::pair<std::string, uint32_t> > features_uint32;

	DirectxDeviceInfo()
        : vendorID(0)
        , deviceID(0)
        , luid(0)
        , majorVersion(0)
        , minorVersion(0)
    { }

	template<class F> void applyVisitor(F visitor) const {

		std::stringstream oss;
		oss << "Feature level: " << majorVersion << "." << minorVersion;
		visitor("major", name); // major string on UI card name
		visitor("minor", oss.str()); // minor string on UI version string

		visitor("vendorID", vendorID);
		visitor("deviceID", deviceID);
		visitor("luid", luid);

		for (const auto &feature : features_string)
		{
			visitor(feature.first, feature.second);
		}

		for (const auto &feature : features_bool)
		{
			visitor(feature.first, feature.second);
		}

		for (const auto &feature : features_uint32)
		{
			visitor(feature.first, feature.second);
		}
	}
};



class DirectxInfo
{
public:
	DirectxInfo()  {}

	std::vector<DirectxDeviceInfo> devices;

	template<class F> void applyVisitor(F visitor) const {
		visitor("devices", devices);
	}
};



class Directx12Info
{
public:
	Directx12Info() {}

	std::vector<DirectxDeviceInfo> dx12_devices;

	template<class F> void applyVisitor(F visitor) const {
		visitor("devices", dx12_devices);
	}
};



}

#endif  // DIRECTXINFO_H
