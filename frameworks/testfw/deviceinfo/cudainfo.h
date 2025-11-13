/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef CUDAINFO_H_
#define CUDAINFO_H_


#include <string>
#include <vector>

namespace tfw
{


class CudaDeviceInfo
{
public:
    CudaDeviceInfo(): totalMemory(0) {}
    std::string name;
    int driverVersion;
    size_t totalMemory;
    std::vector<std::pair<std::string, int> > attributes;
};

template<class F> void applyVisitor(const CudaDeviceInfo& info, F visitor) {
    visitor("cuDeviceGetName", info.name);
    visitor("cuDriverGetVersion", info.driverVersion);
    visitor("cuDeviceTotalMem", info.totalMemory);
    for (auto i = info.attributes.begin(); i != info.attributes.end(); ++i) {
        visitor(i->first, i->second);
    }
}



class CudaInfoCollector
{
public:
    CudaInfoCollector();
    void collect();

    bool isCudaAvailable() const;
    int deviceCount() const;
    const CudaDeviceInfo& device(int i) const;
    std::string serialize() const;
    int driverVersion() const;
private:
    int driverVersion_;
    std::vector<CudaDeviceInfo> devices_;

};



}

#endif  // CUDAINFO_H_
