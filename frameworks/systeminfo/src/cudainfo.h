/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef CUDAINFO_H
#define CUDAINFO_H

#include <string>
#include <vector>

namespace sysinf
{



class CudaDeviceInfo
{
public:
    CudaDeviceInfo(): driverVersion(0), ccMajor(0), ccMinor(0) {}
	std::string name;
    int driverVersion;
	int ccMajor, ccMinor;
    std::vector<std::pair<std::string, int> > attributes;
    
    template<class F> void applyVisitor(F visitor) const {

        std::stringstream oss;
        oss << "CUDA " << ccMajor << "." << ccMinor << " (driver version " << driverVersion << ")";
        visitor("major", name); // major string on UI card name
        visitor("minor", oss.str()); // minor string on UI version string

        visitor("cuDeviceGetName", name);
        visitor("cuDriverGetVersion", driverVersion);
		visitor("ccMajor", ccMajor);
        visitor("ccMinor", ccMinor);

        for (auto i = attributes.begin(); i != attributes.end(); ++i) {
            visitor(i->first, i->second);
        }
    }
};



class CudaInfo
{
public:
    int driverVersion;
    std::vector<CudaDeviceInfo> devices;
    
    template<class F> void applyVisitor(F visitor) const {
        visitor("driverVersion", driverVersion);
        visitor("devices", devices);
    }
};



}

#endif  // CUDAINFO_H
